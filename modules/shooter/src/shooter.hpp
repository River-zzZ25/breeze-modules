/**
 * @file shooter.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 发射控制接口、热量限制与弹速自适应数据定义
 * @version 0.1
 * @date 2026-11-02
 * @note 实现应用层使能控制、唤醒准备、发射限制和整组摩擦轮清堵; 弹速自适应待实现.
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
 * @brief 模块使能与原始发射意图
 *
 */
struct ShootEnableInfo
{
	breeze::models::FiringIntent Intent{breeze::models::FiringIntent::Idle}; ///< 原始发射意图
	bool Enable{};                                                           ///< 模块使能, 由 Module::In.Enable 提供
};

/**
 * @brief 本周期发射意图、剩余资源与设备反馈, Update 不修改或保存其副本
 *
 */
template <std::size_t FricCount>
struct ShooterInput
{
	ShootEnableInfo EnableInfo{};                        ///< 应用层决定唤醒或休眠, Idle 只停止发射
	std::uint16_t RemainingHeat{};                       ///< 应用层计算的剩余热量
	std::uint16_t AllowShootCount{};                     ///< 剩余允许发弹量
	FeedFeedback Feed{};                                 ///< 拨盘轴原始反馈
	etl::array<FricFeedback, FricCount> FricFeedbacks{}; ///< 各摩擦轮原始反馈
};

/**
 * @brief 本周期热量与弹量限制结果
 *
 */
struct ShootHeatLimitResult
{
	bool AllowShoot{};   ///< 是否允许开始拨弹
	float RepeatSpeed{}; ///< 连发目标角速度(°/s), 拨弹方向为正
};

/**
 * @brief 弹速统计信息
 *
 */
struct ShootSpeedStats
{
	etl::circular_buffer<float, SPEED_STAT_COUNT> SpeedBuf; ///< 弹速样本(m/s)
	std::uint8_t HighSpeedCount;                            ///< 统计窗口内超过弹速阈值的样本数
	float LatestSpeed;                                      ///< 最新弹速(m/s)
	float StdDev;                                           ///< 弹速标准差(m/s)
	float Average;                                          ///< 平均弹速(m/s)
	float Range;                                            ///< 弹速极差(m/s)
	float Variance;                                         ///< 弹速方差((m/s)^2)
	std::uint8_t AbnormalCount;                             ///< 被剔除的异常弹速样本数
	bool IsValid;                                           ///< 统计结果是否有效
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
	float CurrentShootSpeed;                                        ///< 当前弹速(m/s)
	float CurrentOverSpeed;                                         ///< 当前弹速超过理想上限的差值(m/s)
	float FricLatestSpeed;                                          ///< 上一发对应的摩擦轮目标转速(rpm)
	float CurrentDivide;                                            ///< 高速减速计算分母
	std::uint16_t AddFricSpeed;                                     ///< 当前摩擦轮目标转速增加量(rpm)
	std::uint16_t DecFricSpeed;                                     ///< 当前摩擦轮目标转速减少量(rpm)

	float CurrentFricTargetSpeed; ///< 当前摩擦轮目标转速(rpm)
};

/**
 * @brief 发射模块跨周期控制状态
 *
 */
struct ShooterControlInfo
{
	breeze::models::FiringIntent LastIntent{breeze::models::FiringIntent::Idle}; ///< 上一周期原始发射意图
	ShootState State{ShootState::Sleep};                                         ///< 发射模块状态
	bool FricReversing{};                                                        ///< 整组摩擦轮是否正在反转清堵
	float FricReverseElapsedMs{};                                                ///< 本次摩擦轮反转已持续时间(ms)
	etl::optional<ShootSpeedAdaptInfo> SpeedAdaptInfo{};                         ///< 弹速自适应信息
	etl::optional<ShootSpeedStats> SpeedStats{};                                 ///< 弹速统计信息
};

/**
 * @brief 集成摩擦轮与拨盘的发射控制器
 * @note 根据应用层使能管理唤醒准备、单发与连发及整组摩擦轮清堵.
 *       弹速自适应接口待实现; 正常摩擦轮目标使用各轮 FricConfig::InintialFricTargetSpeed 配置.
 * @tparam FricCount 摩擦轮数量
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
	 * @brief 复位控制状态和输出, 进入 Sleep
	 *
	 * @note 当前直接调用 Reset; 后续由调用方周期调用 Update 并传入使能状态.
	 *       本函数不创建线程或周期调度.
	 */
	void Start();

	/**
	 * @brief 更新整机生命周期、摩擦轮与拨盘控制
	 *
	 * @param input 本周期完整输入
	 * @param dtMs 控制周期(ms)
	 * @note Enable 为 true 时唤醒机构, 准备完成后才开始拨弹; Idle 只停止发射, 不休眠.
	 *       进入 Init 及 Init 期间的单发只用于准备, 不缓存; 完成后需新的单发边沿才能发射.
	 *       忙碌期间的新单发不排队, 被拒绝后不补发; 连发准备完成后按持续意图执行.
	 *       资源不足或摩擦轮清堵时下发 Hold; 已开始的拨盘动作继续完成.
	 *       休眠时机由应用层决定, Enable 为 false 时立即复位卸力.
	 */
	void Update(const ShooterInput<FricCount>& input, float dtMs);

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
	 * @brief 清除整机控制状态及 Fric/Feed 的 PID、堵转历史和输出, 进入 Sleep
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
	 * @brief 等待摩擦轮与拨盘准备完成, 随后根据拨盘状态更新发射状态
	 *
	 */
	void UpdateState(bool fricReady, float feedAngle);

	/**
	 * @brief 更新整组摩擦轮, 任一轮堵转后以配置速度反转 500ms, 随后恢复并重新检测
	 *
	 * @note 反转期间不检测堵转; 进入 Sleep 或失能时中止清堵.
	 */
	void UpdateFric(const etl::array<FricFeedback, FricCount>& feedbacks, float dtMs);

	/**
	 * @brief 根据剩余热量和弹量计算本周期允许状态及连发速度
	 *
	 */
	[[nodiscard]] ShootHeatLimitResult UpdateHeatLimit(
	    std::uint16_t remainingHeat,
	    std::uint16_t allowShootCount
	) const;

	ShooterConfig<FricCount> _conf;
	ShooterControlInfo _controlInfo{};

	etl::array<Fric, FricCount> _fric; ///< 摩擦轮对象
	Feed _feed;                        ///< 拨盘对象
};

} // namespace breeze::shooter
