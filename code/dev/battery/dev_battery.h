#ifndef _DEV_BATTERY_H__
#define _DEV_BATTERY_H__

// clang-format off
/*
 * Hardware voltage divider & GPCE500A slope ADC reference parameters:
 * - Both VBAT (IOA10 / ADC_Ch_A10) and VBUS (IOA9 / ADC_Ch_A9) use 47K + 47K (1:2) dividers.
 * - Calibrated internal reference channel ADC_Ch_Ref (C_CMPADC_INN_V12) is 1098mV
 *   (calibrated against multimeter 4.101V reference on actual hardware).
 * - Actual input voltage before the 1:2 divider: V_mv = raw * (1098 * 2) / raw_ref.
 */
#define BAT_ADC_REF_MV          1098U
#define BAT_DIVIDER_NUM         2U
#define BAT_REF_DIVIDER_MV      (BAT_ADC_REF_MV * BAT_DIVIDER_NUM)
#define BAT_DEFAULT_REF_RAW     1442U

/* Li-ion 1S battery voltage thresholds in millivolts (mV) */
#define BAT_EMPTY_MV            3300U
#define BAT_LOW_WARN_MV         3450U
#define BAT_FULL_MV             4200U
#define BAT_CHARGE_FULL_MV      4160U

/* USB VBUS detection thresholds with hysteresis in millivolts (mV) */
#define USB_PLUG_ON_MV          4000U
#define USB_PLUG_OFF_MV         3500U

/* Maximum allowed battery voltage slew per 250ms update (mV) after initial boot */
#define BAT_MAX_SLEW_MV         25U

/* Minimum valid raw count for V12 reference and battery channel */
#define BAT_MIN_VALID_RAW       200U

/* Number of ADC conversions per channel (trimmed mean drops min and max of 4) */
#define BAT_ADC_AVG_SAMPLES     4U
// clang-format on

typedef enum
{
    BAT_Charge_Discharging = 0,
    BAT_Charge_Charging = 1,
    BAT_Charge_Full = 2
} BAT_Charge_State;

typedef struct
{
    u16 bat_mv;             /* Filtered battery voltage in mV (e.g., 3850 = 3.85V) */
    u16 usb_mv;             /* Measured USB VBUS voltage in mV (e.g., 5000 = 5.00V) */
    u8 percent;             /* Estimated state-of-charge percentage (0..100%) */
    bool usb_online;        /* true when USB charger power is present on IOA9 */
    BAT_Charge_State state; /* Current charging/discharging state */
} BAT_Info;

void battery_init_module(void);
void battery_sample_update(void);
const BAT_Info *battery_get_info(void);
u16 battery_get_voltage_mv(void);
u8 battery_get_percent(void);
bool battery_is_charging(void);

#endif /* _DEV_BATTERY_H__ */
