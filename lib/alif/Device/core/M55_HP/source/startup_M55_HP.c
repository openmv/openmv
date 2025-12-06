/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
/*
 * Copyright (c) 2020 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/******************************************************************************
 * @file     startup_M55_HP.c
 * @author   Rupesh Kumar
 * @email    rupesh@alifsemi.com
 * @brief    CMSIS Core Device Startup File for
 *           Alif Semiconductor M55_HP Device
 * @version  V1.0.1
 * @date     02. Dec 2022
 * @bug      None
 * @Note	 None
 ******************************************************************************/

#if defined (M55_HP)
  #include "M55_HP.h"
#else
  #error device not specified!
#endif

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;
extern uint32_t __STACK_LIMIT;

extern __NO_RETURN void __PROGRAM_START(void);

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler  (void);
            void Default_Handler(void);

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
void NMI_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler      (void) __attribute__ ((weak, alias("Fault_Handler")));
void MemManage_Handler      (void) __attribute__ ((weak, alias("Fault_Handler")));
void BusFault_Handler       (void) __attribute__ ((weak, alias("Fault_Handler")));
void UsageFault_Handler     (void) __attribute__ ((weak, alias("Fault_Handler")));
void SecureFault_Handler    (void) __attribute__ ((weak, alias("Fault_Handler")));
void SVC_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));

/* Interrupts */
void DMA1_IRQ0Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ1Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ2Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ3Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ4Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ5Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ6Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ7Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ8Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ9Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ10Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ11Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ12Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ13Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ14Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ15Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ16Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ17Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ18Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ19Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ20Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ21Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ22Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ23Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ24Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ25Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ26Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ27Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ28Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ29Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ30Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ31Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQ_ABORT_Handler (void) __attribute__ ((weak, alias("Default_Handler")));

void MHU_A32_M55HP_0_RX_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void MHU_M55HP_A32_0_TX_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void MHU_A32_M55HP_1_RX_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void MHU_M55HP_A32_1_TX_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));

void MHU_SECPU_M55HP_0_RX_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void MHU_M55HP_SECPU_0_TX_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void MHU_SECPU_M55HP_1_RX_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void MHU_M55HP_SECPU_1_TX_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));

void MHU_M55HE_M55HP_0_RX_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void MHU_M55HP_M55HE_0_TX_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void MHU_M55HE_M55HP_1_RX_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void MHU_M55HP_M55HE_1_TX_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));

void NPU_HP_IRQHandler                (void) __attribute__ ((weak, alias("Default_Handler")));
void LPCMP_IRQHandler                 (void) __attribute__ ((weak, alias("Default_Handler")));
void LPGPIO_COMB_IRQHandler           (void) __attribute__ ((weak, alias("Default_Handler")));

void REFCLK_CNTBASE0_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void REFCLK_CNTBASE1_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void REFCLK_CNTBASE2_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void REFCLK_CNTBASE3_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));

void S32K_CNTBASE0_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void S32K_CNTBASE1_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));

void SOC_ETR_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void SOC_CATU_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));

void OSPI0_IRQHandler            (void) __attribute__ ((weak, alias("Default_Handler")));
void OSPI1_IRQHandler            (void) __attribute__ ((weak, alias("Default_Handler")));

void AES0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void AES1_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));

void BOD_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));

void USB_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));

void SDMMC_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SDMMC_WAKEUP_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));

void CANFD_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));

void HWSEM_IRQ0Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ1Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ2Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ3Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ4Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ5Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ6Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ7Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ8Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ9Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ10Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ11Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ12Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ13Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ14Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void HWSEM_IRQ15Handler     (void) __attribute__ ((weak, alias("Default_Handler")));

void PPU0_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void PPU1_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void PPU2_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));

void UART0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART3_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART4_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART5_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART6_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART7_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));

void I2C0_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C2_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C3_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));

void I3C_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));

void SPI0_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI1_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI2_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI3_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));

void I2S0_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void I2S1_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void I2S2_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void I2S3_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));

void PDM_WARN_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void PDM_ERROR_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void PDM_AUDIO_DET_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));

void ETH_SBD_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void ETH_PMT_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));

void ADC120_DONE0_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC121_DONE0_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC122_DONE0_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));

void ADC120_DONE1_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC121_DONE1_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC122_DONE1_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));

void ADC120_CMPA_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC121_CMPA_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC122_CMPA_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));

void ADC120_CMPB_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC121_CMPB_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC122_CMPB_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));

void ADC24_DONE0_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC24_DONE1_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC24_CMPA_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC24_CMPB_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));

void CMP0_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void CMP1_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void CMP2_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void CMP3_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));

void LPGPIO_IRQ0Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void LPGPIO_IRQ1Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void LPGPIO_IRQ2Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void LPGPIO_IRQ3Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void LPGPIO_IRQ4Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void LPGPIO_IRQ5Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void LPGPIO_IRQ6Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void LPGPIO_IRQ7Handler (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO0_IRQ0Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_IRQ1Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_IRQ2Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_IRQ3Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_IRQ4Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_IRQ5Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_IRQ6Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_IRQ7Handler  (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO1_IRQ0Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO1_IRQ1Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO1_IRQ2Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO1_IRQ3Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO1_IRQ4Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO1_IRQ5Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO1_IRQ6Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO1_IRQ7Handler  (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO2_IRQ0Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO2_IRQ1Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO2_IRQ2Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO2_IRQ3Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO2_IRQ4Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO2_IRQ5Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO2_IRQ6Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO2_IRQ7Handler  (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO3_IRQ0Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO3_IRQ1Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO3_IRQ2Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO3_IRQ3Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO3_IRQ4Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO3_IRQ5Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO3_IRQ6Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO3_IRQ7Handler  (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO4_IRQ0Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO4_IRQ1Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO4_IRQ2Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO4_IRQ3Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO4_IRQ4Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO4_IRQ5Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO4_IRQ6Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO4_IRQ7Handler  (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO5_IRQ0Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO5_IRQ1Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO5_IRQ2Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO5_IRQ3Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO5_IRQ4Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO5_IRQ5Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO5_IRQ6Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO5_IRQ7Handler  (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO6_IRQ0Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO6_IRQ1Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO6_IRQ2Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO6_IRQ3Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO6_IRQ4Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO6_IRQ5Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO6_IRQ6Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO6_IRQ7Handler  (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO7_IRQ0Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO7_IRQ1Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO7_IRQ2Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO7_IRQ3Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO7_IRQ4Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO7_IRQ5Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO7_IRQ6Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO7_IRQ7Handler  (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO8_IRQ0Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO8_IRQ1Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO8_IRQ2Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO8_IRQ3Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO8_IRQ4Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO8_IRQ5Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO8_IRQ6Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO8_IRQ7Handler  (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO9_IRQ0Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO9_IRQ1Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO9_IRQ2Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO9_IRQ3Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO9_IRQ4Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO9_IRQ5Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO9_IRQ6Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO9_IRQ7Handler  (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO10_IRQ0Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO10_IRQ1Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO10_IRQ2Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO10_IRQ3Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO10_IRQ4Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO10_IRQ5Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO10_IRQ6Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO10_IRQ7Handler (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO11_IRQ0Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO11_IRQ1Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO11_IRQ2Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO11_IRQ3Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO11_IRQ4Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO11_IRQ5Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO11_IRQ6Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO11_IRQ7Handler (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO12_IRQ0Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO12_IRQ1Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO12_IRQ2Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO12_IRQ3Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO12_IRQ4Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO12_IRQ5Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO12_IRQ6Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO12_IRQ7Handler (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO13_IRQ0Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO13_IRQ1Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO13_IRQ2Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO13_IRQ3Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO13_IRQ4Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO13_IRQ5Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO13_IRQ6Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO13_IRQ7Handler (void) __attribute__ ((weak, alias("Default_Handler")));

void GPIO14_IRQ0Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO14_IRQ1Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO14_IRQ2Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO14_IRQ3Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO14_IRQ4Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO14_IRQ5Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO14_IRQ6Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO14_IRQ7Handler (void) __attribute__ ((weak, alias("Default_Handler")));

void DMA0_IRQ0Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ1Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ2Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ3Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ4Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ5Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ6Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ7Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ8Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ9Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ10Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ11Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ12Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ13Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ14Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ15Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ16Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ17Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ18Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ19Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ20Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ21Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ22Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ23Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ24Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ25Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ26Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ27Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ28Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ29Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ30Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ31Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_IRQ_ABORT_Handler (void) __attribute__ ((weak, alias("Default_Handler")));

void GPU2D_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));

void CDC_SCANLINE0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void CDC_SCANLINE1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void CDC_FIFO_WARNING0_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void CDC_FIFO_WARNING1_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void CDC_FIFO_UNDERRUN0_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void CDC_FIFO_UNDERRUN1_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void CDC_BUS_ERROR0_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void CDC_BUS_ERROR1_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void CDC_REG_RELOAD0_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void CDC_REG_RELOAD1_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));

void DSI_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void CSI_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));

void CAM_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));

void LPTIMER0_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void LPTIMER1_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void LPTIMER2_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void LPTIMER3_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));

void LPRTC_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));

void QEC0_CMPA_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void QEC0_CMPB_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void QEC1_CMPA_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void QEC1_CMPB_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void QEC2_CMPA_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void QEC2_CMPB_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void QEC3_CMPA_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void QEC3_CMPB_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));

void UTIMER_IRQ0Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ1Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ2Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ3Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ4Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ5Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ6Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ7Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ8Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ9Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ10Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ11Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ12Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ13Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ14Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ15Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ16Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ17Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ18Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ19Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ20Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ21Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ22Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ23Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ24Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ25Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ26Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ27Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ28Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ29Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ30Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ31Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ32Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ33Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ34Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ35Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ36Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ37Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ38Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ39Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ40Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ41Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ42Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ43Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ44Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ45Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ46Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ47Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ48Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ49Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ50Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ51Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ52Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ53Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ54Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ55Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ56Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ57Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ58Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ59Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ60Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ61Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ62Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ63Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ64Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ65Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ66Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ67Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ68Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ69Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ70Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ71Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ72Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ73Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ74Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ75Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ76Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ77Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ78Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ79Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ80Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ81Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ82Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ83Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ84Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ85Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ86Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ87Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ88Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ89Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ90Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ91Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ92Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ93Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ94Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void UTIMER_IRQ95Handler    (void) __attribute__ ((weak, alias("Default_Handler")));


/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

extern const VECTOR_TABLE_Type __VECTOR_TABLE[496];
       const VECTOR_TABLE_Type __VECTOR_TABLE[496] __VECTOR_TABLE_ATTRIBUTE = {
  (VECTOR_TABLE_Type)(&__INITIAL_SP),       /*     Initial Stack Pointer */
  Reset_Handler,                            /*     Reset Handler */
  NMI_Handler,                              /* -14 NMI Handler */
  HardFault_Handler,                        /* -13 Hard Fault Handler */
  MemManage_Handler,                        /* -12 MPU Fault Handler */
  BusFault_Handler,                         /* -11 Bus Fault Handler */
  UsageFault_Handler,                       /* -10 Usage Fault Handler */
  SecureFault_Handler,                      /*  -9 Secure Fault Handler */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  SVC_Handler,                              /*  -5 SVCall Handler */
  DebugMon_Handler,                         /*  -4 Debug Monitor Handler */
  0,                                        /*     Reserved */
  PendSV_Handler,                           /*  -2 PendSV Handler */
  SysTick_Handler,                          /*  -1 SysTick Handler */

  /* Interrupts */
  DMA1_IRQ0Handler,                         /*   0 Interrupt 0 */
  DMA1_IRQ1Handler,                         /*   1 Interrupt 1 */
  DMA1_IRQ2Handler,                         /*   2 Interrupt 2 */
  DMA1_IRQ3Handler,                         /*   3 Interrupt 3 */
  DMA1_IRQ4Handler,                         /*   4 Interrupt 4 */
  DMA1_IRQ5Handler,                         /*   5 Interrupt 5 */
  DMA1_IRQ6Handler,                         /*   6 Interrupt 6 */
  DMA1_IRQ7Handler,                         /*   7 Interrupt 7 */
  DMA1_IRQ8Handler,                         /*   8 Interrupt 8 */
  DMA1_IRQ9Handler,                         /*   9 Interrupt 9 */
  DMA1_IRQ10Handler,                        /*   10 Interrupt 10 */
  DMA1_IRQ11Handler,                        /*   11 Interrupt 11 */
  DMA1_IRQ12Handler,                        /*   12 Interrupt 12 */
  DMA1_IRQ13Handler,                        /*   13 Interrupt 13 */
  DMA1_IRQ14Handler,                        /*   14 Interrupt 14 */
  DMA1_IRQ15Handler,                        /*   15 Interrupt 15 */
  DMA1_IRQ16Handler,                        /*   16 Interrupt 16 */
  DMA1_IRQ17Handler,                        /*   17 Interrupt 17 */
  DMA1_IRQ18Handler,                        /*   18 Interrupt 18 */
  DMA1_IRQ19Handler,                        /*   19 Interrupt 19 */
  DMA1_IRQ20Handler,                        /*   20 Interrupt 20 */
  DMA1_IRQ21Handler,                        /*   21 Interrupt 21 */
  DMA1_IRQ22Handler,                        /*   22 Interrupt 22 */
  DMA1_IRQ23Handler,                        /*   23 Interrupt 23 */
  DMA1_IRQ24Handler,                        /*   24 Interrupt 24 */
  DMA1_IRQ25Handler,                        /*   25 Interrupt 25 */
  DMA1_IRQ26Handler,                        /*   26 Interrupt 26 */
  DMA1_IRQ27Handler,                        /*   27 Interrupt 27 */
  DMA1_IRQ28Handler,                        /*   28 Interrupt 28 */
  DMA1_IRQ29Handler,                        /*   29 Interrupt 29 */
  DMA1_IRQ30Handler,                        /*   30 Interrupt 30 */
  DMA1_IRQ31Handler,                        /*   31 Interrupt 31 */
  DMA1_IRQ_ABORT_Handler,                   /*   32 Interrupt 32 */
  MHU_A32_M55HP_0_RX_IRQHandler,            /*   33 Interrupt 33 */
  MHU_M55HP_A32_0_TX_IRQHandler,            /*   34 Interrupt 34 */
  MHU_A32_M55HP_1_RX_IRQHandler,            /*   35 Interrupt 35 */
  MHU_M55HP_A32_1_TX_IRQHandler,            /*   36 Interrupt 36 */
  MHU_SECPU_M55HP_0_RX_IRQHandler,          /*   37 Interrupt 37 */
  MHU_M55HP_SECPU_0_TX_IRQHandler,          /*   38 Interrupt 38 */
  MHU_SECPU_M55HP_1_RX_IRQHandler,          /*   39 Interrupt 39 */
  MHU_M55HP_SECPU_1_TX_IRQHandler,          /*   40 Interrupt 40 */
  MHU_M55HE_M55HP_0_RX_IRQHandler,          /*   41 Interrupt 41 */
  MHU_M55HP_M55HE_0_TX_IRQHandler,          /*   42 Interrupt 42 */
  MHU_M55HE_M55HP_1_RX_IRQHandler,          /*   43 Interrupt 43 */
  MHU_M55HP_M55HE_1_TX_IRQHandler,          /*   44 Interrupt 44 */
  0,                                        /*   45 Reserved Interrupt 45 */
  0,                                        /*   46 Reserved Interrupt 46 */
  0,                                        /*   47 Reserved Interrupt 47 */
  0,                                        /*   48 Reserved Interrupt 48 */
  0,                                        /*   49 Reserved Interrupt 49 */
  0,                                        /*   50 Reserved Interrupt 50 */
  0,                                        /*   51 Reserved Interrupt 51 */
  0,                                        /*   52 Reserved Interrupt 52 */
  0,                                        /*   53 Reserved Interrupt 53 */
  0,                                        /*   54 Reserved Interrupt 54 */
  NPU_HP_IRQHandler,                        /*   55 Interrupt 55 */
  LPCMP_IRQHandler,                         /*   56 Interrupt 56 */
  LPGPIO_COMB_IRQHandler,                   /*   57 Interrupt 57 */
  LPRTC_IRQHandler,                         /*   58 Interrupt 58 */
  0,                                        /*   59 Reserved Interrupt 59 */
  LPTIMER0_IRQHandler,                      /*   60 Interrupt 60 */
  LPTIMER1_IRQHandler,                      /*   61 Interrupt 61 */
  LPTIMER2_IRQHandler,                      /*   62 Interrupt 62 */
  LPTIMER3_IRQHandler,                      /*   63 Interrupt 63 */
  0,                                        /*   64 Reserved Interrupt 64 */
  0,                                        /*   65 Reserved Interrupt 65 */
  0,                                        /*   66 Reserved Interrupt 66 */
  REFCLK_CNTBASE0_IRQHandler,               /*   67 Interrupt 67 */
  REFCLK_CNTBASE1_IRQHandler,               /*   68 Interrupt 68 */
  REFCLK_CNTBASE2_IRQHandler,               /*   69 Interrupt 69 */
  REFCLK_CNTBASE3_IRQHandler,               /*   70 Interrupt 70 */
  S32K_CNTBASE0_IRQHandler,                 /*   71 Interrupt 71 */
  S32K_CNTBASE1_IRQHandler,                 /*   72 Interrupt 72 */
  SOC_ETR_IRQHandler,                       /*   73 Interrupt 73 */
  SOC_CATU_IRQHandler,                      /*   74 Interrupt 74 */
  0,                                        /*   75 Reserved Interrupt 75 */
  0,                                        /*   76 Reserved Interrupt 76 */
  0,                                        /*   77 Reserved Interrupt 77 */
  0,                                        /*   78 Reserved Interrupt 78 */
  0,                                        /*   79 Reserved Interrupt 79 */
  0,                                        /*   80 Reserved Interrupt 80 */
  0,                                        /*   81 Reserved Interrupt 81 */
  0,                                        /*   82 Reserved Interrupt 82 */
  0,                                        /*   83 Reserved Interrupt 83 */
  0,                                        /*   84 Reserved Interrupt 84 */
  0,                                        /*   85 Reserved Interrupt 85 */
  0,                                        /*   86 Reserved Interrupt 86 */
  0,                                        /*   87 Reserved Interrupt 87 */
  0,                                        /*   88 Reserved Interrupt 88 */
  0,                                        /*   89 Reserved Interrupt 89 */
  0,                                        /*   90 Reserved Interrupt 90 */
  0,                                        /*   91 Reserved Interrupt 91 */
  0,                                        /*   92 Reserved Interrupt 92 */
  0,                                        /*   93 Reserved Interrupt 93 */
  0,                                        /*   94 Reserved Interrupt 94 */
  0,                                        /*   95 Reserved Interrupt 95 */
  OSPI0_IRQHandler,                         /*   96 Interrupt 96 */
  OSPI1_IRQHandler,                         /*   97 Interrupt 97 */
  AES0_IRQHandler,                          /*   98 Interrupt 98 */
  AES1_IRQHandler,                          /*   99 Interrupt 99 */
  BOD_IRQHandler,                           /*   100 Interrupt 100 */
  USB_IRQHandler,                           /*   101 Interrupt 101 */
  SDMMC_IRQHandler,                         /*   102 Interrupt 102 */
  SDMMC_WAKEUP_IRQHandler,                  /*   103 Interrupt 103 */
  CANFD_IRQHandler,                         /*   104 Interrupt 104 */
  HWSEM_IRQ0Handler,                        /*   105 Interrupt 105 */
  HWSEM_IRQ1Handler,                        /*   106 Interrupt 106 */
  HWSEM_IRQ2Handler,                        /*   107 Interrupt 107 */
  HWSEM_IRQ3Handler,                        /*   108 Interrupt 108 */
  HWSEM_IRQ4Handler,                        /*   109 Interrupt 109 */
  HWSEM_IRQ5Handler,                        /*   110 Interrupt 110 */
  HWSEM_IRQ6Handler,                        /*   111 Interrupt 111 */
  HWSEM_IRQ7Handler,                        /*   112 Interrupt 112 */
  HWSEM_IRQ8Handler,                        /*   113 Interrupt 113 */
  HWSEM_IRQ9Handler,                        /*   114 Interrupt 114 */
  HWSEM_IRQ10Handler,                       /*   115 Interrupt 115 */
  HWSEM_IRQ11Handler,                       /*   116 Interrupt 116 */
  HWSEM_IRQ12Handler,                       /*   117 Interrupt 117 */
  HWSEM_IRQ13Handler,                       /*   118 Interrupt 118 */
  HWSEM_IRQ14Handler,                       /*   119 Interrupt 119 */
  HWSEM_IRQ15Handler,                       /*   120 Interrupt 120 */
  PPU0_IRQHandler,                          /*   121 Interrupt 121 */
  PPU1_IRQHandler,                          /*   122 Interrupt 122 */
  PPU2_IRQHandler,                          /*   123 Interrupt 123 */
  UART0_IRQHandler,                         /*   124 Interrupt 124 */
  UART1_IRQHandler,                         /*   125 Interrupt 125 */
  UART2_IRQHandler,                         /*   126 Interrupt 126 */
  UART3_IRQHandler,                         /*   127 Interrupt 127 */
  UART4_IRQHandler,                         /*   128 Interrupt 128 */
  UART5_IRQHandler,                         /*   129 Interrupt 129 */
  UART6_IRQHandler,                         /*   130 Interrupt 130 */
  UART7_IRQHandler,                         /*   131 Interrupt 131 */
  I2C0_IRQHandler,                          /*   132 Interrupt 132 */
  I2C1_IRQHandler,                          /*   133 Interrupt 133 */
  I2C2_IRQHandler,                          /*   134 Interrupt 134 */
  I2C3_IRQHandler,                          /*   135 Interrupt 135 */
  I3C_IRQHandler,                           /*   136 Interrupt 136 */
  SPI0_IRQHandler,                          /*   137 Interrupt 137 */
  SPI1_IRQHandler,                          /*   138 Interrupt 138 */
  SPI2_IRQHandler,                          /*   139 Interrupt 139 */
  SPI3_IRQHandler,                          /*   140 Interrupt 140 */
  I2S0_IRQHandler,                          /*   141 Interrupt 141 */
  I2S1_IRQHandler,                          /*   142 Interrupt 142 */
  I2S2_IRQHandler,                          /*   143 Interrupt 143 */
  I2S3_IRQHandler,                          /*   144 Interrupt 144 */
  PDM_WARN_IRQHandler,                      /*   145 Interrupt 145 */
  PDM_ERROR_IRQHandler,                     /*   146 Interrupt 146 */
  PDM_AUDIO_DET_IRQHandler,                 /*   147 Interrupt 147 */
  ETH_SBD_IRQHandler,                       /*   148 Interrupt 148 */
  ETH_PMT_IRQHandler,                       /*   149 Interrupt 149 */
  0,                                        /*   150 Reserved Interrupt 150 */
  ADC120_DONE0_IRQHandler,                  /*   151 Interrupt 151 */
  ADC121_DONE0_IRQHandler,                  /*   152 Interrupt 152 */
  ADC122_DONE0_IRQHandler,                  /*   153 Interrupt 153 */
  ADC120_DONE1_IRQHandler,                  /*   154 Interrupt 154 */
  ADC121_DONE1_IRQHandler,                  /*   155 Interrupt 155 */
  ADC122_DONE1_IRQHandler,                  /*   156 Interrupt 156 */
  ADC120_CMPA_IRQHandler,                   /*   157 Interrupt 157 */
  ADC121_CMPA_IRQHandler,                   /*   158 Interrupt 158 */
  ADC122_CMPA_IRQHandler,                   /*   159 Interrupt 159 */
  ADC120_CMPB_IRQHandler,                   /*   160 Interrupt 160 */
  ADC121_CMPB_IRQHandler,                   /*   161 Interrupt 161 */
  ADC122_CMPB_IRQHandler,                   /*   162 Interrupt 162 */
  ADC24_DONE0_IRQHandler,                   /*   163 Interrupt 163 */
  ADC24_DONE1_IRQHandler,                   /*   164 Interrupt 164 */
  ADC24_CMPA_IRQHandler,                    /*   165 Interrupt 165 */
  ADC24_CMPB_IRQHandler,                    /*   166 Interrupt 166 */
  CMP0_IRQHandler,                          /*   167 Interrupt 167 */
  CMP1_IRQHandler,                          /*   168 Interrupt 168 */
  CMP2_IRQHandler,                          /*   169 Interrupt 169 */
  CMP3_IRQHandler,                          /*   170 Interrupt 170 */
  LPGPIO_IRQ0Handler,                       /*   171 Interrupt 171 */
  LPGPIO_IRQ1Handler,                       /*   172 Interrupt 172 */
  LPGPIO_IRQ2Handler,                       /*   173 Interrupt 173 */
  LPGPIO_IRQ3Handler,                       /*   174 Interrupt 174 */
  LPGPIO_IRQ4Handler,                       /*   175 Interrupt 175 */
  LPGPIO_IRQ5Handler,                       /*   176 Interrupt 176 */
  LPGPIO_IRQ6Handler,                       /*   177 Interrupt 177 */
  LPGPIO_IRQ7Handler,                       /*   178 Interrupt 178 */
  GPIO0_IRQ0Handler,                        /*   179 Interrupt 179 */
  GPIO0_IRQ1Handler,                        /*   180 Interrupt 180 */
  GPIO0_IRQ2Handler,                        /*   181 Interrupt 181 */
  GPIO0_IRQ3Handler,                        /*   182 Interrupt 182 */
  GPIO0_IRQ4Handler,                        /*   183 Interrupt 183 */
  GPIO0_IRQ5Handler,                        /*   184 Interrupt 184 */
  GPIO0_IRQ6Handler,                        /*   185 Interrupt 185 */
  GPIO0_IRQ7Handler,                        /*   186 Interrupt 186 */
  GPIO1_IRQ0Handler,                        /*   187 Interrupt 187 */
  GPIO1_IRQ1Handler,                        /*   188 Interrupt 188 */
  GPIO1_IRQ2Handler,                        /*   189 Interrupt 189 */
  GPIO1_IRQ3Handler,                        /*   190 Interrupt 190 */
  GPIO1_IRQ4Handler,                        /*   191 Interrupt 191 */
  GPIO1_IRQ5Handler,                        /*   192 Interrupt 192 */
  GPIO1_IRQ6Handler,                        /*   193 Interrupt 193 */
  GPIO1_IRQ7Handler,                        /*   194 Interrupt 194 */
  GPIO2_IRQ0Handler,                        /*   195 Interrupt 195 */
  GPIO2_IRQ1Handler,                        /*   196 Interrupt 196 */
  GPIO2_IRQ2Handler,                        /*   197 Interrupt 197 */
  GPIO2_IRQ3Handler,                        /*   198 Interrupt 198 */
  GPIO2_IRQ4Handler,                        /*   199 Interrupt 199 */
  GPIO2_IRQ5Handler,                        /*   200 Interrupt 200 */
  GPIO2_IRQ6Handler,                        /*   201 Interrupt 201 */
  GPIO2_IRQ7Handler,                        /*   202 Interrupt 202 */
  GPIO3_IRQ0Handler,                        /*   203 Interrupt 203 */
  GPIO3_IRQ1Handler,                        /*   204 Interrupt 204 */
  GPIO3_IRQ2Handler,                        /*   205 Interrupt 205 */
  GPIO3_IRQ3Handler,                        /*   206 Interrupt 206 */
  GPIO3_IRQ4Handler,                        /*   207 Interrupt 207 */
  GPIO3_IRQ5Handler,                        /*   208 Interrupt 208 */
  GPIO3_IRQ6Handler,                        /*   209 Interrupt 209 */
  GPIO3_IRQ7Handler,                        /*   210 Interrupt 210 */
  GPIO4_IRQ0Handler,                        /*   211 Interrupt 211 */
  GPIO4_IRQ1Handler,                        /*   212 Interrupt 212 */
  GPIO4_IRQ2Handler,                        /*   213 Interrupt 213 */
  GPIO4_IRQ3Handler,                        /*   214 Interrupt 214 */
  GPIO4_IRQ4Handler,                        /*   215 Interrupt 215 */
  GPIO4_IRQ5Handler,                        /*   216 Interrupt 216 */
  GPIO4_IRQ6Handler,                        /*   217 Interrupt 217 */
  GPIO4_IRQ7Handler,                        /*   218 Interrupt 218 */
  GPIO5_IRQ0Handler,                        /*   219 Interrupt 219 */
  GPIO5_IRQ1Handler,                        /*   220 Interrupt 220 */
  GPIO5_IRQ2Handler,                        /*   221 Interrupt 221 */
  GPIO5_IRQ3Handler,                        /*   222 Interrupt 222 */
  GPIO5_IRQ4Handler,                        /*   223 Interrupt 223 */
  GPIO5_IRQ5Handler,                        /*   224 Interrupt 224 */
  GPIO5_IRQ6Handler,                        /*   225 Interrupt 225 */
  GPIO5_IRQ7Handler,                        /*   226 Interrupt 226 */
  GPIO6_IRQ0Handler,                        /*   227 Interrupt 227 */
  GPIO6_IRQ1Handler,                        /*   228 Interrupt 228 */
  GPIO6_IRQ2Handler,                        /*   229 Interrupt 229 */
  GPIO6_IRQ3Handler,                        /*   230 Interrupt 230 */
  GPIO6_IRQ4Handler,                        /*   231 Interrupt 231 */
  GPIO6_IRQ5Handler,                        /*   232 Interrupt 232 */
  GPIO6_IRQ6Handler,                        /*   233 Interrupt 233 */
  GPIO6_IRQ7Handler,                        /*   234 Interrupt 234 */
  GPIO7_IRQ0Handler,                        /*   235 Interrupt 235 */
  GPIO7_IRQ1Handler,                        /*   236 Interrupt 236 */
  GPIO7_IRQ2Handler,                        /*   237 Interrupt 237 */
  GPIO7_IRQ3Handler,                        /*   238 Interrupt 238 */
  GPIO7_IRQ4Handler,                        /*   239 Interrupt 239 */
  GPIO7_IRQ5Handler,                        /*   240 Interrupt 240 */
  GPIO7_IRQ6Handler,                        /*   241 Interrupt 241 */
  GPIO7_IRQ7Handler,                        /*   242 Interrupt 242 */
  GPIO8_IRQ0Handler,                        /*   243 Interrupt 243 */
  GPIO8_IRQ1Handler,                        /*   244 Interrupt 244 */
  GPIO8_IRQ2Handler,                        /*   245 Interrupt 245 */
  GPIO8_IRQ3Handler,                        /*   246 Interrupt 246 */
  GPIO8_IRQ4Handler,                        /*   247 Interrupt 247 */
  GPIO8_IRQ5Handler,                        /*   248 Interrupt 248 */
  GPIO8_IRQ6Handler,                        /*   249 Interrupt 249 */
  GPIO8_IRQ7Handler,                        /*   250 Interrupt 250 */
  GPIO9_IRQ0Handler,                        /*   251 Interrupt 251 */
  GPIO9_IRQ1Handler,                        /*   252 Interrupt 252 */
  GPIO9_IRQ2Handler,                        /*   253 Interrupt 253 */
  GPIO9_IRQ3Handler,                        /*   254 Interrupt 254 */
  GPIO9_IRQ4Handler,                        /*   255 Interrupt 255 */
  GPIO9_IRQ5Handler,                        /*   256 Interrupt 256 */
  GPIO9_IRQ6Handler,                        /*   257 Interrupt 257 */
  GPIO9_IRQ7Handler,                        /*   258 Interrupt 258 */
  GPIO10_IRQ0Handler,                       /*   259 Interrupt 259 */
  GPIO10_IRQ1Handler,                       /*   260 Interrupt 260 */
  GPIO10_IRQ2Handler,                       /*   261 Interrupt 261 */
  GPIO10_IRQ3Handler,                       /*   262 Interrupt 262 */
  GPIO10_IRQ4Handler,                       /*   263 Interrupt 263 */
  GPIO10_IRQ5Handler,                       /*   264 Interrupt 264 */
  GPIO10_IRQ6Handler,                       /*   265 Interrupt 265 */
  GPIO10_IRQ7Handler,                       /*   266 Interrupt 266 */
  GPIO11_IRQ0Handler,                       /*   267 Interrupt 267 */
  GPIO11_IRQ1Handler,                       /*   268 Interrupt 268 */
  GPIO11_IRQ2Handler,                       /*   269 Interrupt 269 */
  GPIO11_IRQ3Handler,                       /*   270 Interrupt 270 */
  GPIO11_IRQ4Handler,                       /*   271 Interrupt 271 */
  GPIO11_IRQ5Handler,                       /*   272 Interrupt 272 */
  GPIO11_IRQ6Handler,                       /*   273 Interrupt 273 */
  GPIO11_IRQ7Handler,                       /*   274 Interrupt 274 */
  GPIO12_IRQ0Handler,                       /*   275 Interrupt 275 */
  GPIO12_IRQ1Handler,                       /*   276 Interrupt 276 */
  GPIO12_IRQ2Handler,                       /*   277 Interrupt 277 */
  GPIO12_IRQ3Handler,                       /*   278 Interrupt 278 */
  GPIO12_IRQ4Handler,                       /*   279 Interrupt 279 */
  GPIO12_IRQ5Handler,                       /*   280 Interrupt 280 */
  GPIO12_IRQ6Handler,                       /*   281 Interrupt 281 */
  GPIO12_IRQ7Handler,                       /*   282 Interrupt 282 */
  GPIO13_IRQ0Handler,                       /*   283 Interrupt 283 */
  GPIO13_IRQ1Handler,                       /*   284 Interrupt 284 */
  GPIO13_IRQ2Handler,                       /*   285 Interrupt 285 */
  GPIO13_IRQ3Handler,                       /*   286 Interrupt 286 */
  GPIO13_IRQ4Handler,                       /*   287 Interrupt 287 */
  GPIO13_IRQ5Handler,                       /*   288 Interrupt 288 */
  GPIO13_IRQ6Handler,                       /*   289 Interrupt 289 */
  GPIO13_IRQ7Handler,                       /*   290 Interrupt 290 */
  GPIO14_IRQ0Handler,                       /*   291 Interrupt 291 */
  GPIO14_IRQ1Handler,                       /*   292 Interrupt 292 */
  GPIO14_IRQ2Handler,                       /*   293 Interrupt 293 */
  GPIO14_IRQ3Handler,                       /*   294 Interrupt 294 */
  GPIO14_IRQ4Handler,                       /*   295 Interrupt 295 */
  GPIO14_IRQ5Handler,                       /*   296 Interrupt 296 */
  GPIO14_IRQ6Handler,                       /*   297 Interrupt 297 */
  GPIO14_IRQ7Handler,                       /*   298 Interrupt 298 */
  DMA0_IRQ0Handler,                         /*   299 Interrupt 299 */
  DMA0_IRQ1Handler,                         /*   300 Interrupt 300 */
  DMA0_IRQ2Handler,                         /*   301 Interrupt 301 */
  DMA0_IRQ3Handler,                         /*   302 Interrupt 302 */
  DMA0_IRQ4Handler,                         /*   303 Interrupt 303 */
  DMA0_IRQ5Handler,                         /*   304 Interrupt 304 */
  DMA0_IRQ6Handler,                         /*   305 Interrupt 305 */
  DMA0_IRQ7Handler,                         /*   306 Interrupt 306 */
  DMA0_IRQ8Handler,                         /*   307 Interrupt 307 */
  DMA0_IRQ9Handler,                         /*   308 Interrupt 308 */
  DMA0_IRQ10Handler,                        /*   309 Interrupt 309 */
  DMA0_IRQ11Handler,                        /*   310 Interrupt 310 */
  DMA0_IRQ12Handler,                        /*   311 Interrupt 311 */
  DMA0_IRQ13Handler,                        /*   312 Interrupt 312 */
  DMA0_IRQ14Handler,                        /*   313 Interrupt 313 */
  DMA0_IRQ15Handler,                        /*   314 Interrupt 314 */
  DMA0_IRQ16Handler,                        /*   315 Interrupt 315 */
  DMA0_IRQ17Handler,                        /*   316 Interrupt 316 */
  DMA0_IRQ18Handler,                        /*   317 Interrupt 317 */
  DMA0_IRQ19Handler,                        /*   318 Interrupt 318 */
  DMA0_IRQ20Handler,                        /*   319 Interrupt 319 */
  DMA0_IRQ21Handler,                        /*   320 Interrupt 320 */
  DMA0_IRQ22Handler,                        /*   321 Interrupt 321 */
  DMA0_IRQ23Handler,                        /*   322 Interrupt 322 */
  DMA0_IRQ24Handler,                        /*   323 Interrupt 323 */
  DMA0_IRQ25Handler,                        /*   324 Interrupt 324 */
  DMA0_IRQ26Handler,                        /*   325 Interrupt 325 */
  DMA0_IRQ27Handler,                        /*   326 Interrupt 326 */
  DMA0_IRQ28Handler,                        /*   327 Interrupt 327 */
  DMA0_IRQ29Handler,                        /*   328 Interrupt 328 */
  DMA0_IRQ30Handler,                        /*   329 Interrupt 329 */
  DMA0_IRQ31Handler,                        /*   330 Interrupt 330 */
  DMA0_IRQ_ABORT_Handler,                   /*   331 Interrupt 331 */
  GPU2D_IRQHandler,                         /*   332 Interrupt 332 */
  CDC_SCANLINE0_IRQHandler,                 /*   333 Interrupt 333 */
  CDC_SCANLINE1_IRQHandler,                 /*   334 Interrupt 334 */
  CDC_FIFO_WARNING0_IRQHandler,             /*   335 Interrupt 335 */
  CDC_FIFO_WARNING1_IRQHandler,             /*   336 Interrupt 336 */
  CDC_FIFO_UNDERRUN0_IRQHandler,            /*   337 Interrupt 337 */
  CDC_FIFO_UNDERRUN1_IRQHandler,            /*   338 Interrupt 338 */
  CDC_BUS_ERROR0_IRQHandler,                /*   339 Interrupt 339 */
  CDC_BUS_ERROR1_IRQHandler,                /*   340 Interrupt 340 */
  CDC_REG_RELOAD0_IRQHandler,               /*   341 Interrupt 341 */
  CDC_REG_RELOAD1_IRQHandler,               /*   342 Interrupt 342 */
  DSI_IRQHandler,                           /*   343 Interrupt 343 */
  CSI_IRQHandler,                           /*   344 Interrupt 344 */
  CAM_IRQHandler,                           /*   345 Interrupt 345 */
  LPTIMER0_IRQHandler,                      /*   346 Interrupt 346 */
  LPTIMER1_IRQHandler,                      /*   347 Interrupt 347 */
  LPTIMER2_IRQHandler,                      /*   348 Interrupt 348 */
  LPTIMER3_IRQHandler,                      /*   349 Interrupt 349 */
  LPRTC_IRQHandler,                         /*   350 Interrupt 350 */
  0,			                            /*   351 Reserved Interrupt 351 */
  0,					            /*   352 Reserved Interrupt 352 */
  0,					            /*   353 Reserved Interrupt 353 */
  0,					            /*   354 Reserved Interrupt 354 */
  0,					            /*   355 Reserved Interrupt 355 */
  0,					            /*   356 Reserved Interrupt 356 */
  0,					            /*   357 Reserved Interrupt 357 */
  0,					            /*   358 Reserved Interrupt 358 */
  0,					            /*   359 Reserved Interrupt 359 */
  0,					            /*   360 Reserved Interrupt 360 */
  0,					            /*   361 Reserved Interrupt 361 */
  0,					            /*   362 Reserved Interrupt 362 */
  0,                                        /*   363 Reserved Interrupt 363 */
  0,                                        /*   364 Reserved Interrupt 364 */
  0,                                        /*   365 Reserved Interrupt 365 */
  0,                                        /*   366 Reserved Interrupt 366 */
  0,                                        /*   367 Reserved Interrupt 367 */
  0,                                        /*   368 Reserved Interrupt 368 */
  QEC0_CMPA_IRQHandler,                     /*   369 Interrupt 369 */
  QEC0_CMPB_IRQHandler,                     /*   370 Interrupt 370 */
  QEC1_CMPA_IRQHandler,                     /*   371 Interrupt 371 */
  QEC1_CMPB_IRQHandler,                     /*   372 Interrupt 372 */
  QEC2_CMPA_IRQHandler,                     /*   373 Interrupt 373 */
  QEC2_CMPB_IRQHandler,                     /*   374 Interrupt 374 */
  QEC3_CMPA_IRQHandler,                     /*   375 Interrupt 375 */
  QEC3_CMPB_IRQHandler,                     /*   376 Interrupt 376 */
  UTIMER_IRQ0Handler,                       /*   377 Interrupt 377 */
  UTIMER_IRQ1Handler,                       /*   378 Interrupt 378 */
  UTIMER_IRQ2Handler,                       /*   379 Interrupt 379 */
  UTIMER_IRQ3Handler,                       /*   380 Interrupt 380 */
  UTIMER_IRQ4Handler,                       /*   381 Interrupt 381 */
  UTIMER_IRQ5Handler,                       /*   382 Interrupt 382 */
  UTIMER_IRQ6Handler,                       /*   383 Interrupt 383 */
  UTIMER_IRQ7Handler,                       /*   384 Interrupt 384 */
  UTIMER_IRQ8Handler,                       /*   385 Interrupt 385 */
  UTIMER_IRQ9Handler,                       /*   386 Interrupt 386 */
  UTIMER_IRQ10Handler,                      /*   387 Interrupt 387 */
  UTIMER_IRQ11Handler,                      /*   388 Interrupt 388 */
  UTIMER_IRQ12Handler,                      /*   389 Interrupt 389 */
  UTIMER_IRQ13Handler,                      /*   390 Interrupt 390 */
  UTIMER_IRQ14Handler,                      /*   391 Interrupt 391 */
  UTIMER_IRQ15Handler,                      /*   392 Interrupt 392 */
  UTIMER_IRQ16Handler,                      /*   393 Interrupt 393 */
  UTIMER_IRQ17Handler,                      /*   394 Interrupt 394 */
  UTIMER_IRQ18Handler,                      /*   395 Interrupt 395 */
  UTIMER_IRQ19Handler,                      /*   396 Interrupt 396 */
  UTIMER_IRQ20Handler,                      /*   397 Interrupt 397 */
  UTIMER_IRQ21Handler,                      /*   398 Interrupt 398 */
  UTIMER_IRQ22Handler,                      /*   399 Interrupt 399 */
  UTIMER_IRQ23Handler,                      /*   400 Interrupt 400 */
  UTIMER_IRQ24Handler,                      /*   401 Interrupt 401 */
  UTIMER_IRQ25Handler,                      /*   402 Interrupt 402 */
  UTIMER_IRQ26Handler,                      /*   403 Interrupt 403 */
  UTIMER_IRQ27Handler,                      /*   404 Interrupt 404 */
  UTIMER_IRQ28Handler,                      /*   405 Interrupt 405 */
  UTIMER_IRQ29Handler,                      /*   406 Interrupt 406 */
  UTIMER_IRQ30Handler,                      /*   407 Interrupt 407 */
  UTIMER_IRQ31Handler,                      /*   408 Interrupt 408 */
  UTIMER_IRQ32Handler,                      /*   409 Interrupt 409 */
  UTIMER_IRQ33Handler,                      /*   410 Interrupt 410 */
  UTIMER_IRQ34Handler,                      /*   411 Interrupt 411 */
  UTIMER_IRQ35Handler,                      /*   412 Interrupt 412 */
  UTIMER_IRQ36Handler,                      /*   413 Interrupt 413 */
  UTIMER_IRQ37Handler,                      /*   414 Interrupt 414 */
  UTIMER_IRQ38Handler,                      /*   415 Interrupt 415 */
  UTIMER_IRQ39Handler,                      /*   416 Interrupt 416 */
  UTIMER_IRQ40Handler,                      /*   417 Interrupt 417 */
  UTIMER_IRQ41Handler,                      /*   418 Interrupt 418 */
  UTIMER_IRQ42Handler,                      /*   419 Interrupt 419 */
  UTIMER_IRQ43Handler,                      /*   420 Interrupt 420 */
  UTIMER_IRQ44Handler,                      /*   421 Interrupt 421 */
  UTIMER_IRQ45Handler,                      /*   422 Interrupt 422 */
  UTIMER_IRQ46Handler,                      /*   423 Interrupt 423 */
  UTIMER_IRQ47Handler,                      /*   424 Interrupt 424 */
  UTIMER_IRQ48Handler,                      /*   425 Interrupt 425 */
  UTIMER_IRQ49Handler,                      /*   426 Interrupt 426 */
  UTIMER_IRQ50Handler,                      /*   427 Interrupt 427 */
  UTIMER_IRQ51Handler,                      /*   428 Interrupt 428 */
  UTIMER_IRQ52Handler,                      /*   429 Interrupt 429 */
  UTIMER_IRQ53Handler,                      /*   430 Interrupt 430 */
  UTIMER_IRQ54Handler,                      /*   431 Interrupt 431 */
  UTIMER_IRQ55Handler,                      /*   432 Interrupt 432 */
  UTIMER_IRQ56Handler,                      /*   433 Interrupt 433 */
  UTIMER_IRQ57Handler,                      /*   434 Interrupt 434 */
  UTIMER_IRQ58Handler,                      /*   435 Interrupt 435 */
  UTIMER_IRQ59Handler,                      /*   436 Interrupt 436 */
  UTIMER_IRQ60Handler,                      /*   437 Interrupt 437 */
  UTIMER_IRQ61Handler,                      /*   438 Interrupt 438 */
  UTIMER_IRQ62Handler,                      /*   439 Interrupt 439 */
  UTIMER_IRQ63Handler,                      /*   440 Interrupt 440 */
  UTIMER_IRQ64Handler,                      /*   441 Interrupt 441 */
  UTIMER_IRQ65Handler,                      /*   442 Interrupt 442 */
  UTIMER_IRQ66Handler,                      /*   443 Interrupt 443 */
  UTIMER_IRQ67Handler,                      /*   444 Interrupt 444 */
  UTIMER_IRQ68Handler,                      /*   445 Interrupt 445 */
  UTIMER_IRQ69Handler,                      /*   446 Interrupt 446 */
  UTIMER_IRQ70Handler,                      /*   447 Interrupt 447 */
  UTIMER_IRQ71Handler,                      /*   448 Interrupt 448 */
  UTIMER_IRQ72Handler,                      /*   449 Interrupt 449 */
  UTIMER_IRQ73Handler,                      /*   450 Interrupt 450 */
  UTIMER_IRQ74Handler,                      /*   451 Interrupt 451 */
  UTIMER_IRQ75Handler,                      /*   452 Interrupt 452 */
  UTIMER_IRQ76Handler,                      /*   453 Interrupt 453 */
  UTIMER_IRQ77Handler,                      /*   454 Interrupt 454 */
  UTIMER_IRQ78Handler,                      /*   455 Interrupt 455 */
  UTIMER_IRQ79Handler,                      /*   456 Interrupt 456 */
  UTIMER_IRQ80Handler,                      /*   457 Interrupt 457 */
  UTIMER_IRQ81Handler,                      /*   458 Interrupt 458 */
  UTIMER_IRQ82Handler,                      /*   459 Interrupt 459 */
  UTIMER_IRQ83Handler,                      /*   460 Interrupt 460 */
  UTIMER_IRQ84Handler,                      /*   461 Interrupt 461 */
  UTIMER_IRQ85Handler,                      /*   462 Interrupt 462 */
  UTIMER_IRQ86Handler,                      /*   463 Interrupt 463 */
  UTIMER_IRQ87Handler,                      /*   464 Interrupt 464 */
  UTIMER_IRQ88Handler,                      /*   465 Interrupt 465 */
  UTIMER_IRQ89Handler,                      /*   466 Interrupt 466 */
  UTIMER_IRQ90Handler,                      /*   467 Interrupt 467 */
  UTIMER_IRQ91Handler,                      /*   468 Interrupt 468 */
  UTIMER_IRQ92Handler,                      /*   469 Interrupt 469 */
  UTIMER_IRQ93Handler,                      /*   470 Interrupt 470 */
  UTIMER_IRQ94Handler,                      /*   471 Interrupt 471 */
  UTIMER_IRQ95Handler                       /*   472 Interrupt 472 */
                                            /* Interrupts 464 .. 480 are left out */
};

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

#if defined ( __clang__ ) && !defined(__ARMCC_VERSION)

#undef __PROGRAM_START
#define __PROGRAM_START __clang_copy_zero_init

#endif

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
__attribute__((naked))
__NO_RETURN void Reset_Handler(void)
{
  /* Set up the main stack */

  /*
   * Function must be naked to ensure the compiler doesn't use the
   * stack on entry.
   *
   * Only basic asm (no parameters) is permitted for naked functions,
   * so we have to get the values in by text substitution.
   */
#define xstr(s) str(s)
#define str(s) #s
  __asm (
    "LDR     R0, =" xstr(__STACK_LIMIT) "\n\t"
    "LDR     R1, =" xstr(__INITIAL_SP) "\n\t"
    "MSR     MSPLIM, R0\n\t"
    "MSR     MSP, R1\n\t"
    "BL      Reset_Handler_C"
  );
#undef xstr
#undef str
}

__attribute__((used))
__NO_RETURN void Reset_Handler_C(void)
{
  SystemInit();                             /* CMSIS System Initialization */
  __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

/*----------------------------------------------------------------------------
  Default Handler for Faults
 *----------------------------------------------------------------------------*/
void Fault_Handler(void)
{
  while(1);
}

/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
  while(1);
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic pop
#endif
