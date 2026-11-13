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
#include "../ram_heap/ram_heap.h"

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
#include "../gui/geometry/gui_geometry.h"
#include "../gui/font/gui_font.h"
#include "../gui/icon/gui_icon.h"
#include "../gui/widget/button/gui_button.h"
#include "../gui/widget/textbox/gui_textbox.h"
#include "../gui/widget/label/gui_label.h"
#include "../gui/widget/progressbar/gui_progressbar.h"
#include "../gui/widget/checkbox/gui_checkbox.h"
#include "../gui/widget/slider/gui_slider.h"
#include "../gui/widget/switch/gui_switch.h"
#include "../gui/desktop/gui_desktop.h"
#include "../gui/game/gui_game.h"
#include "../gui/game/snake/game_snake.h"
#include "../gui/game/tetris/game_tetris.h"
#include "../gui/game/brick/game_brick.h"
#include "../gui/game/flappy/game_flappy.h"
#include "../gui/game/shooter/game_shooter.h"
#include "../gui/game/g2048/game_2048.h"

#include "../schedule/schedule.h"

#endif // _SPINMASTER_H__

