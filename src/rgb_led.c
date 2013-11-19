#include "rgb_led.h"
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_misc.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_usart.h>
#include <ov9650.h>
static uint16_t led;

void TIM3_IRQHandler(void)
{
    /* Clear TIM2 update interrupt */
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

    /* Toggle LED1 */
    GPIO_ToggleBits(GPIOD, led);
}

void rgb_led_init(enum led_color color)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

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

    /* Enable TIM2 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* TIM2 configuration */
    TIM_TimeBaseStructure.TIM_Period = 0x4AF; 
    TIM_TimeBaseStructure.TIM_Prescaler = ((SystemCoreClock/1680) - 1);
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_OCStructInit(&TIM_OCInitStructure);

    /* Output Compare Timing Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
    TIM_OCInitStructure.TIM_Pulse = 0x0;  
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);

    /* Immediate load of TIM2 Precaler values */
    TIM_PrescalerConfig(TIM3, ((SystemCoreClock/1680) - 1), TIM_PSCReloadMode_Immediate);

    /* Clear TIM2 update pending flags */
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);

    /* Configure two bits for preemption priority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* Enable the TIM2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable TIM2 Update interrupts */
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    /* TIM2 enable counters */
    TIM_Cmd(TIM3, ENABLE);

    led=color;
}


void rgb_led_set_color(enum led_color color)
{
    int old_pin = led;
    led = color;

    /* turn off old LED */
    GPIO_SetBits(GPIOD, old_pin);
}
