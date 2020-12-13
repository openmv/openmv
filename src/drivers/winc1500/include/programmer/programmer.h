/**
 *
 * \file
 *
 * \brief WINC Peripherals Application Interface.
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
 
#ifndef __PROGRAMMER_H__
#define __PROGRAMMER_H__

/**
* Include
*/
#include "spi_flash/include/spi_flash_map.h"
#include "spi_flash/include/spi_flash.h"
#include "programmer/programmer_apis.h"


#define ROOT_CERT_SIZE M2M_TLS_ROOTCER_FLASH_SIZE

#define	programmer_get_flash_size()						(((spi_flash_get_size()*1024)/8)*1024)
#define	programmer_write(pu8Buf, u32Offset, u32Sz)		spi_flash_write(pu8Buf, u32Offset, u32Sz)
#define	programmer_erase(u32Offset, u32Sz)				spi_flash_erase(u32Offset, u32Sz)
#define	programmer_eraseall()							programmer_erase(0, programmer_get_flash_size())
#define	programmer_read(pu8Buf, u32Offset, u32Sz)		spi_flash_read(pu8Buf, u32Offset, u32Sz)	


#define programmer_write_root_cert(buff)				programmer_write((uint8*)buff, M2M_TLS_ROOTCER_FLASH_OFFSET, M2M_TLS_ROOTCER_FLASH_SIZE)
#define programmer_read_root_cert(buff)					programmer_read((uint8*)buff, M2M_TLS_ROOTCER_FLASH_OFFSET, M2M_TLS_ROOTCER_FLASH_SIZE)
#define programmer_erase_root_cert()					programmer_erase(M2M_TLS_ROOTCER_FLASH_OFFSET, M2M_TLS_ROOTCER_FLASH_SIZE)

#define programmer_write_tls_cert_store(buff)			programmer_write((uint8*)buff, M2M_TLS_SERVER_FLASH_OFFSET, M2M_TLS_SERVER_FLASH_SIZE)
#define programmer_read_tls_cert_store(buff)			programmer_read((uint8*)buff, M2M_TLS_SERVER_FLASH_OFFSET, M2M_TLS_SERVER_FLASH_SIZE)
#define programmer_erase_tls_cert_store()				programmer_erase(M2M_TLS_SERVER_FLASH_OFFSET, M2M_TLS_SERVER_FLASH_SIZE)

int burn_firmware(const char *path);
int verify_firmware(const char *path);
int dump_firmware(const char *path);
#endif /* __PROGRAMMER_H__ */
