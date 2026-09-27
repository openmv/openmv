/*
 * stm32n6xx_hal_msp.c -- clocks, pins, interrupts and RIF security attributes of the
 * peripherals used by the cashew camera.
 */
#include "main.h"

void HAL_MspInit(void) {
}

// ---- camera control bus: I2C2 on PB10 (SCL) / PB11 (SDA) ---------------------------------
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance != CAM_I2C) {
        return;
    }
    GPIO_InitTypeDef g = { 0 };
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g.Pin = CAM_I2C_SCL_PIN | CAM_I2C_SDA_PIN;
    g.Mode = GPIO_MODE_AF_OD;
    g.Pull = GPIO_PULLUP;               // weak; the camera module has its own pull-ups
    g.Speed = GPIO_SPEED_FREQ_LOW;
    g.Alternate = CAM_I2C_AF;
    HAL_GPIO_Init(GPIOB, &g);
    __HAL_RCC_I2C2_CLK_ENABLE();
    __HAL_RCC_I2C2_FORCE_RESET();
    __HAL_RCC_I2C2_RELEASE_RESET();
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == CAM_I2C) {
        __HAL_RCC_I2C2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, CAM_I2C_SCL_PIN | CAM_I2C_SDA_PIN);
    }
}

// ---- log port: USART1 on PE5 (TX) / PE6 (RX), ST-LINK virtual COM port --------------------
void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
    if (huart->Instance != USART1) {
        return;
    }
    GPIO_InitTypeDef g = { 0 };
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    g.Pin = GPIO_PIN_5 | GPIO_PIN_6;
    g.Mode = GPIO_MODE_AF_PP;
    g.Pull = GPIO_PULLUP;
    g.Speed = GPIO_SPEED_FREQ_MEDIUM;
    g.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOE, &g);
}

// ---- camera interface: DCMIPP + CSI-2 host ---------------------------------------------
void HAL_DCMIPP_MspInit(DCMIPP_HandleTypeDef *hdcmipp) {
    if (hdcmipp->Instance != DCMIPP) {
        return;
    }
    RCC_PeriphCLKInitTypeDef clk = { 0 };
    RIMC_MasterConfig_t master = { 0 };

    __HAL_RCC_DCMIPP_CLK_ENABLE();
    __HAL_RCC_DCMIPP_FORCE_RESET();
    __HAL_RCC_DCMIPP_RELEASE_RESET();
    __HAL_RCC_CSI_CLK_ENABLE();
    __HAL_RCC_CSI_FORCE_RESET();
    __HAL_RCC_CSI_RELEASE_RESET();

    // DCMIPP kernel clock: IC17 = PLL1 1200 MHz / 4 = 300 MHz.
    clk.PeriphClockSelection = RCC_PERIPHCLK_DCMIPP;
    clk.DcmippClockSelection = RCC_DCMIPPCLKSOURCE_IC17;
    clk.ICSelection[RCC_IC17].ClockSelection = RCC_ICCLKSOURCE_PLL1;
    clk.ICSelection[RCC_IC17].ClockDivider = 4;
    if (HAL_RCCEx_PeriphCLKConfig(&clk) != HAL_OK) {
        Error_Handler();
    }
    // CSI-2 host (D-PHY) configuration clock: IC18 = 1200 MHz / 60 = 20 MHz.
    clk.PeriphClockSelection = RCC_PERIPHCLK_CSI;
    clk.ICSelection[RCC_IC18].ClockSelection = RCC_ICCLKSOURCE_PLL1;
    clk.ICSelection[RCC_IC18].ClockDivider = 60;
    if (HAL_RCCEx_PeriphCLKConfig(&clk) != HAL_OK) {
        Error_Handler();
    }

    // The DCMIPP writes frames as a secure, privileged bus master (the buffers are at the
    // secure AXISRAM1 alias 0x34000000).
    __HAL_RCC_RIFSC_CLK_ENABLE();
    master.MasterCID = RIF_CID_1;
    master.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV;
    HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DCMIPP, &master);
    HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DCMIPP, RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
    HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_CSI, RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);

    HAL_NVIC_SetPriority(DCMIPP_IRQn, IRQ_PRI_CAMERA, 0);
    HAL_NVIC_EnableIRQ(DCMIPP_IRQn);
    HAL_NVIC_SetPriority(CSI_IRQn, IRQ_PRI_CAMERA, 0);
    HAL_NVIC_EnableIRQ(CSI_IRQn);
}
