#include "..\Include\spinmaster.h"

/*
 *@ brief    : Initialize UART registers, IO mux, and baud rate.
 *@ type     : public
 *@ argu in  : rate: UART_Rate baud setting (e.g. UART_Rate_115200).
 *@ argu out : None.
 *@ theme    : Configure 8N1 with FIFO enabled on UART_PORT (Port A or Port B).
 */
void uart_init_serial(UART_Rate rate)
{
#if UART_PORT == SERIAL_PORT_A
    uart_select_ioa();
    *P_IO_Swap &= ~(C_UART_IOA_INV_Enable | C_UART_IOA_Swap_Enable);
    gpio_init_pattern(GPIOA, 2, GPIO_In_Float);
    gpio_init_pattern(GPIOA, 3, GPIO_Out_High);
#else
    uart_select_iob();
    *P_IO_Swap &= ~(C_UART_IOB_INV_Enable | C_UART_IOB_Swap_Enable);
    gpio_init_pattern(GPIOB, 14, GPIO_In_Float);
    gpio_init_pattern(GPIOB, 15, GPIO_Out_High);
#endif

    *P_UART_Ctrl = C_UART_Enable | C_UART_FIFO_Enable | C_UART_Reset | C_UART_Data_Length_8bit |
                   C_UART_Stop_Size_1bit | C_UART_Parity_Disable;
    (void)*P_UART_Ctrl;
    *P_UART_Ctrl = C_UART_Enable | C_UART_FIFO_Enable | C_UART_Data_Length_8bit |
                   C_UART_Stop_Size_1bit | C_UART_Parity_Disable;
    *P_UART_BaudRate = (unsigned)rate;
    *P_UART_RX_Status =
        C_Overrun_Error_Flag | C_Break_Error_Flag | C_Parity_Error_Flag | C_Frame_Error_Flag;
    *P_INT2_Status = C_IRQ7_UART;
}

/*
 *@ brief    : Enable UART receive interrupt on IRQ7.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : UART RX and RX-timeout interrupts enabled on IRQ7.
 *@ theme    : Enable IRQ7_UART without disturbing RTC sources on INT2.
 */
void uart_open_isr(void)
{
    *P_UART_RX_Status =
        C_Overrun_Error_Flag | C_Break_Error_Flag | C_Parity_Error_Flag | C_Frame_Error_Flag;
    *P_UART_Ctrl = (*P_UART_Ctrl & ~C_UART_Reset) | C_UART_RX_INT_Enable | C_UART_RX_Tout_INT_Enable;
    *P_FIQ2_Sel &= ~C_IRQ7_UART;
    *P_INT2_Status = C_IRQ7_UART;
    *P_INT2_Ctrl |= C_IRQ7_UART;
    system_enable_irq();
}

/*
 *@ brief    : Disable UART interrupts on IRQ7.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : UART interrupt sources masked.
 *@ theme    : Preserve RTC interrupt enables in P_INT2_Ctrl.
 */
void uart_close_isr(void)
{
    *P_UART_Ctrl &=
        ~(C_UART_RX_INT_Enable | C_UART_TX_INT_Enable | C_UART_RX_Tout_INT_Enable | C_UART_Reset);
    *P_INT2_Ctrl &= ~C_IRQ7_UART;
    *P_INT2_Status = C_IRQ7_UART;
}

/*
 *@ brief    : Write one byte to UART TX FIFO.
 *@ type     : public
 *@ argu in  : data: byte to transmit.
 *@ argu out : None.
 *@ theme    : Wait for TX FIFO space before writing.
 */
void uart_write_byte(u8 data)
{
    while (*P_UART_Status & C_UART_TX_FIFO_Full_Flag)
    {
        system_clear_watchdog();
    }
    *P_UART_Data = data & 0x00FFU;
}

/*
 *@ brief    : Read one byte from UART RX FIFO.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Received 8-bit value.
 *@ theme    : Wait until RX FIFO is non-empty and clear any sticky error flags.
 */
u8 uart_read_byte(void)
{
    u8 data;

    while (*P_UART_Status & C_UART_RX_FIFO_Empty_Flag)
    {
        system_clear_watchdog();
    }
    data = (u8)(*P_UART_Data & 0x00FFU);
    *P_UART_RX_Status =
        C_Overrun_Error_Flag | C_Break_Error_Flag | C_Parity_Error_Flag | C_Frame_Error_Flag;
    return data;
}

/*
 *@ brief    : Write a buffer of bytes over UART and wait until transmission finishes.
 *@ type     : public
 *@ argu in  : buf: source byte buffer; len: number of bytes.
 *@ argu out : None.
 *@ theme    : Stream through TX FIFO and wait for shifter idle.
 */
void uart_write_buf(const u8 *buf, u16 len)
{
    u16 i;

    if (buf == null || len == 0U)
    {
        return;
    }
    for (i = 0; i < len; i++)
    {
        uart_write_byte(buf[i]);
    }
    while (*P_UART_Status & C_UART_Busy_Flag)
    {
        system_clear_watchdog();
    }
}

/*
 *@ brief    : Read available bytes from UART RX FIFO into a buffer.
 *@ type     : public
 *@ argu in  : buf: destination buffer; len: maximum bytes to read.
 *@ argu out : Number of bytes read.
 *@ theme    : Drain available RX FIFO data or read len bytes when empty.
 */
u16 uart_read_buf(u8 *buf, u16 len)
{
    u16 i = 0;

    if (buf == null || len == 0U)
    {
        return 0U;
    }
    if ((*P_UART_Status & C_UART_RX_FIFO_Empty_Flag) == 0U)
    {
        while (((*P_UART_Status & C_UART_RX_FIFO_Empty_Flag) == 0U) && i < len)
        {
            buf[i++] = (u8)(*P_UART_Data & 0x00FFU);
        }
        *P_UART_RX_Status =
            C_Overrun_Error_Flag | C_Break_Error_Flag | C_Parity_Error_Flag | C_Frame_Error_Flag;
        return i;
    }
    for (i = 0; i < len; i++)
    {
        buf[i] = uart_read_byte();
    }
    return len;
}

/*
 *@ brief    : Check if UART TX DMA2 or UART engine is busy.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true when DMA2 or UART is busy; false otherwise.
 *@ theme    : Poll both DMA2 busy flag and UART busy flag.
 */
bool is_uart_tx_dma_busy(void)
{
    return ((*P_DMA2_CTRL1 & C_DMA_Busy_Flag) || (*P_UART_Status & C_UART_Busy_Flag)) ? true
                                                                                      : false;
}

/*
 *@ brief    : Transmit a buffer over UART via DMA2 and wait until complete.
 *@ type     : public
 *@ argu in  : buf: source word buffer; len: word count to send.
 *@ argu out : None.
 *@ theme    : Configure DMA2 for UART TX and block until shifter is idle.
 */
void uart_transmit_dma(const u8 *buf, u16 len)
{
    if (buf == null || len == 0U)
    {
        return;
    }
    *P_DMA_INTSTS |= C_DMA2_TX_Done_Flag;
    *P_DMA2_CTRL0 = C_DMA_Request_UART | C_DMA_Cont_Disable | C_DMA_Burst_Disable |
                    C_DMA_Src_Inc_Enable | C_DMA_Dest_Inc_Disable | C_DMA_Circular_Disable |
                    C_DMA_Error_INT_Disable | C_DMA_Done_INT_Disable;
    *P_DMA2_CTRL1 = C_DMA_Double_Buf_Disable | C_DMA_Stop_WaitComp;
    *P_DMA2_DTN = len;
    *P_DMA2_SRCADR = (u32)buf;
    *P_DMA2_DSTADR = (u32)P_UART_Data;

    *P_DMA2_CTRL0 |= C_DMA_Enable;
    *P_UART_Ctrl = (*P_UART_Ctrl & ~C_UART_Reset) | C_UART_TX_DMA_Enable;

    while (*P_DMA2_CTRL1 & C_DMA_Busy_Flag)
    {
        system_clear_watchdog();
    }
    while (*P_UART_Status & C_UART_Busy_Flag)
    {
        system_clear_watchdog();
    }
    *P_UART_Ctrl &= ~(C_UART_TX_DMA_Enable | C_UART_Reset);
    *P_DMA2_CTRL0 = 0;
}

/*
 *@ brief    : Service UART IRQ7 and DMA2 IRQ3 flags.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Clears pending UART / DMA2 interrupt flags.
 *@ theme    : Invoked by _IRQ3 and _IRQ7 vectors in isr.asm.
 */
void uart_service_irq(void)
{
    if (*P_DMA_INTSTS & C_DMA2_TX_Done_Flag)
    {
        *P_DMA_INTSTS |= C_DMA2_TX_Done_Flag;
        *P_UART_Ctrl &= ~(C_UART_TX_DMA_Enable | C_UART_Reset);
        *P_DMA2_CTRL0 = 0;
    }
    if (*P_INT2_Status & C_IRQ7_UART)
    {
        *P_INT2_Status = C_IRQ7_UART;
    }
}
