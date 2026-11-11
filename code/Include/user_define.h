#ifndef _USER_DEFINE_H_
#define _USER_DEFINE_H_

#include "..\hal\hal_gpio.h"

// clang-format off
#define POWER_LOCK_PIN      ((gpio_pin_s){GPIOB, 0})

#define LCD_A0_PIN          ((gpio_pin_s){GPIOB, 8})
#define LCD_RESET_PIN       ((gpio_pin_s){GPIOB, 9})
#define LCD_TE_PIN          ((gpio_pin_s){GPIOB, 10})
#define LCD_CS_PIN          ((gpio_pin_s){GPIOB, 12})
#define LCD_BL_PWM_CH       IOPWM_Ch_8
// clang-format on
#endif