// CPU primitives, calibrated delays and RAM sleep for gpce500a.c.
.include GPCE36_CE5.inc

.external _spi_read_words

.public _System_CPU_Initial
.public _System_CPU_EnableIRQ
.public _System_DisableInterrupts
.public _System_CacheWait
.public _System_WakeupWait
.public _System_LVD_Delay
.public _System_RunSleepCodeFromRAM
.public _SACM_Delay

.CODE
_System_CPU_Initial: .proc
    int off
    fir_mov off
    retf
    .endp

_System_CPU_EnableIRQ: .proc
    irq on
    retf
    .endp

_System_DisableInterrupts: .proc
    int off
    retf
    .endp

_System_CacheWait: .proc
    nop
    nop
    nop
    nop
    nop
    retf
    .endp

_System_WakeupWait: .proc
    nop
    nop
    nop
    nop
    retf
    .endp

_System_LVD_Delay: .proc
    R3 = 10
?L_LVDDelay:
    R3 -= 1
    jne ?L_LVDDelay
    retf
    .endp

_SACM_Delay: .proc
    push R2 to [SP]
    R2 = 80
?L_RampDelay:
    R2 -= 1
    jnz ?L_RampDelay
    pop R2 from [SP]
    retf
    .endp

// The linker stores this section at Flash 0x10000, linked for RAM 0x1000.
// Copy and return while Flash is available; only the RAM procedure powers it down.
_System_RunSleepCodeFromRAM: .proc
    push R1, R5 to [SP]
    push SR to [SP]
    R1 = 0x1000
    R2 = F_SleepCodeRAMEnd - F_SleepCodeRAMStr
    R3 = 0
    R4 = 1
    push R4 to [SP]
    push R3 to [SP]
    push R2 to [SP]
    push R1 to [SP]
    call _spi_read_words
    SP += 4
    call F_SleepCodeRAMStr
    pop SR from [SP]
    pop R1, R5 from [SP]
    retf
    .endp

// No C calls or Flash data accesses are allowed inside this section.
SleepCode_RAMCode_SEC: .section .CODE
F_SleepCodeRAMStr: .proc
    R1 = [P_SPI_Ctrl]
    R4 = R1
    R4 &= C_SPI_MultiIO_Sel
    R1 &= ~(C_SPI_Auto_Enable + C_SPI_CS_SPI + C_SPI_MultiIO_Sel)
    R1 |= 0x0080
    [P_SPI_Ctrl] = R1
    clrb [P_SPI_Man_Ctrl], 1
    R1 = 0xFF
    [P_SPI_TX_Data] = R1
    [P_SPI_TX_Data] = R1
?L_Check_TXIF_1:
    R1 = [P_SPI_Misc]
    test R1, C_SPI_Busy_Flag
    jnz ?L_Check_TXIF_1
    test R1, C_SPI_RX_NonEmpty_Flag
    jz ?L_Check_TXIF_1
    nop
    R1 = [P_SPI_RX_Data]
    R1 = [P_SPI_RX_Data]
    setb [P_SPI_Man_Ctrl], 1
    clrb [P_SPI_Man_Ctrl], 1
    R1 = 0xB9
    [P_SPI_TX_Data] = R1
?L_Check_TXIF_2:
    R1 = [P_SPI_Misc]
    test R1, C_SPI_Busy_Flag
    jnz ?L_Check_TXIF_2
    test R1, C_SPI_RX_NonEmpty_Flag
    jz ?L_Check_TXIF_2
    nop
    R1 = [P_SPI_RX_Data]
    setb [P_SPI_Man_Ctrl], 1
    R1 = [P_SPI_Ctrl]
    R1 &= ~C_SPI_Enable
    [P_SPI_Ctrl] = R1
    R1 = [P_IOA_Data]
    R1 = [P_IOA_Attrib]
    R1 = [P_IOB_Data]
    R1 = [P_IOB_Attrib]
    R1 = [P_IOC_Data]
    R1 = [P_IOC_Attrib]
    R1 = C_IRQ4_KEY
    [P_INT_Ctrl] = R1
    R1 = 0xFF00
    [P_IOA_WakeUp_Mask] = R1
    R1 = 0xFFFF
    [P_IOB_WakeUp_Mask] = R1
    [P_IOC_WakeUp_Mask] = R1
    nop
    nop
    R1 = 0xFFFF
    [P_INT_Status] = R1
    [P_INT2_Status] = R1
    R1 = C_System_Sleep
    [P_System_Sleep] = R1
    nop
    nop
    nop
    nop
    R1 = 0
    [P_INT_Ctrl] = R1
    [P_INT2_Ctrl] = R1
    R1 = 0xFFFF
    [P_INT_Status] = R1
    [P_INT2_Status] = R1
    R1 = [P_SPI_Ctrl]
    R1 |= (C_SPI_Enable + C_SPI_Reset)
    [P_SPI_Ctrl] = R1
    clrb [P_SPI_Man_Ctrl], 1
    R1 = 0xAB
    [P_SPI_TX_Data] = R1
?L_Check_TXIF_Wakeup:
    R1 = [P_SPI_Misc]
    test R1, C_SPI_Busy_Flag
    jnz ?L_Check_TXIF_Wakeup
    test R1, C_SPI_RX_NonEmpty_Flag
    jz ?L_Check_TXIF_Wakeup
    R1 = [P_SPI_RX_Data]
    setb [P_SPI_Man_Ctrl], 1
    R1 = 4096
?_Loop_0:
    R1 -= 1
    jnz ?_Loop_0
    R1 = [P_SPI_Auto_Ctrl]
    R3 = 0
    [P_SPI_Auto_Ctrl] = R3
    R3 = [P_SPI_Ctrl]
    R3 |= (C_SPI_Auto_Enable + C_SPI_CS_SPI)
    R3 |= R4
    [P_SPI_Ctrl] = R3
    [P_SPI_Auto_Ctrl] = R1
    retf
F_SleepCodeRAMEnd:
    .endp
