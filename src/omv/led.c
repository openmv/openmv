#include <stm32f4xx_hal.h>

#include "mpconfig.h"
#include "nlr.h"
#include "misc.h"
#include "qstr.h"
#include "obj.h"
#include "runtime.h"
#include "timer.h"
#include "led.h"
#include "pin.h"
#include "genhdr/pins.h"

STATIC const pin_obj_t *led_objs[] = {
    &MICROPY_HW_LED1,
    &MICROPY_HW_LED2,
    &MICROPY_HW_LED3,
};
#define NUM_LEDS ARRAY_SIZE(led_objs)

void led_init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /* Configure LED pins in output mode */
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;

    /* Initialize LEDS */
    for (int led = 0; led<NUM_LEDS; led++) {
        const pin_obj_t *led_pin = led_objs[led];
        MICROPY_HW_LED_OFF(led_pin);
        GPIO_InitStructure.Pin = led_pin->pin_mask;
        HAL_GPIO_Init(led_pin->gpio, &GPIO_InitStructure);
    }
}

void led_state(enum led_id id, int state)
{
    if (id < NUM_LEDS) {
        if (state) {
            /* turn on LED */
            MICROPY_HW_LED_ON(led_objs[id]);
        } else {
            /* turn off LED */
            MICROPY_HW_LED_OFF(led_objs[id]);
        }
    }
}

void led_toggle(enum led_id id)
{
    /* Invert LED state */
    HAL_GPIO_TogglePin(led_objs[id]->gpio, led_objs[id]->pin_mask);
}
