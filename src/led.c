#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_misc.h>
#include <stm32f4xx_tim.h>
#include "led.h"
#include "systick.h"
static uint16_t led;

static void led_cb()
{
    /* Toggle LED1 */
    GPIO_ToggleBits(GPIOD, led);
}

void led_init(enum led_color color)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    /* Enable GPIOG clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    /* Configure LED pins in output mode */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_SetBits(GPIOD, GPIO_Pin_4);
    GPIO_SetBits(GPIOD, GPIO_Pin_5);
    GPIO_SetBits(GPIOD, GPIO_Pin_6);

    led=color;

    /* Call back LED function every 1 second */
    systick_sched_task(led_cb, 1000);
}


void led_set_color(enum led_color color)
{
    int old_pin = led;
    led = color;

    /* turn off old LED */
    GPIO_SetBits(GPIOD, old_pin);
}
