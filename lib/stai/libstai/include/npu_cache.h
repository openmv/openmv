/**
  ******************************************************************************
  * @file    npu_cache.h
  * @brief   Prototypes of NPU-cache-handling functions (CACHEAXI)
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

#ifndef NPU_CACHE_H
#define NPU_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 
Weak functions to be implemented by the user if needed:
  The CACHEAXI IP must be clocked and reset before use.
*/
void npu_cache_enable_clocks_and_reset(void);
void npu_cache_disable_clocks_and_reset(void);

/*
  The user is responsible for calling npu_cache_enable before using any
  of the CACHEAXI cache maintenance functions below.
  Calling maintenance functions without enabling the cache first will result
  in an assert failure.
*/
void npu_cache_enable(void);
void npu_cache_disable(void);

/* 
  Cache maintenance functions
  reminder: npu_cache_enable() must be called before using any of these functions
*/
void npu_cache_invalidate(void);
void npu_cache_clean_invalidate_range(uint32_t start_addr, uint32_t end_addr);
void npu_cache_clean_range(uint32_t start_addr, uint32_t end_addr);

#ifdef __cplusplus
}
#endif

#endif /* NPU_CACHE_H */
