#include "soft_timer.h"
#include "..\bsp\bsp_global.h"

static volatile u16 swt_tick = 0;
static SWT_Timer *swt_head = null;
static bool swt_busy = false;

/*
 *@ brief    : Find the link to a software timer or the end of the list.
 *@ type     : private
 *@ argu in  : timer: caller-owned node.
 *@ argu out : Pointer to the matching link or the null tail link.
 *@ theme    : Do not read an unregistered node's uninitialized fields.
 */
static SWT_Timer **_swt_find_timer(SWT_Timer *timer)
{
    SWT_Timer **link = &swt_head;

    if (timer == null)
    {
        bsp_halt_fault(BSP_Fault_SWT_Node);
    }
    while (*link != null && *link != timer)
    {
        link = &(*link)->next;
    }
    return link;
}

/*
 *@ brief    : Unlink stopped timers after callback dispatch.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Only running nodes remain registered.
 *@ theme    : Defer removal so callbacks cannot break the current traversal.
 */
static void _swt_remove_stopped(void)
{
    SWT_Timer **link = &swt_head;

    while (*link != null)
    {
        SWT_Timer *timer = *link;

        if ((timer->flags & SWT_RUN) != 0)
        {
            link = &timer->next;
        }
        else
        {
            *link = timer->next;
            timer->next = null;
            timer->handler = null;
            timer->flags = 0;
        }
    }
}

/*
 *@ brief    : Initialize all software timers.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : All registered nodes detached and the tick counter reset.
 *@ theme    : Run before enabling RTC ticks; allocate no dynamic memory.
 */
void swt_init_module(void)
{
    if (swt_busy)
    {
        bsp_halt_fault(BSP_Fault_SWT_State);
    }
    while (swt_head != null)
    {
        swt_stop_timer(swt_head);
    }
    swt_tick = 0;
}

/*
 *@ brief    : Start or replace one software timer.
 *@ type     : public
 *@ argu in  : timer: node; ticks: nonzero period; mode: once/repeat; handler: callback.
 *@ argu out : The node is running and linked once, in registration order.
 *@ theme    : Initialize new nodes fully; restart linked nodes without moving them.
 */
void swt_start_timer(SWT_Timer *timer, u16 ticks, SWT_Mode mode, SWT_Handler handler)
{
    SWT_Timer **link = _swt_find_timer(timer);

    if (ticks == 0)
    {
        bsp_halt_fault(BSP_Fault_SWT_Ticks);
    }
    if (mode != SWT_Once && mode != SWT_Repeat)
    {
        bsp_halt_fault(BSP_Fault_SWT_Mode);
    }
    if (handler == null)
    {
        bsp_halt_fault(BSP_Fault_SWT_Handler);
    }

    timer->handler = handler;
    timer->period = ticks;
    timer->left = ticks;
    timer->stamp = swt_tick;
    timer->flags = SWT_RUN;
    if (mode == SWT_Repeat)
    {
        timer->flags |= SWT_REPEAT;
    }
    if (*link == null)
    {
        timer->next = null;
        *link = timer;
    }
}

/*
 *@ brief    : Stop one software timer.
 *@ type     : public
 *@ argu in  : timer: caller-owned node.
 *@ argu out : Running and pending work canceled; an unregistered node is unchanged.
 *@ theme    : Unlink now, or after dispatch when called by a timer callback.
 */
void swt_stop_timer(SWT_Timer *timer)
{
    SWT_Timer **link = _swt_find_timer(timer);

    if (*link == null)
    {
        return;
    }
    timer->flags = 0;
    timer->handler = null;
    if (!swt_busy)
    {
        *link = timer->next;
        timer->next = null;
    }
}

/*
 *@ brief    : Advance the software timebase by one tick.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : The 16-bit tick counter advances with natural wraparound.
 *@ theme    : RTC ISR only; do not scan timers or execute callbacks here.
 */
void swt_service_tick(void) { ++swt_tick; }

/*
 *@ brief    : Update timers and dispatch due callbacks.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Each due node invokes its callback at most once.
 *@ theme    : Foreground only; preserve repeat phase and allow callback start/stop.
 */
void swt_service_loop(void)
{
    u16 now = swt_tick;
    SWT_Timer *timer;
    bool due = false;

    if (swt_busy)
    {
        bsp_halt_fault(BSP_Fault_SWT_State);
    }

    /* Age all nodes before callbacks can start or replace another node. */
    for (timer = swt_head; timer != null; timer = timer->next)
    {
        u16 elapsed;

        if ((timer->flags & SWT_RUN) == 0)
        {
            continue;
        }
        elapsed = (u16)(now - timer->stamp);
        if (elapsed == 0)
        {
            continue;
        }
        timer->stamp = now;
        if (elapsed < timer->left)
        {
            timer->left -= elapsed;
            continue;
        }

        if ((timer->flags & SWT_REPEAT) != 0)
        {
            timer->left = timer->period - (elapsed - timer->left) % timer->period;
        }
        else
        {
            timer->flags &= ~SWT_RUN;
        }
        timer->flags |= SWT_DUE;
        due = true;
    }

    if (!due)
    {
        return;
    }

    swt_busy = true;
    for (timer = swt_head; timer != null; timer = timer->next)
    {
        SWT_Handler handler;

        if ((timer->flags & SWT_DUE) == 0)
        {
            continue;
        }
        handler = timer->handler;
        if (handler == null)
        {
            bsp_halt_fault(BSP_Fault_SWT_State);
        }
        timer->flags &= ~SWT_DUE;
        if ((timer->flags & SWT_RUN) == 0)
        {
            timer->handler = null;
        }
        handler();
    }
    _swt_remove_stopped();
    swt_busy = false;
}
