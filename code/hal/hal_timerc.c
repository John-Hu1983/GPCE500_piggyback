#include "..\Include\spinmaster.h"

static TMRC_Handler tmrc_irq_handler = null;

/*
 *@ brief    : Compute 16-bit TimerC reload value and clock select bits for a period in us.
 *@ type     : private
 *@ argu in  : period_us: target interval in microseconds (1..50000).
 *@ argu out : clk_sel: C_TimerC_SYSCLK_Div8 or C_TimerC_SYSCLK_Div64; returns reload word.
 *@ theme    : Exact integer tick scaling at 81.92MHz (10.24MHz Div8 / 1.28MHz Div64).
 */
static u16 _tmrc_calc_reload(u16 period_us, unsigned *clk_sel)
{
    u32 ticks;

    if (period_us == 0U || period_us > TMRC_MAX_PERIOD_US)
    {
        bsp_halt_fault(BSP_Fault_TMRC_Period);
    }

    if (period_us <= TMRC_MAX_DIV8_US)
    {
        /* 81.92MHz / 8 = 10.24MHz => ticks = period_us * 1024 / 100 */
        ticks = ((u32)period_us * 1024UL + 50UL) / 100UL;
        *clk_sel = C_TimerC_SYSCLK_Div8;
    }
    else
    {
        /* 81.92MHz / 64 = 1.28MHz => ticks = period_us * 128 / 100 */
        ticks = ((u32)period_us * 128UL + 50UL) / 100UL;
        *clk_sel = C_TimerC_SYSCLK_Div64;
    }

    if (ticks == 0UL)
    {
        ticks = 1UL;
    }
    if (ticks > 65535UL)
    {
        ticks = 65535UL;
    }

    return (u16)(65536UL - ticks);
}

/*
 *@ brief    : Initialize TimerC in stopped state and register its IRQ2 callback.
 *@ type     : public
 *@ argu in  : handler: ISR callback invoked on TimerC overflow.
 *@ argu out : TimerC disabled, IRQ2 masked and cleared, callback stored.
 *@ theme    : Preserve TimerA and TimerB settings in P_Timer_Ctrl.
 */
void tmrc_init_module(TMRC_Handler handler)
{
    if (handler == null)
    {
        bsp_halt_fault(BSP_Fault_TMRC_Handler);
    }

    *P_INT_Ctrl &= ~C_IRQ2_TMC;
    *P_FIQ_Sel &= ~C_IRQ2_TMC;
    *P_Timer_Ctrl &= ~C_TimerC_Sel;
    *P_INT_Status = C_IRQ2_TMC;
    tmrc_irq_handler = handler;
}

/*
 *@ brief    : Start TimerC with the specified period in microseconds and enable IRQ2.
 *@ type     : public
 *@ argu in  : period_us: interval in microseconds (1..50000).
 *@ argu out : TimerC reload/counter loaded, pending flag cleared, IRQ2 and TimerC enabled.
 *@ theme    : Microsecond-accurate hardware timing for pulse and ADC control.
 */
void tmrc_start_us(u16 period_us)
{
    unsigned clk_sel = C_TimerC_SYSCLK_Div8;
    u16 reload;

    if (tmrc_irq_handler == null)
    {
        bsp_halt_fault(BSP_Fault_TMRC_Handler);
    }

    reload = _tmrc_calc_reload(period_us, &clk_sel);

    *P_INT_Ctrl &= ~C_IRQ2_TMC;
    *P_FIQ_Sel &= ~C_IRQ2_TMC;
    *P_Timer_Ctrl &= ~C_TimerC_Sel;
    *P_TimerC_Data = reload;
    *P_TimerC_CNTR = reload;
    *P_INT_Status = C_IRQ2_TMC;
    *P_INT_Ctrl |= C_IRQ2_TMC;
    *P_Timer_Ctrl = (*P_Timer_Ctrl & ~C_TimerC_Sel) | clk_sel;
}

/*
 *@ brief    : Update TimerC period in microseconds while running (e.g. inside IRQ2 ISR).
 *@ type     : public
 *@ argu in  : period_us: new interval in microseconds (1..50000).
 *@ argu out : TimerC reload and counter updated immediately for the next interval.
 *@ theme    : Fast phase transition from pulse delay to periodic ADC sampling.
 */
void tmrc_set_period_us(u16 period_us)
{
    unsigned clk_sel = C_TimerC_SYSCLK_Div8;
    u16 reload = _tmrc_calc_reload(period_us, &clk_sel);

    *P_TimerC_Data = reload;
    *P_TimerC_CNTR = reload;
    *P_Timer_Ctrl = (*P_Timer_Ctrl & ~C_TimerC_Sel) | clk_sel;
}

/*
 *@ brief    : Stop TimerC counter and mask IRQ2 interrupt.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : TimerC clock disabled, IRQ2 masked and pending flag cleared.
 *@ theme    : Safe shutdown after a sampling burst or page exit.
 */
void tmrc_stop_module(void)
{
    *P_Timer_Ctrl &= ~C_TimerC_Sel;
    *P_INT_Ctrl &= ~C_IRQ2_TMC;
    *P_INT_Status = C_IRQ2_TMC;
}

/*
 *@ brief    : Acknowledge and dispatch a pending TimerC IRQ2 interrupt.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Clears C_IRQ2_TMC and invokes the registered TMRC_Handler.
 *@ theme    : Called from _IRQ2 in isr.asm.
 */
void tmrc_service_irq(void)
{
    unsigned flags = *P_INT_Status & *P_INT_Ctrl & ~*P_FIQ_Sel & C_IRQ2_TMC;

    if (flags == 0U)
    {
        return;
    }

    *P_INT_Status = C_IRQ2_TMC;
    if (tmrc_irq_handler == null)
    {
        bsp_halt_fault(BSP_Fault_TMRC_IRQ);
    }
    tmrc_irq_handler();
}
