#ifndef _HAL_UART_H_
#define _HAL_UART_H_

#define uart_select_ioa() (*P_IO_Ctrl &= ~C_UART_IOB)
#define uart_select_iob() (*P_IO_Ctrl |= C_UART_IOB)

typedef enum
{
    UART_Rate_1200 = C_BaudRate_Setting_1200,
    UART_Rate_2400 = C_BaudRate_Setting_2400,
    UART_Rate_9600 = C_BaudRate_Setting_9600,
    UART_Rate_14400 = C_BaudRate_Setting_14400,
    UART_Rate_19200 = C_BaudRate_Setting_19200,
    UART_Rate_28800 = C_BaudRate_Setting_28800,
    UART_Rate_38400 = C_BaudRate_Setting_38400,
    UART_Rate_57600 = C_BaudRate_Setting_57600,
    UART_Rate_76800 = C_BaudRate_Setting_76800,
    UART_Rate_115200 = C_BaudRate_Setting_115200,
    UART_Rate_230400 = C_BaudRate_Setting_230400,
    UART_Rate_460800 = C_BaudRate_Setting_460800,
} UART_Rate;

void uart_init_serial(UART_Rate rate);
void uart_open_isr(void);
void uart_close_isr(void);
u8 uart_read_byte(void);
void uart_write_byte(u8 data);
u16 uart_read_buf(u8 *buf, u16 len);
void uart_write_buf(const u8 *buf, u16 len);
void uart_transmit_dma(const u8 *buf, u16 len);
bool is_uart_tx_dma_busy(void);
void uart_service_irq(void);

#endif // _HAL_UART_H_

