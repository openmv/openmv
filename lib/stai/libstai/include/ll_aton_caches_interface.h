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

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "ll_aton_osal.h"
#include "ll_aton_platform.h"

#if !defined(BUILD_AI_NETWORK_RELOC)

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
   *       passing thru the MCU cache) and should be called AFTER that the buffer has been filled
   */
  static inline void LL_ATON_Cache_MCU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
  {
    LL_ATON_OSAL_LOCK_MCU_CACHE();
    mcu_cache_clean_range(virtual_addr, virtual_addr + size);
    LL_ATON_OSAL_UNLOCK_MCU_CACHE();
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
   *       cache (e.g. using a DMA) and should be called BEFORE the buffer gets filled
   */
  static inline void LL_ATON_Cache_MCU_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
  {
    LL_ATON_OSAL_LOCK_MCU_CACHE();
    mcu_cache_invalidate_range(virtual_addr, virtual_addr + size);
    LL_ATON_OSAL_UNLOCK_MCU_CACHE();
  }

  /**
   * @brief perform MCU cache clean & invalidate maintenance operation on an address range
   * @details whenever the content of a buffer is changed by the application (which is especially the case for input
   *          buffers) cache maintenance MUST be taken into account before being able to run the network.
   * @param[in] virtual_addr start address (host-side/virtual) of address range
   * @param[in] size size of address range
   *
   * @note the address range should fulfill alignment constraints with respect to the MCU cache line size
   *       for both its `address` & `size` (to better correspond to what this operation will actually do)!
   * @note this function is intended to handle the case where a buffer has been filled by the MCU/processor
   *       (such passing thru the MCU cache) and is gonna to be modified immediately afterwards by-passing
   *       the MCU/processor cache (e.g. using a DMA). It should be called AFTER that the buffer has been
   *       filled
   */
  static inline void LL_ATON_Cache_MCU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
  {
    LL_ATON_OSAL_LOCK_MCU_CACHE();
    mcu_cache_clean_invalidate_range(virtual_addr, virtual_addr + size);
    LL_ATON_OSAL_UNLOCK_MCU_CACHE();
  }

  /*** NPU cache maintenance functions ***/

  /**
   * @brief perform NPU cache clean maintenance operation on an address range
   * @details whenever the content of a buffer is changed by the application (which is especially the case for input
   *          buffers) cache maintenance MUST be taken into account before being able to run the network.
   * @param[in] address start address (host-side/virtual) of address range
   * @param[in] size size of address range
   *
   * @note this cache maintenance function needs only be called for buffers which are NPU cacheable
   * @note the address range should fulfill alignment constraints with respect to the NPU cache line size
   *       for both its `address` & `size` (to better correspond to what this operation will actually do)!
   * @note this function is intended to handle the case where a buffer has been filled passing thru the NPU cache
   *       and should be called AFTER that the buffer has been filled
   */
  static inline void LL_ATON_Cache_NPU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
  {
    LL_ATON_OSAL_LOCK_NPU_CACHE();
    npu_cache_clean_range(ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR(virtual_addr),
                          ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR(virtual_addr + size));
    LL_ATON_OSAL_UNLOCK_NPU_CACHE();
  }

  /**
   * @brief perform NPU cache clean & invalidate maintenance operation on an address range
   * @details whenever the content of a buffer is changed by the application (which is especially the case for input
   *          buffers) cache maintenance MUST be taken into account before being able to run the network.
   * @param[in] address start address (host-side/virtual) of address range
   * @param[in] size size of address range
   *
   * @note this cache maintenance function needs only be called for buffers which are NPU cacheable
   * @note the address range should fulfill alignment constraints with respect to the NPU cache line size
   *       for both its `address` & `size` (to better correspond to what this operation will actually do)!
   * @note this function is intended to handle the case where a buffer is NPU cacheable and has been filled by-passing
   *       the NPU cache and should be called BEFORE the buffer gets filled
   * @note the NPU cache provides only a "clean & invalidate range" (and not a - pure - "invalidate range") cache
   *       maintenance function which will be called by "stai_ext_cache_npu_clean_invalidate_range()", therefore it is
   *       even more important to call it BEFORE the buffer gets filled
   */
  static inline void LL_ATON_Cache_NPU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
  {
    /* NOTE: The ATON NPU cache does not provide a pure invalidate-range function, but only a clean-invalidate range
       function! One has to take this into account when using `stai_ext_cache_npu_clean_invalidate_range`. */
    LL_ATON_OSAL_LOCK_NPU_CACHE();
    npu_cache_clean_invalidate_range(ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR(virtual_addr),
                                     ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR(virtual_addr + size));
    LL_ATON_OSAL_UNLOCK_NPU_CACHE();
  }

#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_STM32H7P)

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
   *       passing thru the MCU cache) and should be called AFTER that the buffer has been filled
   */
  static inline void LL_ATON_Cache_MCU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
  {
    LL_ATON_OSAL_LOCK_MCU_CACHE();
    mcu_cache_clean_range(virtual_addr, virtual_addr + size);
    LL_ATON_OSAL_UNLOCK_MCU_CACHE();
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
   *       cache (e.g. using a DMA) and should be called BEFORE the buffer gets filled
   */
  static inline void LL_ATON_Cache_MCU_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
  {
    LL_ATON_OSAL_LOCK_MCU_CACHE();
    mcu_cache_invalidate_range(virtual_addr, virtual_addr + size);
    LL_ATON_OSAL_UNLOCK_MCU_CACHE();
  }

  /**
   * @brief perform MCU cache clean & invalidate maintenance operation on an address range
   * @details whenever the content of a buffer is changed by the application (which is especially the case for input
   *          buffers) cache maintenance MUST be taken into account before being able to run the network.
   * @param[in] virtual_addr start address (host-side/virtual) of address range
   * @param[in] size size of address range
   *
   * @note the address range should fulfill alignment constraints with respect to the MCU cache line size
   *       for both its `address` & `size` (to better correspond to what this operation will actually do)!
   * @note this function is intended to handle the case where a buffer has been filled by the MCU/processor
   *       (such passing thru the MCU cache) and is gonna to be modified immediately afterwards by-passing
   *       the MCU/processor cache (e.g. using a DMA). It should be called AFTER that the buffer has been
   *       filled
   */
  static inline void LL_ATON_Cache_MCU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
  {
    LL_ATON_OSAL_LOCK_MCU_CACHE();
    mcu_cache_clean_invalidate_range(virtual_addr, virtual_addr + size);
    LL_ATON_OSAL_UNLOCK_MCU_CACHE();
  }

  /*** NPU cache maintenance functions ***/

  static inline void LL_ATON_Cache_NPU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
  {
  }
  static inline void LL_ATON_Cache_NPU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
  {
  }

#elif (LL_ATON_PLATFORM == LL_ATON_OSAL_USER_IMPL)

  /* Just declare the functions (to be implemented by platform maintainer) */
  /* MCU */
  void LL_ATON_Cache_MCU_Clean_Range(uintptr_t virtual_addr, uint32_t size);
  void LL_ATON_Cache_MCU_Invalidate_Range(uintptr_t virtual_addr, uint32_t size);
  void LL_ATON_Cache_MCU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size);

  /* NPU */
  void LL_ATON_Cache_NPU_Clean_Range(uintptr_t virtual_addr, uint32_t size);
  void LL_ATON_Cache_NPU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size);

#else // Platforms with no caches

  /* MCU */
  static inline void LL_ATON_Cache_MCU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
  {
  }
  static inline void LL_ATON_Cache_MCU_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
  {
  }
  static inline void LL_ATON_Cache_MCU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
  {
  }

  /* NPU */
  static inline void LL_ATON_Cache_NPU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
  {
  }
  static inline void LL_ATON_Cache_NPU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
  {
  }

#endif // Platforms with no caches

#else // defined(BUILD_AI_NETWORK_RELOC)

#if (LL_ATON_PLATFORM != LL_ATON_PLAT_STM32N6)
#error "Relocatable models are only supported for platform LL_ATON_PLAT_STM32N6"
#endif // (LL_ATON_PLATFORM != LL_ATON_PLAT_STM32N6)

/* Just declare the functions (to be implemented in `ll_aton_reloc_callbacks`) */
/* MCU */
void LL_ATON_Cache_MCU_Clean_Range(uintptr_t virtual_addr, uint32_t size);
void LL_ATON_Cache_MCU_Invalidate_Range(uintptr_t virtual_addr, uint32_t size);
void LL_ATON_Cache_MCU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size);

/* NPU */
void LL_ATON_Cache_NPU_Clean_Range(uintptr_t virtual_addr, uint32_t size);
void LL_ATON_Cache_NPU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size);
#endif // defined(BUILD_AI_NETWORK_RELOC)

#ifdef __cplusplus
}
#endif

#endif // __LL_ATON_CACHES_H
