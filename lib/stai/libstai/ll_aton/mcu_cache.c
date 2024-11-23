/**
  ******************************************************************************
  * @file    mcu_cache.c
  * @brief   Implementation of MCU cache-handling functions
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

#include "mcu_cache.h"

int mcu_cache_enable(void)
{
  SCB_EnableDCache();
  return 0;
}

int mcu_cache_disable(void)
{
  SCB_DisableDCache();
  return 0;
}
 
int mcu_cache_invalidate(void)
{
  if(mcu_cache_enabled()) {
    SCB_InvalidateDCache();
  }  
  return 0;
}

int mcu_cache_clean(void)
{
  if(mcu_cache_enabled()) {
    SCB_CleanDCache();
  }  
  return 0;
}

int mcu_cache_clean_invalidate(void)
{
  if(mcu_cache_enabled()) {
    SCB_CleanInvalidateDCache();
  }  
  return 0;
}

int mcu_cache_invalidate_range(uint32_t start_addr, uint32_t end_addr) 
{
  if(mcu_cache_enabled()) {
    SCB_InvalidateDCache_by_Addr((volatile void *)start_addr, (int32_t)(end_addr - start_addr));
  }
  return 0;
}

int mcu_cache_clean_range(uint32_t start_addr, uint32_t end_addr) {
  if(mcu_cache_enabled()) {
    SCB_CleanDCache_by_Addr((volatile void *)start_addr, (int32_t)(end_addr - start_addr));
  }
  return 0;
}
 int mcu_cache_clean_invalidate_range(uint32_t start_addr, uint32_t end_addr) {
  if(mcu_cache_enabled()) {
    SCB_CleanInvalidateDCache_by_Addr((volatile void *)start_addr, (int32_t)(end_addr - start_addr));
  }
  return 0;
}
