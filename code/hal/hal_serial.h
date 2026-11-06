#ifndef HAL_SERIAL_H
#define HAL_SERIAL_H

#include "hal_gpio.h"
#include "GPCE36_CE5.h"
#include "user_config.h"

#define SERIAL_OSC_HZ 81920000UL
#define SERIAL_RAM_END 0x1000UL
#define SERIAL_BYTE_MASK 0x00FFU
#define SERIAL_UART_ERRORS 0x0F00U
#define SERIAL_UART_RX_ERRORS 0x000FU
#define SERIAL_SPI_FILL 0x00FFU

#if defined(SUNPLUS) || defined(unSP) || defined(__CHAR_16__)
#define SERIAL_ADDR(ptr) ((unsigned long)(ptr))
#else
#define SERIAL_ADDR(ptr) ((uintptr_t)(ptr))
#endif

typedef enum
{
    Serial_Ok = 0,
    Serial_Busy,
    Serial_Done,
    Serial_Aborted,
    Serial_Err_Arg,
    Serial_Err_Rate,
    Serial_Err_Port,
    Serial_Err_Mode,
    Serial_Err_Timeout,
    Serial_Err_RX,
    Serial_Err_DMA
} Serial_Result;

typedef enum
{
    DMA_None = 0,
    DMA_Ch_1 = 1,
    DMA_Ch_2 = 2
} DMA_Channel;

typedef struct
{
    volatile unsigned ctrl0;
    volatile unsigned ctrl1;
    volatile unsigned count;
    volatile unsigned src;
    volatile unsigned reserved_src;
    volatile unsigned dst;
} Serial_DMA;

/* Shared register helpers, with no channel-owner table or static buffers.
 * DMA buffers use one 16-bit working-RAM word per serial byte (low eight bits).
 * DMA accepts working RAM below SERIAL_RAM_END, not Flash or active cache RAM.
 * All submission/reconfiguration calls must be serialized by the application.
 */
unsigned long serial_read_clock(void);
Serial_Result serial_check_bytes(const unsigned *buf, unsigned count);
Serial_Result serial_check_dma(DMA_Channel channel);
Serial_Result serial_check_memory(const unsigned *buf, unsigned count);
void serial_start_dma(DMA_Channel channel, unsigned request, const unsigned *src, unsigned *dst,
                      unsigned count, unsigned inc);
Serial_Result serial_read_dma(DMA_Channel channel);
void serial_stop_dma(DMA_Channel channel);

#endif
