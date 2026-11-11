#include "..\Include\spinmaster.h"

/*
 *@ brief    : Get a validated DMA register block.
 *@ type     : private
 *@ argu in  : channel: DMA1 or DMA2.
 *@ argu out : Register pointer.
 *@ theme    : Registers are word addressed; reserved address words are not written.
 */
static Serial_DMA *_serial_get_dma(DMA_Channel channel)
{
    if (channel == DMA_Ch_1)
    {
        return (Serial_DMA *)P_DMA1_CTRL0;
    }
    if (channel == DMA_Ch_2)
    {
        return (Serial_DMA *)P_DMA2_CTRL0;
    }
    bsp_halt_fault(BSP_Fault_DMA_Ch);
    return null;
}

/*
 *@ brief    : Read the nominal current system clock.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Clock frequency in Hz.
 *@ theme    : Decode clock selection without a RAM lookup table.
 */
unsigned long serial_read_clock(void)
{
    unsigned div;
    switch (*P_System_Clock & C_SYSCLK_Sel)
    {
    case C_SYSCLK_81920KHz:
        div = 1;
        break;
    case C_SYSCLK_81920KHz_DIV2:
        div = 2;
        break;
    case C_SYSCLK_81920KHz_DIV3:
        div = 3;
        break;
    case C_SYSCLK_81920KHz_DIV4:
        div = 4;
        break;
    case C_SYSCLK_81920KHz_DIV6:
        div = 6;
        break;
    case C_SYSCLK_81920KHz_DIV8:
        div = 8;
        break;
    case C_SYSCLK_81920KHz_DIV12:
        div = 12;
        break;
    default:
        div = 16;
        break;
    }
    return SERIAL_OSC_HZ / div;
}

/*
 *@ brief    : Validate a transmit word-per-byte buffer.
 *@ type     : public
 *@ argu in  : buf: source; count: nonzero byte count.
 *@ argu out : Serial_Ok or Serial_Err_Arg.
 *@ theme    : Reject values above 255 before starting transmission.
 */
Serial_Result serial_check_bytes(const unsigned *buf, unsigned count)
{
    unsigned i;
    if (buf == null || count == 0)
    {
        return Serial_Err_Arg;
    }
    for (i = 0; i < count; ++i)
    {
        if (buf[i] > SERIAL_BYTE_MASK)
        {
            return Serial_Err_Arg;
        }
    }
    return Serial_Ok;
}

/*
 *@ brief    : Check whether a DMA channel is available.
 *@ type     : public
 *@ argu in  : channel: DMA1 or DMA2.
 *@ argu out : Serial_Ok, Serial_Busy or Serial_Err_Arg.
 *@ theme    : Do not steal a channel enabled by any other peripheral.
 */
Serial_Result serial_check_dma(DMA_Channel channel)
{
    Serial_DMA *dma;
    if (channel != DMA_Ch_1 && channel != DMA_Ch_2)
    {
        return Serial_Err_Arg;
    }
    dma = _serial_get_dma(channel);
    if ((dma->ctrl0 & C_DMA_Enable) != 0 || (dma->ctrl1 & C_DMA_Busy_Flag) != 0)
    {
        return Serial_Busy;
    }
    return Serial_Ok;
}

/*
 *@ brief    : Validate a DMA working-RAM buffer.
 *@ type     : public
 *@ argu in  : buf: word buffer; count: nonzero word count.
 *@ argu out : Serial_Ok or Serial_Err_Arg.
 *@ theme    : Prevent pointer truncation, overflow and cache/Flash/MMIO access.
 */
Serial_Result serial_check_memory(const unsigned *buf, unsigned count)
{
    if (buf == null || count == 0 || SERIAL_ADDR(buf) >= SERIAL_RAM_END ||
        count > SERIAL_RAM_END - SERIAL_ADDR(buf))
    {
        return Serial_Err_Arg;
    }
    return Serial_Ok;
}

/*
 *@ brief    : Configure an idle peripheral-request DMA channel.
 *@ type     : public
 *@ argu in  : channel/request: hardware source; src/dst: addresses; count: words; inc: flags.
 *@ argu out : Channel is enabled and its error/done IRQ is unmasked.
 *@ theme    : Set count and addresses before enable; never software-trigger a peripheral transfer.
 */
void serial_start_dma(DMA_Channel channel, unsigned request, const unsigned *src, unsigned *dst,
                      unsigned count, unsigned inc)
{
    Serial_DMA *dma = _serial_get_dma(channel);
    unsigned bit = 1U << (channel - 1);
    unsigned irq = channel == DMA_Ch_1 ? C_IRQ3_DMA1 : C_IRQ3_DMA2;
    if (serial_check_dma(channel) != Serial_Ok || count == 0 ||
        (request != C_DMA_Request_UART && request != C_DMA_Request_SPI2) ||
        (inc & ~(C_DMA_Src_Inc_Enable | C_DMA_Dest_Inc_Enable)) != 0 ||
        SERIAL_ADDR(src) > 0xFFFFUL || SERIAL_ADDR(dst) > 0xFFFFUL)
    {
        bsp_halt_fault(BSP_Fault_DMA_Start);
    }
    dma->ctrl0 = 0;
    dma->ctrl1 = 0;
    dma->count = count;
    dma->src = (unsigned)SERIAL_ADDR(src);
    dma->dst = (unsigned)SERIAL_ADDR(dst);
    *P_DMA_INTSTS = bit | (bit << 8);
    *P_INT_Status = irq;
    *P_FIQ_Sel &= ~irq;
    *P_INT_Ctrl |= irq;
    dma->ctrl0 = request | inc | C_DMA_Error_INT_Enable | C_DMA_Done_INT_Enable | C_DMA_Enable;
    system_enable_irq();
}

/*
 *@ brief    : Read a channel's latched DMA result.
 *@ type     : public
 *@ argu in  : channel: DMA1 or DMA2.
 *@ argu out : Busy, Done or DMA error; error takes priority.
 *@ theme    : W1C flags are cleared only when stopping the owned channel.
 */
Serial_Result serial_read_dma(DMA_Channel channel)
{
    unsigned bit;
    unsigned irq;
    unsigned status;
    if (channel != DMA_Ch_1 && channel != DMA_Ch_2)
    {
        return Serial_Err_Arg;
    }
    irq = channel == DMA_Ch_1 ? C_IRQ3_DMA1 : C_IRQ3_DMA2;
    if ((*P_INT_Ctrl & irq) == 0)
    {
        return Serial_Busy;
    }
    bit = 1U << (channel - 1);
    status = *P_DMA_INTSTS;
    if ((status & (bit << 8)) != 0)
    {
        return Serial_Err_DMA;
    }
    return (status & bit) != 0 ? Serial_Done : Serial_Busy;
}

/*
 *@ brief    : Stop and release an owned DMA channel.
 *@ type     : public
 *@ argu in  : channel: DMA1 or DMA2.
 *@ argu out : Channel is idle with only its flags/IRQ cleared.
 *@ theme    : Wait for stop before allowing buffer reuse.
 */
void serial_stop_dma(DMA_Channel channel)
{
    Serial_DMA *dma = _serial_get_dma(channel);
    unsigned bit = 1U << (channel - 1);
    unsigned irq = channel == DMA_Ch_1 ? C_IRQ3_DMA1 : C_IRQ3_DMA2;
    unsigned long wait = SERIAL_WAIT_LIMIT;
    dma->ctrl1 = C_DMA_Stop;
    while ((dma->ctrl1 & C_DMA_Busy_Flag) != 0)
    {
        if (--wait == 0)
        {
            bsp_halt_fault(BSP_Fault_DMA_Stop);
        }
    }
    dma->ctrl0 = 0;
    *P_INT_Ctrl &= ~irq;
    *P_DMA_INTSTS = bit | (bit << 8);
    *P_INT_Status = irq;
}
