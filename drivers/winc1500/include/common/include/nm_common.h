/*******************************************************************************
  File Name:
    nm_common.h

  Summary:
    This module contains common APIs declarations.

  Description:
    This module contains common APIs declarations.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2022, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/
//DOM-IGNORE-END

/** @defgroup COMMON Common
    @{
        @defgroup COMMONDEF Defines
        @defgroup COMMONAPI Functions
    @}
 */

#ifndef _NM_COMMON_H_
#define _NM_COMMON_H_

#include "nm_bsp.h"
#include "nm_debug.h"
#include "osal/osal.h"

// Prefix the BSD socket API so it can't collide with the MicroPython/POSIX one.
#define WINC1500_EXPORT(name) winc1500_ ## name

/**@addtogroup COMMONDEF
 * @{
 */

/*states*/
#define M2M_SUCCESS                 ((int8_t)0)
#define M2M_ERR_SEND                ((int8_t)-1)
#define M2M_ERR_RCV                 ((int8_t)-2)
#define M2M_ERR_MEM_ALLOC           ((int8_t)-3)
#define M2M_ERR_TIME_OUT            ((int8_t)-4)
#define M2M_ERR_INIT                ((int8_t)-5)
#define M2M_ERR_BUS_FAIL            ((int8_t)-6)
#define M2M_NOT_YET                 ((int8_t)-7)
#define M2M_ERR_FIRMWARE            ((int8_t)-8)
#define M2M_SPI_FAIL                ((int8_t)-9)
#define M2M_ERR_FIRMWARE_bURN       ((int8_t)-10)
#define M2M_ACK                     ((int8_t)-11)
#define M2M_ERR_FAIL                ((int8_t)-12)
#define M2M_ERR_FW_VER_MISMATCH     ((int8_t)-13)
#define M2M_ERR_SCAN_IN_PROGRESS    ((int8_t)-14)

/* Invalid argument */
#define M2M_ERR_INVALID_ARG         ((int8_t)-15)
#define M2M_ERR_INVALID             ((int8_t)-16)

#define NBIT31                      (0x80000000)
#define NBIT30                      (0x40000000)
#define NBIT29                      (0x20000000)
#define NBIT28                      (0x10000000)
#define NBIT27                      (0x08000000)
#define NBIT26                      (0x04000000)
#define NBIT25                      (0x02000000)
#define NBIT24                      (0x01000000)
#define NBIT23                      (0x00800000)
#define NBIT22                      (0x00400000)
#define NBIT21                      (0x00200000)
#define NBIT20                      (0x00100000)
#define NBIT19                      (0x00080000)
#define NBIT18                      (0x00040000)
#define NBIT17                      (0x00020000)
#define NBIT16                      (0x00010000)
#define NBIT15                      (0x00008000)
#define NBIT14                      (0x00004000)
#define NBIT13                      (0x00002000)
#define NBIT12                      (0x00001000)
#define NBIT11                      (0x00000800)
#define NBIT10                      (0x00000400)
#define NBIT9                       (0x00000200)
#define NBIT8                       (0x00000100)
#define NBIT7                       (0x00000080)
#define NBIT6                       (0x00000040)
#define NBIT5                       (0x00000020)
#define NBIT4                       (0x00000010)
#define NBIT3                       (0x00000008)
#define NBIT2                       (0x00000004)
#define NBIT1                       (0x00000002)
#define NBIT0                       (0x00000001)

#ifndef BIG_ENDIAN
/*! Most significant byte of 32bit word (LE) */
#define BYTE_0(word)                    ((uint8_t)(((word) >> 0 ) & 0x000000FFUL))
/*! Second most significant byte of 32bit word (LE) */
#define BYTE_1(word)                    ((uint8_t)(((word) >> 8 ) & 0x000000FFUL))
/*! Third most significant byte of 32bit word (LE) */
#define BYTE_2(word)                    ((uint8_t)(((word) >> 16) & 0x000000FFUL))
/*! Least significant byte of 32bit word (LE) */
#define BYTE_3(word)                    ((uint8_t)(((word) >> 24) & 0x000000FFUL))
#else
/*! Most significant byte of 32bit word (BE) */
#define BYTE_0(word)                    ((uint8_t)(((word) >> 24) & 0x000000FFUL))
/*! Second most significant byte of 32bit word (BE) */
#define BYTE_1(word)                    ((uint8_t)(((word) >> 16) & 0x000000FFUL))
/*! Third most significant byte of 32bit word (BE) */
#define BYTE_2(word)                    ((uint8_t)(((word) >> 8 ) & 0x000000FFUL))
/*! Least significant byte of 32bit word (BE) */
#define BYTE_3(word)                    ((uint8_t)(((word) >> 0 ) & 0x000000FFUL))
#endif

#ifndef LOW
#define LOW 0
#endif
#ifndef HIGH
#define HIGH 1
#endif

/**@}*/

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @fn         void nm_sleep(uint32_t);
 *  @brief
 *  @param[in]  u32TimeMsec
 *              Time unit in milliseconds
 *  @pre
 *  @warning    Maximum value must nor exceed 4294967295 milliseconds which is equal to 4294967.295 seconds.\n
 *  @note       Implementation of this function is host dependent.
 *  @see
 *  @return     None
 */
void nm_sleep(uint32_t u32TimeMsec);

/*!
 *  @fn     nm_reset
 *  @brief  Reset NMC1500 SoC by setting CHIP_EN and RESET_N signals low,
 *           CHIP_EN high then RESET_N high
 */
void nm_reset(void);

#ifdef __cplusplus
}
#endif
#endif  /*_NM_COMMON_H_*/
