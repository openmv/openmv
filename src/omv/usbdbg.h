/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * USB debug support.
 *
 */
#ifndef __USBDBG_H__
#define __USBDBG_H__
/**
  * Firmware major, minor and patch versions.
  * Increment the major version if the ABI has changed.
  * Increment the minor version when a new command is added.
  * Increment the patch version for fixes that don't affect the ABI.
  *
  * Note: incrementing the major version will require a fw upgrade,
  * the IDE will Not connect if the major version number is different.
  */
#define FIRMWARE_VERSION_MAJOR      (2)
#define FIRMWARE_VERSION_MINOR      (7)
#define FIRMWARE_VERSION_PATCH      (0)

/**
  * To add a new debugging command, increment the last command value used.
  * Set the MSB of the value if the request has a device-to-host data phase.
  * Add the command to usr/openmv.py using the same value.
  * Handle the command control and data in/out (if any) phases in usbdbg.c.
  *
  * See usbdbg.c for examples.
  */
enum usbdbg_cmd {
    USBDBG_NONE             =0x00,
    USBDBG_FW_VERSION       =0x80,
    USBDBG_FRAME_SIZE       =0x81,
    USBDBG_FRAME_DUMP       =0x82,
    USBDBG_ARCH_STR         =0x83,
    USBDBG_SCRIPT_EXEC      =0x05,
    USBDBG_SCRIPT_STOP      =0x06,
    USBDBG_SCRIPT_SAVE      =0x07,
    USBDBG_SCRIPT_RUNNING   =0x87,
    USBDBG_TEMPLATE_SAVE    =0x08,
    USBDBG_DESCRIPTOR_SAVE  =0x09,
    USBDBG_ATTR_READ        =0x8A,
    USBDBG_ATTR_WRITE       =0x0B,
    USBDBG_SYS_RESET        =0x0C,
    USBDBG_FB_ENABLE        =0x0D,
    USBDBG_TX_BUF_LEN       =0x8E,
    USBDBG_TX_BUF           =0x8F
};
void usbdbg_init();
bool usbdbg_script_ready();
vstr_t *usbdbg_get_script();
bool usbdbg_get_irq_enabled();
void usbdbg_set_irq_enabled(bool enabled);
void usbdbg_set_script_running(bool running);
#endif /* __USBDBG_H__ */
