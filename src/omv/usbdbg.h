#ifndef __USBDBG_H__
#define __USBDBG_H__
enum usbdbg_cmd {
    USBDBG_NONE=0,
    USBDBG_FB_SIZE,
    USBDBG_DUMP_FB,
    USBDBG_EXEC_SCRIPT,
    USBDBG_READ_SCRIPT,
    USBDBG_WRITE_SCRIPT,
    USBDBG_STOP_SCRIPT,
};
void usbdbg_init();
int usbdbg_script_ready();
vstr_t *usbdbg_get_script();
void usbdbg_clr_script();
#endif /* __USBDBG_H__ */
