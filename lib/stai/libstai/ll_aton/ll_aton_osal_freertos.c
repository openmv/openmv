/**
 ******************************************************************************
 * @file    ll_aton_osal_freertos.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Interface to FreeRTOS as the underlying OS/platform for ATON
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

#include "ll_aton_config.h"

#if (LL_ATON_OSAL == LL_ATON_OSAL_FREERTOS)

#include <limits.h>
#include <stdbool.h>

/* FreeRTOS include */
#include "ll_aton_osal_freertos.h"

/*** FreeRTOS dependent static variables ***/
static StaticSemaphore_t _dao_mutex_buffer;      // buffer for `_dao_mutex` which holds its state
static StaticSemaphore_t _dao_wait_queue_buffer; // buffer for `_dao_wait_queue` which holds its state
static StaticSemaphore_t _wfe_sem_buffer;        // buffer for `_wfe_sem` which holds its state
static StaticSemaphore_t _cache_mutex_buffer;    // buffer for `_cache_mutex` which holds its state

/* Include common RTOS `.c` template */
#include "ll_aton_osal_rtos_template.c"

#endif // (LL_ATON_OSAL == LL_ATON_OSAL_FREERTOS)
