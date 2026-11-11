#ifndef _HAL_SPI_H_
#define _HAL_SPI_H_

#define SPI_SEND_BUSY while (*P_SPI2_Misc & C_SPI_Busy_Flag)
#define SPI_READ_BUSY while (!(*P_SPI2_RX_Status & 0x0F))
#define WAIT_SPI_TX_NONFULL while (!(*P_SPI2_Misc & C_SPI_TX_NonFull_Flag))

typedef enum
{
    SPI_Rate_Div2 = C_SPI_SCK_SYSCLK_Div_2,
    SPI_Rate_Div4 = C_SPI_SCK_SYSCLK_Div_4,
    SPI_Rate_Div8 = C_SPI_SCK_SYSCLK_Div_8,
    SPI_Rate_Div16 = C_SPI_SCK_SYSCLK_Div_16,
    SPI_Rate_Div32 = C_SPI_SCK_SYSCLK_Div_32,
    SPI_Rate_Div64 = C_SPI_SCK_SYSCLK_Div_64,
    SPI_Rate_Div128 = C_SPI_SCK_SYSCLK_Div_128,
} SPI_Rate;

void spi_init_serial(SPI_Rate rate);
void spi_open_isr(void);
void spi_close_isr(void);
u8 spi_read_byte(void);
void spi_write_byte(u8 data);
u16 spi_read_buf(u8 *buf, u16 len);
void spi_write_buf(const u8 *buf, u16 len);
void spi_transmit_dma(const u8 *buf, u16 len);
bool is_spi_dma_free(void);
u8 spi_TxFIFO_used(void);
u8 spi_TxFIFO_free(void);
void spi_service_irq(void);

#endif // _HAL_SPI_H_


