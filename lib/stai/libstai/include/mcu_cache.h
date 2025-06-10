/**
  ******************************************************************************
  * @file    mcu_cache.h
  * @brief   Prototypes of MCU cache-handling functions
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

#ifndef __MCU_CACHE_H
#define __MCU_CACHE_H

#include "stm32n6xx_hal.h"

__STATIC_FORCEINLINE int mcu_cache_enabled(void) {
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
   if (SCB->CCR & SCB_CCR_DC_Msk) return 1;  /* return `1` if DCache is enabled */
#endif // (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)

  return 0;
}


int mcu_cache_enable(void);
int mcu_cache_disable(void);
int mcu_cache_invalidate(void);
int mcu_cache_clean(void);
int mcu_cache_clean_invalidate(void);
int mcu_cache_invalidate_range(uint32_t start_addr, uint32_t end_addr);
int mcu_cache_clean_range(uint32_t start_addr, uint32_t end_addr);
int mcu_cache_clean_invalidate_range(uint32_t start_addr, uint32_t end_addr);

#endif // __MCU_CACHE_H
