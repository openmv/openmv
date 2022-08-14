/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * HAL MSP.
 */
#include STM32_HAL_H 
#include "omv_boardconfig.h"
#include "omv_bootconfig.h"

extern void SystemClock_Config();

void HAL_MspInit()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;

    #if defined(OMV_BOOT_OSCEN_PIN)
    OMV_BOOT_OSCEN_CLK_ENABLE();
    GPIO_InitStructure.Pin   = OMV_BOOT_OSCEN_PIN;
    HAL_GPIO_Init(OMV_BOOT_OSCEN_PORT, &GPIO_InitStructure);
    HAL_GPIO_WritePin(OMV_BOOT_OSCEN_PORT, OMV_BOOT_OSCEN_PIN, GPIO_PIN_SET);
    #endif

    // Set the system clock
    SystemClock_Config();

    // Config Systick
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

    /* Enable GPIO clocks */
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
    #ifdef OMV_ENABLE_GPIO_BANK_F
    __GPIOF_CLK_ENABLE();
    #endif
    #ifdef OMV_ENABLE_GPIO_BANK_G
    __GPIOG_CLK_ENABLE();
    #endif
    #ifdef OMV_ENABLE_GPIO_BANK_H
    __GPIOH_CLK_ENABLE();
    #endif
    #ifdef OMV_ENABLE_GPIO_BANK_I
    __GPIOI_CLK_ENABLE();
    #endif
    #ifdef OMV_ENABLE_GPIO_BANK_J
    __GPIOJ_CLK_ENABLE();
    #endif
    #ifdef OMV_ENABLE_GPIO_BANK_K
    __GPIOK_CLK_ENABLE();
    #endif

    GPIO_InitStructure.Pin = OMV_BOOT_LED_PIN;
    HAL_GPIO_Init(OMV_BOOT_LED_PORT, &GPIO_InitStructure);
    HAL_GPIO_WritePin(OMV_BOOT_LED_PORT, OMV_BOOT_LED_PIN, GPIO_PIN_SET);

    #if defined(OMV_BOOT_USBEN_PIN)
    OMV_BOOT_USBEN_CLK_ENABLE();
    GPIO_InitStructure.Pin   = OMV_BOOT_USBEN_PIN;
    HAL_GPIO_Init(OMV_BOOT_USBEN_PORT, &GPIO_InitStructure);
    HAL_GPIO_WritePin(OMV_BOOT_USBEN_PORT, OMV_BOOT_USBEN_PIN, GPIO_PIN_SET);
    #endif
}

#if defined(OMV_BOOT_QSPIF_LAYOUT)
void HAL_QSPI_MspInit(QSPI_HandleTypeDef *hqspi)
{
    GPIO_InitTypeDef gpio_init_structure;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable the QuadSPI memory interface clock */
    OMV_BOOT_QSPIF_CLK_ENABLE();

    /* Reset the QuadSPI memory interface */
    OMV_BOOT_QSPIF_FORCE_RESET();
    OMV_BOOT_QSPIF_RELEASE_RESET();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* QSPI CLK GPIO pin configuration  */
    gpio_init_structure.Pin       = OMV_BOOT_QSPIF_CLK_PIN;
    gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
    gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_structure.Pull      = GPIO_NOPULL;
    gpio_init_structure.Alternate = OMV_BOOT_QSPIF_CLK_ALT;
    HAL_GPIO_Init(OMV_BOOT_QSPIF_CLK_PORT, &gpio_init_structure);

    /* QSPI CS GPIO pin configuration  */
    gpio_init_structure.Pin       = OMV_BOOT_QSPIF_CS_PIN;
    gpio_init_structure.Pull      = GPIO_PULLUP;
    gpio_init_structure.Alternate = OMV_BOOT_QSPIF_CS_ALT;
    HAL_GPIO_Init(OMV_BOOT_QSPIF_CS_PORT, &gpio_init_structure);

    /* QSPI D0 GPIO pin configuration  */
    gpio_init_structure.Pin       = OMV_BOOT_QSPIF_D0_PIN;
    gpio_init_structure.Pull      = GPIO_NOPULL;
    gpio_init_structure.Alternate = OMV_BOOT_QSPIF_D0_ALT;
    HAL_GPIO_Init(OMV_BOOT_QSPIF_D0_PORT, &gpio_init_structure);

    /* QSPI D1 GPIO pin configuration  */
    gpio_init_structure.Pin       = OMV_BOOT_QSPIF_D1_PIN;
    gpio_init_structure.Alternate = OMV_BOOT_QSPIF_D1_ALT;
    HAL_GPIO_Init(OMV_BOOT_QSPIF_D1_PORT, &gpio_init_structure);

    /* QSPI D2 GPIO pin configuration  */
    gpio_init_structure.Pin       = OMV_BOOT_QSPIF_D2_PIN;
    gpio_init_structure.Alternate = OMV_BOOT_QSPIF_D2_ALT;
    HAL_GPIO_Init(OMV_BOOT_QSPIF_D2_PORT, &gpio_init_structure);

    /* QSPI D3 GPIO pin configuration  */
    gpio_init_structure.Pin       = OMV_BOOT_QSPIF_D3_PIN;
    gpio_init_structure.Alternate = OMV_BOOT_QSPIF_D3_ALT;
    HAL_GPIO_Init(OMV_BOOT_QSPIF_D3_PORT, &gpio_init_structure);
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi)
{
    /*##-1- Disable peripherals and GPIO Clocks ################################*/
    /* De-Configure QSPI pins */
    HAL_GPIO_DeInit(OMV_BOOT_QSPIF_CLK_PORT, OMV_BOOT_QSPIF_CLK_PIN);
    HAL_GPIO_DeInit(OMV_BOOT_QSPIF_CS_PORT, OMV_BOOT_QSPIF_CS_PIN);
    HAL_GPIO_DeInit(OMV_BOOT_QSPIF_D0_PORT, OMV_BOOT_QSPIF_D0_PIN);
    HAL_GPIO_DeInit(OMV_BOOT_QSPIF_D1_PORT, OMV_BOOT_QSPIF_D1_PIN);
    HAL_GPIO_DeInit(OMV_BOOT_QSPIF_D2_PORT, OMV_BOOT_QSPIF_D2_PIN);
    HAL_GPIO_DeInit(OMV_BOOT_QSPIF_D3_PORT, OMV_BOOT_QSPIF_D3_PIN);

    /*##-2- Reset peripherals ##################################################*/
    /* Reset the QuadSPI memory interface */
    OMV_BOOT_QSPIF_FORCE_RESET();
    OMV_BOOT_QSPIF_RELEASE_RESET();

    /* Disable the QuadSPI memory interface clock */
    OMV_BOOT_QSPIF_CLK_DISABLE();
}
#endif // OMV_BOOT_QSPIF_LAYOUT
