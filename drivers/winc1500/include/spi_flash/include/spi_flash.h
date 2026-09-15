/*******************************************************************************
  WINC1500 SPI Flash Interface

  File Name:
    spi_flash.h

  Summary:
    WINC1500 SPI Flash Interface

  Description:
    WINC1500 SPI Flash Interface
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

/** \defgroup SPIFLASH Spi Flash
 * @file           spi_flash.h
 * @brief          This file describe SPI flash APIs, how to use it and limitations with each one.
 * @section      Example
 *                 This example illustrates a complete guide of how to use these APIs.
 * @code{.c}
                    #include "spi_flash.h"

                    #define DATA_TO_REPLACE "THIS IS A NEW SECTOR IN FLASH"

                    int main()
                    {
                        uint8_t au8FlashContent[FLASH_SECTOR_SZ] = {0};
                        uint32_t    u32FlashTotalSize = 0;
                        uint32_t    u32FlashOffset = 0;

                        ret = m2m_wifi_download_mode();
                        if(M2M_SUCCESS != ret)
                        {
                            printf("Unable to enter download mode\r\n");
                        }
                        else
                        {
                            u32FlashTotalSize = spi_flash_get_size();
                        }

                        while((u32FlashTotalSize > u32FlashOffset) && (M2M_SUCCESS == ret))
                        {
                            ret = spi_flash_read(au8FlashContent, u32FlashOffset, FLASH_SECTOR_SZ);
                            if(M2M_SUCCESS != ret)
                            {
                                printf("Unable to read SPI sector\r\n");
                                break;
                            }
                            memcpy(au8FlashContent, DATA_TO_REPLACE, strlen(DATA_TO_REPLACE));

                            ret = spi_flash_erase(u32FlashOffset, FLASH_SECTOR_SZ);
                            if(M2M_SUCCESS != ret)
                            {
                                printf("Unable to erase SPI sector\r\n");
                                break;
                            }

                            ret = spi_flash_write(au8FlashContent, u32FlashOffset, FLASH_SECTOR_SZ);
                            if(M2M_SUCCESS != ret)
                            {
                                printf("Unable to write SPI sector\r\n");
                                break;
                            }
                            u32FlashOffset += FLASH_SECTOR_SZ;
                        }

                        if(M2M_SUCCESS == ret)
                        {
                            printf("Successful operations\r\n");
                        }
                        else
                        {
                            printf("Failed operations\r\n");
                        }

                        while(1);
                        return M2M_SUCCESS;
                    }
 * @endcode
 */

#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__
#include "nm_common.h"
#include "nmbus.h"
#include "nmasic.h"

#define FLASH_SECTOR_SZ						(4 * 1024UL)
/*!<Sector Size in Flash Memory
 */

/**
 *  @fn     spi_flash_enable
 *  @brief  Enable spi flash operations
 *  @version    1.0
 */
int8_t spi_flash_enable(uint8_t enable);
/** \defgroup SPIFLASHAPI Function
 *   @ingroup SPIFLASH
 */

 /** @defgroup SPiFlashGetFn spi_flash_get_size
 *  @ingroup SPIFLASHAPI
 */
  /**@{*/
/*!
 * @fn             uint32_t spi_flash_get_size(void);
 * @brief         Returns with \ref uint32_t value which is total flash size\n
 * @note         Returned value in Mb (Mega Bit).
 * @return      SPI flash size in case of success and a ZERO value in case of failure.
 */
uint32_t spi_flash_get_size(void);
 /**@}*/

  /** @defgroup SPiFlashRead spi_flash_read
 *  @ingroup SPIFLASHAPI
 */
  /**@{*/
/*!
 * @fn             int8_t spi_flash_read(uint8_t *, uint32_t, uint32_t);
 * @brief          Read a specified portion of data from SPI Flash.\n
 * @param [out]    pu8Buf
 *                 Pointer to data buffer which will fill in with data in case of successful operation.
 * @param [in]     u32Addr
 *                 Address (Offset) to read from at the SPI flash.
 * @param [in]     u32Sz
 *                 Total size of data to be read in bytes
 * @warning
 *                 - Address (offset) plus size of data must not exceed flash size.\n
 *                 - No firmware is required for reading from SPI flash.\n
 *                 - In case of there is a running firmware, it is required to pause your firmware first
 *                   before any trial to access SPI flash to avoid any racing between host and running firmware on bus using
 *                   @ref m2m_wifi_download_mode
 * @note
 *                 - It is blocking function\n
 * @sa             m2m_wifi_download_mode, spi_flash_get_size
 * @return        The function returns @ref M2M_SUCCESS for successful operations  and a negative value otherwise.
 */
int8_t spi_flash_read(uint8_t *pu8Buf, uint32_t u32Addr, uint32_t u32Sz);
 /**@}*/

  /** @defgroup SPiFlashWrite spi_flash_write
 *  @ingroup SPIFLASHAPI
 */
  /**@{*/
/*!
 * @fn             int8_t spi_flash_write(uint8_t *, uint32_t, uint32_t);
 * @brief          Write a specified portion of data to SPI Flash.\n
 * @param [in]     pu8Buf
 *                 Pointer to data buffer which contains the required to be written.
 * @param [in]     u32Offset
 *                 Address (Offset) to write at the SPI flash.
 * @param [in]     u32Sz
 *                 Total number of size of data bytes
 * @note
 *                 - It is blocking function\n
 *                 - It is user's responsibility to verify that data has been written successfully
 *                   by reading data again and compare it with the original.
 * @warning
 *                 - Address (offset) plus size of data must not exceed flash size.\n
 *                 - No firmware is required for writing to SPI flash.\n
 *                 - In case of there is a running firmware, it is required to pause your firmware first
 *                   before any trial to access SPI flash to avoid any racing between host and running firmware on bus using
 *                   @ref m2m_wifi_download_mode.
 *                 - Before writing to any section, it is required to erase it first.
 * @sa             m2m_wifi_download_mode, spi_flash_get_size, spi_flash_erase
 * @return       The function returns @ref M2M_SUCCESS for successful operations  and a negative value otherwise.

 */
int8_t spi_flash_write(uint8_t* pu8Buf, uint32_t u32Offset, uint32_t u32Sz);
 /**@}*/

  /** @defgroup SPiFlashErase spi_flash_erase
 *  @ingroup SPIFLASHAPI
 */
  /**@{*/
/*!
 * @fn             int8_t spi_flash_erase(uint32_t, uint32_t);
 * @brief          Erase a specified portion of SPI Flash.\n
 * @param [in]     u32Offset
 *                 Address (Offset) to erase from the SPI flash.
 * @param [in]     u32Sz
 *                 Size of SPI flash required to be erased.
 * @note         It is blocking function \n
* @warning
*                 - Address (offset) plus size of data must not exceed flash size.\n
*                 - No firmware is required for writing to SPI flash.\n
 *                 - In case of there is a running firmware, it is required to pause your firmware first
 *                   before any trial to access SPI flash to avoid any racing between host and running firmware on bus using
 *                   @ref m2m_wifi_download_mode
 *                 - It is blocking function\n
 * @sa             m2m_wifi_download_mode, spi_flash_get_size
 * @return       The function returns @ref M2M_SUCCESS for successful operations  and a negative value otherwise.

 */
int8_t spi_flash_erase(uint32_t u32Offset, uint32_t u32Sz);
 /**@}*/

#endif  //__SPI_FLASH_H__
