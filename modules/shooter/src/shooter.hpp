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

#include "feed.hpp"
#include "fric.hpp"
#include "state.hpp"
#include <breeze/models/control.hpp>
#include <breeze/shooter/config.hpp>
#include <breeze/shooter/io.hpp>
#include <cstddef>
#include <cstdint>
#include <etl/array.h>
#include <etl/circular_buffer.h>
#include <etl/optional.h>
#include <utility>

namespace breeze::shooter
{

constexpr std::uint8_t SPEED_STAT_COUNT = 50;
constexpr std::uint16_t SPEED_COUNT_MAX = 10;

/**
 * @brief
 * @note 用于存储发射使能信息。
 */
struct ShootEnableInfo
{
	breeze::models::FiringIntent Intent{breeze::models::FiringIntent::Idle}; ///< 原始发射意图
	bool Enable{}; ///< 模块使能, 由 Module::SetEnabled 提供
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
	ShootState State{ShootState::Sleep};                 ///< 发射模块状态
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
	explicit Shooter(const ShooterConfig<FricCount>& conf);

	/**
	 * @brief 启动 Shooter 功能入口, 当前执行一次 Update(1ms)
	 *
	 * @note 不创建线程或周期调度; 后续功能在此补充, 模块使能仍由 Module 接口负责.
	 */
	void Start();

	/**
	 * @brief 接收单个新弹速样本(m/s); 当前仅声明
	 *
	 */
	void UpdateShootSpeed(float speed);

	/**
	 * @brief 读取拨盘状态
	 *
	 */
	[[nodiscard]] const FeedInfo& GetFeedStatus() const;

	/**
	 * @brief 读取拨盘输出力矩(N·m), 电机原始极性
	 *
	 */
	[[nodiscard]] float GetFeedOutput() const;

	/**
	 * @brief 读取整机内部状态
	 *
	 */
	[[nodiscard]] ShootState GetState() const { return _controlInfo.State; }

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
	void UpdateFric(
	    const etl::array<FricCmd, FricCount>& commands,
	    const etl::array<FricFeedback, FricCount>& feedbacks,
	    float dtMs
	);

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
	 * @brief 发射模块重置函数
	 *
	 */
	void Reset();

	Shooter(const Shooter&) = delete;
	Shooter(Shooter&&) = delete;
	Shooter& operator=(const Shooter&) = delete;
	Shooter& operator=(Shooter&&) = delete;

private:
	/**
	 * @brief 按配置索引直接构造不可复制、不可移动的 Fric 数组
	 *
	 */
	template <std::size_t... Indices>
	Shooter(const ShooterConfig<FricCount>& conf, std::index_sequence<Indices...>);

	/**
	 * @brief 控制周期更新入口, 由 Start 调用; 控制业务待补充
	 *
	 */
	void Update(float dtMs);

	/**
	 * @brief 更新整机状态机; 当前仅声明
	 *
	 */
	void UpdateState();

	/**
	 * @brief 根据热量和弹量限制发射; 当前仅声明
	 *
	 */
	void UpdateHeatLimit(std::uint16_t heatValue, std::uint16_t allowShootCount);

	/**
	 * @brief 更新拨盘控制; 当前仅声明
	 *
	 */
	void UpdateFeed(const FeedCmd& command, const FeedFeedback& feedback, float dtMs);

	ShooterConfig<FricCount> _conf;
	ShooterControlInfo _controlInfo{};

	etl::array<Fric, FricCount> _fric; ///< 摩擦轮对象
	Feed _feed;                        ///< 拨盘对象
};

} // namespace breeze::shooter

#include "shooter.cpp"
