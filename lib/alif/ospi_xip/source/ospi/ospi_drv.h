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
 * @file     ospi_drv.h
 * @version  V1.0.0
 * @brief    Header file for OSPI driver to set up flash in XIP mode.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef OSPI_DRV_H
#define OSPI_DRV_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ospi_private.h"

typedef struct {
    ssi_regs_t *regs;                  ///< Pointer to OSPI0 or OSPI1 base address
    aes_regs_t *aes_regs;              ///< Pointer to AES0 or AES1 decryption module & XIP enable line registers
    volatile void *xip_base;           ///< Pointer to OSPI0 or OSPI1 XIP base address
    uint32_t    ospi_clock;            ///< Octal SPI clock - Device and Bus characteristics defined
    uint32_t    ser;                   ///< Slave Select / Enable register - application defined (to drive ospiN_ss0 or ospiN_ss1)
    uint32_t    addrlen;               ///< Address length 3 or 4 bytes depending on the device capacity
    uint32_t    ddr_en;                ///< DDR enable if set to 1, default 0
    uint32_t    rx_req;                ///< Requested data to receive
    uint32_t    rx_cnt;                ///< Received data count
    uint32_t    device_id;             ///< Manufacturer Device ID
    uint32_t    wait_cycles;           ///< Wait cycles - Device defined for switching from Rx to Tx
} ospi_flash_cfg_t ;

/* Function Prototypes */
void ospi_init(ospi_flash_cfg_t *ospi_cfg);
void ospi_xip_enter(ospi_flash_cfg_t *ospi_cfg, uint16_t incr_command, uint16_t wrap_command);
void ospi_recv_blocking(ospi_flash_cfg_t *ospi_cfg, uint32_t command, uint8_t *buffer);
void ospi_push(ospi_flash_cfg_t *ospi_cfg, uint32_t data);
void ospi_send_blocking(ospi_flash_cfg_t *ospi_cfg, uint32_t data);
void ospi_setup_write(ospi_flash_cfg_t *ospi_cfg, uint32_t addr_len);
void ospi_setup_write_sdr(ospi_flash_cfg_t *ospi_cfg, uint32_t addr_len);
void ospi_setup_read(ospi_flash_cfg_t *ospi_cfg, uint32_t addr_len, uint32_t read_len, uint32_t wait_cycles);
void ospi_xip_exit(ospi_flash_cfg_t *ospi_cfg, uint16_t incr_command, uint16_t wrap_command);
bool ospi_xip_enabled(ospi_flash_cfg_t *ospi_cfg);

#ifdef  __cplusplus
}
#endif

#endif /* OSPI_DRV_H */
