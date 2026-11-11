/*
 * GPCE500A 的 C 系统驱动，与 system.asm 一起参与当前工程构建。
 *
 * 对照资料：spec/MCU/GPCE500A_Programming Guide_v0.1(Spin Master LTD)(Draft).pdf
 * PDF 第 13~15 页：字地址、Cache RAM 和 SPI Bank；第 26~29 页：休眠与唤醒；
 * 第 86~90 页：音频寄存器；第 124~125 页：SPI Bank 和驱动能力。
 *
 * *P_xxx 表示读写硬件寄存器；本芯片 unsigned 为 16 位，unsigned long 为 32 位。
 * 本文件只保留 C 实现；CPU 指令、定长延时和 RAM 休眠代码
 * 由 system.asm 提供。普通逻辑只在本文件实现，不再在汇编中重复定义。
 * C 循环开销与原汇编不同，声音渐变和休眠唤醒仍需在实机上验证。
 */

#include "..\Include\spinmaster.h"

/*
 *@ brief    : Enable CPU IRQ.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : None.
 *@ theme    : Leave FIQ and peripheral interrupt masks unchanged.
 */
void system_enable_irq(void) { System_CPU_EnableIRQ(); }

/*
 *@ brief    : Initialize CPU, system clock and basic peripherals.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : None.
 *@ theme    : Select 81.92MHz, clear interrupt flags and reset GPIO A/B.
 */
void system_init_module(void)
{
    System_CPU_Initial();

    *P_System_Clock = C_SYSCLK_81920KHz | C_Sleep_IOSC32K_SLP_Off | C_IOSC8192M_Trim_from_Fuse;
    *P_IO_Ctrl = 0;
    *P_INT_Ctrl = 0;
    *P_INT2_Ctrl = 0;
    *P_FIQ_Sel = 0;
    *P_FIQ2_Sel = 0;

    /* 中断状态位是写 1 清除，不能改成写 0。 */
    *P_INT_Status = 0xFFFF;
    *P_INT2_Status = 0xFFFF;
    *P_TimeBase_Clear = C_TimeBase_Clear;

    *P_IOA_Attrib = 0;
    *P_IOA_Dir = 0;
    *P_IOA_Buffer = 0;
    *P_IOB_Attrib = 0;
    *P_IOB_Dir = 0;
    *P_IOB_Buffer = 0;
}

/*
 *@ brief    : Preserve the legacy channel-switch placeholder.
 *@ type     : public
 *@ argu in  : channel: unused legacy index.
 *@ argu out : None.
 *@ theme    : No hardware action; use adc_read_channel for actual conversions.
 */
void system_switch_channel(unsigned channel)
{
    /* 原汇编只有返回指令，并没有实现切换 ADC 通道。 */
    (void)channel;
}

/*
 *@ brief    : Clear the watchdog timer.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : None.
 *@ theme    : Write the hardware watchdog-clear key.
 */
void system_clear_watchdog(void) { *P_Watchdog_Clear = C_Watchdog_Clear; }

/*
 *@ brief    : Ramp an audio data register toward zero.
 *@ type     : private
 *@ argu in  : reg: audio channel data register.
 *@ argu out : Register reaches zero.
 *@ theme    : Preserve signed ramp direction, 0x40 steps and calibrated delays.
 */
static void _audio_ramp_zero(volatile unsigned *reg)
{
    unsigned sample = *reg & 0xFFC0;

    /*
     * 原汇编的 RampUp 和 RampDn 实际完全相同：以 0x40 为步长回到零。
     * bit15 为符号位；保留原行为，不根据函数名改写成另一种渐变。
     */
    if ((sample & 0x8000) != 0)
    {
        while (sample != 0)
        {
            SACM_Delay();
            sample = (sample + 0x0040) & 0xFFFF;
            *reg = sample;
        }
    }
    else
    {
        while (sample != 0)
        {
            SACM_Delay();
            sample -= 0x0040;
            *reg = sample;
        }
    }
}

/*
 *@ brief    : Ramp DAC channel 1 down to zero.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Channel 1 data becomes zero.
 *@ theme    : Preserve the original ramp behavior.
 */
void audio_lower_dac1(void) { _audio_ramp_zero(P_AUDIO_CH1_Data); }

/*
 *@ brief    : Ramp DAC channel 2 down to zero.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Channel 2 data becomes zero.
 *@ theme    : Preserve the original ramp behavior.
 */
void audio_lower_dac2(void) { _audio_ramp_zero(P_AUDIO_CH2_Data); }

/*
 *@ brief    : Apply the legacy ramp-up operation to DAC channel 1.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Channel 1 data becomes zero.
 *@ theme    : Legacy ramp-up and ramp-down intentionally share the same behavior.
 */
void audio_raise_dac1(void) { _audio_ramp_zero(P_AUDIO_CH1_Data); }

/*
 *@ brief    : Apply the legacy ramp-up operation to DAC channel 2.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Channel 2 data becomes zero.
 *@ theme    : Legacy ramp-up and ramp-down intentionally share the same behavior.
 */
void audio_raise_dac2(void) { _audio_ramp_zero(P_AUDIO_CH2_Data); }

/*
 *@ brief    : Check SPI Flash high-drive selection against supply voltage.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : May update the SPI drive flag.
 *@ theme    : Restore the original LVD configuration after the check.
 */
void spi_check_drive(void)
{
    unsigned lvd = *P_Reset_LVD_Ctrl & C_LVD_Ctrl;

    if ((*P_SPI_Auto_Setting & 0x8000) != 0)
    {
        *P_Reset_LVD_Ctrl = C_LVD_Ctrl_2D6V;
        System_LVD_Delay();
        if ((*P_Reset_LVD_Ctrl & 0x0800) == 0)
        {
            *P_SPI_Auto_Setting ^= 0x8000;
        }
    }
    else
    {
        *P_Reset_LVD_Ctrl = C_LVD_Ctrl_2D4V;
        System_LVD_Delay();
        if ((*P_Reset_LVD_Ctrl & 0x0800) != 0)
        {
            *P_SPI_Auto_Setting ^= 0x8000;
        }
    }

    /* 与原汇编一致：只恢复低压检测配置位，而非整个状态寄存器。 */
    *P_Reset_LVD_Ctrl = lvd;
    System_LVD_Delay();
}

/*
 *@ brief    : Disable the SPI Flash cache.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : None.
 *@ theme    : Wait for the hardware transition using the assembly delay.
 */
void cache_disable_module(void)
{
    *P_Cache_Ctrl = C_Cache_Disable;
    System_CacheWait();
}

/*
 *@ brief    : Clear and enable the SPI Flash cache.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : None.
 *@ theme    : Wait for cache clear to complete before enabling.
 */
void cache_enable_module(void)
{
    cache_disable_module();
    *P_Cache_Ctrl = C_Cache_Clear;
    System_CacheWait();

    while ((*P_Cache_Ctrl & C_Cache_Clear) != 0)
    {
        /* 等待硬件完成清除。 */
    }

    *P_Cache_Ctrl = C_Cache_Enable;
}

/*
 *@ brief    : Read one word through the SPI Flash bank window.
 *@ type     : public
 *@ argu in  : addr_lo/addr_hi: window word address.
 *@ argu out : Flash word.
 *@ theme    : Preserve legacy bank-selection side effects.
 */
unsigned spi_read_word(unsigned addr_lo, unsigned addr_hi)
{
    unsigned long addr;

    *P_SPI_Bank = addr_hi >> 4;
    addr = ((unsigned long)(0x30 + (addr_hi & 0x000F)) << 16) | addr_lo;

    /*
     * 对应 DS = 0x30 + (高字 & 0x0F)，再读取 D:[低字]。
     * 原单字读取虽然备份 Bank，却没有恢复；这里保留这个副作用。
     * 参数沿用原函数的窗口字地址，不是普通文件的字节偏移。
     */
    return *(volatile unsigned *)addr;
}

/*
 *@ brief    : Copy words from the SPI Flash window to RAM.
 *@ type     : public
 *@ argu in  : buf: RAM word address; count: words; addr_lo/addr_hi: Flash word address.
 *@ argu out : Destination buffer receives data; original SPI bank is restored.
 *@ theme    : Preserve address wrapping and legacy count=0 meaning 65536 words.
 */
void spi_read_words(unsigned buf, unsigned count, unsigned addr_lo, unsigned addr_hi)
{
    unsigned bank = *P_SPI_Bank;
    unsigned seg = 0x30 + (addr_hi & 0x000F);
    unsigned long addr;
    unsigned value;

    *P_SPI_Bank = addr_hi >> 4;

    /*
     * 原循环先读再减计数，wordCount == 0 时会读 65536 个字，而不是零个。
     * 原目的地址只有 16 位；这里显式保留地址回绕，避免变成另一种接口。
     */
    do
    {
        addr = ((unsigned long)seg << 16) | addr_lo;
        value = *(volatile unsigned *)addr;
        *(unsigned *)((unsigned long)buf) = value;
        buf = (buf + 1) & 0xFFFF;
        addr_lo = (addr_lo + 1) & 0xFFFF;
        count = (count - 1) & 0xFFFF;
        if (count == 0)
        {
            break;
        }

        if (addr_lo == 0)
        {
            seg = (seg + 1) & 0x003F;
            if (seg == 0)
            {
                *P_SPI_Bank += 1;
                seg = 0x30;
            }
        }
    } while (1);

    *P_SPI_Bank = bank;
}

/*
 *@ brief    : Enter sleep using the RAM-resident system routine.
 *@ type     : public
 *@ argu in  : rtc: nonzero keeps the RTC clock active.
 *@ argu out : Returns after wakeup.
 *@ theme    : Preserve peripheral shutdown, RAM copy and wakeup behavior.
 */
void system_enter_sleep(unsigned rtc)
{
    if (rtc != 0)
    {
        *P_System_Clock |= C_Sleep_IOSC32K_SLP_Work;
    }
    else
    {
        *P_System_Clock &= ~C_Sleep_IOSC32K_SLP_Work;
    }

    System_DisableInterrupts();

    *P_SPI2_Ctrl = 0;
    *P_IOA_Attrib = 0;
    *P_IOA_Dir = 0;
    *P_IOA_Buffer = 0;
    *P_IOB_Attrib = 0;
    *P_IOB_Dir = 0;
    *P_IOB_Buffer = 0;
    *P_IOC_Attrib = 0;
    *P_IOC_Dir = 0;
    *P_IOC_Buffer = 0;
    *P_CMPADC_Ctrl0 = 0;
    *P_PGA_Ctrl = 0;
    *P_UART_Ctrl = 0;
    *P_CTS_Ctrl0 = 0;

    audio_lower_dac1();
    audio_lower_dac2();
    *P_AUDIO_Ctrl1 &= ~(C_AUDIO_PWMIP_Enable | C_AUDIO_PWM_Enable);
    *P_AUDIO_Ctrl3 &= ~C_RDAC_Enable;
    *P_SPU_Clock_Ctrl = 0;

    cache_disable_module();
    /* 此调用先复制休眠代码到 Cache RAM，Flash 关闭期间只在 RAM 中执行。 */
    System_RunSleepCodeFromRAM();
    cache_enable_module();
    System_WakeupWait();

    /* 原函数唤醒后也没有重新开启中断或恢复其他外设。 */
}
