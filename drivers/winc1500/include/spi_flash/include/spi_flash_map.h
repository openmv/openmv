/**
 *
 * \file
 *
 * \brief WINC1500 SPI Flash.
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

/**
*  @file		spi_flash_map.h
*  @brief		This module contains spi flash CONTENT
*  @author		M.S.M
*  @date		17 SEPT 2013
*  @version		1.0
*/
#ifndef __SPI_FLASH_MAP_H__
#define __SPI_FLASH_MAP_H__

#define FLASH_MAP_VER_0		(0)
#define FLASH_MAP_VER_1		(1)
#define FLASH_MAP_VER_2		(2)
#define FLASH_MAP_VER_3		(3)
#define FLASH_MAP_VER_4     (4)

#define FLASH_MAP_VERSION   FLASH_MAP_VER_4

//#define DOWNLOAD_ROLLBACK
//#define OTA_GEN
#define _PROGRAM_POWER_SAVE_

/* =======*=======*=======*=======*=======
 * 	  General Sizes for Flash Memory
 * =======*=======*=======*=======*=======
 */

#define FLASH_START_ADDR					(0UL)
/*!<Starting Address of Flash Memory
 *
 */
#define FLASH_BLOCK_SIZE					(32 * 1024UL)
/*!<Block Size in Flash Memory
 */
#define FLASH_SECTOR_SZ						(4 * 1024UL)
/*!<Sector Size in Flash Memory
 */
#define FLASH_PAGE_SZ						(256)
/*!<Page Size in Flash Memory
 */
#define FLASH_2M_TOTAL_SZ					(256 * 1024UL)
/*!<Total Size of 2M Flash Memory
 */
#define FLASH_4M_TOTAL_SZ					(512 * 1024UL)
/*!<Total Size of 4M Flash Memory
 */
#define FLASH_8M_TOTAL_SZ					(1024 * 1024UL)
/*!<Total Size of 8M Flash Memory
 */

/*
 * Detailed Sizes and locations for Flash Memory:
 *  ____________________ ___________ ____________________________________________________________________________
 * | Starting Address	|	Size	|	Location's Name			|	Description						   			|
 * |____________________|___________|___________________________|_______________________________________________|
 * |	  0 K  			|	  4	K	| 	Boot Firmware			|	Firmware to select which version to run		|
 * |	  4	K 			|	  8 K	|	Control Section			|	Structured data used by Boot firmware		|
 * |	 12 K			|     4	K	|	PLL+GAIN :				|	LookUp Table for PLL and Gain calculations	|
 * |	  				|     		|	PLL  Size = 1K			|		PLL				 						|
 * |	  				|     		|	GAIN Size = 3K			|		Gain configuration				 		|
 * |	 16	K			|	  4	K	|	CERTIFICATE				|	X.509 Certificate storage					|
 * |	 20	K			|	  8	K	|	TLS Server				|	TLS Server Private Key and certificates		|
 * |	 28	K			|	  8	K	|	HTTP Files				|	Files used with Provisioning Mode			|
 * |	 36	K			|	  4	K	|	Connection Parameters	|	Parameters for success connection to AP		|
 * |	 40	K			|	236 K 	|	Main Firmware/program	|	Main Firmware to run WiFi Chip				|
 * |	276	K			|	236 K	|	OTA Firmware		    |	OTA firmware								|
 * |    512 K           |   512 K   |   Host File Storage       |    WINC1510 (8Mb of Flash) only               |
 * |------------------------------------------------------------------------------------------------------------|
 * |                     Total flash size is 512 K for WINC1500 and 1024 K for WINC1510                         |
 * |____________________________________________________________________________________________________________|
 *
 *
 * *Keys for Comments with each MACRO:
 * 		"L:xxxK" -means-> location 	:xxxK
 *		"S:xxxK" -means-> Size is 	:xxxK
 */

/*
 * Boot Firmware: which used to select which firmware to run
 *
 */
#define M2M_BOOT_FIRMWARE_STARTING_ADDR			(FLASH_START_ADDR)
#define M2M_BOOT_FIRMWARE_FLASH_SZ				(FLASH_SECTOR_SZ)

/*
 * Control Section: which used by Boot firmware
 *
 */
#define M2M_CONTROL_FLASH_OFFSET				(M2M_BOOT_FIRMWARE_STARTING_ADDR + M2M_BOOT_FIRMWARE_FLASH_SZ)
#define M2M_CONTROL_FLASH_BKP_OFFSET			(M2M_CONTROL_FLASH_OFFSET + FLASH_SECTOR_SZ)
#define M2M_CONTROL_FLASH_SEC_SZ				(FLASH_SECTOR_SZ)
#define M2M_CONTROL_FLASH_TOTAL_SZ				(FLASH_SECTOR_SZ * 2)

/*
 * LUT for PLL and TX Gain settings:
 *
 */
#define M2M_PLL_FLASH_OFFSET					(M2M_CONTROL_FLASH_OFFSET + M2M_CONTROL_FLASH_TOTAL_SZ)
#define M2M_PLL_FLASH_SZ						(1024 * 1)
#define M2M_GAIN_FLASH_OFFSET					(M2M_PLL_FLASH_OFFSET + M2M_PLL_FLASH_SZ)
#define M2M_GAIN_FLASH_SZ						(M2M_CONFIG_SECT_TOTAL_SZ - M2M_PLL_FLASH_SZ)
#define M2M_CONFIG_SECT_TOTAL_SZ				(FLASH_SECTOR_SZ)

/*
 * Certificate:
 *
 */
#define M2M_TLS_ROOTCER_FLASH_OFFSET			(M2M_PLL_FLASH_OFFSET + M2M_CONFIG_SECT_TOTAL_SZ)
#define M2M_TLS_ROOTCER_FLASH_SIZE				(FLASH_SECTOR_SZ * 1)

/*
 * TLS Server Key Files
 *
 */
#define M2M_TLS_SERVER_FLASH_OFFSET				(M2M_TLS_ROOTCER_FLASH_OFFSET + M2M_TLS_ROOTCER_FLASH_SIZE)
#define M2M_TLS_SERVER_FLASH_SIZE				(FLASH_SECTOR_SZ * 2)

/*
 * HTTP Files
 *
 */
#define M2M_HTTP_MEM_FLASH_OFFSET				(M2M_TLS_SERVER_FLASH_OFFSET + M2M_TLS_SERVER_FLASH_SIZE)
#define M2M_HTTP_MEM_FLASH_SZ					(FLASH_SECTOR_SZ * 2)

/*
 * Saved Connection Parameters:
 *
 */
#define M2M_CACHED_CONNS_FLASH_OFFSET			(M2M_HTTP_MEM_FLASH_OFFSET + M2M_HTTP_MEM_FLASH_SZ)
#define M2M_CACHED_CONNS_FLASH_SZ				(FLASH_SECTOR_SZ * 1)

/*
 *
 * Common section size
 */

#define M2M_COMMON_DATA_SEC		\
	(\
		M2M_BOOT_FIRMWARE_FLASH_SZ 		+ 	\
		M2M_CONTROL_FLASH_TOTAL_SZ 		+ 	\
		M2M_CONFIG_SECT_TOTAL_SZ 		+ 	\
		M2M_TLS_ROOTCER_FLASH_SIZE 		+ 	\
		M2M_TLS_SERVER_FLASH_SIZE 		+ 	\
		M2M_HTTP_MEM_FLASH_SZ			+ 	\
		M2M_CACHED_CONNS_FLASH_SZ		 	\
	)
	
/*
 *
 * OTA image1 Offset
 */								

#define M2M_OTA_IMAGE1_OFFSET					(M2M_CACHED_CONNS_FLASH_OFFSET + M2M_CACHED_CONNS_FLASH_SZ)
/*
 * Firmware Offset
 *
 */
#if (defined _FIRMWARE_)||(defined OTA_GEN)
#define M2M_FIRMWARE_FLASH_OFFSET				(0UL)
#else
#if (defined DOWNLOAD_ROLLBACK)
#define M2M_FIRMWARE_FLASH_OFFSET				(M2M_OTA_IMAGE2_OFFSET)
#else
#define M2M_FIRMWARE_FLASH_OFFSET				(M2M_OTA_IMAGE1_OFFSET)
#endif
#endif
/*
 *
 * Firmware
 */
#define M2M_FIRMWARE_FLASH_SZ					(236 * 1024UL)
/**
 *
 * OTA image Size
 */
#define OTA_IMAGE_SIZE							(M2M_FIRMWARE_FLASH_SZ)
/**
 *
 * Flash Total size
 */
#define FLASH_IMAGE1_CONTENT_SZ 				(M2M_COMMON_DATA_SEC  +  OTA_IMAGE_SIZE)
									
/**
 *
 * OTA image 2 offset
 */
#define M2M_OTA_IMAGE2_OFFSET					(FLASH_IMAGE1_CONTENT_SZ)

/*
 * App(Cortus App 4M): App. which runs over firmware
 *
 */
#ifdef CORTUS_APP
#define M2M_APP_4M_MEM_FLASH_SZ					(FLASH_SECTOR_SZ * 16)
#define M2M_APP_4M_MEM_FLASH_OFFSET				(FLASH_4M_TOTAL_SZ - M2M_APP_4M_MEM_FLASH_SZ)
#define M2M_APP_8M_MEM_FLASH_OFFSET				(M2M_OTA_IMAGE2_OFFSET + OTA_IMAGE_SIZE)
#define M2M_APP_8M_MEM_FLASH_SZ					(FLASH_SECTOR_SZ * 32)
#define M2M_APP_OTA_MEM_FLASH_OFFSET			(M2M_APP_8M_MEM_FLASH_OFFSET + M2M_APP_8M_MEM_FLASH_SZ)
#define M2M_HFD_8M_MEM_FLASH_OFFSET             (M2M_APP_8M_MEM_FLASH_OFFSET + M2M_APP_8M_MEM_FLASH_SZ)
#else
#define M2M_HFD_8M_MEM_FLASH_OFFSET             (M2M_OTA_IMAGE2_OFFSET + OTA_IMAGE_SIZE)
#endif
/* Check if total size of content
 *  don't exceed total size of memory allowed
 **/
#if (M2M_COMMON_DATA_SEC  +  (OTA_IMAGE_SIZE *2)> FLASH_4M_TOTAL_SZ)
#error "Exceeds 4M Flash Size"
#endif /* (FLASH_CONTENT_SZ > FLASH_TOTAL_SZ) */

/**
 * Magic value to differentiate between old HTTP flash section format and newer formats.
 * The lowest byte is ignored when checking the value as it contains the
 * version number (it should always be 00 here, image_builder will set this value in flash).
 **/
#define HTTP_FLASH_SECTION_MAGIC	0xB00B1500
#define HTTP_FLASH_SECTION_VERSION	2


#endif /* __SPI_FLASH_MAP_H__ */
