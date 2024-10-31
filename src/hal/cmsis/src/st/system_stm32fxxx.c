/**
  ******************************************************************************
  * @file    system_stm32f4xx.c
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    18-February-2014
  * @brief   CMSIS Cortex-M4 Device Peripheral Access Layer System Source File.
  *
  *   This file provides two functions and one global variable to be called from
  *   user application:
  *      - SystemInit(): This function is called at startup just after reset and
  *                      before branch to main program. This call is made inside
  *                      the "startup_stm32f4xx.s" file.
  *
  *      - SystemCoreClock variable: Contains the core clock (HCLK), it can be used
  *                                  by the user application to setup the SysTick
  *                                  timer or configure other parameters.
  *
  *      - SystemCoreClockUpdate(): Updates the variable SystemCoreClock and must
  *                                 be called whenever the core clock is changed
  *                                 during program execution.
  *
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include STM32_HAL_H
#include "omv_boardconfig.h"

/** This variable is updated in two ways:
  * 1) by calling HAL API function HAL_RCC_GetHCLKFreq()
  * 2) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
  */
uint32_t SystemCoreClock = 16000000;
extern void __fatal_error(const char *msg);

#if defined(MCU_SERIES_H7)

#define SRAM_BASE           D1_AXISRAM_BASE
#define FLASH_BASE          FLASH_BANK1_BASE

#define CONFIG_RCC_CR_1ST   (RCC_CR_HSION)
#define CONFIG_RCC_CR_2ND   (0xEAF6ED7F)
#define CONFIG_RCC_PLLCFGR  (0x00000000)

uint32_t SystemD2Clock = 64000000;
const uint8_t D1CorePrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};

#elif defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)

#define CONFIG_RCC_CR_1ST   (RCC_CR_HSION)
#define CONFIG_RCC_CR_2ND   (0xFEF6FFFF)
#define CONFIG_RCC_PLLCFGR  (0x24003010)

const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};
const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

#endif

/**
  * @brief  Setup the microcontroller system
  *         Initialize the FPU setting, vector table location and External memory
  *         configuration.
  * @param  None
  * @retval None
  */
void SystemInit(void)
{
    /* FPU settings ------------------------------------------------------------*/
    #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
      SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
    #endif

    /*SEVONPEND enabled so that an interrupt coming from the CPU(n) interrupt
      signal is detectable by the CPU after a WFI/WFE instruction. */
    //SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

    #if !defined(CORE_CM4)
    /* Reset the RCC clock configuration to the default reset state ------------*/
    /* Set HSION bit */
    RCC->CR |= CONFIG_RCC_CR_1ST;

    /* Reset CFGR register */
    RCC->CFGR = 0x00000000;

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= (uint32_t) CONFIG_RCC_CR_2ND;

    /* Reset PLLCFGR register */
    RCC->PLLCFGR = CONFIG_RCC_PLLCFGR;

    #if defined(MCU_SERIES_H7)
    /* Reset D1CFGR register */
    RCC->D1CFGR = 0x00000000;

    /* Reset D2CFGR register */
    RCC->D2CFGR = 0x00000000;

    /* Reset D3CFGR register */
    RCC->D3CFGR = 0x00000000;

    /* Reset PLLCKSELR register */
    RCC->PLLCKSELR = 0x00000000;

    /* Reset PLL1DIVR register */
    RCC->PLL1DIVR = 0x00000000;

    /* Reset PLL1FRACR register */
    RCC->PLL1FRACR = 0x00000000;

    /* Reset PLL2DIVR register */
    RCC->PLL2DIVR = 0x00000000;

    /* Reset PLL2FRACR register */
    RCC->PLL2FRACR = 0x00000000;

    /* Reset PLL3DIVR register */
    RCC->PLL3DIVR = 0x00000000;

    /* Reset PLL3FRACR register */
    RCC->PLL3FRACR = 0x00000000;
    #endif // defined(MCU_SERIES_H7)

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Disable all interrupts */
    #if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)
    RCC->CIR = 0x00000000;
    #elif defined(MCU_SERIES_L4) || defined(MCU_SERIES_H7)
    RCC->CIER = 0x00000000;
    #endif

    #if defined(MCU_SERIES_H7)
    /* Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7) */
    // See Errata 2.2.9 "Reading from AXI SRAM may lead to data read corruption"
    *((__IO uint32_t*)0x51008108) = 0x00000001;
    #endif // defined(MCU_SERIES_H7)

    /* dpgeorge: enable 8-byte stack alignment for IRQ handlers, in accord with EABI */
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;

    #if !defined(NDEBUG)
    #if defined(MCU_SERIES_F4) ||  defined(MCU_SERIES_F7)
    DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP;
    #elif defined(MCU_SERIES_H7)
    DBGMCU->CR |= DBGMCU_CR_DBG_SLEEPD1;
    DBGMCU->CR |= DBGMCU_CR_DBG_STOPD1;
    DBGMCU->CR |= DBGMCU_CR_DBG_STANDBYD1;
    #endif
    #endif

    #endif // !defined(CORE_CM4)

    /* Configure the Vector Table location add offset address ------------------*/
    #ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
    #else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
    #endif
}

void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    #if defined(MCU_SERIES_H7)
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    #endif

    #if defined(MCU_SERIES_H7)
    /* Supply configuration update enable */
    HAL_PWREx_ConfigSupply(OMV_PWR_SUPPLY);
    #else
    /* Enable Power Control clock */
    __PWR_CLK_ENABLE();
    #endif

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet.  */
    #if defined(MCU_SERIES_H7)
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
    #if defined(MCU_SERIES_H7)
    while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY) == RESET) {
    }
    #endif

    #if defined(OMV_OSC_LSE_DRIVE)
    // Configure LSE drive strength.
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(OMV_OSC_LSE_DRIVE);
    HAL_PWR_DisableBkUpAccess();
    #endif

    /* Macro to configure the PLL clock source */
    __HAL_RCC_PLL_PLLSOURCE_CONFIG(OMV_OSC_PLL_CLKSOURCE);

    /* Enable HSE Oscillator and activate PLL with HSE as source */
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
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
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

    #if defined(MCU_SERIES_H7)
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

    #if defined(MCU_SERIES_H7)

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
    #endif // defined(MCU_SERIES_H7)

    #if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)
    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        // Initialization Error
        __fatal_error("HAL_PWREx_EnableOverDrive");
    }
    #endif

    /* Select PLL as system clock source and configure the HCLK, PCLK clocks dividers */
    #if defined(MCU_SERIES_H7)
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 |
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
    #else
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    #endif

    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, OMV_FLASH_LATENCY) != HAL_OK) {
        // Initialization Error
        __fatal_error("HAL_RCC_ClockConfig");
    }

    #if defined(MCU_SERIES_H7)
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
