/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * HAL MSP.
 *
 */
#include STM32_HAL_H
#include "omv_boardconfig.h"

/* GPIO struct */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} gpio_t;

/* DCMI GPIOs */
static const gpio_t dcmi_pins[] = {
    {DCMI_D0_PORT, DCMI_D0_PIN},
    {DCMI_D1_PORT, DCMI_D1_PIN},
    {DCMI_D2_PORT, DCMI_D2_PIN},
    {DCMI_D3_PORT, DCMI_D3_PIN},
    {DCMI_D4_PORT, DCMI_D4_PIN},
    {DCMI_D5_PORT, DCMI_D5_PIN},
    {DCMI_D6_PORT, DCMI_D6_PIN},
    {DCMI_D7_PORT, DCMI_D7_PIN},
    {DCMI_HSYNC_PORT, DCMI_HSYNC_PIN},
    {DCMI_VSYNC_PORT, DCMI_VSYNC_PIN},
    {DCMI_PXCLK_PORT, DCMI_PXCLK_PIN},
};

#define NUM_DCMI_PINS   (sizeof(dcmi_pins)/sizeof(dcmi_pins[0]))

void SystemClock_Config(void);

void HAL_MspInit(void)
{
    #if defined(MCU_SERIES_F7) || defined(MCU_SERIES_H7)
    // Invalidate each cache before enabling it
    SCB_InvalidateICache();
    SCB_InvalidateDCache();

    /* Enable the CPU Cache */
    SCB_EnableICache();
    SCB_EnableDCache();
    #endif

    /* Set the system clock */
    SystemClock_Config();

    /* Config Systick */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

    /* Enable GPIO clocks */
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();

#if defined(STM32F769xx)
    __GPIOF_CLK_ENABLE();
    __GPIOG_CLK_ENABLE();
    __GPIOH_CLK_ENABLE();
    __GPIOI_CLK_ENABLE();
    __GPIOJ_CLK_ENABLE();
    __GPIOK_CLK_ENABLE();

    /* Enable JPEG clock */
    __HAL_RCC_JPEG_CLK_ENABLE();
#endif

    /* Enable DMA clocks */
    __DMA1_CLK_ENABLE();
    __DMA2_CLK_ENABLE();

    /* Configure DCMI GPIO */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;

    GPIO_InitStructure.Pin = DCMI_RESET_PIN;
    HAL_GPIO_Init(DCMI_RESET_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_PWDN_PIN;
    HAL_GPIO_Init(DCMI_PWDN_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_FSIN_PIN;
    HAL_GPIO_Init(DCMI_FSIN_PORT, &GPIO_InitStructure);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == SCCB_I2C) {
        /* Enable I2C clock */
        SCCB_CLK_ENABLE();

        /* Configure SCCB GPIOs */
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.Pull      = GPIO_NOPULL;
        GPIO_InitStructure.Speed     = GPIO_SPEED_LOW;
        GPIO_InitStructure.Mode      = GPIO_MODE_AF_OD;
        GPIO_InitStructure.Alternate = SCCB_AF;

        GPIO_InitStructure.Pin = SCCB_SCL_PIN;
        HAL_GPIO_Init(SCCB_PORT, &GPIO_InitStructure);

        GPIO_InitStructure.Pin = SCCB_SDA_PIN;
        HAL_GPIO_Init(SCCB_PORT, &GPIO_InitStructure);
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == SCCB_I2C) {
        SCCB_CLK_DISABLE();
    }
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    #if (OMV_XCLK_SOURCE == OMV_XCLK_TIM)
    if (htim->Instance == DCMI_TIM) {
        /* Enable DCMI timer clock */
        DCMI_TIM_CLK_ENABLE();

        /* Timer GPIO configuration */
        GPIO_InitTypeDef  GPIO_InitStructure;
        GPIO_InitStructure.Pin       = DCMI_TIM_PIN;
        GPIO_InitStructure.Pull      = GPIO_NOPULL;
        GPIO_InitStructure.Speed     = GPIO_SPEED_HIGH;
        GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStructure.Alternate = DCMI_TIM_AF;
        HAL_GPIO_Init(DCMI_TIM_PORT, &GPIO_InitStructure);
    }
    #endif // (OMV_XCLK_SOURCE == OMV_XCLK_TIM)
}

void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi)
{
    /* DCMI clock enable */
    __DCMI_CLK_ENABLE();

    /* DCMI GPIOs configuration */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull      = GPIO_PULLDOWN;
    GPIO_InitStructure.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStructure.Alternate = GPIO_AF13_DCMI;

    /* Enable VSYNC EXTI */
    GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStructure.Pin  = DCMI_VSYNC_PIN;
    HAL_GPIO_Init(DCMI_VSYNC_PORT, &GPIO_InitStructure);

    /* Configure DCMI pins */
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    for (int i=0; i<NUM_DCMI_PINS; i++) {
        GPIO_InitStructure.Pin = dcmi_pins[i].pin;
        HAL_GPIO_Init(dcmi_pins[i].port, &GPIO_InitStructure);
    }
}

void HAL_MspDeInit(void)
{

}
