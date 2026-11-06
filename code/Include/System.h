//==========================================================================
// File Name   : System.h
// Description : system APIs declaration
// Written by  : Ray Cheng
// Last modified date:
//              2012/10/24
//==========================================================================
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#define SYSTEM_DEBOUNCE_COUNT 0x0080

extern void system_init_module(void);
/* Enable CPU IRQ only; peripheral interrupt masks and FIQ are unchanged. */
extern void system_enable_irq(void);
extern void system_service_loop(void);
extern void system_clear_watchdog(void);
extern unsigned key_read_event(void);
/* Legacy placeholder only; this does not perform ADC channel switching. */
extern void system_switch_channel(unsigned channel);
extern void cache_enable_module(void);
extern void cache_disable_module(void);

extern void audio_raise_dac1(void);
extern void audio_lower_dac1(void);
extern void audio_raise_dac2(void);
extern void audio_lower_dac2(void);

extern void spi_check_drive(void);
extern void system_enter_sleep(unsigned rtc);
extern unsigned spi_read_word(unsigned addr_lo, unsigned addr_hi);
/* count=0 retains the legacy 65536-word copy behavior. */
extern void spi_read_words(unsigned buf, unsigned count, unsigned addr_lo, unsigned addr_hi);

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