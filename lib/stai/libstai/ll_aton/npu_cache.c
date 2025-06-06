/**
  ******************************************************************************
  * @file    npu_cache.c
  * @brief   Implementation of NPU-cache-handling functions (CACHEAXI)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "npu_cache.h"
#include "stm32n6xx_hal_cacheaxi.h"

static CACHEAXI_HandleTypeDef hcacheaxi_s;

void npu_cache_init(void)
{
  hcacheaxi_s.Instance = CACHEAXI;
  HAL_CACHEAXI_Init(&hcacheaxi_s);      // Side effect: cacheaxi should be enabled (but one should call npu_enable_cache to be sure)
}

void npu_cache_enable(void)
{
  HAL_StatusTypeDef status;
  // Enable is wrapped in a loop because most of times, the first call returns
  //    HAL_BUSY, resulting in a cache not enabled.
  do
  {
    status = HAL_CACHEAXI_Enable(&hcacheaxi_s);
  } while (status == HAL_BUSY);
}

void npu_cache_disable(void)
{
  HAL_StatusTypeDef status;
  do
  {
    status = HAL_CACHEAXI_Disable(&hcacheaxi_s);
  } while (status == HAL_BUSY);
}

void npu_cache_invalidate(void)
{
  HAL_CACHEAXI_Invalidate(&hcacheaxi_s);
}

void npu_cache_clean_range(uint32_t start_addr, uint32_t end_addr)
{
  HAL_CACHEAXI_CleanByAddr(&hcacheaxi_s, (uint32_t*)start_addr, end_addr-start_addr);
}

void npu_cache_clean_invalidate_range(uint32_t start_addr, uint32_t end_addr)
{
  HAL_CACHEAXI_CleanInvalidByAddr(&hcacheaxi_s, (uint32_t*)start_addr, end_addr-start_addr);
}

void NPU_CACHE_IRQHandler(void)
{
  __NOP();
}
