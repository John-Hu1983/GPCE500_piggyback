#ifndef _HAL_EXISR_H__
#define _HAL_EXISR_H__

/*
 * GPCE500A external edge interrupts on IRQ5:
 * - EXISR_Ch_1 (EXT1): IOB11, C_IRQ5_EXT1
 * - EXISR_Ch_2 (EXT2): IOB10, C_IRQ5_EXT2
 *
 * Notice: IOB8..IOB15 do NOT support internal pull-high on GPCE500A (only IOB0..IOB7 do).
 * Initialization configures the pin as GPIO_In_PullLow (rising edge) or GPIO_In_Float (falling edge),
 * enables the hardware Schmitt trigger (C_EXT1/2_Schmitt_Enable), selects normal IO edge trigger,
 * clears pending IRQ5_EXTx status, and leaves the channel interrupt masked until exisr_enable_channel.
 */
#define EXISR_IRQ_MASK (C_IRQ5_EXT1 | C_IRQ5_EXT2)

typedef enum
{
    EXISR_Ch_1 = 0,
    EXISR_Ch_2 = 1,
    EXISR_Ch_Count = 2
} EXISR_Channel;

typedef enum
{
    EXISR_Edge_Rising = 0,
    EXISR_Edge_Falling = 1
} EXISR_Edge;

typedef void (*EXISR_Handler)(void);

void exisr_init_channel(EXISR_Channel channel, EXISR_Edge edge, EXISR_Handler handler);
void exisr_enable_channel(EXISR_Channel channel);
void exisr_disable_channel(EXISR_Channel channel);
void exisr_service_irq(void);

#endif /* _HAL_EXISR_H__ */
