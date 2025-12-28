/**
 ******************************************************************************
 * @file    ll_aton_osal_threadx.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Interface to ThreadX as the underlying OS/platform for ATON
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

#if (LL_ATON_OSAL == LL_ATON_OSAL_THREADX)

#include <limits.h>
#include <stdbool.h>

/* ThreadX include */
#include "ll_aton_osal_threadx.h"

/* Include common RTOS `.c` template */
#include "ll_aton_osal_rtos_template.c"

#endif // (LL_ATON_OSAL == LL_ATON_OSAL_THREADX)
