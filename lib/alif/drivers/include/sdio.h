/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     sdio.h
 * @author   Deepak Kumar
 * @email    deepak@alifsemi.com
 * @version  V0.0.1
 * @date     28-Nov-2022
 * @brief    exposed SDIO Driver variables and APIs.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef _SDIO_H_
#define _SDIO_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SDIO commands                         type  argument     response */
#define SDIO_SEND_OP_COND          5U /* bcr  [23:0] OCR         R4  */
#define SDIO_RW_DIRECT            52U /* ac   [31:0] See below   R5  */
#define SDIO_RW_EXTENDED          53U /* adtc [31:0] See below   R5  */

/*
 * SD_IO_RW_DIRECT argument format:
 *
 *      [31] R/W flag
 *      [30:28] Function number
 *      [27] RAW flag
 *      [25:9] Register address
 *      [7:0] Data
 */
#define SDIO_RW_FLAG_Pos   (31U)
#define SDIO_RW_FLAG_Msk   (1U << SDIO_RW_FLAG_Pos)
#define SDIO_FN_Pos        (28U)
#define SDIO_RAW_FLAG_Pos  (27U)
#define SDIO_RAW_FLAG_Msk  (1U << SDIO_RAW_FLAG_Pos)
#define SDIO_REG_ADDR_Pos  (9U)

/*
 * SD_IO_RW_EXTENDED argument format:
 *
 *      [31] R/W flag
 *      [30:28] Function number
 *      [27] Block mode
 *      [26] Increment address
 *      [25:9] Register address
 *      [8:0] Byte/block count
 */
#define SDIO_RW_EXT_BLK_MODE_Pos   (27U)
#define SDIO_RW_EXT_BLK_MODE_Msk   (1U << SDIO_RW_EXT_BLK_MODE_Pos)
#define SDIO_RW_EXT_INCR_ADDR_Pos  (26U)
#define SDIO_RW_EXT_INCR_ADDR_Msk  (1U << SDIO_RW_EXT_INCR_ADDR_Pos)

#define CMD5_RESP_IO_READY_Msk     (1U << 31U)
#define CMD5_RESP_18V_PRES_Msk     (1U << 24U)
#define CMD5_RESP_MEMORY_PRES_Msk  (1U << 27U)
#define CMD5_RESP_NIOF_PRES_Pos    28U
#define CMD5_RESP_NIOF_PRES_Msk    (7U << CMD5_RESP_NIOF_PRES_Pos)
#define CMD5_RESP_OCR_Msk          0x00FFFFFFU
#define CMD5_RESP_OCR_3V3_Msk      (1 << 21U)

/*
   SDIO status in R5
   Type
    e : error bit
    s : status bit
    r : detected and set for the actual command response
    x : detected and set during command execution. the host must poll
    the card by sending status command in order to read these bits.
    Clear condition
    a : according to the card state
    b : always related to the previous command. Reception of
    a valid command will clear it (with a delay of one command)
    c : clear by read
*/

#define SDIO_R5_COM_CRC_ERROR        (1U << 15U)    /* er, b   */
#define SDIO_R5_ILLEGAL_COMMAND      (1U << 14U)    /* er, b   */
#define SDIO_R5_ERROR                (1U << 11U)    /* erx, c  */
#define SDIO_R5_FUNCTION_NUMBER      (1U << 9U)     /* er, c   */
#define SDIO_R5_OUT_OF_RANGE         (1U << 8U)     /* er, c   */
#define SDIO_R5_STATUS(x)            (x & 0xCB00U)
#define SDIO_R5_IO_CURRENT_STATE(x)  ((x & 0x3000U) >> 12U) /* s, b */
#define SDIO_MAX_CIA_ADDR       (0x1FFFFU)
#define SDIO_MAX_FUNCTION       (7U)

/*
 * Card Common Control Registers (CCCR)
 */

#define SDIO_CCCR_CCCR          0x00U

#define SDIO_CCCR_REV_1_00      0U    /* CCCR/FBR Version 1.00 */
#define SDIO_CCCR_REV_1_10      1U    /* CCCR/FBR Version 1.10 */
#define SDIO_CCCR_REV_1_20      2U    /* CCCR/FBR Version 1.20 */
#define SDIO_CCCR_REV_3_00      3U    /* CCCR/FBR Version 3.00 */

#define SDIO_SDIO_REV_1_00      0U    /* SDIO Spec Version 1.00 */
#define SDIO_SDIO_REV_1_10      1U    /* SDIO Spec Version 1.10 */
#define SDIO_SDIO_REV_1_20      2U    /* SDIO Spec Version 1.20 */
#define SDIO_SDIO_REV_2_00      3U    /* SDIO Spec Version 2.00 */
#define SDIO_SDIO_REV_3_00      4U    /* SDIO Spec Version 3.00 */

#define SDIO_CCCR_SD            0x01U

#define SDIO_SD_REV_1_01        0U    /* SD Physical Spec Version 1.01 */
#define SDIO_SD_REV_1_10        1U    /* SD Physical Spec Version 1.10 */
#define SDIO_SD_REV_2_00        2U    /* SD Physical Spec Version 2.00 */
#define SDIO_SD_REV_3_00        3U    /* SD Physical Spec Version 3.00 */

#define SDIO_CCCR_IOEx          0x02U
#define SDIO_CCCR_IORx          0x03U

#define SDIO_CCCR_IENx          0x04U    /* Function/Master Interrupt Enable */
#define SDIO_CCCR_INTx          0x05U    /* Function Interrupt Pending */

#define SDIO_CCCR_ABORT         0x06U    /* function abort/card reset */

#define SDIO_CCCR_IF            0x07U    /* bus interface controls */

#define SDIO_BUS_WIDTH_Msk      0x03U    /* data bus width setting */
#define SDIO_BUS_WIDTH_1BIT     0x00U
#define SDIO_BUS_WIDTH_RESERVED 0x01U
#define SDIO_BUS_WIDTH_4BIT     0x02U
#define SDIO_BUS_ECSI           0x20U    /* Enable continuous SPI interrupt */
#define SDIO_BUS_SCSI           0x40U    /* Support continuous SPI interrupt */

#define SDIO_BUS_ASYNC_INT      0x20U

#define SDIO_BUS_CD_DISABLE     0x80U    /* disable pull-up on DAT3 (pin 1) */

#define SDIO_CCCR_CAPS          0x08U

#define SDIO_CCCR_CAP_SDC       0x01U    /* can do CMD52 while data transfer */
#define SDIO_CCCR_CAP_SMB       0x02U    /* can do multi-block xfers (CMD53) */
#define SDIO_CCCR_CAP_SRW       0x04U    /* supports read-wait protocol */
#define SDIO_CCCR_CAP_SBS       0x08U    /* supports suspend/resume */
#define SDIO_CCCR_CAP_S4MI      0x10U    /* interrupt during 4-bit CMD53 */
#define SDIO_CCCR_CAP_E4MI      0x20U    /* enable ints during 4-bit CMD53 */
#define SDIO_CCCR_CAP_LSC       0x40U    /* low speed card */
#define SDIO_CCCR_CAP_4BLS      0x80U    /* 4 bit low speed card */

#define SDIO_CCCR_CIS           0x09U    /* common CIS pointer (3 bytes) */

/* Following 4 regs are valid only if SBS is set */
#define SDIO_CCCR_SUSPEND           0x0CU
#define SDIO_CCCR_SELx              0x0DU
#define SDIO_CCCR_EXECx             0x0EU
#define SDIO_CCCR_READYx            0x0FU

#define SDIO_CCCR_BLKSIZE           0x10U

#define SDIO_CCCR_POWER             0x12U

#define  SDIO_POWER_SMPC            0x01U    /* Supports Master Power Control */
#define  SDIO_POWER_EMPC            0x02U    /* Enable Master Power Control */

#define SDIO_CCCR_SPEED             0x13U

#define  SDIO_SPEED_SHS             0x01U    /* Supports High-Speed mode */
#define  SDIO_SPEED_BSS_SHIFT       1U
#define  SDIO_SPEED_BSS_Msk         (7U << SDIO_SPEED_BSS_SHIFT)
#define  SDIO_SPEED_SDR12           (0U << SDIO_SPEED_BSS_SHIFT)
#define  SDIO_SPEED_SDR25           (1U << SDIO_SPEED_BSS_SHIFT)
#define  SDIO_SPEED_SDR50           (2U << SDIO_SPEED_BSS_SHIFT)
#define  SDIO_SPEED_SDR104          (3U << SDIO_SPEED_BSS_SHIFT)
#define  SDIO_SPEED_DDR50           (4U << SDIO_SPEED_BSS_SHIFT)
#define  SDIO_SPEED_EHS             SDIO_SPEED_SDR25    /* Enable High-Speed */

#define SDIO_CCCR_UHS               0x14U
#define  SDIO_UHS_SDR50             0x01U
#define  SDIO_UHS_SDR104            0x02U
#define  SDIO_UHS_DDR50             0x04U

#define SDIO_CCCR_DRIVE_STRENGTH    0x15U
#define SDIO_SDTx_Msk               0x07U
#define SDIO_DRIVE_SDTA             (1U << 0U)
#define SDIO_DRIVE_SDTC             (1U << 1U)
#define SDIO_DRIVE_SDTD             (1U << 2U)
#define SDIO_DRIVE_DTSx_Msk         0x03U
#define SDIO_DRIVE_DTSx_SHIFT       4U
#define SDIO_DTSx_SET_TYPE_B        (0U << SDIO_DRIVE_DTSx_SHIFT)
#define SDIO_DTSx_SET_TYPE_A        (1U << SDIO_DRIVE_DTSx_SHIFT)
#define SDIO_DTSx_SET_TYPE_C        (2U << SDIO_DRIVE_DTSx_SHIFT)
#define SDIO_DTSx_SET_TYPE_D        (3U << SDIO_DRIVE_DTSx_SHIFT)

#define SDIO_CCCR_INTERRUPT_EXT     0x16U
#define SDIO_INTERRUPT_EXT_SAI      (1U << 0U)
#define SDIO_INTERRUPT_EXT_EAI      (1U << 1U)

/*
 * Function Basic Registers (FBR)
 */

#define SDIO_FBR_BASE(f)            ((f) * 0x100U) /* base of function f's FBRs */
#define SDIO_FBR_STD_IF             0x00U
#define SDIO_FBR_SUPPORTS_CSA       0x40U    /* supports Code Storage Area */
#define SDIO_FBR_ENABLE_CSA         0x80U    /* enable Code Storage Area */
#define SDIO_FBR_STD_IF_EXT         0x01U
#define SDIO_FBR_POWER              0x02U
#define SDIO_FBR_POWER_SPS          0x01U    /* Supports Power Selection */
#define SDIO_FBR_POWER_EPS          0x02U    /* Enable (low) Power Selection */
#define SDIO_FBR_CIS                0x09U    /* CIS pointer (3 bytes) */
#define SDIO_FBR_CSA                0x0CU    /* CSA pointer (3 bytes) */
#define SDIO_FBR_CSA_DATA           0x0FU
#define SDIO_FBR_BLKSIZE            0x10U    /* block size (2 bytes) */

typedef struct _sdio_opcond_t{
    uint8_t isInitialized;  /*!< indicates Card is initialized              */
    uint8_t func_number;    /*!< Number of function available in the card   */
    uint8_t memory_present; /*!< Memory Present Flag                        */
    uint8_t s18a;           /*!< low voltage support flag                   */
    uint8_t ocr;            /*!< operating voltage                          */
}sdio_opcond_t;

typedef struct _sdio_t{
    sdio_opcond_t sdio_opcd; /*!< sdio card information */
}sdio_t;

#ifdef __cplusplus
}
#endif

#endif
