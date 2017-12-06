/**
 *
 * \file
 *
 * \brief Programmer APIs.
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

#ifndef FIRMWARE_PROGRAMMER_APIS_H_INCLUDED
#define FIRMWARE_PROGRAMMER_APIS_H_INCLUDED

#include "common/include/nm_common.h"
#include "programmer/programmer.h"
#include "spi_flash/include/spi_flash_map.h"

#define programmer_write_cert_image(buff)   programmer_write((uint8*)buff, M2M_TLS_FLASH_ROOTCERT_CACHE_OFFSET, M2M_TLS_FLASH_ROOTCERT_CACHE_SIZE)
#define programmer_read_cert_image(buff)    programmer_read((uint8*)buff, M2M_TLS_FLASH_ROOTCERT_CACHE_OFFSET, M2M_TLS_FLASH_ROOTCERT_CACHE_SIZE)
#define programmer_erase_cert_image()       programmer_erase(M2M_TLS_FLASH_ROOTCERT_CACHE_OFFSET, M2M_TLS_FLASH_ROOTCERT_CACHE_SIZE)

#define programmer_write_firmware_image(buff,offSet,sz) programmer_write((uint8*)buff, offSet, sz)
#define programmer_read_firmware_image(buff,offSet,sz)  programmer_read((uint8*)buff, offSet, sz)

#define programmer_erase_all()               programmer_erase(0, programmer_get_flash_size())

#endif /* FIRMWARE_PROGRAMMER_APIS_H_INCLUDED */
