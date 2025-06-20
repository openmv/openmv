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

#define MEMATTR_NORMAL_NCACHE      0
#define MEMATTR_NORMAL_WB_RA_WA    1

extern void SystemClock_Config(void);
extern uint32_t omv_exti_get_gpio(uint32_t line);

void HAL_MspInit(void) {
    // Set the system clock
    SystemClock_Config();

    #if __DCACHE_PRESENT && defined(OMV_DMA_MEMORY)
    // Configure MPU regions.
    typedef struct {
        uint32_t addr;
        uint32_t size;
    } dma_memory_table_t;

    // Start from the last region.
    uint8_t region_number = (MPU->TYPE >> 8) - 1;
    extern const dma_memory_table_t _dma_memory_table_start;
    extern const dma_memory_table_t _dma_memory_table_end;

    #if (__ARM_ARCH == 8)
    ARM_MPU_Disable();
    // Clear all regions.
    for (size_t i = 0; i < (MPU->TYPE >> 8); i++) {
        ARM_MPU_ClrRegion(i);
    }

    ARM_MPU_SetMemAttr(MEMATTR_NORMAL_NCACHE, ARM_MPU_ATTR(
                           ARM_MPU_ATTR_NON_CACHEABLE,
                           ARM_MPU_ATTR_NON_CACHEABLE));

    ARM_MPU_SetMemAttr(MEMATTR_NORMAL_WB_RA_WA, ARM_MPU_ATTR(
                           ARM_MPU_ATTR_MEMORY_(1, 1, 1, 1),
                           ARM_MPU_ATTR_MEMORY_(1, 1, 1, 1)));

    MPU->RNR = 0;
    MPU->RBAR = ARM_MPU_RBAR(0x90000000, ARM_MPU_SH_NON, 0, 1, 0); // RO-0, NP-1, XN-0
    MPU->RLAR = ARM_MPU_RLAR(0x9FFFFFFF, MEMATTR_NORMAL_WB_RA_WA);

    for (dma_memory_table_t const *buf = &_dma_memory_table_start; buf < &_dma_memory_table_end; buf++) {
        uint32_t region_base = buf->addr;
        uint32_t region_size = buf->size;
        if (region_size) {
            MPU->RNR = region_number--;
            MPU->RBAR = ARM_MPU_RBAR(region_base, ARM_MPU_SH_NON, 0, 1, 0); // RO-0, NP-1, XN-0
            MPU->RLAR = ARM_MPU_RLAR(region_base + region_size - 1, MEMATTR_NORMAL_NCACHE);
        }
    }
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk);
    #elif (__ARM_ARCH == 7)
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
    for (int i = 0; i < (MPU->TYPE >> 8); i++) {
        MPU_InitStruct.Number = i;
        MPU_InitStruct.Enable = MPU_REGION_DISABLE;
        HAL_MPU_ConfigRegion(&MPU_InitStruct);
    }

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
    #else
    #error Unsupported ARM architecture
    #endif  // __ARM_ARCH
    #endif  // defined(OMV_DMA_MEMORY)

    // Enable I/D cache.
    #ifdef __DCACHE_PRESENT
    // Enable caches if not enabled, or clean and invalidate.
    if (!(SCB->CCR & (uint32_t) SCB_CCR_IC_Msk)) {
        SCB_EnableICache();
    } else {
        SCB_InvalidateICache();
        __ISB(); __DSB(); __DMB();
    }

    if (!(SCB->CCR & (uint32_t) SCB_CCR_DC_Msk)) {
        // See ST Errata ES0620 - Rev 0.2 section 2.1.2
        SCB_EnableDCache();
    } else {
        SCB_CleanInvalidateDCache();
        __ISB(); __DSB(); __DMB();
    }
    #endif

    // Config Systick.
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

    // Enable GPIO clocks.
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_SLEEP_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_SLEEP_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_SLEEP_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_SLEEP_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_SLEEP_ENABLE();
    #if OMV_GPIO_PORT_F_ENABLE
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_SLEEP_ENABLE();
    #endif
    #if OMV_GPIO_PORT_G_ENABLE
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_SLEEP_ENABLE();
    #endif
    #if OMV_GPIO_PORT_H_ENABLE
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_SLEEP_ENABLE();
    #endif
    #if OMV_GPIO_PORT_I_ENABLE
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_SLEEP_ENABLE();
    #endif
    #if OMV_GPIO_PORT_J_ENABLE
    __HAL_RCC_GPIOJ_CLK_ENABLE();
    __HAL_RCC_GPIOJ_CLK_SLEEP_ENABLE();
    #endif
    #if OMV_GPIO_PORT_K_ENABLE
    __HAL_RCC_GPIOK_CLK_ENABLE();
    __HAL_RCC_GPIOK_CLK_SLEEP_ENABLE();
    #endif
    #if OMV_GPIO_PORT_N_ENABLE
    __HAL_RCC_GPION_CLK_ENABLE();
    __HAL_RCC_GPION_CLK_SLEEP_ENABLE();
    #endif
    #if OMV_GPIO_PORT_O_ENABLE
    __HAL_RCC_GPIOO_CLK_ENABLE();
    __HAL_RCC_GPIOO_CLK_SLEEP_ENABLE();
    #endif
    #if OMV_GPIO_PORT_P_ENABLE
    __HAL_RCC_GPIOP_CLK_ENABLE();
    __HAL_RCC_GPIOP_CLK_SLEEP_ENABLE();
    #endif
    #if OMV_GPIO_PORT_Q_ENABLE
    __HAL_RCC_GPIOQ_CLK_ENABLE();
    __HAL_RCC_GPIOQ_CLK_SLEEP_ENABLE();
    #endif

    // Enable DMA clocks.
    #if defined(__HAL_RCC_DMA1_CLK_ENABLE)
    __HAL_RCC_DMA1_CLK_ENABLE();
    #endif
    #if defined(__HAL_RCC_DMA2_CLK_ENABLE)
    __HAL_RCC_DMA2_CLK_ENABLE();
    #endif
    #if defined(__HAL_RCC_GPDMA1_CLK_ENABLE)
    __HAL_RCC_GPDMA1_CLK_ENABLE();
    __HAL_RCC_GPDMA1_CLK_SLEEP_ENABLE();
    #endif
    #if defined(__HAL_RCC_HPDMA1_CLK_ENABLE)
    __HAL_RCC_HPDMA1_CLK_ENABLE();
    __HAL_RCC_HPDMA1_CLK_SLEEP_ENABLE();
    #endif
    #if defined(__HAL_RCC_MDMA_CLK_ENABLE)
    __HAL_RCC_MDMA_CLK_ENABLE();
    NVIC_SetPriority(MDMA_IRQn, IRQ_PRI_MDMA);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);
    #endif

    // Enable SDMMCx clocks.
    #if defined(STM32N6)
    __HAL_RCC_SDMMC1_CLK_ENABLE();
    __HAL_RCC_SDMMC1_CLK_SLEEP_ENABLE();

    __HAL_RCC_SDMMC2_CLK_ENABLE();
    __HAL_RCC_SDMMC2_CLK_SLEEP_ENABLE();
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

    #if defined(STM32H7)
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

    #if defined(STM32N6)
    __HAL_RCC_RIFSC_CLK_ENABLE();

    RIMC_MasterConfig_t RIMC_master = {
        .MasterCID = RIF_CID_1,
        .SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV,
    };

    // Array of master/peripheral index pairs [master_index, periph_index]
    static const uint32_t rif_config_pairs[][2] = {
        { RIF_MASTER_INDEX_NPU,    RIF_RISC_PERIPH_INDEX_NPU },
        { RIF_MASTER_INDEX_OTG1,   RIF_RISC_PERIPH_INDEX_OTG1HS },
        { RIF_MASTER_INDEX_DMA2D,  RIF_RISC_PERIPH_INDEX_DMA2D },
        { RIF_MASTER_INDEX_GPU2D,  RIF_RISC_PERIPH_INDEX_GPU2D },
        { RIF_MASTER_INDEX_VENC,   RIF_RISC_PERIPH_INDEX_VENC },
        { RIF_MASTER_INDEX_SDMMC1, RIF_RISC_PERIPH_INDEX_SDMMC1 },
        { RIF_MASTER_INDEX_SDMMC2, RIF_RISC_PERIPH_INDEX_SDMMC2 },
        { RIF_MASTER_INDEX_DCMIPP, RIF_RISC_PERIPH_INDEX_DCMIPP },
        { RIF_MASTER_INDEX_DCMIPP, RIF_RISC_PERIPH_INDEX_CSI },
    };
    
    for (int i = 0; i < sizeof(rif_config_pairs) / sizeof(rif_config_pairs[0]); i++) {
        HAL_RIF_RIMC_ConfigMasterAttributes(rif_config_pairs[i][0], &RIMC_master);
        HAL_RIF_RISC_SetSlaveSecureAttributes(rif_config_pairs[i][1], RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
    }
    #endif
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) {
    omv_gpio_t scl_pin = NULL;
    omv_gpio_t sda_pin = NULL;

    if (0) {
    #if defined(OMV_I2C1_ID)
    } else if (hi2c->Instance == I2C1) {
        scl_pin = OMV_I2C1_SCL_PIN;
        sda_pin = OMV_I2C1_SDA_PIN;
        __HAL_RCC_I2C1_CLK_ENABLE();
        __HAL_RCC_I2C1_CLK_SLEEP_ENABLE();
    #endif
    #if defined(OMV_I2C2_ID)
    } else if (hi2c->Instance == I2C2) {
        scl_pin = OMV_I2C2_SCL_PIN;
        sda_pin = OMV_I2C2_SDA_PIN;
        __HAL_RCC_I2C2_CLK_ENABLE();
        __HAL_RCC_I2C2_CLK_SLEEP_ENABLE();
    #endif
    #if defined(OMV_I2C3_ID)
    } else if (hi2c->Instance == I2C3) {
        scl_pin = OMV_I2C3_SCL_PIN;
        sda_pin = OMV_I2C3_SDA_PIN;
        __HAL_RCC_I2C3_CLK_ENABLE();
        __HAL_RCC_I2C3_CLK_SLEEP_ENABLE();
    #endif
    #if defined(OMV_I2C4_ID)
    } else if (hi2c->Instance == I2C4) {
        scl_pin = OMV_I2C4_SCL_PIN;
        sda_pin = OMV_I2C4_SDA_PIN;
        __HAL_RCC_I2C4_CLK_ENABLE();
        __HAL_RCC_I2C4_CLK_SLEEP_ENABLE();
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
        __HAL_RCC_I2C1_CLK_SLEEP_DISABLE();
    #endif
    #if defined(OMV_I2C2_ID)
    } else if (hi2c->Instance == I2C2) {
        __HAL_RCC_I2C2_FORCE_RESET();
        __HAL_RCC_I2C2_RELEASE_RESET();
        __HAL_RCC_I2C2_CLK_DISABLE();
        __HAL_RCC_I2C2_CLK_SLEEP_DISABLE();
    #endif
    #if defined(OMV_I2C3_ID)
    } else if (hi2c->Instance == I2C3) {
        __HAL_RCC_I2C3_FORCE_RESET();
        __HAL_RCC_I2C3_RELEASE_RESET();
        __HAL_RCC_I2C3_CLK_DISABLE();
        __HAL_RCC_I2C3_CLK_SLEEP_DISABLE();
    #endif
    #if defined(OMV_I2C4_ID)
    } else if (hi2c->Instance == I2C4) {
        __HAL_RCC_I2C4_FORCE_RESET();
        __HAL_RCC_I2C4_RELEASE_RESET();
        __HAL_RCC_I2C4_CLK_DISABLE();
        __HAL_RCC_I2C4_CLK_SLEEP_DISABLE();
    #endif
    }
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim) {
    #if defined(OMV_CSI_TIM)
    if (htim->Instance == OMV_CSI_TIM) {
        // Enable DCMI timer clock.
        OMV_CSI_TIM_CLK_ENABLE();
        OMV_CSI_TIM_CLK_SLEEP_ENABLE();
        // Timer GPIO configuration.
        omv_gpio_config(OMV_CSI_TIM_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
        #if defined(OMV_CSI_TIM_EXT_PIN)
        omv_gpio_config(OMV_CSI_TIM_EXT_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
        #endif
    }
    #endif // (OMV_CSI_TIM)

    #if defined(OMV_BUZZER_TIM)
    if (htim->Instance == OMV_BUZZER_TIM) {
        OMV_BUZZER_TIM_CLK_ENABLE();
    }
    #endif
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef *htim) {
    #if defined(OMV_CSI_TIM)
    if (htim->Instance == OMV_CSI_TIM) {
        // Disable DCMI timer clock.
        OMV_CSI_TIM_CLK_DISABLE();
        OMV_CSI_TIM_CLK_SLEEP_DISABLE();
        // Deinit timer GPIO.
        omv_gpio_deinit(OMV_CSI_TIM_PIN);
        #if defined(OMV_CSI_TIM_EXT_PIN)
        omv_gpio_deinit(OMV_CSI_TIM_EXT_PIN);
        #endif
    }
    #endif // (OMV_CSI_TIM)

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
        #ifdef OMV_CSI_D0_PIN
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
        #endif
    };

    // Enable DCMI clock.
    #if defined(PSSI)
    __HAL_RCC_DCMI_PSSI_CLK_ENABLE();
    __HAL_RCC_DCMI_PSSI_CLK_SLEEP_ENABLE();
    #else
    __HAL_RCC_DCMI_CLK_ENABLE();
    __HAL_RCC_DCMI_CLK_SLEEP_ENABLE();
    #endif

    #ifdef OMV_CSI_VSYNC_PIN
    // Configure VSYNC EXTI.
    #if DCMI_VSYNC_EXTI_SHARED
    if (exti_gpio == 0)
    #endif  // DCMI_VSYNC_EXTI_SHARED
    omv_gpio_config(OMV_CSI_VSYNC_PIN, OMV_GPIO_MODE_IT_BOTH, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_MAX, -1);
    #endif  // OMV_CSI_VSYNC_PIN

    // Configure DCMI pins.
    for (int i = 0; i < OMV_ARRAY_SIZE(dcmi_pins); i++) {
        omv_gpio_config(dcmi_pins[i], OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_MAX, -1);
    }
}

void HAL_DCMI_MspDeInit(DCMI_HandleTypeDef *hdcmi) {
    const omv_gpio_t dcmi_pins[] = {
        #ifdef OMV_CSI_D0_PIN
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
        #endif
    };

    // Disable DCMI clock.
    #if defined(PSSI)
    __HAL_RCC_DCMI_PSSI_CLK_DISABLE();
    __HAL_RCC_DCMI_PSSI_CLK_SLEEP_DISABLE();
    #else
    __HAL_RCC_DCMI_CLK_DISABLE();
    __HAL_RCC_DCMI_CLK_SLEEP_DISABLE();
    #endif

    // Deinit pins.
    for (int i = 0; i < OMV_ARRAY_SIZE(dcmi_pins); i++) {
        omv_gpio_deinit(dcmi_pins[i]);
    }
}

#if defined(DCMIPP)
void HAL_DCMIPP_MspInit(DCMIPP_HandleTypeDef *hdcmipp) {
    const omv_gpio_t dcmi_pins[] = {
        #ifdef OMV_CSI_D0_PIN
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
        #endif
    };

    // Enable DCMIPP clock.
    __HAL_RCC_DCMIPP_CLK_ENABLE();
    __HAL_RCC_DCMIPP_CLK_SLEEP_ENABLE();
    __HAL_RCC_DCMIPP_FORCE_RESET();
    __HAL_RCC_DCMIPP_RELEASE_RESET();

    // Enable CSI clock
    __HAL_RCC_CSI_CLK_ENABLE();
    __HAL_RCC_CSI_CLK_SLEEP_ENABLE();
    __HAL_RCC_CSI_FORCE_RESET();
    __HAL_RCC_CSI_RELEASE_RESET();

    #ifdef OMV_CSI_VSYNC_PIN
    // Configure VSYNC EXTI.
    #if DCMI_VSYNC_EXTI_SHARED
    if (exti_gpio == 0)
    #endif  // DCMI_VSYNC_EXTI_SHARED
    omv_gpio_config(OMV_CSI_VSYNC_PIN, OMV_GPIO_MODE_IT_BOTH, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_MAX, -1);
    #endif  // OMV_CSI_VSYNC_PIN

    // Configure DCMI pins.
    for (int i = 0; i < OMV_ARRAY_SIZE(dcmi_pins); i++) {
        omv_gpio_config(dcmi_pins[i], OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_MAX, -1);
    }
}

void HAL_DCMIPP_MspDeInit(DCMIPP_HandleTypeDef *hdcmipp) {
    const omv_gpio_t dcmi_pins[] = {
        #ifdef OMV_CSI_D0_PIN
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
        #endif
    };

    // Disable DCMI clock.
    __HAL_RCC_DCMIPP_FORCE_RESET();
    __HAL_RCC_DCMIPP_RELEASE_RESET();
    __HAL_RCC_DCMIPP_CLK_DISABLE();
    __HAL_RCC_DCMIPP_CLK_SLEEP_DISABLE();

    __HAL_RCC_CSI_FORCE_RESET();
    __HAL_RCC_CSI_RELEASE_RESET();
    __HAL_RCC_CSI_CLK_DISABLE();
    __HAL_RCC_CSI_CLK_SLEEP_DISABLE();

    // Deinit pins.
    for (int i = 0; i < OMV_ARRAY_SIZE(dcmi_pins); i++) {
        omv_gpio_deinit(dcmi_pins[i]);
    }
}
#endif

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
        __HAL_RCC_SPI1_CLK_SLEEP_ENABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI1_SCLK_PIN, OMV_SPI1_MISO_PIN, OMV_SPI1_MOSI_PIN, OMV_SPI1_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI2_ID)
    } else if (hspi->Instance == SPI2) {
        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_SPI2_CLK_SLEEP_ENABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI2_SCLK_PIN, OMV_SPI2_MISO_PIN, OMV_SPI2_MOSI_PIN, OMV_SPI2_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI3_ID)
    } else if (hspi->Instance == SPI3) {
        __HAL_RCC_SPI3_CLK_ENABLE();
        __HAL_RCC_SPI3_CLK_SLEEP_ENABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI3_SCLK_PIN, OMV_SPI3_MISO_PIN, OMV_SPI3_MOSI_PIN, OMV_SPI3_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI4_ID)
    } else if (hspi->Instance == SPI4) {
        __HAL_RCC_SPI4_CLK_ENABLE();
        __HAL_RCC_SPI4_CLK_SLEEP_ENABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI4_SCLK_PIN, OMV_SPI4_MISO_PIN, OMV_SPI4_MOSI_PIN, OMV_SPI4_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI5_ID)
    } else if (hspi->Instance == SPI5) {
        __HAL_RCC_SPI5_CLK_ENABLE();
        __HAL_RCC_SPI5_CLK_SLEEP_ENABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI5_SCLK_PIN, OMV_SPI5_MISO_PIN, OMV_SPI5_MOSI_PIN, OMV_SPI5_SSEL_PIN
        };
    #endif
    #if defined(SPI6_ID)
    } else if (hspi->Instance == SPI6) {
        __HAL_RCC_SPI6_CLK_ENABLE();
        __HAL_RCC_SPI6_CLK_SLEEP_ENABLE();
        spi_pins = (spi_pins_t) {
            SPI6_SCLK_PIN, SPI6_MISO_PIN, SPI6_MOSI_PIN, SPI6_SSEL_PIN
        };
    #endif
    } else {
        return;
    }

    #if defined(STM32H7) || defined(STM32N6)
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
    if (hspi->Init.Direction == SPI_DIRECTION_2LINES || hspi->Init.Direction == SPI_DIRECTION_2LINES_RXONLY) {
        omv_gpio_config(spi_pins.miso_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    }
    #if defined(STM32H7)
    if (hspi->Init.Direction == SPI_DIRECTION_2LINES || hspi->Init.Direction == SPI_DIRECTION_2LINES_TXONLY) {
    #else
    if (hspi->Init.Direction == SPI_DIRECTION_2LINES || hspi->Init.Direction == SPI_DIRECTION_1LINE) {
    #endif
        omv_gpio_config(spi_pins.mosi_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    }
    if (hspi->Init.NSS != SPI_NSS_SOFT) {
        omv_gpio_config(spi_pins.ssel_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
    } else {
        if (hspi->Init.Mode == SPI_MODE_MASTER) {
            omv_gpio_config(spi_pins.ssel_pin, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_HIGH, -1);
            #if defined(STM32H7) || defined(STM32N6)
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
        __HAL_RCC_SPI1_CLK_SLEEP_DISABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI1_SCLK_PIN, OMV_SPI1_MISO_PIN, OMV_SPI1_MOSI_PIN, OMV_SPI1_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI2_ID)
    } else if (hspi->Instance == SPI2) {
        __HAL_RCC_SPI2_FORCE_RESET();
        __HAL_RCC_SPI2_RELEASE_RESET();
        __HAL_RCC_SPI2_CLK_DISABLE();
        __HAL_RCC_SPI2_CLK_SLEEP_DISABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI2_SCLK_PIN, OMV_SPI2_MISO_PIN, OMV_SPI2_MOSI_PIN, OMV_SPI2_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI3_ID)
    } else if (hspi->Instance == SPI3) {
        __HAL_RCC_SPI3_FORCE_RESET();
        __HAL_RCC_SPI3_RELEASE_RESET();
        __HAL_RCC_SPI3_CLK_DISABLE();
        __HAL_RCC_SPI3_CLK_SLEEP_DISABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI3_SCLK_PIN, OMV_SPI3_MISO_PIN, OMV_SPI3_MOSI_PIN, OMV_SPI3_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI4_ID)
    } else if (hspi->Instance == SPI4) {
        __HAL_RCC_SPI4_FORCE_RESET();
        __HAL_RCC_SPI4_RELEASE_RESET();
        __HAL_RCC_SPI4_CLK_DISABLE();
        __HAL_RCC_SPI4_CLK_SLEEP_DISABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI4_SCLK_PIN, OMV_SPI4_MISO_PIN, OMV_SPI4_MOSI_PIN, OMV_SPI4_SSEL_PIN
        };
    #endif
    #if defined(OMV_SPI5_ID)
    } else if (hspi->Instance == SPI5) {
        __HAL_RCC_SPI5_FORCE_RESET();
        __HAL_RCC_SPI5_RELEASE_RESET();
        __HAL_RCC_SPI5_CLK_DISABLE();
        __HAL_RCC_SPI5_CLK_SLEEP_DISABLE();
        spi_pins = (spi_pins_t) {
            OMV_SPI5_SCLK_PIN, OMV_SPI5_MISO_PIN, OMV_SPI5_MOSI_PIN, OMV_SPI5_SSEL_PIN
        };
    #endif
    #if defined(SPI6_ID)
    } else if (hspi->Instance == SPI6) {
        __HAL_RCC_SPI6_FORCE_RESET();
        __HAL_RCC_SPI6_RELEASE_RESET();
        __HAL_RCC_SPI6_CLK_DISABLE();
        __HAL_RCC_SPI6_CLK_SLEEP_DISABLE();
        spi_pins = (spi_pins_t) {
            SPI6_SCLK_PIN, SPI6_MISO_PIN, SPI6_MOSI_PIN, SPI6_SSEL_PIN
        };
    #endif
    } else {
        return;
    }

    omv_gpio_deinit(spi_pins.sclk_pin);
    if (hspi->Init.Direction == SPI_DIRECTION_2LINES || hspi->Init.Direction == SPI_DIRECTION_2LINES_RXONLY) {
        omv_gpio_deinit(spi_pins.miso_pin);
    }
    #if defined(STM32H7)
    if (hspi->Init.Direction == SPI_DIRECTION_2LINES || hspi->Init.Direction == SPI_DIRECTION_2LINES_TXONLY) {
    #else
    if (hspi->Init.Direction == SPI_DIRECTION_2LINES || hspi->Init.Direction == SPI_DIRECTION_1LINE) {
    #endif
        omv_gpio_deinit(spi_pins.mosi_pin);
    }
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
    if (hsai->Instance == OMV_SAI) {
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
#elif defined(OMV_MDF)
void HAL_MDF_MspInit(MDF_HandleTypeDef *hmdf) {
    if (hmdf->Instance == OMV_MDF) {
        if (IS_ADF_INSTANCE(hmdf->Instance)) {
            __HAL_RCC_ADF1_CLK_ENABLE();
            __HAL_RCC_ADF1_CLK_SLEEP_ENABLE();
        } else {
            __HAL_RCC_MDF1_CLK_ENABLE();
            __HAL_RCC_MDF1_CLK_SLEEP_ENABLE();
        }

        omv_gpio_config(OMV_MDF_CK_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_config(OMV_MDF_D1_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    }
}

void HAL_MDF_MspDeInit(MDF_HandleTypeDef *hmdf) {
    if (hmdf->Instance == OMV_MDF) {
        if (IS_ADF_INSTANCE(hmdf->Instance)) {
            __HAL_RCC_ADF1_CLK_DISABLE();
            __HAL_RCC_ADF1_CLK_SLEEP_DISABLE();
        } else {
            __HAL_RCC_MDF1_CLK_DISABLE();
            __HAL_RCC_MDF1_CLK_SLEEP_DISABLE();
        }

        omv_gpio_deinit(OMV_MDF_CK_PIN);
        omv_gpio_deinit(OMV_MDF_D1_PIN);
    }
}
#endif

void HAL_CRC_MspInit(CRC_HandleTypeDef *hcrc) {
    __HAL_RCC_CRC_CLK_ENABLE();
    __HAL_RCC_CRC_CLK_SLEEP_ENABLE();
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef *hcrc) {
    __HAL_RCC_CRC_CLK_DISABLE();
    __HAL_RCC_CRC_CLK_SLEEP_DISABLE();
}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef *hdma2d) {
    __HAL_RCC_DMA2D_CLK_ENABLE();
    __HAL_RCC_DMA2D_CLK_SLEEP_ENABLE();
}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef *hdma2d) {
    __HAL_RCC_DMA2D_FORCE_RESET();
    __HAL_RCC_DMA2D_RELEASE_RESET();

    __HAL_RCC_DMA2D_CLK_DISABLE();
    __HAL_RCC_DMA2D_CLK_SLEEP_DISABLE();
}

#if defined(GPU2D)
void HAL_GPU2D_MspInit(GPU2D_HandleTypeDef *hgpu2d) {
    __HAL_RCC_GPU2D_FORCE_RESET();
    __HAL_RCC_GPU2D_RELEASE_RESET();

    __HAL_RCC_GPU2D_CLK_ENABLE();
    __HAL_RCC_GPU2D_CLK_SLEEP_ENABLE();
}

void HAL_GPU2D_MspDeInit(GPU2D_HandleTypeDef *hgpu2d) {
    __HAL_RCC_GPU2D_CLK_DISABLE();
    __HAL_RCC_GPU2D_CLK_SLEEP_DISABLE();
}
#endif

#if defined(GFXMMU)
void HAL_GFXMMU_MspInit(GFXMMU_HandleTypeDef *hgfxmmu) {
    __HAL_RCC_GFXMMU_CLK_ENABLE();
    __HAL_RCC_GFXMMU_CLK_SLEEP_ENABLE();
}

void HAL_GFXMMU_MspDeInit(GFXMMU_HandleTypeDef *hgfxmmu) {
    __HAL_RCC_GFXMMU_CLK_DISABLE();
    __HAL_RCC_GFXMMU_CLK_SLEEP_DISABLE();
}
#endif

#if (OMV_JPEG_CODEC_ENABLE == 1)
void HAL_JPEG_MspInit(JPEG_HandleTypeDef *hjpeg) {
    __HAL_RCC_JPEG_CLK_ENABLE();
    __HAL_RCC_JPEG_CLK_SLEEP_ENABLE();
}

void HAL_JPEG_MspDeInit(JPEG_HandleTypeDef *hjpeg) {
    __HAL_RCC_JPEG_FORCE_RESET();
    __HAL_RCC_JPEG_RELEASE_RESET();
    __HAL_RCC_JPEG_CLK_DISABLE();
    __HAL_RCC_JPEG_CLK_SLEEP_DISABLE();
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

#if defined(OMV_XSPI1_IO00_PIN)
void HAL_XSPI_MspInit(XSPI_HandleTypeDef *hxspi) {
    const omv_gpio_t xspi_pins[] = {
        OMV_XSPI1_IO00_PIN,
        OMV_XSPI1_IO01_PIN,
        OMV_XSPI1_IO02_PIN,
        OMV_XSPI1_IO03_PIN,
        OMV_XSPI1_IO04_PIN,
        OMV_XSPI1_IO05_PIN,
        OMV_XSPI1_IO06_PIN,
        OMV_XSPI1_IO07_PIN,
        OMV_XSPI1_IO08_PIN,
        OMV_XSPI1_IO09_PIN,
        OMV_XSPI1_IO10_PIN,
        OMV_XSPI1_IO11_PIN,
        OMV_XSPI1_IO12_PIN,
        OMV_XSPI1_IO13_PIN,
        OMV_XSPI1_IO14_PIN,
        OMV_XSPI1_IO15_PIN,
        OMV_XSPI1_NCS1_PIN,
        OMV_XSPI1_DQS0_PIN,
        OMV_XSPI1_DQS1_PIN,
        OMV_XSPI1_CLKP_PIN,
    };

    // Reset and enable XSPI clock.
    if (hxspi->Instance == XSPI1) {
        __HAL_RCC_XSPI1_FORCE_RESET();
        __HAL_RCC_XSPI1_RELEASE_RESET();
        __HAL_RCC_XSPI1_CLK_ENABLE();
        __HAL_RCC_XSPI1_CLK_SLEEP_ENABLE();
    } else if (hxspi->Instance == XSPI2) {
        __HAL_RCC_XSPI2_FORCE_RESET();
        __HAL_RCC_XSPI2_RELEASE_RESET();
        __HAL_RCC_XSPI2_CLK_ENABLE();
        __HAL_RCC_XSPI2_CLK_SLEEP_ENABLE();
    } else if (hxspi->Instance == XSPI3) {
        __HAL_RCC_XSPI3_FORCE_RESET();
        __HAL_RCC_XSPI3_RELEASE_RESET();
        __HAL_RCC_XSPI3_CLK_ENABLE();
        __HAL_RCC_XSPI3_CLK_SLEEP_ENABLE();
    }

    for (int i = 0; i < OMV_ARRAY_SIZE(xspi_pins); i++) {
        omv_gpio_config(xspi_pins[i], OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MAX, -1);
    }
}

void HAL_XSPI_MspDeInit(XSPI_HandleTypeDef *hxspi) {
    const omv_gpio_t xspi_pins[] = {
        OMV_XSPI1_IO00_PIN,
        OMV_XSPI1_IO01_PIN,
        OMV_XSPI1_IO02_PIN,
        OMV_XSPI1_IO03_PIN,
        OMV_XSPI1_IO04_PIN,
        OMV_XSPI1_IO05_PIN,
        OMV_XSPI1_IO06_PIN,
        OMV_XSPI1_IO07_PIN,
        OMV_XSPI1_IO08_PIN,
        OMV_XSPI1_IO09_PIN,
        OMV_XSPI1_IO10_PIN,
        OMV_XSPI1_IO11_PIN,
        OMV_XSPI1_IO12_PIN,
        OMV_XSPI1_IO13_PIN,
        OMV_XSPI1_IO14_PIN,
        OMV_XSPI1_IO15_PIN,
        OMV_XSPI1_NCS1_PIN,
        OMV_XSPI1_DQS0_PIN,
        OMV_XSPI1_DQS1_PIN,
        OMV_XSPI1_CLKP_PIN,
    };

    // Reset and enable XSPI clock.
    if (hxspi->Instance == XSPI1) {
        __HAL_RCC_XSPI1_FORCE_RESET();
        __HAL_RCC_XSPI1_RELEASE_RESET();
        __HAL_RCC_XSPI1_CLK_DISABLE();
        __HAL_RCC_XSPI1_CLK_SLEEP_DISABLE();
    } else if (hxspi->Instance == XSPI2) {
        __HAL_RCC_XSPI2_FORCE_RESET();
        __HAL_RCC_XSPI2_RELEASE_RESET();
        __HAL_RCC_XSPI2_CLK_DISABLE();
        __HAL_RCC_XSPI2_CLK_SLEEP_DISABLE();
    } else if (hxspi->Instance == XSPI3) {
        __HAL_RCC_XSPI3_FORCE_RESET();
        __HAL_RCC_XSPI3_RELEASE_RESET();
        __HAL_RCC_XSPI3_CLK_DISABLE();
        __HAL_RCC_XSPI3_CLK_SLEEP_DISABLE();
    }

    for (int i = 0; i < OMV_ARRAY_SIZE(xspi_pins); i++) {
        omv_gpio_deinit(xspi_pins[i]);
    }
}
#endif

void HAL_MspDeInit(void) {

}

void MDMA_IRQHandler() {
    IRQ_ENTER(MDMA_IRQn);
    #if defined(OMV_MDMA_CHANNEL_JPEG_IN)
    extern void jpeg_mdma_irq_handler(void);
    jpeg_mdma_irq_handler();
    #endif
    #if defined(OMV_MDMA_CHANNEL_DCMI_0)
    extern void omv_csi_mdma_irq_handler(void);
    omv_csi_mdma_irq_handler();
    #endif
    IRQ_EXIT(MDMA_IRQn);
}
