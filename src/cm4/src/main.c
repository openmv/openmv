#include STM32_HAL_H
#define HSEM_ID_0    (0U)    /* HW semaphore 0*/
#define LED_RED      GPIO_PIN_5
#define LED_GREEN    GPIO_PIN_6
#define LED_BLUE     GPIO_PIN_7

void blink_led(uint32_t led) {
    __GPIOK_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin = led;
    GPIO_InitStructure.Pull = GPIO_PULLDOWN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOK, &GPIO_InitStructure);
    HAL_GPIO_WritePin(GPIOK, led, GPIO_PIN_SET);

    for (int i = 0; i < 5; i++) {
        HAL_GPIO_WritePin(GPIOK, led, GPIO_PIN_RESET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOK, led, GPIO_PIN_SET);
        HAL_Delay(100);
    }
    __GPIOK_CLK_DISABLE();
}

int main(void) {
    HAL_Init();

    blink_led(LED_GREEN);

    // HW semaphore Clock enable
    __HAL_RCC_HSEM_CLK_ENABLE();

    // Configure the NVIC HSEM notification interrupt for CM4
    HAL_NVIC_SetPriority(HSEM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(HSEM2_IRQn);

    // Activate HSEM notification for Cortex-M4
    HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

    HAL_PWREx_ClearPendingEvent();
    HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D2_DOMAIN);

    HAL_PWREx_ClearPendingEvent();

    // Deactivate HSEM notification for Cortex-M4
    HAL_HSEM_DeactivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

    // HW semaphore Clock disable
    __HAL_RCC_HSEM_CLK_DISABLE();

    // Enter D3 domain to DStandby mode
    HAL_PWREx_EnterSTANDBYMode(PWR_D3_DOMAIN);

    // Enter D2 domain to DStandby mode
    HAL_PWREx_EnterSTANDBYMode(PWR_D2_DOMAIN);

    HAL_Init();
    while (1) {
        blink_led(LED_RED);
    }
}
