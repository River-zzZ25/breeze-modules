# Shooter Module
发射模块由外部 ramen/ramenx 周期源驱动。`Module` 只持有控制器和输入缓存，
`Start()` 初始化控制状态，`In.Tick` 每收到一次实际时间间隔（毫秒）就执行一次控制。

以下接线放在应用初始化中，`shooterConfig` 由应用提供：

```cpp
#include <breeze/shooter/module.hpp>
#include <ramenx/time.hpp>

static breeze::shooter::Module<3> shooter(shooterConfig);
static ramen::FrameProvider<2048> frame({
    .period = K_MSEC(1),
    .priority = 2,
    .name = "shooter_clock",
});
static ramen::PushUnary<float, k_ticks_t> tickToMs = [](const k_ticks_t& delta) {
    return static_cast<float>(k_ticks_to_us_floor64(delta)) / 1000.0f;
};

frame.out_delta >> tickToMs.in;
tickToMs.out >> shooter.In.Tick;

// 应用先连接使能、资源、命令、逐轮反馈和力矩输出端口。
shooter.Start();
frame.start();
```

`FrameProvider`、转换器及模块均常驻；周期、优先级和栈由应用层安排。
Tick 由单一周期源串行推送，控制回调在推送者上下文中同步执行。
发射使能通过 `In.Enable` 输入，`Start()` 不隐式使能发射。


拨盘速度统一使用输出轴角速度 `°/s`：反馈、连发目标、复位速度、堵转速度阈值，
以及位置 PID 的输出和输出限幅均使用该单位。摩擦轮速度继续使用 `rpm`。
应用层在输入边界完成驱动速度换算；输出轴 `rpm × 6 = °/s`，电机轴反馈还需先按减速比换算到输出轴。

`FeedSpeedCoefficient` 表示每发拨弹转过的角度，乘以射频 `Hz` 得到连发角速度 `°/s`。
例如每发转过 `90°`、射频为 `2 Hz`，目标角速度为 `180°/s`。
旧配置中的拨盘 rpm 速度值、速度阈值和 `rpm/Hz` 换算系数迁移时需乘以 6；PID 参数需按各控制环的新输入输出单位调整。
