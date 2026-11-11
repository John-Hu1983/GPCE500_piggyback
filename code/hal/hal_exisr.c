#include "..\Include\spinmaster.h"

static EXISR_Handler exisr_handlers[EXISR_Ch_Count] = {null, null};

/*
 *@ brief    : Validate one external interrupt channel index.
 *@ type     : private
 *@ argu in  : channel: EXISR_Ch_1 (IOB11) or EXISR_Ch_2 (IOB10).
 *@ argu out : None; halts via bsp_halt_fault if out of range.
 *@ theme    : Guard array and register indexing.
 */
static void _exisr_check_channel(EXISR_Channel channel)
{
    if ((unsigned)channel >= (unsigned)EXISR_Ch_Count)
    {
        bsp_halt_fault(BSP_Fault_EXISR_Ch);
    }
}

/*
 *@ brief    : Initialize one external edge interrupt channel (masked until enabled).
 *@ type     : public
 *@ argu in  : channel: EXISR_Ch_1/2; edge: Rising/Falling; handler: ISR callback.
 *@ argu out : Pin, Schmitt trigger, edge mode and callback configured; IRQ masked.
 *@ theme    : Preserve unrelated P_IO_Ctrl and IRQ5 (SPU) bits.
 */
void exisr_init_channel(EXISR_Channel channel, EXISR_Edge edge, EXISR_Handler handler)
{
    uint8_t pin;
    unsigned irq_bit;
    unsigned ctrl_mask;
    unsigned ctrl_val;
    GPIO_Pattern pattern;

    _exisr_check_channel(channel);
    if (edge != EXISR_Edge_Rising && edge != EXISR_Edge_Falling)
    {
        bsp_halt_fault(BSP_Fault_EXISR_Edge);
    }
    if (handler == null)
    {
        bsp_halt_fault(BSP_Fault_EXISR_Handler);
    }

    if (channel == EXISR_Ch_1)
    {
        pin = 11U;
        irq_bit = C_IRQ5_EXT1;
        ctrl_mask = C_EXT1_Ctrl | C_EXT1_Schmitt_Enable;
        ctrl_val = C_EXT1_Schmitt_Enable |
                   (edge == EXISR_Edge_Rising ? C_EXT1_Normal_Rising : C_EXT1_Normal_Falling);
    }
    else
    {
        pin = 10U;
        irq_bit = C_IRQ5_EXT2;
        ctrl_mask = C_EXT2_Ctrl | C_EXT2_Schmitt_Enable;
        ctrl_val = C_EXT2_Schmitt_Enable |
                   (edge == EXISR_Edge_Rising ? C_EXT2_Normal_Rising : C_EXT2_Normal_Falling);
    }

    /* IOB10/IOB11 only support PullLow or Float in hardware. */
    pattern = (edge == EXISR_Edge_Rising) ? GPIO_In_PullLow : GPIO_In_Float;

    *P_INT_Ctrl &= ~irq_bit;
    *P_FIQ_Sel &= ~irq_bit;
    gpio_init_pattern(GPIOB, pin, pattern);
    *P_IO_Ctrl = (*P_IO_Ctrl & ~ctrl_mask) | ctrl_val;
    exisr_handlers[channel] = handler;
    *P_INT_Status = irq_bit;
}

/*
 *@ brief    : Clear pending flag and unmask one external interrupt channel on IRQ5.
 *@ type     : public
 *@ argu in  : channel: EXISR_Ch_1 or EXISR_Ch_2.
 *@ argu out : Selected EXT channel interrupt enabled.
 *@ theme    : Require a registered callback before unmasking hardware interrupt.
 */
void exisr_enable_channel(EXISR_Channel channel)
{
    unsigned irq_bit;

    _exisr_check_channel(channel);
    if (exisr_handlers[channel] == null)
    {
        bsp_halt_fault(BSP_Fault_EXISR_Handler);
    }

    irq_bit = (channel == EXISR_Ch_1) ? C_IRQ5_EXT1 : C_IRQ5_EXT2;
    *P_FIQ_Sel &= ~irq_bit;
    *P_INT_Status = irq_bit;
    *P_INT_Ctrl |= irq_bit;
}

/*
 *@ brief    : Mask one external interrupt channel and clear its pending flag.
 *@ type     : public
 *@ argu in  : channel: EXISR_Ch_1 or EXISR_Ch_2.
 *@ argu out : Selected EXT channel interrupt disabled.
 *@ theme    : Leave SPU and the other EXT channel untouched.
 */
void exisr_disable_channel(EXISR_Channel channel)
{
    unsigned irq_bit;

    _exisr_check_channel(channel);
    irq_bit = (channel == EXISR_Ch_1) ? C_IRQ5_EXT1 : C_IRQ5_EXT2;
    *P_INT_Ctrl &= ~irq_bit;
    *P_INT_Status = irq_bit;
}

/*
 *@ brief    : Service pending EXT1 / EXT2 interrupts on IRQ5.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Acknowledges enabled pending EXT flags and invokes registered callbacks.
 *@ theme    : Called from _IRQ5 alongside spu_service_irq; never touch C_IRQ5_SPU.
 */
void exisr_service_irq(void)
{
    unsigned flags = *P_INT_Status & *P_INT_Ctrl & ~*P_FIQ_Sel & EXISR_IRQ_MASK;

    if (flags == 0U)
    {
        return;
    }

    if ((flags & C_IRQ5_EXT1) != 0U)
    {
        EXISR_Handler handler = exisr_handlers[EXISR_Ch_1];
        *P_INT_Status = C_IRQ5_EXT1;
        if (handler == null)
        {
            bsp_halt_fault(BSP_Fault_EXISR_Handler);
        }
        handler();
    }

    if ((flags & C_IRQ5_EXT2) != 0U)
    {
        EXISR_Handler handler = exisr_handlers[EXISR_Ch_2];
        *P_INT_Status = C_IRQ5_EXT2;
        if (handler == null)
        {
            bsp_halt_fault(BSP_Fault_EXISR_Handler);
        }
        handler();
    }
}
