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

#if defined(STM32H743xx)
#define SRAM_BASE D1_AXISRAM_BASE
#define FLASH_BASE FLASH_BANK1_BASE
#endif

// Uncomment the following line if you need to use external SRAM or SDRAM
//#define DATA_IN_ExtSRAM
//#define DATA_IN_ExtSDRAM

#if defined(DATA_IN_ExtSRAM) && defined(DATA_IN_ExtSDRAM)
#error "Please select DATA_IN_ExtSRAM or DATA_IN_ExtSDRAM "
#endif /* DATA_IN_ExtSRAM && DATA_IN_ExtSDRAM */

// Uncomment the following line if you need to relocate your vector Table in internal SRAM.
//#define VECT_TAB_SRAM

// Vector Table base offset field (defined in board config files).
//#define VECT_TAB_OFFSET  0x10000

/** This variable is updated in three ways:
  * 1) by calling CMSIS function SystemCoreClockUpdate()
  * 2) by calling HAL API function HAL_RCC_GetHCLKFreq()
  * 3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
  *    Note: If you use this function to configure the system clock; then there
  *          is no need to call the 2 first functions listed above, since SystemCoreClock
  *          variable is updated automatically.
  */
uint32_t SystemCoreClock = 16000000;

const uint8_t APBPrescTable[8] = {0, 0, 0, 0, 1, 2, 3, 4};
#if defined STM32H743xx
uint32_t SystemD2Clock = 64000000;
const uint8_t D1CorePrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};
#else
const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
#endif

#if defined (DATA_IN_ExtSRAM) || defined (DATA_IN_ExtSDRAM)
static void SystemInit_ExtMemCtl(void);
#endif /* DATA_IN_ExtSRAM || DATA_IN_ExtSDRAM */

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
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */

    /* Reset the RCC clock configuration to the default reset state ------------*/
    /* Set HSION bit */
    RCC->CR |= RCC_CR_HSION;

    /* Reset CFGR register */
    RCC->CFGR = 0x00000000;

#if defined(STM32H743xx)
    /* Reset HSEON, CSSON , CSION, RC48ON, CSIKERON PLL1ON, PLL2ON and PLL3ON bits */
    RCC->CR &= (uint32_t)0xEAF6ED7F;

    /* Reset D1CFGR register */
    RCC->D1CFGR = 0x00000000;

    /* Reset D2CFGR register */
    RCC->D2CFGR = 0x00000000;

    /* Reset D3CFGR register */
    RCC->D3CFGR = 0x00000000;

    /* Reset PLLCKSELR register */
    RCC->PLLCKSELR = 0x00000000;

    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x00000000;

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
#else
    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= (uint32_t)0xFEF6FFFF;

    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x24003010;
#endif

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

#if defined(STM32H743xx)
    /* Disable all interrupts */
    RCC->CIER = 0x00000000;

    /* Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7) */
    *((__IO uint32_t*)0x51008108) = 0x000000001;
#else
    /* Disable all interrupts */
    RCC->CIR = 0x00000000;
#endif

#if defined (DATA_IN_ExtSRAM) || defined (DATA_IN_ExtSDRAM)
    SystemInit_ExtMemCtl();
#endif /* DATA_IN_ExtSRAM || DATA_IN_ExtSDRAM */

    /* Configure the Vector Table location add offset address */
#ifdef VECT_TAB_SRAM
    /* Vector Table Relocation in Internal SRAM */
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET;
#else
    /* Vector Table Relocation in Internal FLASH */
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
#endif

#if !defined(STM32H743xx)
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;
#endif
}

void __fatal_error(const char *msg);
#define Error_Handler() __fatal_error("SystemClock_Config")

void SystemClock_Config(void)
{
    uint32_t flash_latency;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;
#if defined(MCU_SERIES_H7)
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
#endif

#if defined (STM32H743xx)// 400MHz/48MHz
    /* Supply configuration update enable */
    MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);
#else
    /* Enable Power Control clock */
    __PWR_CLK_ENABLE();
#endif

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

#if defined (STM32H743xx)// 400MHz/48MHz
    while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY) {
    }
#endif

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
#if defined(MCU_SERIES_H7)
    RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
#endif
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

#if   defined (STM32F407xx) // 168MHz/48MHz
    flash_latency = FLASH_LATENCY_5;
    RCC_OscInitStruct.PLL.PLLM = 12;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
#elif defined (STM32F427xx)// 192MHz/48MHz
    flash_latency = FLASH_LATENCY_7;
    RCC_OscInitStruct.PLL.PLLM = 12;
    RCC_OscInitStruct.PLL.PLLN = 384;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
#elif defined (STM32F765xx)// 216MHz/48MHz
    flash_latency = FLASH_LATENCY_7;
    RCC_OscInitStruct.PLL.PLLM = 12;
    RCC_OscInitStruct.PLL.PLLN = 432;
    RCC_OscInitStruct.PLL.PLLQ = 9;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLR = 2;
#elif defined (STM32H743xx)// 400MHz/48MHz
    flash_latency = FLASH_LATENCY_4;
    RCC_OscInitStruct.PLL.PLLM = 3;
    RCC_OscInitStruct.PLL.PLLN = 200;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
#endif
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }

#if defined(STM32H743xx)
    /* PLL3 for USB Clock */
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInitStruct.PLL3.PLL3M = 6;
    PeriphClkInitStruct.PLL3.PLL3N = 192;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 8;
    PeriphClkInitStruct.PLL3.PLL3R = 2;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_1;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }
#endif


#if defined(MCU_SERIES_H7)
    HAL_PWREx_EnableUSBVoltageDetector();
#endif

#if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)
    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
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

    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, flash_latency) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }

#if defined(MCU_SERIES_H7)
    // Activate CSI clock mondatory for I/O Compensation Cell
    __HAL_RCC_CSI_ENABLE() ;

    // Enable SYSCFG clock mondatory for I/O Compensation Cell
    __HAL_RCC_SYSCFG_CLK_ENABLE() ;

    // Enables the I/O Compensation Cell
    HAL_EnableCompensationCell();
#endif
}
#undef Error_Handler
