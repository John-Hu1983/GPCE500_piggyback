#ifndef _HAL_ADC_H__
#define _HAL_ADC_H__

#define ADC_CTRL0_BASE                                                                             \
    (C_CMPADC_Discharge_Enable | C_CMPADC_SH_8us | C_CMPADC_Hysteresis_Enable |                    \
     C_CMPADC_IBIAS_60uA)
#define ADC_MODE (C_CMPADC_Auto_Disable | C_CMPADC_CMPO_None | C_CMPADC_Manual | C_CMPADC_Start)
#define ADC_AUTO_DMA_MODE                                                                          \
    (C_CMPADC_DMA_Enable | C_CMPADC_Auto_Enable | C_CMPADC_CMPO_None | C_CMPADC_TMC |              \
     C_CMPADC_Start)
#define ADC_AUTO_IRQ_MODE                                                                          \
    (C_CMPADC_DMA_Disable | C_CMPADC_Auto_Enable | C_CMPADC_CMPO_None | C_CMPADC_TMC |             \
     C_CMPADC_Start)
#define ADC_FLAGS (C_CMPADC_INT_Flag | C_CMPADC_Timeout_Flag)
#define ADC_DMA1_FLAGS (C_DMA1_TX_Done_Flag | C_DMA1_TX_Error_Flag)
#define ADC_DMA1_CTRL0                                                                             \
    (C_DMA_Request_ADC | C_DMA_Cont_Disable | C_DMA_Burst_Disable | C_DMA_Src_Inc_Disable |        \
     C_DMA_Dest_Inc_Enable | C_DMA_Circular_Disable | C_DMA_Error_INT_Disable |                    \
     C_DMA_Done_INT_Enable | C_DMA_Enable)

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

typedef void (*ADC_Handler)(void);

/*
 * GPCE500A slope ADC: INP/IOA12 needs an external capacitor to GND.
 * Defaults: 60uA charging current, SH_SEL=3 (4us at 81.92MHz), software manual trigger or
 * TimerC hardware auto-trigger (with DMA1 or IRQ3 capture).
 * About 1nF gives ~4096
 * counts at 3.3V/81.92MHz; adc_read_channel returns the full 16-bit raw count.
 * Call adc_init_module after system_init_module; it leaves ADC disabled. Then adc_enable_module.
 * Initialization floats IOA12; reads float only the selected external input.
 * Caller must release conflicting pin peripherals and configure PGA for ADC_Ch_PGA.
 * Reference voltage needs calibration; no fixed value is assumed.
 * Manual reads are blocking. Disabled/misconfigured ADC, invalid channel or timeout
 * disables ADC and calls bsp_halt_fault with an ADC fault code; no global fault RAM is stored.
 * Serialize all ADC access, including ISR/DMA access. Disable ADC before sleep.
 */
void adc_init_module(void);
void adc_enable_module(void);
void adc_disable_module(void);
unsigned adc_read_channel(ADC_Channel channel);
unsigned adc_read_channel_isr(ADC_Channel channel);
void adc_start_auto_dma(ADC_Channel channel, volatile u16 *buf, u16 count, ADC_Handler handler);
void adc_start_auto_irq(ADC_Channel channel, volatile u16 *buf, u16 count, ADC_Handler handler);
void adc_stop_auto_sample(void);
void adc_service_irq(void);

#endif /* _HAL_ADC_H__ */
