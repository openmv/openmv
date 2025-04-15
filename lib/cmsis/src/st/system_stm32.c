/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 * Copyright (c) 2013-2024 STMicroelectronics.
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
 * STM32 system initialization code.
 */
#include STM32_HAL_H
#include "omv_boardconfig.h"

// This variable is updated in two ways:
// 1) by calling HAL API function HAL_RCC_GetHCLKFreq()
// 2) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
uint32_t SystemCoreClock = HSI_VALUE;

#if defined(STM32H7)
#define CONFIG_RCC_CR_1ST   (RCC_CR_HSION)
#define CONFIG_RCC_CR_2ND   (0xEAF6ED7F)
#define CONFIG_RCC_PLLCFGR  (0x00000000)

uint32_t SystemD2Clock = 64000000;
const uint8_t D1CorePrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};
#elif defined(STM32F4) || defined(STM32F7)
#define CONFIG_RCC_CR_1ST   (RCC_CR_HSION)
#define CONFIG_RCC_CR_2ND   (0xFEF6FFFF)
#define CONFIG_RCC_PLLCFGR  (0x24003010)

const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};
const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
#endif

extern void __fatal_error(const char *msg);

void SystemInit(void) {
    // Configure IRQ vector table address.
    SCB->VTOR = OMV_VTOR_BASE;

    // FPU settings
    #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));  // Set CP10 and CP11 Full Access
    #endif

    #if defined(STM32F4) || defined(STM32F7) || defined(STM32H7)
    // Enable to allow other cores to interrupt during WFI/WFE.
    //SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

    // Set HSION bit
    RCC->CR |= CONFIG_RCC_CR_1ST;

    // Reset CFGR register
    RCC->CFGR = 0x00000000;

    // Reset HSEON, CSSON and PLLON
    RCC->CR &= (uint32_t) CONFIG_RCC_CR_2ND;

    // Reset PLLCFGR register
    RCC->PLLCFGR = CONFIG_RCC_PLLCFGR;

    #if defined(STM32H7)
    // Reset D1CFGR register
    RCC->D1CFGR = 0x00000000;

    // Reset D2CFGR register
    RCC->D2CFGR = 0x00000000;

    // Reset D3CFGR register
    RCC->D3CFGR = 0x00000000;

    // Reset PLLCKSELR register
    RCC->PLLCKSELR = 0x00000000;

    // Reset PLL1DIVR register
    RCC->PLL1DIVR = 0x00000000;

    // Reset PLL1FRACR register
    RCC->PLL1FRACR = 0x00000000;

    // Reset PLL2DIVR register
    RCC->PLL2DIVR = 0x00000000;

    // Reset PLL2FRACR register
    RCC->PLL2FRACR = 0x00000000;

    // Reset PLL3DIVR register
    RCC->PLL3DIVR = 0x00000000;

    // Reset PLL3FRACR register
    RCC->PLL3FRACR = 0x00000000;
    #endif // defined(STM32H7)

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;
    #endif // STM32F4 || STM32F7 || STM32H7

    // Disable all interrupts
    #if defined(STM32F4) || defined(STM32F7)
    RCC->CIR = 0x00000000;
    #elif defined(STM32L4) || defined(STM32H7)
    RCC->CIER = 0x00000000;
    #endif

    #if defined(STM32H7)
    // Change the switch matrix read issuing capability to 1 for AXI SRAM.
    // Errata 2.2.9 "Reading from AXI SRAM may lead to data read corruption"
    *((__IO uint32_t*)0x51008108) = 0x00000001;
    #endif // defined(STM32H7)

    // Enable 8-byte stack alignment for IRQ handlers, in accord with EABI
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;

    #if !defined(NDEBUG)
    #if defined(STM32F4) ||  defined(STM32F7)
    DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP;
    #elif defined(STM32H7)
    DBGMCU->CR |= DBGMCU_CR_DBG_SLEEPD1;
    DBGMCU->CR |= DBGMCU_CR_DBG_STOPD1;
    DBGMCU->CR |= DBGMCU_CR_DBG_STANDBYD1;
    #endif
    #endif
}

void SystemClock_Config(void) {
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    #if defined(STM32H7)
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    #endif

    #if defined(STM32H7)
    // Supply configuration update enable
    HAL_PWREx_ConfigSupply(OMV_PWR_SUPPLY);
    #elif defined(STM32F4) || defined(STM32F7)
    // Enable Power Control clock
    __PWR_CLK_ENABLE();
    #endif

    // Configure voltage scaling.
    #if defined(STM32H7)
    // Enable VSCALE0 for revision V devices.
    if (HAL_GetREVID() >= 0x2003) {
        __HAL_RCC_SYSCFG_CLK_ENABLE();
        __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
    } else {
    #else
    if (1) {
    #endif
        __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    }

    // Wait for PWR_FLAG_VOSRDY
    #if defined(STM32H7)
    while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY) == RESET) {
    }
    #endif

    #if defined(OMV_OSC_LSE_DRIVE)
    // Configure LSE drive strength.
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(OMV_OSC_LSE_DRIVE);
    HAL_PWR_DisableBkUpAccess();
    #endif

    // Configure and enable oscillators
    #if defined(OMV_OSC_LSE_STATE)
    RCC_OscInitStruct.LSEState = OMV_OSC_LSE_STATE;
    RCC_OscInitStruct.OscillatorType |= RCC_OSCILLATORTYPE_LSE;
    #endif
    #if defined(OMV_OSC_HSE_STATE)
    RCC_OscInitStruct.HSEState = OMV_OSC_HSE_STATE;
    RCC_OscInitStruct.OscillatorType |= RCC_OSCILLATORTYPE_HSE;
    #endif
    #if defined(OMV_OSC_LSI_STATE)
    RCC_OscInitStruct.LSIState = OMV_OSC_LSI_STATE;
    RCC_OscInitStruct.OscillatorType |= RCC_OSCILLATORTYPE_LSI;
    #endif
    #if defined(OMV_OSC_HSI_STATE)
    RCC_OscInitStruct.HSIState = OMV_OSC_HSI_STATE;
    RCC_OscInitStruct.OscillatorType |= RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIDiv = OMV_OSC_HSI_CAL;
    RCC_OscInitStruct.HSICalibrationValue = OMV_OSC_HSI_DIV;
    #endif
    #if defined(OMV_OSC_CSI_STATE)
    RCC_OscInitStruct.CSIState = OMV_OSC_CSI_STATE;
    RCC_OscInitStruct.OscillatorType |= RCC_OSCILLATORTYPE_CSI;
    #endif
    #if defined(OMV_OSC_HSI48_STATE)
    RCC_OscInitStruct.HSI48State = OMV_OSC_HSI48_STATE;
    RCC_OscInitStruct.OscillatorType |= RCC_OSCILLATORTYPE_HSI48;
    #endif

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = OMV_OSC_PLL_CLKSOURCE;
    RCC_OscInitStruct.PLL.PLLM = OMV_OSC_PLL1M;
    RCC_OscInitStruct.PLL.PLLN = OMV_OSC_PLL1N;
    RCC_OscInitStruct.PLL.PLLQ = OMV_OSC_PLL1Q;
    RCC_OscInitStruct.PLL.PLLP = OMV_OSC_PLL1P;

    #if defined(STM32H7)
    // Override PLL1 frequency for revision Y devices,
    // with maximum frequency of 400MHz CPU 200MHz Bus.
    if (HAL_GetREVID() < 0x2003) {
        // 400MHz/200MHz
        RCC_OscInitStruct.PLL.PLLN = 200;
        RCC_OscInitStruct.PLL.PLLQ = 16;
    }
    #endif

    #if defined(OMV_OSC_PLL1R)
    RCC_OscInitStruct.PLL.PLLR = OMV_OSC_PLL1R;
    #endif
    #if defined(OMV_OSC_PLL1VCI)
    RCC_OscInitStruct.PLL.PLLRGE = OMV_OSC_PLL1VCI;
    #endif
    #if defined(OMV_OSC_PLL1VCO)
    RCC_OscInitStruct.PLL.PLLVCOSEL = OMV_OSC_PLL1VCO;
    #endif
    #if defined(OMV_OSC_PLL1FRAC)
    RCC_OscInitStruct.PLL.PLLFRACN = OMV_OSC_PLL1FRAC;
    #endif

    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        // Initialization Error
        __fatal_error("HAL_RCC_OscConfig");
    }

    #if defined(OMV_OSC_PLL2M)
    PeriphClkInitStruct.PLL2.PLL2M = OMV_OSC_PLL2M;
    PeriphClkInitStruct.PLL2.PLL2N = OMV_OSC_PLL2N;
    PeriphClkInitStruct.PLL2.PLL2P = OMV_OSC_PLL2P;
    PeriphClkInitStruct.PLL2.PLL2Q = OMV_OSC_PLL2Q;
    PeriphClkInitStruct.PLL2.PLL2R = OMV_OSC_PLL2R;
    PeriphClkInitStruct.PLL2.PLL2RGE = OMV_OSC_PLL2VCI;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = OMV_OSC_PLL2VCO;
    PeriphClkInitStruct.PLL2.PLL2FRACN = OMV_OSC_PLL2FRAC;
    #endif

    #if defined(OMV_OSC_PLL3M)
    PeriphClkInitStruct.PLL3.PLL3M = OMV_OSC_PLL3M;
    PeriphClkInitStruct.PLL3.PLL3N = OMV_OSC_PLL3N;
    PeriphClkInitStruct.PLL3.PLL3P = OMV_OSC_PLL3P;
    PeriphClkInitStruct.PLL3.PLL3Q = OMV_OSC_PLL3Q;
    PeriphClkInitStruct.PLL3.PLL3R = OMV_OSC_PLL3R;
    PeriphClkInitStruct.PLL3.PLL3RGE = OMV_OSC_PLL3VCI;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = OMV_OSC_PLL3VCO;
    PeriphClkInitStruct.PLL3.PLL3FRACN = OMV_OSC_PLL3FRAC;
    #endif

    #if defined(STM32H7)

    #if !defined(OMV_OMVPT_ERRATA_RTC)
    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_RTC;
    #if defined(OMV_OSC_RTC_CLKSOURCE)
    PeriphClkInitStruct.RTCClockSelection     = OMV_OSC_RTC_CLKSOURCE;
    #else
    PeriphClkInitStruct.RTCClockSelection     = RCC_RTCCLKSOURCE_LSI;
    #endif
    #endif

    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_RNG;
    PeriphClkInitStruct.RngClockSelection     = OMV_OSC_RNG_CLKSOURCE;

    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_USB;
    PeriphClkInitStruct.UsbClockSelection     = OMV_OSC_USB_CLKSOURCE;

    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_FMC;
    PeriphClkInitStruct.FmcClockSelection     = RCC_FMCCLKSOURCE_PLL2;

    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_ADC;
    PeriphClkInitStruct.AdcClockSelection     = OMV_OSC_ADC_CLKSOURCE;

    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_SDMMC;
    PeriphClkInitStruct.SdmmcClockSelection   = RCC_SDMMCCLKSOURCE_PLL;

    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.FdcanClockSelection   = RCC_FDCANCLKSOURCE_PLL;

    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_QSPI;
    PeriphClkInitStruct.QspiClockSelection    = RCC_QSPICLKSOURCE_PLL2;

    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_SPI123;
    PeriphClkInitStruct.Spi123ClockSelection  = OMV_OSC_SPI123_CLKSOURCE;

    #if defined(OMV_OSC_SPI6_CLKSOURCE)
    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_SPI6;
    PeriphClkInitStruct.Spi6ClockSelection    = OMV_OSC_SPI6_CLKSOURCE;
    #endif

    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_I2C123;
    PeriphClkInitStruct.I2c123ClockSelection  = RCC_I2C123CLKSOURCE_D2PCLK1;

    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_SAI4A;
    PeriphClkInitStruct.Sai4AClockSelection   = RCC_SAI4ACLKSOURCE_PLL;

    #if defined(OMV_OSC_DFSDM1_CLKSOURCE)
    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_DFSDM1;
    PeriphClkInitStruct.Dfsdm1ClockSelection  = OMV_OSC_DFSDM1_CLKSOURCE;
    #endif

    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_SPI45;
    #if defined(OMV_OSC_SPI45_CLKSOURCE)
    PeriphClkInitStruct.Spi45ClockSelection   = OMV_OSC_SPI45_CLKSOURCE;
    #else
    PeriphClkInitStruct.Spi45ClockSelection   = RCC_SPI45CLKSOURCE_PLL2;
    #endif

    #if defined(DSI)
    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_DSI;
    #if defined(OMV_OSC_DSI_CLKSOURCE)
    PeriphClkInitStruct.DsiClockSelection = OMV_OSC_DSI_CLKSOURCE;
    #else
    PeriphClkInitStruct.DsiClockSelection = RCC_DSICLKSOURCE_PHY;
    #endif // defined(OMV_OSC_DSI_CLKSOURCE)
    #endif // defined(DSI)

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        // Initialization Error
        __fatal_error("HAL_RCCEx_PeriphCLKConfig");
    }
    #endif // defined(STM32H7)

    #if defined(STM32F4) || defined(STM32F7)
    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        // Initialization Error
        __fatal_error("HAL_PWREx_EnableOverDrive");
    }
    #endif

    // Configure CPU and System bus clock source.
    #if defined(STM32H7)
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 |
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
    #elif defined(STM32F4) || defined(STM32F7)
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
                                   RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    #endif

    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, OMV_FLASH_LATENCY) != HAL_OK) {
        // Initialization Error
        __fatal_error("HAL_RCC_ClockConfig");
    }

    #if defined(STM32H7)
    // Activate CSI clock mondatory for I/O Compensation Cell
    __HAL_RCC_CSI_ENABLE() ;

    // Enable SYSCFG clock mondatory for I/O Compensation Cell
    __HAL_RCC_SYSCFG_CLK_ENABLE() ;

    // Enables the I/O Compensation Cell
    HAL_EnableCompensationCell();

    // Enable the USB voltage level detector
    HAL_PWREx_EnableUSBVoltageDetector();
    #endif
}
