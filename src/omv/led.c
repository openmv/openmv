#include <stm32f4xx_hal.h>
#include "pincfg.h"
#include "led.h"

/* LED GPIOs */
static const gpio_t led_pincfg[] = {
    {PINCFG_LED_PORT, PINCFG_LED_RED_PIN},
    {PINCFG_LED_PORT, PINCFG_LED_GREEN_PIN},
    {PINCFG_LED_PORT, PINCFG_LED_BLUE_PIN},
};
#define NUM_LEDS (sizeof(led_pincfg)/sizeof(led_pincfg[0]))

void led_init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;

    /* Initialize LEDS */
    for (int i=0; i<NUM_LEDS; i++) {
        PINCFG_LED_OFF(led_pincfg[i]);
        GPIO_InitStructure.Pin = led_pincfg[i].pin;
        HAL_GPIO_Init(led_pincfg[i].port, &GPIO_InitStructure);
    }
}

void led_state(enum led_id id, int state)
{
    if (id < NUM_LEDS) {
        if (state) {
            /* turn on LED */
            PINCFG_LED_ON(led_pincfg[id]);
        } else {
            /* turn off LED */
            PINCFG_LED_OFF(led_pincfg[id]);
        }
    }
}

void led_toggle(enum led_id id)
{
    /* Invert LED state */
    HAL_GPIO_TogglePin(led_pincfg[id].port, led_pincfg[id].pin);
}
