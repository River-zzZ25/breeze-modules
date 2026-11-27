# 发射模块（Shooter）

这个模块负责把发射命令和目标速度转换成电机力矩输出，完成摩擦轮控速、拨盘拨弹和堵转清理。
应用层决定什么时候发射、以多快的速度发射；热量限制、剩余弹量判断、弹速自适应和普通/吊射转速选择都放在应用层。

## 一、怎么使用

### 1. 启用模块并准备配置

工程需要接入 `breeze-modules`、`ramenx`、`algo` 和 ETL，使用 C++20。在应用的 `prj.conf` 中启用：

```ini
CONFIG_CPP=y
CONFIG_STD_CPP20=y
CONFIG_ALGO=y
CONFIG_BREEZE_MODULES=y
CONFIG_BREEZE_MODULE_REFEREE=y
CONFIG_BREEZE_MODULE_SHOOTER=y
```


构造模块前，准备好 `ShooterConfig<FricCount>`：

|         配置        |                  需要填写的内容                       |
| ------------------ | -------------------------------                     |
|   `FricConf[i]`    | 第 i 个摩擦轮的安装方向、速度 PID、到速判断容差            |
| `FricReverseSpeed` | 摩擦轮清堵时的反转速度大小，单位 rpm                     |
|    `FeedConf`      | 拨盘方向、三组 PID、弹位模式、复位参数、到位容差、动作超时和重载堵转次数上限 |

`FricCount` 是摩擦轮数量，目前支持直接使用 2、3、6 路。应用中只创建一个常驻模块，不需要反复构造、销毁。
拨盘可以选择相对等距弹位或绝对弹位表，两种模式的区别见下文。

### 2. 连接输入和输出

包含公开头文件后，通过ramenx和 `In` 和 `Out` 进行接线。

| 端口 | 用途 |
| --- | --- |
| `In.Command` | 发射命令：Sleep、Idle、Once、Repeat，默认 Sleep |
| `In.FricTargetSpeeds` | 每个摩擦轮的目标转速大小，类型为 `etl::array<float, FricCount>`，单位 rpm |
| `In.FeedTargetSpeed` | 连发时拨盘的目标角速度大小，单位 °/s，默认零 |
| `In.Tick` | 触发一次控制更新，传入距上次更新的实际时间，单位 ms，必须大于零 |
| `In.FricFeedback[i]` | 每个控制周期主动读取第 i 个摩擦轮的 `Speed` 和 `Stuck` |
| `In.FeedFeedback` | 每个控制周期主动读取拨盘的 `Speed`、`Angle` 和 `Stuck` |
| `Out.FricOutput[i]` | 第 i 个摩擦轮的力矩指令，单位 N·m |
| `Out.FeedOutput` | 拨盘的力矩指令，单位 N·m |

命令和目标速度是推送输入，收到后保存最新值。设备反馈是拉取输入，每次 Tick 都会读取；力矩输出则在计算完成后推送。

```cpp
#include <breeze/shooter/module.hpp>
#include <ramenx/time.hpp>

static breeze::shooter::Module<2> shooter(shooterConfig);

void Sample()
{

    shooter.In.FricFeedback[0] >> leftFeedback;
    shooter.In.FricFeedback[1] >> rightFeedback;
    shooter.In.FeedFeedback >> feedFeedback;
    shooter.Out.FricOutput[0] >> leftOutput;
    shooter.Out.FricOutput[1] >> rightOutput;
    shooter.Out.FeedOutput >> feedOutput;

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

    shooter.Start();
    shooter.In.FricTargetSpeeds({6300.0f, 6300.0f});
    shooter.In.FeedTargetSpeed(180.0f);
    shooter.In.Command(breeze::shooter::ShooterCommand::Idle);
    frame.start();
}
```

初始化函数只调用一次。示例中的转速、周期、线程栈和优先级仅用于展示接线方式，实际值由应用决定。
`FrameProvider` 由应用持有；发射模块内部不创建线程。也可以用已有周期源直接向 `In.Tick` 推送毫秒间隔。

`Start()` 只清理控制状态和输出，不会启动周期任务，也不会让机构开转。
任意非 Sleep 命令会在随后的控制周期唤醒机构。所有反馈适配器、输出适配器和周期源都要保持有效。
Tick 应由一个线程定时推送，计算会在推送者的上下文中执行（使用ramenx的`FrameProvider`进行定时调用）。其它输入回调只更新缓存，不运行 PID。
各输入独立保存最新值，不保证多个端口在同一时刻一起更新。


### 3. 下发发射命令

| 命令 | 实际行为 |
| --- | --- |
| `Sleep` | 下一 Tick 中止所有动作、清除控制历史并输出零力矩；目标速度缓存保留 |
| `Idle` | 唤醒并待命，摩擦轮继续按目标运行；拨盘保持位置，已开始的单发或清堵继续完成 |
| `Once` | 命令从其它值变成 Once 时请求一发，持续保持或重复推送 Once 不会重复发射 |
| `Repeat` | 准备完成、摩擦轮到速且拨盘目标速度大于零时持续连发，条件恢复后可继续 |

唤醒后会先进入 Init：摩擦轮加速，拨盘复位或对齐。两者准备完成后才进入 Running，允许正常拨弹。
**用于唤醒的 Once 不会打一发**，Init、拨盘忙碌或摩擦轮未到速时收到的 Once 也不会排队、补发。

要再打一发，需要让控制周期先读到 Idle，再读到 Once。命令读取的是最新缓存，不能把连续调用 `Idle → Once` 当成两个已经执行的控制周期。

连发时可以直接更新 `FricTargetSpeeds` 和 `FeedTargetSpeed`。`FeedTargetSpeed = 0` 会暂停新增连发，但不禁止位置环单发。
Idle、未到速或摩擦轮清堵期间，Shooter 会向 Feed 下发内部的 Hold 指令：不开始新的拨弹，已开始的单发或拨盘清堵仍会完成。
连发停止时还会向前对齐弹位，可能继续拨出一发，应用层的热量和弹量判断需要计入这个动作。

需要卸力时下发 Sleep。Sleep 也要经过一次 Tick 才会提交零力矩，因此应先完成这次更新，再考虑停止外部周期源。

### 4. 统一单位和方向

- 摩擦轮目标使用非负的 rpm 大小，由每个轮子的 `Direction` 处理安装方向；反馈保留电机原始正负号。
- 拨盘目标速度使用非负的输出轴角速度 °/s；复位速度可以为负，表示退弹方向。速度目标和相对位移按拨盘 `Direction` 换算。
- 拨盘反馈角度使用输出轴单圈角度 `[0, 360)`；防窥速度直接使用电机原始数据方向。
- 绝对弹位表与拨盘反馈使用同一零点和坐标。
- 控制周期和动作超时使用 ms；力矩输出使用电机原始极性下的数据。

输出轴是拨盘轴的角度， 如果读取的是电机轴数据，要先按减速比换算到输出轴。
拨盘轴的速度单位是`°/s`。

## 二、模块分为哪几层

```text
应用层：决定命令和目标速度，处理热量、弹量、弹速自适应
                         ↓
Module：对外端口、最新输入更新、周期接线和力矩输出
                         ↓
Shooter：触发唤醒、摩擦轮和拨盘总体控制
                         ↓
Fric：单个摩擦轮的速度控制    Feed：拨盘控制、复位、退弹和重载
                         ↓
pid.hpp：适配算法库 PID 的参数、周期、死区和限幅
```

设备层负责采集反馈、检测和解除堵转，并执行力矩指令，通过 Module 的反馈和输出端口与控制逻辑连接。
应用层只使用 Module 的公开接口；内部控制类不负责线程或驱动通信。

## 三、具体实现了什么

### 周期控制和唤醒

每个 Tick 依次读取最新命令、目标和电机反馈，更新摩擦轮，决定拨盘本周期的指令，最后计算并推送力矩。
整机只保留 Sleep、Init、Running 三个阶段，具体的单发、连发和拨盘清堵状态由 Feed 自己维护。
Sleep 的优先级最高，会终止复位、单发、连发及清堵。

### 摩擦轮控速和清堵

每个 Fric 独立执行速度 PID，并根据实际转速与目标的误差判断是否到速。
到速容差取“目标转速 × `SpeedRatio`”与 `MinTolerance` 中较大的值；目标为零或低于 `MinTarget` 时不判定为就绪。
所有摩擦轮就绪后，Shooter 才会放行新的拨弹请求。

堵转由设备层通过 `FricFeedback::Stuck` 提供，不需要传入力矩，也不在发射模块内配置检测阈值或确认时间。
任一轮堵转，Shooter 就让整组以 `FricReverseSpeed` 反转 500ms。期间忽略堵转标志，结束时仍堵转则再反转一轮；解除后恢复应用层最新的逐轮目标速度。

### 拨盘控制和弹位

拨盘使用两种控制方式，位置控制的速度内环与独立速度环分别保存 PID 参数和历史：

| 场景 | 控制方式 |
| --- | --- |
| 待命、单发、退弹、重载 | `PositionPid` 位置外环 → `PositionSpeedPid` 速度内环 → 力矩 |
| 相对模式复位、连发 | `SpeedPid` 独立速度环 → 力矩 |
| Sleep 或刚进入 Stuck | 输出零力矩 |

弹位模式在构造时选择：

| 模式 | 唤醒和单发行为 |
| --- | --- |
| `RelativeFeedPosition` | 每次唤醒按 `ResetSpeed` 复位，堵转或超时后记录位置基准；每发前进一个 `Step` |
| `AbsoluteFeedPosition` | 不做机械复位，唤醒时对齐最近弹位；按应用提供的弹位表依次拨弹，首尾循环 |

相对模式的 Step 大于零且不超过 180°。绝对表按实际拨弹顺序排列，沿拨弹方向的相邻位移也不超过 180°，无需按数值从小到大排序。
绝对表通过 span 引用应用层数组，数组需要一直有效且不被修改。两种模式都处理跨零点运动，连发停止后对齐前方第一个弹位。
单发到位或达到 `ShootTimeOut` 后结束；单发超时会把当前位置作为保持目标。

### 拨盘堵转处理

设备层通过 `FeedFeedback::Stuck` 提供当前堵转状态，也负责在条件解除后清除标志。
复位期间遇到堵转表示复位结束；正常待命、单发、连发或重载期间遇到堵转，则进入退弹流程。

- 相对模式：从卡住的位置后退一个 Step，再回到卡住的位置。
- 绝对模式：退到卡住位置的上一个弹位，再前进到下一个弹位。例如表为 0°、30°、60°……，在 45° 卡住，会退到 30°，再前进到 60°。

退弹不判断堵转，到位或达到 `HandleStuckReverseTimeout` 后进入重载，重载目标始终根据退弹目标计算。
重载到位后保持重载目标；重载超时则保持当前位置。这两种结束方式都会清零本轮重载堵转计数。
重载中再次堵转时计数加一，未达到 `FeedConf.Shoot.RetryMaxCount` 就重新退弹。该参数默认 1，允许配置 1~255 次；首次堵转和退弹阶段不计入次数。
达到上限后，把本次重载再次堵转时的实际角度作为目标保持，不再退弹或开始新的拨弹。
外部 `Stuck` 解除后，清零计数并按最新命令继续；持续的 Repeat 可以恢复连发，保持或等待期间收到的 Once 不会补发。Sleep 也会清零计数。
清堵完成后清除retry计数，按最新发射命令工作：Repeat 仍有效就继续连发，之前的 Once 不会被补发。
