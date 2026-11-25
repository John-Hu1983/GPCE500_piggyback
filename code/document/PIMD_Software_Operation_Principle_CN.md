# 脉冲涡流金属探测 (PIMD) 软件运行原理与架构说明

> English Version: [PIMD_Software_Operation_Principle_EN.md](./PIMD_Software_Operation_Principle_EN.md)
> 软硬件联调踩坑与计算日志：[PIMD_Debugging_Log_CN.md](./PIMD_Debugging_Log_CN.md)
>
> 相关代码文件：
> - 物理采集与算法层：[phy_pimd.h](../application/pimd/phy/phy_pimd.h)、[phy_pimd.c](../application/pimd/phy/phy_pimd.c)
> - 界面交互与调度层：[gui_pimd.h](../application/pimd/gui/gui_pimd.h)、[gui_pimd.c](../application/pimd/gui/gui_pimd.c)
> - 硬件驱动层：[hal_timerc.c](../hal/hal_timerc.c)、[hal_adc.c](../hal/hal_adc.c)、[isr.asm](../main/isr.asm)

---

## 一、物理背景与软硬件协同原理

本系统采用 **脉冲涡流金属探测（Pulse Induction Metal Detection, PIMD）** 原理：

1. **脉冲激励阶段（$0 \sim 80\,\mu\text{s}$）**：
   MCU 将 `P1_METAL_PULSE`（`IOB1`）拉高 $80\,\mu\text{s}$，经 `R102 (10R) -> C101 (1uF) -> [D101 // R101 (10K)] -> Q103 (AO3402A)` 直接驱动探测线圈 `L101 (155uH)` 导通储能。此时反相运放 `U101 (LMV321)` 输出高电平平台。
2. **关断反峰与涡流激发阶段（下降沿后 $0 \sim 300\,\mu\text{s}$）**：
   在 $t = 80\,\mu\text{s}$ 时刻，MCU 将 `IOB1` 陡峭拉低，`Q103` 瞬间关断。线圈磁场急剧坍塌产生高压反峰（由 `R107 = 560Ω` 临界阻尼消振，并由 `D102/D103` 削顶在 $+4.5\text{V} \pm 0.7\text{V}$），把反相运放 `U101` 输出打向谷底：
   - **无金属（Idle）时**：反峰迅速衰减，运放输出谷底仅下探至约 `900~950 mV`，随后平缓回升至静态基线 `~1450 mV`（上升斜率仅约 `40 mV / 100us`）；
   - **有金属靠近时**：金属内部感应出的涡流（Eddy Current）阻碍磁场衰减，产生毫伏级涡流尾迹电压，经 `U101` 约 $3570$ 倍反相放大后，将输出死死压在 **`0 mV` 谷底**，随后在下降沿后约 $450\,\mu\text{s}$ 处开始以 **`> 200 mV / 100us` 的陡峭斜率** 快速爬升并冲高至真实峰值。
3. **斜坡采样阶段（下降沿后 $375 \sim 1500\,\mu\text{s}$）**：
   MCU 在脉冲下降沿等待 $300\,\mu\text{s}$ 后启动 **`75us` 硬件定时触发**（严格等于 $25\,\mu\text{s}$ 背光 PWM 周期的 **3.000 倍整数周期**，且留足 $>57\,\mu\text{s}$ 满量程 $3.2\text{V}$ 斜率充电时间），在下降沿后 **$375\,\mu\text{s}$ 至 $1500\,\mu\text{s}$** 期间于 `P1_METAL_ADC`（`IOA11`）上连续采集 **16 个点**（第 0 点 `375us` 锁定起跳前谷底 `Flr`，第 1 点 `450us` 对齐斜坡起跳点 $T_0$，第 `1~15` 点覆盖完整 $1050\,\mu\text{s}$ 斜坡与 $3.2\text{V}$ 真实峰值 `Peak`）。

```text
IOA11 电压 (mV)
3200mV |                                                .---*---.  <-- 有金属峰值 (~3200mV)
2350mV |      +--------+                           .---*         *---
1450mV |------+        |                   .---*=====================  静态基线 (~1450mV)
 950mV |               |        .----.====*                           无金属曲线 (平缓, ~40mV/100us)
       |               |        |   /
   0mV |               +--------+--*---------------------------------  有金属曲线 (谷底 0mV, ~450us 起跳)
       +------+--------+--------+--+--------------------+------> 距下降沿 Δt (us)
            -250us    0us     375us 450us             1500us
             |<----->|<------->|<---------------------->|
              250us   300us等待  16点 x 75us (375..1500us)
             脉冲激励  静默等待     硬件 Auto-ADC 窗口
```

---

## 二、软件分层架构

金属探测应用位于 `application/pimd/` 下，严格拆分为 **物理层（`phy`）** 与 **界面交互层（`gui`）**：

| 模块层级 | 文件路径 | 核心职责 |
| :--- | :--- | :--- |
| **GUI 应用层** | [gui_pimd.h](../application/pimd/gui/gui_pimd.h) / [gui_pimd.c](../application/pimd/gui/gui_pimd.c) | 周期定时器调度（`53.7ms` 周期）、时分复用静音控制、按键消息路由、5 个 `GUI_Textbox` 诊断面板无闪烁刷新 |
| **PHY 物理层** | [phy_pimd.h](../application/pimd/phy/phy_pimd.h) / [phy_pimd.c](../application/pimd/phy/phy_pimd.c) | `TimerC` 微秒脉冲状态机、9 步数字信号处理与金属判别状态机 |
| **HAL 硬件层** | [hal_timerc.c](../hal/hal_timerc.c) / [hal_adc.c](../hal/hal_adc.c) | `TimerC`（`81.92MHz / 8 = 10.24MHz`）微秒定时触发、`CMPADC` 自动采样 + `DMA1`/`IRQ3` 硬件搬运与底层寄存器控制 |

---

## 三、微秒级硬件采集状态机与双模式方案

### 1. 硬件采集 4 阶段状态机（`PIMD_Phy_Phase`）

每次调用 `pimd_trigger_pulse()` 时，硬件按以下精确微秒时序推进：

1. **`PIMD_Phy_Pulse_High`（$0 \sim 250\,\mu\text{s}$）**：
   - 将 `P1_METAL_PULSE`（`IOB1`）拉高，同时启动 `TimerC` 定时 `PIMD_PULSE_HIGH_US (250us)`。
2. **`PIMD_Phy_Pre_Adc_Wait`（$250 \sim 400\,\mu\text{s}$）**：
   - $t = 250\,\mu\text{s}$ 时触发 `IRQ2`（`_pimd_timerc_isr`），立即将 `IOB1` 拉低关断线圈激励，并将 `TimerC` 周期重装载为 `PIMD_PRE_ADC_WAIT_US (150us)`，避开高压反峰削顶区。
3. **`PIMD_Phy_Sample_Adc`（$400 \sim 950\,\mu\text{s}$）**：
   - $t = 400\,\mu\text{s}$ 时再次进入 `_pimd_timerc_isr`，调用 `_pimd_start_auto_adc()`：
     - 将 `TimerC` 设为 `50us` 步进，并调用 `tmrc_disable_irq()` **屏蔽 TimerC 的 CPU 中断（IRQ2）**，使 `TimerC` 仅作为纯硬件触发源驱动 `CMPADC`；
     - 调用 `hal_adc` 接口 `adc_start_auto_dma()`（或 `adc_start_auto_irq()`）启动 `CMPADC` 自动模式（`C_CMPADC_Auto_Enable | C_CMPADC_TMC`）：每当 `TimerC` 溢出，硬件自动将 `IOA12 (INP)` 斜率电容放电 $4\,\mu\text{s}$（`C_CMPADC_SH_8us`），再以 $60\,\mu\text{A}$ 恒流源充电计数，直至等于 `IOA11` 输入电压。
4. **`PIMD_Phy_Idle`（$t \ge 950\,\mu\text{s}$）**：
   - 采满 `12` 个样本后，`adc_service_irq()` 自动调用 `adc_stop_auto_sample()`（停止 `DMA1`/`IRQ3`，将 `CMPADC` 切回内部 `V12` 通道待机）并触发完成回调 `_pimd_adc_done_isr()`，进而通过 `_pimd_finish_acquire(true)` 停止 `TimerC` 并置位 `pimd_frame_ready = true`。

### 2. 双模式物理采集方案（`PIMD_PHY_MODE`）

在 [phy_pimd.h](../application/pimd/phy/phy_pimd.h) 中可通过宏 `PIMD_PHY_MODE` 选择调用 [hal_adc.c](../hal/hal_adc.c) 提供的两种硬件采集接口（`hal_adc` 本身不依赖任何 `PIMD_*` 宏，保持完全解耦）：

- **方案 0：`PIMD_PHY_MODE_DMA` $\rightarrow$ `adc_start_auto_dma()`（终极零中断方案，默认启用）**
  - 利用 `TimerC` 硬件触发 `CMPADC`，转换完成后由硬件直接请求 `DMA1`（`C_DMA_Request_ADC`），自动将 `*P_CMPADC_Data` 搬运至 `pimd_raw_samples[0..11]`。
  - **优势**：在整个 $600\,\mu\text{s}$ 的 12 点高频采样期间，**CPU 中断次数为 0**，仅在全部 12 点搬运结束时产生 1 次 `IRQ3_DMA1` 中断（由 `adc_service_irq()` 处理），与 `A1800` 语音播放器的 $62.5\,\mu\text{s}$ 中断完全零冲突。
- **方案 1：`PIMD_PHY_MODE_IRQ` $\rightarrow$ `adc_start_auto_irq()`（次优轻量中断方案）**
  - 同样由 `TimerC` 硬件触发 `CMPADC`，每次单点转换完成触发 `IRQ3_ADC`，`adc_service_irq()` 内仅执行单行寄存器读取 `*P_CMPADC_Data`（耗时 $< 1.5\,\mu\text{s}$），无需软件死等放电与充电。

---

## 四、时分复用与抗干扰软件设计

由于运放 `U101` 交流增益高达 $\sim 3570$ 倍，任何同步发生的数字 SPI 刷屏、喇叭大电流发声或背光 PWM 开关都会对模拟链路造成干扰。软件通过 **3 项核心隔离措施** 确保采样纯净度：

### 1. 单周期内的严格时分复用（[_pimd_cycle_timer_cb](../application/pimd/gui/gui_pimd.c)）

每个测量周期为 `PIMD_CYCLE_TICKS = 110 ticks`（在 `2048Hz` 软件定时器基准下约为 **`53.7 ms`**，即 `18.6 Hz`），周期内严格按时间片串行执行：

```text
|<------------------------------- 单周期 110 ticks (~53.7ms) ------------------------------>|
| 0 ~ 1ms      | 1ms ~ 11ms          | 11ms ~ 23ms (24 ticks) | 23ms ~ 53.7ms (86 ticks)    |
+--------------+---------------------+------------------------+-----------------------------+
| [1] 脉冲+ADC | [2] 算法+LCD刷屏    | [3] 短促"滴"声 (可选)  | [4] 全静默电源/电容恢复期   |
| 喇叭/SPI全关 | 模拟采样已结束      | 仅新检出金属时响 12ms  | 喇叭关闭，等待 C104 稳态恢复|
+--------------+---------------------+------------------------+-----------------------------+
```

1. **`0 ~ 1 ms`（纯模拟窗口）**：
   先强制关闭喇叭通道（`spu_stop_channel(SPU_Channel_1)`），随后调用 `pimd_trigger_pulse()` 并在原地等待约 $950\,\mu\text{s}$ 直至 `pimd_is_phy_busy()` 结束。在此 $950\,\mu\text{s}$ 内，**无 SPI2 通信、无 `GPIOB` 端口竞争、无喇叭电流、后台电池监测任务自动让行**。
2. **`1 ~ 11 ms`（数字计算与 UI 刷新窗口）**：
   模拟采样完成后，立即调用 `pimd_process_frame()` 计算本帧数据，并通过高速窗口流式字符渲染（`gui_draw_str_bg`）更新 LCD 上的状态框与 4 行诊断数据。
3. **`11 ~ 23 ms`（短促提示音窗口）**：
   若本帧刚触发金属检测（`beep_pending == true`），启动 `SPU_Channel_1` 播放 `24 ticks (≈12ms)` 的短促提示音。
4. **`23 ~ 53.7 ms`（长静默恢复窗口）**：
   `_pimd_beep_off_timer_cb()` 在第 `23 ms` 准时关闭喇叭，在下一轮脉冲到来前留出整整 **`86 ticks (≈42ms)` 的完全静默期**，让 `+4V5` 电源轨与耦合电容 `C104 (1uF)` 彻底平息喇叭引起的锯齿波动。

### 2. 背光 PWM 倍频与同相共模抵消（[st7735p3.c](../dev/lcd/st7735p3.c)）

- 将 LCD 背光 `IOA8 (LCD_BL_PWM_CH)` 的时钟源从默认的 `IOPWM_Clk_Sys64`（`5 kHz`，周期 $200\,\mu\text{s}$）提升为 **`IOPWM_Clk_Sys8`（`40 kHz`，周期 $25.000\,\mu\text{s}$）**。
- `IOPWM_Clk_Sys8` 与 `TimerC` 均源自同一个 `SYSCLK/8 (10.24MHz)` 硬件分频器，且 ADC 采样间隔 $50.000\,\mu\text{s}$ 恰好是 $25.000\,\mu\text{s}$ 背光周期的 **严格 2.000 倍整数周期**。因此 12 个采样点始终落在背光 PWM 的同一相位点上，求差计算斜率时实现 100% 抵消。

---

## 五、`pimd_process_frame()` 9 步信号处理与识别算法

每采集完一帧 12 个点的原始计数值 `pimd_raw_samples[0..11]` 后，`pimd_process_frame()` 按以下 9 个步骤提取特征并判别金属状态：

### 第 1 步：内部基准比率换算（Ratiometric Raw $\rightarrow$ mV）
在校准阶段静态读取芯片内部 `1.5V` 基准通道计数值 `ref_raw`，将 12 个原始斜率计数值换算为毫伏（`mV`）：
$$\text{raw\_mv}[i] = \frac{\text{pimd\_raw\_samples}[i] \times 1500 + \text{ref\_raw}/2}{\text{ref\_raw}}$$
因为信号通道与内部基准通道共用同一颗斜率电容（`IOA12`）和同一个恒流源（$60\,\mu\text{A}$），比率换算可自动抵消电容容差、温度漂移与电源电压波动。

### 第 2 步：3 点 `[1, 2, 1]/4` 空间曲线平滑
对 12 个点的电压序列进行一维零相移高斯平滑，滤除单点 ADC 采样毛刺，同时完好保留斜坡形状：
$$\text{sm\_mv}[i] = \frac{\text{raw\_mv}[i-1] + 2 \times \text{raw\_mv}[i] + \text{raw\_mv}[i+1] + 2}{4}$$

### 第 3 步：定位谷底 `Flr`、峰值 `Pk` 与上升起始时刻 `T0`
遍历平滑后的 12 个点（对应 $t = 400, 450, \dots, 950\,\mu\text{s}$）：
- **谷底电压 `v_floor_mv` (`Flr`)**：窗口内的最小值 $v_{\min}$；
- **峰值电压 `v_peak_mv` (`Pk`)**：窗口内的最大值 $v_{\max}$（及其索引 `max_idx`）；
- **上升起始时刻 `rise_us` (`T0`)**：曲线首次回升超过 $v_{\min} + 35\,\text{mV}$（`PIMD_RISE_DETECT_MV`）所对应的微秒时刻：
  $$T_0 = 400\,\mu\text{s} + i \times 50\,\mu\text{s}$$

### 第 4 步：计算 $200\,\mu\text{s}$ 跨度最大上升斜率（`raw_slope`）
在到达峰值前的上升段（$i \le \text{max\_idx}$），以 4 个采样步长（即 $4 \times 50\,\mu\text{s} = 200\,\mu\text{s}$ 跨度）滑动计算斜率，并归一化为 **每 $100\,\mu\text{s}$ 上升的毫伏数（$\text{mV}/100\,\mu\text{s}$）**：
$$\text{slp}(i) = \frac{\text{sm\_mv}[i+4] - \text{sm\_mv}[i] + 1}{2}, \quad \text{raw\_slope} = \max_{i} \text{slp}(i)$$
采用 $200\,\mu\text{s}$ 跨度差分代替相邻点 $50\,\mu\text{s}$ 差分，可将斜率信噪比提升 4 倍。

### 第 5 步：阶跃感知自适应 IIR 滤波（`filt_slope` / `filt_peak`）
根据帧间跳变幅度动态切换 IIR 滤波系数：
- **快速跳变态**（当 $|\text{raw\_slope} - \text{filt\_slope}| > 15$ 或 $|\text{v\_max} - \text{filt\_peak}| > 25$）：采用 **`1/2` 权重**，确保金属靠近或移开时在 $1 \sim 2$ 帧（$< 0.1\,\text{s}$）内迅速响应；
- **平稳静置态**：采用 **`3/4` 历史 + `1/4` 当前** 权重，压制静止状态下的读数跳动。

### 第 6 步：两阶段自动基线校准（`CAL...`）
点击 **Start** 后：
1. **预热期（前 `PIMD_WARMUP_PULSES = 12` 帧，$\approx 0.65\,\text{s}$）**：只发脉冲和更新滤波器，不累加基准，让耦合电容 `C104 (1uF)` 先达到动态充放电平衡；
2. **校准期（第 `13 ~ 20` 帧，共 `PIMD_CAL_PULSES = 8` 帧，$\approx 0.45\,\text{s}$）**：对 8 帧的 `filt_slope` 和 `filt_peak` 求算术平均，锁定无金属静态基线 `base_slope` 与 `base_peak`，状态由 `CAL...` 切换为 `NO METAL`。

### 第 7 步：稳态门控基线负向修正（防止单向死锁）
当检测到帧间抖动 `jitter <= 12` 且持续稳定达到 `stable_cnt >= 3` 帧时，若当前滤波值低于基线（`filt < base`），按 `1/8` 步长平缓下调 `base_slope` / `base_peak`。
- 既避免了偶发单帧下陷把基线瞬间拉低导致回升误报死锁，又能保证金属移开后若出现轻微负过冲可在 $0.3\,\text{s}$ 内平滑归零。

### 第 8 步：死区滤波、多特征融合评分与双阈值迟滞判决
1. **计算净增量**：
   $$d_{\text{slope}} = \text{filt\_slope} - \text{base\_slope}, \qquad d_{\text{peak}} = \text{filt\_peak} - \text{base\_peak}$$
2. **扣除噪声死区**（`PIMD_SLOPE_DEADBAND_MV = 18`、`PIMD_PEAK_DEADBAND_MV = 30`）：
   $$\text{eff\_slp} = \max(0,\, d_{\text{slope}} - 18), \qquad \text{eff\_pk} = \max(0,\, d_{\text{peak}} - 30)$$
   在无金属静置时，小于死区的微小波动被直接归零，使静态得分稳定为 **`Sig = 0`**。
3. **融合信号得分（斜率权重 $\times 2$ + 峰值权重 $\times 1$）**：
   $$\text{Sig} = 2 \times \text{eff\_slp} + \text{eff\_pk}$$
4. **双阈值迟滞 + 2 帧消抖判决**：
   - **触发进入（`NO METAL` $\rightarrow$ `METAL !`）**：当 $\text{Sig} \ge 55$（`PIMD_DETECT_ON_TH`）时递增命中计数器 `hit_cnt`；连续 **2 帧**（`Hit: 2/2`）达标后置位 `metal_detected = true` 并触发一次短促“滴”声；
   - **释放退出（`METAL !` $\rightarrow$ `NO METAL`）**：当 $\text{Sig} < 35$（`PIMD_DETECT_OFF_TH`）时立即清除报警状态并将 `hit_cnt` 清零（20 分的迟滞回差可防止临界距离处反复闪跳）。

### 第 9 步：无金属静默期的慢速温漂跟踪
当处于 `NO METAL` 且连续稳定 `stable_cnt >= 4` 帧时，每 4 个脉冲周期允许 `base_slope` / `base_peak` 向 `filt_slope` / `filt_peak` 微调 $+1$，以消除环境温度或电池电压缓慢变化带来的长期漂移，同时绝不会在 `METAL !` 状态下把金属误学习为基线。

---

## 六、LCD 诊断面板参数含义速查表

在 PIMD 页面上方的诊断面板中，实时显示 1 个状态徽章和 4 行核心遥测数据：

| 显示行 | 示例显示内容 | 物理含义说明 |
| :--- | :--- | :--- |
| **顶部状态框** | `STOP` / `CAL...` / `NO METAL` / `METAL !` | 当前运行状态：停止、基准校准中、无金属（绿）、检测到金属（红） |
| **第 1 行 (`dSlp`)** | `dSlp:+145 (185/40)` | **斜率净增量 `d_slope`**（括号内为：`当前滤波斜率 filt_slope / 基准斜率 base_slope`，单位 $\text{mV}/100\,\mu\text{s}$）。有金属时该值显著增大。 |
| **第 2 行 (`dPk`)** | `dPk :+320 (Pk:1780mV)` | **峰值净增量 `d_peak`**（括号内为：当前采样窗口最高电压 `filt_peak`，单位 $\text{mV}$）。 |
| **第 3 行 (`T0/Flr`)** | `T0:500us Flr:12mV` | **`T0`**：斜坡开始明显抬升的时刻（$\mu\text{s}$）；**`Flr`**：采样窗口内的谷底最低电压（$\text{mV}$）。无金属时 `Flr` 较高（约 $800\sim 950\,\text{mV}$），有金属时 `Flr` 被打到接近 $0\,\text{mV}$。 |
| **第 4 行 (`Sig/Hit`)** | `Sig:314/55 Hit:2/2` | **`Sig`**：扣除死区后的综合信号得分 / 触发阈值 `55`；**`Hit`**：连续超阈值命中帧数（满 `2/2` 即判定为有金属）。 |
