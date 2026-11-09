#include "hal_uart.h"
#include "System.h"

/* Native startup copies initialized data but does not clear ordinary .RAM. */
static UART_Job *volatile uart_tx = null;
static UART_Job *volatile uart_rx = null;

/*
 *@ brief    : Reset UART FIFOs and acknowledge the reset command.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Both FIFOs are empty.
 *@ theme    : Reset is write-only; the required dummy read completes the operation.
 */
static void _uart_reset_fifo(void)
{
    *P_UART_Ctrl = (*P_UART_Ctrl & ~C_UART_Reset) | C_UART_Reset;
    (void)*P_UART_Ctrl;
    *P_UART_RX_Status = SERIAL_UART_RX_ERRORS;
}

/*
 *@ brief    : Match the shared UART IRQ mask to its FIFO enables.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Only the UART IRQ source is changed.
 *@ theme    : Clear the global latch after servicing the peripheral.
 */
static void _uart_sync_irq(void)
{
    *P_INT2_Status = C_IRQ7_UART;
    if ((*P_UART_Ctrl &
         (C_UART_TX_INT_Enable | C_UART_RX_INT_Enable | C_UART_RX_Tout_INT_Enable)) != 0)
    {
        *P_FIQ2_Sel &= ~C_IRQ7_UART;
        *P_INT2_Ctrl |= C_IRQ7_UART;
    }
    else
    {
        *P_INT2_Ctrl &= ~C_IRQ7_UART;
    }
}

/*
 *@ brief    : Fill available TX FIFO cells from an ISR job.
 *@ type     : private
 *@ argu in  : job: active transmit request.
 *@ argu out : job->done counts FIFO-accepted bytes.
 *@ theme    : Mask the level IRQ once all bytes are queued; wire completion is polled.
 */
static void _uart_fill_fifo(UART_Job *job)
{
    unsigned budget = 16;
    while (budget-- != 0 && job->done < job->count &&
           (*P_UART_Status & C_UART_TX_FIFO_Full_Flag) == 0)
    {
        *P_UART_Data = job->buf.tx[job->done];
        ++job->done;
    }
    if (job->done == job->count)
    {
        *P_UART_Ctrl &= ~(C_UART_TX_INT_Enable | C_UART_Reset);
    }
}

/*
 *@ brief    : Publish a receive result and release its resources.
 *@ type     : private
 *@ argu in  : result: Done or a receive/DMA error.
 *@ argu out : Caller job is complete; TX is not interrupted.
 *@ theme    : DMA RX words carry error bits and are sanitized only after validation.
 */
static void _uart_finish_rx(Serial_Result result)
{
    UART_Job *job = uart_rx;
    unsigned i;
    if (job == null)
    {
        return;
    }
    *P_UART_Ctrl &=
        ~(C_UART_RX_INT_Enable | C_UART_RX_Tout_INT_Enable | C_UART_RX_DMA_Enable | C_UART_Reset);
    if (job->dma != DMA_None)
    {
        serial_stop_dma(job->dma);
        job->dma = DMA_None;
        if (result == Serial_Done)
        {
            for (i = 0; i < job->count; ++i)
            {
                if ((job->buf.rx[i] & SERIAL_UART_ERRORS) != 0)
                {
                    result = Serial_Err_RX;
                    break;
                }
                job->buf.rx[i] &= SERIAL_BYTE_MASK;
                ++job->done;
            }
        }
    }
    if (result != Serial_Err_DMA && (*P_UART_RX_Status & SERIAL_UART_RX_ERRORS) != 0)
    {
        result = Serial_Err_RX;
    }
    *P_UART_RX_Status = SERIAL_UART_RX_ERRORS;
    uart_rx = null;
    job->state = result;
    _uart_sync_irq();
}

/*
 *@ brief    : Validate an asynchronous UART submission.
 *@ type     : private
 *@ argu in  : job: persistent caller storage; tx: direction; count: bytes.
 *@ argu out : Explicit readiness/argument result.
 *@ theme    : A Busy job cannot be reused for either direction.
 */
static Serial_Result _uart_check_job(UART_Job *job, bool tx, unsigned count)
{
    if (job == null || count == 0)
    {
        return Serial_Err_Arg;
    }
    if ((*P_UART_Ctrl & C_UART_Enable) == 0)
    {
        return Serial_Err_Mode;
    }
    if (job == uart_tx || job == uart_rx || (tx ? uart_tx != null : uart_rx != null))
    {
        return Serial_Busy;
    }
    return Serial_Ok;
}

/*
 *@ brief    : Initialize UART 8N1 at the requested baud rate.
 *@ type     : public
 *@ argu in  : baud: desired bits per second.
 *@ argu out : Ok or rate/port/busy error; actual rate is read separately.
 *@ theme    : Round the divider and enforce the configured baud-error bound.
 */
Serial_Result uart_init_module(unsigned long baud)
{
    unsigned long base = serial_read_clock() / 16UL;
    unsigned long div;
    unsigned long actual;
    unsigned long err;
#if UART_PORT == SERIAL_PORT_A
    unsigned i;
#endif
    if (uart_tx != null || uart_rx != null)
    {
        return Serial_Busy;
    }
    if (baud == 0 || baud > base)
    {
        return Serial_Err_Rate;
    }
    div = (base + baud / 2UL) / baud;
    if (div == 0 || div > 65536UL)
    {
        return Serial_Err_Rate;
    }
    actual = base / div;
    err = actual > baud ? actual - baud : baud - actual;
    if (err * 100UL > baud * UART_BAUD_ERROR_PERCENT)
    {
        return Serial_Err_Rate;
    }
    if ((*P_SPI2_Ctrl & C_SPI_Enable) != 0 &&
        (*P_IO_Ctrl & C_SPI2_IOB) == (UART_PORT == SERIAL_PORT_B ? C_SPI2_IOB : 0))
    {
        return Serial_Err_Port;
    }
#if UART_PORT == SERIAL_PORT_A
    for (i = 2; i <= 3; ++i)
    {
        if ((((volatile unsigned *)P_PWM0_Ctrl)[i] & C_IOPWM_Enable) != 0)
        {
            return Serial_Err_Port;
        }
    }
#endif
    uart_disable_module();
#if UART_PORT == SERIAL_PORT_A
    *P_IO_Ctrl &= ~C_UART_IOB;
    *P_IO_Swap &= ~(C_UART_IOA_INV_Enable | C_UART_IOA_Swap_Enable);
    gpio_init_pattern(GPIOA, 3, GPIO_Out_High);
    gpio_init_pattern(GPIOA, 2, GPIO_In_Float);
#else
    *P_IO_Ctrl |= C_UART_IOB;
    *P_IO_Swap &= ~(C_UART_IOB_INV_Enable | C_UART_IOB_Swap_Enable);
    gpio_init_pattern(GPIOB, 15, GPIO_Out_High);
    gpio_init_pattern(GPIOB, 14, GPIO_In_Float);
#endif
    *P_UART_BaudRate = (unsigned)(div - 1UL);
    *P_UART_Ctrl = C_UART_Enable | C_UART_Data_Length_8bit | C_UART_FIFO_Enable;
    _uart_reset_fifo();
    return Serial_Ok;
}

/*
 *@ brief    : Read the current nominal baud rate.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Bits per second; zero when disabled.
 *@ theme    : Application must not change Fsys during a transfer.
 */
unsigned long uart_read_rate(void)
{
    return (*P_UART_Ctrl & C_UART_Enable) != 0
               ? (serial_read_clock() / 16UL) / ((unsigned long)*P_UART_BaudRate + 1UL)
               : 0;
}

/*
 *@ brief    : Disable UART and cancel pending requests.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : UART and its own IRQ/DMA requests are disabled.
 *@ theme    : No other peripheral or DMA channel is reset.
 */
void uart_disable_module(void)
{
    uart_abort_transfer();
    *P_UART_Ctrl = 0;
}

/*
 *@ brief    : Send a word-per-byte buffer synchronously.
 *@ type     : public
 *@ argu in  : buf: source; count: nonzero bytes.
 *@ argu out : Done only after the last stop bit, or explicit failure.
 *@ theme    : A TX timeout resets both FIFOs and aborts any concurrent async RX.
 */
Serial_Result uart_send_manual(const unsigned *buf, unsigned count)
{
    unsigned i;
    unsigned long wait;
    Serial_Result result = serial_check_bytes(buf, count);
    if (result != Serial_Ok)
    {
        return result;
    }
    if ((*P_UART_Ctrl & C_UART_Enable) == 0)
    {
        return Serial_Err_Mode;
    }
    if (uart_tx != null)
    {
        return Serial_Busy;
    }
    for (i = 0; i < count; ++i)
    {
        wait = SERIAL_WAIT_LIMIT;
        while ((*P_UART_Status & C_UART_TX_FIFO_Full_Flag) != 0)
        {
            if (--wait == 0)
            {
                uart_abort_transfer();
                return Serial_Err_Timeout;
            }
        }
        *P_UART_Data = buf[i];
    }
    wait = SERIAL_WAIT_LIMIT;
    while ((*P_UART_Status & (C_UART_TX_FIFO_Empty_Flag | C_UART_Busy_Flag)) !=
           C_UART_TX_FIFO_Empty_Flag)
    {
        if (--wait == 0)
        {
            uart_abort_transfer();
            return Serial_Err_Timeout;
        }
    }
    return Serial_Done;
}

/*
 *@ brief    : Receive exactly count bytes synchronously.
 *@ type     : public
 *@ argu in  : buf: destination; count: nonzero bytes.
 *@ argu out : Done, timeout or RX error; an error leaves only a valid prefix.
 *@ theme    : Frame/parity/break/overrun errors are not returned as valid payload.
 */
Serial_Result uart_recv_manual(unsigned *buf, unsigned count)
{
    unsigned i;
    unsigned data;
    unsigned long wait;
    if (buf == null || count == 0)
    {
        return Serial_Err_Arg;
    }
    if ((*P_UART_Ctrl & C_UART_Enable) == 0)
    {
        return Serial_Err_Mode;
    }
    if (uart_rx != null)
    {
        return Serial_Busy;
    }
    for (i = 0; i < count; ++i)
    {
        wait = SERIAL_WAIT_LIMIT;
        while ((*P_UART_Status & C_UART_RX_FIFO_Empty_Flag) != 0)
        {
            if ((*P_UART_RX_Status & SERIAL_UART_RX_ERRORS) != 0)
            {
                *P_UART_RX_Status = SERIAL_UART_RX_ERRORS;
                return Serial_Err_RX;
            }
            if (--wait == 0)
            {
                return Serial_Err_Timeout;
            }
        }
        data = *P_UART_Data;
        if ((data & SERIAL_UART_ERRORS) != 0 || (*P_UART_RX_Status & SERIAL_UART_RX_ERRORS) != 0)
        {
            *P_UART_RX_Status = SERIAL_UART_RX_ERRORS;
            return Serial_Err_RX;
        }
        buf[i] = data & SERIAL_BYTE_MASK;
    }
    return Serial_Done;
}

/*
 *@ brief    : Submit an interrupt-driven UART transmit.
 *@ type     : public
 *@ argu in  : job: caller storage; buf/count: source bytes.
 *@ argu out : Submission result; completion is in job->state.
 *@ theme    : Prefill before enabling TX IRQ to avoid foreground/ISR FIFO races.
 */
Serial_Result uart_send_isr(UART_Job *job, const unsigned *buf, unsigned count)
{
    Serial_Result result = _uart_check_job(job, true, count);
    if (result == Serial_Ok)
    {
        result = serial_check_bytes(buf, count);
    }
    if (result != Serial_Ok)
    {
        return result;
    }
    job->buf.tx = buf;
    job->count = count;
    job->done = 0;
    job->dma = DMA_None;
    job->state = Serial_Busy;
    uart_tx = job;
    _uart_fill_fifo(job);
    if (job->done < count)
    {
        *P_UART_Ctrl = (*P_UART_Ctrl & ~C_UART_Reset) | C_UART_TX_INT_Enable;
    }
    _uart_sync_irq();
    system_enable_irq();
    return Serial_Ok;
}

/*
 *@ brief    : Submit an interrupt-driven UART receive.
 *@ type     : public
 *@ argu in  : job: caller storage; buf/count: destination bytes.
 *@ argu out : Submission result; valid prefix length is job->done.
 *@ theme    : RX timeout IRQ drains short packets below the eight-byte FIFO threshold.
 */
Serial_Result uart_recv_isr(UART_Job *job, unsigned *buf, unsigned count)
{
    Serial_Result result = _uart_check_job(job, false, count);
    if (buf == null)
    {
        return Serial_Err_Arg;
    }
    if (result != Serial_Ok)
    {
        return result;
    }
    job->buf.rx = buf;
    job->count = count;
    job->done = 0;
    job->dma = DMA_None;
    job->state = Serial_Busy;
    uart_rx = job;
    *P_UART_Ctrl =
        (*P_UART_Ctrl & ~C_UART_Reset) | C_UART_RX_INT_Enable | C_UART_RX_Tout_INT_Enable;
    _uart_sync_irq();
    system_enable_irq();
    return Serial_Ok;
}

/*
 *@ brief    : Submit a DMA-driven UART transmit.
 *@ type     : public
 *@ argu in  : job/buf/count: persistent request; channel: free DMA1 or DMA2.
 *@ argu out : Submission result; DMA completion alone is not wire completion.
 *@ theme    : Use incrementing working-RAM words and a fixed UART data address.
 */
Serial_Result uart_send_dma(UART_Job *job, const unsigned *buf, unsigned count, DMA_Channel channel)
{
    Serial_Result result = _uart_check_job(job, true, count);
    if (result == Serial_Ok)
        result = serial_check_bytes(buf, count);
    if (result == Serial_Ok)
        result = serial_check_memory(buf, count);
    if (result == Serial_Ok)
        result = serial_check_dma(channel);
    if (result != Serial_Ok)
    {
        return result;
    }
    job->buf.tx = buf;
    job->count = count;
    job->done = 0;
    job->dma = channel;
    job->state = Serial_Busy;
    uart_tx = job;
    serial_start_dma(channel, C_DMA_Request_UART, buf, (unsigned *)P_UART_Data, count,
                     C_DMA_Src_Inc_Enable);
    *P_UART_Ctrl = (*P_UART_Ctrl & ~C_UART_Reset) | C_UART_TX_DMA_Enable;
    return Serial_Ok;
}

/*
 *@ brief    : Submit a DMA-driven UART receive.
 *@ type     : public
 *@ argu in  : job/buf/count: persistent request; channel: free DMA1 or DMA2.
 *@ argu out : Submission result; RX error bits are validated on completion.
 *@ theme    : DMA stores raw sixteen-bit UART data words, not packed bytes.
 */
Serial_Result uart_recv_dma(UART_Job *job, unsigned *buf, unsigned count, DMA_Channel channel)
{
    Serial_Result result = _uart_check_job(job, false, count);
    if (result == Serial_Ok)
        result = serial_check_memory(buf, count);
    if (result == Serial_Ok)
        result = serial_check_dma(channel);
    if (result != Serial_Ok)
    {
        return result;
    }
    job->buf.rx = buf;
    job->count = count;
    job->done = 0;
    job->dma = channel;
    job->state = Serial_Busy;
    uart_rx = job;
    serial_start_dma(channel, C_DMA_Request_UART, (const unsigned *)P_UART_Data, buf, count,
                     C_DMA_Dest_Inc_Enable);
    *P_UART_Ctrl = (*P_UART_Ctrl & ~C_UART_Reset) | C_UART_RX_DMA_Enable;
    return Serial_Ok;
}

/*
 *@ brief    : Publish final UART wire completion from the main loop.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : A fully shifted TX job becomes Done.
 *@ theme    : DMA result processing is IRQ-only to prevent simultaneous buffer scanning.
 */
void uart_poll_transfer(void)
{
    UART_Job *job = uart_tx;
    if (job != null && job->dma == DMA_None && job->done == job->count &&
        (*P_UART_Status & (C_UART_TX_FIFO_Empty_Flag | C_UART_Busy_Flag)) ==
            C_UART_TX_FIFO_Empty_Flag)
    {
        uart_tx = null;
        job->state = Serial_Done;
    }
}

/*
 *@ brief    : Cancel both UART directions and reset their FIFOs.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Active jobs are Aborted only after their DMA channels stop.
 *@ theme    : Mask owned sources before releasing pointers; do not disturb unrelated DMA.
 */
void uart_abort_transfer(void)
{
    UART_Job *tx = uart_tx;
    UART_Job *rx = uart_rx;
    unsigned mask = 0;
    *P_INT2_Ctrl &= ~C_IRQ7_UART;
    if (tx != null)
    {
        if (tx->dma == DMA_Ch_1)
            mask |= C_IRQ3_DMA1;
        if (tx->dma == DMA_Ch_2)
            mask |= C_IRQ3_DMA2;
    }
    if (rx != null)
    {
        if (rx->dma == DMA_Ch_1)
            mask |= C_IRQ3_DMA1;
        if (rx->dma == DMA_Ch_2)
            mask |= C_IRQ3_DMA2;
    }
    *P_INT_Ctrl &= ~mask;
    *P_UART_Ctrl &= ~(C_UART_TX_INT_Enable | C_UART_RX_INT_Enable | C_UART_RX_Tout_INT_Enable |
                      C_UART_TX_DMA_Enable | C_UART_RX_DMA_Enable | C_UART_Reset);
    uart_tx = null;
    uart_rx = null;
    if (tx != null)
    {
        if (tx->dma != DMA_None)
            serial_stop_dma(tx->dma);
        tx->dma = DMA_None;
        tx->state = Serial_Aborted;
    }
    if (rx != null)
    {
        if (rx->dma != DMA_None)
            serial_stop_dma(rx->dma);
        rx->dma = DMA_None;
        rx->state = Serial_Aborted;
    }
    _uart_reset_fifo();
    *P_INT2_Status = C_IRQ7_UART;
}

/*
 *@ brief    : Service UART FIFO IRQ7 and owned DMA IRQ3 sources.
 *@ type     : public
 *@ argu in  : None; called by the assembly vectors.
 *@ argu out : FIFO progress, explicit receive/DMA errors and owned-source acknowledgement.
 *@ theme    : Never write read-only UART status or acknowledge somebody else's DMA flags.
 */
void uart_service_irq(void)
{
    UART_Job *tx = uart_tx;
    UART_Job *rx = uart_rx;
    unsigned data;
    unsigned budget = 16;
    Serial_Result result;

    if (tx != null && tx->dma != DMA_None)
    {
        result = serial_read_dma(tx->dma);
        if (result == Serial_Done || result == Serial_Err_DMA)
        {
            *P_UART_Ctrl &= ~(C_UART_TX_DMA_Enable | C_UART_Reset);
            serial_stop_dma(tx->dma);
            tx->dma = DMA_None;
            if (result == Serial_Done)
            {
                tx->done = tx->count;
            }
            else
            {
                uart_abort_transfer();
                tx->state = result;
            }
        }
    }

    rx = uart_rx;
    if (rx != null && rx->dma != DMA_None)
    {
        result = serial_read_dma(rx->dma);
        if (result == Serial_Done || result == Serial_Err_DMA)
        {
            _uart_finish_rx(result);
        }
    }

    if ((*P_INT2_Ctrl & C_IRQ7_UART) == 0)
    {
        return;
    }

    tx = uart_tx;
    rx = uart_rx;
    if (tx != null && tx->dma == DMA_None && (*P_UART_Ctrl & C_UART_TX_INT_Enable) != 0)
    {
        _uart_fill_fifo(tx);
    }

    if (rx != null && rx->dma == DMA_None && (*P_UART_Ctrl & C_UART_RX_INT_Enable) != 0)
    {
        while (budget-- != 0 && rx->done < rx->count &&
               (*P_UART_Status & C_UART_RX_FIFO_Empty_Flag) == 0)
        {
            data = *P_UART_Data;
            if ((data & SERIAL_UART_ERRORS) != 0 ||
                (*P_UART_RX_Status & SERIAL_UART_RX_ERRORS) != 0)
            {
                _uart_finish_rx(Serial_Err_RX);
                break;
            }
            rx->buf.rx[rx->done] = data & SERIAL_BYTE_MASK;
            ++rx->done;
        }
        if (uart_rx != null && rx->done == rx->count)
        {
            _uart_finish_rx(Serial_Done);
        }
        else if (uart_rx != null && (*P_UART_RX_Status & SERIAL_UART_RX_ERRORS) != 0)
        {
            _uart_finish_rx(Serial_Err_RX);
        }
    }

    _uart_sync_irq();
}