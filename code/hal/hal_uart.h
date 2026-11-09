#ifndef _HAL_UART_H_
#define _HAL_UART_H_

#include "hal_serial.h"

typedef union
{
    const unsigned *tx;
    unsigned *rx;
} UART_Buffer;

typedef struct
{
    UART_Buffer buf;
    unsigned count;
    volatile unsigned done;
    volatile Serial_Result state;
    DMA_Channel dma;
} UART_Job;

/*
 * UART 8N1; UART_PORT selects A(TX3/RX2) or B(TX15/RX14).
 * Initialize after system_init_module and the startup audio setup.
 * init accepts baud in bps; read_rate returns the actual nominal rate.
 * Choose send/recv_manual, send/recv_isr or send/recv_dma per request.
 * Manual is blocking with bounded polling; ISR/DMA return Serial_Ok after submission.
 * Check every returned Serial_Result. Async results are published in job->state;
 * job->done counts valid RX words / TX words accepted, not necessarily on the wire.
 * Call uart_poll_transfer in the main loop: TX becomes Done only after the last stop bit.
 * Async RX waits for exactly count bytes, not a packet delimiter; abort cancels it.
 * User owns jobs/buffers until Done/error/Aborted. Do not reuse a Busy job.
 * Buffers contain unsigned words, one low-8-bit byte per word, not packed byte storage.
 * DMA requires ordinary working RAM and a free chosen DMA channel.
 * Only two job pointers are stored by this driver; no internal payload buffers.
 * Serialize submission/reinit/abort calls; do not submit from callbacks or nested ISRs.
 * Release conflicting peripherals first; an enabled SPI2 on the same port is rejected.
 */
Serial_Result uart_init_module(unsigned long baud);
unsigned long uart_read_rate(void);
void uart_disable_module(void);
Serial_Result uart_send_manual(const unsigned *buf, unsigned count);
Serial_Result uart_recv_manual(unsigned *buf, unsigned count);
Serial_Result uart_send_isr(UART_Job *job, const unsigned *buf, unsigned count);
Serial_Result uart_recv_isr(UART_Job *job, unsigned *buf, unsigned count);
Serial_Result uart_send_dma(UART_Job *job, const unsigned *buf, unsigned count,
                            DMA_Channel channel);
Serial_Result uart_recv_dma(UART_Job *job, unsigned *buf, unsigned count, DMA_Channel channel);
void uart_poll_transfer(void);
void uart_abort_transfer(void);
void uart_service_irq(void);

#endif // _HAL_UART_H_
