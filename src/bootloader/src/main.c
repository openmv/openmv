#include STM32_HAL_H
#include "usbd_cdc.h"
#include "usbd_desc.h"
#include "omv_boardconfig.h"

USBD_HandleTypeDef  USBD_Device;
extern USBD_CDC_ItfTypeDef  USBD_CDC_fops;

void __flash_led()
{
    HAL_GPIO_TogglePin(OMV_BOOTLDR_LED_PORT, OMV_BOOTLDR_LED_PIN);
    HAL_Delay(100);
    HAL_GPIO_TogglePin(OMV_BOOTLDR_LED_PORT, OMV_BOOTLDR_LED_PIN);
    HAL_Delay(100);
}

void __attribute__((noreturn)) __fatal_error()
{
    while (1) {
        __flash_led();
    }
}

int main()
{
    // Override main app interrupt vector offset (set in system_stm32fxxx.c)
    SCB->VTOR = FLASH_BASE | 0x0;

    HAL_Init();

    /* Init Device Library */
    USBD_Init(&USBD_Device, &VCP_Desc, 0);

    /* Add Supported Class */
    USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);

    /* Add CDC Interface Class */
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);

    /* Start Device Process */
    USBD_Start(&USBD_Device);

    // Wait for IDE to connect
    uint32_t start = HAL_GetTick();
    while (!USBD_IDE_Connected() && (HAL_GetTick() - start) < 500) {
        __flash_led();
    }

    // Wait for new firmware image
    while (USBD_IDE_Connected()) {
        __flash_led();
    }

    // Deinit USB
    USBD_DeInit(&USBD_Device);

    // Disable IRQs
    __disable_irq(); __DSB(); __ISB();

    // Jump to main app
    ((void (*)(void))(*((uint32_t*) (MAIN_APP_ADDR+4))))();
}
