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
enum usbdbg_cmd {
    USBDBG_NONE=0,
    USBDBG_FRAME_SIZE       =0x81,
    USBDBG_FRAME_DUMP       =0x82,
    USBDBG_FRAME_LOCK       =0x83,
    USBDBG_FRAME_UPDATE     =0x04,
    USBDBG_SCRIPT_EXEC      =0x05,
    USBDBG_SCRIPT_STOP      =0x06,
    USBDBG_SCRIPT_SAVE      =0x07,
    USBDBG_TEMPLATE_SAVE    =0x08,
    USBDBG_DESCRIPTOR_SAVE  =0x09,
    USBDBG_ATTR_READ        =0x8A,
    USBDBG_ATTR_WRITE       =0x0B,
    USBDBG_SYS_RESET        =0x0C,
    USBDBG_BOOT             =0x0D,
    USBDBG_TX_BUF_LEN       =0x8E,
    USBDBG_TX_BUF           =0x8F
};
void usbdbg_init();
int usbdbg_script_ready();
vstr_t *usbdbg_get_script();
void usbdbg_clr_script();
#endif /* __USBDBG_H__ */
