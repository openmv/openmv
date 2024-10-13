/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * HAL MSP.
 */
#include STM32_HAL_H
#include "axiqos.h"
#include "imlib.h"
#include "irq.h"
#include "omv_boardconfig.h"
#include "omv_common.h"
// Define pin objects in this file.
#define OMV_GPIO_DEFINE_PINS    (1)
#include "omv_gpio.h"
#include "dma_utils.h"

#if defined(MPU_REGION_NUMBER15)
#define MPU_REGION_NUMBER_MAX   (MPU_REGION_NUMBER15)
#else
#define MPU_REGION_NUMBER_MAX   (MPU_REGION_NUMBER7)
#endif

extern void SystemClock_Config(void);
extern uint32_t omv_exti_get_gpio(uint32_t line);

void HAL_MspInit(void) {
    /* Set the system clock */
    SystemClock_Config();

    #if (__DCACHE_PRESENT == 1) && defined(OMV_DMA_MEMORY)
    HAL_MPU_Disable();

    // Configure the MPU attributes to disable caching DMA buffers.
    MPU_Region_InitTypeDef MPU_InitStruct;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

    // Disable all regions.
    for (int i = MPU_REGION_NUMBER0; i <= MPU_REGION_NUMBER_MAX; i++) {
        MPU_InitStruct.Number = i;
        MPU_InitStruct.Enable = MPU_REGION_DISABLE;
        HAL_MPU_ConfigRegion(&MPU_InitStruct);
    }

    typedef struct {
        uint32_t addr;
        uint32_t size;
    } dma_memory_table_t;

    uint8_t region_number = MPU_REGION_NUMBER_MAX;
    extern const dma_memory_table_t _dma_memory_table_start;
    extern const dma_memory_table_t _dma_memory_table_end;

    for (dma_memory_table_t const *buf = &_dma_memory_table_start; buf < &_dma_memory_table_end; buf++) {
        if (buf->size >= 32) {
            MPU_InitStruct.Number = region_number--;
            MPU_InitStruct.Enable = MPU_REGION_ENABLE;
            MPU_InitStruct.BaseAddress = buf->addr;
            MPU_InitStruct.Size = dma_utils_mpu_region_size(buf->size);
            HAL_MPU_ConfigRegion(&MPU_InitStruct);
        }
    }

    // Enable the MPU.
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
    __DSB(); __ISB();
    #endif // defined(OMV_DMA_MEMORY)

    // Enable I/D cache.
    #if defined(MCU_SERIES_F7) || defined(MCU_SERIES_H7)
    #ifdef OMV_DISABLE_CACHE
    // Disable caches for testing.
    SCB_DisableICache();
    SCB_DisableDCache();
    #else
    // Enable caches if not enabled, or clean and invalidate.
    if (!(SCB->CCR & (uint32_t) SCB_CCR_IC_Msk)) {
        SCB_EnableICache();
    } else {
        SCB_InvalidateICache();
        __ISB(); __DSB(); __DMB();
    }

    if (!(SCB->CCR & (uint32_t) SCB_CCR_DC_Msk)) {
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
    #if OMV_GPIO_PORT_F_ENABLE
    __GPIOF_CLK_ENABLE();
    #endif
    #if OMV_GPIO_PORT_G_ENABLE
    __GPIOG_CLK_ENABLE();
    #endif
    #if OMV_GPIO_PORT_H_ENABLE
    __GPIOH_CLK_ENABLE();
    #endif
    #if OMV_GPIO_PORT_I_ENABLE
    __GPIOI_CLK_ENABLE();
    #endif
    #if OMV_GPIO_PORT_J_ENABLE
    __GPIOJ_CLK_ENABLE();
    #endif
    #if OMV_GPIO_PORT_K_ENABLE
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

    #if defined(OMV_CSI_RESET_PIN)
    omv_gpio_config(OMV_CSI_RESET_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_DOWN, OMV_GPIO_SPEED_LOW, -1);
    #endif
    #if defined(OMV_CSI_FSYNC_PIN)
    omv_gpio_config(OMV_CSI_FSYNC_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_DOWN, OMV_GPIO_SPEED_LOW, -1);
    #endif
    #if defined(OMV_CSI_POWER_PIN)
    omv_gpio_config(OMV_CSI_POWER_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
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

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) {
    omv_gpio_t scl_pin = NULL;
    omv_gpio_t sda_pin = NULL;

    if (0) {
    #if defined(OMV_I2C1_ID)
    } else if (hi2c->Instance == I2C1) {
        __HAL_RCC_I2C1_CLK_ENABLE();
        scl_pin = OMV_I2C1_SCL_PIN;
        sda_pin = OMV_I2C1_SDA_PIN;
    #endif
    #if defined(OMV_I2C2_ID)
    } else if (hi2c->Instance == I2C2) {
        __HAL_RCC_I2C2_CLK_ENABLE();
        scl_pin = OMV_I2C2_SCL_PIN;
        sda_pin = OMV_I2C2_SDA_PIN;
    #endif
    #if defined(OMV_I2C3_ID)
    } else if (hi2c->Instance == I2C3) {
        __HAL_RCC_I2C3_CLK_ENABLE();
        scl_pin = OMV_I2C3_SCL_PIN;
        sda_pin = OMV_I2C3_SDA_PIN;
    #endif
    #if defined(OMV_I2C4_ID)
    } else if (hi2c->Instance == I2C4) {
        __HAL_RCC_I2C4_CLK_ENABLE();
        scl_pin = OMV_I2C4_SCL_PIN;
        sda_pin = OMV_I2C4_SDA_PIN;
    #endif
    }

    if (scl_pin && sda_pin) {
        omv_gpio_config(scl_pin, OMV_GPIO_MODE_ALT_OD, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_config(sda_pin, OMV_GPIO_MODE_ALT_OD, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c) {
    if (0) {
    #if defined(OMV_I2C1_ID)
    } else if (hi2c->Instance == I2C1) {
        __HAL_RCC_I2C1_FORCE_RESET();
        __HAL_RCC_I2C1_RELEASE_RESET();
        __HAL_RCC_I2C1_CLK_DISABLE();
    #endif
    #if defined(OMV_I2C2_ID)
    } else if (hi2c->Instance == I2C2) {
        __HAL_RCC_I2C2_FORCE_RESET();
        __HAL_RCC_I2C2_RELEASE_RESET();
        __HAL_RCC_I2C2_CLK_DISABLE();
    #endif
    #if defined(OMV_I2C3_ID)
    } else if (hi2c->Instance == I2C3) {
        __HAL_RCC_I2C3_FORCE_RESET();
        __HAL_RCC_I2C3_RELEASE_RESET();
        __HAL_RCC_I2C3_CLK_DISABLE();
    #endif
    #if defined(OMV_I2C4_ID)
    } else if (hi2c->Instance == I2C4) {
        __HAL_RCC_I2C4_FORCE_RESET();
        __HAL_RCC_I2C4_RELEASE_RESET();
        __HAL_RCC_I2C4_CLK_DISABLE();
    #endif
    }
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim) {
    #if (OMV_CSI_CLK_SOURCE == OMV_CSI_CLK_SOURCE_TIM)
    if (htim->Instance == OMV_CSI_TIM) {
        // Enable DCMI timer clock.
        OMV_CSI_TIM_CLK_ENABLE();
        // Timer GPIO configuration.
        omv_gpio_config(OMV_CSI_TIM_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
        #if defined(OMV_CSI_TIM_EXT_PIN)
        omv_gpio_config(OMV_CSI_TIM_EXT_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
        #endif
    }
    #endif // (OMV_CSI_CLK_SOURCE == OMV_CSI_CLK_SOURCE_TIM)

    #if defined(OMV_BUZZER_TIM)
    if (htim->Instance == OMV_BUZZER_TIM) {
        OMV_BUZZER_TIM_CLK_ENABLE();
    }
    #endif
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef *htim) {
    #if defined(OMV_BUZZER_TIM)
    if (htim->Instance == OMV_BUZZER_TIM) {
        OMV_BUZZER_TIM_FORCE_RESET();
        OMV_BUZZER_TIM_RELEASE_RESET();
        OMV_BUZZER_TIM_CLK_DISABLE();
    }
    #endif
}

void HAL_DCMI_MspInit(DCMI_HandleTypeDef *hdcmi) {
    const omv_gpio_t dcmi_pins[] = {
        OMV_CSI_D0_PIN,
        OMV_CSI_D1_PIN,
        OMV_CSI_D2_PIN,
        OMV_CSI_D3_PIN,
        OMV_CSI_D4_PIN,
        OMV_CSI_D5_PIN,
        OMV_CSI_D6_PIN,
        OMV_CSI_D7_PIN,
        OMV_CSI_HSYNC_PIN,
        OMV_CSI_VSYNC_PIN,
        OMV_CSI_PXCLK_PIN,
    };

    // DCMI clock enable
    __DCMI_CLK_ENABLE();

    // Configure VSYNC EXTI.
    #if (DCMI_VSYNC_EXTI_SHARED == 1)
    if (exti_gpio == 0)
    #endif
    {
        omv_gpio_config(OMV_CSI_VSYNC_PIN, OMV_GPIO_MODE_IT_BOTH, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_MAX, -1);
    }

    // Configure DCMI pins.
    for (int i = 0; i < OMV_ARRAY_SIZE(dcmi_pins); i++) {
        omv_gpio_config(dcmi_pins[i], OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_MAX, -1);
    }
}

void HAL_DCMI_MspDeInit(DCMI_HandleTypeDef *hdcmi) {
    const omv_gpio_t dcmi_pins[] = {
        OMV_CSI_D0_PIN,
        OMV_CSI_D1_PIN,
        OMV_CSI_D2_PIN,
        OMV_CSI_D3_PIN,
        OMV_CSI_D4_PIN,
        OMV_CSI_D5_PIN,
        OMV_CSI_D6_PIN,
        OMV_CSI_D7_PIN,
        OMV_CSI_HSYNC_PIN,
        OMV_CSI_VSYNC_PIN,
        OMV_CSI_PXCLK_PIN,
    };

    // Disable DCMI clock.
    __DCMI_CLK_DISABLE();

    // Deinit pins.
    for (int i = 0; i < OMV_ARRAY_SIZE(dcmi_pins); i++) {
        omv_gpio_deinit(dcmi_pins[i]);
    }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi) {
    typedef struct {
        omv_gpio_t sclk_pin;
        omv_gpio_t miso_pin;
        omv_gpio_t mosi_pin;
        omv_gpio_t ssel_pin;
    } spi_pins_t;

    spi_pins_t spi_pins = { NULL, NULL, NULL, NULL };

    if (0) {
    #if defined(OMV_SPI1_ID)
    } else if (hspi->Instance == SPI1) {
        __HAL_RCC_SPI1_CLK_ENABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI1_SCLK_PIN, OMV_SPI1_MISO_PIN, OMV_SPI1_MOSI_PIN, OMV_SPI1_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI2_ID)
    } else if (hspi->Instance == SPI2) {
        __HAL_RCC_SPI2_CLK_ENABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI2_SCLK_PIN, OMV_SPI2_MISO_PIN, OMV_SPI2_MOSI_PIN, OMV_SPI2_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI3_ID)
    } else if (hspi->Instance == SPI3) {
        __HAL_RCC_SPI3_CLK_ENABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI3_SCLK_PIN, OMV_SPI3_MISO_PIN, OMV_SPI3_MOSI_PIN, OMV_SPI3_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI4_ID)
    } else if (hspi->Instance == SPI4) {
        __HAL_RCC_SPI4_CLK_ENABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI4_SCLK_PIN, OMV_SPI4_MISO_PIN, OMV_SPI4_MOSI_PIN, OMV_SPI4_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI5_ID)
    } else if (hspi->Instance == SPI5) {
        __HAL_RCC_SPI5_CLK_ENABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI5_SCLK_PIN, OMV_SPI5_MISO_PIN, OMV_SPI5_MOSI_PIN, OMV_SPI5_SSEL_PIN
        };
    #endif
    #if defined(SPI6_ID)
    } else if (hspi->Instance == SPI6) {
        __HAL_RCC_SPI6_CLK_ENABLE();
        spi_pins = (spi_pins_t) {
            SPI6_SCLK_PIN, SPI6_MISO_PIN, SPI6_MOSI_PIN, SPI6_SSEL_PIN
        };
    #endif
    } else {
        return;
    }

    #if defined(MCU_SERIES_H7)
    omv_gpio_config(spi_pins.sclk_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    #else
    // The STM32F4 and STM32F7 don't set the initial state of the clock line until transmitting the
    // first packet... which means the first packet may be seen as corrupt by the slave device.
    if (hspi->Init.CLKPolarity == SPI_POLARITY_HIGH) {
        omv_gpio_config(spi_pins.sclk_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
    } else {
        omv_gpio_config(spi_pins.sclk_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_DOWN, OMV_GPIO_SPEED_HIGH, -1);
    }
    #endif
    omv_gpio_config(spi_pins.miso_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(spi_pins.mosi_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    if (hspi->Init.NSS != SPI_NSS_SOFT) {
        omv_gpio_config(spi_pins.ssel_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
    } else {
        if (hspi->Init.Mode == SPI_MODE_MASTER) {
            omv_gpio_config(spi_pins.ssel_pin, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
            #if defined(MCU_SERIES_H7)
            if (hspi->Init.NSSPolarity == SPI_NSS_POLARITY_LOW) {
                omv_gpio_write(spi_pins.ssel_pin, 1);
            } else {
                omv_gpio_write(spi_pins.ssel_pin, 0);
            }
            #else
            omv_gpio_write(spi_pins.ssel_pin, 1);
            #endif
        } else {
            omv_gpio_config(spi_pins.ssel_pin, OMV_GPIO_MODE_INPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
        }
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi) {
    typedef struct {
        omv_gpio_t sclk_pin;
        omv_gpio_t miso_pin;
        omv_gpio_t mosi_pin;
        omv_gpio_t ssel_pin;
    } spi_pins_t;

    spi_pins_t spi_pins = { NULL, NULL, NULL, NULL };

    if (0) {
    #if defined(OMV_SPI1_ID)
    } else if (hspi->Instance == SPI1) {
        __HAL_RCC_SPI1_FORCE_RESET();
        __HAL_RCC_SPI1_RELEASE_RESET();
        __HAL_RCC_SPI1_CLK_DISABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI1_SCLK_PIN, OMV_SPI1_MISO_PIN, OMV_SPI1_MOSI_PIN, OMV_SPI1_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI2_ID)
    } else if (hspi->Instance == SPI2) {
        __HAL_RCC_SPI2_FORCE_RESET();
        __HAL_RCC_SPI2_RELEASE_RESET();
        __HAL_RCC_SPI2_CLK_DISABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI2_SCLK_PIN, OMV_SPI2_MISO_PIN, OMV_SPI2_MOSI_PIN, OMV_SPI2_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI3_ID)
    } else if (hspi->Instance == SPI3) {
        __HAL_RCC_SPI3_FORCE_RESET();
        __HAL_RCC_SPI3_RELEASE_RESET();
        __HAL_RCC_SPI3_CLK_DISABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI3_SCLK_PIN, OMV_SPI3_MISO_PIN, OMV_SPI3_MOSI_PIN, OMV_SPI3_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI4_ID)
    } else if (hspi->Instance == SPI4) {
        __HAL_RCC_SPI4_FORCE_RESET();
        __HAL_RCC_SPI4_RELEASE_RESET();
        __HAL_RCC_SPI4_CLK_DISABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI4_SCLK_PIN, OMV_SPI4_MISO_PIN, OMV_SPI4_MOSI_PIN, OMV_SPI4_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI5_ID)
    } else if (hspi->Instance == SPI5) {
        __HAL_RCC_SPI5_FORCE_RESET();
        __HAL_RCC_SPI5_RELEASE_RESET();
        __HAL_RCC_SPI5_CLK_DISABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI5_SCLK_PIN, OMV_SPI5_MISO_PIN, OMV_SPI5_MOSI_PIN, OMV_SPI5_SSEL_PIN
        };
    #endif
    #if defined(SPI6_ID)
    } else if (hspi->Instance == SPI6) {
        __HAL_RCC_SPI6_FORCE_RESET();
        __HAL_RCC_SPI6_RELEASE_RESET();
        __HAL_RCC_SPI6_CLK_DISABLE();
        spi_pins = (spi_pins_t) {
            SPI6_SCLK_PIN, SPI6_MISO_PIN, SPI6_MOSI_PIN, SPI6_SSEL_PIN
        };
    #endif
    } else {
        return;
    }

    omv_gpio_deinit(spi_pins.sclk_pin);
    omv_gpio_deinit(spi_pins.miso_pin);
    omv_gpio_deinit(spi_pins.mosi_pin);
    // Deinited by omv_spi.c so as to not deinit the pin when HAL_SPI_MspDeInit is called
    // from deiniting the SPI bus from the machine or pyb module.
    // omv_gpio_deinit(spi_pins.ssel_pin);
}

#if defined(OMV_SAI)
void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai) {
    if (hsai->Instance == OMV_SAI) {
        OMV_SAI_CLK_ENABLE();
        omv_gpio_config(OMV_SAI_CK_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_config(OMV_SAI_D1_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    }
}

void HAL_SAI_MspDeInit(SAI_HandleTypeDef *hsai) {
    if (hsai->Instance == SAI4_Block_A) {
        OMV_SAI_CLK_DISABLE();
        omv_gpio_deinit(OMV_SAI_CK_PIN);
        omv_gpio_deinit(OMV_SAI_D1_PIN);
    }
}
#elif defined(OMV_DFSDM)
void HAL_DFSDM_ChannelMspInit(DFSDM_Channel_HandleTypeDef *hdfsdm) {
    if (hdfsdm->Instance == OMV_DFSDM) {
        OMV_DFSDM_CLK_ENABLE();
        omv_gpio_config(OMV_DFSDM_CK_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_config(OMV_DFSDM_D1_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    }
}

void HAL_DFSDM_ChannelMspDeInit(DFSDM_Channel_HandleTypeDef *hdfsdm) {
    if (hdfsdm->Instance == OMV_DFSDM) {
        OMV_DFSDM_CLK_DISABLE();
        omv_gpio_deinit(OMV_DFSDM_CK_PIN);
        omv_gpio_deinit(OMV_DFSDM_D1_PIN);
    }
}
#endif

void HAL_CRC_MspInit(CRC_HandleTypeDef *hcrc) {
    __HAL_RCC_CRC_CLK_ENABLE();
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef *hcrc) {
    __HAL_RCC_CRC_CLK_DISABLE();
}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef *hdma2d) {
    __HAL_RCC_DMA2D_CLK_ENABLE();
}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef *hdma2d) {
    __HAL_RCC_DMA2D_FORCE_RESET();
    __HAL_RCC_DMA2D_RELEASE_RESET();
    __HAL_RCC_DMA2D_CLK_DISABLE();
}

#if (OMV_JPEG_CODEC_ENABLE == 1)
void HAL_JPEG_MspInit(JPEG_HandleTypeDef *hjpeg) {
    __HAL_RCC_JPEG_CLK_ENABLE();
}

void HAL_JPEG_MspDeInit(JPEG_HandleTypeDef *hjpeg) {
    __HAL_RCC_JPEG_FORCE_RESET();
    __HAL_RCC_JPEG_RELEASE_RESET();
    __HAL_RCC_JPEG_CLK_DISABLE();
}
#endif

#if defined(OMV_DSI_DISPLAY_CONTROLLER)
void HAL_DSI_MspInit(DSI_HandleTypeDef *hdsi) {
    __HAL_RCC_DSI_CLK_ENABLE();
}

void HAL_DSI_MspDeInit(DSI_HandleTypeDef *hdsi) {
    __HAL_RCC_DSI_FORCE_RESET();
    __HAL_RCC_DSI_RELEASE_RESET();
    __HAL_RCC_DSI_CLK_DISABLE();
}
#endif

#if defined(OMV_RGB_DISPLAY_CONTROLLER) || defined(OMV_DSI_DISPLAY_CONTROLLER)
void HAL_LTDC_MspInit(LTDC_HandleTypeDef *hltdc) {
    #if defined(OMV_RGB_DISPLAY_R0_PIN)
    const omv_gpio_t ltdc_pins[] = {
        OMV_RGB_DISPLAY_R0_PIN,
        OMV_RGB_DISPLAY_R1_PIN,
        OMV_RGB_DISPLAY_R2_PIN,
        OMV_RGB_DISPLAY_R3_PIN,
        OMV_RGB_DISPLAY_R4_PIN,
        OMV_RGB_DISPLAY_R5_PIN,
        OMV_RGB_DISPLAY_R6_PIN,
        OMV_RGB_DISPLAY_R7_PIN,
        OMV_RGB_DISPLAY_G0_PIN,
        OMV_RGB_DISPLAY_G1_PIN,
        OMV_RGB_DISPLAY_G2_PIN,
        OMV_RGB_DISPLAY_G3_PIN,
        OMV_RGB_DISPLAY_G4_PIN,
        OMV_RGB_DISPLAY_G5_PIN,
        OMV_RGB_DISPLAY_G6_PIN,
        OMV_RGB_DISPLAY_G7_PIN,
        OMV_RGB_DISPLAY_B0_PIN,
        OMV_RGB_DISPLAY_B1_PIN,
        OMV_RGB_DISPLAY_B2_PIN,
        OMV_RGB_DISPLAY_B3_PIN,
        OMV_RGB_DISPLAY_B4_PIN,
        OMV_RGB_DISPLAY_B5_PIN,
        OMV_RGB_DISPLAY_B6_PIN,
        OMV_RGB_DISPLAY_B7_PIN,
        OMV_RGB_DISPLAY_CLK_PIN,
        OMV_RGB_DISPLAY_DE_PIN,
        OMV_RGB_DISPLAY_HSYNC_PIN,
        OMV_RGB_DISPLAY_VSYNC_PIN,
    };
    #endif

    OMV_RGB_DISPLAY_CLK_ENABLE();

    #if defined(OMV_RGB_DISPLAY_R0_PIN)
    for (int i = 0; i < OMV_ARRAY_SIZE(ltdc_pins); i++) {
        omv_gpio_config(ltdc_pins[i], OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MAX, -1);
    }
    #endif

    #if defined(OMV_RGB_DISPLAY_DISP_PIN)
    omv_gpio_config(OMV_RGB_DISPLAY_DISP_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_RGB_DISPLAY_DISP_PIN, 0);
    #endif
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef *hltdc) {
    #if defined(OMV_RGB_DISPLAY_R0_PIN)
    const omv_gpio_t ltdc_pins[] = {
        OMV_RGB_DISPLAY_R0_PIN,
        OMV_RGB_DISPLAY_R1_PIN,
        OMV_RGB_DISPLAY_R2_PIN,
        OMV_RGB_DISPLAY_R3_PIN,
        OMV_RGB_DISPLAY_R4_PIN,
        OMV_RGB_DISPLAY_R5_PIN,
        OMV_RGB_DISPLAY_R6_PIN,
        OMV_RGB_DISPLAY_R7_PIN,
        OMV_RGB_DISPLAY_G0_PIN,
        OMV_RGB_DISPLAY_G1_PIN,
        OMV_RGB_DISPLAY_G2_PIN,
        OMV_RGB_DISPLAY_G3_PIN,
        OMV_RGB_DISPLAY_G4_PIN,
        OMV_RGB_DISPLAY_G5_PIN,
        OMV_RGB_DISPLAY_G6_PIN,
        OMV_RGB_DISPLAY_G7_PIN,
        OMV_RGB_DISPLAY_B0_PIN,
        OMV_RGB_DISPLAY_B1_PIN,
        OMV_RGB_DISPLAY_B2_PIN,
        OMV_RGB_DISPLAY_B3_PIN,
        OMV_RGB_DISPLAY_B4_PIN,
        OMV_RGB_DISPLAY_B5_PIN,
        OMV_RGB_DISPLAY_B6_PIN,
        OMV_RGB_DISPLAY_B7_PIN,
        OMV_RGB_DISPLAY_CLK_PIN,
        OMV_RGB_DISPLAY_DE_PIN,
        OMV_RGB_DISPLAY_HSYNC_PIN,
        OMV_RGB_DISPLAY_VSYNC_PIN,
    };
    #endif

    OMV_RGB_DISPLAY_FORCE_RESET();
    OMV_RGB_DISPLAY_RELEASE_RESET();
    OMV_RGB_DISPLAY_CLK_DISABLE();

    #if defined(OMV_RGB_DISPLAY_R0_PIN)
    for (int i = 0; i < OMV_ARRAY_SIZE(ltdc_pins); i++) {
        omv_gpio_deinit(ltdc_pins[i]);
    }
    #endif

    #if defined(OMV_RGB_DISPLAY_DISP_PIN)
    omv_gpio_deinit(OMV_RGB_DISPLAY_DISP_PIN);
    #endif
}
#endif

void HAL_MspDeInit(void) {

}

void MDMA_IRQHandler() {
    IRQ_ENTER(MDMA_IRQn);
    #if (OMV_JPEG_CODEC_ENABLE == 1)
    extern void jpeg_mdma_irq_handler(void);
    jpeg_mdma_irq_handler();
    #endif
    #if defined(OMV_MDMA_CHANNEL_DCMI_0)
    extern void omv_csi_mdma_irq_handler(void);
    omv_csi_mdma_irq_handler();
    #endif
    IRQ_EXIT(MDMA_IRQn);
}
