#include "..\Include\spinmaster.h"

static volatile u16 *adc_auto_buf = null;
static volatile u16 adc_auto_count = 0U;
static volatile u16 adc_auto_idx = 0U;
static ADC_Handler adc_auto_handler = null;
static volatile bool adc_auto_active = false;

/*
 *@ brief    : Validate parameters and select ADC channel before starting TimerC Auto conversion.
 *@ type     : private
 *@ argu in  : channel: ADC_Channel; buf: sample destination; count: sample count; handler: done cb.
 *@ argu out : Auto-capture state initialized and CMPADC channel selected with auto S&H config.
 *@ theme    : Shared hardware setup for DMA and IRQ auto-sampling modes.
 */
static void _adc_prepare_auto_channel(ADC_Channel channel, volatile u16 *buf, u16 count,
                                      ADC_Handler handler)
{
    if ((unsigned)channel > ADC_Ch_Ref || buf == null || count == 0U)
    {
        adc_disable_module();
        bsp_halt_fault(BSP_Fault_ADC_State);
    }

    adc_auto_buf = buf;
    adc_auto_count = count;
    adc_auto_idx = 0U;
    adc_auto_handler = handler;
    adc_auto_active = true;

    *P_CMPADC_Ctrl1 = C_CMPADC_Stop;
    *P_CMPADC_Ctrl0 = ((unsigned)channel << 10) | ADC_CTRL0_BASE | C_CMPADC_Enable;
    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;
}

/*
 *@ brief    : Stop and disable the ADC.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : None.
 *@ theme    : Mask only ADC/DMA1 IRQ/FIQ and clear pending ADC flags.
 */
void adc_disable_module(void)
{
    adc_auto_active = false;
    adc_auto_idx = 0U;
    *P_DMA1_CTRL1 = C_DMA_Stop;
    *P_DMA1_CTRL0 = C_DMA_Disable;
    *P_INT_Ctrl &= ~(C_IRQ3_ADC | C_IRQ3_DMA1);
    *P_FIQ_Sel &= ~(C_IRQ3_ADC | C_IRQ3_DMA1);
    *P_CMPADC_Ctrl1 &= ~C_CMPADC_Start;
    *P_CMPADC_Ctrl0 &= ~C_CMPADC_Enable;
    *P_DMA_INTSTS = ADC_DMA1_FLAGS;
    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC | C_IRQ3_DMA1;
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

/*
 *@ brief    : Convert one selected ADC input inside an ISR without altering CPU IRQ state.
 *@ type     : public
 *@ argu in  : channel: external input, PGA, VDD/2 or reference.
 *@ argu out : Full 16-bit raw slope count (0 if input is 0V or conversion times out).
 *@ theme    : ISR-safe slope conversion for TimerC high-speed sampling.
 */
unsigned adc_read_channel_isr(ADC_Channel channel)
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

    *P_CMPADC_Ctrl0 = (*P_CMPADC_Ctrl0 & ~C_CMPADC_INN_Sel) | ((unsigned)channel << 10) |
                      C_CMPADC_Discharge_Enable;
    *P_CMPADC_Ctrl1 = ADC_MODE;
    for (dly = 0U; dly < 80U; ++dly)
    {
    }

    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;
    *P_CMPADC_Ctrl0 &= ~C_CMPADC_Discharge_Enable;

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

    *P_CMPADC_Ctrl0 |= C_CMPADC_Discharge_Enable;
    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;

    return data;
}

/*
 *@ brief    : Start TimerC-triggered CMPADC auto conversion using DMA1 hardware transfer.
 *@ type     : public
 *@ argu in  : channel: ADC_Channel; buf: sample buffer; count: sample words; handler: done ISR cb.
 *@ argu out : DMA1 and CMPADC Auto-mode armed; 0 CPU interrupts until all `count` samples finish.
 *@ theme    : Zero-CPU-interrupt burst ADC acquisition via TimerC trigger + DMA1.
 */
void adc_start_auto_dma(ADC_Channel channel, volatile u16 *buf, u16 count, ADC_Handler handler)
{
    _adc_prepare_auto_channel(channel, buf, count, handler);

    *P_DMA1_CTRL1 = C_DMA_Stop;
    *P_DMA1_CTRL0 = C_DMA_Disable;
    *P_DMA_INTSTS = ADC_DMA1_FLAGS;
    *P_INT_Status = C_IRQ3_DMA1;

    *P_DMA1_SRCADR = (unsigned)(u32)P_CMPADC_Data;
    *P_DMA1_DSTADR = (unsigned)(u32)buf;
    *P_DMA1_DTN = count;
    *P_DMA1_CTRL1 = C_DMA_Double_Buf_Disable | C_DMA_Stop_Immdly;
    *P_DMA1_CTRL0 = ADC_DMA1_CTRL0;

    *P_FIQ_Sel &= ~C_IRQ3_DMA1;
    *P_INT_Ctrl |= C_IRQ3_DMA1;

    *P_CMPADC_Ctrl1 = ADC_AUTO_DMA_MODE;
}

/*
 *@ brief    : Start TimerC-triggered CMPADC auto conversion using lightweight IRQ3 EOC reads.
 *@ type     : public
 *@ argu in  : channel: ADC_Channel; buf: sample buffer; count: sample words; handler: done ISR cb.
 *@ argu out : IRQ3_ADC and CMPADC Auto-mode armed to capture `count` samples into `buf`.
 *@ theme    : Hardware-triggered slope ADC burst with 1-register read per EOC interrupt.
 */
void adc_start_auto_irq(ADC_Channel channel, volatile u16 *buf, u16 count, ADC_Handler handler)
{
    _adc_prepare_auto_channel(channel, buf, count, handler);

    *P_FIQ_Sel &= ~C_IRQ3_ADC;
    *P_INT_Ctrl |= C_IRQ3_ADC;

    *P_CMPADC_Ctrl1 = ADC_AUTO_IRQ_MODE;
}

/*
 *@ brief    : Stop hardware-triggered ADC auto conversion (DMA1 or IRQ3) and restore manual mode.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : DMA1/IRQ3 disabled and CMPADC parked on internal V12 in manual discharge state.
 *@ theme    : Clean handoff from auto-burst mode back to shared manual ADC state.
 */
void adc_stop_auto_sample(void)
{
    adc_auto_active = false;
    adc_auto_idx = 0U;

    *P_DMA1_CTRL1 = C_DMA_Stop;
    *P_DMA1_CTRL0 = C_DMA_Disable;
    *P_INT_Ctrl &= ~(C_IRQ3_DMA1 | C_IRQ3_ADC);
    *P_DMA_INTSTS = ADC_DMA1_FLAGS;
    *P_INT_Status = C_IRQ3_DMA1 | C_IRQ3_ADC;

    *P_CMPADC_Ctrl1 = ADC_MODE;
    *P_CMPADC_Ctrl0 = C_CMPADC_INN_V12 | ADC_CTRL0_BASE | C_CMPADC_Enable;
    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;
}

/*
 *@ brief    : Service IRQ3 for ADC auto-sampling (DMA1 transfer done or ADC EOC interrupt).
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Completes burst on DMA1 done or collects sample words in IRQ mode, then calls cb.
 *@ theme    : Called from _IRQ3 in isr.asm; exits immediately when auto-sampling is inactive.
 */
void adc_service_irq(void)
{
    unsigned dma_flags;
    unsigned adc_flags;

    if (!adc_auto_active)
    {
        return;
    }

    dma_flags = *P_INT_Status & *P_INT_Ctrl & ~*P_FIQ_Sel & C_IRQ3_DMA1;
    if (dma_flags != 0U)
    {
        if ((*P_DMA_INTSTS & ADC_DMA1_FLAGS) != 0U)
        {
            ADC_Handler handler = adc_auto_handler;
            adc_stop_auto_sample();
            if (handler != null)
            {
                handler();
            }
        }
        else
        {
            *P_INT_Status = C_IRQ3_DMA1;
        }
        return;
    }

    adc_flags = *P_INT_Status & *P_INT_Ctrl & ~*P_FIQ_Sel & C_IRQ3_ADC;
    if (adc_flags != 0U)
    {
        *P_CMPADC_Status = ADC_FLAGS;
        *P_INT_Status = C_IRQ3_ADC;

        if (adc_auto_buf != null && adc_auto_idx < adc_auto_count)
        {
            adc_auto_buf[adc_auto_idx] = (u16)(*P_CMPADC_Data);
            adc_auto_idx++;
        }

        if (adc_auto_idx >= adc_auto_count)
        {
            ADC_Handler handler = adc_auto_handler;
            adc_stop_auto_sample();
            if (handler != null)
            {
                handler();
            }
        }
    }
}
