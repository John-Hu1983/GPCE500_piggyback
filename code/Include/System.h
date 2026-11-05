//==========================================================================
// File Name   : System.h
// Description : system APIs declaration
// Written by  : Ray Cheng
// Last modified date:
//              2012/10/24
//==========================================================================
#ifndef	__SYSTEM_H__
#define	__SYSTEM_H__

extern void System_Initial(void);
/* Enable CPU IRQ only; peripheral interrupt masks and FIQ are unchanged. */
extern void System_EnableIRQ(void);
extern void System_ServiceLoop(void);
extern void WatchdogClear(void);
extern unsigned SP_GetCh(void);
extern void SP_SwitchChannel(unsigned ADC_Channel);
extern void Cache_Enable(void);
extern void Cache_Disable(void);

extern void SP_RampUpDAC1(void);
extern void SP_RampDnDAC1(void);
extern void SP_RampUpDAC2(void);
extern void SP_RampDnDAC2(void);

extern void SPIFCHiDrivingCheck(void);
extern void GotoSleep(unsigned keepRTC);
extern unsigned AutoModeReadAWords(unsigned addressLow, unsigned addressHigh);
extern void AutoModeReadNWords(unsigned bufferAddress, unsigned wordCount,
                              unsigned addressLow, unsigned addressHigh);

// CPU-specific primitives and calibrated delays implemented in system.asm.
extern void System_CPU_Initial(void);
extern void System_CPU_EnableIRQ(void);
extern void System_DisableInterrupts(void);
extern void System_CacheWait(void);
extern void System_WakeupWait(void);
extern void System_LVD_Delay(void);
extern void System_RunSleepCodeFromRAM(void);
extern void SACM_Delay(void);

#endif