#include "hal_adc.h"
#include "hal_gpio.h"
#include "GPCE36_CE5.h"

#define ADC_MODE (C_CMPADC_Auto_Enable | C_CMPADC_Manual)
#define ADC_FLAGS (C_CMPADC_INT_Flag | C_CMPADC_Timeout_Flag)

void adc_disable(void)
{
    *P_INT_Ctrl &= ~C_IRQ3_ADC;
    *P_FIQ_Sel &= ~C_IRQ3_ADC;
    *P_CMPADC_Ctrl1 &= ~C_CMPADC_Start;
    *P_CMPADC_Ctrl0 &= ~C_CMPADC_Enable;
    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;
}

static void adc_fail(void)
{
    adc_disable();
    for (;;)
    {
    }
}

void adc_init(void)
{
    adc_disable();
    gpio_init_pattern(GPIOA, 12, GPIO_In_Float);
    /* SH_SEL=3 is 4us at 81.92MHz; the SDK's SH_*us names use another clock. */
    *P_CMPADC_Ctrl0 = C_CMPADC_IBIAS_70uA | C_CMPADC_SH_Sel;
    *P_CMPADC_Ctrl1 = ADC_MODE;
}

void adc_enable(void)
{
    *P_CMPADC_Ctrl0 |= C_CMPADC_Enable;
}

unsigned adc_read_channel(ADC_Channel channel)
{
    unsigned status;
    unsigned timeout = 0xFFFFU;

    if ((unsigned)channel > ADC_Ch_Ref ||
        (*P_CMPADC_Ctrl0 & C_CMPADC_Enable) == 0 ||
        *P_CMPADC_Ctrl1 != ADC_MODE)
    {
        adc_fail();
    }
    if (channel <= ADC_Ch_A7)
    {
        gpio_init_pattern(GPIOA, (uint8_t)(11 - channel), GPIO_In_Float);
    }
    *P_CMPADC_Ctrl0 = (*P_CMPADC_Ctrl0 & ~C_CMPADC_INN_Sel) |
                      ((unsigned)channel << 10);
    /* Flags are W1C. STS is the comparator level, not a conversion busy flag. */
    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;
    *P_CMPADC_Ctrl1 = ADC_MODE | C_CMPADC_Start;

    for (;;)
    {
        status = *P_CMPADC_Status;
        if ((status & C_CMPADC_Timeout_Flag) != 0)
        {
            adc_fail();
        }
        if ((status & C_CMPADC_INT_Flag) != 0)
        {
            break;
        }
        if (--timeout == 0)
        {
            adc_fail();
        }
    }
    *P_CMPADC_Ctrl1 = ADC_MODE;
    status = *P_CMPADC_Data;
    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;
    return status;
}
