/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2015 Glenn Ruben Bakke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/mperrno.h"
#include "py/lexer.h"
#include "py/parse.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/stackctrl.h"
#include "py/gc.h"
#include "py/compile.h"
#include "shared/runtime/pyexec.h"
#include "shared/readline/readline.h"
#include "gccollect.h"
#include "modmachine.h"
#include "modmusic.h"
#include "modules/os/microbitfs.h"
#include "led.h"
#include "uart.h"
#include "nrf.h"
#include "pin.h"
#include "spi.h"
#include "i2c.h"
#include "adc.h"
#include "rtcounter.h"
#include "mphalport.h"
#include "pendsv.h"

#if MICROPY_PY_MACHINE_HW_PWM
#include "pwm.h"
#endif
#include "timer.h"

#if BLUETOOTH_SD
#include "nrf_sdm.h"
#endif

#if (MICROPY_PY_BLE_NUS)
#include "ble_uart.h"
#endif

#if MICROPY_PY_MACHINE_SOFT_PWM
#include "ticker.h"
#include "softpwm.h"
#endif

#if MICROPY_HW_USB_CDC
#include "usb_cdc.h"
#endif

#if MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE
#include "extmod/vfs_fat.h"
#include "lib/oofatfs/ff.h"
#include "extmod/vfs.h"
#include "modules/nrf/flashbdev.h"
#endif

#include "usbdbg.h"
#include "py_audio.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"
#include "omv_i2c.h"
#include "omv_csi.h"
#include "mp_utils.h"

extern uint32_t _heap_start;
extern uint32_t _heap_end;

uint32_t HAL_GetHalVersion() {
    // Hard-coded because it's not defined in SDK
    return ((2 << 24) | (0 << 16) | (0 << 8) | (0 << 0));
}

void NORETURN __fatal_error(const char *msg) {
    while (1) {
        ;
    }
}

#if MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE
static int vfs_mount_and_chdir(mp_obj_t bdev, mp_obj_t mount_point) {
    nlr_buf_t nlr;
    mp_int_t ret = -MP_EIO;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t args[] = { bdev, mount_point };
        mp_vfs_mount(2, args, (mp_map_t *) &mp_const_empty_map);
        mp_vfs_chdir(mount_point);
        ret = 0; // success
        nlr_pop();
    } else {
        mp_obj_base_t *exc = nlr.ret_val;
        if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(exc->type), MP_OBJ_FROM_PTR(&mp_type_OSError))) {
            mp_obj_t v = mp_obj_exception_get_value(MP_OBJ_FROM_PTR(exc));
            mp_obj_get_int_maybe(v, &ret); // get errno value
            ret = -ret;
        }
    }
    return ret;
}
#endif

void NORETURN _start(void) {
    bool first_soft_reset = true;

soft_reset:
    #if defined(MICROPY_BOARD_EARLY_INIT)
    MICROPY_BOARD_EARLY_INIT();
    #endif

    #if MICROPY_PY_TIME_TICKS
    rtc1_init_time_ticks();
    #endif

    led_init();
    led_state(1, 1); // MICROPY_HW_LED_1 aka MICROPY_HW_LED_RED

    // Initialize the stack and GC memory.
    mp_init_gc_stack(&_heap_end, &_ram_end, &_heap_start, &_heap_end, 400);

    machine_init();
    mp_init();
    readline_init0();
    pin_init0();
    #if MICROPY_PY_MACHINE_SPI
    spi_init0();
    #endif
    #if MICROPY_PY_MACHINE_I2C
    i2c_init0();
    #endif
    #if MICROPY_PY_MACHINE_ADC
    adc_init0();
    #endif
    #if MICROPY_PY_MACHINE_HW_PWM
    pwm_init0();
    #endif
    #if MICROPY_PY_MACHINE_RTCOUNTER
    rtc_init0();
    #endif
    #if MICROPY_PY_MACHINE_TIMER_NRF
    timer_init0();
    #endif
    #if MICROPY_PY_MACHINE_UART
    uart_init0();
    #endif
    fb_alloc_init0();
    framebuffer_init0();
    #if MICROPY_PY_CSI
    omv_csi_init0();
    #endif

    #if MICROPY_PY_CSI
    // Initialize the csi.
    if (first_soft_reset) {
        int ret = omv_csi_init();
        if (ret != 0 && ret != OMV_CSI_ERROR_ISC_UNDETECTED) {
            __fatal_error("Failed to init the CSI");
        }
    }
    #endif

    #if (MICROPY_PY_BLE_NUS == 0) && (MICROPY_HW_USB_CDC == 0)
    mp_obj_t args[2] = {
        MP_OBJ_NEW_SMALL_INT(0),
        MP_OBJ_NEW_SMALL_INT(115200),
    };
    MP_STATE_PORT(board_stdio_uart) = machine_hard_uart_type.make_new(
        (mp_obj_t) &machine_hard_uart_type, MP_ARRAY_SIZE(args), 0, args);
    #endif

    #if MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE
    flashbdev_init();

    // Try to mount the flash on "/flash" and chdir to it for the boot-up directory.
    mp_obj_t mount_point = MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash);
    int ret = vfs_mount_and_chdir((mp_obj_t) &nrf_flash_obj, mount_point);

    if ((ret == -MP_ENODEV) || (ret == -MP_EIO)) {
        pyexec_frozen_module("_mkfs.py", false); // Frozen script for formatting flash filesystem.
        ret = vfs_mount_and_chdir((mp_obj_t) &nrf_flash_obj, mount_point);
    }

    if (ret != 0) {
        printf("MPY: can't mount flash\n");
    }
    #endif

    #if MICROPY_MBFS
    microbit_filesystem_init();
    #endif

    #if MICROPY_HW_HAS_SDCARD
    // if an SD card is present then mount it on /sd/
    if (sdcard_is_present()) {
        // create vfs object
        fs_user_mount_t *vfs = m_new_obj_maybe(fs_user_mount_t);
        if (vfs == NULL) {
            goto no_mem_for_sd;
        }
        vfs->str = "/sd";
        vfs->len = 3;
        vfs->flags = MP_BLOCKDEV_FLAG_FREE_OBJ;
        sdcard_init_vfs(vfs);

        // put the sd device in slot 1 (it will be unused at this point)
        MP_STATE_PORT(fs_user_mount)[1] = vfs;

        FRESULT res = f_mount(&vfs->fatfs, vfs->str, 1);
        if (res != FR_OK) {
            printf("MPY: can't mount SD card\n");
            MP_STATE_PORT(fs_user_mount)[1] = NULL;
            m_del_obj(fs_user_mount_t, vfs);
        } else {
            // TODO these should go before the /flash entries in the path
            mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_sd));
            mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_sd_slash_lib));

            // use SD card as current directory
            f_chdrive("/sd");
        }
    no_mem_for_sd:;
    }
    #endif

    // Main script is finished, so now go into REPL mode.

    #if MICROPY_PY_BLE_NUS
    ble_uart_init0();
    #endif

    #if MICROPY_PY_MACHINE_SOFT_PWM
    ticker_init0();
    softpwm_init0();
    #endif

    #if MICROPY_PY_MUSIC
    microbit_music_init0();
    #endif
    #if BOARD_SPECIFIC_MODULES
    board_modules_init0();
    #endif

    #if MICROPY_PY_MACHINE_SOFT_PWM
    ticker_start();
    pwm_start();
    #endif

    led_state(1, 0);

    #if MICROPY_HW_USB_CDC
    usb_cdc_init();
    #endif

    usbdbg_init();
    pendsv_init();

    #if MICROPY_VFS || MICROPY_MBFS || MICROPY_MODULE_FROZEN
    // Run boot.py script.
    bool interrupted = mp_exec_bootscript("boot.py", true);

    // Run main.py script on first soft-reset.
    if (first_soft_reset && !interrupted && mp_vfs_import_stat("main.py")) {
        mp_exec_bootscript("main.py", true);
        goto soft_reset_exit;
    }
    #endif

    // If there's no script ready, just re-exec REPL
    while (!usbdbg_script_ready()) {
        nlr_buf_t nlr;

        if (nlr_push(&nlr) == 0) {
            // Enable IDE interrupt
            usbdbg_set_irq_enabled(true);

            // run REPL
            if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
                if (pyexec_raw_repl() != 0) {
                    break;
                }
            } else {
                if (pyexec_friendly_repl() != 0) {
                    break;
                }
            }

            nlr_pop();
        }
    }

    if (usbdbg_script_ready()) {
        nlr_buf_t nlr;
        if (nlr_push(&nlr) == 0) {
            // Enable IDE interrupt
            usbdbg_set_irq_enabled(true);
            // Execute the script.
            pyexec_str(usbdbg_get_script(), true);
            // Disable IDE interrupts
            usbdbg_set_irq_enabled(false);
            nlr_pop();
        } else {
            mp_obj_print_exception(&mp_plat_print, (mp_obj_t) nlr.ret_val);
        }
    }

soft_reset_exit:
    printf("MPY: soft reboot\n");
    #if MICROPY_PY_CSI
    omv_csi_abort_all();
    #endif
    #if MICROPY_PY_AUDIO
    py_audio_deinit();
    #endif
    #if MICROPY_PY_MACHINE_HW_PWM
    pwm_deinit_all();
    #endif
    #if BLUETOOTH_SD
    sd_softdevice_disable();
    #endif
    #if defined(MICROPY_BOARD_DEINIT)
    MICROPY_BOARD_DEINIT();
    #endif
    mp_deinit();

    first_soft_reset = false;
    goto soft_reset;
}

#if !MICROPY_VFS
#if MICROPY_MBFS
// Use micro:bit filesystem
mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    return uos_mbfs_new_reader(filename);
}

mp_import_stat_t mp_import_stat(const char *path) {
    return uos_mbfs_import_stat(path);
}

mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return uos_mbfs_open(n_args, args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

#else
// use dummy functions - no filesystem available
mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    mp_raise_OSError(MP_ENOENT);
}

mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    mp_raise_OSError(MP_EPERM);
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);
#endif
#endif

void HardFault_Handler(void) {
    #if defined(NRF52_SERIES) || defined(NRF91_SERIES)
    static volatile uint32_t reg;
    static volatile uint32_t reg2;
    static volatile uint32_t bfar;
    reg = SCB->HFSR;
    reg2 = SCB->CFSR;
    bfar = SCB->BFAR;
    for (int i = 0; i < 0; i++) {
        (void) reg;
        (void) reg2;
        (void) bfar;
    }
    #endif
}

void nlr_jump_fail(void *val) {
    printf("FATAL: uncaught exception %p\n", val);
    mp_obj_print_exception(&mp_plat_print, (mp_obj_t) val);
    __fatal_error("");
}

void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
