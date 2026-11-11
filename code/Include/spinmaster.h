#ifndef _SPINMASTER_H__
#define _SPINMASTER_H__

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#if !defined(SUNPLUS) && !defined(unSP) && !defined(__CHAR_16__)
#include <stdint.h>
#include <stdbool.h>
#endif

#include "GPCE36_CE5.h"
#include "Resource.h"
#include "voice/FileMergerVersion1/FileMerger.h"

#include "../Include/System.h"
#include "../Include/SPI_Flash.h"
#include "../Include/user_config.h"

#include "../bsp/bsp_global.h"

#include "../hal/hal_gpio.h"
#include "../Include/user_define.h"
#include "../hal/hal_adc.h"
#include "../hal/hal_iopwm.h"
#include "../hal/hal_rtc.h"
#include "../hal/hal_exisr.h"
#include "../hal/hal_spi.h"
#include "../hal/hal_uart.h"

#include "../audio/audio_spu.h"

#include "../dev/timer/soft_timer.h"
#include "../message/message.h"
#include "../dev/button/dev_button.h"
#include "../dev/lcd/st7735p3.h"
#include "../dev/lcd/lcd_monitor.h"
#include "../gui/common/gui_common.h"

#include "../schedule/schedule.h"
#include "../task/task_led.h"

#endif // _SPINMASTER_H__

