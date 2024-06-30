/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     IS25WX256.h
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     21-Oct-2022
 * @brief    Device Specific Header file for ISSI FLASH Driver.
 ******************************************************************************/

#ifndef __IS25WX256_H__
#define __IS25WX256_H__

#ifdef  __cplusplus
extern "C"
{
#endif

#define FLASH_ISSI_SECTOR_COUNT                       ((uint32_t)8192)    /* Number of sectors */
#define FLASH_ISSI_SECTOR_SIZE                        ((uint32_t)4096)    /* Sector size: 4kB */
#define FLASH_ISSI_PAGE_SIZE                          ((uint32_t)256)     /* Programming page size in bytes */
#define FLASH_ISSI_PROGRAM_UNIT                       ((uint32_t)2)       /* Smallest programmable unit in bytes */
#define FLASH_ISSI_ERASED_VALUE                       ((uint8_t)0xFF)     /* Contents of erased memory */

#ifdef __cplusplus
}
#endif

#endif
