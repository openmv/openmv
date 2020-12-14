/**
 *
 * \file
 *
 * \brief Programmer APIs.
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
