#ifndef _TASK_LED_H__
#define _TASK_LED_H__

#ifndef LED_PORT
#define LED_PORT GPIOB
#endif
#ifndef LED_PIN
#define LED_PIN 2U
#endif
#ifndef LED_ACTIVE_LEVEL
#define LED_ACTIVE_LEVEL 1
#endif

/*
 * Toggle half-periods in RTC ticks (at RTC_Freq_2K = 2048Hz):
 * - 0.5Hz (2.0s period -> toggle every 1.0s):  2048 ticks
 * - 1Hz   (1.0s period -> toggle every 0.5s):  1024 ticks
 * - 2Hz   (0.5s period -> toggle every 0.25s):  512 ticks
 */
#ifndef LED_SLICE_0_5HZ
#define LED_SLICE_0_5HZ 2048U
#endif
#ifndef LED_SLICE_1HZ
#define LED_SLICE_1HZ 1024U
#endif
#ifndef LED_SLICE_2HZ
#define LED_SLICE_2HZ 512U
#endif

#if LED_ACTIVE_LEVEL != 0 && LED_ACTIVE_LEVEL != 1
#error LED_ACTIVE_LEVEL must be 0 or 1
#endif
#if LED_SLICE_0_5HZ < 1 || LED_SLICE_1HZ < 1 || LED_SLICE_2HZ < 1
#error LED blink slices must be >= 1 RTC tick
#endif

typedef enum
{
    LED_Mode_Off = 0,
    LED_Mode_On,
    LED_Mode_0_5Hz,
    LED_Mode_1Hz,
    LED_Mode_2Hz
} LED_Mode;

/*
 * Scheduled LED task on GPIOB2 (configurable via LED_PORT / LED_PIN / LED_ACTIVE_LEVEL):
 * - LED_Mode_Off / LED_Mode_On drive the output statically and stop the scheduler task.
 * - LED_Mode_0_5Hz / LED_Mode_1Hz / LED_Mode_2Hz drive the LED ON immediately and
 *   schedule a repeating toggle task at the corresponding half-period.
 *
 * Example:
 *   led_init_task();
 *   led_set_mode(LED_Mode_1Hz);
 */
void led_init_task(void);
void led_set_mode(LED_Mode mode);
LED_Mode led_get_mode(void);

#endif /* _TASK_LED_H__ */
