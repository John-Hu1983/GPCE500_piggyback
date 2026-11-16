#include "..\Include\spinmaster.h"

/*
 *@ brief    : Stop and disable the ADC.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : None.
 *@ theme    : Mask only ADC IRQ/FIQ and clear pending ADC flags.
 */
void adc_disable_module(void)
{
    *P_INT_Ctrl &= ~C_IRQ3_ADC;
    *P_FIQ_Sel &= ~C_IRQ3_ADC;
    *P_CMPADC_Ctrl1 &= ~C_CMPADC_Start;
    *P_CMPADC_Ctrl0 &= ~C_CMPADC_Enable;
    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;
}

/*
 *@ brief    : Initialize software-triggered slope ADC.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : None.
 *@ theme    : Float INP and set manual-mode discharge defaults; leave ADC disabled.
 */
void adc_init_module(void)
{
    adc_disable_module();
    gpio_init_pattern(GPIOA, 12, GPIO_In_Float);
    *P_CMPADC_Ctrl0 = ADC_CTRL0_BASE;
    *P_CMPADC_Ctrl1 = ADC_MODE;
    *P_CMPADC_Status = ADC_FLAGS;
}

/*
 *@ brief    : Enable the initialized ADC.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : None.
 *@ theme    : Enable comparator with discharge held active until conversion starts.
 */
void adc_enable_module(void)
{
    *P_CMPADC_Ctrl0 |= C_CMPADC_Discharge_Enable | C_CMPADC_Enable;
    *P_CMPADC_Ctrl1 = ADC_MODE;
    *P_CMPADC_Status = ADC_FLAGS;
}

/*
 *@ brief    : Convert one selected ADC input with atomic interrupt protection.
 *@ type     : public
 *@ argu in  : channel: external input, PGA, VDD/2 or reference.
 *@ argu out : Full 16-bit raw slope count (0 if input is 0V or conversion times out).
 *@ theme    : Atomic manual slope conversion via C_CMPADC_Discharge_Enable release.
 */
unsigned adc_read_channel(ADC_Channel channel)
{
    unsigned status;
    unsigned data = 0U;
    unsigned timeout = 512U;
    volatile unsigned dly;

    if ((unsigned)channel > ADC_Ch_Ref || (*P_CMPADC_Ctrl0 & C_CMPADC_Enable) == 0)
    {
        adc_disable_module();
        bsp_halt_fault(BSP_Fault_ADC_State);
    }
    if (channel <= ADC_Ch_A7)
    {
        gpio_init_pattern(GPIOA, (uint8_t)(11 - channel), GPIO_In_Float);
    }

    /* Lock CPU interrupts during the ~25us discharge + slope ramp critical window */
    System_DisableInterrupts();

    /* 1. Select channel while holding Discharge_Enable to discharge INP (IOA12) capacitor */
    *P_CMPADC_Ctrl0 = (*P_CMPADC_Ctrl0 & ~C_CMPADC_INN_Sel) | ((unsigned)channel << 10) |
                      C_CMPADC_Discharge_Enable;
    *P_CMPADC_Ctrl1 = ADC_MODE;
    for (dly = 0U; dly < 80U; ++dly)
    {
    }

    /* 2. Clear flags and release Discharge_Enable to start ramp charging & counter */
    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;
    *P_CMPADC_Ctrl0 &= ~C_CMPADC_Discharge_Enable;

    /* 3. Wait exclusively for C_CMPADC_INT_Flag latch (never break early on raw STS bit) */
    for (;;)
    {
        status = *P_CMPADC_Status;
        if ((status & C_CMPADC_INT_Flag) != 0U)
        {
            data = *P_CMPADC_Data;
            break;
        }
        if ((status & C_CMPADC_Timeout_Flag) != 0U || --timeout == 0U)
        {
            data = 0U;
            break;
        }
    }

    /* 4. Re-assert Discharge_Enable immediately and restore CPU IRQ */
    *P_CMPADC_Ctrl0 |= C_CMPADC_Discharge_Enable;
    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;

    System_CPU_EnableIRQ();
    return data;
}
