#include "..\Include\spinmaster.h"

/*
 *@ brief    : Initialize SPI2 master pins and controller with the selected clock divider.
 *@ type     : public
 *@ argu in  : rate: SPI_Rate divider (Div2..Div128).
 *@ argu out : None.
 *@ theme    : Configure SPI2 on Port A or Port B according to SPI_PORT.
 */
void spi_init_serial(SPI_Rate rate)
{
#if SPI_PORT == SERIAL_PORT_A
    *P_IO_Ctrl &= ~C_SPI2_IOB;
    *P_IO_Swap &= ~(C_SPI2_IOA_Swap_Enable | C_SPI2_IOA_MOSI_Enable);
    *P_IOA_Attrib |= 0x000E;
    *P_IOA_Dir |= 0x000E;
    *P_IOA_Buffer |= 0x000E;
#else
    *P_IO_Swap &= ~(C_SPI2_IOB_Swap_Enable | C_SPI2_IOB_MOSI_Enable);
    *P_IOB_Attrib |= 0xF000;
    *P_IOB_Dir |= 0xF000;
    *P_IOB_Buffer |= 0xF000;
#endif

    *P_SPI2_Ctrl = C_SPI_Enable | C_MasterMode | C_SPI_Reset | C_SPI_CS_SPI |
                   C_SPI_Clock_Phase_Normal | C_SPI_Clock_Pol_Normal;
    while (*P_SPI2_Ctrl & C_SPI_Reset)
    {
        system_clear_watchdog();
    }
    *P_SPI2_Ctrl |= ((unsigned)rate & C_SPI_SCK_SEL);
    *P_SPI2_TX_Status = C_SPI_TX_INT_Flag | C_SPI_TX_INT_DIS | C_SPI_TX_FIFO_Level_0;
    *P_SPI2_RX_Status = C_SPI_RX_INT_Flag | C_SPI_RX_INT_DIS | C_SPI_RX_FIFO_Level_1;
#if SPI_PORT == SERIAL_PORT_B
    *P_IO_Ctrl |= C_SPI2_IOB;
#endif
}

/*
 *@ brief    : Enable SPI2 receive interrupt on IRQ3.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : SPI2 RX interrupt enabled in Smart FIFO mode.
 *@ theme    : Route RX FIFO level 1 interrupt to IRQ3.
 */
void spi_open_isr(void)
{
    *P_SPI2_TX_Status = C_SPI_TX_INT_Flag | C_SPI_TX_INT_DIS | C_SPI_TX_FIFO_Level_0;
    *P_SPI2_RX_Status = C_SPI_RX_INT_Flag | C_SPI_RX_INT_EN | C_SPI_RX_FIFO_Level_1;
    *P_SPI2_Misc |= C_SPI_FIFO_Smart_Mode;
    *P_FIQ_Sel &= ~C_IRQ3_SPI2;
    *P_INT_Status = C_IRQ3_SPI2;
    *P_INT_Ctrl |= C_IRQ3_SPI2;
    system_enable_irq();
}

/*
 *@ brief    : Disable SPI2 receive interrupt on IRQ3.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : SPI2 RX interrupt masked.
 *@ theme    : Preserve other IRQ3 sources.
 */
void spi_close_isr(void)
{
    *P_SPI2_RX_Status = C_SPI_RX_INT_Flag | C_SPI_RX_INT_DIS | C_SPI_RX_FIFO_Level_1;
    *P_INT_Ctrl &= ~C_IRQ3_SPI2;
    *P_INT_Status = C_IRQ3_SPI2;
}

/*
 *@ brief    : Write one byte to SPI2 and wait until transfer completes.
 *@ type     : public
 *@ argu in  : data: byte to transmit.
 *@ argu out : None.
 *@ theme    : Single-byte blocking transmit.
 */
void spi_write_byte(u8 data)
{
    *P_SPI2_TX_Data = data & 0x00FFU;
    SPI_SEND_BUSY;
    *P_SPI2_Misc = 0;
}

/*
 *@ brief    : Clock one dummy byte and read one byte from SPI2.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Received 8-bit value.
 *@ theme    : Clear stale RX FIFO entries before clocking a new byte.
 */
u8 spi_read_byte(void)
{
    u8 rd = 0;

    while (*P_SPI2_Misc & C_SPI_RX_NonEmpty_Flag)
    {
        rd = (u8)*P_SPI2_RX_Data;
    }
    *P_SPI2_TX_Data = 0x00U;
    SPI_SEND_BUSY;
    SPI_READ_BUSY;
    while (*P_SPI2_Misc & C_SPI_RX_NonEmpty_Flag)
    {
        rd = (u8)*P_SPI2_RX_Data;
    }
    *P_SPI2_Misc = 0;
    return rd;
}

/*
 *@ brief    : Write a buffer of bytes over SPI2 FIFO.
 *@ type     : public
 *@ argu in  : buf: source byte buffer; len: number of bytes.
 *@ argu out : None.
 *@ theme    : Stream through TX FIFO while servicing watchdog.
 */
void spi_write_buf(const u8 *buf, u16 len)
{
    u16 sent = 0;

    if (buf == null || len == 0U)
    {
        return;
    }
    while (sent < len)
    {
        system_clear_watchdog();
        if (*P_SPI2_Misc & C_SPI_TX_NonFull_Flag)
        {
            *P_SPI2_TX_Data = buf[sent] & 0x00FFU;
            sent++;
        }
    }
    SPI_SEND_BUSY;
    *P_SPI2_Misc = 0;
}

/*
 *@ brief    : Read bytes from SPI2 into a buffer.
 *@ type     : public
 *@ argu in  : buf: destination buffer; len: maximum bytes to read.
 *@ argu out : Number of bytes read.
 *@ theme    : Drain available RX FIFO data or clock len bytes when empty.
 */
u16 spi_read_buf(u8 *buf, u16 len)
{
    u16 i = 0;

    if (buf == null || len == 0U)
    {
        return 0U;
    }
    if (*P_SPI2_Misc & C_SPI_RX_NonEmpty_Flag)
    {
        while ((*P_SPI2_Misc & C_SPI_RX_NonEmpty_Flag) && i < len)
        {
            buf[i++] = (u8)(*P_SPI2_RX_Data & 0x00FFU);
        }
        return i;
    }
    for (i = 0; i < len; i++)
    {
        system_clear_watchdog();
        *P_SPI2_TX_Data = 0x00U;
        SPI_SEND_BUSY;
        SPI_READ_BUSY;
        buf[i] = (u8)(*P_SPI2_RX_Data & 0x00FFU);
    }
    *P_SPI2_Misc = 0;
    return len;
}

/*
 *@ brief    : Transmit a buffer over SPI2 using DMA1.
 *@ type     : public
 *@ argu in  : buf: source word buffer; len: number of bytes/words.
 *@ argu out : None.
 *@ theme    : Configure DMA1 for SPI2 TX and wait for completion.
 */
void spi_transmit_dma(const u8 *buf, u16 len)
{
    if (buf == null || len == 0U)
    {
        return;
    }
    *P_DMA_INTSTS |= C_DMA1_TX_Done_Flag;
    *P_DMA1_CTRL0 = C_DMA_Request_SPI2 | C_DMA_Cont_Disable | C_DMA_Burst_Disable |
                    C_DMA_Src_Inc_Enable | C_DMA_Dest_Inc_Disable | C_DMA_Circular_Disable |
                    C_DMA_Error_INT_Disable | C_DMA_Done_INT_Disable;
    *P_DMA1_CTRL1 = C_DMA_Double_Buf_Disable | C_DMA_Stop_WaitComp;
    *P_DMA1_DTN = len;
    *P_DMA1_SRCADR = (u32)buf;
    *P_DMA1_DSTADR = (u32)P_SPI2_TX_Data;

    *P_DMA1_CTRL0 |= C_DMA_Enable;
    *P_SPI2_Man_Ctrl |= C_SPI_TX_DMA_Enable;

    while (*P_DMA1_CTRL1 & C_DMA_Busy_Flag)
    {
        system_clear_watchdog();
    }
    while (*P_SPI2_Misc & C_SPI_Busy_Flag)
    {
        system_clear_watchdog();
    }
    *P_SPI2_Man_Ctrl &= ~C_SPI_TX_DMA_Enable;
    *P_DMA1_CTRL0 = 0;
}

u8 spi_TxFIFO_used(void) { return (*P_SPI2_TX_Status & 0x0F); }

u8 spi_TxFIFO_free(void) { return (8 - spi_TxFIFO_used()); }

bool is_spi_dma_free(void)
{
    if ((*P_DMA1_CTRL1 & C_DMA_Busy_Flag) || (*P_SPI2_Misc & C_SPI_Busy_Flag))
    {
        return false;
    }
    return true;
}

void spi_service_irq(void)
{
    if (*P_INT_Status & C_IRQ3_SPI2)
    {
        *P_INT_Status = C_IRQ3_SPI2;
    }
    if (*P_DMA_INTSTS & C_DMA1_TX_Done_Flag)
    {
        *P_DMA_INTSTS |= C_DMA1_TX_Done_Flag;
        *P_SPI2_Man_Ctrl &= ~C_SPI_TX_DMA_Enable;
        *P_DMA1_CTRL0 = 0;
    }
}

