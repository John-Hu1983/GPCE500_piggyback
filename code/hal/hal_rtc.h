#ifndef _HAL_RTC_H_
#define _HAL_RTC_H_

#include "GPCE36_CE5.h"
#include "user_config.h"

#define RTC_IRQ6_MASK (C_IRQ6_512Hz | C_IRQ6_2048Hz | C_IRQ6_4096Hz)
#define RTC_IRQ7_MASK (C_IRQ7_2Hz | C_IRQ7_16Hz | C_IRQ7_64Hz)
#define RTC_IRQ_MASK (RTC_IRQ6_MASK | RTC_IRQ7_MASK)

typedef enum
{
    RTC_Freq_2Hz = C_IRQ7_2Hz,
    RTC_Freq_16Hz = C_IRQ7_16Hz,
    RTC_Freq_64Hz = C_IRQ7_64Hz,
    RTC_Freq_512Hz = C_IRQ6_512Hz,
    RTC_Freq_2K = C_IRQ6_2048Hz,
    RTC_Freq_4K = C_IRQ6_4096Hz
} RTC_Freq;

typedef void (*RTC_Handler)(void);

/*
 * At a nominal 32.768kHz RTC clock, the six periods are 500ms, 62.5ms,
 * 15.625ms, 1.953125ms, 488.28125us and 244.140625us respectively.
 * Actual timing follows the RTC clock, not the CPU/system clock.
 *
 * init enables exactly one RTC source and installs a required non-null callback.
 * Reinitialize to change frequency/callback or restart after disable.
 * init/disable must run with CPU IRQ/FIQ disabled, or in a non-nested ISR,
 * serialized with other users of the shared interrupt registers. They do not
 * change CPU interrupt enables; call system_enable_irq after startup setup.
 *
 * The callback runs in IRQ6 (512Hz/2K/4K) or IRQ7 (2Hz/16Hz/64Hz).
 * Keep it short/non-blocking: flag work for the main loop when necessary.
 * UART and CTS enable/status/FIQ bits are preserved. RTC owns all six RTC sources.
 * IRQ wrappers pass their RTC mask; only that vector's enabled pending RTC flags
 * are acknowledged before invoking the callback.
 * Hardware flags do not count missed ticks; long interrupt masking merges events.
 *
 * The shared RTC divider and system clock/sleep settings are not reset/changed.
 * The first tick after init can arrive before a complete period has elapsed.
 * This is a periodic interrupt driver, not a high-resolution timestamp/calendar.
 * Invalid frequency/null callback halts before register writes, without fault RAM.
 *
 * Startup: system_init_module();
 *          rtc_init_module(RTC_Freq_2K, app_service_tick);
 *          system_enable_irq();
 */
void rtc_init_module(RTC_Freq freq, RTC_Handler handler);
void rtc_disable_module(void);
void rtc_service_irq(unsigned mask);

#endif /* _HAL_RTC_H_ */
