#ifndef _BACKEND_SERVICE_H__
#define _BACKEND_SERVICE_H__

// clang-format off
/*
 * Backend service cooperative task period in RTC ticks (at 2048Hz, 512 ticks = 250ms).
 * Samples battery voltage (IOA10) and USB charging status (IOA9), manages charge/low-battery
 * events, and refreshes the Desktop top bar power indicator when Desktop is active.
 */
#define BACKEND_SERVICE_SLICE   512U
// clang-format on

void backend_service_init(void);
void backend_service_stop(void);

#endif /* _BACKEND_SERVICE_H__ */
