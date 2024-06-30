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
 * @file     issi_flash_private.h
 * @version  V1.0.0
 * @brief    Private header file for Flash driver to set up flash in XIP mode.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef ISSI_FLASH_PRIVATE_H
#define ISSI_FLASH_PRIVATE_H

#ifdef  __cplusplus
extern "C"
{
#endif

#define DEVICE_ID_ISSI_FLASH_IS25WX256                    0x9D

/* defines the Length of Address to be transmitted by Host controller */
#define ADDR_LENGTH_0_BITS                                0x0
#define ADDR_LENGTH_8_BITS                                0x2
#define ADDR_LENGTH_24_BITS                               0x6
#define ADDR_LENGTH_32_BITS                               0x8

/* defines the mode in which the slave Device is operating in */
#define DEVICE_MODE_SINGLE                                0x1
#define DEVICE_MODE_DUAL                                  0x2
#define DEVICE_MODE_QUAD                                  0x4
#define DEVICE_MODE_OCTAL                                 0x8

/* ISSI Flash Memory device Commands */
#define ISSI_RESET_ENABLE                                 0x66
#define ISSI_RESET_MEMORY                                 0x99

#define ISSI_READ_ID                                      0x9E

/* READ REGISTER OPERATIONS */
#define ISSI_READ_STATUS_REG                              0x05
#define ISSI_READ_FLAG_STATUS_REG                         0x70
#define ISSI_READ_NONVOLATILE_CONFIG_REG                  0xB5
#define ISSI_READ_VOLATILE_CONFIG_REG                     0x85

/* READ MEMORY OPERATIONS with 4-Byte Address */
#define ISSI_4BYTE_READ                                   0x13
#define ISSI_4BYTE_FAST_READ                              0x0C
#define ISSI_4BYTE_OCTAL_OUTPUT_FAST_READ                 0x7C
#define ISSI_4BYTE_OCTAL_IO_FAST_READ                     0xCC
#define ISSI_DDR_OCTAL_IO_FAST_READ                       0xFD

/* WRITE OPERATIONS */
#define ISSI_WRITE_ENABLE                                 0x06
#define ISSI_WRITE_DISABLE                                0x04

/* WRITE REGISTER OPERATIONS */
#define ISSI_WRITE_STATUS_REG                             0x01
#define ISSI_WRITE_NONVOLATILE_CONFIG_REG                 0xB1
#define ISSI_WRITE_VOLATILE_CONFIG_REG                    0x81
#define ISSI_WRITE_PROTECTION_MANAGEMENT_REG              0x68

/* Configuration Registers */
#define VOLATILE_CONFIG_REG                               0x0
#define NONVOLATILE_CONFIG_REG                            0x1

/* ISSI Register Settings */
#define OCTAL_DDR                                         0xC7
#define OCTAL_DDR_DQS                                     0xE7
#define WRAP_32_BYTE                                      0xFD
#define XIP_8IOFR                                         0xFE
#define DEFAULT_WAIT_CYCLES_ISSI                          0x10

#ifdef  __cplusplus
}
#endif

#endif /* ISSI_FLASH_PRIVATE_H */
