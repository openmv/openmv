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
#include "axiqos.h"
#include "imlib.h"
#include "irq.h"
#include "omv_boardconfig.h"
#include "common.h"
// Define pin objects in this file.
#define OMV_GPIO_DEFINE_PINS    (1)
#include "omv_gpio.h"

extern void SystemClock_Config(void);
extern uint32_t omv_exti_get_gpio(uint32_t line);

void HAL_MspInit(void)
{
    /* Set the system clock */
    SystemClock_Config();

    #if defined(OMV_DMA_REGION_D1_BASE)\
     || defined(OMV_DMA_REGION_D2_BASE)\
     || defined(OMV_DMA_REGION_D3_BASE)
    __DSB(); __ISB();
    HAL_MPU_Disable();

    // Configure the MPU attributes to disable caching DMA buffers.
    MPU_Region_InitTypeDef MPU_InitStruct;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

    // Disable all regions.
    for (int i=MPU_REGION_NUMBER0; i<MPU_REGION_NUMBER15; i++) {
        MPU_InitStruct.Number = i;
        MPU_InitStruct.Enable = MPU_REGION_DISABLE;
        HAL_MPU_ConfigRegion(&MPU_InitStruct);
    }

    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    #if defined(OMV_DMA_REGION_D1_BASE)
    MPU_InitStruct.Number           = MPU_REGION_NUMBER15;
    MPU_InitStruct.BaseAddress      = OMV_DMA_REGION_D1_BASE;
    MPU_InitStruct.Size             = OMV_DMA_REGION_D1_SIZE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    #endif // defined(OMV_DMA_REGION_D1_BASE)

    #if defined(OMV_DMA_REGION_D2_BASE)
    MPU_InitStruct.Number           = MPU_REGION_NUMBER14;
    MPU_InitStruct.BaseAddress      = OMV_DMA_REGION_D2_BASE;
    MPU_InitStruct.Size             = OMV_DMA_REGION_D2_SIZE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    #endif // defined(OMV_DMA_REGION_D2_BASE)

    #if defined(OMV_DMA_REGION_D3_BASE)
    MPU_InitStruct.Number           = MPU_REGION_NUMBER13;
    MPU_InitStruct.BaseAddress      = OMV_DMA_REGION_D3_BASE;
    MPU_InitStruct.Size             = OMV_DMA_REGION_D3_SIZE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    #endif // defined(OMV_DMA_REGION_D3_BASE)

    // Enable the MPU.
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
    __DSB(); __ISB();
    #endif // defined(OMV_DMA_REGION_D1_BASE || OMV_DMA_REGION_D2_BASE || OMV_DMA_REGION_D3_BASE)

    // Enable I/D cache.
    #if defined(MCU_SERIES_F7) || defined(MCU_SERIES_H7)
    #ifdef OMV_DISABLE_CACHE
    // Disable caches for testing.
    SCB_DisableICache();
    SCB_DisableDCache();
    #else
    // Enable caches if not enabled, or clean and invalidate.
    if (!(SCB->CCR & (uint32_t)SCB_CCR_IC_Msk)) {
        SCB_EnableICache();
    } else {
        SCB_InvalidateICache();
        __ISB(); __DSB(); __DMB();
    }

    if (!(SCB->CCR & (uint32_t)SCB_CCR_DC_Msk)) {
        SCB_EnableDCache();
    } else {
        SCB_CleanInvalidateDCache();
        __ISB(); __DSB(); __DMB();
    }
    #endif
    #endif

    // Config Systick.
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

    // Enable GPIO clocks.
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
    #if OMV_ENABLE_GPIO_BANK_F
    __GPIOF_CLK_ENABLE();
    #endif
    #if OMV_ENABLE_GPIO_BANK_G
    __GPIOG_CLK_ENABLE();
    #endif
    #if OMV_ENABLE_GPIO_BANK_H
    __GPIOH_CLK_ENABLE();
    #endif
    #if OMV_ENABLE_GPIO_BANK_I
    __GPIOI_CLK_ENABLE();
    #endif
    #if OMV_ENABLE_GPIO_BANK_J
    __GPIOJ_CLK_ENABLE();
    #endif
    #if OMV_ENABLE_GPIO_BANK_K
    __GPIOK_CLK_ENABLE();
    #endif

    // Enable DMA clocks.
    __DMA1_CLK_ENABLE();
    __DMA2_CLK_ENABLE();

    #if defined(MCU_SERIES_H7)
    // MDMA clock.
    __HAL_RCC_MDMA_CLK_ENABLE();
    NVIC_SetPriority(MDMA_IRQn, IRQ_PRI_MDMA);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);
    #endif

    // Setup AXI QoS
    #if defined(OMV_AXI_QOS_D2_AHB_R_PRI)
    OMV_AXI_QOS_D2_AHB_R_SET(OMV_AXI_QOS_D2_AHB_R_PRI);
    #endif
    #if defined(OMV_AXI_QOS_D2_AHB_W_PRI)
    OMV_AXI_QOS_D2_AHB_W_SET(OMV_AXI_QOS_D2_AHB_W_PRI);
    #endif
    #if defined(OMV_AXI_QOS_C_M7_R_PRI)
    OMV_AXI_QOS_C_M7_R_SET(OMV_AXI_QOS_C_M7_R_PRI);
    #endif
    #if defined(OMV_AXI_QOS_C_M7_W_PRI)
    OMV_AXI_QOS_C_M7_W_SET(OMV_AXI_QOS_C_M7_W_PRI);
    #endif
    #if defined(OMV_AXI_QOS_SDMMC1_R_PRI)
    OMV_AXI_QOS_SDMMC1_R_SET(OMV_AXI_QOS_SDMMC1_R_PRI);
    #endif
    #if defined(OMV_AXI_QOS_SDMMC1_W_PRI)
    OMV_AXI_QOS_SDMMC1_W_SET(OMV_AXI_QOS_SDMMC1_W_PRI);
    #endif
    #if defined(OMV_AXI_QOS_MDMA_R_PRI)
    OMV_AXI_QOS_MDMA_R_SET(OMV_AXI_QOS_MDMA_R_PRI);
    #endif
    #if defined(OMV_AXI_QOS_MDMA_W_PRI)
    OMV_AXI_QOS_MDMA_W_SET(OMV_AXI_QOS_MDMA_W_PRI);
    #endif
    #if defined(OMV_AXI_QOS_DMA2D_R_PRI)
    OMV_AXI_QOS_DMA2D_R_SET(OMV_AXI_QOS_DMA2D_R_PRI);
    #endif
    #if defined(OMV_AXI_QOS_DMA2D_W_PRI)
    OMV_AXI_QOS_DMA2D_W_SET(OMV_AXI_QOS_DMA2D_W_PRI);
    #endif
    #if defined(OMV_AXI_QOS_LTDC_R_PRI)
    OMV_AXI_QOS_LTDC_R_SET(OMV_AXI_QOS_LTDC_R_PRI);
    #endif
    #if defined(OMV_AXI_QOS_LTDC_W_PRI)
    OMV_AXI_QOS_LTDC_W_SET(OMV_AXI_QOS_LTDC_W_PRI);
    #endif

    #if defined(DCMI_RESET_PIN)
    omv_gpio_config(DCMI_RESET_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_DOWN, OMV_GPIO_SPEED_LOW, -1);
    #endif
    #if defined(DCMI_FSYNC_PIN)
    omv_gpio_config(DCMI_FSYNC_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_DOWN, OMV_GPIO_SPEED_LOW, -1);
    #endif
    #if defined(DCMI_POWER_PIN)
    omv_gpio_config(DCMI_POWER_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    #endif

    #if defined(OMV_FIR_LEPTON_RESET_PIN)
    omv_gpio_config(OMV_FIR_LEPTON_RESET_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_FIR_LEPTON_RESET_PIN, 0);
    #endif

    #if defined(OMV_FIR_LEPTON_POWER_PIN)
    omv_gpio_config(OMV_FIR_LEPTON_POWER_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_FIR_LEPTON_POWER_PIN, 0);
    #endif

    #if defined(MCU_SERIES_H7)
    // This disconnects PA0/PA1 from PA0_C/PA1_C.
    // PA0_C/PA1_C connect to ADC1/2 Channels P0/P1
    HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_OPEN);
    HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_OPEN);
    // This connects PC2/PC3 from PC2_C/PC3_C.
    // PC2_C/PC3_C connect to ADC3 Channels P0/P1.
    // PC2_C is connected to the blue LED on the OpenMV Cam H7.
    // PC3_C is unused.
    HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC2, SYSCFG_SWITCH_PC2_CLOSE);
    HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC3, SYSCFG_SWITCH_PC3_CLOSE);
    #endif
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    omv_gpio_t scl_pin = NULL;
    omv_gpio_t sda_pin = NULL;

    if (hi2c->Instance == ISC_I2C) {
        // Enable I2C clock.
        ISC_I2C_CLK_ENABLE();
        scl_pin = ISC_I2C_SCL_PIN;
        sda_pin = ISC_I2C_SDA_PIN;
    #if defined(FIR_I2C)
    } else if (hi2c->Instance == FIR_I2C) {
        // Enable I2C clock.
        FIR_I2C_CLK_ENABLE();
        scl_pin = FIR_I2C_SCL_PIN;
        sda_pin = FIR_I2C_SDA_PIN;
    #endif
    #if defined(TOF_I2C)
    } else if (hi2c->Instance == TOF_I2C) {
        // Enable I2C clock.
        TOF_I2C_CLK_ENABLE();
        scl_pin = TOF_I2C_SCL_PIN;
        sda_pin = TOF_I2C_SDA_PIN;
    #endif
    #if defined(IMU_I2C)
    } else if (hi2c->Instance == IMU_I2C) {
        // Enable I2C clock.
        IMU_I2C_CLK_ENABLE();
        scl_pin = IMU_I2C_SCL_PIN;
        sda_pin = IMU_I2C_SDA_PIN;
    #endif
    #if defined(ISC_I2C_ALT)
    } else if (hi2c->Instance == ISC_I2C_ALT) {
        // Enable I2C clock.
        ISC_I2C_ALT_CLK_ENABLE();
        scl_pin = ISC_I2C_ALT_SCL_PIN;
        sda_pin = ISC_I2C_ALT_SDA_PIN;
    #endif
    }
    if (scl_pin && sda_pin) {
        omv_gpio_config(scl_pin, OMV_GPIO_MODE_ALT_OD, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_config(sda_pin, OMV_GPIO_MODE_ALT_OD, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == ISC_I2C) {
        ISC_I2C_FORCE_RESET();
        ISC_I2C_RELEASE_RESET();
        ISC_I2C_CLK_DISABLE();
    #if defined(FIR_I2C)
    } else if (hi2c->Instance == FIR_I2C) {
        FIR_I2C_FORCE_RESET();
        FIR_I2C_RELEASE_RESET();
        FIR_I2C_CLK_DISABLE();
    #endif
    #if defined(TOF_I2C)
    } else if (hi2c->Instance == TOF_I2C) {
        TOF_I2C_FORCE_RESET();
        TOF_I2C_RELEASE_RESET();
        TOF_I2C_CLK_DISABLE();
    #endif
    #if defined(IMU_I2C)
    } else if (hi2c->Instance == IMU_I2C) {
        IMU_I2C_FORCE_RESET();
        IMU_I2C_RELEASE_RESET();
        IMU_I2C_CLK_DISABLE();
    #endif
    #if defined(ISC_I2C_ALT)
    } else if (hi2c->Instance == ISC_I2C_ALT) {
        ISC_I2C_ALT_FORCE_RESET();
        ISC_I2C_ALT_RELEASE_RESET();
        ISC_I2C_ALT_CLK_DISABLE();
    #endif
    }
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    #if (OMV_XCLK_SOURCE == OMV_XCLK_TIM)
    if (htim->Instance == DCMI_TIM) {
        // Enable DCMI timer clock.
        DCMI_TIM_CLK_ENABLE();
        // Timer GPIO configuration.
        omv_gpio_config(DCMI_TIM_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
        #if defined(DCMI_TIM_EXT_PIN)
        omv_gpio_config(DCMI_TIM_EXT_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
        #endif
    }
    #endif // (OMV_XCLK_SOURCE == OMV_XCLK_TIM)

    #if defined(OMV_LCD_BL_TIM)
    if (htim->Instance == OMV_LCD_BL_TIM) {
        OMV_LCD_BL_TIM_CLK_ENABLE();
    }
    #endif

    #if defined(OMV_BUZZER_TIM)
    if (htim->Instance == OMV_BUZZER_TIM) {
        OMV_BUZZER_TIM_CLK_ENABLE();
    }
    #endif
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef *htim)
{
    #if defined(OMV_LCD_BL_TIM)
    if (htim->Instance == OMV_LCD_BL_TIM) {
        OMV_LCD_BL_TIM_FORCE_RESET();
        OMV_LCD_BL_TIM_RELEASE_RESET();
        OMV_LCD_BL_TIM_CLK_DISABLE();
    }
    #endif

    #if defined(OMV_BUZZER_TIM)
    if (htim->Instance == OMV_BUZZER_TIM) {
        OMV_BUZZER_TIM_FORCE_RESET();
        OMV_BUZZER_TIM_RELEASE_RESET();
        OMV_BUZZER_TIM_CLK_DISABLE();
    }
    #endif
}

void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi)
{
    const omv_gpio_t dcmi_pins[] = {
        DCMI_D0_PIN,
        DCMI_D1_PIN,
        DCMI_D2_PIN,
        DCMI_D3_PIN,
        DCMI_D4_PIN,
        DCMI_D5_PIN,
        DCMI_D6_PIN,
        DCMI_D7_PIN,
        DCMI_HSYNC_PIN,
        DCMI_VSYNC_PIN,
        DCMI_PXCLK_PIN,
    };

    // DCMI clock enable
    __DCMI_CLK_ENABLE();

    // Configure VSYNC EXTI.
    #if (DCMI_VSYNC_EXTI_SHARED == 1)
    if (exti_gpio == 0)
    #endif
    {
    omv_gpio_config(DCMI_VSYNC_PIN, OMV_GPIO_MODE_IT_BOTH, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_MAX, -1);
    }

    // Configure DCMI pins.
    for (int i=0; i<OMV_ARRAY_SIZE(dcmi_pins); i++) {
        omv_gpio_config(dcmi_pins[i], OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_MAX, -1);
    }
}

void HAL_DCMI_MspDeInit(DCMI_HandleTypeDef* hdcmi)
{
    const omv_gpio_t dcmi_pins[] = {
        DCMI_D0_PIN,
        DCMI_D1_PIN,
        DCMI_D2_PIN,
        DCMI_D3_PIN,
        DCMI_D4_PIN,
        DCMI_D5_PIN,
        DCMI_D6_PIN,
        DCMI_D7_PIN,
        DCMI_HSYNC_PIN,
        DCMI_VSYNC_PIN,
        DCMI_PXCLK_PIN,
    };

    // Disable DCMI clock.
    __DCMI_CLK_DISABLE();

    // Deinit pins.
    for (int i=0; i<OMV_ARRAY_SIZE(dcmi_pins); i++) {
        omv_gpio_deinit(dcmi_pins[i]);
    }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    typedef struct {
        omv_gpio_t sclk_pin;
        omv_gpio_t miso_pin;
        omv_gpio_t mosi_pin;
        omv_gpio_t ssel_pin;
    } spi_pins_t;

    spi_pins_t spi_pins = { NULL, NULL, NULL, NULL };

    if (0) {
    #if defined(SPI1_ID)
    } else if (hspi->Instance == SPI1) {
        __HAL_RCC_SPI1_CLK_ENABLE();
        spi_pins = (spi_pins_t) { SPI1_SCLK_PIN, SPI1_MISO_PIN, SPI1_MOSI_PIN, SPI1_SSEL_PIN };
    #endif
    #if defined(SPI2_ID)
    } else if (hspi->Instance == SPI2) {
        __HAL_RCC_SPI2_CLK_ENABLE();
        spi_pins = (spi_pins_t) { SPI2_SCLK_PIN, SPI2_MISO_PIN, SPI2_MOSI_PIN, SPI2_SSEL_PIN };
    #endif
    #if defined(SPI3_ID)
    } else if (hspi->Instance == SPI3) {
        __HAL_RCC_SPI3_CLK_ENABLE();
        spi_pins = (spi_pins_t) { SPI3_SCLK_PIN, SPI3_MISO_PIN, SPI3_MOSI_PIN, SPI3_SSEL_PIN };
    #endif
    #if defined(SPI4_ID)
    } else if (hspi->Instance == SPI4) {
        __HAL_RCC_SPI4_CLK_ENABLE();
        spi_pins = (spi_pins_t) { SPI4_SCLK_PIN, SPI4_MISO_PIN, SPI4_MOSI_PIN, SPI4_SSEL_PIN };
    #endif
    #if defined(SPI5_ID)
    } else if (hspi->Instance == SPI5) {
        __HAL_RCC_SPI5_CLK_ENABLE();
        spi_pins = (spi_pins_t) { SPI5_SCLK_PIN, SPI5_MISO_PIN, SPI5_MOSI_PIN, SPI5_SSEL_PIN };
    #endif
    #if defined(SPI6_ID)
    } else if (hspi->Instance == SPI6) {
        __HAL_RCC_SPI6_CLK_ENABLE();
        spi_pins = (spi_pins_t) { SPI6_SCLK_PIN, SPI6_MISO_PIN, SPI6_MOSI_PIN, SPI6_SSEL_PIN };
    #endif
    } else {
        return;
    }

    omv_gpio_config(spi_pins.sclk_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(spi_pins.miso_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(spi_pins.mosi_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    if (hspi->Init.NSS != SPI_NSS_SOFT) {
        omv_gpio_config(spi_pins.ssel_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
    } else {
        omv_gpio_config(spi_pins.ssel_pin, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
        #if defined(MCU_SERIES_H7)
        if (hspi->Init.NSSPolarity == SPI_NSS_POLARITY_LOW) {
            omv_gpio_write(spi_pins.ssel_pin, 1);
        } else {
            omv_gpio_write(spi_pins.ssel_pin, 0);
        }
        #endif
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{

}

#if defined(AUDIO_SAI)
void HAL_SAI_MspInit(SAI_HandleTypeDef* hsai)
{
    if (hsai->Instance == AUDIO_SAI) {
        AUDIO_SAI_CLK_ENABLE();
        omv_gpio_config(AUDIO_SAI_CK_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_config(AUDIO_SAI_D1_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    }
}

void HAL_SAI_MspDeInit(SAI_HandleTypeDef* hsai)
{
    if (hsai->Instance == SAI4_Block_A) {
        AUDIO_SAI_CLK_DISABLE();
        omv_gpio_deinit(AUDIO_SAI_CK_PIN);
        omv_gpio_deinit(AUDIO_SAI_D1_PIN);
    }
}
#elif defined(AUDIO_DFSDM)
void HAL_DFSDM_ChannelMspInit(DFSDM_Channel_HandleTypeDef *hdfsdm)
{
    if (hdfsdm->Instance == AUDIO_DFSDM) {
        AUDIO_DFSDM_CLK_ENABLE();
        omv_gpio_config(AUDIO_DFSDM_CK_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_config(AUDIO_DFSDM_D1_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    }
}

void HAL_DFSDM_ChannelMspDeInit(DFSDM_Channel_HandleTypeDef *hdfsdm)
{
    if (hdfsdm->Instance == AUDIO_DFSDM) {
        AUDIO_DFSDM_CLK_DISABLE();
        omv_gpio_deinit(AUDIO_DFSDM_CK_PIN);
        omv_gpio_deinit(AUDIO_DFSDM_D1_PIN);
    }
}
#endif

void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc)
{
    __HAL_RCC_CRC_CLK_ENABLE();
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc)
{
    __HAL_RCC_CRC_CLK_DISABLE();
}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef *hdma2d)
{
    __HAL_RCC_DMA2D_CLK_ENABLE();
}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef *hdma2d)
{
    __HAL_RCC_DMA2D_FORCE_RESET();
    __HAL_RCC_DMA2D_RELEASE_RESET();
    __HAL_RCC_DMA2D_CLK_DISABLE();
}

#if (OMV_HARDWARE_JPEG == 1)
void HAL_JPEG_MspInit(JPEG_HandleTypeDef *hjpeg)
{
    __HAL_RCC_JPEG_CLK_ENABLE();
}

void HAL_JPEG_MspDeInit(JPEG_HandleTypeDef *hjpeg)
{
    __HAL_RCC_JPEG_FORCE_RESET();
    __HAL_RCC_JPEG_RELEASE_RESET();
    __HAL_RCC_JPEG_CLK_DISABLE();
}
#endif

#if defined(OMV_LCD_CONTROLLER) && (!defined(OMV_DSI_CONTROLLER))
static const omv_gpio_t ltdc_pins[] = {
    OMV_LCD_R0_PIN,
    OMV_LCD_R1_PIN,
    OMV_LCD_R2_PIN,
    OMV_LCD_R3_PIN,
    OMV_LCD_R4_PIN,
    OMV_LCD_R5_PIN,
    OMV_LCD_R6_PIN,
    OMV_LCD_R7_PIN,
    OMV_LCD_G0_PIN,
    OMV_LCD_G1_PIN,
    OMV_LCD_G2_PIN,
    OMV_LCD_G3_PIN,
    OMV_LCD_G4_PIN,
    OMV_LCD_G5_PIN,
    OMV_LCD_G6_PIN,
    OMV_LCD_G7_PIN,
    OMV_LCD_B0_PIN,
    OMV_LCD_B1_PIN,
    OMV_LCD_B2_PIN,
    OMV_LCD_B3_PIN,
    OMV_LCD_B4_PIN,
    OMV_LCD_B5_PIN,
    OMV_LCD_B6_PIN,
    OMV_LCD_B7_PIN,
    OMV_LCD_CLK_PIN,
    OMV_LCD_DE_PIN,
    OMV_LCD_HSYNC_PIN,
    OMV_LCD_VSYNC_PIN,
};
#endif

#if defined(OMV_LCD_CONTROLLER) || defined(OMV_DSI_CONTROLLER)
void HAL_LTDC_MspInit(LTDC_HandleTypeDef *hltdc)
{
    #if defined(OMV_DSI_CONTROLLER)
    if (hltdc->Instance == OMV_LCD_CONTROLLER) {
        OMV_LCD_CLK_ENABLE();
    }
    #elif defined(OMV_LCD_CONTROLLER)
    if (hltdc->Instance == OMV_LCD_CONTROLLER) {
        OMV_LCD_CLK_ENABLE();

        for (int i=0; i<OMV_ARRAY_SIZE(ltdc_pins); i++) {
            omv_gpio_config(ltdc_pins[i], OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MAX, -1);
        }

        #if defined(OMV_LCD_DISP_PIN)
        omv_gpio_config(OMV_LCD_DISP_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_write(OMV_LCD_DISP_PIN, 0);
        #endif

        #if defined(OMV_LCD_BL_PIN)
        omv_gpio_config(OMV_LCD_BL_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_write(OMV_LCD_BL_PIN, 0);
        #endif
    }
    #endif
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef *hltdc)
{
    #if defined(OMV_DSI_CONTROLLER)
    if (hltdc->Instance == OMV_LCD_CONTROLLER) {
        OMV_LCD_FORCE_RESET();
        OMV_LCD_RELEASE_RESET();
        OMV_LCD_CLK_DISABLE();
    }
    #elif defined(OMV_LCD_CONTROLLER)
    if (hltdc->Instance == OMV_LCD_CONTROLLER) {
        OMV_LCD_FORCE_RESET();
        OMV_LCD_RELEASE_RESET();
        OMV_LCD_CLK_DISABLE();

        for (int i=0; i<OMV_ARRAY_SIZE(ltdc_pins); i++) {
            omv_gpio_deinit(ltdc_pins[i]);
        }

        #if defined(OMV_LCD_DISP_PIN)
        omv_gpio_deinit(OMV_LCD_DISP_PIN);
        #endif

        #if defined(OMV_LCD_BL_PIN)
        omv_gpio_deinit(OMV_LCD_BL_PIN);
        #endif
    }
    #endif
}
#endif

void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac)
{
    #if defined(OMV_SPI_LCD_BL_DAC)
    if (hdac->Instance == OMV_SPI_LCD_BL_DAC) {
        OMV_SPI_LCD_BL_DAC_CLK_ENABLE();
    }
    #endif
}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef *hdac)
{
    #if defined(OMV_SPI_LCD_BL_DAC)
    if (hdac->Instance == OMV_SPI_LCD_BL_DAC) {
        OMV_SPI_LCD_BL_DAC_FORCE_RESET();
        OMV_SPI_LCD_BL_DAC_RELEASE_RESET();
        OMV_SPI_LCD_BL_DAC_CLK_DISABLE();
    }
    #endif
}

void HAL_MspDeInit(void)
{

}

void MDMA_IRQHandler()
{
    IRQ_ENTER(MDMA_IRQn);
    #if (OMV_HARDWARE_JPEG == 1)
    jpeg_mdma_irq_handler();
    #endif
    IRQ_EXIT(MDMA_IRQn);
}
