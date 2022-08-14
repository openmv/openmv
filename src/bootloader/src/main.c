/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * main function.
 */
#include STM32_HAL_H
#include "usbdev/usbd_cdc.h"
#include "usbdev/usbd_desc.h"
#include "omv_boardconfig.h"
#include "omv_bootconfig.h"
#include "qspif.h"

#define IDE_TIMEOUT     (1000)
#define CONFIG_TIMEOUT  (2000)
USBD_HandleTypeDef  USBD_Device;
extern USBD_CDC_ItfTypeDef  USBD_CDC_fops;

void __flash_led()
{
    HAL_GPIO_TogglePin(OMV_BOOT_LED_PORT, OMV_BOOT_LED_PIN);
    HAL_Delay(100);
    HAL_GPIO_TogglePin(OMV_BOOT_LED_PORT, OMV_BOOT_LED_PIN);
    HAL_Delay(100);
}

void __attribute__((noreturn)) __fatal_error()
{
    while (1) {
        __flash_led();
    }
}

#ifdef STACK_PROTECTOR
uint32_t __stack_chk_guard=0xDEADBEEF;

void __attribute__((noreturn)) __stack_chk_fail(void)
{
    __asm__ volatile ("BKPT");
    while (1) {
        __flash_led();
    }
}
#endif

int main()
{
    // Override main app interrupt vector offset (set in system_stm32fxxx.c)
    SCB->VTOR = FLASH_BASE | 0x0;

    HAL_Init();

    #if defined(OMV_BOOT_QSPIF_LAYOUT)
    if (qspif_init() != 0) {
        __fatal_error();
    }
    #endif

    /* Init Device Library */
    USBD_Init(&USBD_Device, &VCP_Desc, 0);

    /* Add Supported Class */
    USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);

    /* Add CDC Interface Class */
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);

    /* Start Device Process */
    USBD_Start(&USBD_Device);

    // Note: The SRQINT interrupt is triggered when VBUS is in the valid range, I assume it's safe
    // to use it to detect if USB is connected or not. The dev_connection_status is set in usbd_conf.c
    // It wasn't used anywhere else, so again assuming it's safe to use it for connection status.
    if (USBD_Device.dev_connection_status) {
        uint32_t start = HAL_GetTick();
        // Wait for device to be configured
        while (USBD_Device.dev_state != USBD_STATE_CONFIGURED
                // We still have to timeout because the camera
                // might be connected to a power bank or charger
                && (HAL_GetTick() - start) < CONFIG_TIMEOUT) {
            __flash_led();
        }

        // If the device is configured, wait for IDE to connect or timeout
        if (USBD_Device.dev_state == USBD_STATE_CONFIGURED) {
            uint32_t start = HAL_GetTick();
            while (!USBD_IDE_Connected()
                    && (HAL_GetTick() - start) < IDE_TIMEOUT) {
                __flash_led();
            }

            // Wait for new firmware image if the IDE is connected
            while (USBD_IDE_Connected()) {
                __flash_led();
            }
        }
    }

    // Deinit USB
    USBD_DeInit(&USBD_Device);

    #if defined(OMV_BOOT_QSPIF_LAYOUT)
    qspif_reset();
    qspif_deinit();
    #endif

    // Disable IRQs
    __disable_irq(); __DSB(); __ISB();

    // Jump to main app
    ((void (*)(void))(*((uint32_t*) (MAIN_APP_ADDR+4))))();
}
