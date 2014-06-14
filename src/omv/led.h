#ifndef __LED_H__
#define __LED_H__
#include <stm32f4xx_gpio.h>
enum led_color {
    LED_RED=GPIO_Pin_4,
    LED_GREEN=GPIO_Pin_6,
    LED_BLUE=GPIO_Pin_5
};

void led_init(enum led_color color);
void led_set_color(enum led_color color);
void led_state(enum led_color color, int state);
void led_toggle(enum led_color color);
#endif //__LED_H__
