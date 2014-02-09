#ifndef __USBDBG_H__
#define __USBDBG_H__
enum usbdbg_cmd { 
    USBDBG_NONE=0,
    USBDBG_DUMP_FB,
    USBDBG_EXEC_SCRIPT,
    USBDBG_READ_SCRIPT,
    USBDBG_WRITE_SCRIPT,
};
#endif /* __USBDBG_H__ */
