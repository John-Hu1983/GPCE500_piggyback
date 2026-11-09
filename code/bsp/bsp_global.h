#ifndef _BSP_GLOBAL_H__
#define _BSP_GLOBAL_H__

#include "../Include/user_config.h"

/* High byte identifies the driver; low byte identifies the failing check. */
typedef enum
{
    BSP_Fault_GPIO_Port = 0x0101,
    BSP_Fault_GPIO_Pin = 0x0102,
    BSP_Fault_GPIO_Out = 0x0103,
    BSP_Fault_GPIO_Pattern = 0x0104,
    BSP_Fault_GPIO_Pull = 0x0105,
    BSP_Fault_GPIO_Input = 0x0106,
    BSP_Fault_ADC_State = 0x0201,
    BSP_Fault_ADC_Time = 0x0202,
    BSP_Fault_ADC_Wait = 0x0203,
    BSP_Fault_IOPWM_Ch = 0x0301,
    BSP_Fault_IOPWM_Clk = 0x0302,
    BSP_Fault_IOPWM_Duty = 0x0303,
    BSP_Fault_RTC_Freq = 0x0401,
    BSP_Fault_RTC_Handler = 0x0402,
    BSP_Fault_RTC_IRQ = 0x0403,
    BSP_Fault_DMA_Ch = 0x0501,
    BSP_Fault_DMA_Start = 0x0502,
    BSP_Fault_DMA_Stop = 0x0503,
    BSP_Fault_SWT_Node = 0x0601,
    BSP_Fault_SWT_Ticks = 0x0602,
    BSP_Fault_SWT_Mode = 0x0603,
    BSP_Fault_SWT_Handler = 0x0604,
    BSP_Fault_SWT_State = 0x0605
} BSP_Fault;

typedef enum
{
    BSP_phy_RTC = 0,
} BSP_phy_name;

/*
 * Set a debugger breakpoint in bsp_halt_fault and inspect the fault argument.
 * The volatile argument remains readable even in optimized builds. It occupies
 * one native 16-bit stack word; no global fault state, strings or callbacks exist.
 * Callers must perform peripheral-specific shutdown before calling this function.
 * This function never returns and does not change CPU IRQ/FIQ or watchdog policy.
 * A watchdog reset can still occur; fault information is not retained across reset.
 */
void bsp_halt_fault(volatile BSP_Fault fault);
bool bsp_is_phy_init(BSP_phy_name phy);
void bsp_set_phy_init(BSP_phy_name phy);
void bsp_clear_phy_init(BSP_phy_name phy);
void bsp_service_rtc(void);
void bsp_init_global(void);
#endif
