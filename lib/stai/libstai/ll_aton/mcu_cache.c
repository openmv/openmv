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
#include "stm32n6xx.h"          // Needed for cmsis_compiler (__STATIC_FORCEINLINE) and core-cm55 (tuned for n6-cm55 embodiement -> MCU cache maintenance functions)

__STATIC_FORCEINLINE uint32_t mcu_cache_enabled(void) {
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT != 0U)
  return (SCB->CCR & SCB_CCR_DC_Msk);
#else
  return 0;
#endif
}

void mcu_cache_invalidate(void)
{
  if(mcu_cache_enabled() != 0) {
    SCB_InvalidateDCache();
  }
}

void mcu_cache_clean(void)
{
  if(mcu_cache_enabled() != 0) {
    SCB_CleanDCache();
  }
}

void mcu_cache_clean_invalidate(void)
{
  if(mcu_cache_enabled() != 0) {
    SCB_CleanInvalidateDCache();
  }
}

void mcu_cache_invalidate_range(uint32_t start_addr, uint32_t end_addr)
{
  if(mcu_cache_enabled() != 0) {
    SCB_InvalidateDCache_by_Addr((volatile void *)start_addr, (int32_t)(end_addr - start_addr));
  }
}

void mcu_cache_clean_range(uint32_t start_addr, uint32_t end_addr) {
  if(mcu_cache_enabled() != 0) {
    SCB_CleanDCache_by_Addr((volatile void *)start_addr, (int32_t)(end_addr - start_addr));
  }
}

void mcu_cache_clean_invalidate_range(uint32_t start_addr, uint32_t end_addr) {
  if(mcu_cache_enabled() != 0) {
    SCB_CleanInvalidateDCache_by_Addr((volatile void *)start_addr, (int32_t)(end_addr - start_addr));
  }
}