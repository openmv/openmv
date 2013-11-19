#ifndef __RGB_LED_H__
#define __RGB_LED_H__
#include <stm32f4xx_gpio.h>
enum led_color {
    LED_RED=GPIO_Pin_4,
    LED_GREEN=GPIO_Pin_6,
    LED_BLUE=GPIO_Pin_5
};

void rgb_led_init(enum led_color color);
void rgb_led_set_color(enum led_color color);
#endif //__RGB_LED_H__
