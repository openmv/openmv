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
#include <assert.h>

#include "npu_cache.h"
#include "stm32n6xx_hal_cacheaxi.h"
#include "ll_aton_config.h"

#if (LL_ATON_PLATFORM != LL_ATON_PLAT_STM32N6)
#error "LL_ATON_PLATFORM should be equal to LL_ATON_PLAT_STM32N6"
#endif

static CACHEAXI_HandleTypeDef hcacheaxi_s;

static inline void npu_cache_init(void)
{
  if(hcacheaxi_s.Instance != CACHEAXI) {
    hcacheaxi_s.Instance = CACHEAXI;
    HAL_CACHEAXI_Init(&hcacheaxi_s);      // Side effect: cacheaxi should be enabled (but one should call npu_enable_cache to be sure)
  }
}
static inline void npu_cache_deinit(void)
{
  if(hcacheaxi_s.Instance == CACHEAXI) {
    HAL_CACHEAXI_DeInit(&hcacheaxi_s);
    hcacheaxi_s.Instance = NULL;
  }
}

void npu_cache_enable(void)
{
  HAL_StatusTypeDef status;
  
  npu_cache_init();
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
  if(hcacheaxi_s.Instance == CACHEAXI) {
    do
    {
      status = HAL_CACHEAXI_Disable(&hcacheaxi_s);
    } while (status == HAL_BUSY);
    npu_cache_deinit();
  }
}

void npu_cache_invalidate(void)
{
  assert(hcacheaxi_s.Instance == CACHEAXI);
  if(hcacheaxi_s.Instance == CACHEAXI) {
    HAL_CACHEAXI_Invalidate(&hcacheaxi_s);	
  }
}

void npu_cache_clean_range(uint32_t start_addr, uint32_t end_addr)
{
  assert(hcacheaxi_s.Instance == CACHEAXI);
  if(hcacheaxi_s.Instance == CACHEAXI) {
    if (start_addr >= end_addr) {  // prevent unpredictable case.
      return;
    }
    HAL_CACHEAXI_CleanByAddr(&hcacheaxi_s, (uint32_t*)start_addr, end_addr-start_addr);
  }
}

void npu_cache_clean_invalidate_range(uint32_t start_addr, uint32_t end_addr)
{
  assert(hcacheaxi_s.Instance == CACHEAXI);
  if(hcacheaxi_s.Instance == CACHEAXI) {
    if (start_addr >= end_addr) {  // prevent unpredictable case.
      return;
    }
    HAL_CACHEAXI_CleanInvalidByAddr(&hcacheaxi_s, (uint32_t*)start_addr, end_addr-start_addr);
  }
}

/* 
  Weak functions called by HAL_CACHEAXI_Init/DeInit implementations 
*/
void HAL_CACHEAXI_MspInit(CACHEAXI_HandleTypeDef *hcacheaxi)
{
  (void)hcacheaxi;
  npu_cache_enable_clocks_and_reset();
}

void HAL_CACHEAXI_MspDeInit(CACHEAXI_HandleTypeDef *hcacheaxi){
  (void)hcacheaxi;
  npu_cache_disable_clocks_and_reset();
}

/*
  Exposed weak functions to be implemented by the user if needed:
  The CACHEAXI IP must be clocked and reset before use.
*/
__weak void npu_cache_enable_clocks_and_reset(void){};
__weak void npu_cache_disable_clocks_and_reset(void){};