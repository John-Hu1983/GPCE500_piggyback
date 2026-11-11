#include "..\Include\spinmaster.h"

static const RTC_Handler rtc_handler[] = {swt_service_tick};

/*
 *@ brief    : Validate one hardware RTC frequency.
 *@ type     : private
 *@ argu in  : freq: one RTC_Freq value.
 *@ argu out : None.
 *@ theme    : Reject unsupported values and combined interrupt sources.
 */
static void _rtc_check_freq(RTC_Freq freq)
{
    switch (freq)
    {
    case RTC_Freq_2Hz:
    case RTC_Freq_16Hz:
    case RTC_Freq_64Hz:
    case RTC_Freq_512Hz:
    case RTC_Freq_2K:
    case RTC_Freq_4K:
        break;
    default:
        bsp_halt_fault(BSP_Fault_RTC_Freq);
    }
}

/*
 *@ brief    : Select and enable a periodic RTC interrupt.
 *@ type     : public
 *@ argu in  : freq: RTC frequency.
 *@ argu out : None.
 *@ theme    : Configure with interrupts masked; preserve UART/CTS and divider phase.
 */
void rtc_init_module(RTC_Freq freq)
{
    _rtc_check_freq(freq);
    if (bsp_is_phy_init(BSP_phy_RTC))
    {
        return;
    }

    *P_INT2_Ctrl &= ~RTC_IRQ_MASK;
    *P_FIQ2_Sel &= ~RTC_IRQ_MASK;
    *P_INT2_Status = RTC_IRQ_MASK;
    bsp_set_phy_init(BSP_phy_RTC);
    *P_INT2_Ctrl |= (unsigned)freq;
}

/*
 *@ brief    : Disable all RTC periodic interrupt sources.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : None.
 *@ theme    : Configure with interrupts masked; leave UART/CTS and clock running.
 */
void rtc_disable_module(void)
{
    *P_INT2_Ctrl &= ~RTC_IRQ_MASK;
    *P_INT2_Status = RTC_IRQ_MASK;
    bsp_clear_phy_init(BSP_phy_RTC);
}

/*
 *@ brief    : Acknowledge and dispatch an enabled RTC interrupt.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Invokes every table callback once when an RTC tick is pending.
 *@ theme    : Called by IRQ6/IRQ7; never clear UART/CTS or disabled RTC flags.
 */
void rtc_service_irq(void)
{
    unsigned flags = *P_INT2_Status & *P_INT2_Ctrl & ~*P_FIQ2_Sel & RTC_IRQ_MASK;
    u16 i;

    if (flags == 0)
    {
        return;
    }

    if (!bsp_is_phy_init(BSP_phy_RTC))
    {
        bsp_halt_fault(BSP_Fault_RTC_IRQ);
    }
    *P_INT2_Status = flags;
    for (i = 0; i < sizeof(rtc_handler) / sizeof(rtc_handler[0]); ++i)
    {
        RTC_Handler handler = rtc_handler[i];

        if (handler == null)
        {
            bsp_halt_fault(BSP_Fault_RTC_Handler);
        }
        handler();
    }
}