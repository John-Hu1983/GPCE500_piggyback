#include "..\Include\spinmaster.h"

static SCH_Task *sch_head = null;
static SCH_Task *sch_cursor = null;
static bool sch_protect = false;

/*
 *@ brief    : Find a task link or the tail link in the scheduler list.
 *@ type     : private
 *@ argu in  : task: caller-owned task node.
 *@ argu out : Pointer to the matching link or the null tail link.
 *@ theme    : Preserve registration order and prevent duplicate links.
 */
static SCH_Task **_sch_find_task(SCH_Task *task)
{
    SCH_Task **link = &sch_head;

    if (task == null)
    {
        bsp_halt_fault(BSP_Fault_SCH_Task);
    }
    while (*link != null && *link != task)
    {
        link = &(*link)->next;
    }
    return link;
}

/*
 *@ brief    : Unlink stopped tasks after a dispatch pass completes.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Stopped nodes detached and round-robin cursor kept valid.
 *@ theme    : Defer removal during callbacks so traversal links remain intact.
 */
static void _sch_remove_stopped(void)
{
    SCH_Task **link = &sch_head;

    while (*link != null)
    {
        SCH_Task *task = *link;

        if ((task->flags & SCH_RUN) != 0)
        {
            link = &task->next;
        }
        else
        {
            if (sch_cursor == task)
            {
                sch_cursor = task->next;
            }
            *link = task->next;
            task->next = null;
            task->handler = null;
            task->flags = 0;
        }
    }
    if (sch_cursor == null)
    {
        sch_cursor = sch_head;
    }
}

/*
 *@ brief    : Dispatch ready tasks in a range until the time-slice quota is used.
 *@ type     : private
 *@ argu in  : begin: inclusive start node; end: exclusive stop node; start_tick: pass start tick.
 *@ argu out : True if the time-slice quota was exhausted; false otherwise.
 *@ theme    : Advance sch_cursor after each task to guarantee round-robin fairness.
 */
static bool _sch_dispatch_range(SCH_Task *begin, SCH_Task *end, u16 start_tick)
{
    SCH_Task *task;

    for (task = begin; task != null && task != end; task = task->next)
    {
        SCH_Handler handler;

        if ((task->flags & (SCH_RUN | SCH_SUSPEND | SCH_READY)) != (SCH_RUN | SCH_READY))
        {
            continue;
        }
        handler = task->handler;
        if (handler == null)
        {
            bsp_halt_fault(BSP_Fault_SCH_State);
        }
        task->flags &= ~SCH_READY;
        sch_cursor = (task->next != null) ? task->next : sch_head;
        handler();
        if ((u16)(swt_read_tick() - start_tick) >= SCH_QUOTA_TICKS)
        {
            return true;
        }
    }
    return false;
}

/*
 *@ brief    : Initialize the cooperative time-slice scheduler.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : All registered tasks detached and round-robin state reset.
 *@ theme    : Call at startup before registering tasks; never call inside a task.
 */
void sch_init_module(void)
{
    if (sch_protect)
    {
        bsp_halt_fault(BSP_Fault_SCH_State);
    }
    while (sch_head != null)
    {
        sch_stop_task(sch_head);
    }
    sch_cursor = null;
}

/*
 *@ brief    : Start or reconfigure a periodic time-slice task.
 *@ type     : public
 *@ argu in  : task: node; slice: period (1..65535); delay: initial offset; handler: callback.
 *@ argu out : Task linked once and running with the specified time slice.
 *@ theme    : delay==0 waits one full slice; delay>0 staggers the first execution phase.
 */
void sch_start_task(SCH_Task *task, u16 slice, u16 delay, SCH_Handler handler)
{
    SCH_Task **link = _sch_find_task(task);

    if (slice == 0)
    {
        bsp_halt_fault(BSP_Fault_SCH_Ticks);
    }
    if (handler == null)
    {
        bsp_halt_fault(BSP_Fault_SCH_Handler);
    }

    task->handler = handler;
    task->slice = slice;
    task->left = (delay == 0) ? slice : delay;
    task->stamp = swt_read_tick();
    task->flags = SCH_RUN;
    if (*link == null)
    {
        task->next = null;
        *link = task;
        if (sch_cursor == null)
        {
            sch_cursor = task;
        }
    }
}

/*
 *@ brief    : Stop and remove a scheduled task.
 *@ type     : public
 *@ argu in  : task: caller-owned task node.
 *@ argu out : Task canceled; unlinked immediately or after the active dispatch pass.
 *@ theme    : Stopping an unregistered task is a safe no-op.
 */
void sch_stop_task(SCH_Task *task)
{
    SCH_Task **link = _sch_find_task(task);

    if (*link == null)
    {
        return;
    }
    task->flags = 0;
    task->handler = null;
    if (!sch_protect)
    {
        if (sch_cursor == task)
        {
            sch_cursor = (task->next != null) ? task->next : sch_head;
            if (sch_cursor == task)
            {
                sch_cursor = null;
            }
        }
        *link = task->next;
        task->next = null;
    }
}

/*
 *@ brief    : Suspend a registered task without unlinking it.
 *@ type     : public
 *@ argu in  : task: registered task node.
 *@ argu out : Task paused and any pending ready state cleared.
 *@ theme    : Preserve slice configuration and list position while suspended.
 */
void sch_suspend_task(SCH_Task *task)
{
    SCH_Task **link = _sch_find_task(task);

    if (*link == null || (task->flags & SCH_RUN) == 0)
    {
        bsp_halt_fault(BSP_Fault_SCH_State);
    }
    task->flags = (task->flags & ~SCH_READY) | SCH_SUSPEND;
}

/*
 *@ brief    : Resume a suspended task from the current tick.
 *@ type     : public
 *@ argu in  : task: registered task node.
 *@ argu out : Task resumes counting down from the current timebase snapshot.
 *@ theme    : Do not accumulate ticks that elapsed while the task was suspended.
 */
void sch_resume_task(SCH_Task *task)
{
    SCH_Task **link = _sch_find_task(task);

    if (*link == null || (task->flags & SCH_RUN) == 0)
    {
        bsp_halt_fault(BSP_Fault_SCH_State);
    }
    if ((task->flags & SCH_SUSPEND) != 0)
    {
        task->flags &= ~SCH_SUSPEND;
        task->stamp = swt_read_tick();
    }
}

/*
 *@ brief    : Delay a running task's next execution by a given number of ticks.
 *@ type     : public
 *@ argu in  : task: registered task node; ticks: nonzero delay from now.
 *@ argu out : Pending ready state cleared and countdown reset to ticks.
 *@ theme    : Allow tasks to yield for a custom interval without changing their slice.
 */
void sch_delay_task(SCH_Task *task, u16 ticks)
{
    SCH_Task **link = _sch_find_task(task);

    if (ticks == 0)
    {
        bsp_halt_fault(BSP_Fault_SCH_Ticks);
    }
    if (*link == null || (task->flags & SCH_RUN) == 0)
    {
        bsp_halt_fault(BSP_Fault_SCH_State);
    }
    task->left = ticks;
    task->stamp = swt_read_tick();
    task->flags &= ~SCH_READY;
}

/*
 *@ brief    : Update task slices and dispatch ready tasks within the time-slice quota.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Ready tasks run in round-robin order until complete or quota expires.
 *@ theme    : Foreground only; coalesce missed slices and yield when quota is reached.
 */
void sch_service_loop(void)
{
    u16 now = swt_read_tick();
    SCH_Task *task;
    SCH_Task *start;
    bool ready = false;

    if (sch_protect)
    {
        bsp_halt_fault(BSP_Fault_SCH_State);
    }

    for (task = sch_head; task != null; task = task->next)
    {
        u16 elapsed;

        if ((task->flags & (SCH_RUN | SCH_SUSPEND)) != SCH_RUN)
        {
            continue;
        }
        elapsed = (u16)(now - task->stamp);
        if (elapsed > 0)
        {
            task->stamp = now;
            if (elapsed < task->left)
            {
                task->left -= elapsed;
            }
            else
            {
                task->left = task->slice - (elapsed - task->left) % task->slice;
                task->flags |= SCH_READY;
            }
        }
        if ((task->flags & SCH_READY) != 0)
        {
            ready = true;
        }
    }

    if (!ready)
    {
        return;
    }

    sch_protect = true;
    start = (sch_cursor != null) ? sch_cursor : sch_head;
    if (!_sch_dispatch_range(start, null, now))
    {
        (void)_sch_dispatch_range(sch_head, start, now);
    }
    _sch_remove_stopped();
    sch_protect = false;
}
