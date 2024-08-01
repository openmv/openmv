/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Interrupt handlers.
 */

/**
  ******************************************************************************
  * @file    USB_Device/CDC_Standalone/Src/stm32f4xx_it.c
  * @author  MCD Application Team
  * @version V1.2.2
  * @date    25-May-2015
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

#include STM32_HAL_H 
#include <stm32fxxx_it.h>

extern PCD_HandleTypeDef hpcd;
extern void DCMI_VsyncExtiCallback();
extern TIM_HandleTypeDef TIM5_Handle;
extern DMA_HandleTypeDef *dma_handle[16];

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick(); 
}

/**
  * @brief  This function handles USB-On-The-Go FS global interrupt request.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_FS
void OTG_FS_IRQHandler(void)
#else
void OTG_HS_IRQHandler(void)
#endif
{
  HAL_PCD_IRQHandler(&hpcd);
}

void DMA1_Stream0_IRQHandler(void)
{
    if (dma_handle[0] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[0]);
    }
}

void DMA1_Stream1_IRQHandler(void)
{
    if (dma_handle[1] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[1]);
    }
}

void DMA1_Stream2_IRQHandler(void)
{
    if (dma_handle[2] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[2]);
    }
}

void DMA1_Stream3_IRQHandler(void)
{
    if (dma_handle[3] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[3]);
    }
}

void DMA1_Stream4_IRQHandler(void)
{
    if (dma_handle[4] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[4]);
    }
}

void DMA1_Stream5_IRQHandler(void)
{
    if (dma_handle[5] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[5]);
    }
}

void DMA1_Stream6_IRQHandler(void)
{
    if (dma_handle[6] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[6]);
    }
}

void DMA1_Stream7_IRQHandler(void)
{
    if (dma_handle[7] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[7]);
    }
}

void DMA2_Stream0_IRQHandler(void)
{
    if (dma_handle[8] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[8]);
    }
}

#if 0
void DMA2_Stream1_IRQHandler(void)
{
    if (dma_handle[9] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[9]);
    }
}
#endif

void DMA2_Stream2_IRQHandler(void)
{
    if (dma_handle[10] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[10]);
    }
}

void DMA2_Stream3_IRQHandler(void)
{
    if (dma_handle[11] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[11]);
    }
}

void DMA2_Stream4_IRQHandler(void)
{
    if (dma_handle[12] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[12]);
    }
}

void DMA2_Stream5_IRQHandler(void)
{
    if (dma_handle[13] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[13]);
    }
}

void DMA2_Stream6_IRQHandler(void)
{
    if (dma_handle[14] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[14]);
    }
}

void DMA2_Stream7_IRQHandler(void)
{
    if (dma_handle[15] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[15]);
    }
}
