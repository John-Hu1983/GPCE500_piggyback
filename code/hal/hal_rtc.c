#include "hal_rtc.h"

static RTC_Handler volatile rtc_handler = null;

/*
 *@ brief    : Halt on an invalid RTC request.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Does not return.
 *@ theme    : Fail explicitly without storing fault state in RAM.
 */
static void _rtc_halt_fault(void)
{
    for (;;)
    {
    }
}

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
        _rtc_halt_fault();
    }
}

/*
 *@ brief    : Select and enable a periodic RTC interrupt.
 *@ type     : public
 *@ argu in  : freq: RTC frequency; handler: non-null interrupt callback.
 *@ argu out : None.
 *@ theme    : Configure with interrupts masked; preserve UART/CTS and divider phase.
 */
void rtc_init_module(RTC_Freq freq, RTC_Handler handler)
{
    _rtc_check_freq(freq);
    if (handler == null)
    {
        _rtc_halt_fault();
    }

    *P_INT2_Ctrl &= ~RTC_IRQ_MASK;
    *P_FIQ2_Sel &= ~RTC_IRQ_MASK;
    rtc_handler = handler;
    *P_INT2_Status = RTC_IRQ_MASK;
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
    rtc_handler = null;
}

/*
 *@ brief    : Acknowledge and dispatch an enabled RTC interrupt.
 *@ type     : public
 *@ argu in  : mask: RTC_IRQ6_MASK or RTC_IRQ7_MASK, supplied by the IRQ wrapper.
 *@ argu out : Invokes the installed callback once when an RTC tick is pending.
 *@ theme    : Called by IRQ6/IRQ7; never clear UART/CTS or disabled RTC flags.
 */
void rtc_service_irq(unsigned mask)
{
    unsigned flags = *P_INT2_Status & *P_INT2_Ctrl & RTC_IRQ_MASK & mask;
    RTC_Handler handler;

    if (flags == 0)
    {
        return;
    }

    handler = rtc_handler;
    if (handler == null)
    {
        _rtc_halt_fault();
    }
    *P_INT2_Status = flags;
    handler();
}