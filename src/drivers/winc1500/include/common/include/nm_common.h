/**
 *
 * \file
 *
 * \brief WINC Driver Common API Declarations.
 *
 * Copyright (c) 2016-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

/** @defgroup COMMON Common
@{
    @defgroup COMMONDEF Defines
    @defgroup COMMONAPI Functions
@}
 */

#ifndef _NM_COMMON_H_
#define _NM_COMMON_H_

#include "bsp/include/nm_bsp.h"
#include "common/include/nm_debug.h"
#define WINC1500_EXPORT(name) winc1500_ ## name
/**@addtogroup COMMONDEF
 */
/**@{*/
#define M2M_TIME_OUT_DELAY 10000

/*states*/
#define M2M_SUCCESS         ((sint8)0)
#define M2M_ERR_SEND		((sint8)-1)
#define M2M_ERR_RCV			((sint8)-2)
#define M2M_ERR_MEM_ALLOC	((sint8)-3)
#define M2M_ERR_TIME_OUT	((sint8)-4)
#define M2M_ERR_INIT        ((sint8)-5)
#define M2M_ERR_BUS_FAIL    ((sint8)-6)
#define M2M_NOT_YET			((sint8)-7)
#define M2M_ERR_FIRMWARE	((sint8)-8)
#define M2M_SPI_FAIL		((sint8)-9)
#define M2M_ERR_FIRMWARE_bURN	 ((sint8)-10)
#define M2M_ACK				((sint8)-11)
#define M2M_ERR_FAIL		((sint8)-12)
#define M2M_ERR_FW_VER_MISMATCH         ((sint8)-13)
#define M2M_ERR_SCAN_IN_PROGRESS         ((sint8)-14)
#define M2M_ERR_INVALID_ARG				 ((sint8)-15)
#define M2M_ERR_INVALID					((sint8)-16)

/*i2c MAASTER ERR*/
#define I2C_ERR_LARGE_ADDRESS       0xE1UL
/*!< The address exceeds the max addressing mode in I2C flash.
 */
#define I2C_ERR_TX_ABRT             0xE2UL
/*!< NO ACK from slave.
 */
#define I2C_ERR_OVER_SIZE           0xE3UL
#define ERR_PREFIX_NMIS             0xE4UL
/*!< Wrong first four bytes in flash NMIS.
 */
#define ERR_FIRMEWARE_EXCEED_SIZE   0xE5UL
/*!< Total size of firmware exceeds the max size 256k.
 */
/**/
#define PROGRAM_START		0x26961735UL
#define BOOT_SUCCESS		0x10add09eUL
#define BOOT_START		    0x12345678UL

#define NBIT31				(0x80000000)
#define NBIT30				(0x40000000)
#define NBIT29				(0x20000000)
#define NBIT28				(0x10000000)
#define NBIT27				(0x08000000)
#define NBIT26				(0x04000000)
#define NBIT25				(0x02000000)
#define NBIT24				(0x01000000)
#define NBIT23				(0x00800000)
#define NBIT22				(0x00400000)
#define NBIT21				(0x00200000)
#define NBIT20				(0x00100000)
#define NBIT19				(0x00080000)
#define NBIT18				(0x00040000)
#define NBIT17				(0x00020000)
#define NBIT16				(0x00010000)
#define NBIT15				(0x00008000)
#define NBIT14				(0x00004000)
#define NBIT13				(0x00002000)
#define NBIT12				(0x00001000)
#define NBIT11				(0x00000800)
#define NBIT10				(0x00000400)
#define NBIT9				(0x00000200)
#define NBIT8				(0x00000100)
#define NBIT7				(0x00000080)
#define NBIT6				(0x00000040)
#define NBIT5				(0x00000020)
#define NBIT4				(0x00000010)
#define NBIT3				(0x00000008)
#define NBIT2				(0x00000004)
#define NBIT1				(0x00000002)
#define NBIT0				(0x00000001)

#define M2M_MAX(A,B)					((A) > (B) ? (A) : (B))
#define M2M_SEL(x,m1,m2,m3)				((x>1)?((x>2)?(m3):(m2)):(m1))
#define WORD_ALIGN(val) 				(((val) & 0x03) ? ((val) + 4 - ((val) & 0x03)) : (val))

#define DATA_PKT_OFFSET	4

#if _BYTE_ORDER == _LITTLE_ENDIAN
#define BYTE_0(word)   					((uint8)(((word) >> 0 	) & 0x000000FFUL))
#define BYTE_1(word)  	 				((uint8)(((word) >> 8 	) & 0x000000FFUL))
#define BYTE_2(word)   					((uint8)(((word) >> 16) & 0x000000FFUL))
#define BYTE_3(word)   					((uint8)(((word) >> 24) & 0x000000FFUL))
#else
#define BYTE_0(word)   					((uint8)(((word) >> 24) & 0x000000FFUL))
#define BYTE_1(word)  	 				((uint8)(((word) >> 16) & 0x000000FFUL))
#define BYTE_2(word)   					((uint8)(((word) >> 8 	) & 0x000000FFUL))
#define BYTE_3(word)   					((uint8)(((word) >> 0 	) & 0x000000FFUL))
#endif

/**@}*/     //COMMONDEF
#ifdef __cplusplus
     extern "C" {
 #endif

/*!
 * @ingroup     COMMONAPI
 * @fn          void m2m_memcpy(uint8* pDst, uint8* pSrc, uint32 sz);
 * @brief       Copy specified number of bytes from source buffer to destination buffer.
 * @param [in]  sz
 *                  Number of data bytes to copy.
 * @param [in]  pSrc
 *                  Source buffer.
 * @param [out] pDst
 *                  Destination buffer.
 * @return      None
 */
NMI_API void m2m_memcpy(uint8* pDst,uint8* pSrc,uint32 sz);

/*!
 * @ingroup     COMMONAPI
 * @fn          void m2m_memset(uint8* pBuf, uint8 val, uint32 sz);
 * @brief       Set specified number of data bytes in specified data buffer to specified value.
 * @param [in]  sz
 *                  Number of data bytes (in specified data buffer whose values are to be set to the specified value).
 * @param [in]  val
 *                  The specified value (to which data bytes in data buffer will be set).
 * @param [out] pBuf
 *                  The specified data buffer (whose data bytes will be set to the specified value).
 * @return      None
 */
NMI_API void m2m_memset(uint8* pBuf,uint8 val,uint32 sz);

/*!
 * @ingroup     COMMONAPI
 * @fn          uint16 m2m_strlen(uint8 * pcStr);
 * @brief       Returns the length of a null terminated string buffer.
 * @param [in]  pcStr
 *                  Null terminated string buffer.
 * @return      Length of the string in the specified string buffer.
 */
NMI_API uint16 m2m_strlen(uint8 * pcStr);

/*!
 * @ingroup     COMMONAPI
 * @fn          sint8 m2m_memcmp(uint8 *pu8Buff1, uint8 *pu8Buff2, uint32 u32Size);
 * @brief       Compare specified number of data bytes in pu8Buff1 and pu8Buff2 and decide if they all match.
 * @param [in]  u32Size
 *                  Number of data bytes to compare.
 * @param [in]  pu8Buff1
 *                  One of two data buffers for the comparison.
 * @param [in]  pu8Buff2
 *                  One of two data buffers for the comparison.
 * @return      Zero if matched, one if not matched.
 */
NMI_API sint8 m2m_memcmp(uint8 *pu8Buff1,uint8 *pu8Buff2 ,uint32 u32Size);

/*!
 * @ingroup     COMMONAPI
 * @fn          uint8 m2m_strncmp(uint8 *pcS1, uint8 *pcS2, uint16 u16Len);
 * @brief       Compare specified number of data bytes in string buffers pcS1 and pcS2.
 * @param [in]  u16Len
 *                  Number of data bytes to compare.
 * @param [in]  pcS1
 *                  First of two string buffers for the comparison.
 * @param [in]  pcS2
 *                  Second of two string buffers for the comparison.
 * @return      0 if matched, -1 if the first non-matching byte in pcS1 is smaller than that in pcS2, +1 if it is bigger.
 */
NMI_API uint8 m2m_strncmp(uint8 *pcS1, uint8 *pcS2, uint16 u16Len);

/*!
 * @ingroup     COMMONAPI
 * @fn          uint8 * m2m_strstr(uint8 *pcIn, uint8 *pcStr);
 * @brief       Find the occurrence of pcStr string in pcIn string.
 * @param [in]  pcStr
 *                  One of two string buffers.
 * @param [in]  pcIn
 *                  One of two string buffers.
 * @return      If pcStr string is part of pcIn string return a valid pointer to the start of pcStr within pcIn. If not, a NULL Pointer is returned
 */
NMI_API uint8 * m2m_strstr(uint8 *pcIn, uint8 *pcStr);

/*!
 * @ingroup     COMMONAPI
 * @fn          uint8 m2m_checksum(uint8* buf, int sz);
 * @brief       Calculates checksum for the specified number of data bytes in specified data buffer.
 * @param [in]  sz
 *                  Number of data bytes used in the checksum calculation.
 * @param [in]  buf
 *                  The specified data buffer (whose data bytes will be used to calculate the checksum).
 * @return      The calculated checksum.
 */
NMI_API uint8 m2m_checksum(uint8* buf, int sz);

/*!
 * @ingroup     COMMONAPI
 * @fn           void (*at_sb_printf)(const char *_format, ...);
 * @brief       Chooses which function to use in order to output debug.
 */
extern NMI_API void (*at_sb_printf)(const char *_format, ...);
#ifdef __cplusplus
}
 #endif
#endif	/*_NM_COMMON_H_*/
