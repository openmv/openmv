#include <stm32f4xx_hal.h>
#include "usbd_desc.h"
#include "usbd_cdc.h" 

#define LED_RED     GPIO_PIN_0
#define LED_GREEN   GPIO_PIN_2
#define LED_BLUE    GPIO_PIN_1

#define APP_ADDR (0x08010000)

USBD_HandleTypeDef  USBD_Device;
extern USBD_CDC_ItfTypeDef  USBD_CDC_fops;

void __flash_led(uint16_t led)
{
    HAL_GPIO_TogglePin(GPIOC, led);
    HAL_Delay(100);
    HAL_GPIO_TogglePin(GPIOC, led);
    HAL_Delay(100);
}

void __attribute__((noreturn)) __fatal_error()
{
    while (1) {
        __flash_led(LED_RED);
    }
}

int main()
{
    // Override main app interrupt vector offset (set in system_stm32f4xx.c)
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
    while (!USBD_IDE_Connected() &&
            (HAL_GetTick() - start) < 500) {
        __flash_led(LED_GREEN);
    }

    // Wait for new firmware image
    while (USBD_IDE_Connected()) {
        __flash_led(LED_GREEN);
    }

    // Deinit USB
    USBD_DeInit(&USBD_Device);

    // Disable IRQs
    __disable_irq(); __DSB(); __ISB();

    // Jump to main app
    ((void (*)(void))(*((uint32_t*) (APP_ADDR+4))))();
}
