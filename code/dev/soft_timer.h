#ifndef SOFT_TIMER_H
#define SOFT_TIMER_H

#include "user_config.h"

// clang-format off
#define SWT_RUN                 0x0001U
#define SWT_REPEAT              0x0002U
#define SWT_DUE                 0x0004U
// clang-format on

typedef enum
{
    SWT_Once = 0,
    SWT_Repeat = 1
} SWT_Mode;

typedef void (*SWT_Handler)(void);

typedef struct SWT_Timer
{
    struct SWT_Timer *next;
    SWT_Handler handler;
    u16 period;
    u16 left;
    u16 stamp;
    u16 flags;
} SWT_Timer;

/*
 * Caller-owned intrusive list; no fixed slot limit, heap or payload buffers.
 * Native RAM: eight 16-bit words per node plus four global words (16N + 8 bytes).
 * Use static/global nodes, or automatic nodes whose lifetime covers registration.
 * start initializes every field of a new node; no prior zero initialization is needed.
 * Do not modify a registered node's fields or let its storage expire.
 * init must run before any start/stop/service_loop call and before enabling ticks;
 * it detaches registered nodes and resets the tick counter.
 * Call swt_service_tick once per RTC interrupt; the ISR only increments one 16-bit
 * counter and never reads the list or calls handlers.
 * Call swt_service_loop in the foreground; callbacks execute there, not in an ISR.
 *
 * Periods are 1..65535 RTC ticks, not milliseconds. At RTC_Freq_2K (2048Hz),
 * 16 ticks = 7.8125ms and 1024 ticks = 500ms. Quantization is up to one tick;
 * foreground work adds callback latency. Keep the RTC frequency fixed while running.
 * Service the foreground at least once every 65535 ticks while timers are active:
 * a full 65536-tick wrap cannot be distinguished from no elapsed time.
 *
 * Starting a node measures from the current tick; a linked node is restarted in place.
 * New nodes are appended in registration order, without an ID or duplicate links.
 * Once timers stop before their callback. Repeat timers retain their original phase;
 * missed periods are coalesced into one callback per service, never replayed in a burst.
 * Stop cancels pending work and detaches the node; stopping an unregistered node is
 * a no-op. During callbacks removal is deferred until service_loop returns, so nodes
 * must remain alive through that return. Once nodes are also detached after dispatch.
 * Callbacks may start/stop any node, including themselves. New/restarted timers do
 * not dispatch in that same pass. Do not reenter service_loop or call init in a callback.
 * All init/start/stop/service_loop calls belong to one foreground context; only
 * service_tick belongs to one non-nested ISR. No interrupt masks are changed.
 * Invalid arguments halt through bsp_halt_fault with an SWT fault code.
 *
 * Example integration:
 *   static SWT_Timer app_timer;
 *   swt_init_module();
 *   // Initialize board pins used by the RTC array before starting RTC.
 *   rtc_init_module(RTC_Freq_2K);
 *   swt_start_timer(&app_timer, 16, SWT_Repeat, app_service_task);
 *   // In the main loop: swt_service_loop();
 *   // To cancel: swt_stop_timer(&app_timer);
 */
void swt_init_module(void);
void swt_start_timer(SWT_Timer *timer, u16 ticks, SWT_Mode mode, SWT_Handler handler);
void swt_stop_timer(SWT_Timer *timer);
void swt_service_tick(void);
void swt_service_loop(void);

#endif /* SOFT_TIMER_H */
