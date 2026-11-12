#ifndef SCHEDULE_H
#define SCHEDULE_H

/*
 * Maximum RTC ticks a single sch_service_loop() pass may spend executing ready tasks
 * before yielding back to the main loop (at 2048Hz, 2 ticks ~= 0.98ms).
 * Remaining ready tasks are dispatched on the next pass starting from the round-robin cursor.
 */
#define SCH_QUOTA_TICKS 2U

#if SCH_QUOTA_TICKS < 1 || SCH_QUOTA_TICKS > 32767U
#error SCH_QUOTA_TICKS must be 1..32767 RTC ticks
#endif

// clang-format off
#define SCH_RUN                 0x0001U
#define SCH_SUSPEND             0x0002U
#define SCH_READY               0x0004U
#define SCH_TASK_INIT           { null, null, 0, 0, 0, 0 }
// clang-format on

typedef void (*SCH_Handler)(void);

typedef struct SCH_Task
{
    struct SCH_Task *next;
    SCH_Handler handler;
    u16 slice;
    u16 left;
    u16 stamp;
    u16 flags;
} SCH_Task;

/*
 * Lightweight cooperative time-slice scheduler for MCU foreground tasks.
 * Caller-owned intrusive list; no dynamic allocation or per-task stack.
 * Native RAM: 8 words (16 bytes) per task node and 5 global words (10 bytes).
 *
 * Scheduling model:
 * 1. Time-slice period (`slice`): each task becomes ready every `slice` RTC ticks
 *    (1..65535). Use `delay` (0..65535) when starting a task to stagger phases
 *    (`delay == 0` waits one full `slice`; `delay > 0` waits `delay` ticks first).
 * 2. Round-robin quota (`SCH_QUOTA_TICKS`): ready tasks execute in round-robin order
 *    starting after the last dispatched task (`sch_cursor`). Once elapsed ticks in
 *    the current pass reach `SCH_QUOTA_TICKS`, the scheduler yields back to the main
 *    loop so timers and watchdog service are never starved, resuming from `sch_cursor`
 *    on the next pass.
 * 3. Task control: tasks can be stopped, suspended, resumed, or delayed by N ticks
 *    from either the main loop or inside a task callback.
 *
 * Example:
 *   static SCH_Task led_task = SCH_TASK_INIT;
 *   static SCH_Task sensor_task = SCH_TASK_INIT;
 *   sch_init_module();
 *   sch_start_task(&led_task, 1024, 0, app_led_task);       // Every 500ms
 *   sch_start_task(&sensor_task, 20, 5, app_sensor_task);   // Every ~10ms, offset by 5 ticks
 *   // In main while(1):
 *   sch_service_loop();
 */
void sch_init_module(void);
void sch_start_task(SCH_Task *task, u16 slice, u16 delay, SCH_Handler handler);
void sch_stop_task(SCH_Task *task);
void sch_suspend_task(SCH_Task *task);
void sch_resume_task(SCH_Task *task);
void sch_delay_task(SCH_Task *task, u16 ticks);
void sch_service_loop(void);

#endif /* SCHEDULE_H */
