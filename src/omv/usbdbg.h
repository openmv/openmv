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
    USBDBG_FRAME_SIZE,
    USBDBG_FRAME_DUMP,
    USBDBG_FRAME_LOCK,
    USBDBG_FRAME_UPDATE,
    USBDBG_SCRIPT_EXEC,
    USBDBG_SCRIPT_STOP,
    USBDBG_SCRIPT_SAVE,
    USBDBG_TEMPLATE_SAVE,
    USBDBG_DESCRIPTOR_SAVE,
    USBDBG_ATTR_READ,
    USBDBG_ATTR_WRITE,
    USBDBG_SYS_RESET,
    USBDBG_BOOT
};
void usbdbg_init();
int usbdbg_script_ready();
vstr_t *usbdbg_get_script();
void usbdbg_clr_script();
#endif /* __USBDBG_H__ */
