#ifndef _USER_DEFINE_H_
#define _USER_DEFINE_H_

#include "..\hal\hal_gpio.h"

// clang-format off
#define POWER_LOCK_PIN      ((gpio_pin_s){GPIOB, 0})

#define USB_DET_PIN         ((gpio_pin_s){GPIOA, 9})
#define BAT_DET_PIN         ((gpio_pin_s){GPIOA, 10})
#define USB_DET_ADC_CH      ADC_Ch_A9
#define BAT_DET_ADC_CH      ADC_Ch_A10

#define LCD_A0_PIN          ((gpio_pin_s){GPIOB, 8})
#define LCD_RESET_PIN       ((gpio_pin_s){GPIOB, 9})
#define LCD_TE_PIN          ((gpio_pin_s){GPIOB, 10})
#define LCD_CS_PIN          ((gpio_pin_s){GPIOB, 12})
#define LCD_BL_PWM_CH       IOPWM_Ch_8
// clang-format on
#endif