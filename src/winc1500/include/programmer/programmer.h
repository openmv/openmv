/**
 *
 * \file
 *
 * \brief WINC Peripherals Application Interface.
 *
 * Copyright (c) 2016-2017 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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

#endif /* __PROGRAMMER_H__ */