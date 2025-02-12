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
 * @file     tcm_partition.c
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     06-August-2024
 * @brief    TCM paritioning to non-secure by configuring the SAU and TGU
 *
 *           This routine will be used only for partitioning the TCM for the
 *           non-secure only when the CMSE feature is not enabled.
 *           If complete trustzone is going to be used, please do the
 *           modifications in the respective partition_* header file.
 * @bug      None.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#if defined (M55_HP)
  #include "M55_HP.h"
#elif defined (M55_HE)
  #include "M55_HE.h"
#else
  #error device not specified!
#endif

#include "tcm_partition.h"
#include "tgu_M55.h"

#if !(defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U))

typedef struct
{
  __IOM uint32_t CTRL;                   /*!< Offset: 0x000 (R/W)  SAU Control Register */
  __IM  uint32_t TYPE;                   /*!< Offset: 0x004 (R/ )  SAU Type Register */
  __IOM uint32_t RNR;                    /*!< Offset: 0x008 (R/W)  SAU Region Number Register */
  __IOM uint32_t RBAR;                   /*!< Offset: 0x00C (R/W)  SAU Region Base Address Register */
  __IOM uint32_t RLAR;                   /*!< Offset: 0x010 (R/W)  SAU Region Limit Address Register */
  __IOM uint32_t SFSR;                   /*!< Offset: 0x014 (R/W)  Secure Fault Status Register */
  __IOM uint32_t SFAR;                   /*!< Offset: 0x018 (R/W)  Secure Fault Address Register */
} SAU_Type;

#define SAU_CTRL_ENABLE_Pos                 0U                                            /*!< SAU CTRL: ENABLE Position */
#define SAU_CTRL_ENABLE_Msk                (1UL /*<< SAU_CTRL_ENABLE_Pos*/)               /*!< SAU CTRL: ENABLE Mask */

/* SAU Region Base Address Register Definitions */
#define SAU_RBAR_BADDR_Pos                  5U                                            /*!< SAU RBAR: BADDR Position */
#define SAU_RBAR_BADDR_Msk                 (0x7FFFFFFUL << SAU_RBAR_BADDR_Pos)            /*!< SAU RBAR: BADDR Mask */

/* SAU Region Limit Address Register Definitions */
#define SAU_RLAR_LADDR_Pos                  5U                                            /*!< SAU RLAR: LADDR Position */
#define SAU_RLAR_LADDR_Msk                 (0x7FFFFFFUL << SAU_RLAR_LADDR_Pos)            /*!< SAU RLAR: LADDR Mask */

#define SAU_RLAR_ENABLE_Pos                 0U                                            /*!< SAU RLAR: ENABLE Position */
#define SAU_RLAR_ENABLE_Msk                (1UL /*<< SAU_RLAR_ENABLE_Pos*/)               /*!< SAU RLAR: ENABLE Mask */

#define SAU_BASE                            (SCS_BASE +  0x0DD0UL)                        /*!< Security Attribution Unit */
#define SAU                                 ((SAU_Type       *)     SAU_BASE)             /*!< Security Attribution Unit */

__STATIC_INLINE void SAU_TCM_NS_Setup (void)
{
    SAU->RNR = 0;
    SAU->RBAR = (uint32_t)&ns_region_0_start & SAU_RBAR_BADDR_Msk;
    SAU->RLAR = (((uint32_t)&ns_region_0_end - 1) & SAU_RLAR_LADDR_Msk) | \
                SAU_RLAR_ENABLE_Msk;

    SAU->CTRL = ((1U << SAU_CTRL_ENABLE_Pos) & SAU_CTRL_ENABLE_Msk);
}

void setup_tcm_ns_partition (void)
{
    /* Do nothing if partitions are not defined in the linker script */
    if (&ns_region_0_end == &ns_region_0_start)
        return;

    SAU_TCM_NS_Setup();
    TGU_Setup();
}

#endif /* __ARM_FEATURE_CMSE */