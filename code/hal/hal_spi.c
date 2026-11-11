#include "..\Include\spinmaster.h"

void spi_init_port(void)
{
    *P_IOB_Attrib |= 0xF000;
    *P_IOB_Dir |= 0xF000;
    *P_IOB_Buffer |= 0xF000;

    *P_SPI2_Ctrl = C_SPI_Enable | C_MasterMode | C_SPI_Reset | C_SPI_CS_SPI |
                   C_SPI_Clock_Phase_Normal | C_SPI_Clock_Pol_Normal;
    while (*P_SPI2_Ctrl & C_SPI_Reset)
    {
        system_clear_watchdog();
    }
    *P_SPI2_Ctrl |= C_SPI_SCK_SYSCLK_Div_2;
    *P_SPI2_TX_Status = C_SPI_TX_INT_Flag | C_SPI_TX_INT_DIS | C_SPI_TX_FIFO_Level_0;
    *P_SPI2_RX_Status = C_SPI_RX_INT_Flag | C_SPI_RX_INT_DIS | C_SPI_RX_FIFO_Level_1;
    *P_IO_Ctrl |= C_SPI2_IOB;
}

u8 spi_TxFIFO_used(void) { return (*P_SPI2_TX_Status & 0x0F); }

u8 spi_TxFIFO_free(void) { return (8 - spi_TxFIFO_used()); }

void spi_receive_isr_enable(void)
{
    *P_SPI2_TX_Status = C_SPI_TX_INT_Flag | C_SPI_TX_INT_DIS | C_SPI_TX_FIFO_Level_0;
    *P_SPI2_RX_Status = C_SPI_RX_INT_Flag | C_SPI_RX_INT_EN | C_SPI_RX_FIFO_Level_1;
    *P_SPI2_Misc |= C_SPI_FIFO_Smart_Mode;
    *P_INT_Status |= C_IRQ3_SPI2;
    *P_INT_Ctrl |= C_IRQ3_SPI2;
    system_enable_irq();
}

void spi_receive_isr_disable(void)
{
    *P_SPI2_RX_Status = C_SPI_RX_INT_Flag | C_SPI_RX_INT_DIS | C_SPI_RX_FIFO_Level_1;
    *P_INT_Ctrl &= ~C_IRQ3_SPI2;
}

void spi_open_dma_transmit(unsigned *buf, unsigned len)
{
    *P_DMA_INTSTS |= C_DMA1_TX_Done_Flag;
    *P_DMA1_CTRL0 = C_DMA_Request_SPI2 | C_DMA_Cont_Disable | C_DMA_Burst_Disable |
                    C_DMA_Src_Inc_Enable | C_DMA_Dest_Inc_Disable | C_DMA_Circular_Disable |
                    C_DMA_Error_INT_Disable | C_DMA_Done_INT_Enable;
    *P_DMA1_CTRL1 = C_DMA_Double_Buf_Disable | C_DMA_Stop_WaitComp;
    *P_DMA1_DTN = len;
    *P_DMA1_SRCADR = (u32)buf;
    *P_DMA1_DSTADR = (u32)P_SPI2_TX_Data;

    *P_DMA1_CTRL0 |= C_DMA_Enable;
    *P_SPI2_Man_Ctrl |= C_SPI_TX_DMA_Enable;
}

bool is_spi_dma_free(void)
{
    if ((*P_DMA1_CTRL1 & C_DMA_Busy_Flag) || (*P_SPI2_Misc & C_SPI_Busy_Flag))
    {
        return false;
    }
    return true;
}

void spi_send_dma_preemptive(unsigned *buf, unsigned len)
{
    spi_open_dma_transmit(buf, len);

    while (*P_DMA1_CTRL1 & C_DMA_Busy_Flag)
        ;
    while (*P_SPI2_Misc & C_SPI_Busy_Flag)
        ;
}

void spi_receive_dma_preemptive(unsigned *buf, unsigned len)
{
    *P_DMA_INTSTS |= C_DMA1_TX_Done_Flag;
    *P_DMA1_CTRL0 = C_DMA_Request_SPI2 | C_DMA_Cont_Disable | C_DMA_Burst_Disable |
                    C_DMA_Src_Inc_Disable | C_DMA_Dest_Inc_Enable | C_DMA_Circular_Disable |
                    C_DMA_Error_INT_Disable | C_DMA_Done_INT_Disable;
    *P_DMA1_CTRL1 = C_DMA_Double_Buf_Disable | C_DMA_Stop_WaitComp;
    *P_DMA1_DTN = len;
    *P_DMA1_SRCADR = (u32)P_SPI2_RX_Data;
    *P_DMA1_DSTADR = (u32)buf;

    *P_DMA1_CTRL0 |= C_DMA_Enable;
    *P_SPI2_Ctrl |= C_SPI_RX_DMA_Enable;

    while (*P_DMA1_CTRL1 & C_DMA_Busy_Flag)
    {
        system_clear_watchdog();
    }
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
