# Pulse Induction Metal Detector (PIMD) Software Operation Principle & Architecture

> 中文版文档：[PIMD_Software_Operation_Principle_CN.md](./PIMD_Software_Operation_Principle_CN.md)
>
> Related Source Files:
> - Physical Acquisition & DSP Layer: [phy_pimd.h](../application/pimd/phy/phy_pimd.h), [phy_pimd.c](../application/pimd/phy/phy_pimd.c)
> - GUI & Cycle Scheduling Layer: [gui_pimd.h](../application/pimd/gui/gui_pimd.h), [gui_pimd.c](../application/pimd/gui/gui_pimd.c)
> - Hardware Abstraction Layer: [hal_timerc.c](../hal/hal_timerc.c), [hal_adc.c](../hal/hal_adc.c), [isr.asm](../main/isr.asm)

---

## 1. Physical Principle & Hardware-Software Co-Design

This system implements a **Pulse Induction Metal Detector (PIMD)** by combining microsecond-accurate coil excitation with high-gain eddy-current tail sampling:

1. **Excitation Phase ($0 \sim 80\,\mu\text{s}$)**:
   The MCU drives `P1_METAL_PULSE` (`IOB1`) HIGH for $80\,\mu\text{s}$, directly driving `Q103 (AO3402A)` via `R102 (10R) -> C101 (1uF) -> [D101 // R101 (10K)]` to build magnetic energy in the search coil `L101 (155uH)`. During this phase, the inverting amplifier `U101 (LMV321)` outputs a high plateau (`~2350 mV` at `IOA11`).
2. **Flyback Decay & Eddy-Current Induction Phase ($0 \sim 300\,\mu\text{s}$ after falling edge)**:
   At $t = 80\,\mu\text{s}$, the MCU pulls `IOB1` sharply LOW, snapping `Q103` OFF. The collapsing magnetic field generates a flyback pulse (critically damped by `R107 = 560Ω` and clipped within $+4.5\text{V} \pm 0.7\text{V}$ by `D102/D103`), driving the inverting op-amp output downward:
   - **No Metal (Idle)**: The coil energy decays rapidly; the op-amp output dips only to **`~900–950 mV`** and gently recovers to the `~1450 mV` baseline with a shallow slope (`~40 mV / 100us`).
   - **With Metal Present**: Eddy currents induced in the metal target oppose the magnetic field collapse and generate a millivolt-level decay tail. Amplified by `U101` ($\text{gain} \approx -3570\times$), this drives the output all the way down to **`0 mV`**, followed by a **steep rising slope (`> 200 mV / 100us`)** starting at $\Delta t \approx 450\,\mu\text{s}$ after the falling edge and climbing to the true peak.
3. **ADC Ramp Sampling Window ($375 \sim 1500\,\mu\text{s}$ after falling edge)**:
   After a $300\,\mu\text{s}$ post-falling-edge wait, hardware `TimerC` triggers the slope ADC on `P1_METAL_ADC` (`IOA11`) every **$75\,\mu\text{s}$** (exact $3\times$ of the $25\,\mu\text{s}$ `40 kHz` backlight PWM period for synchronous ripple cancellation, while providing $>57\,\mu\text{s}$ per sample for full-scale $3.2\text{V}$ slope conversion) at $\Delta t = 375, 450, 525, \dots, 1500\,\mu\text{s}$ to capture **16 samples**, spanning the valley floor (`Flr` at $375\,\mu\text{s}$), rising-edge onset time (`T0` at $450\,\mu\text{s}$), maximum rising slope (`Slope` over a 3-step $225\,\mu\text{s}$ span), and full-scale peak voltage (`Peak`).

```text
IOA11 Voltage (mV)
3200mV |                                                .---*---.  <-- Metal Peak (~3200mV)
2350mV |      +--------+                           .---*         *---
1450mV |------+        |                   .---*=====================  Steady Baseline (~1450mV)
 950mV |               |        .----.====*                           Idle Curve (Shallow, ~40mV/100us)
       |               |        |   /
   0mV |               +--------+--*---------------------------------  Metal Curve (0mV floor, ~450us onset)
       +------+--------+--------+--+--------------------+------> Time Δt from falling edge (us)
            -250us    0us     375us 450us             1500us
             |<----->|<------->|<---------------------->|
              250us   300us Wait  16 pts x 75us (375..1500us)
             Pulse ON  Wait       Hardware Auto-ADC Window
```

---

## 2. Software Layered Architecture

The PIMD module under `application/pimd/` is cleanly separated into a **Physical Layer (`phy`)** and a **GUI Presentation & Scheduling Layer (`gui`)**:

| Layer | Files | Key Responsibilities |
| :--- | :--- | :--- |
| **GUI Layer** | [gui_pimd.h](../application/pimd/gui/gui_pimd.h) / [gui_pimd.c](../application/pimd/gui/gui_pimd.c) | Periodic cycle timer (`53.7ms`), time-division audio/SPI isolation, key input handling, and flicker-free `GUI_Textbox` telemetry rendering |
| **PHY Layer** | [phy_pimd.h](../application/pimd/phy/phy_pimd.h) / [phy_pimd.c](../application/pimd/phy/phy_pimd.c) | `TimerC` microsecond pulse state machine and 9-step DSP metal detection algorithm |
| **HAL Layer** | [hal_timerc.c](../hal/hal_timerc.c) / [hal_adc.c](../hal/hal_adc.c) | `TimerC` (`81.92MHz / 8 = 10.24MHz`) microsecond timer driver and `CMPADC` Auto-mode + `DMA1`/`IRQ3` hardware capture control |

---

## 3. Microsecond Acquisition State Machine & Dual-Mode PHY

### 3.1 Four-Phase Hardware State Machine (`PIMD_Phy_Phase`)

Each call to `pimd_trigger_pulse()` advances through four hardware-timed phases:

1. **`PIMD_Phy_Pulse_High` ($0 \sim 250\,\mu\text{s}$)**:
   - Drives `P1_METAL_PULSE` (`IOB1`) HIGH and starts `TimerC` for `PIMD_PULSE_HIGH_US (250us)`.
2. **`PIMD_Phy_Pre_Adc_Wait` ($250 \sim 400\,\mu\text{s}$)**:
   - At $t = 250\,\mu\text{s}$, the `TimerC` ISR (`_pimd_timerc_isr` on `IRQ2`) pulls `IOB1` LOW and reloads `TimerC` with `PIMD_PRE_ADC_WAIT_US (150us)` to wait out the initial flyback clipping transient.
3. **`PIMD_Phy_Sample_Adc` ($400 \sim 950\,\mu\text{s}$)**:
   - At $t = 400\,\mu\text{s}$, `_pimd_timerc_isr` invokes `_pimd_start_auto_adc()`:
     - Reconfigures `TimerC` to a `50us` period and calls `tmrc_disable_irq()` to **mask CPU `IRQ2` interrupts**, turning `TimerC` into a pure hardware trigger source;
     - Calls HAL API `adc_start_auto_dma()` (or `adc_start_auto_irq()`) to start `CMPADC` in Auto Mode (`C_CMPADC_Auto_Enable | C_CMPADC_TMC`): on every `50us` `TimerC` overflow, the hardware automatically discharges the slope capacitor on `IOA12 (INP)` for $4\,\mu\text{s}$ (`C_CMPADC_SH_8us`) and ramps the $60\,\mu\text{A}$ current source until it matches `IOA11`.
4. **`PIMD_Phy_Idle` ($t \ge 950\,\mu\text{s}$)**:
   - Once all 12 samples are captured, `adc_service_irq()` automatically calls `adc_stop_auto_sample()` (stopping `DMA1`/`IRQ3` and parking `CMPADC` on the internal `V12` reference channel) and invokes `_pimd_adc_done_isr()`, which calls `_pimd_finish_acquire(true)` to stop `TimerC` and set `pimd_frame_ready = true`.

### 3.2 Dual Acquisition Schemes (`PIMD_PHY_MODE`)

Configured in [phy_pimd.h](../application/pimd/phy/phy_pimd.h) to select between the two hardware capture APIs provided by [hal_adc.c](../hal/hal_adc.c) (`hal_adc` itself has zero dependency on any `PIMD_*` macros):

- **Scheme 0: `PIMD_PHY_MODE_DMA` $\rightarrow$ `adc_start_auto_dma()` (Ultimate Zero-CPU-IRQ Scheme, Default)**
  - `TimerC` triggers `CMPADC` in hardware, and each completed conversion triggers `DMA1` (`C_DMA_Request_ADC`) to transfer `*P_CMPADC_Data` into `pimd_raw_samples[0..11]`.
  - **Advantage**: Generates **0 CPU interrupts** across the entire $600\,\mu\text{s}$ 12-sample burst (only 1 `IRQ3_DMA1` interrupt handled by `adc_service_irq()` at the end of all 12 samples), completely eliminating any ISR contention with the $62.5\,\mu\text{s}$ `A1800` audio playback interrupt.
- **Scheme 1: `PIMD_PHY_MODE_IRQ` $\rightarrow$ `adc_start_auto_irq()` (Lightweight EOC Interrupt Scheme)**
  - `TimerC` triggers `CMPADC` in hardware, and `adc_service_irq()` on `IRQ3_ADC` executes a single register read `*P_CMPADC_Data` ($< 1.5\,\mu\text{s}$ ISR execution time) per sample.

---

## 4. Time-Division Multiplexing & Anti-Interference Design

Because `U101` operates at an AC gain of $\sim 3570\times$, concurrent SPI display traffic, speaker current spikes, or low-frequency backlight PWM switching would perturb the analog signal. The software eliminates cross-talk through three mechanisms:

### 4.1 Strict Single-Cycle Time-Division Multiplexing ([_pimd_cycle_timer_cb](../application/pimd/gui/gui_pimd.c))

Each measurement cycle lasts `PIMD_CYCLE_TICKS = 110 ticks` (**`~53.7 ms`** at `2048 Hz` software timer rate, or `18.6 Hz`) and is partitioned into four non-overlapping time slots:

```text
|<------------------------------ Single Cycle: 110 ticks (~53.7ms) ---------------------------->|
| 0 ~ 1ms      | 1ms ~ 11ms          | 11ms ~ 23ms (24 ticks) | 23ms ~ 53.7ms (86 ticks)        |
+--------------+---------------------+------------------------+---------------------------------+
| [1] Pulse+ADC| [2] DSP + LCD Draw  | [3] Short Beep (Opt.)  | [4] Quiet Rail Settling Window  |
| SPK/SPI Off  | Analog Burst Done   | 12ms tone on new hit   | Speaker Off, C104/Rail Settles  |
+--------------+---------------------+------------------------+---------------------------------+
```

1. **`0 ~ 1 ms` (Quiet Analog Window)**:
   Silences `SPU_Channel_1`, triggers the $950\,\mu\text{s}$ hardware pulse + Auto-ADC burst, and waits for `pimd_is_phy_busy()` to clear. During this $950\,\mu\text{s}$ window, **SPI2 is idle, `GPIOB` LCD pins (`CS`/`A0`) do not toggle, the speaker is off, and background battery ADC polling yields**.
2. **`1 ~ 11 ms` (DSP & UI Update Window)**:
   Immediately processes the freshly captured 12-sample frame via `pimd_process_frame()` and updates the LCD textboxes using high-speed window-streamed glyph rendering (`gui_draw_str_bg`).
3. **`11 ~ 23 ms` (Short Detection Beep Window)**:
   If a new metal hit occurred (`beep_pending == true`), plays a crisp `24-tick (~12ms)` tone at `TONE_VOL_MID`.
4. **`23 ~ 53.7 ms` (Quiet Power-Rail Settling Window)**:
   `_pimd_beep_off_timer_cb()` stops the speaker at `23 ms`, leaving **`86 ticks (~42ms)` of complete audio silence** so `+4V5` and `C104 (1uF)` fully settle before the next excitation pulse.

### 4.2 Backlight PWM Frequency Shift & Synchronous Phase Cancellation ([st7735p3.c](../dev/lcd/st7735p3.c))

- The LCD backlight PWM (`IOA8`) clock is configured to **`IOPWM_Clk_Sys8` (`40 kHz`, period $25.000\,\mu\text{s}$)** instead of the default `5 kHz`.
- Because both `IOPWM_Clk_Sys8` and `TimerC` divide the exact same `SYSCLK/8 (10.24MHz)` clock, the $50.000\,\mu\text{s}$ ADC sample interval equals **exactly 2.000 backlight PWM periods**. Every ADC sample in the 12-point burst is taken at the identical phase of the `40 kHz` wave, canceling any residual ripple to zero during differential slope calculation.

---

## 5. Nine-Step DSP & Metal Detection Pipeline (`pimd_process_frame()`)

Once `pimd_raw_samples[0..11]` is acquired, `pimd_process_frame()` executes a 9-step pipeline:

### Step 1: Ratiometric Millivolt Conversion
Converts raw slope counts to millivolts (`mV`) using the internal `1.5V` reference count (`ref_raw`) measured during silent calibration:
$$\text{raw\_mv}[i] = \frac{\text{pimd\_raw\_samples}[i] \times 1500 + \text{ref\_raw}/2}{\text{ref\_raw}}$$
Because both `IOA11` and the internal reference use the same `IOA12` ramp capacitor and $60\,\mu\text{A}$ current source, ratiometric scaling cancels component tolerance, temperature drift, and supply variations.

### Step 2: 3-Point `[1, 2, 1]/4` Curve Smoothing
Applies a zero-phase 3-point binomial filter along the 12-sample curve to reject single-sample ADC glitches while preserving ramp slope:
$$\text{sm\_mv}[i] = \frac{\text{raw\_mv}[i-1] + 2 \times \text{raw\_mv}[i] + \text{raw\_mv}[i+1] + 2}{4}$$

### Step 3: Valley Floor (`Flr`), Peak (`Pk`), and Onset Time (`T0`) Extraction
Scans the smoothed 12-sample array ($t = 400, 450, \dots, 950\,\mu\text{s}$):
- **`v_floor_mv` (`Flr`)**: Minimum voltage $v_{\min}$ in the window;
- **`v_peak_mv` (`Pk`)**: Maximum voltage $v_{\max}$ (and its index `max_idx`);
- **`rise_us` (`T0`)**: Timestamp ($\mu\text{s}$) of the first sample that rises above $v_{\min} + 35\,\text{mV}$ (`PIMD_RISE_DETECT_MV`).

### Step 4: $200\,\mu\text{s}$-Span Maximum Rising Slope (`raw_slope`)
Evaluates the rising slope across a 4-step ($4 \times 50\,\mu\text{s} = 200\,\mu\text{s}$) sliding span up to `max_idx`, normalized to **$\text{mV} / 100\,\mu\text{s}$**:
$$\text{slp}(i) = \frac{\text{sm\_mv}[i+4] - \text{sm\_mv}[i] + 1}{2}, \quad \text{raw\_slope} = \max_{i} \text{slp}(i)$$
Using a $200\,\mu\text{s}$ span improves slope SNR by $4\times$ compared to single-step ($50\,\mu\text{s}$) differences.

### Step 5: Step-Aware Adaptive IIR Filter (`filt_slope` / `filt_peak`)
Adapts its IIR smoothing factor based on frame-to-frame delta:
- **Fast Transition** ($|\text{raw\_slope} - \text{filt\_slope}| > 15$ or $|\text{v\_max} - \text{filt\_peak}| > 25$): Uses **`1/2` weight** for sub-100ms response when approaching or removing metal;
- **Steady State**: Uses **`3/4` history + `1/4` new** weight to keep idle readouts rock-solid.

### Step 6: Two-Stage Startup Calibration (`CAL...`)
Upon pressing **Start**:
1. **Warmup Phase (`PIMD_WARMUP_PULSES = 12` frames, $\approx 0.65\,\text{s}$)**: Pulses the coil and updates the IIR filters without accumulating baseline sums, allowing coupling capacitor `C104 (1uF)` to reach dynamic charge equilibrium;
2. **Calibration Phase (`PIMD_CAL_PULSES = 8` frames, $\approx 0.45\,\text{s}$)**: Averages 8 frames of `filt_slope` and `filt_peak` to lock `base_slope` and `base_peak`, transitioning from `CAL...` to `NO METAL`.

### Step 7: Stability-Gated Negative Baseline Tracking (Deadlock Prevention)
When frame-to-frame `jitter <= 12` for at least `stable_cnt >= 3` consecutive frames and `filt < base`, smoothly steps `base_slope` / `base_peak` downward by `1/8` of the difference. This prevents single-frame dips from ratcheting the baseline down and causing a false-positive deadlock upon recovery.

### Step 8: Deadband Filtering, Score Fusion, and Hysteresis Debounce
1. **Baseline Deltas**:
   $$d_{\text{slope}} = \text{filt\_slope} - \text{base\_slope}, \qquad d_{\text{peak}} = \text{filt\_peak} - \text{base\_peak}$$
2. **Noise Deadband Subtraction** (`PIMD_SLOPE_DEADBAND_MV = 18`, `PIMD_PEAK_DEADBAND_MV = 30`):
   $$\text{eff\_slp} = \max(0,\, d_{\text{slope}} - 18), \qquad \text{eff\_pk} = \max(0,\, d_{\text{peak}} - 30)$$
3. **Combined Signal Score**:
   $$\text{Sig} = 2 \times \text{eff\_slp} + \text{eff\_pk}$$
4. **Hysteresis & 2-Frame Debounce**:
   - **Enter (`NO METAL` $\rightarrow$ `METAL !`)**: When $\text{Sig} \ge 55$ (`PIMD_DETECT_ON_TH`) for **2 consecutive frames** (`Hit: 2/2`), sets `metal_detected = true` and queues a single rising-edge beep;
   - **Exit (`METAL !` $\rightarrow$ `NO METAL`)**: Clears `metal_detected` and resets `hit_cnt = 0` as soon as $\text{Sig} < 35$ (`PIMD_DETECT_OFF_TH`).

### Step 9: Slow Upward Thermal Drift Tracking
While in `NO METAL` and stationary (`stable_cnt >= 4`), nudges `base_slope` / `base_peak` upward by $+1$ every 4 pulses if `filt > base`, tracking slow ambient temperature or battery drift without ever learning a metal target as baseline.

---

## 6. LCD Diagnostic Telemetry Panel Reference

| Display Row | Example Readout | Description |
| :--- | :--- | :--- |
| **Status Badge** | `STOP` / `CAL...` / `NO METAL` / `METAL !` | Current detector state: Stopped, Calibrating baseline, Idle (green), or Metal Detected (red) |
| **Row 0 (`dSlp`)** | `dSlp:+145 (185/40)` | **Net slope delta `d_slope`** (`filt_slope / base_slope` in $\text{mV}/100\,\mu\text{s}$). Increases sharply when metal approaches. |
| **Row 1 (`dPk`)** | `dPk :+320 (Pk:1780mV)` | **Net peak delta `d_peak`** (and current filtered peak voltage `filt_peak` in $\text{mV}$). |
| **Row 2 (`T0/Flr`)** | `T0:500us Flr:12mV` | **`T0`**: Rising-edge onset time ($\mu\text{s}$); **`Flr`**: Valley floor voltage ($\text{mV}$), which drops from $\sim 900\,\text{mV}$ at idle to $\sim 0\,\text{mV}$ with metal. |
| **Row 3 (`Sig/Hit`)** | `Sig:314/55 Hit:2/2` | **`Sig`**: Deadband-filtered signal score vs. threshold `55`; **`Hit`**: Consecutive hit counter (`0..2`). |
