#ifndef _HAL_SPI_H_
#define _HAL_SPI_H_

#define SPI_SEND_BUSY while (*P_SPI2_Misc & C_SPI_Busy_Flag)
#define SPI_READ_BUSY while (!(*P_SPI2_RX_Status & 0xf))

void spi_init_port(void);
void spi_receive_isr_enable(void);
void spi_receive_isr_disable(void);
void spi_send_dma_preemptive(unsigned *buf, unsigned len);
void spi_receive_dma_preemptive(unsigned *buf, unsigned len);
void spi_open_dma_transmit(unsigned *buf, unsigned len);
bool is_spi_dma_free(void);
u8 spi_TxFIFO_used(void);
u8 spi_TxFIFO_free(void);
void spi_service_irq(void);

#endif // _HAL_SPI_H_

