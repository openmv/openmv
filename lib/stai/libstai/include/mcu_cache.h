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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void mcu_cache_invalidate(void);
void mcu_cache_clean(void);
void mcu_cache_clean_invalidate(void);
void mcu_cache_invalidate_range(uint32_t start_addr, uint32_t end_addr);
void mcu_cache_clean_range(uint32_t start_addr, uint32_t end_addr);
void mcu_cache_clean_invalidate_range(uint32_t start_addr, uint32_t end_addr);

#ifdef __cplusplus
}
#endif

#endif // __MCU_CACHE_H
