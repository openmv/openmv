#ifndef __USBDBG_H__
#define __USBDBG_H__
enum usbdbg_cmd {
    USBDBG_NONE=0,
    USBDBG_FRAME_SIZE,
    USBDBG_FRAME_DUMP,
    USBDBG_FRAME_LOCK,
    USBDBG_SCRIPT_EXEC,
    USBDBG_SCRIPT_STOP,
    USBDBG_SCRIPT_SAVE,
    USBDBG_TEMPLATE_SAVE,
    USBDBG_ATTR_READ,
    USBDBG_ATTR_WRITE,
};
void usbdbg_init();
int usbdbg_script_ready();
vstr_t *usbdbg_get_script();
void usbdbg_clr_script();
#endif /* __USBDBG_H__ */
