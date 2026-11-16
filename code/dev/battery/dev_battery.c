#include "..\..\Include\spinmaster.h"

static BAT_Info bat_info = {3800U, 0U, 55U, false, BAT_Charge_Discharging};
static bool bat_initialized = false;

/*
 *@ brief    : Read 4 slope-ADC conversions and compute an outlier-rejected trimmed mean.
 *@ type     : private
 *@ argu in  : ch: target ADC_Channel.
 *@ argu out : Trimmed-mean 16-bit raw slope count (drops min and max of 4 samples).
 *@ theme    : Reject single-sample glitches or audio PWM current spikes.
 */
static u16 _battery_read_avg(ADC_Channel ch)
{
    u32 sum = 0UL;
    u16 min_v = 0xFFFFU;
    u16 max_v = 0U;
    u8 valid_cnt = 0U;
    u8 i;

    for (i = 0U; i < BAT_ADC_AVG_SAMPLES; ++i)
    {
        u16 v = (u16)adc_read_channel(ch);
        if (v >= BAT_MIN_VALID_RAW)
        {
            sum += (u32)v;
            if (v < min_v)
            {
                min_v = v;
            }
            if (v > max_v)
            {
                max_v = v;
            }
            valid_cnt++;
        }
    }

    if (valid_cnt == 4U)
    {
        return (u16)((sum - (u32)min_v - (u32)max_v + 1UL) / 2UL);
    }
    if (valid_cnt > 0U)
    {
        return (u16)(sum / (u32)valid_cnt);
    }
    return 0U;
}

/*
 *@ brief    : Convert battery voltage in mV (3300..4200mV) to percentage (0..100%).
 *@ type     : private
 *@ argu in  : mv: battery voltage in millivolts.
 *@ argu out : Clamped percentage 0..100.
 *@ theme    : Linear state-of-charge mapping for 1S Li-ion battery.
 */
static u8 _battery_mv_to_percent(u16 mv)
{
    if (mv <= BAT_EMPTY_MV)
    {
        return 0U;
    }
    if (mv >= BAT_FULL_MV)
    {
        return 100U;
    }
    return (u8)(((u32)(mv - BAT_EMPTY_MV) * 100UL + (u32)((BAT_FULL_MV - BAT_EMPTY_MV) / 2U)) /
                (u32)(BAT_FULL_MV - BAT_EMPTY_MV));
}

/*
 *@ brief    : Sample IOA10 (BAT_DET) and IOA9 (USB_DET) via ADC and update battery state.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : bat_info updated with filtered battery voltage, percentage, and USB charge status.
 *@ theme    : Ratiometric slope-ADC conversion referenced to internal V15 reference channel.
 */
void battery_sample_update(void)
{
    u16 ref_raw;
    u16 bat_raw;
    u16 usb_raw = 0U;
    u16 sample_bat_mv;
    u16 sample_usb_mv = 0U;

    ref_raw = _battery_read_avg(ADC_Ch_Ref);
    bat_raw = _battery_read_avg(BAT_DET_ADC_CH);

    /* Only run slope ADC on IOA9 (USB_DET) when IOA9 pad is above logic low */
    if (gpio_read_level(USB_DET_PIN.port, USB_DET_PIN.pin))
    {
        usb_raw = _battery_read_avg(USB_DET_ADC_CH);
    }

    if (ref_raw >= BAT_MIN_VALID_RAW && bat_raw >= BAT_MIN_VALID_RAW)
    {
        sample_bat_mv = (u16)(((u32)bat_raw * (u32)BAT_REF_DIVIDER_MV + (u32)(ref_raw / 2U)) /
                              (u32)ref_raw);

        if (!bat_initialized)
        {
            bat_info.bat_mv = sample_bat_mv;
            bat_initialized = true;
        }
        else
        {
            /* Slew-rate limiter + 1/8 IIR filter to suppress PWM audio current-pulse ripple */
            u16 target_mv = sample_bat_mv;
            if (target_mv > (u16)(bat_info.bat_mv + BAT_MAX_SLEW_MV))
            {
                target_mv = (u16)(bat_info.bat_mv + BAT_MAX_SLEW_MV);
            }
            else if (bat_info.bat_mv > (u16)(target_mv + BAT_MAX_SLEW_MV))
            {
                target_mv = (u16)(bat_info.bat_mv - BAT_MAX_SLEW_MV);
            }
            bat_info.bat_mv = (u16)(((u32)bat_info.bat_mv * 7UL + (u32)target_mv + 4UL) / 8UL);
        }
    }

    if (ref_raw >= BAT_MIN_VALID_RAW && usb_raw >= BAT_MIN_VALID_RAW)
    {
        sample_usb_mv = (u16)(((u32)usb_raw * (u32)BAT_REF_DIVIDER_MV + (u32)(ref_raw / 2U)) /
                              (u32)ref_raw);
    }
    bat_info.usb_mv = sample_usb_mv;

    /* Hysteresis comparator for USB 5V VBUS presence on IOA9 */
    if (bat_info.usb_online)
    {
        if (sample_usb_mv < USB_PLUG_OFF_MV)
        {
            bat_info.usb_online = false;
        }
    }
    else
    {
        if (sample_usb_mv >= USB_PLUG_ON_MV)
        {
            bat_info.usb_online = true;
        }
    }

    bat_info.percent = _battery_mv_to_percent(bat_info.bat_mv);

    if (!bat_info.usb_online)
    {
        bat_info.state = BAT_Charge_Discharging;
    }
    else if (bat_info.bat_mv >= BAT_CHARGE_FULL_MV)
    {
        bat_info.state = BAT_Charge_Full;
    }
    else
    {
        bat_info.state = BAT_Charge_Charging;
    }
}

/*
 *@ brief    : Initialize ADC and battery/USB detection pins (IOA10 & IOA9) and take first reading.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Slope ADC enabled and initial BAT_Info populated.
 *@ theme    : Call once during startup before entering Desktop UI.
 */
void battery_init_module(void)
{
    volatile u16 settle;

    gpio_init_pattern(USB_DET_PIN.port, USB_DET_PIN.pin, GPIO_In_Float);
    gpio_init_pattern(BAT_DET_PIN.port, BAT_DET_PIN.pin, GPIO_In_Float);

    adc_init_module();
    adc_enable_module();

    /*
     * Allow external 47K/47K + 0.1uF RC filters (C209/C210, tau ~= 2.35ms) to settle
     * to steady-state voltage after releasing the default power-on GPIO pull-down.
     */
    for (settle = 0U; settle < 45000U; ++settle)
    {
    }
    system_clear_watchdog();

    bat_initialized = false;
    battery_sample_update();
}

/*
 *@ brief    : Get a read-only pointer to the latest battery and USB charging status.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Pointer to current BAT_Info structure.
 *@ theme    : Zero-copy status query for UI and backend service.
 */
const BAT_Info *battery_get_info(void) { return &bat_info; }

/*
 *@ brief    : Get the current filtered battery voltage in millivolts (mV).
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Battery voltage in mV (e.g., 3850 for 3.85V).
 *@ theme    : Convenience getter.
 */
u16 battery_get_voltage_mv(void) { return bat_info.bat_mv; }

/*
 *@ brief    : Get the current battery state-of-charge percentage (0..100%).
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Percentage 0..100.
 *@ theme    : Convenience getter.
 */
u8 battery_get_percent(void) { return bat_info.percent; }

/*
 *@ brief    : Check whether external USB charging power is connected.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true if USB VBUS is present on IOA9.
 *@ theme    : Convenience getter for charging indicator icon.
 */
bool battery_is_charging(void) { return bat_info.usb_online; }
