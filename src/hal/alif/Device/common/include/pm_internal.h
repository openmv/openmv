/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     pm_internal.h
 * @author   Raj Ranjan
 * @email    raj.ranjan@alifsemi.com
 * @version  V1.0.0
 * @date     19-Apr-2023
 * @brief    Power Management Services API
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef PM_INTERNAL_H_
#define PM_INTERNAL_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>


#ifdef  __cplusplus
extern "C"
{
#endif

/* Coprocessor Power Control Register Definitions */
#define ICB_CPPWR_SU11_Pos         22U                             /*!< CPPWR: State Unknown 11 Position */
#define ICB_CPPWR_SU11_Msk        (0x1UL << ICB_CPPWR_SU11_Pos)    /*!< CPPWR: State Unknown 11 Mask */

#define ICB_CPPWR_SU10_Pos         20U                             /*!< CPPWR: State Unknown 10 Position */
#define ICB_CPPWR_SU10_Msk        (0x1UL << ICB_CPPWR_SU10_Pos)    /*!< CPPWR: State Unknown 10 Mask */


/**
  \ingroup  CMSIS_core_register
  \defgroup EWIC_Type     External Wakeup Interrupt Controller Registers
  \brief    Type definitions for the External Wakeup Interrupt Controller Registers (EWIC)
  @{
 */

/**
  @brief  Structure type to access the External Wakeup Interrupt Controller Registers (EWIC).
 */
typedef struct
{
  __IOM uint32_t EWIC_CR;                /*!< Offset: 0x000 (R/W)  EWIC Control Register */
  __IOM uint32_t EWIC_ASCR;              /*!< Offset: 0x004 (R/W)  EWIC Automatic Sequence Control Register */
  __OM  uint32_t EWIC_CLRMASK;           /*!< Offset: 0x008 ( /W)  EWIC Clear Mask Register */
  __IM  uint32_t EWIC_NUMID;             /*!< Offset: 0x00C (R/ )  EWIC Event Number ID Register */
        uint32_t RESERVED0[124U];
  __IOM uint32_t EWIC_MASKA;             /*!< Offset: 0x200 (R/W)  EWIC MaskA Register */
  __IOM uint32_t EWIC_MASKn[15];         /*!< Offset: 0x204 (R/W)  EWIC Maskn Registers */
        uint32_t RESERVED1[112U];
  __IM  uint32_t EWIC_PENDA;             /*!< Offset: 0x400 (R/ )  EWIC PendA Event Register */
  __IOM uint32_t EWIC_PENDn[15];         /*!< Offset: 0x404 (R/W)  EWIC Pendn Event Registers */
        uint32_t RESERVED2[112U];
  __IM  uint32_t EWIC_PSR;               /*!< Offset: 0x600 (R/ )  EWIC Pend Summary Register */
} _EWIC_Type;

/* EWIC Control (EWIC_CR) Register Definitions */
#define EWIC_EWIC_CR_EN_Pos                 0U                                         /*!< EWIC EWIC_CR: EN Position */
#define EWIC_EWIC_CR_EN_Msk                (0x1UL /*<< EWIC_EWIC_CR_EN_Pos*/)          /*!< EWIC EWIC_CR: EN Mask */

/* EWIC Automatic Sequence Control (EWIC_ASCR) Register Definitions */
#define EWIC_EWIC_ASCR_ASPU_Pos             1U                                         /*!< EWIC EWIC_ASCR: ASPU Position */
#define EWIC_EWIC_ASCR_ASPU_Msk            (0x1UL << EWIC_EWIC_ASCR_ASPU_Pos)          /*!< EWIC EWIC_ASCR: ASPU Mask */

#define EWIC_EWIC_ASCR_ASPD_Pos             0U                                         /*!< EWIC EWIC_ASCR: ASPD Position */
#define EWIC_EWIC_ASCR_ASPD_Msk            (0x1UL /*<< EWIC_EWIC_ASCR_ASPD_Pos*/)      /*!< EWIC EWIC_ASCR: ASPD Mask */

/* EWIC Event Number ID (EWIC_NUMID) Register Definitions */
#define EWIC_EWIC_NUMID_NUMEVENT_Pos        0U                                         /*!< EWIC_NUMID: NUMEVENT Position */
#define EWIC_EWIC_NUMID_NUMEVENT_Msk       (0xFFFFUL /*<< EWIC_EWIC_NUMID_NUMEVENT_Pos*/) /*!< EWIC_NUMID: NUMEVENT Mask */

/* EWIC Mask A (EWIC_MASKA) Register Definitions */
#define EWIC_EWIC_MASKA_EDBGREQ_Pos         2U                                         /*!< EWIC EWIC_MASKA: EDBGREQ Position */
#define EWIC_EWIC_MASKA_EDBGREQ_Msk        (0x1UL << EWIC_EWIC_MASKA_EDBGREQ_Pos)      /*!< EWIC EWIC_MASKA: EDBGREQ Mask */

#define EWIC_EWIC_MASKA_NMI_Pos             1U                                         /*!< EWIC EWIC_MASKA: NMI Position */
#define EWIC_EWIC_MASKA_NMI_Msk            (0x1UL << EWIC_EWIC_MASKA_NMI_Pos)          /*!< EWIC EWIC_MASKA: NMI Mask */

#define EWIC_EWIC_MASKA_EVENT_Pos           0U                                         /*!< EWIC EWIC_MASKA: EVENT Position */
#define EWIC_EWIC_MASKA_EVENT_Msk          (0x1UL /*<< EWIC_EWIC_MASKA_EVENT_Pos*/)    /*!< EWIC EWIC_MASKA: EVENT Mask */

/* EWIC Mask n (EWIC_MASKn) Register Definitions */
#define EWIC_EWIC_MASKn_IRQ_Pos             0U                                           /*!< EWIC EWIC_MASKn: IRQ Position */
#define EWIC_EWIC_MASKn_IRQ_Msk            (0xFFFFFFFFUL /*<< EWIC_EWIC_MASKn_IRQ_Pos*/) /*!< EWIC EWIC_MASKn: IRQ Mask */

/* EWIC Pend A (EWIC_PENDA) Register Definitions */
#define EWIC_EWIC_PENDA_EDBGREQ_Pos         2U                                         /*!< EWIC EWIC_PENDA: EDBGREQ Position */
#define EWIC_EWIC_PENDA_EDBGREQ_Msk        (0x1UL << EWIC_EWIC_PENDA_EDBGREQ_Pos)      /*!< EWIC EWIC_PENDA: EDBGREQ Mask */

#define EWIC_EWIC_PENDA_NMI_Pos             1U                                         /*!< EWIC EWIC_PENDA: NMI Position */
#define EWIC_EWIC_PENDA_NMI_Msk            (0x1UL << EWIC_EWIC_PENDA_NMI_Pos)          /*!< EWIC EWIC_PENDA: NMI Mask */

#define EWIC_EWIC_PENDA_EVENT_Pos           0U                                         /*!< EWIC EWIC_PENDA: EVENT Position */
#define EWIC_EWIC_PENDA_EVENT_Msk          (0x1UL /*<< EWIC_EWIC_PENDA_EVENT_Pos*/)    /*!< EWIC EWIC_PENDA: EVENT Mask */

/* EWIC Pend n (EWIC_PENDn) Register Definitions */
#define EWIC_EWIC_PENDn_IRQ_Pos             0U                                           /*!< EWIC EWIC_PENDn: IRQ Position */
#define EWIC_EWIC_PENDn_IRQ_Msk            (0xFFFFFFFFUL /*<< EWIC_EWIC_PENDn_IRQ_Pos*/) /*!< EWIC EWIC_PENDn: IRQ Mask */

/* EWIC Pend Summary (EWIC_PSR) Register Definitions */
#define EWIC_EWIC_PSR_NZ_Pos                1U                                         /*!< EWIC EWIC_PSR: NZ Position */
#define EWIC_EWIC_PSR_NZ_Msk               (0x7FFFUL << EWIC_EWIC_PSR_NZ_Pos)          /*!< EWIC EWIC_PSR: NZ Mask */

#define EWIC_EWIC_PSR_NZA_Pos               0U                                         /*!< EWIC EWIC_PSR: NZA Position */
#define EWIC_EWIC_PSR_NZA_Msk              (0x1UL /*<< EWIC_EWIC_PSR_NZA_Pos*/)        /*!< EWIC EWIC_PSR: NZA Mask */

/*@}*/ /* end of group EWIC_Type */

/**
  @brief WakeUp Interrupt Controller(WIC) Type:-
 */
typedef enum _PM_WIC
{
    PM_WIC_IS_EWIC,                     /*!<  WIC used is EWIC  */
    PM_WIC_IS_IWIC ,                    /*!<  WIC used is IWIC  */
} PM_WIC;

/**
  @brief enum Low power state:-
 */
typedef enum _PM_LPSTATE
{
    PM_LPSTATE_ON,                      /*!<  ON                */
    PM_LPSTATE_ON_CLK_OFF ,             /*!<  ON, clock is off  */
    PM_LPSTATE_RET,                     /*!<  Not supported     */
    PM_LPSTATE_OFF                      /*!<  OFF               */
} PM_LPSTATE;

/* Same address for both RTSS */
/* Cold_Wakeup bit in external system 0/1 */
/*!< EXTSYS0/1 : bit 0 (architecture dependent) */
#define COLD_WAKEUP_Pos                     (0U)
#define COLD_WAKEUP_Msk                     (1U << COLD_WAKEUP_Pos)

/* WIC bit positions in WICCONTROL */
/*!< WICCONTROL: bit 8 (architecture dependent) */
#define WICCONTROL_WIC_Pos                  (8U)
#define WICCONTROL_WIC_Msk                  (1U << WICCONTROL_WIC_Pos)

/* IWIC bit positions in WICCONTROL */
 /*!< WICCONTROL: bit 9 (architecture dependent)*/
#define WICCONTROL_IWIC_Pos                 (9U)
#define WICCONTROL_IWIC_Msk                 (1U << WICCONTROL_IWIC_Pos)

/*!< External Wakeup Interrupt Controller Base Address */
#define _EWIC_BASE                          (0xE0047000UL)

/*!< EWIC configuration struct */
#define _EWIC                               ((_EWIC_Type *)  _EWIC_BASE )

#ifdef  __cplusplus
}
#endif

#endif /* POWER_MANAGEMENT_INTERNAL_H_ */
