/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     tcm_partition.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     6-August-2024
 * @brief    TCM Nonsecure Partition Information
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef TCM_PARTITION_H
#define TCM_PARTITION_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined ( __ARMCC_VERSION )
extern const uint32_t Image$$NS_REGION_0$$Base;
extern const uint32_t Image$$NS_REGION_0_PAD$$Base;
static const uint32_t ns_region_0_start __attribute__((weakref("Image$$NS_REGION_0$$Base"))) __STARTUP_RO_DATA_ATTRIBUTE;
static const uint32_t ns_region_0_end __attribute__((weakref("Image$$NS_REGION_0_PAD$$Base"))) __STARTUP_RO_DATA_ATTRIBUTE;
#elif defined ( __GNUC__ )
extern const uint32_t __ns_region_0_start;
extern const uint32_t __ns_region_0_end;
static const uint32_t ns_region_0_start __attribute__((weakref("__ns_region_0_start"))) __STARTUP_RO_DATA_ATTRIBUTE;
static const uint32_t ns_region_0_end __attribute__((weakref("__ns_region_0_end"))) __STARTUP_RO_DATA_ATTRIBUTE;
#else
  #error Unknown compiler.
#endif

/*
 * setup_tcm_ns_partition()
 * Set up the TCM Nonsecure partitioning in SAU and TGU
 */
void setup_tcm_ns_partition (void);

#ifdef __cplusplus
}
#endif

#endif
