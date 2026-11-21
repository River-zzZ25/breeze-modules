/**
 * @file shooter.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief
 * @version 0.1
 * @date 2026-11-02
 * @note 在Shooter中实现弹速只适应，热量限制等计算
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "state.hpp"
#include <breeze/shooter/config.hpp>
#include <breeze/shooter/io.hpp>
#include <cstddef>
#include <cstdint>
#include <etl/array.h>
#include <etl/circular_buffer.h>
#include <etl/optional.h>
#include <etl/span.h>
#include <fric.hpp>

namespace breeze::shooter
{

constexpr std::uint8_t SPEED_STAT_COUNT = 50;
constexpr std::uint16_t SPEED_COUNT_MAX = 10;

class Feed;

/**
 * @brief 发射模式枚举
 *
 */
enum class ShootMode : std::uint8_t
{
	None,   ///< 无模式
	Single, ///< 单发
	Repeat  ///< 连发
};

/**
 * @brief
 * @note 用于存储发射使能信息。
 */
struct ShootEnableInfo
{
	bool CtrlFlag;  ///< 遥控器状态：关控直接整个模块离线，开控才允许发射
	ShootMode Mode; ///< 发射模式：判断用户的发射模式是单发还是连发
	bool Enable;    ///< 是否允许发射：判断当前是否可以发射
};

/**
 * @brief 弹速统计信息
 *
 */
struct ShootSpeedStats
{
	etl::circular_buffer<float, SPEED_STAT_COUNT> SpeedBuf; // 弹速环形缓存
	std::uint8_t HighSpeedCount;                            // 最近50发中弹速超过阈值的数量
	float LatestSpeed;                                      // 最近一颗弹丸初速度
	float StdDev;                                           // 标准差
	float Average;                                          // 平均弹速
	float Range;                                            // 极差(最大值-最小值)
	float Variance;                                         // 方差
	std::uint8_t AbnormalCount;                             // 异常(剔除)弹速计数
	bool IsValid;                                           // 统计是否有效;
};

/**
 * @brief 弹速自适应调试信息
 *
 */

struct ShootSpeedAdaptInfo
{
	etl::circular_buffer<float, SPEED_COUNT_MAX> RecentShootSpeeds; ///< 最近发射的弹速
	std::uint16_t LowSpeedCount;                                    ///< 欠速计数
	std::uint16_t HighSpeedCount;                                   ///< 高速计数
	float CurrentShootSpeed;                                        ///< 当前发射的弹速
	float CurrentOverSpeed;                                         ///< 当前超理想上限速度差
	float FricLatestSpeed;      ///< 上一发摩擦轮转速(rpm),可以在算出FricTargetSpeed后，直接赋值给这个变量；
	float CurrentDivide;        ///< 高速减速计算分母
	std::uint16_t AddFricSpeed; ///< 当前摩擦轮转速增加量
	std::uint16_t DecFricSpeed; ///< 当前摩擦轮转速减少量

	float CurrentFricTargetSpeed; ///< 当前摩擦轮目标转速(rpm)
};

/**
 * @brief 发射模块配置
 *
 * @tparam FricCount
 */
struct ShooterControlInfo
{
	ShootEnableInfo EnableInfo{};                        ///< 发射使能信息
	ShootState State{};                                  ///< 发射模块状态
	etl::optional<ShootSpeedAdaptInfo> SpeedAdaptInfo{}; ///< 弹速自适应信息
	etl::optional<ShootSpeedStats> SpeedStats{};         ///< 弹速统计信息
};

/**
 * @brief 发射类
 * @note
 * shooter只对摩擦轮和拨盘进行抽象。在接收到数据后，shooter会根据开火意图和上一发弹速来更新摩擦轮和拨盘的控制参数。
 *       弹速自适应，在接收到上一次数据后
 */
template <std::size_t FricCount>
class Shooter
{
public:
	/**
	 * @brief 构造函数
	 *
	 * @param conf Shooter配置
	 * @note 该构造函数用于初始化Shooter类的配置，包括摩擦轮和拨盘的配置，以及弹速自适应和热量限制的配置。
	 */
	Shooter(const ShooterConfig<FricCount>& conf);

	/**
	 * @brief 更新整组摩擦轮
	 * @param commands 各摩擦轮本周期命令
	 * @param feedbacks 各摩擦轮本周期反馈
	 * @param dtMs 控制周期(ms)
	 *
	 * @note 本接口当前仅声明. 后续实现约定: 任一摩擦轮上报 Stuck 后,
	 *       整组以 FricReverseSpeed 反转 500ms;
	 *       到时恢复用户 Running 目标并清零堵转计数, 仍满足条件时再次进入清堵.
	 *       反转期间不检测堵转, 停止类命令优先并立即中止清堵.
	 */
	void UpdateFric(const etl::array<FricCmd, FricCount>& commands,
	                const etl::array<FricFeedback, FricCount>& feedbacks, float dtMs);

	/**
	 * @brief 读取指定摩擦轮状态
	 * @param index 摩擦轮索引, 调用方保证 index < FricCount
	 */
	[[nodiscard]] const FricInfo& GetFricStatus(std::size_t index) const;

	/**
	 * @brief 读取指定摩擦轮本周期电机力矩指令
	 * @param index 摩擦轮索引, 调用方保证 index < FricCount
	 * @return 电机正转极性下的力矩(N·m)
	 */
	[[nodiscard]] float GetFricOutput(std::size_t index) const;

	/**
	 * @brief 发射模块启动函数
	 * @note 该函数用于启动发射模块，将模块状态设置为运行状态。
	 */
	void ShootStart();

	/**
	 * @brief 发射模块重置函数
	 *
	 */
	void Reset();

	Shooter(const Shooter&) = delete;
	Shooter(Shooter&&) = delete;
	Shooter& operator=(const Shooter&) = delete;
	Shooter& operator=(Shooter&&) = delete;

private:
	ShooterConfig<FricCount> _conf;
	ShooterControlInfo _controlInfo;

	etl::array<Fric, FricCount> _fric; ///< 摩擦轮对象
	std::size_t _fricCount;            ///< 摩擦轮数量
	Feed* _feed;                       ///< 拨盘对象
};

#pragma region InputOutput

#pragma endregion InputOutput

} // namespace breeze::shooter
