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
 * @file     canfd.h
 * @author   Shreehari H K
 * @email    shreehari.hk@alifsemi.com
 * @version  V1.0.0
 * @date     05-July-2023
 * @brief    Header file for canfd
 * @bug      None.
 * @Note     None.
 ******************************************************************************/
#ifndef CANFD_H_
#define CANFD_H_

#include <stdint.h>
#include <stdbool.h>

/* CANFD register structure */
typedef struct _CANFD_Type
{                                                   /*!< (@ 0x49036000) CANFD Structure                                            */
  volatile const  uint32_t  CANFD_RBUF[20];         /*!< (@ 0x00000000) Receive Buffer Register                                    */
  volatile        uint32_t  CANFD_TBUF[18];         /*!< (@ 0x00000050) Transmit Buffer Register                                   */
  volatile const  uint32_t  CANFD_TTS;              /*!< (@ 0x00000098) Transmission Time Stamp Register                           */
  volatile const  uint32_t  RESERVED;
  volatile        uint8_t   CANFD_CFG_STAT;         /*!< (@ 0x000000A0) Configuration and Status Register                          */
  volatile        uint8_t   CANFD_TCMD;             /*!< (@ 0x000000A1) Command Register                                           */
  volatile        uint8_t   CANFD_TCTRL;            /*!< (@ 0x000000A2) Transmit Control Register                                  */
  volatile        uint8_t   CANFD_RCTRL;            /*!< (@ 0x000000A3) Receive Control Register                                   */
  volatile        uint8_t   CANFD_RTIE;             /*!< (@ 0x000000A4) Receive and Transmit Interrupt Enable Register             */
  volatile        uint8_t   CANFD_RTIF;             /*!< (@ 0x000000A5) Receive and Transmit Interrupt Flag Register               */
  volatile        uint8_t   CANFD_ERRINT;           /*!< (@ 0x000000A6) Error Interrupt Enable and Flag Register                   */
  volatile        uint8_t   CANFD_LIMIT;            /*!< (@ 0x000000A7) Warning Limits Register                                    */
  volatile        uint8_t   CANFD_S_SEG_1;          /*!< (@ 0x000000A8) Slow Speed Bit Timing 1 Register (Segment 1)               */
  volatile        uint8_t   CANFD_S_SEG_2;          /*!< (@ 0x000000A9) Slow Speed Bit Timing 2 Register (Segment 2)               */
  volatile        uint8_t   CANFD_S_SJW;            /*!< (@ 0x000000AA) Slow Speed Bit Timing 3 Register (Synchronization
                                                                        Jump Width)                                                */
  volatile        uint8_t   CANFD_S_PRESC;          /*!< (@ 0x000000AB) Slow Speed Prescaler Register                              */
  volatile        uint8_t   CANFD_F_SEG_1;          /*!< (@ 0x000000AC) Fast Speed Bit Timing 1 Register (Segment 1)               */
  volatile        uint8_t   CANFD_F_SEG_2;          /*!< (@ 0x000000AD) Fast Speed Bit Timing 2 Register (Segment 2)               */
  volatile        uint8_t   CANFD_F_SJW;            /*!< (@ 0x000000AE) Fast Speed Bit Timing 3 Register (Synchronization
                                                                        Jump Width)                                                */
  volatile        uint8_t   CANFD_F_PRESC;          /*!< (@ 0x000000AF) Fast Speed Prescaler Register                              */
  volatile const  uint8_t   CANFD_EALCAP;           /*!< (@ 0x000000B0) Error and Arbitration Lost Capture Register                */
  volatile        uint8_t   CANFD_TDC;              /*!< (@ 0x000000B1) Transmitter Delay Compensation Register                    */
  volatile const  uint8_t   CANFD_RECNT;            /*!< (@ 0x000000B2) Receive Error Counter Register                             */
  volatile const  uint8_t   CANFD_TECNT;            /*!< (@ 0x000000B3) Transmit Error Counter Register                            */
  volatile        uint8_t   CANFD_ACFCTRL;          /*!< (@ 0x000000B4) Acceptance Filter Control Register                         */
  volatile        uint8_t   CANFD_TIMECFG;          /*!< (@ 0x000000B5) CiA 603 Time-Stamping Register                             */
  volatile        uint8_t   CANFD_ACF_EN_0;         /*!< (@ 0x000000B6) Acceptance Filter Enable 0 Register                        */
  volatile        uint8_t   CANFD_ACF_EN_1;         /*!< (@ 0x000000B7) Acceptance Filter Enable 1 Register                        */

  union {
    volatile      uint32_t CANFD_ACF_0_3_CODE;      /*!< (@ 0x000000B8) Acceptance CODE Register                                   */
    volatile      uint32_t CANFD_ACF_0_3_MASK;      /*!< (@ 0x000000B8) Acceptance MASK Register                                   */
  };
  volatile const  uint8_t   CANFD_VER_0;            /*!< (@ 0x000000BC) Reserved                                                   */
  volatile const  uint8_t   CANFD_VER_1;            /*!< (@ 0x000000BD) Reserved                                                   */
  volatile const  uint16_t  RESERVED1;
  volatile const  uint32_t  RESERVED2[2];
  volatile const  uint16_t  RESERVED3;
  volatile        uint8_t   CANFD_MEM_PROT;         /*!< (@ 0x000000CA) Memory Protection Register                                 */
  volatile        uint8_t   CANFD_MEM_STAT;         /*!< (@ 0x000000CB) Memory Status Register                                     */
  volatile        uint8_t   CANFD_MEM_ES_0;         /*!< (@ 0x000000CC) Memory Error Stimulation 0 Register                        */
  volatile        uint8_t   CANFD_MEM_ES_1;         /*!< (@ 0x000000CD) Memory Error Stimulation 1 Register                        */
  volatile        uint8_t   CANFD_MEM_ES_2;         /*!< (@ 0x000000CE) Memory Error Stimulation 2 Register                        */
  volatile        uint8_t   CANFD_MEM_ES_3;         /*!< (@ 0x000000CF) Memory Error Stimulation 3 Register                        */
  volatile        uint8_t   CANFD_SRCFG;            /*!< (@ 0x000000D0) Spatial Redundancy Configuration Register                  */
  volatile const  uint8_t   RESERVED4;
  volatile const  uint16_t  RESERVED5;
}CANFD_Type;                                        /*!< Size = 212 (0xd4)                                                         */

/* CANFD register structure */
typedef struct _CANFD_CNT_Type
{
    volatile      uint32_t  CANFD_CNTR_CTRL;        /*!< (@ 0x00000000) Counter control register                                   */
    volatile      uint32_t  CANFD_CNTR_LOW;         /*!< (@ 0x00000004) Counter Low register                                       */
    volatile      uint32_t  CANFD_CNTR_HIGH;        /*!< (@ 0x00000008) Counter High register                                      */
}CANFD_CNT_Type;

/* Hardware related configuration */

#define CANFD_MAX_BITRATE                        10000000U
#define CANFD_MAX_ACCEPTANCE_FILTERS             3U
#define CANFD_MAX_ERROR_WARN_LIMIT               128U
#define CANFD_NOM_DATA_FRAME_SIZE_MIN            0U
#define CANFD_NOM_DATA_FRAME_SIZE_MAX            8U
#define CANFD_FAST_DATA_FRAME_SIZE_MIN           0U
#define CANFD_FAST_DATA_FRAME_SIZE_MAX           64U
#define CANFD_DECREMENT(x, pos)                  (x - pos)
#define CANFD_TRANSCEIVER_STANDBY_DELAY          3U                /* CANFD transceiver requires 3us to switch from Stdby to Dominant state  */

/* Macros for CANFD specifications */
#define CANFD_SPEC_NON_ISO                       0U
#define CANFD_SPEC_ISO                           1U

/* Macros for RBUF overflow mode */
#define CANFD_RBUF_OVF_MODE_OVERWRITE_OLD_MSG    0U
#define CANFD_RBUF_OVF_MODE_DISCARD_NEW_MSG      1U

/* Macros for RBUF Storage format */
#define CANFD_RBUF_STORE_NORMAL_MSG              0U
#define CANFD_RBUF_STORE_ALL_MSG                 1U

#define CANFD_RBUF_AFWL_MAX                      15U               /* Max AFWL value is 15 */

/* Macros for acpt filter config */
#define CANFD_ACPT_FILTER_CFG_ALL_FRAMES         0U
#define CANFD_ACPT_FILTER_CFG_STD_FRAMES         2U
#define CANFD_ACPT_FILTER_CFG_EXT_FRAMES         3U

/* Macros for Tx buffer to choose */
#define CANFD_BUF_TYPE_PRIMARY                   0U
#define CANFD_BUF_TYPE_SECONDARY                 1U
#define CANFD_SECONDARY_BUF_MODE_FIFO            0U
#define CANFD_SECONDARY_BUF_MODE_PRIORITY        1U

/* Macros for Timestamp poistion to choose */
#define CANFD_TIMESTAMP_POSITION_SOF             0U
#define CANFD_TIMESTAMP_POSITION_EOF             1U

/* Macros for Counter control register */
#define CANFD_CNTR_CTRL_CNTR_CLEAR               (1U << 2U)
#define CANFD_CNTR_CTRL_CNTR_STOP                (1U << 1U)
#define CANFD_CNTR_CTRL_CNTR_START               (1U << 0U)

/* Macros for Configuration and status register */
#define CANFD_CFG_STAT_RESET                     (1U << 7U)
#define CANFD_CFG_STAT_LBME                      (1U << 6U)
#define CANFD_CFG_STAT_LBMI                      (1U << 5U)
#define CANFD_CFG_STAT_TPSS                      (1U << 4U)
#define CANFD_CFG_STAT_TSSS                      (1U << 3U)
#define CANFD_CFG_STAT_RACTIVE                   (1U << 2U)
#define CANFD_CFG_STAT_TACTIVE                   (1U << 1U)
#define CANFD_CFG_STAT_BUSOFF                    (1U << 0U)

/* Macros for Command Register */
#define CANFD_TCMD_TBSEL                         (1U << 7U)        /* 0-Primary, 1-Secondary */
#define CANFD_TCMD_LOM                           (1U << 6U)
#define CANFD_TCMD_STBY                          (1U << 5U)
#define CANFD_TCMD_TPE                           (1U << 4U)
#define CANFD_TCMD_TPA                           (1U << 3U)
#define CANFD_TCMD_TSONE                         (1U << 2U)
#define CANFD_TCMD_TSALL                         (1U << 1U)
#define CANFD_TCMD_TSA                           (1U << 0U)

/* Macros for Transmit Control Register */
#define CANFD_TCTRL_FD_ISO                       (1U << 7U)
#define CANFD_TCTRL_TSNEXT                       (1U << 6U)
#define CANFD_TCTRL_TSMODE                       (1U << 5U)        /* 0-FIFO, 1-Priority */
#define CANFD_TCTRL_TSSTAT_Pos                   (0U)
#define CANFD_TCTRL_TSSTAT_Msk                   (3U << CANFD_TCTRL_TSSTAT_Pos)
#define CANFD_TCTRL_SEC_BUF_FULL                 (3U)

/* Macros for Reception Control Register */
#define CANFD_RCTRL_SACK                         (1U << 7U)
#define CANFD_RCTRL_ROM                          (1U << 6U)        /* 0 - Old msg overwritten, 1 - New msg discarded */
#define CANFD_RCTRL_ROV                          (1U << 5U)
#define CANFD_RCTRL_RREL                         (1U << 4U)
#define CANFD_RCTRL_RBALL                        (1U << 3U)        /* 0 - Normal frames, 1 - Normal and error frames */
#define CANFD_RCTRL_RSTAT_Pos                    (0U)
#define CANFD_RCTRL_RSTAT_Msk                    (3U << CANFD_RCTRL_RSTAT_Pos)

/* Macros for Receive and Transmit Interrupt enable register */
#define CANFD_RTIE_RIE                           (1U << 7U)
#define CANFD_RTIE_ROIE                          (1U << 6U)
#define CANFD_RTIE_RFIE                          (1U << 5U)
#define CANFD_RTIE_RAFIE                         (1U << 4U)
#define CANFD_RTIE_TPIE                          (1U << 3U)
#define CANFD_RTIE_TSIE                          (1U << 2U)
#define CANFD_RTIE_EIE                           (1U << 1U)
#define CANFD_RTIE_TSFF                          (1U << 0U)

/* Macros for Receive and Transmit Interrupt Flag register */
#define CANFD_RTIF_REG_Msk                       (255U)
#define CANFD_RTIF_RIF                           (1U << 7U)
#define CANFD_RTIF_ROIF                          (1U << 6U)
#define CANFD_RTIF_RFIF                          (1U << 5U)
#define CANFD_RTIF_RAFIF                         (1U << 4U)
#define CANFD_RTIF_TPIF                          (1U << 3U)
#define CANFD_RTIF_TSIF                          (1U << 2U)
#define CANFD_RTIF_EIF                           (1U << 1U)
#define CANFD_RTIF_AIF                           (1U << 0U)

/* Macros for Error Interrupt Enable and Flag Register */
#define CANFD_ERRINT_REG_Msk                     (21U)             /* Mask for Error Flags */
#define CANFD_ERRINT_EN_Msk                      (42)              /* Mask for Error interrupt enabling */
#define CANFD_ERRINT_EWARN                       (1U << 7U)
#define CANFD_ERRINT_EPASS                       (1U << 6U)
#define CANFD_ERRINT_EPIE                        (1U << 5U)
#define CANFD_ERRINT_EPIF                        (1U << 4U)
#define CANFD_ERRINT_ALIE                        (1U << 3U)
#define CANFD_ERRINT_ALIF                        (1U << 2U)
#define CANFD_ERRINT_BEIE                        (1U << 1U)
#define CANFD_ERRINT_BEIF                        (1U << 0U)

/* Macros for Warning Limits Register */
#define CANFD_LIMIT_AFWL_Pos                     (4U)
#define CANFD_LIMIT_AFWL_Msk                     (0xFU << CANFD_LIMIT_AFWL_Pos) /* Max limit is 15 */
#define CANFD_LIMIT_AFWL(x)                      ((x << CANFD_LIMIT_AFWL_Pos) & (CANFD_LIMIT_AFWL_Msk))

#define CANFD_LIMIT_EWL_Pos                      (0U)
#define CANFD_LIMIT_EWL_Msk                      (15U << CANFD_LIMIT_EWL_Pos)
#define CANFD_LIMIT_EWL(x)                       ((x << CANFD_LIMIT_EWL_Pos) & (CANFD_LIMIT_EWL_Msk))

/* Macros for Error and Arbitration Lost Capture Register */
#define CANFD_EALCAP_KOER_Pos                    (5U)
#define CANFD_EALCAP_KOER_Msk                    (7U << CANFD_EALCAP_KOER_Pos)

/* Macros for Kind Errors */
#define CANFD_EALCAP_KOER_NONE                   (0U)
#define CANFD_EALCAP_KOER_BIT                    (1U)
#define CANFD_EALCAP_KOER_FORM                   (2U)
#define CANFD_EALCAP_KOER_STUFF                  (3U)
#define CANFD_EALCAP_KOER_ACK                    (4U)
#define CANFD_EALCAP_KOER_CRC                    (5U)
#define CANFD_EALCAP_KOER_OTHER                  (6U)

/* Macros for Arbitration Lost Capture Register */
#define CANFD_EALCAP_ALC_Pos                     (0U)
#define CANFD_EALCAP_ALC_Msk                     (31U << CANFD_EALCAP_ALC_Pos)

/* Macros for Transmitter Delay Compensation Register */
#define CANFD_TDC_TDCEN                          (1U << 7U)
#define CANFD_TDC_SSPOFF_Pos                     (0U)
#define CANFD_TDC_SSPOFF_Msk                     (0x7FU << CANFD_TDC_SSPOFF_Pos)

/* Macros for Acceptance Filter Control Register */
#define CANFD_ACFCTRL_SELMASK                    (1U << 5U)     /* Acceptance mask, 0- Acceptance code */
#define CANFD_ACFCTRL_ACFADR_Pos                 (0U)
#define CANFD_ACFCTRL_ACFADR_Msk                 (0xFU << CANFD_ACFCTRL_ACFADR_Pos)

/* Macros for Acceptance filter Enable Register */
#define CANFD_ACF_EN_0_AE_X_MAX_Msk              (7U << 0U)

/* Macros for Acceptance CODE and MASK Register */
#define CANFD_ACF0_3_AMASK_ACODE_X_Pos           (0U)
#define CANFD_ACF0_3_AMASK_ACODE_X_Msk           (0x1FFFFFFFU << CANFD_ACF0_3_AMASK_ACODE_X_Pos)
#define CANFD_ACF0_3_ACODE_X(x)                  (x & CANFD_ACF0_3_AMASK_ACODE_X_Msk)
#define CANFD_ACF0_3_AMASK_X_Msk(x)              (x & CANFD_ACF0_3_AMASK_ACODE_X_Msk)

#define CANFD_ACF_3_MASK_AIDEE                   (1U << 30U)     /* 1- Accepts either std or ext frame, 0 - accepts both */
#define CANFD_ACF_3_MASK_AIDE_Pos                (29U)
#define CANFD_ACF_3_MASK_AIDE                    (1U << 29U)     /* 1 - Accepts only extended frame, 0-Accepts only std frames */

/* Macros for Time config Register */
#define CANFD_TIMECFG_TIMEPOS                    (1U << 1U)      /* 1- Timestamp at EOF, 0- at SOF */
#define CANFD_TIMECFG_TIMEEN                     (1U << 0U)

/* Macros for Memory Protection Register */
#define CANFD_MEM_PROT_MAEIF                     (1U << 4U)
#define CANFD_MEM_PROT_MDEIF                     (1U << 3U)
#define CANFD_MEM_PROT_MDWIF                     (1U << 2U)
#define CANFD_MEM_PROT_MDWIE                     (1U << 1U)
#define CANFD_MEM_PROT_MPEN                      (1U << 0U)

/* Macros for Memory Status Register bit info*/
#define CANFD_MEM_STAT_HELOC_Pos                 (3U)
#define CANFD_MEM_STAT_HELOC_Msk                 (3U << CANFD_MEM_STAT_HELOC_Pos)
#define CANFD_MEM_STAT_TXB                       (1U << 2U)
#define CANFD_MEM_STAT_TXS                       (1U << 1U)
#define CANFD_MEM_STAT_ACFA                      (1U << 0U)    /* 0-Filter enabled, 1-Filter Disabled */

/* Macros for Memory Error Stimulation 0 Register */
#define CANFD_MEM_ES_0_MEAEE                     (1U << 7U)
#define CANFD_MEM_ES_0_ME1EE                     (1U << 6U)
#define CANFD_MEM_ES_0_MEBP1_Msk                 (3FU << 0U)

/* Macros for Memory Error Stimulation 1 Register */
#define CANFD_MEM_ES_1_ME2EE                     (1U << 6U)
#define CANFD_MEM_ES_1_MEBP2_Msk                 (3FU << 0U)

/* Macros for Memory Error Stimulation 2 Register */
#define CANFD_MEM_ES_2_MENEC                     (15U << 4U)
#define CANFD_MEM_ES_2_MEEEC                     (15U << 0U)

/* Macros for Memory Error Stimulation 3 Register */
#define CANFD_MEM_ES_3_MES                       (1U << 2U)    /* 0-Host side, 1-CAN side */
#define CANFD_MEM_ES_3_MEL_Pos                   (0U)
#define CANFD_MEM_ES_3_MEL_Msk                   (3U << 0U)
#define CANFD_MEM_ES_3_MEL(x)                    (x & CANFD_MEM_ES_3_MEL_Msk)


/* Macros for Spatial Redundancy Configuration Register */
#define CANFD_SRCFG_SREEC                        (1U << 4U)    /* 0-No error stimulation, 1-Error stimulation */
#define CANFD_SRCFG_SREEH                        (1U << 3U)    /* 0-No error stimulation, 1-Error stimulation */
#define CANFD_SRCFG_SRIEF                        (1U << 2U)
#define CANFD_SRCFG_SRISEL                       (1U << 1U)
#define CANFD_SRCFG_SREN                         (1U << 0U)

/* Macros for CAN Msg access */
#define CANFD_MSG_TTSEN                          (1U << 31U)
#define CANFD_MSG_ESI_Pos                        (31U)
#define CANFD_MSG_ESI_Msk                        (1U << CANFD_MSG_ESI_Pos)
#define CANFD_MSG_ESI(x)                         (x << 31U)
#define CANFD_MSG_IDE_Pos                        (7U)
#define CANFD_MSG_IDE_Msk                        (1U << CANFD_MSG_IDE_Pos)
#define CANFD_MSG_IDE(x)                         ((x << CANFD_MSG_IDE_Pos) & CANFD_MSG_IDE_Msk)
#define CANFD_MSG_RTR_Pos                        (6U)
#define CANFD_MSG_RTR_Msk                        (1U << CANFD_MSG_RTR_Pos)
#define CANFD_MSG_RTR(x)                         ((x << CANFD_MSG_RTR_Pos) & CANFD_MSG_RTR_Msk)
#define CANFD_MSG_FDF_Pos                        (5U)
#define CANFD_MSG_FDF_Msk                        (1U << CANFD_MSG_FDF_Pos)
#define CANFD_MSG_FDF(x)                         ((x << CANFD_MSG_FDF_Pos) & CANFD_MSG_FDF_Msk)
#define CANFD_MSG_BRS_Pos                        (4U)
#define CANFD_MSG_BRS_Msk                        (1U << CANFD_MSG_BRS_Pos)
#define CANFD_MSG_BRS(x)                         ((x << CANFD_MSG_BRS_Pos) & CANFD_MSG_BRS_Msk)
#define CANFD_MSG_DLC_Pos                        (0U)
#define CANFD_MSG_DLC_Msk                        (15U << CANFD_MSG_DLC_Pos)
#define CANFD_MSG_DLC(x)                         (x & CANFD_MSG_DLC_Msk)

/* Macros for Bit time segments */
#define CANFD_BIT_PROP_SEG_Pos                   (0U)
#define CANFD_BIT_PHASE_SEG1_Pos                 (8U)
#define CANFD_BIT_PHASE_SEG2_Pos                 (16)
#define CANFD_BIT_SJW_Pos                        (24U)

/* Macros for Interrupt events */
#define CANFD_TX_ABORT_EVENT                     (1U << 0U)
#define CANFD_ERROR_EVENT                        (1U << 1U)
#define CANFD_SECONDARY_BUF_TX_COMPLETE_EVENT    (1U << 2U)
#define CANFD_PRIMARY_BUF_TX_COMPLETE_EVENT      (1U << 3U)
#define CANFD_RBUF_ALMOST_FULL_EVENT             (1U << 4U)
#define CANFD_RBUF_FULL_EVENT                    (1U << 5U)
#define CANFD_RBUF_OVERRUN_EVENT                 (1U << 6U)
#define CANFD_RBUF_AVAILABLE_EVENT               (1U << 7U)
#define CANFD_BUS_ERROR_EVENT                    (1U << 8U)
#define CANFD_ARBTR_LOST_EVENT                   (1U << 10U)
#define CANFD_ERROR_PASSIVE_EVENT                (1U << 12U)

/* CANFD Acceptance Filter Operation codes */
typedef enum _CANFD_ACPT_FLTR_OP
{
  CANFD_ACPT_FLTR_OP_ADD_EXACT_ID,          /* Add exact id filter */
  CANFD_ACPT_FLTR_OP_REMOVE_EXACT_ID,       /* Remove exact id filter */
  CANFD_ACPT_FLTR_OP_ADD_MASKABLE_ID,       /* Add maskable id filter */
  CANFD_ACPT_FLTR_OP_REMOVE_MASKABLE_ID     /* Remove maskable id filter */
}CANFD_ACPT_FLTR_OP;

/* CANFD Message errors*/
typedef enum _CANFD_MSG_ERROR
{
    CANFD_MSG_ERROR_NONE          = 0x0,
    CANFD_MSG_ERROR_BIT           = 0x1,
    CANFD_MSG_ERROR_FORM          = 0x2,
    CANFD_MSG_ERROR_STUFF         = 0x3,
    CANFD_MSG_ERROR_ACK           = 0x4,
    CANFD_MSG_ERROR_CRC           = 0x5
}CANFD_MSG_ERROR;

/* CANFD Bus Status */
typedef enum _CANFD_BUS_STATUS
{
    CANFD_BUS_STATUS_ON   = 0x0,
    CANFD_BUS_STATUS_OFF  = 0x1
}CANFD_BUS_STATUS;

/* CANFD Acceptance filter status */
typedef enum _CANFD_ACPT_FLTR_STATUS
{
    CANFD_ACPT_FLTR_STATUS_NONE         = 0x0,
    CANFD_ACPT_FLTR_STATUS_FREE         = 0x1,
    CANFD_ACPT_FLTR_STATUS_OCCUPIED     = 0x2
}CANFD_ACPT_FLTR_STATUS;

/* CANFD Acceptance filter structure */
typedef struct _canfd_acpt_fltr_t
{
    CANFD_ACPT_FLTR_OP  op_code;
    uint32_t            ac_code;
    uint32_t            ac_mask;
    uint8_t             filter;
    uint8_t             frame_type;
}canfd_acpt_fltr_t;

/* CANFD Transmit Buffer Registers' structure:
 * for Hardware register access */
typedef struct _tbuf_regs_t
{
    uint32_t    can_id;
    uint32_t    control;
    uint8_t     data[CANFD_FAST_DATA_FRAME_SIZE_MAX];     /* Maximum payload size is 64 bytes */
}tbuf_regs_t;

/* CANFD Receive Buffer Registers' structure:
 * for Hardware register access */
typedef volatile const struct _rbuf_regs_t
{
    uint32_t    can_id;
    uint8_t     control;
    uint8_t     status;
    uint8_t     reserved[2U];
    uint8_t     data[CANFD_FAST_DATA_FRAME_SIZE_MAX];     /* Maximum payload size is 64 bytes */
    uint32_t    rx_timestamp[2U];
}rbuf_regs_t;

/* Current tx info */
typedef struct _canfd_tx_info_t
{
  uint32_t id;                          /* CAN identifier                                      */
  uint32_t frame_type       : 1;        /* frame type - Normal/Extended                        */
  uint32_t rtr              : 1;        /* Remote transmission request frame                   */
  uint32_t edl              : 1;        /* Flexible data-rate format extended data length      */
  uint32_t brs              : 1;        /* Flexible data-rate format with bitrate switch       */
  uint32_t esi              : 1;        /* Flexible data-rate format error state indicator     */
  uint32_t dlc              : 4;        /* Data length code                                    */
  uint32_t buf_type         : 1;        /* Tx Buffer type (Primary/Secondary)                  */
  uint32_t reserved         : 22;
}canfd_tx_info_t;

/* Current Rx info */
typedef struct _canfd_rx_info_t
{
  uint32_t id;                          /* CAN identifier with frame format specifier (bit 31) */
  uint32_t frame_type       : 1;        /* frame type - Normal/Extended                        */
  uint32_t rtr              : 1;        /* Remote transmission request frame                   */
  uint32_t edl              : 1;        /* Flexible data-rate format extended data length      */
  uint32_t brs              : 1;        /* Flexible data-rate format with bitrate switch       */
  uint32_t esi              : 1;        /* Flexible data-rate format error state indicator     */
  uint32_t dlc              : 4;        /* Data length code                                    */
  uint32_t status           : 8;        /* KOER and a bit to show LBMI msg                     */
  uint32_t reserved         : 15;
  uint32_t timestamp[2U];               /* Rx Timestamp                                        */
}canfd_rx_info_t;

/* CANFD Driver Data Transfer info */
typedef struct _canfd_transfer_t
{
    const    uint8_t    *tx_ptr;        /* Pointer to Tx buffer                        */
    uint8_t             *rx_ptr;        /* Pointer to Rx buffer                        */
    volatile uint8_t    tx_count;       /* Current Transmission completed data count   */
    volatile uint8_t    rx_count;       /* Current Reception completed data count      */
    canfd_tx_info_t     tx_header;      /* Tx Data header                              */
    canfd_rx_info_t     rx_header;      /* Rx Data header                              */
}canfd_transfer_t;

/**
  \fn          static inline void canfd_reset(CANFD_Type* canfd)
  \brief       Resets CANFD instance.
  \param[in]   canfd  : Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_reset(CANFD_Type* canfd)
{
    /* Resets CANFD module*/
    canfd->CANFD_CFG_STAT |= CANFD_CFG_STAT_RESET;
}

/**
  \fn          static inline void canfd_set_specification(CANFD_Type* canfd,
  \                                                       const uint8_t spec)
  \brief       Sets CANFD mode.
  \param[in]   canfd : Pointer to the CANFD register map
  \param[in]   spec  : Specification type (ISO/Non-ISO)
  \return      none
*/
static inline void canfd_set_specification(CANFD_Type* canfd,
                                           const uint8_t spec)
{
    if(spec == CANFD_SPEC_NON_ISO)
    {
        /* Set NON-ISO mode*/
        canfd->CANFD_TCTRL &= (~CANFD_TCTRL_FD_ISO);
    }
    else
    {
        /* Set ISO mode*/
        canfd->CANFD_TCTRL |= CANFD_TCTRL_FD_ISO;
    }
}

/**
  \fn          static inline void canfd_enable_timestamp(CANFD_Type* canfd,
  \                                                      const uint8_t position)
  \brief       Enables the CANFD Timestamp
  \param[in]   canfd    : Pointer to the CANFD register map
  \param[in]   position : SOF/EOF
  \return      None
*/
static inline void canfd_enable_timestamp(CANFD_Type* canfd,
                                          const uint8_t position)
{
    if(position == CANFD_TIMESTAMP_POSITION_EOF)
    {
        /* Enables timestamp at EOF */
        canfd->CANFD_TIMECFG = (CANFD_TIMECFG_TIMEEN | CANFD_TIMECFG_TIMEPOS);
    }
    else
    {
        /* Enables timestamp at SOF */
        canfd->CANFD_TIMECFG = CANFD_TIMECFG_TIMEEN;
    }
}

/**
  \fn          static inline void canfd_disable_timestamp(CANFD_Type* canfd)
  \brief       Disables the CANFD Timestamp
  \param[in]   canfd    : Pointer to the CANFD register map
  \return      None
*/
static inline void canfd_disable_timestamp(CANFD_Type* canfd)
{
    canfd->CANFD_TIMECFG &= ~CANFD_TIMECFG_TIMEEN;
}

/**
  \fn          static inline uint32_t canfd_get_tx_timestamp(CANFD_Type* canfd)
  \brief       Gets the latest CANFD Tx msg's Timestamp
  \param[in]   canfd    : Pointer to the CANFD register map
  \return      Tx Timestamp
*/
static inline uint32_t canfd_get_tx_timestamp(CANFD_Type* canfd)
{
    /* Returns Tx message timestamp */
    return (uint32_t)canfd->CANFD_TTS;
}

/**
  \fn          static inline void canfd_counter_set(CANFD_CNT_Type* canfd_cntr,
  \                                                 const uint32_t value)
  \brief       Sets the CANFD low timer counter
  \param[in]   canfd_cntr : Pointer to the CANFD counter map
  \param[in]   value      : Countet value
  \return      None
*/
static inline void canfd_counter_set(CANFD_CNT_Type* canfd_cntr,
                                     const uint32_t value)
{
    canfd_cntr->CANFD_CNTR_CTRL = CANFD_CNTR_CTRL_CNTR_STOP;
    canfd_cntr->CANFD_CNTR_LOW  = value;
}

/**
  \fn          static inline void canfd_counter_start(CANFD_CNT_Type* canfd_cntr)
  \brief       Starts the CANFD timer counter
  \param[in]   canfd_cntr : Pointer to the CANFD counter map
  \return      None
*/
static inline void canfd_counter_start(CANFD_CNT_Type* canfd_cntr)
{
    canfd_cntr->CANFD_CNTR_CTRL = CANFD_CNTR_CTRL_CNTR_START;
}

/**
  \fn          static inline void canfd_counter_stop(CANFD_CNT_Type* canfd_cntr)
  \brief       Stops the CANFD timer counter
  \param[in]   canfd_cntr : Pointer to the CANFD counter map
  \return      None
*/
static inline void canfd_counter_stop(CANFD_CNT_Type* canfd_cntr)
{
    canfd_cntr->CANFD_CNTR_CTRL = CANFD_CNTR_CTRL_CNTR_STOP;
}

/**
  \fn          static inline void canfd_counter_clear(CANFD_CNT_Type* canfd_cntr)
  \brief       Clears the CANFD timer counter
  \param[in]   canfd_cntr : Pointer to the CANFD counter map
  \return      None
*/
static inline void canfd_counter_clear(CANFD_CNT_Type* canfd_cntr)
{
    canfd_cntr->CANFD_CNTR_CTRL = CANFD_CNTR_CTRL_CNTR_CLEAR;
}

/**
  \fn          static inline CANFD_BUS_STATUS canfd_get_bus_status(CANFD_Type* canfd)
  \brief       Fetches the current bus status
  \param[in]   canfd : Pointer to the CANFD register map
  \return      bus status - CANFD_BUS_STATUS_ON/CANFD_BUS_STATUS_OFF
*/
static inline CANFD_BUS_STATUS canfd_get_bus_status(CANFD_Type* canfd)
{
    /* Returns current bus status*/
    if(canfd->CANFD_CFG_STAT & CANFD_CFG_STAT_BUSOFF)
    {
        return CANFD_BUS_STATUS_OFF;
    }
    else
    {
        return CANFD_BUS_STATUS_ON;
    }
}

/**
  \fn          static inline void canfd_select_tx_buf(CANFD_Type* canfd,
  \                                                   const uint8_t buf_type)
  \brief       Selects the requested buffer for next transmission
  \param[in]   canfd    : Pointer to the CANFD register map
  \param[in]   buf_type : Tx Buffer type
  \return      None
*/
static inline void canfd_select_tx_buf(CANFD_Type* canfd,
                                       const uint8_t buf_type)
{
    if(buf_type != CANFD_BUF_TYPE_PRIMARY)
    {
        /* Secondary buffer is selected for next msg tx */
        canfd->CANFD_TCMD |= CANFD_TCMD_TBSEL;
    }
    else
    {
        /* Primary buffer is selected for next msg tx */
        canfd->CANFD_TCMD &= ~CANFD_TCMD_TBSEL;
    }
}

/**
  \fn          static inline bool canfd_ptb_tx_active(CANFD_Type* canfd)
  \brief       Fetches the Primary Trasmit buffer Tx status
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      transmission status - Active/Inactive
*/
static inline bool canfd_ptb_tx_active(CANFD_Type* canfd)
{
    return ((canfd->CANFD_TCMD & CANFD_TCMD_TPE) != 0);
}

/**
  \fn          static inline bool canfd_stb_tx_active(CANFD_Type* canfd)
  \brief       Fetches the Secondary Trasmit buffer Tx status
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      transmission status - Active/Inactive
*/
static inline bool canfd_stb_tx_active(CANFD_Type* canfd)
{
    return (((canfd->CANFD_TCMD & CANFD_TCMD_TSONE)  != 0) ||
            ((canfd->CANFD_TCMD & CANFD_TCMD_TSALL) != 0));
}

/**
  \fn          static inline bool canfd_stb_free(CANFD_Type* canfd)
  \brief       Fetches the Secondary Trasmit buffer Free status
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      Buffer status - Free/Full
*/
static inline bool canfd_stb_free(CANFD_Type* canfd)
{
    return ((canfd->CANFD_TCTRL & CANFD_TCTRL_TSSTAT_Msk) !=
             CANFD_TCTRL_SEC_BUF_FULL);
}

/**
  \fn          static inline bool canfd_stb_empty(CANFD_Type* canfd)
  \brief       Fetches the Secondary Trasmit buffer empty status
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      Buffer status - Empty/Non_empty
*/
static inline bool canfd_stb_empty(CANFD_Type* canfd)
{
    return ((canfd->CANFD_TCTRL & CANFD_TCTRL_TSSTAT_Msk) == 0);
}

/**
  \fn          static inline void canfd_set_stb_mode(CANFD_Type* canfd,
  \                                                  const uint8_t mode)
  \brief       Sets Secondary Trasmit buffer mode
  \param[in]   canfd :  Pointer to the CANFD register map
  \param[in]   mode  :  Fifo/Priority mode
  \return      None
*/
static inline void canfd_set_stb_mode(CANFD_Type* canfd, const uint8_t mode)
{
    if(mode == CANFD_SECONDARY_BUF_MODE_PRIORITY)
    {
        canfd->CANFD_TCTRL |= CANFD_TCTRL_TSMODE;
    }
    else
    {
        canfd->CANFD_TCTRL &= (~CANFD_TCTRL_TSMODE);
    }
}

/**
  \fn          static inline bool canfd_comm_active(CANFD_Type* canfd)
  \brief       Fetches message communication status
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      Message comm status(Comm active/Inactive)
*/
static inline bool canfd_comm_active(CANFD_Type* canfd)
{
    return (canfd->CANFD_CFG_STAT & (CANFD_CFG_STAT_TACTIVE |
                                     CANFD_CFG_STAT_RACTIVE));
}

/**
  \fn          static inline uint8_t canfd_get_tx_error_count(CANFD_Type* canfd)
  \brief       Fetches the latest Transmission error count
  \param[in]   canfd : Pointer to the CANFD register map
  \return      Transmission error count
*/
static inline uint8_t canfd_get_tx_error_count(CANFD_Type* canfd)
{
    return canfd->CANFD_TECNT;
}

/**
  \fn          static inline uint8_t canfd_get_rx_error_count(CANFD_Type* canfd)
  \brief       Fetches the latest Reception error count
  \param[in]   canfd : Pointer to the CANFD register map
  \return      Reception error count
*/
static inline uint8_t canfd_get_rx_error_count(CANFD_Type* canfd)
{
    return canfd->CANFD_RECNT;
}

/**
  \fn          static inline void canfd_abort_tx(CANFD_Type* canfd,
  \                                              const uint8_t buf_type)
  \brief       Abort the current message transmission
  \param[in]   canfd    : Pointer to the CANFD register map
  \param[in]   buf_type : Tx Buffer type
  \return      none
*/
static inline void canfd_abort_tx(CANFD_Type* canfd, const uint8_t buf_type)
{
    if(buf_type != CANFD_BUF_TYPE_PRIMARY)
    {
        /* Aborts Secondary buffer transmission */
        canfd->CANFD_TCMD |= CANFD_TCMD_TSA;
    }
    else
    {
        /* Aborts Primary buffer transmission */
        canfd->CANFD_TCMD |= CANFD_TCMD_TPA;
    }
}

/**
  \fn          static inline bool canfd_error_passive_mode(CANFD_Type* canfd)
  \brief       Returns the passive mode status
  \param[in]   canfd   : Pointer to the CANFD register map
  \return      passive mode status
*/
static inline bool canfd_error_passive_mode(CANFD_Type* canfd)
{
    return ((canfd->CANFD_ERRINT & CANFD_ERRINT_EPASS) != 0);
}

/**
  \fn          static inline bool canfd_err_warn_limit_reached(CANFD_Type* canfd)
  \brief       Returns the passive mode status
  \param[in]   canfd   : Pointer to the CANFD register map
  \return      Warning limit reached status
*/
static inline bool canfd_err_warn_limit_reached(CANFD_Type* canfd)
{
    return ((canfd->CANFD_ERRINT & CANFD_ERRINT_EWARN) != 0);
}

/**
  \fn          static inline void canfd_set_rbuf_overflow_mode(CANFD_Type* canfd,
  \                                                            const uint8_t mode)
  \brief       Sets CANFD RBUF overflow mode.
  \param[in]   canfd : Pointer to the CANFD register map
  \param[in]   mode  : rx buffer overflow mode
  \return      none
*/
static inline void canfd_set_rbuf_overflow_mode(CANFD_Type* canfd,
                                                const uint8_t mode)
{
    if(mode == CANFD_RBUF_OVF_MODE_OVERWRITE_OLD_MSG)
    {
        /* Configures to overwrite old msg */
        canfd->CANFD_RCTRL &= (~CANFD_RCTRL_ROM);
    }
    else
    {
        /* Configures to discard new msg*/
        canfd->CANFD_RCTRL |= CANFD_RCTRL_ROM;
    }
}

/**
  \fn          static inline void canfd_set_rbuf_storage_format(CANFD_Type* canfd,
  \                                                             const uint8_t format)
  \brief       Sets CANFD RBUF storage format
  \param[in]   canfd   : Pointer to the CANFD register map
  \param[in]   format  : Rbuf Storage format
  \return      none
*/
static inline void canfd_set_rbuf_storage_format(CANFD_Type* canfd,
                                                 const uint8_t format)
{
    if(format == CANFD_RBUF_STORE_NORMAL_MSG)
    {
        /* Configures to store normal msgs */
        canfd->CANFD_RCTRL &= (~CANFD_RCTRL_RBALL);
    }
    else
    {
        /* Configures to store all msgs including error ones*/
        canfd->CANFD_RCTRL |= CANFD_RCTRL_RBALL;
    }
}

/**
  \fn          static inline void canfd_set_rbuf_almost_full_warn_limit(CANFD_Type* canfd,
  \                                                                     const uint8_t val)
  \brief       Sets Rbuf almost full warning limit (AFWL)
  \param[in]   canfd : Pointer to the CANFD register map
  \param[in]   val   : AFWL value
  \return      none
*/
static inline void canfd_set_rbuf_almost_full_warn_limit(CANFD_Type* canfd,
                                                         const uint8_t val)
{
    /* Clears and sets new AFWL value */
    canfd->CANFD_LIMIT &= ~CANFD_LIMIT_AFWL_Msk;
    canfd->CANFD_LIMIT |= CANFD_LIMIT_AFWL(val);
}

/**
  \fn          static inline bool canfd_rx_msg_available(CANFD_Type* canfd)
  \brief       Returns Rx msg availability status
  \param[in]   canfd   : Pointer to the CANFD register map
  \return      Rx msg availability status
*/
static inline bool canfd_rx_msg_available(CANFD_Type* canfd)
{
    return ((canfd->CANFD_RCTRL & CANFD_RCTRL_RSTAT_Msk)!= 0);
}

/**
  \fn          static inline void canfd_reset_acpt_fltr(CANFD_Type* canfd)
  \brief       Resets all acceptance filters
  \param[in]   canfd  : Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_reset_acpt_fltrs(CANFD_Type* canfd)
{
    /* Disable filter */
    canfd->CANFD_ACF_EN_0 &= ~CANFD_ACF_EN_0_AE_X_MAX_Msk;
}

/**
  \fn          static inline bool canfd_acpt_fltr_configured(CANFD_Type* canfd)
  \brief       Returns accpetance filters configured status
  \param[in]   canfd   : Pointer to the CANFD register map
  \return      filters configured status
*/
static inline bool canfd_acpt_fltr_configured(CANFD_Type* canfd)
{
   return ((canfd->CANFD_ACF_EN_0 & CANFD_ACF_EN_0_AE_X_MAX_Msk) != 0);
}

/**
  \fn          static inline void canfd_disable_acpt_fltr(CANFD_Type* canfd,
                                                          const uint8_t filter)
  \brief       Resets and disables the particular acceptance filter
  \param[in]   canfd  : Pointer to the CANFD register map
  \param[in]   filter : Acceptance filter number
  \return      none
*/
static inline void canfd_disable_acpt_fltr(CANFD_Type* canfd,
                                           const uint8_t filter)
{
    /* Disable filter */
    canfd->CANFD_ACF_EN_0 &= ~(1U << filter);
}

/**
  \fn          static inline void canfd_enable_tx_interrupts(CANFD_Type* canfd)
  \brief       Enables CANFD Tx interrupts
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_enable_tx_interrupts(CANFD_Type* canfd)
{
    /* Enables CANFD Tx interrupts */
    canfd->CANFD_RTIE     |= (CANFD_RTIE_TPIE      |
                              CANFD_RTIE_TSIE);
}

/**
  \fn          static inline void canfd_disable_tx_interrupts(CANFD_Type* canfd)
  \brief       Disables CANFD Tx interrupts
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_disable_tx_interrupts(CANFD_Type* canfd)
{
    /* Disables CANFD Tx interrupts */
    canfd->CANFD_RTIE     &= ~(CANFD_RTIE_TPIE     |
                               CANFD_RTIE_TSIE);
}

/**
  \fn          static inline void canfd_enable_rx_interrupts(CANFD_Type* canfd)
  \brief       Enables CANFD Rx interrupts
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_enable_rx_interrupts(CANFD_Type* canfd)
{
    /* Enables CANFD Rx interrupts */
    canfd->CANFD_RTIE     |= (CANFD_RTIE_RIE        |
                              CANFD_RTIE_ROIE       |
                              CANFD_RTIE_RFIE       |
                              CANFD_RTIE_RAFIE);
}

/**
  \fn          static inline void canfd_disable_rx_interrupts(CANFD_Type* canfd)
  \brief       Disables CANFD Rx interrupts
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_disable_rx_interrupts(CANFD_Type* canfd)
{
    /* Enables CANFD Rx interrupts */
    canfd->CANFD_RTIE     &= ~(CANFD_RTIE_RIE       |
                               CANFD_RTIE_ROIE      |
                               CANFD_RTIE_RFIE      |
                               CANFD_RTIE_RAFIE);
}


/**
  \fn          static inline void canfd_enable_error_interrupts(CANFD_Type* canfd)
  \brief       Enables CANFD error interrupts
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_enable_error_interrupts(CANFD_Type* canfd)
{
    /* Enables error interrupts */
    canfd->CANFD_RTIE     |= CANFD_RTIE_EIE;

    canfd->CANFD_ERRINT   |= (CANFD_ERRINT_EPIE     |
                              CANFD_ERRINT_ALIE     |
                              CANFD_ERRINT_BEIE);
}

/**
  \fn          static inline void canfd_disable_error_interrupts(CANFD_Type* canfd)
  \brief       Disables CANFD error interrupts
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_disable_error_interrupts(CANFD_Type* canfd)
{
    /* Enables error interrupts */
    canfd->CANFD_RTIE     &= ~CANFD_RTIE_EIE;

    canfd->CANFD_ERRINT   &= ~(CANFD_ERRINT_EPIE    |
                               CANFD_ERRINT_ALIE    |
                               CANFD_ERRINT_BEIE);
}

/**
  \fn          static inline void canfd_clear_interrupts(CANFD_Type* canfd)
  \brief       Clears all CANFD interrupts
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_clear_interrupts(CANFD_Type* canfd)
{
    /* Clears data and error interrupts */
    canfd->CANFD_RTIF   = 0U;
    canfd->CANFD_ERRINT &= ~CANFD_ERRINT_REG_Msk;
}

/**
  \fn          static inline void canfd_enable_standby_mode(CANFD_Type* canfd)
  \brief       Enables Standby Mode.
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_enable_standby_mode(CANFD_Type* canfd)
{
    /* Enables Transceiver Standby mode */
    canfd->CANFD_TCMD |= CANFD_TCMD_STBY;
}

/**
  \fn          static inline void canfd_disable_standby_mode(CANFD_Type* canfd)
  \brief       Disables Standby Mode.
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_disable_standby_mode(CANFD_Type* canfd)
{
    /* Disables Transceiver Standby mode */
    canfd->CANFD_TCMD &= (~CANFD_TCMD_STBY);
}

/**
  \fn          static inline void canfd_enable_normal_mode(CANFD_Type* canfd)
  \brief       Enables Normal Mode operation.
  \param[in]   canfd :  Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_enable_normal_mode(CANFD_Type* canfd)
{
    /* Disables the CANFD reset, Internal and External loopback*/
    canfd->CANFD_CFG_STAT &= ~(CANFD_CFG_STAT_RESET     |
                               CANFD_CFG_STAT_LBMI      |
                               CANFD_CFG_STAT_LBME);

    /* Disables Listen only mode */
    canfd->CANFD_TCMD     &= ~CANFD_TCMD_LOM;
}

/**
  \fn          static inline void canfd_enable_external_loop_back_mode(CANFD_Type* canfd)
  \brief       Enables External LoopBack Mode of CANFD instance.
  \param[in]   canfd  : Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_enable_external_loop_back_mode(CANFD_Type* canfd)
{
    /* Disables the CANFD reset, Internal and External loopback*/
    canfd->CANFD_CFG_STAT &= ~(CANFD_CFG_STAT_RESET     |
                               CANFD_CFG_STAT_LBMI);

    /* Disables Listen only mode */
    canfd->CANFD_TCMD     &= ~CANFD_TCMD_LOM;

    /* Enables external loopback with Self ACK */
    canfd->CANFD_CFG_STAT |= CANFD_CFG_STAT_LBME;
    canfd->CANFD_RCTRL    |= CANFD_RCTRL_SACK;
}

/**
  \fn          static inline void canfd_enable_internal_loop_back_mode(CANFD_Type* canfd)
  \brief       Enables Internal LoopBack Mode.
  \param[in]   canfd  :  Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_enable_internal_loop_back_mode(CANFD_Type* canfd)
{
    /* Disables the CANFD reset, Internal and External loopback*/
    canfd->CANFD_CFG_STAT  &= ~(CANFD_CFG_STAT_RESET    |
                                CANFD_CFG_STAT_LBME);

    /* Disables Listen only mode */
    canfd->CANFD_TCMD      &= ~CANFD_TCMD_LOM;

    /* Enables internal loopback with */
    canfd->CANFD_CFG_STAT  |= CANFD_CFG_STAT_LBMI;
}

/**
  \fn          static inline void canfd_enable_listen_only_mode(CANFD_Type* canfd)
  \brief       Enables Listen only Mode of CANFD instance.
  \param[in]   canfd  : Pointer to the CANFD register map
  \return      none
*/
static inline void canfd_enable_listen_only_mode(CANFD_Type* canfd)
{
    /* Disables the CANFD reset, Internal and External loopback*/
    canfd->CANFD_CFG_STAT &= ~(CANFD_CFG_STAT_RESET     |
                               CANFD_CFG_STAT_LBMI      |
                               CANFD_CFG_STAT_LBME);

    /* Disables CANFD Tx interrupts */
    canfd_disable_tx_interrupts(canfd);

    /* Enables Listen only mode */
    canfd->CANFD_TCMD     |= CANFD_TCMD_LOM;
}

/**
  \fn          void canfd_set_nominal_bit_time(CANFD_Type* canfd,
  \                                            const uint32_t bitrate_seg,
  \                                            const uint8_t prescaler)
  \brief       Sets the slow speed bit-timing of CANFD instance.
  \param[in]   canfd       : Pointer to the CANFD register map
  \param[in]   bitrate_seg : Segments - Propagation, Sampling
  \param[in]   prescaler   : Prescaler value
  \return      none
*/
void canfd_set_nominal_bit_time(CANFD_Type* canfd,
                                const uint32_t bitrate_seg,
                                const uint8_t prescaler);

/**
  \fn          void canfd_set_fd_bit_time(CANFD_Type* canfd,
  \                                       const uint32_t bitrate_seg,
  \                                       const uint8_t prescaler)
  \brief       Sets the fast speed bit-timing of CANFD instance.
  \param[in]   canfd       : Pointer to the CANFD register map
  \param[in]   bitrate_seg : Segments - Propagation, Sampling
  \param[in]   prescaler   : Prescaler value
  \return      none
*/
void canfd_set_fd_bit_time(CANFD_Type* canfd,
                           const uint32_t bitrate_seg,
                           const uint8_t prescaler);

/**
  \fn          void canfd_enable_acpt_fltr(CANFD_Type* canfd,
  \                                        canfd_acpt_fltr_t filter_config
  \brief       Configures and enables the particular acceptance filter.
  \param[in]   canfd          : Pointer to the CANFD register map
  \param[in]   filter_config  : Filter configuration
  \return      none
*/
void canfd_enable_acpt_fltr(CANFD_Type* canfd, canfd_acpt_fltr_t filter_config);

/**
  \fn          CANFD_ACPT_FLTR_STATUS canfd_get_acpt_fltr_status(CANFD_Type* canfd,
  \                                                              uint8_t filter)
  \brief       Retrieves whether the filter is free or occupied.
  \param[in]   canfd  : Pointer to the CANFD register map
  \param[in]   filter : Acceptance filter number
  \return      status of the filter (Free/Occupied)
*/
CANFD_ACPT_FLTR_STATUS canfd_get_acpt_fltr_status(CANFD_Type* canfd,
                                                  const uint8_t filter);

/**
  \fn          void canfd_get_acpt_fltr_data(CANFD_Type* canfd,
  \                                          canfd_acpt_fltr_t *filter_config)
  \brief       Retrieves the acceptance filter data.
  \param[in]   canfd          : Pointer to the CANFD register map
  \param[in]   filter_config  : Filter configuration
  \return      none
*/
void canfd_get_acpt_fltr_data(CANFD_Type* canfd,
                              canfd_acpt_fltr_t *filter_config);

/**
  \fn          void canfd_setup_tx_retrans(CANFD_Type* canfd,
  \                                        const uint8_t buf_type,
  \                                        const bool enable)
  \brief       Enables/Disables the Tx msg retransmission
  \param[in]   canfd    : Pointer to the CANFD register map
  \param[in]   buf_type : Bufer type
  \param[in]   enable   : Command to enable/disable msg retransmission
  \return      none
*/
void canfd_setup_tx_retrans(CANFD_Type* canfd,
                            const uint8_t buf_type,
                            const bool enable);

/**
  \fn          void canfd_setup_tx_delay_comp(CANFD_Type* canfd,
  \                                           const uint8_t offset,
  \                                           const bool enable)
  \brief       Enables/Disables the Tx delay compensation
  \param[in]   canfd  : Pointer to the CANFD register map
  \param[in]   offset : Secondary sampling point offest value
  \param[in]   enable : Command to enable/disable TDC
  \return      none
*/
void canfd_setup_tx_delay_comp(CANFD_Type* canfd,
                               const uint8_t offset,
                               const bool enable);

/**
  \fn          void canfd_set_err_warn_limit(CANFD_Type* canfd,
  \                                          const uint8_t ewl)
  \brief       Configures Warning limits for errors
  \param[in]   canfd : Pointer to the CANFD register map
  \param[in]   ewl   : Limit value for Error warning
  \return      none
*/
void canfd_set_err_warn_limit(CANFD_Type* canfd, const uint8_t ewl);

/**
  \fn          CANFD_MSG_ERROR canfd_get_last_error_code(CANFD_Type* canfd)
  \brief       Fetches the latest error occurred
  \param[in]   canfd : Pointer to the CANFD register map
  \return      last found error type
*/
CANFD_MSG_ERROR canfd_get_last_error_code(CANFD_Type* canfd);

/**
  \fn          void canfd_send(CANFD_Type* canfd,
  \                            const canfd_tx_info_t tx_header,
  \                            const uint8_t *data,
  \                            const uint8_t size)
  \brief       Prepares and transmits the message
  \param[in]   canfd      : Pointer to the CANFD register map
  \param[in]   tx_header  : Header of tx message
  \param[in]   data       : Message payload
  \param[in]   size       : payload size
  \return      none
*/
void canfd_send(CANFD_Type* canfd, const canfd_tx_info_t tx_header,
                const uint8_t *data, const uint8_t size);

/**
  \fn          void canfd_receive(CANFD_Type* canfd,
  \                               canfd_data_transfer_t *dest_data))
  \brief       Fetches the data from Rx buffer
  \param[in]   canfd         : Pointer to the CANFD register map
  \param[in]   dest_data     : Destination Data pointer
  \return      none
*/
void canfd_receive(CANFD_Type* canfd, canfd_transfer_t *dest_data);

/**
  \fn          void canfd_send_blocking(CANFD_Type* canfd,
  \                                     const canfd_tx_info_t tx_header,
  \                                     const uint8_t *data,
  \                                     const uint8_t size)
  \brief       Prepares and transmits the message in blocking mode
  \param[in]   canfd      : Pointer to the CANFD register map
  \param[in]   tx_header  : Header of tx message
  \param[in]   data       : Message payload
  \param[in]   size       : payload size
  \return      none
*/
void canfd_send_blocking(CANFD_Type* canfd, const canfd_tx_info_t tx_header,
                         const uint8_t *data, const uint8_t size);

/**
  \fn          void canfd_receive_blocking(CANFD_Type* canfd,
  \                                        canfd_data_transfer_t *dest_data))
  \brief       Fetches the data from Rx buffer in blocking mode
  \param[in]   canfd      : Pointer to the CANFD register map
  \param[in]   dest_data  : Destination Data pointer
  \return      none
*/
void canfd_receive_blocking(CANFD_Type* canfd, canfd_transfer_t *dest_data);

/**
  \fn          void canfd_clear_interrupt(CANFD_Type* canfd, const uint32_t event)
  \brief       Clears the interrupt
  \param[in]   canfd : Pointer to the CANFD register map
  \param[in]   event : Interrupt event
  \return      none
*/
void canfd_clear_interrupt(CANFD_Type* canfd, const uint32_t event);

/**
  \fn          uint32_t canfd_irq_handler(CANFD_Type* canfd)
  \brief       Returns the interrupt event
  \param[in]   canfd  : Pointer to the CANFD register map
  \return      CANFD interrupt event
*/
uint32_t canfd_irq_handler(CANFD_Type* canfd);

#endif /* CANFD_H_ */
