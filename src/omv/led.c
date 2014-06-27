#include <stm32f4xx_hal.h>
#include "pincfg.h"
#include "led.h"

void led_init(enum led_id id)
{
    led_state(id, 1);
}

void led_toggle(enum led_id id)
{
    if (id >= 0 && id <= LED_BLUE) {
        /* Invert LED state */
        HAL_GPIO_TogglePin(led_pins[id].port, led_pins[id].pin);
    }
}

void led_state(enum led_id id, int state)
{
    if (id >= 0 && id <= LED_BLUE) {
        HAL_GPIO_WritePin(led_pins[id].port,
                led_pins[id].pin, (state)? GPIO_PIN_RESET:GPIO_PIN_SET);
    }
}


