#ifndef _HAL_SPI_H_
#define _HAL_SPI_H_

typedef enum
{
    SPI_Mode_0 = 0x0000,
    SPI_Mode_1 = 0x0020,
    SPI_Mode_2 = 0x0010,
    SPI_Mode_3 = 0x0030
} SPI_Mode;

typedef struct
{
    const unsigned *tx;
    unsigned *rx;
    unsigned count;
    volatile unsigned tx_done;
    volatile unsigned rx_done;
    volatile Serial_Result state;
    DMA_Channel tx_dma;
    DMA_Channel rx_dma;
    unsigned pad;
    unsigned discard;
} SPI_Job;

/*
 * SPI2 only: 8-bit, single-I/O master. SPI1/program Flash remains untouched.
 * SPI_PORT A: CLK1/MISO2/MOSI3; B: CLK13/MISO14/MOSI15.
 * init accepts maximum SCK Hz and mode0..3. Choose the fastest Fsys/2..128
 * not exceeding hz; below Fsys/128 is rejected. read_rate reports actual SCK.
 * This driver never initializes or drives any external CS pin. A0/B12 remain caller-owned GPIO.
 * The peripheral driver chooses its own CS pin, frames commands/data and owns the bus
 * from CS assertion to deassertion; keep other devices deselected throughout that frame.
 * Each call transfers only count bytes, not an entire CS frame. One CS LOW-to-HIGH frame
 * may contain multiple calls with DIFFERENT counts and modes, without reinitializing SPI.
 * On success, wait for Serial_Done before the next segment or changing CS/DC/buffers:
 * manual returns Done; ISR/DMA require job->state == Done via spi_poll_transfer.
 * Success, failure, abort and disable never change external CS; caller handles recovery.
 * Configure/change clock or mode only with all peripheral CS pins inactive.
 * Select manual/ISR/DMA by API. send/recv wrappers use transfer APIs.
 * null TX clocks 0xFF; null RX discards data. At least one buffer is required.
 * All paths drain RX, including transmit-only. SPI ISR queues one byte at a time;
 * use DMA for high rates. Duplex DMA reserves two DIFFERENT free DMA channels,
 * even for TX-only/RX-only, using the caller job's pad/discard words.
 * Check all returned results and job->state; call spi_poll_transfer in the main loop.
 * Async jobs/buffers must persist until completion/error/abort; DMA job/scratch
 * and buffers must live in ordinary working RAM. No internal payload buffer.
 * Serialize submission/reinit/abort calls; release conflicting pin peripherals.
 * Initialize after system_init_module/spu_init_audio; disable before sleep.
 * Enabled UART on the same port is rejected rather than silently disconnected.
 */
Serial_Result spi_init_module(unsigned long hz, SPI_Mode mode);
unsigned long spi_read_rate(void);
void spi_disable_module(void);
Serial_Result spi_transfer_manual(const unsigned *tx, unsigned *rx, unsigned count);
Serial_Result spi_send_manual(const unsigned *buf, unsigned count);
Serial_Result spi_recv_manual(unsigned *buf, unsigned count);
Serial_Result spi_transfer_isr(SPI_Job *job, const unsigned *tx, unsigned *rx, unsigned count);
Serial_Result spi_send_isr(SPI_Job *job, const unsigned *buf, unsigned count);
Serial_Result spi_recv_isr(SPI_Job *job, unsigned *buf, unsigned count);
Serial_Result spi_transfer_dma(SPI_Job *job, const unsigned *tx, unsigned *rx, unsigned count,
                               DMA_Channel tx_dma, DMA_Channel rx_dma);
Serial_Result spi_send_dma(SPI_Job *job, const unsigned *buf, unsigned count, DMA_Channel tx_dma,
                           DMA_Channel rx_dma);
Serial_Result spi_recv_dma(SPI_Job *job, unsigned *buf, unsigned count, DMA_Channel tx_dma,
                           DMA_Channel rx_dma);
void spi_poll_transfer(void);
void spi_abort_transfer(void);
void spi_service_irq(void);

#endif // _HAL_SPI_H_
