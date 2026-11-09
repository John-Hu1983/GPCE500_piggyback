#include "hal_spi.h"
#include "System.h"

/* Native startup copies initialized data but does not clear ordinary .RAM. */
static SPI_Job *volatile spi_job = null;

/*
 *@ brief    : Reset SPI2 and clear its FIFO flags.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : SPI2 FIFOs and requests are reset.
 *@ theme    : Keep CS routing in GPIO mode; never replay read-only busy or reset bits.
 */
static void _spi_reset_fifo(void)
{
    *P_SPI2_Ctrl =
        (*P_SPI2_Ctrl & ~(C_SPI_Reset | C_SPI_Module_Busy_Flag | C_SPI_CS)) | C_SPI_Reset;
    *P_SPI2_TX_Status = C_SPI_TX_INT_Flag;
    *P_SPI2_RX_Status = C_SPI_RX_INT_Flag | C_SPI_RX_BUF_OVER_ERROR;
    *P_SPI2_Man_Ctrl = C_SPI_Continuous_Enable;
}

/*
 *@ brief    : Validate a SPI2 buffer request.
 *@ type     : private
 *@ argu in  : tx/rx: at least one buffer; count: nonzero bytes.
 *@ argu out : Explicit argument/mode/busy result.
 *@ theme    : All transfer paths are full-duplex on the wire.
 */
static Serial_Result _spi_check_buffer(const unsigned *tx, unsigned *rx, unsigned count)
{
    if ((tx == null && rx == null) || count == 0)
    {
        return Serial_Err_Arg;
    }
    if ((*P_SPI2_Ctrl & C_SPI_Enable) == 0)
    {
        return Serial_Err_Mode;
    }
    if (spi_job != null || (*P_SPI2_Misc & C_SPI_Busy_Flag) != 0)
    {
        return Serial_Busy;
    }
    return tx != null ? serial_check_bytes(tx, count) : Serial_Ok;
}

/*
 *@ brief    : Finish or fail the active SPI2 request.
 *@ type     : private
 *@ argu in  : result: Done, error or Aborted.
 *@ argu out : Caller state is published after DMA stop.
 *@ theme    : Error cleanup stops clocks and permits safe buffer reuse without changing external
 * CS.
 */
static void _spi_finish_job(Serial_Result result)
{
    SPI_Job *job = spi_job;
    unsigned mask = C_IRQ3_SPI2;
    if (job == null)
    {
        return;
    }
    if (job->tx_dma == DMA_Ch_1 || job->rx_dma == DMA_Ch_1)
        mask |= C_IRQ3_DMA1;
    if (job->tx_dma == DMA_Ch_2 || job->rx_dma == DMA_Ch_2)
        mask |= C_IRQ3_DMA2;
    *P_INT_Ctrl &= ~mask;
    *P_SPI2_Man_Ctrl &= ~(C_SPI_TX_DMA_Enable | C_SPI_RX_DMA_Enable);
    *P_SPI2_TX_Status = C_SPI_TX_INT_Flag;
    *P_SPI2_RX_Status = C_SPI_RX_INT_Flag | C_SPI_RX_BUF_OVER_ERROR;
    spi_job = null;
    if (job->tx_dma != DMA_None)
        serial_stop_dma(job->tx_dma);
    if (job->rx_dma != DMA_None)
        serial_stop_dma(job->rx_dma);
    job->tx_dma = DMA_None;
    job->rx_dma = DMA_None;
    if (result != Serial_Done)
    {
        _spi_reset_fifo();
    }
    *P_INT_Status = C_IRQ3_SPI2;
    job->state = result;
}

/*
 *@ brief    : Queue the next ISR byte after the previous RX was drained.
 *@ type     : private
 *@ argu in  : job: active request.
 *@ argu out : Exactly one byte is in flight.
 *@ theme    : Bound RX occupancy even when CPU interrupt latency exceeds byte time.
 */
static void _spi_send_next(SPI_Job *job)
{
    if (job->tx_done < job->count && (*P_SPI2_Misc & C_SPI_TX_NonFull_Flag) != 0)
    {
        *P_SPI2_TX_Data = job->tx != null ? job->tx[job->tx_done] : SERIAL_SPI_FILL;
        ++job->tx_done;
    }
}

/*
 *@ brief    : Initialize SPI2 master clock and timing.
 *@ type     : public
 *@ argu in  : hz: maximum requested SCK Hz; mode: clock phase/polarity.
 *@ argu out : Ok or rate/port/mode/busy error.
 *@ theme    : Choose a system-clock divider without changing shared Timer A or SPI1.
 */
Serial_Result spi_init_module(unsigned long hz, SPI_Mode mode)
{
    unsigned sel = 0;
    unsigned long clock = serial_read_clock();
#if SPI_PORT == SERIAL_PORT_A
    unsigned i;
#endif
    if (spi_job != null || (*P_SPI2_Misc & C_SPI_Busy_Flag) != 0)
    {
        return Serial_Busy;
    }
    if (mode != SPI_Mode_0 && mode != SPI_Mode_1 && mode != SPI_Mode_2 && mode != SPI_Mode_3)
    {
        return Serial_Err_Mode;
    }
    if (hz == 0)
    {
        return Serial_Err_Rate;
    }
    while (sel < 6 && (clock + (2UL << sel) - 1UL) / (2UL << sel) > hz)
    {
        ++sel;
    }
    if (hz < clock / 128UL + (clock % 128UL != 0 ? 1UL : 0UL))
    {
        return Serial_Err_Rate;
    }
    if ((*P_UART_Ctrl & C_UART_Enable) != 0 &&
        (*P_IO_Ctrl & C_UART_IOB) == (SPI_PORT == SERIAL_PORT_B ? C_UART_IOB : 0))
    {
        return Serial_Err_Port;
    }
#if SPI_PORT == SERIAL_PORT_A
    for (i = 1; i <= 3; ++i)
    {
        if ((((volatile unsigned *)P_PWM0_Ctrl)[i] & C_IOPWM_Enable) != 0)
        {
            return Serial_Err_Port;
        }
    }
#endif
    *P_INT_Ctrl &= ~C_IRQ3_SPI2;
    *P_SPI2_Ctrl = 0;
#if SPI_PORT == SERIAL_PORT_A
    *P_IO_Ctrl &= ~C_SPI2_IOB;
    *P_IO_Swap = (*P_IO_Swap & ~C_SPI2_IOA_Swap_Enable) | C_SPI2_IOA_MOSI_Enable;
    gpio_init_pattern(GPIOA, 1, (mode & C_SPI_Clock_Pol) != 0 ? GPIO_Out_High : GPIO_Out_Low);
    gpio_init_pattern(GPIOA, 2, GPIO_In_Float);
    gpio_init_pattern(GPIOA, 3, GPIO_Out_Low);
#else
    *P_IO_Ctrl |= C_SPI2_IOB;
    *P_IO_Swap = (*P_IO_Swap & ~C_SPI2_IOB_Swap_Enable) | C_SPI2_IOB_MOSI_Enable;
    gpio_init_pattern(GPIOB, 13, (mode & C_SPI_Clock_Pol) != 0 ? GPIO_Out_High : GPIO_Out_Low);
    gpio_init_pattern(GPIOB, 14, GPIO_In_Float);
    gpio_init_pattern(GPIOB, 15, GPIO_Out_Low);
#endif
    *P_SPI2_Ctrl = C_SPI_CS_GPIO | (unsigned)mode | sel;
    _spi_reset_fifo();
    *P_SPI2_Misc = C_SPI_FIFO_Smart_Mode;
    *P_SPI2_Ctrl = C_SPI_Enable | C_SPI_CS_GPIO | (unsigned)mode | sel;
    *P_FIQ_Sel &= ~C_IRQ3_SPI2;
    *P_INT_Status = C_IRQ3_SPI2;
    return Serial_Ok;
}

/*
 *@ brief    : Read the current nominal SPI2 SCK frequency.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Hz, or zero when disabled/externally switched to Timer A.
 *@ theme    : Application must not change Fsys during a transfer.
 */
unsigned long spi_read_rate(void)
{
    unsigned sel = *P_SPI2_Ctrl & C_SPI_SCK_SEL;
    return (*P_SPI2_Ctrl & C_SPI_Enable) != 0 && sel < 7 ? serial_read_clock() / (2UL << sel) : 0;
}

/*
 *@ brief    : Disable SPI2 and cancel a pending transfer.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : SPI2 requests are disabled; external CS is unchanged.
 *@ theme    : SPI1 Flash and unrelated interrupt sources remain unchanged.
 */
void spi_disable_module(void)
{
    spi_abort_transfer();
    *P_INT_Ctrl &= ~C_IRQ3_SPI2;
    *P_SPI2_Ctrl = 0;
}

/*
 *@ brief    : Transfer a blocking SPI2 full-duplex data segment.
 *@ type     : public
 *@ argu in  : tx/rx: optional source/destination; count: bytes.
 *@ argu out : Done or explicit failure; RX has a valid prefix on failure.
 *@ theme    : Drain each RX byte and wait for idle; framing and external CS belong to the caller.
 */
Serial_Result spi_transfer_manual(const unsigned *tx, unsigned *rx, unsigned count)
{
    Serial_Result result = _spi_check_buffer(tx, rx, count);
    unsigned i;
    unsigned data;
    unsigned long wait;
    if (result != Serial_Ok)
    {
        return result;
    }
    for (i = 0; i < count; ++i)
    {
        wait = SERIAL_WAIT_LIMIT;
        while ((*P_SPI2_Misc & C_SPI_TX_NonFull_Flag) == 0)
        {
            if (--wait == 0)
            {
                _spi_reset_fifo();
                return Serial_Err_Timeout;
            }
        }
        *P_SPI2_TX_Data = tx != null ? tx[i] : SERIAL_SPI_FILL;
        wait = SERIAL_WAIT_LIMIT;
        while ((*P_SPI2_Misc & C_SPI_RX_NonEmpty_Flag) == 0)
        {
            if (--wait == 0)
            {
                _spi_reset_fifo();
                return Serial_Err_Timeout;
            }
        }
        if ((*P_SPI2_RX_Status & C_SPI_RX_BUF_OVER_ERROR) != 0)
        {
            _spi_reset_fifo();
            return Serial_Err_RX;
        }
        data = *P_SPI2_RX_Data;
        if (rx != null)
            rx[i] = data & SERIAL_BYTE_MASK;
    }
    wait = SERIAL_WAIT_LIMIT;
    while ((*P_SPI2_Misc & C_SPI_Busy_Flag) != 0)
    {
        if (--wait == 0)
        {
            _spi_reset_fifo();
            return Serial_Err_Timeout;
        }
    }
    return Serial_Done;
}

/*
 *@ brief    : Send SPI2 bytes synchronously, discarding received bytes.
 *@ type     : public
 *@ argu in  : buf/count: source bytes.
 *@ argu out : Blocking data-segment result.
 *@ theme    : RX is drained even for transmit-only traffic.
 */
Serial_Result spi_send_manual(const unsigned *buf, unsigned count)
{
    return spi_transfer_manual(buf, null, count);
}

/*
 *@ brief    : Receive SPI2 bytes synchronously using 0xFF clocks.
 *@ type     : public
 *@ argu in  : buf/count: destination bytes.
 *@ argu out : Blocking data-segment result.
 *@ theme    : A master receive always generates transmit clocks.
 */
Serial_Result spi_recv_manual(unsigned *buf, unsigned count)
{
    return spi_transfer_manual(null, buf, count);
}

/*
 *@ brief    : Submit an interrupt-driven SPI2 data segment.
 *@ type     : public
 *@ argu in  : job: persistent caller storage; tx/rx/count: buffers.
 *@ argu out : Submission result; completion is job->state.
 *@ theme    : RX IRQ drives the next TX byte, avoiding receive FIFO overflow.
 */
Serial_Result spi_transfer_isr(SPI_Job *job, const unsigned *tx, unsigned *rx, unsigned count)
{
    Serial_Result result = _spi_check_buffer(tx, rx, count);
    if (job == null)
    {
        return Serial_Err_Arg;
    }
    if (result != Serial_Ok)
    {
        return result;
    }
    job->tx = tx;
    job->rx = rx;
    job->count = count;
    job->tx_done = 0;
    job->rx_done = 0;
    job->tx_dma = DMA_None;
    job->rx_dma = DMA_None;
    job->state = Serial_Busy;
    spi_job = job;
    _spi_send_next(job);
    *P_SPI2_RX_Status = C_SPI_RX_INT_Flag | C_SPI_RX_INT_EN | C_SPI_RX_FIFO_Level_1;
    *P_INT_Status = C_IRQ3_SPI2;
    *P_INT_Ctrl |= C_IRQ3_SPI2;
    system_enable_irq();
    return Serial_Ok;
}

/*
 *@ brief    : Submit interrupt-driven SPI2 transmit-only traffic.
 *@ type     : public
 *@ argu in  : job/buf/count: persistent request.
 *@ argu out : Submission result.
 *@ theme    : The receive side is still drained by the ISR.
 */
Serial_Result spi_send_isr(SPI_Job *job, const unsigned *buf, unsigned count)
{
    return spi_transfer_isr(job, buf, null, count);
}

/*
 *@ brief    : Submit interrupt-driven SPI2 receive-only traffic.
 *@ type     : public
 *@ argu in  : job/buf/count: persistent request.
 *@ argu out : Submission result.
 *@ theme    : The ISR sends one 0xFF dummy word per received byte.
 */
Serial_Result spi_recv_isr(SPI_Job *job, unsigned *buf, unsigned count)
{
    return spi_transfer_isr(job, null, buf, count);
}

/*
 *@ brief    : Submit a two-channel DMA SPI2 data segment.
 *@ type     : public
 *@ argu in  : job/buffers/count: working-RAM request; tx_dma/rx_dma: different free channels.
 *@ argu out : Submission result; both directions and wire activity must finish before Done.
 *@ theme    : Arm RX first, then TX; dummy/discard use fixed caller-owned scratch addresses.
 */
Serial_Result spi_transfer_dma(SPI_Job *job, const unsigned *tx, unsigned *rx, unsigned count,
                               DMA_Channel tx_dma, DMA_Channel rx_dma)
{
    Serial_Result result = _spi_check_buffer(tx, rx, count);
    if (job == null || tx_dma == rx_dma)
    {
        return Serial_Err_Arg;
    }
    if (result == Serial_Ok)
        result = serial_check_memory(&job->pad, 1);
    if (result == Serial_Ok)
        result = serial_check_memory(&job->discard, 1);
    if (result == Serial_Ok && tx != null)
        result = serial_check_memory(tx, count);
    if (result == Serial_Ok && rx != null)
        result = serial_check_memory(rx, count);
    if (result == Serial_Ok)
        result = serial_check_dma(tx_dma);
    if (result == Serial_Ok)
        result = serial_check_dma(rx_dma);
    if (result != Serial_Ok)
    {
        return result;
    }
    job->tx = tx;
    job->rx = rx;
    job->count = count;
    job->tx_done = 0;
    job->rx_done = 0;
    job->tx_dma = tx_dma;
    job->rx_dma = rx_dma;
    job->pad = SERIAL_SPI_FILL;
    job->discard = 0;
    job->state = Serial_Busy;
    spi_job = job;
    serial_start_dma(rx_dma, C_DMA_Request_SPI2, (const unsigned *)P_SPI2_RX_Data,
                     rx != null ? rx : &job->discard, count,
                     rx != null ? C_DMA_Dest_Inc_Enable : 0);
    *P_SPI2_Man_Ctrl |= C_SPI_RX_DMA_Enable;
    serial_start_dma(tx_dma, C_DMA_Request_SPI2, tx != null ? tx : &job->pad,
                     (unsigned *)P_SPI2_TX_Data, count, tx != null ? C_DMA_Src_Inc_Enable : 0);
    *P_SPI2_Man_Ctrl |= C_SPI_TX_DMA_Enable;
    return Serial_Ok;
}

/*
 *@ brief    : Submit SPI2 DMA transmit-only traffic.
 *@ type     : public
 *@ argu in  : job/buf/count: working-RAM request; tx_dma/rx_dma: two free channels.
 *@ argu out : Submission result.
 *@ theme    : RX DMA discards incoming words into the job's single scratch word.
 */
Serial_Result spi_send_dma(SPI_Job *job, const unsigned *buf, unsigned count, DMA_Channel tx_dma,
                           DMA_Channel rx_dma)
{
    return spi_transfer_dma(job, buf, null, count, tx_dma, rx_dma);
}

/*
 *@ brief    : Submit SPI2 DMA receive-only traffic.
 *@ type     : public
 *@ argu in  : job/buf/count: working-RAM request; tx_dma/rx_dma: two free channels.
 *@ argu out : Submission result.
 *@ theme    : TX DMA repeatedly sends the job's fixed 0xFF word.
 */
Serial_Result spi_recv_dma(SPI_Job *job, unsigned *buf, unsigned count, DMA_Channel tx_dma,
                           DMA_Channel rx_dma)
{
    return spi_transfer_dma(job, null, buf, count, tx_dma, rx_dma);
}

/*
 *@ brief    : Publish SPI2 completion when the controller is idle.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Completed job is Done; the caller may submit the next data segment.
 *@ theme    : Call from the main loop; completion never changes external CS.
 */
void spi_poll_transfer(void)
{
    SPI_Job *job = spi_job;
    if (job != null && job->tx_done == job->count && job->rx_done == job->count &&
        job->tx_dma == DMA_None && job->rx_dma == DMA_None && (*P_SPI2_Misc & C_SPI_Busy_Flag) == 0)
    {
        _spi_finish_job(Serial_Done);
    }
}

/*
 *@ brief    : Abort the active SPI2 request.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Active DMA stops, clocks cease, and caller state is Aborted.
 *@ theme    : External CS stays under caller control, including errors and cancellation.
 */
void spi_abort_transfer(void) { _spi_finish_job(Serial_Aborted); }

/*
 *@ brief    : Service owned SPI2 FIFO and DMA IRQ3 sources.
 *@ type     : public
 *@ argu in  : None; called by the assembly vector.
 *@ argu out : Request progress and explicit error notification.
 *@ theme    : DMA errors take priority over both channels' done flags; other sources are preserved.
 */
void spi_service_irq(void)
{
    SPI_Job *job = spi_job;
    Serial_Result tx_result = Serial_Busy;
    Serial_Result rx_result = Serial_Busy;
    unsigned data;
    if (job == null)
    {
        return;
    }
    if (job->tx_dma != DMA_None)
        tx_result = serial_read_dma(job->tx_dma);
    if (job->rx_dma != DMA_None)
        rx_result = serial_read_dma(job->rx_dma);
    if (tx_result == Serial_Err_DMA || rx_result == Serial_Err_DMA)
    {
        _spi_finish_job(Serial_Err_DMA);
        return;
    }
    if ((*P_SPI2_RX_Status & C_SPI_RX_BUF_OVER_ERROR) != 0)
    {
        _spi_finish_job(Serial_Err_RX);
        return;
    }
    if (tx_result == Serial_Done)
    {
        serial_stop_dma(job->tx_dma);
        job->tx_dma = DMA_None;
        job->tx_done = job->count;
    }
    if (rx_result == Serial_Done)
    {
        serial_stop_dma(job->rx_dma);
        job->rx_dma = DMA_None;
        job->rx_done = job->count;
    }
    if (job->tx_done == job->count && job->rx_done == job->count)
    {
        *P_SPI2_RX_Status = C_SPI_RX_INT_Flag;
        *P_INT_Ctrl &= ~C_IRQ3_SPI2;
        *P_INT_Status = C_IRQ3_SPI2;
        return;
    }
    if ((*P_INT_Ctrl & C_IRQ3_SPI2) == 0 || (*P_SPI2_RX_Status & C_SPI_RX_INT_EN) == 0)
    {
        return;
    }
    if ((*P_SPI2_Misc & C_SPI_RX_NonEmpty_Flag) != 0 && job->rx_done < job->count)
    {
        data = *P_SPI2_RX_Data;
        if (job->rx != null)
            job->rx[job->rx_done] = data & SERIAL_BYTE_MASK;
        ++job->rx_done;
        if (job->rx_done < job->count)
        {
            _spi_send_next(job);
        }
        else
        {
            *P_SPI2_RX_Status = C_SPI_RX_INT_Flag;
            *P_INT_Ctrl &= ~C_IRQ3_SPI2;
        }
    }
    *P_INT_Status = C_IRQ3_SPI2;
}