#ifndef _HAL_ADC_H__
#define _HAL_ADC_H__

typedef enum
{
    ADC_Ch_A11 = 0,
    ADC_Ch_A10 = 1,
    ADC_Ch_A9 = 2,
    ADC_Ch_A8 = 3,
    ADC_Ch_A7 = 4,
    ADC_Ch_PGA = 5,
    ADC_Ch_VDD2 = 6,
    ADC_Ch_Ref = 7
} ADC_Channel;

/*
 * GPCE500A slope ADC: INP/IOA12 needs an external capacitor to GND.
 * Defaults: 70uA charging current, SH_SEL=3 (4us at 81.92MHz), software trigger,
 * hardware auto discharge, no timer/DMA. ADC IRQ/FIQ are masked; other sources are preserved.
 * About 1nF gives ~4096
 * counts at 3.3V/81.92MHz; adc_read_channel returns the full 16-bit raw count.
 * Call adc_init after System_Initial; it leaves ADC disabled. Then adc_enable.
 * Initialization floats IOA12; reads float only the selected external input.
 * Caller must release conflicting pin peripherals and configure PGA for ADC_Ch_PGA.
 * Reference voltage needs calibration; no fixed value is assumed.
 * Reads are blocking. Disabled/misconfigured ADC, invalid channel or timeout
 * disables ADC and halts in adc_fail (debugger breakpoint); no fault RAM is stored.
 * Serialize all ADC access, including ISR access. Disable ADC before sleep.
 */
void adc_init(void);
void adc_enable(void);
void adc_disable(void);
unsigned adc_read_channel(ADC_Channel channel);

#endif /* _HAL_ADC_H__ */
