/**
 ******************************************************************************
 * @file    ll_aton_caches_interface.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file for defining an implementing generic cache handling
 *          functions for the application writer
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

#ifndef __LL_ATON_CACHES_H
#define __LL_ATON_CACHES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "ll_aton_osal.h"
#include "ll_aton_platform.h"

#if (LL_ATON_PLATFORM == LL_ATON_PLAT_STM32N6)

  /*** MCU cache maintenance functions ***/

  /**
   * @brief perform MCU cache clean maintenance operation on an address range
   * @details whenever the content of a buffer is changed by the application (which is especially the case for input
   *          buffers) cache maintenance MUST be taken into account before being able to run the network.
   * @param[in] virtual_addr start address (host-side/virtual) of address range
   * @param[in] size size of address range
   *
   * @note the address range should fulfill alignment constraints with respect to the MCU cache line size
   *       for both its `address` & `size` (to better correspond to what this operation will actually do)!
   * @note this function is intended to handle the case where a buffer has been filled by the MCU/processor (such
   * passing thru the MCU cache) and should be called AFTER that the buffer has been filled
   */
  static inline void LL_ATON_Cache_MCU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
  {
    LL_ATON_LOCK_MCU_CACHE();
    mcu_cache_clean_range((uintptr_t)((unsigned char *)(virtual_addr + 0)),
                          (uintptr_t)((unsigned char *)(virtual_addr + size)));
    LL_ATON_UNLOCK_MCU_CACHE();
  }

  /**
   * @brief perform MCU cache invalidate maintenance operation on an address range
   * @details whenever the content of a buffer is changed by the application (which is especially the case for input
   *          buffers) cache maintenance MUST be taken into account before being able to run the network.
   * @param[in] virtual_addr start address (host-side/virtual) of address range
   * @param[in] size size of address range
   *
   * @note the address range should fulfill alignment constraints with respect to the MCU cache line size
   *       for both its `address` & `size` (to better correspond to what this operation will actually do)!
   * @note this function is intended to handle the case where a buffer has been filled by-passing the MCU/processor
   * cache (e.g. using a DMA) and should be called BEFORE the buffer gets filled
   */
  static inline void LL_ATON_Cache_MCU_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
  {
    LL_ATON_LOCK_MCU_CACHE();
    mcu_cache_invalidate_range((uintptr_t)((unsigned char *)(virtual_addr + 0)),
                               (uintptr_t)((unsigned char *)(virtual_addr + size)));
    LL_ATON_UNLOCK_MCU_CACHE();
  }

  /*** NPU cache maintainence functions ***/

  /**
   * @brief perform NPU cache clean maintenance operation on an address range
   * @details whenever the content of a buffer is changed by the application (which is especially the case for input
   *          buffers) cache maintenance MUST be taken into account before being able to run the network.
   * @param[in] address start address (host-side/virtual) of address range
   * @param[in] size size of address range
   *
   * @note this cache maintainence function needs only be called for buffers which are NPU cacheable
   * @note the address range should fulfill alignment constraints with respect to the NPU cache line size
   *       for both its `address` & `size` (to better correspond to what this operation will actually do)!
   * @note this function is intended to handle the case where a buffer has been filled passing thru the NPU cache
   *       and should be called AFTER that the buffer has been filled
   */
  static inline void LL_ATON_Cache_NPU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
  {
    LL_ATON_LOCK_NPU_CACHE();
    npu_cache_clean_range(__LL_ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR((uintptr_t)((unsigned char *)(virtual_addr + 0))),
                          __LL_ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR((uintptr_t)((unsigned char *)(virtual_addr + size))));
    LL_ATON_UNLOCK_NPU_CACHE();
  }

  /**
   * @brief perform NPU cache clean & invalidate maintenance operation on an address range
   * @details whenever the content of a buffer is changed by the application (which is especially the case for input
   *          buffers) cache maintenance MUST be taken into account before being able to run the network.
   * @param[in] address start address (host-side/virtual) of address range
   * @param[in] size size of address range
   *
   * @note this cache maintainence function needs only be called for buffers which are NPU cacheable
   * @note the address range should fulfill alignment constraints with respect to the NPU cache line size
   *       for both its `address` & `size` (to better correspond to what this operation will actually do)!
   * @note this function is intended to handle the case where a buffer is NPU cacheable and has been filled by-passing
   * the NPU cache and should be called BEFORE the buffer gets filled
   * @note the NPU cache provides only a "clean & invalidate range" (and not a - pure - "invalidate range") cache
   *       maintenance function which will be called by "stai_ext_cache_npu_clean_invalidate_range()", therefore it is
   *       even more important to call it BEFORE the buffer gets filled
   */
  static inline void LL_ATON_Cache_NPU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
  {
    /* NOTE: The ATON NPU cache does not provide a pure invalidate-range function, but only a clean-invalidate range
       function! One has to take this into account when using `stai_ext_cache_npu_clean_invalidate_range`. */
    LL_ATON_LOCK_NPU_CACHE();
    npu_cache_clean_invalidate_range(
        __LL_ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR((uintptr_t)((unsigned char *)(virtual_addr + 0))),
        __LL_ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR((uintptr_t)((unsigned char *)(virtual_addr + size))));
    LL_ATON_UNLOCK_NPU_CACHE();
  }

#else  // (LL_ATON_PLATFORM != LL_ATON_PLAT_STM32N6)
static inline void LL_ATON_Cache_MCU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
{
}
static inline void LL_ATON_Cache_MCU_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
{
}
static inline void LL_ATON_Cache_NPU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
{
}
static inline void LL_ATON_Cache_NPU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
{
}
#endif // (LL_ATON_PLATFORM != LL_ATON_PLAT_STM32N6)

#ifdef __cplusplus
}
#endif

#endif // __LL_ATON_CACHES_H
