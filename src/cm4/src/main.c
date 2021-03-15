#include STM32_HAL_H

int main(void)
{  
    __disable_irq();

    // Add Cortex-M4 code here.
    HAL_PWREx_ClearPendingEvent();
    HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D2_DOMAIN);

    while (1) {
        __WFI();
    }
}
