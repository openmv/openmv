/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
/*
 * Copyright (c) 2020 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/******************************************************************************
 * @file     system_M55.c
 * @author   Rupesh Kumar
 * @email    rupesh@alifsemi.com
 * @brief    CMSIS Device System Source File for
 *           Alif Semiconductor M55_HP / M55_HE Device
 * @version  V1.0.0
 * @date     23. Feb 2021
 * @bug      None
 * @Note	 None
 ******************************************************************************/

#if defined (M55_HP)
  #include "M55_HP.h"
  #include "M55_HP_Config.h"
#elif defined (M55_HE)
  #include "M55_HE.h"
  #include "M55_HE_Config.h"
#else
  #error device not specified!
#endif

#if defined (__ARM_FEATURE_CMSE) &&  (__ARM_FEATURE_CMSE == 3U)
  #if defined (M55_HP)
    #include "partition_M55_HP.h"
  #elif defined (M55_HE)
    #include "partition_M55_HE.h"
  #endif
#endif

#include "tcm_partition.h"
#include "tgu_M55.h"


#if defined (__MPU_PRESENT) && (__MPU_PRESENT == 1U)
  #include <mpu_M55.h>
#endif

#include "app_map.h"
/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define  MHZ            ( 1000000UL)

#ifndef SYSTEM_CLOCK
#if defined (M55_HP)
#define  SYSTEM_CLOCK    (400U * MHZ)
#elif defined (M55_HE)
#define  SYSTEM_CLOCK    (160U * MHZ)
#endif
#endif

/*----------------------------------------------------------------------------
  WICCONTROL register
 *----------------------------------------------------------------------------*/
/* WIC bit positions in WICCONTROL */
#define WICCONTROL_WIC_Pos          (8U)
#define WICCONTROL_WIC_Msk          (1U << WICCONTROL_WIC_Pos)

#if defined(M55_HP)
#define WICCONTROL                  (AON->RTSS_HP_CTRL)
#elif defined(M55_HE)
#define WICCONTROL                  (AON->RTSS_HE_CTRL)
#endif

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
extern const VECTOR_TABLE_Type __VECTOR_TABLE[496];


/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = SYSTEM_CLOCK;


/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)
{
  SystemCoreClock = SYSTEM_CLOCK;
}

/*----------------------------------------------------------------------------
  Get System Core Clock function
 *----------------------------------------------------------------------------*/
uint32_t GetSystemCoreClock (void)
{
  return SystemCoreClock;
}

/*----------------------------------------------------------------------------
  Default Handler for Spurious wakeup
 *----------------------------------------------------------------------------*/
__attribute__ ((weak))
void System_HandleSpuriousWakeup (void)
{
/*
 * pm.c has the implementation to handle the spurious wakeup.
 * User may override and can have their own implementation.
 */
}

/* This hook is called automatically by the ARM C library after scatter loading */
/* We add it to the preinit table for GCC */
void _platform_pre_stackheap_init(void)
{
    /* Synchronise the caches for any copied code */
    if (!(MEMSYSCTL->MSCR & MEMSYSCTL_MSCR_DCCLEAN_Msk))
    {
        SCB_CleanDCache();
    }
    SCB_InvalidateICache();

    /* Enable the Counter module for busy loops */
    sys_busy_loop_init();
}

#if !defined(__ARMCC_VERSION)
void (*_do_platform_pre_stackheap_init)() __attribute__((section(".preinit_array"))) = _platform_pre_stackheap_init;
#endif

/*----------------------------------------------------------------------------
  System initialization function
 *----------------------------------------------------------------------------*/
void SystemInit (void)
{
  // Avoid DSB as long as possible, as it will block until cache
  // auto-invalidation has completed. First DSB is currently at the
  // end of MPU_Setup.

#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
  SCB->VTOR = (uint32_t) __VECTOR_TABLE;
#endif

  /* Enable UsageFault, BusFault, MemFault and SecurityFault exceptions */
  /* Otherwise all you see is HardFault, even in the debugger */
  SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk |
                SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_SECUREFAULTENA_Msk;

  /*
   * Handle Spurious Wakeup
   */
  System_HandleSpuriousWakeup();

  /* Clear the WIC Sleep */
  WICCONTROL &= ~WICCONTROL_WIC_Msk;

#if (defined (__FPU_USED) && (__FPU_USED == 1U)) || \
    (defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
  SCB->CPACR |= ((3U << 10U*2U) |           /* enable CP10 Full Access */
                 (3U << 11U*2U)  );         /* enable CP11 Full Access */
#endif

#ifdef UNALIGNED_SUPPORT_DISABLE
  SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif

 /*
  * Prefetch Control
  *
  * By Reset, the Prefetch is enabled with the below values,
  * MAX_LA = 6
  * MIN_LA = 2
  * MAX_OS = 2
  *
  * Here we modify only the MAX_OS based on the performance achieved in our
  * trials.
  *
  */
 MEMSYSCTL->PFCR = (MEMSYSCTL_PFCR_MAX_OS_DEFAULT_VALUE << MEMSYSCTL_PFCR_MAX_OS_Pos) |
                   (MEMSYSCTL_PFCR_MAX_LA_DEFAULT_VALUE << MEMSYSCTL_PFCR_MAX_LA_Pos) |
                   (MEMSYSCTL_PFCR_MIN_LA_DEFAULT_VALUE << MEMSYSCTL_PFCR_MIN_LA_Pos) |
                    MEMSYSCTL_PFCR_ENABLE_Msk;

#if defined (__MPU_PRESENT) && (__MPU_PRESENT == 1U)
/*
 * Do not do MPU_Setup() if running from the OSPI XIP regions as MPU_Setup() temporarily
 * disables the MPU which causes the default Device/XN attributes to take effect for the
 * OSPI XIP regions.
 */
#if !BOOT_FROM_OSPI_FLASH
  MPU_Setup();
#endif
#endif

  // Enable caches now, for speed, but we will have to clean
  // after scatter-loading, in _platform_pre_stackheap_init

  // We do not use the CMSIS functions, as these manually invalidate the
  // cache - this is not required on the M55, as it is auto-invalidated
  // (and we implicitly rely on this already before activating, if booting
  // from MRAM).
  // Enable Loop and branch info cache
  SCB->CCR |= SCB_CCR_IC_Msk | SCB_CCR_DC_Msk | SCB_CCR_LOB_Msk;

  // Enable limited static branch prediction using low overhead loops
  ICB->ACTLR &= ~ICB_ACTLR_DISLOBR_Msk;

  __DSB();
  __ISB();

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
  TZ_SAU_Setup();
  TGU_Setup();
#else
  setup_tcm_ns_partition();
#endif

  SystemCoreClock = SYSTEM_CLOCK;

  /* Add a feature to bypass the clock gating in the EXPMST0.
   *
   * Note: This will be removed in the future release
   */
#define FORCE_ENABLE_SYSTEM_CLOCKS 1
#if FORCE_ENABLE_SYSTEM_CLOCKS
  /* Bypass clock gating */
  enable_force_peripheral_functional_clk();

  /* Bypass clock gating */
  enable_force_apb_interface_clk();
#endif
}
