#ifndef __RGB_LED_H__
#define __RGB_LED_H__
enum led_color {
    LED_RED,
    LED_GREEN,
    LED_BLUE
};

void rgb_led_init();
void rgb_led_set_color(enum led_color color);
#endif //__RGB_LED_H__
