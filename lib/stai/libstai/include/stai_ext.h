/**
  ******************************************************************************
  * @file    stai_ext.h
  * @author  STMicroelectronics
  * @brief   Definitions of ST.AI platform extended public APIs types
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef STAI_EXT_H
#define STAI_EXT_H

#include "stai.h"

/*****************************************************************************/
STAI_API_DECLARE_BEGIN

/*****************************************************************************/
/***  ST.AI extended APIs set section                                      ***/

/*****************************************************************************/
/*** Caching & Cache Maintenance                                           ***/
/**
 * @brief perform MCU cache clean maintenance operation on an address range
 * @details whenever the content of a buffer is changed by the application (which is especially the case for input buffers)
 *          cache maintenance MUST be taken into account before being able to run the network.
 * @param[in] virtual_addr start address (host-side/virtual) of address range
 * @param[in] size size of address range
 * 
 * @return ST.AI error code
 * 
 * @note the address range should fulfill alignment constraints with respect to the MCU cache line size (`STAI_CACHE_MCU_LINESIZE`) 
 *       for both its `address` & `size` (to better correspond to what this operation will actually do)! 
 * @note this function is intended to handle the case where a buffer has been filled by the MCU/processor (such passing thru the MCU cache)
 *       and should be called AFTER that the buffer has been filled
 */
STAI_API_ENTRY
stai_return_code stai_cache_mcu_clean_range(uintptr_t virtual_addr, stai_size size);

/**
 * @brief perform MCU cache invalidate maintenance operation on an address range
 * @details whenever the content of a buffer is changed by the application (which is especially the case for input buffers)
 *          cache maintenance MUST be taken into account before being able to run the network.
 * @param[in] virtual_addr start address (host-side/virtual) of address range
 * @param[in] size size of address range
 * 
 * @return ST.AI error code
 * 
 * @note the address range should fulfill alignment constraints with respect to the MCU cache line size (`STAI_CACHE_MCU_LINESIZE`) 
 *       for both its `address` & `size` (to better correspond to what this operation will actually do)! 
 * @note this function is intended to handle the case where a buffer has been filled by-passing the MCU/processor cache (e.g. using a DMA) 
 *       and should be called BEFORE the buffer gets filled
 */
STAI_API_ENTRY
stai_return_code stai_cache_mcu_invalidate_range(uintptr_t virtual_addr, stai_size size);

/**
 * @brief perform NPU cache clean maintenance operation on an address range
 * @details whenever the content of a buffer is changed by the application (which is especially the case for input buffers)
 *          cache maintenance MUST be taken into account before being able to run the network.
 * @param[in] address start address (host-side/virtual) of address range
 * @param[in] size size of address range
 * 
 * @return ST.AI error code
 * 
 * @note this cache maintainence function needs only be called for buffers which are NPU cacheable
 * @note the address range should fulfill alignment constraints with respect to the NPU cache line size (`STAI_EXT_CACHE_NPU_LINESIZE`) 
 *       for both its `address` & `size` (to better correspond to what this operation will actually do)! 
 * @note this function is intended to handle the case where a buffer has been filled passing thru the NPU cache
 *       and should be called AFTER that the buffer has been filled
 */
STAI_API_ENTRY
stai_return_code stai_ext_cache_npu_clean_range(uintptr_t virtual_addr, stai_size size);

/**
 * @brief perform NPU cache clean & invalidate maintenance operation on an address range
 * @details whenever the content of a buffer is changed by the application (which is especially the case for input buffers)
 *          cache maintenance MUST be taken into account before being able to run the network.
 * @param[in] address start address (host-side/virtual) of address range
 * @param[in] size size of address range
 * 
 * @return ST.AI error code
 * 
 * @note this cache maintainence function needs only be called for buffers which are NPU cacheable
 * @note the address range should fulfill alignment constraints with respect to the NPU cache line size (`STAI_EXT_CACHE_NPU_LINESIZE`) 
 *       for both its `address` & `size` (to better correspond to what this operation will actually do)! 
 * @note this function is intended to handle the case where a buffer is NPU cacheable and has been filled by-passing the NPU cache 
 *       and should be called BEFORE the buffer gets filled
 * @note the NPU cache provides only a "clean & invalidate range" (and not a - pure - "invalidate range") cache maintenance
 *       function which will be called by "stai_ext_cache_npu_clean_invalidate_range()", therefore it is even more important to call it
 *       BEFORE the buffer gets filled 
 */
STAI_API_ENTRY
stai_return_code stai_ext_cache_npu_clean_invalidate_range(uintptr_t virtual_addr, stai_size size);

/***************************************************************************/
/*** Asynchronous Execution ***/
/**
 * @brief execute a WFE
 * @return ST.AI error code
 */
STAI_API_ENTRY
stai_return_code stai_ext_wfe(void);


STAI_API_DECLARE_END

#endif    /* STAI_EXT_H */
