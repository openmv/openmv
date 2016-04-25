/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * HAL MSP.
 *
 */
#include <stm32f4xx_hal.h>

#define LED_RED     GPIO_PIN_0
#define LED_GREEN   GPIO_PIN_2
#define LED_BLUE    GPIO_PIN_1

void SystemClock_Config();

void HAL_MspInit()
{
    // Set the system clock
    SystemClock_Config();

    // Config Systick
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();

    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;

    GPIO_InitStructure.Pin = LED_RED;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.Pin = LED_GREEN;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
 
    GPIO_InitStructure.Pin = LED_BLUE;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    HAL_GPIO_WritePin(GPIOC, LED_RED, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, LED_GREEN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, LED_BLUE, GPIO_PIN_SET);
}
