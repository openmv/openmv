/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
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
 *
 * STM32 main function.
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include STM32_HAL_H

#include "mpconfig.h"
#include "systick.h"
#include "pendsv.h"
#include "nlr.h"
#include "runtime.h"
#include "obj.h"
#include "gc.h"
#include "stackctrl.h"
#include "gccollect.h"
#include "timer.h"
#include "pin.h"
#include "usb.h"
#include "rtc.h"
#include "irq.h"
#include "rng.h"
#include "led.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "dac.h"
#include "pyb_can.h"
#include "extint.h"
#include "servo.h"
#include "storage.h"
#include "sdcard.h"
#include "modmachine.h"
#include "extmod/modmachine.h"
#if MICROPY_PY_NETWORK
#include "extmod/modnetwork.h"
#endif
#if MICROPY_PY_BLUETOOTH
#include "mpbthciport.h"
#include "extmod/modbluetooth.h"
#endif
#if MICROPY_PY_LWIP
#include "lwip/init.h"
#include "lwip/apps/mdns.h"
#if MICROPY_PY_NETWORK_CYW43
#include "lib/cyw43-driver/src/cyw43.h"
#endif
#endif
#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#include "shared/runtime/pyexec.h"
#include "shared/readline/readline.h"

#include "omv_boardconfig.h"
#include "omv_gpio.h"
#include "omv_i2c.h"
#include "omv_csi.h"
#include "mp_utils.h"
#include "framebuffer.h"

#include "usbdbg.h"
#include "sdram.h"
#include "stm_xspi.h"
#include "fb_alloc.h"
#include "dma_alloc.h"
#include "file_utils.h"

#include "py_image.h"
#include "py_fir.h"
#include "py_tv.h"
#include "py_imu.h"
#include "py_audio.h"

int errno;
extern void SystemClock_Config(void);

#if MICROPY_PY_THREAD
pyb_thread_t pyb_thread_main;
#endif

void NORETURN __fatal_error(const char *msg) {
    for (uint i = 0;;) {
        led_toggle(((i++) & 3));
        for (volatile uint delay = 0; delay < 500000; delay++) {
        }
    }
}

void nlr_jump_fail(void *val) {
    printf("FATAL: uncaught exception %p\n", val);
    __fatal_error("");
}

#ifndef NDEBUG
void __attribute__((weak)) __assert_func(const char *file, int line, const char *func, const char *expr) {
    (void) func;
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("");
}
#endif

#ifdef STACK_PROTECTOR
uint32_t __stack_chk_guard = 0xDEADBEEF;

void NORETURN __stack_chk_fail(void) {
    __fatal_error("stack check failed");
}
#endif

int main(void) {
    #if MICROPY_HW_SDRAM_SIZE
    bool sdram_ok = false;
    #endif
    bool first_soft_reset = true;

    // Initialize SysTick.
    HAL_InitTick(TICK_INT_PRIORITY);

    // Configure PLLs, oscillators, and system/peripheral clocks
    SystemClock_Config();

    #if defined(MICROPY_BOARD_EARLY_INIT)
    MICROPY_BOARD_EARLY_INIT();
    #endif

    // Uncomment to disable write buffer to get precise faults.
    // NOTE: Cache should be disabled on M7.
    //SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;

    // STM32Fxxx HAL library initialization:
    //  - Set NVIC Group Priority to 4
    //  - Configure the Flash prefetch, instruction and Data caches
    //  - Configure the Systick to generate an interrupt each 1 msec
    //  NOTE: The bootloader enables the CCM/DTCM memory.
    HAL_Init();

    #if MICROPY_HW_SDRAM_SIZE
    sdram_ok = sdram_init();
    #endif

    #if MICROPY_HW_SDRAM_STARTUP_TEST
    sdram_test(false);
    #endif

    #if MICROPY_HW_ENABLE_STORAGE
    storage_init();
    #endif

    #if OMV_XSPI_PSRAM_SIZE
    if (stm_xspi_psram_init() != 0) {
        __fatal_error("Failed to init XSPI PSRAM!");
    }
    #endif

    #if OMV_XSPI_PSRAM_STARTUP_TEST
    if (stm_xspi_psram_test(true) == 0) {
        __fatal_error("XSPI PSRAM test failed");
    }
    #endif

    // Basic sub-system init
    led_init();
    pendsv_init();
    #if MICROPY_PY_THREAD
    pyb_thread_init(&pyb_thread_main);
    #endif

    // Re-enable IRQs (disabled by bootloader)
    __enable_irq();

soft_reset:
    for (size_t i = 0; i < 4; i++) {
        led_state(i + 1, 0);
    }

    machine_init();

    #if MICROPY_PY_THREAD
    // Python threading init
    mp_thread_init();
    #endif

    // Initialize the stack and GC memory.
    mp_init_gc_stack(&_sstack, &_estack, &_heap_start, &_heap_end, 1024);

    // Micro Python init
    mp_init();

    // Initialise low-level sub-systems.
    #if MICROPY_PY_FIR
    py_fir_init0();
    #endif // MICROPY_PY_FIR
    #if MICROPY_PY_TV
    py_tv_init0();
    #endif
    imlib_init_all();
    readline_init0();
    pin_init0();
    extint_init0();
    timer_init0();
    #if MICROPY_HW_ENABLE_CAN
    pyb_can_init0();
    #endif
    #if MICROPY_PY_PYB_LEGACY && MICROPY_HW_ENABLE_HW_I2C
    i2c_init0();
    #endif
    spi_init0();
    uart_init0();
    fb_alloc_init0();
    omv_gpio_init0();
    framebuffer_init0();
    #if MICROPY_PY_CSI
    omv_csi_init0();
    #endif
    #if OMV_DMA_ALLOC
    dma_alloc_init0();
    #endif
    #ifdef IMLIB_ENABLE_IMAGE_FILE_IO
    file_buffer_init0();
    #endif
    #if MICROPY_HW_ENABLE_SERVO
    servo_init();
    #endif
    usbdbg_init();
    #if MICROPY_HW_ENABLE_SDCARD
    sdcard_init();
    #endif
    rtc_init_start(false);
    #if MICROPY_PY_LWIP
    // lwIP can only be initialized once, because the system timeout
    // list (next_timeout), is only ever reset by BSS clearing.
    if (first_soft_reset) {
        lwip_init();
        #if LWIP_MDNS_RESPONDER
        mdns_resp_init();
        #endif
    }
    systick_enable_dispatch(SYSTICK_DISPATCH_LWIP, mod_network_lwip_poll_wrapper);
    #endif

    #if MICROPY_PY_BLUETOOTH
    mp_bluetooth_hci_init();
    #endif

    #if MICROPY_PY_NETWORK_CYW43
    cyw43_init(&cyw43_state);
    uint8_t buf[8];
    memcpy(&buf[0], "PYBD", 4);
    mp_hal_get_mac_ascii(MP_HAL_MAC_WLAN0, 8, 4, (char *) &buf[4]);
    cyw43_wifi_ap_set_ssid(&cyw43_state, 8, buf);
    cyw43_wifi_ap_set_auth(&cyw43_state, CYW43_AUTH_WPA2_AES_PSK);
    cyw43_wifi_ap_set_password(&cyw43_state, 8, (const uint8_t *) "pybd0123");
    #endif

    pyb_usb_init0();
    MP_STATE_PORT(pyb_stdio_uart) = NULL;

    #if MICROPY_PY_CSI
    // Initialize the csi.
    if (first_soft_reset) {
        int ret = omv_csi_init();
        if (ret != 0 && ret != OMV_CSI_ERROR_ISC_UNDETECTED) {
            __fatal_error("Failed to init the CSI");
        }
    }
    #endif

    #if MICROPY_PY_IMU
    py_imu_init();
    #endif // MICROPY_PY_IMU

    #if MICROPY_PY_NETWORK
    mod_network_init();
    #endif

    // Execute _boot.py to set up the filesystem.
    pyexec_frozen_module("_boot.py", false);

    // Set the USB medium to flash block device.
    pyb_usb_storage_medium = PYB_USB_STORAGE_MEDIUM_FLASH;

    const char *path = "/sdcard";
    // If SD is mounted, set the USB medium to SD.
    if (mp_vfs_lookup_path(path, &path) != MP_VFS_NONE) {
        pyb_usb_storage_medium = PYB_USB_STORAGE_MEDIUM_SDCARD;
    }

    // Init USB device to default setting if it was not already configured
    if (!(pyb_usb_flags & PYB_USB_FLAG_USB_MODE_CALLED)) {
        uint8_t usb_mode = USBD_MODE_CDC_MSC;
        #if MICROPY_HW_USB_HS
        usb_mode |= USBD_MODE_HIGH_SPEED;
        #endif
        pyb_usb_dev_init(pyb_usb_dev_detect(), MICROPY_HW_USB_VID,
                         MICROPY_HW_USB_PID, usb_mode, 0, NULL, NULL);
    }

    // report if SDRAM failed
    #if MICROPY_HW_SDRAM_SIZE
    if (first_soft_reset && !sdram_ok) {
        __fatal_error("Failed to init sdram!");
    }
    #endif

    // Run boot.py script.
    bool interrupted = mp_exec_bootscript("boot.py", true);

    // Run main.py script on first soft-reset.
    if (first_soft_reset && !interrupted && mp_vfs_import_stat("main.py")) {
        mp_exec_bootscript("main.py", true);
        goto soft_reset_exit;
    }

    // If there's no script ready, just re-exec REPL
    while (!usbdbg_script_ready()) {
        nlr_buf_t nlr;

        if (nlr_push(&nlr) == 0) {
            // enable IDE interrupt
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
            // Enable IDE interrupts
            usbdbg_set_irq_enabled(true);
            // Execute the script.
            pyexec_str(usbdbg_get_script(), true);
            // Disable IDE interrupts
            usbdbg_set_irq_enabled(false);
            nlr_pop();
        } else {
            mp_obj_print_exception(MP_PYTHON_PRINTER, (mp_obj_t) nlr.ret_val);
        }
    }

soft_reset_exit:
    // soft reset
    mp_printf(MP_PYTHON_PRINTER, "MPY: soft reboot\n");
    #if MICROPY_PY_CSI
    omv_csi_abort_all();
    #endif
    #if MICROPY_PY_LWIP
    systick_disable_dispatch(SYSTICK_DISPATCH_LWIP);
    #endif
    #if MICROPY_PY_BLUETOOTH
    mp_bluetooth_deinit();
    #endif
    #if MICROPY_PY_NETWORK
    mod_network_deinit();
    #endif
    #if MICROPY_PY_NETWORK_CYW43
    cyw43_deinit(&cyw43_state);
    #endif
    timer_deinit();
    #if MICROPY_PY_PYB_LEGACY && MICROPY_HW_ENABLE_HW_I2C
    pyb_i2c_deinit_all();
    #endif
    #if MICROPY_PY_MACHINE_I2C_TARGET
    mp_machine_i2c_target_deinit_all();
    #endif
    spi_deinit_all();
    uart_deinit_all();
    #if MICROPY_HW_ENABLE_CAN
    pyb_can_deinit_all();
    #endif
    #if MICROPY_PY_THREAD
    pyb_thread_deinit();
    #endif
    #if MICROPY_PY_AUDIO
    py_audio_deinit();
    #endif
    imlib_deinit_all();
    gc_sweep_all();
    mp_deinit();
    first_soft_reset = false;
    goto soft_reset;
}

static mp_obj_t pyb_main(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // Unused.
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(pyb_main_obj, 1, pyb_main);
