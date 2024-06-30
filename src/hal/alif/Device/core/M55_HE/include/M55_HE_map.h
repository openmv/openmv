/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef M55_HE_MAP_H
#define M55_HE_MAP_H

#include "global_map.h"
/******************************************************************************/
/*                         Local memory map                              */
/******************************************************************************/
#define ITCM_BASE                   (0x00000000UL)
#define ITCM_ALIAS_BIT              (0x01000000UL)
#define ITCM_REGION_SIZE            (0x02000000UL)
#define ITCM_SIZE                   (SRAM4_SIZE) /* 256K */
#define ITCM_GLOBAL_BASE            (SRAM4_BASE)

#define DTCM_BASE                   (0x20000000UL)
#define DTCM_ALIAS_BIT              (0x01000000UL)
#define DTCM_REGION_SIZE            (0x02000000UL)
#define DTCM_SIZE                   (SRAM5_SIZE) /* 256K */
#define DTCM_GLOBAL_BASE            (SRAM5_BASE)

#define MHU_A32_M55HE_0_RX_BASE     0x40000000UL
#define MHU_M55HE_A32_0_TX_BASE     0x40010000UL
#define MHU_A32_M55HE_1_RX_BASE     0x40020000UL
#define MHU_M55HE_A32_1_TX_BASE     0x40030000UL
#define MHU_SECPU_M55HE_0_RX_BASE   0x40040000UL
#define MHU_M55HE_SECPU_0_TX_BASE   0x40050000UL
#define MHU_SECPU_M55HE_1_RX_BASE   0x40060000UL
#define MHU_M55HE_SECPU_1_TX_BASE   0x40070000UL
#define MHU_M55HP_M55HE_0_RX_BASE   0x40080000UL
#define MHU_M55HE_M55HP_0_TX_BASE   0x40090000UL
#define MHU_M55HP_M55HE_1_RX_BASE   0x400A0000UL
#define MHU_M55HE_M55HP_1_TX_BASE   0x400B0000UL

#define MHU_APSS_S_RX_BASE          MHU_A32_M55HE_0_RX_BASE
#define MHU_APSS_S_TX_BASE          MHU_M55HE_A32_0_TX_BASE
#define MHU_APSS_NS_RX_BASE         MHU_A32_M55HE_1_RX_BASE
#define MHU_APSS_NS_TX_BASE         MHU_M55HE_A32_1_TX_BASE
#define MHU_SESS_S_RX_BASE          MHU_SECPU_M55HE_0_RX_BASE
#define MHU_SESS_S_TX_BASE          MHU_M55HE_SECPU_0_TX_BASE
#define MHU_SESS_NS_RX_BASE         MHU_SECPU_M55HE_1_RX_BASE
#define MHU_SESS_NS_TX_BASE         MHU_M55HE_SECPU_1_TX_BASE
#define MHU_RTSS_S_RX_BASE          MHU_M55HP_M55HE_0_RX_BASE
#define MHU_RTSS_S_TX_BASE          MHU_M55HE_M55HP_0_TX_BASE
#define MHU_RTSS_NS_RX_BASE         MHU_M55HP_M55HE_1_RX_BASE
#define MHU_RTSS_NS_TX_BASE         MHU_M55HE_M55HP_1_TX_BASE

#define DMA2_SEC_BASE               0x400C0000UL
#define DMA2_NS_BASE                0x400E0000UL
#define DMALOCAL_SEC_BASE           DMA2_SEC_BASE
#define DMALOCAL_NS_BASE            DMA2_NS_BASE

#define NPU_HE_BASE                 0x400E1000UL
#define LOCAL_NPU_BASE              NPU_HE_BASE

#define EVTRTR2_BASE                0x400E2000UL
#define EVTRTRLOCAL_BASE            EVTRTR2_BASE

#define WDT_HE_CTRL_BASE            0x40100000UL
#define WDT_HE_REFRESH_BASE         0x40101000UL
#define LOCAL_WDT_CTRL_BASE         WDT_HE_CTRL_BASE
#define LOCAL_WDT_REFRESH_BASE      WDT_HE_REFRESH_BASE

#endif /* M55_HE_MAP_H */
