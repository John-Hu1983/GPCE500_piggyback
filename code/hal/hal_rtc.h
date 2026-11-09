#ifndef _HAL_RTC_H_
#define _HAL_RTC_H_

#include "GPCE36_CE5.h"
#include "user_config.h"

#define RTC_IRQ_MASK                                                                               \
    (C_IRQ7_2Hz | C_IRQ7_16Hz | C_IRQ7_64Hz | C_IRQ6_512Hz | C_IRQ6_2048Hz | C_IRQ6_4096Hz)

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
 * init enables exactly one RTC source; callbacks are compiled into the const
 * RTC_Handler array in hal_rtc.c. Append no-argument ISR functions there; array
 * length is inferred automatically. No callback parameter or registration is needed.
 * The current table advances software ticks and runs the BSP GPIOB0 test.
 * Initialize software timers and the board pins before enabling RTC.
 * init is idempotent while active; disable before changing frequency or restarting.
 * init/disable must run with CPU IRQ/FIQ disabled, or in a non-nested ISR,
 * serialized with other users of the shared interrupt registers. They do not
 * change CPU interrupt enables; call system_enable_irq after startup setup.
 *
 * IRQ6 and IRQ7 call the same no-argument handler. Either entry can dispatch
 * the selected RTC source when its flag is pending, including during UART/CTS IRQs.
 * Table functions run in array order in the ISR; keep every function short and
 * non-blocking. Flag business work for the main loop when necessary.
 * UART and CTS enable/status/FIQ bits are preserved. RTC owns all six RTC sources.
 * Only enabled, IRQ-routed pending RTC flags are acknowledged before invoking
 * the callbacks. Each table entry runs once per dispatched flag batch, not once
 * per flag. No pending RTC flag means no callback and no status write.
 * Hardware flags do not count missed ticks; long interrupt masking merges events.
 *
 * The shared RTC divider and system clock/sleep settings are not reset/changed.
 * The first tick after init can arrive before a complete period has elapsed.
 * This is a periodic interrupt driver, not a high-resolution timestamp/calendar.
 * Invalid frequency halts before register/state writes. Pending RTC IRQs without
 * initialization halt with RTC_IRQ; null table entries halt with RTC_Handler.
 *
 * Startup: system_init_module();
 *          swt_init_module();
 *          gpio_init_pattern(GPIOB, 0, GPIO_Out_Low);
 *          rtc_init_module(RTC_Freq_2K);
 *          system_enable_irq();
 */
void rtc_init_module(RTC_Freq freq);
void rtc_disable_module(void);
void rtc_service_irq(void);

#endif /* _HAL_RTC_H_ */
