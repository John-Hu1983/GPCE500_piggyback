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

#include "GPCE36_CE5.h"
#include "System.h"

#define C_DebounceCnt       0x0080

/* 对应原汇编的 RAM 变量；计数单位是 ServiceLoop 的调用次数，不是毫秒。 */
static unsigned R_DebounceReg;
static unsigned R_DebounceCnt;
static unsigned R_KeyBuf;
static unsigned R_KeyStrobe;

void System_EnableIRQ(void)
{
    System_CPU_EnableIRQ();
}

void System_Initial(void)
{
    System_CPU_Initial();

    *P_System_Clock = C_SYSCLK_81920KHz |
                      C_Sleep_IOSC32K_SLP_Off |
                      C_IOSC8192M_Trim_from_Fuse;
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

void SP_SwitchChannel(unsigned ADC_Channel)
{
    /* 原汇编只有返回指令，并没有实现切换 ADC 通道。 */
    (void)ADC_Channel;
}

void WatchdogClear(void)
{
    *P_Watchdog_Clear = C_Watchdog_Clear;
}

static void Key_DebounceCnt_Down(void)
{
    if (R_DebounceCnt != 0) {
        --R_DebounceCnt;
    }
}

static void Key_Scan_ServiceLoop(void)
{
    unsigned sample = *P_IOA_Data & 0x00FF;
    unsigned previousSample = R_DebounceReg;
    unsigned previousKeys;

    R_DebounceReg = sample;
    if (previousSample != sample) {
        R_DebounceCnt = C_DebounceCnt;
        return;
    }

    if (R_DebounceCnt != 0) {
        return;
    }

    previousKeys = R_KeyBuf;
    R_KeyBuf = sample;

    /* 只记录由低到高的新按下；多个按键事件按位累积，读取后才清空。 */
    R_KeyStrobe |= (previousKeys ^ 0xFFFF) & R_KeyBuf;
}

void System_ServiceLoop(void)
{
    Key_DebounceCnt_Down();
    Key_Scan_ServiceLoop();
    WatchdogClear();
}

unsigned SP_GetCh(void)
{
    unsigned keys = R_KeyStrobe;

    R_KeyStrobe = 0;
    return keys;
}

static void SP_RampDACToZero(volatile unsigned *dataRegister)
{
    unsigned sample = *dataRegister & 0xFFC0;

    /*
     * 原汇编的 RampUp 和 RampDn 实际完全相同：以 0x40 为步长回到零。
     * bit15 为符号位；保留原行为，不根据函数名改写成另一种渐变。
     */
    if ((sample & 0x8000) != 0) {
        while (sample != 0) {
            SACM_Delay();
            sample = (sample + 0x0040) & 0xFFFF;
            *dataRegister = sample;
        }
    } else {
        while (sample != 0) {
            SACM_Delay();
            sample -= 0x0040;
            *dataRegister = sample;
        }
    }
}

void SP_RampDnDAC1(void)
{
    SP_RampDACToZero(P_AUDIO_CH1_Data);
}

void SP_RampDnDAC2(void)
{
    SP_RampDACToZero(P_AUDIO_CH2_Data);
}

void SP_RampUpDAC1(void)
{
    SP_RampDACToZero(P_AUDIO_CH1_Data);
}

void SP_RampUpDAC2(void)
{
    SP_RampDACToZero(P_AUDIO_CH2_Data);
}

void SPIFCHiDrivingCheck(void)
{
    unsigned previousLVD = *P_Reset_LVD_Ctrl & C_LVD_Ctrl;

    if ((*P_SPI_Auto_Setting & 0x8000) != 0) {
        *P_Reset_LVD_Ctrl = C_LVD_Ctrl_2D6V;
        System_LVD_Delay();
        if ((*P_Reset_LVD_Ctrl & 0x0800) == 0) {
            *P_SPI_Auto_Setting ^= 0x8000;
        }
    } else {
        *P_Reset_LVD_Ctrl = C_LVD_Ctrl_2D4V;
        System_LVD_Delay();
        if ((*P_Reset_LVD_Ctrl & 0x0800) != 0) {
            *P_SPI_Auto_Setting ^= 0x8000;
        }
    }

    /* 与原汇编一致：只恢复低压检测配置位，而非整个状态寄存器。 */
    *P_Reset_LVD_Ctrl = previousLVD;
    System_LVD_Delay();
}

void Cache_Disable(void)
{
    *P_Cache_Ctrl = C_Cache_Disable;
    System_CacheWait();
}

void Cache_Enable(void)
{
    Cache_Disable();
    *P_Cache_Ctrl = C_Cache_Clear;
    System_CacheWait();

    while ((*P_Cache_Ctrl & C_Cache_Clear) != 0) {
        /* 等待硬件完成清除。 */
    }

    *P_Cache_Ctrl = C_Cache_Enable;
}

unsigned AutoModeReadAWords(unsigned addressLow, unsigned addressHigh)
{
    unsigned long cpuAddress;

    *P_SPI_Bank = addressHigh >> 4;
    cpuAddress = ((unsigned long)(0x30 + (addressHigh & 0x000F)) << 16) |
                 addressLow;

    /*
     * 对应 DS = 0x30 + (高字 & 0x0F)，再读取 D:[低字]。
     * 原单字读取虽然备份 Bank，却没有恢复；这里保留这个副作用。
     * 参数沿用原函数的窗口字地址，不是普通文件的字节偏移。
     */
    return *(volatile unsigned *)cpuAddress;
}

void AutoModeReadNWords(unsigned bufferAddress, unsigned wordCount,
                        unsigned addressLow, unsigned addressHigh)
{
    unsigned previousBank = *P_SPI_Bank;
    unsigned segment = 0x30 + (addressHigh & 0x000F);
    unsigned long cpuAddress;
    unsigned value;

    *P_SPI_Bank = addressHigh >> 4;

    /*
     * 原循环先读再减计数，wordCount == 0 时会读 65536 个字，而不是零个。
     * 原目的地址只有 16 位；这里显式保留地址回绕，避免变成另一种接口。
     */
    do {
        cpuAddress = ((unsigned long)segment << 16) | addressLow;
        value = *(volatile unsigned *)cpuAddress;
        *(unsigned *)((unsigned long)bufferAddress) = value;
        bufferAddress = (bufferAddress + 1) & 0xFFFF;
        addressLow = (addressLow + 1) & 0xFFFF;
        wordCount = (wordCount - 1) & 0xFFFF;
        if (wordCount == 0) {
            break;
        }

        if (addressLow == 0) {
            segment = (segment + 1) & 0x003F;
            if (segment == 0) {
                *P_SPI_Bank += 1;
                segment = 0x30;
            }
        }
    } while (1);

    *P_SPI_Bank = previousBank;
}

void GotoSleep(unsigned keepRTC)
{
    if (keepRTC != 0) {
        *P_System_Clock |= C_Sleep_IOSC32K_SLP_Work;
    } else {
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

    SP_RampDnDAC1();
    SP_RampDnDAC2();
    *P_AUDIO_Ctrl1 &= ~(C_AUDIO_PWMIP_Enable | C_AUDIO_PWM_Enable);
    *P_AUDIO_Ctrl3 &= ~C_RDAC_Enable;
    *P_SPU_Clock_Ctrl = 0;

    Cache_Disable();
    /* 此调用先复制休眠代码到 Cache RAM，Flash 关闭期间只在 RAM 中执行。 */
    System_RunSleepCodeFromRAM();
    Cache_Enable();
    System_WakeupWait();

    /* 原函数唤醒后也没有重新开启中断或恢复其他外设。 */
}
