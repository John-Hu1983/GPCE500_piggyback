#ifndef _HAL_TIMERC_H__
#define _HAL_TIMERC_H__

// clang-format off
/*
 * GPCE500A 16-bit TimerC hardware driver on IRQ2 (C_IRQ2_TMC):
 * - SystemClock = 81.92MHz
 * - C_TimerC_SYSCLK_Div8  = 10.24MHz (10.24 ticks/us, max period ~6400us)
 * - C_TimerC_SYSCLK_Div64 = 1.28MHz  (1.28  ticks/us, max period ~51200us)
 */
#define TMRC_MAX_DIV8_US        6000U
#define TMRC_MAX_PERIOD_US      50000U
// clang-format on

typedef void (*TMRC_Handler)(void);

void tmrc_init_module(TMRC_Handler handler);
void tmrc_start_us(u16 period_us);
void tmrc_set_period_us(u16 period_us);
void tmrc_disable_irq(void);
void tmrc_stop_module(void);
void tmrc_service_irq(void);

#endif /* _HAL_TIMERC_H__ */
