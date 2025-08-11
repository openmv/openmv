/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2024 OpenMV, LLC.
 * Copyright (c) 2024 STMicroelectronics.
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
#include <string.h>
#include "omv_boardconfig.h"
// Trustzone-M core secure attributes
#include "partition_stm32n6xx.h"

#define BSEC_HW_CONFIG_ID        124U
#define BSEC_HWS_HSLV_VDDIO3     (1U<<15)
#define BSEC_HWS_HSLV_VDDIO2     (1U<<16)

// This variable is updated in two ways:
// 1) by calling HAL API function HAL_RCC_GetHCLKFreq()
// 2) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
uint32_t SystemCoreClock = HSI_VALUE;

extern void __fatal_error(const char *msg);

#if !defined(OMV_BOOTLOADER)//!defined(CPU_IN_SECURE_STATE)
void SystemInit(void) {
    // Configure IRQ vector table address.
    SCB->VTOR = OMV_VTOR_BASE;

    // Enable RAMCFG clock.
    RCC->AHB2ENR |= RCC_AHB2ENR_RAMCFGEN;
    (void) RCC->AHB2ENR;

    // Enable SRAM3 to SRAM6
    RAMCFG_SRAM3_AXI->CR &= ~(RAMCFG_AXISRAM_POWERDOWN);
    (void) RAMCFG_SRAM3_AXI->CR;
    RAMCFG_SRAM4_AXI->CR &= ~(RAMCFG_AXISRAM_POWERDOWN);
    (void) RAMCFG_SRAM4_AXI->CR;
    RAMCFG_SRAM5_AXI->CR &= ~(RAMCFG_AXISRAM_POWERDOWN);
    (void) RAMCFG_SRAM5_AXI->CR;
    RAMCFG_SRAM6_AXI->CR &= ~(RAMCFG_AXISRAM_POWERDOWN);
    (void) RAMCFG_SRAM6_AXI->CR;

    // Disable RAMCFG clock.
    RCC->AHB2ENR &= ~(RCC_AHB2ENR_RAMCFGEN);
}

void SystemCoreClockUpdate(void) {
    // Implemented in HAL no need to duplicate it.
    SystemCoreClock = HAL_RCC_GetCpuClockFreq();
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInit = {0};

    // Leave PLLs unchanged.
    RCC_OscInit.PLL1.PLLState = RCC_PLL_NONE;
    RCC_OscInit.PLL2.PLLState = RCC_PLL_NONE;
    RCC_OscInit.PLL3.PLLState = RCC_PLL_NONE;
    RCC_OscInit.PLL4.PLLState = RCC_PLL_NONE;
    //RCC_OscInit.OscillatorType = RCC_OSCILLATORTYPE_NONE;

    // Configure extra PLLs.
    #if defined(OMV_OSC_PLL3SOURCE)
    RCC_OscInit.PLL3.PLLState = RCC_PLL_ON;
    RCC_OscInit.PLL3.PLLSource = OMV_OSC_PLL3SOURCE;
    RCC_OscInit.PLL3.PLLM = OMV_OSC_PLL3M;
    RCC_OscInit.PLL3.PLLN = OMV_OSC_PLL3N;
    RCC_OscInit.PLL3.PLLP1 = OMV_OSC_PLL3P1;
    RCC_OscInit.PLL3.PLLP2 = OMV_OSC_PLL3P2;
    RCC_OscInit.PLL3.PLLFractional = OMV_OSC_PLL3FRAC;
    #endif

    #if defined(OMV_OSC_PLL4SOURCE)
    RCC_OscInit.PLL4.PLLState = RCC_PLL_ON;
    RCC_OscInit.PLL4.PLLSource = OMV_OSC_PLL4SOURCE;
    RCC_OscInit.PLL4.PLLM = OMV_OSC_PLL4M;
    RCC_OscInit.PLL4.PLLN = OMV_OSC_PLL4N;
    RCC_OscInit.PLL4.PLLP1 = OMV_OSC_PLL4P1;
    RCC_OscInit.PLL4.PLLP2 = OMV_OSC_PLL4P2;
    RCC_OscInit.PLL4.PLLFractional = OMV_OSC_PLL4FRAC;
    #endif

    // Configure enabled oscillators.
    #if defined(OMV_OSC_LSE_STATE)
    RCC_OscInit.LSEState = OMV_OSC_LSE_STATE;
    RCC_OscInit.OscillatorType |= RCC_OSCILLATORTYPE_LSE;
    #endif
    #if defined(OMV_OSC_HSE_STATE)
    RCC_OscInit.HSEState = OMV_OSC_HSE_STATE;
    RCC_OscInit.OscillatorType |= RCC_OSCILLATORTYPE_HSE;
    #endif
    #if defined(OMV_OSC_LSI_STATE)
    RCC_OscInit.LSIState = OMV_OSC_LSI_STATE;
    RCC_OscInit.OscillatorType |= RCC_OSCILLATORTYPE_LSI;
    #endif
    #if defined(OMV_OSC_HSI_STATE)
    RCC_OscInit.HSIState = OMV_OSC_HSI_STATE;
    RCC_OscInit.OscillatorType |= RCC_OSCILLATORTYPE_HSI;
    RCC_OscInit.HSIDiv = OMV_OSC_HSI_DIV;
    RCC_OscInit.HSICalibrationValue = OMV_OSC_HSI_CAL;
    #endif

    if (HAL_RCC_OscConfig(&RCC_OscInit) != HAL_OK) {
        __fatal_error("HAL_RCC_OscConfig");
    }

    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    // Peripherals clocks configuration.
    #if defined(OMV_OSC_I2C3_SOURCE)
    PeriphClkInit.PeriphClockSelection   |= RCC_PERIPHCLK_I2C3;
    PeriphClkInit.I2c3ClockSelection      = OMV_OSC_I2C3_SOURCE;
    #endif

    #if defined(OMV_OSC_SPI5_SOURCE)
    PeriphClkInit.PeriphClockSelection   |= RCC_PERIPHCLK_SPI5;
    PeriphClkInit.Spi5ClockSelection      = OMV_OSC_SPI5_SOURCE;
    #endif

    #if defined(OMV_OSC_USART2_SOURCE)
    PeriphClkInit.PeriphClockSelection   |= RCC_PERIPHCLK_USART2;
    PeriphClkInit.Usart2ClockSelection    = OMV_OSC_USART2_SOURCE;
    #endif

    #if defined(OMV_OSC_DCMIPP_SOURCE)
    PeriphClkInit.PeriphClockSelection   |= RCC_PERIPHCLK_DCMIPP;
    PeriphClkInit.DcmippClockSelection    = OMV_OSC_DCMIPP_SOURCE;
    #endif

    #if defined(OMV_OSC_CSI_SOURCE)
    PeriphClkInit.PeriphClockSelection   |= RCC_PERIPHCLK_CSI;
    //PeriphClkInit.CsiClockSelection     = OMV_OSC_DCMIPP_SOURCE;
    #endif

    #if defined(OMV_OSC_MDF1_SOURCE)
    PeriphClkInit.PeriphClockSelection   |= RCC_PERIPHCLK_MDF1;
    PeriphClkInit.Mdf1ClockSelection      = OMV_OSC_MDF1_SOURCE;
    #endif

    #if defined(OMV_OSC_ADF1_SOURCE)
    PeriphClkInit.PeriphClockSelection   |= RCC_PERIPHCLK_ADF1;
    PeriphClkInit.Adf1ClockSelection      = OMV_OSC_ADF1_SOURCE;
    #endif

    PeriphClkInit.PeriphClockSelection   |= RCC_PERIPHCLK_XSPI1;
    PeriphClkInit.Xspi1ClockSelection     = RCC_XSPI1CLKSOURCE_HCLK;

    PeriphClkInit.PeriphClockSelection   |= RCC_PERIPHCLK_SDMMC1;
    PeriphClkInit.Sdmmc1ClockSelection    = RCC_SDMMC1CLKSOURCE_HCLK;

    PeriphClkInit.PeriphClockSelection   |= RCC_PERIPHCLK_SDMMC2;
    PeriphClkInit.Sdmmc2ClockSelection    = RCC_SDMMC2CLKSOURCE_HCLK;

    #if defined(OMV_RCC_IC0_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC0].ClockSelection = OMV_RCC_IC0_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC0].ClockDivider   = OMV_RCC_IC0_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC1_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC1].ClockSelection = OMV_RCC_IC1_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC1].ClockDivider   = OMV_RCC_IC1_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC2_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC2].ClockSelection = OMV_RCC_IC2_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC2].ClockDivider   = OMV_RCC_IC2_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC3_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC3].ClockSelection = OMV_RCC_IC3_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC3].ClockDivider   = OMV_RCC_IC3_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC4_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC4].ClockSelection = OMV_RCC_IC4_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC4].ClockDivider   = OMV_RCC_IC4_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC5_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC5].ClockSelection = OMV_RCC_IC5_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC5].ClockDivider   = OMV_RCC_IC5_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC6_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC6].ClockSelection = OMV_RCC_IC6_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC6].ClockDivider   = OMV_RCC_IC6_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC7_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC7].ClockSelection = OMV_RCC_IC7_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC7].ClockDivider   = OMV_RCC_IC7_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC8_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC8].ClockSelection = OMV_RCC_IC8_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC8].ClockDivider   = OMV_RCC_IC8_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC9_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC9].ClockSelection = OMV_RCC_IC9_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC9].ClockDivider   = OMV_RCC_IC9_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC10_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC10].ClockSelection = OMV_RCC_IC10_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC10].ClockDivider   = OMV_RCC_IC10_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC11_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC11].ClockSelection = OMV_RCC_IC11_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC11].ClockDivider   = OMV_RCC_IC11_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC12_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC12].ClockSelection = OMV_RCC_IC12_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC12].ClockDivider   = OMV_RCC_IC12_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC13_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC13].ClockSelection = OMV_RCC_IC13_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC13].ClockDivider   = OMV_RCC_IC13_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC14_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC14].ClockSelection = OMV_RCC_IC14_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC14].ClockDivider   = OMV_RCC_IC14_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC15_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC15].ClockSelection = OMV_RCC_IC15_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC15].ClockDivider   = OMV_RCC_IC15_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC16_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC16].ClockSelection = OMV_RCC_IC16_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC16].ClockDivider   = OMV_RCC_IC16_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC17_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC17].ClockSelection = OMV_RCC_IC17_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC17].ClockDivider   = OMV_RCC_IC17_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC18_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC18].ClockSelection = OMV_RCC_IC18_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC18].ClockDivider   = OMV_RCC_IC18_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC19_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC19].ClockSelection = OMV_RCC_IC19_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC19].ClockDivider   = OMV_RCC_IC19_CLKDIV;
    #endif
    
    #if defined(OMV_RCC_IC20_SOURCE)
    PeriphClkInit.ICSelection[RCC_IC20].ClockSelection = OMV_RCC_IC20_SOURCE;
    PeriphClkInit.ICSelection[RCC_IC20].ClockDivider   = OMV_RCC_IC20_CLKDIV;
    #endif

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        __fatal_error("HAL_RCCEx_PeriphCLKConfig");
    }
}
#else
void SystemInit(void) {
    // Configure IRQ vector table address.
    SCB->VTOR = OMV_VTOR_BASE;

    // Reset RNG
    RCC->AHB3RSTSR = RCC_AHB3RSTSR_RNGRSTS;
    RCC->AHB3RSTCR = RCC_AHB3RSTCR_RNGRSTC;
    // Disable RNG clock
    RCC->AHB3ENCR = RCC_AHB3ENCR_RNGENC;

    // Clear SAU.
    for (size_t i=0; i<SAU_REGIONS_MAX; i++) {
        SAU->RNR = i;
        SAU->RBAR = 0;
        SAU->RLAR = 0;
    }

    // System configuration setup
    RCC->APB4ENSR2 = RCC_APB4ENSR2_SYSCFGENS;
    // Delay after an RCC peripheral clock enabling
    (void)RCC->APB4ENR2;

    // Set default Vector Table location after system reset or return from Standby
    SYSCFG->INITSVTORCR = SCB->VTOR;

    // Enable VDDADC CLAMP
    PWR->SVMCR3 |= PWR_SVMCR3_ASV;
    PWR->SVMCR3 |= PWR_SVMCR3_AVMEN;
    // Read back the register to make sure that the transaction has taken place
    (void) PWR->SVMCR3;
    // Enable VREF
    RCC->APB4ENR1 |= RCC_APB4ENR1_VREFBUFEN;

    // RCC Fix to lower power consumption
    RCC->APB4ENR2 |= 0x00000010UL;
    (void) RCC->APB4ENR2;
    RCC->APB4ENR2 &= ~(0x00000010UL);

    // XSPI2 & XSPIM reset
    RCC->AHB5RSTSR = RCC_AHB5RSTSR_XSPIMRSTS | RCC_AHB5RSTSR_XSPI2RSTS;
    RCC->AHB5RSTCR = RCC_AHB5RSTCR_XSPIMRSTC | RCC_AHB5RSTCR_XSPI2RSTC;

    // TIM2 reset
    RCC->APB1RSTSR1 = RCC_APB1RSTSR1_TIM2RSTS;
    RCC->APB1RSTCR1 = RCC_APB1RSTCR1_TIM2RSTC;

    // Disable TIM2 clock
    RCC->APB1ENCR1 = RCC_APB1ENCR1_TIM2ENC;

    // Disable GPIOG clock
    RCC->AHB4ENCR = RCC_AHB4ENCR_GPIOGENC;
    (void) SYSCFG->INITSVTORCR;

    // Disable SYSCFG clock
    RCC->APB4ENCR2 = RCC_APB4ENCR2_SYSCFGENC;

    #if defined(USER_TZ_SAU_SETUP)
    // SAU/IDAU, FPU and Interrupts secure/non-secure allocation settings
    TZ_SAU_Setup();
    #endif // USER_TZ_SAU_SETUP

    // FPU settings
    #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));      // Set CP10 and CP11 Full Access
    SCB_NS->CPACR |= ((3UL << 20U)|(3UL << 22U));   // Set CP10 and CP11 Full Access
    #endif

    // Enable NS debug
    BSEC_HandleTypeDef hbsec = {
        .Instance = BSEC
    };

    BSEC_DebugCfgTypeDef debug_config = {
        .Sec_Dbg_Auth = HAL_BSEC_SEC_DBG_AUTH,
        .NonSec_Dbg_Auth = HAL_BSEC_NONSEC_DBG_AUTH,
        .HDPL_Open_Dbg = HAL_BSEC_OPEN_DBG_LEVEL_0,
    };

    // Enable BSEC clock.
    __HAL_RCC_BSEC_CLK_ENABLE();

    // NOTE: BSEC_DBGCR is writeable only once per reset.
    HAL_BSEC_UnlockDebug(&hbsec);
    HAL_BSEC_ConfigDebug(&hbsec, &debug_config);
}

void SystemCoreClockUpdate(void) {
    // Implemented in HAL no need to duplicate it.
    SystemCoreClock = HAL_RCC_GetCpuClockFreq();
}

void SystemClock_Config(void) {
    RCC_ClkInitTypeDef RCC_ClkInit = {0};
    RCC_OscInitTypeDef RCC_OscInit = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    // Enable PWR clock.
    __HAL_RCC_PWR_CLK_ENABLE();

    // Enable BSEC clock.
    __HAL_RCC_BSEC_CLK_ENABLE();

    // Enable SYSCFG clock.
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    // Configure power-supply
    if (HAL_PWREx_ConfigSupply(OMV_PWR_SUPPLY) != HAL_OK) {
        __fatal_error("HAL_PWREx_ConfigSupply");
    }

    // Configure voltage scaling to the highest.
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE0) != HAL_OK) {
        __fatal_error("HAL_PWREx_ControlVoltageScaling");
    }

    // Leave PLLs unchanged.
    RCC_OscInit.PLL1.PLLState = RCC_PLL_NONE;
    RCC_OscInit.PLL2.PLLState = RCC_PLL_NONE;
    RCC_OscInit.PLL3.PLLState = RCC_PLL_NONE;
    RCC_OscInit.PLL4.PLLState = RCC_PLL_NONE;

    // Enable HIS and HSE oscillators.
    RCC_OscInit.HSEState = RCC_HSE_ON;
    RCC_OscInit.OscillatorType |= RCC_OSCILLATORTYPE_HSE;

    RCC_OscInit.HSIState = RCC_HSI_ON;
    RCC_OscInit.HSIDiv = OMV_OSC_HSI_DIV;
    RCC_OscInit.HSICalibrationValue = OMV_OSC_HSI_CAL;
    RCC_OscInit.OscillatorType |= RCC_OSCILLATORTYPE_HSI;

    if (HAL_RCC_OscConfig(&RCC_OscInit) != HAL_OK) {
        __fatal_error("HAL_RCC_OscConfig");
    }

    // Switch the CPU clock source to HSI and reconfigure the PLL.
    // The ROM bootloader enables the PLL by default (unless disabled in OTP).
    // So CPUCLK/SYSCLK source must be switched to HSI first before the PLL
    // can be reconfigured.
    RCC_ClkInit.ClockType = (RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK);
    RCC_ClkInit.CPUCLKSource = RCC_CPUCLKSOURCE_HSI;
    RCC_ClkInit.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    if (HAL_RCC_ClockConfig(&RCC_ClkInit) != HAL_OK) {
        while (1);
    }

    // Configure PLL1.
    RCC_OscInit.PLL1.PLLState = RCC_PLL_ON;
    RCC_OscInit.PLL1.PLLSource = OMV_OSC_PLL1SOURCE;
    RCC_OscInit.PLL1.PLLM = OMV_OSC_PLL1M;
    RCC_OscInit.PLL1.PLLN = OMV_OSC_PLL1N;
    RCC_OscInit.PLL1.PLLP1 = OMV_OSC_PLL1P1;
    RCC_OscInit.PLL1.PLLP2 = OMV_OSC_PLL1P2;
    RCC_OscInit.PLL1.PLLFractional = OMV_OSC_PLL1FRAC;

    // Configure PLL2.
    RCC_OscInit.PLL2.PLLState = RCC_PLL_ON;
    RCC_OscInit.PLL2.PLLSource = OMV_OSC_PLL2SOURCE;
    RCC_OscInit.PLL2.PLLM = OMV_OSC_PLL2M;
    RCC_OscInit.PLL2.PLLN = OMV_OSC_PLL2N;
    RCC_OscInit.PLL2.PLLP1 = OMV_OSC_PLL2P1;
    RCC_OscInit.PLL2.PLLP2 = OMV_OSC_PLL2P2;
    RCC_OscInit.PLL2.PLLFractional = OMV_OSC_PLL2FRAC;

    // Turn off the rest of the PLLs.
    RCC_OscInit.PLL3.PLLState = RCC_PLL_OFF;
    RCC_OscInit.PLL4.PLLState = RCC_PLL_OFF;

    if (HAL_RCC_OscConfig(&RCC_OscInit) != HAL_OK) {
        __fatal_error("HAL_RCC_OscConfig");
    }

    // Configure CPU and System bus clock source.
    RCC_ClkInit.ClockType  = (RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
                              RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK4 |  \
                              RCC_CLOCKTYPE_PCLK5);
    RCC_ClkInit.CPUCLKSource = RCC_CPUCLKSOURCE_IC1;
    RCC_ClkInit.SYSCLKSource = RCC_SYSCLKSOURCE_IC2_IC6_IC11;

    RCC_ClkInit.AHBCLKDivider  = RCC_HCLK_DIV2;
    RCC_ClkInit.APB1CLKDivider = RCC_APB1_DIV1;
    RCC_ClkInit.APB2CLKDivider = RCC_APB2_DIV1;
    RCC_ClkInit.APB4CLKDivider = RCC_APB4_DIV1;
    RCC_ClkInit.APB5CLKDivider = RCC_APB5_DIV1;

    RCC_ClkInit.IC1Selection.ClockSelection  = RCC_ICCLKSOURCE_PLL1;
    RCC_ClkInit.IC1Selection.ClockDivider    = 1;

    RCC_ClkInit.IC2Selection.ClockSelection  = RCC_ICCLKSOURCE_PLL1;
    RCC_ClkInit.IC2Selection.ClockDivider    = 2;

    RCC_ClkInit.IC6Selection.ClockSelection  = RCC_ICCLKSOURCE_PLL2;
    RCC_ClkInit.IC6Selection.ClockDivider    = 1;

    RCC_ClkInit.IC11Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
    RCC_ClkInit.IC11Selection.ClockDivider   = 1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInit) != HAL_OK) {
        __fatal_error("HAL_RCC_ClockConfig");
    }

    // Select XSPI2 clock source.
    PeriphClkInit.PeriphClockSelection   |= RCC_PERIPHCLK_XSPI2;
    PeriphClkInit.Xspi2ClockSelection     = RCC_XSPI2CLKSOURCE_HCLK;

    // Select USB OTG clock source.
    PeriphClkInit.PeriphClockSelection   |= RCC_PERIPHCLK_USBOTGHS1;
    PeriphClkInit.UsbPhy1ClockSelection   = RCC_USBOTGHS1CLKSOURCE_HSE_DIV2;
    PeriphClkInit.UsbOtgHs1ClockSelection = RCC_USBPHY1REFCLKSOURCE_OTGPHY1;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        __fatal_error("HAL_RCCEx_PeriphCLKConfig");
    }

    // Check if high speed IO optimization fuse is set.
    uint32_t fuse;
    BSEC_HandleTypeDef hbsec = { .Instance = BSEC };
    uint32_t mask = BSEC_HWS_HSLV_VDDIO2 | BSEC_HWS_HSLV_VDDIO3;
    if (HAL_BSEC_OTP_Read(&hbsec, BSEC_HW_CONFIG_ID, &fuse) != HAL_OK) {
        __fatal_error("HAL_BSEC_OTP_Read");
    } else if ((fuse & mask) != mask) {
        // Program the fuse.
        if (HAL_BSEC_OTP_Program(&hbsec, BSEC_HW_CONFIG_ID, fuse | mask, HAL_BSEC_NORMAL_PROG) != HAL_OK) {
            __fatal_error("HAL_BSEC_OTP_Program");
        }
        // Read back the fuse to verify the programming.
        if (HAL_BSEC_OTP_Read(&hbsec, BSEC_HW_CONFIG_ID, &fuse) != HAL_OK || ((fuse & mask) != mask)) {
            __fatal_error("HAL_BSEC_OTP_Read");
        }
    }

    HAL_PWREx_EnableVddA();

    // Configure VDDIO2
    HAL_PWREx_EnableVddIO2();
    HAL_PWREx_ConfigVddIORange(PWR_VDDIO2, PWR_VDDIO_RANGE_1V8);
    HAL_SYSCFG_EnableVDDIO2CompensationCell();

    // Configure VDDIO3
    HAL_PWREx_EnableVddIO3();
    HAL_PWREx_ConfigVddIORange(PWR_VDDIO3, PWR_VDDIO_RANGE_1V8);
    HAL_SYSCFG_EnableVDDIO3CompensationCell();

    // Configure VDDIO4
    HAL_PWREx_EnableVddIO4();
    HAL_PWREx_ConfigVddIORange(PWR_VDDIO4, PWR_VDDIO_RANGE_3V3);
    HAL_SYSCFG_EnableVDDIO4CompensationCell();

    // Configure VDDUSB supply
    HAL_PWREx_EnableVddUSB();
    HAL_PWREx_EnableVddUSBVMEN();
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_USB33RDY)) {

    }
}
#endif // OMV_BOOTLOADER
