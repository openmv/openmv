/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * main function.
 */

#include "py/compile.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/stackctrl.h"
#include "shared/readline/readline.h"
#include "shared/runtime/gchelper.h"
#include "shared/runtime/pyexec.h"
#include "shared/runtime/softtimer.h"
#include "ticks.h"
#include "tusb.h"
#include "led.h"
#include "pendsv.h"
#include "modmachine.h"
#include "sdcard.h"
#include "systick.h"
#include "modmimxrt.h"

#include "py_fir.h"
#include "py_tv.h"

#if MICROPY_PY_LWIP
#include "lwip/init.h"
#include "lwip/apps/mdns.h"
#if MICROPY_PY_NETWORK_CYW43
#include "lib/cyw43-driver/src/cyw43.h"
#endif
#endif

#if MICROPY_PY_BLUETOOTH
#include "mpbthciport.h"
#include "extmod/modbluetooth.h"
#endif

#if MICROPY_PY_NETWORK
#include "extmod/modnetwork.h"
#endif

#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#include "common/factoryreset.h"

#include "omv_boardconfig.h"
#include "framebuffer.h"
#include "sensor.h"
#include "usbdbg.h"
#include "tinyusb_debug.h"
#include "fb_alloc.h"
#include "dma_alloc.h"
#include "ff_wrapper.h"
#include "mimxrt_hal.h"

extern uint8_t _sstack, _estack, _heap_start, _heap_end;

void exec_boot_script(const char *path, bool interruptible) {
    nlr_buf_t nlr;
    bool interrupted = false;
    if (nlr_push(&nlr) == 0) {
        // Enable IDE interrupts if allowed.
        if (interruptible) {
            usbdbg_set_irq_enabled(true);
            usbdbg_set_script_running(true);
        }

        // Parse, compile and execute the script.
        pyexec_file_if_exists(path, true);
        nlr_pop();
    } else {
        interrupted = true;
    }

    // Disable IDE interrupts
    usbdbg_set_irq_enabled(false);
    usbdbg_set_script_running(false);

    if (interrupted) {
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t) nlr.ret_val);
    }
}

int main(void) {
    bool sdcard_detected = false;
    bool sdcard_mounted = false;
    bool first_soft_reset = true;

    mimxrt_hal_init();
    ticks_init();
    led_init();
    pendsv_init();

soft_reset:
    #if defined(MICROPY_HW_LED1)
    led_init();
    #endif

    // Initialise stack extents and GC heap.
    mp_stack_set_top(&_estack);
    mp_stack_set_limit(&_estack - &_sstack - 1024);
    gc_init(&_heap_start, &_heap_end);

    // Initialise MicroPython runtime.
    mp_init();

    // Initialise low-level sub-systems.
    py_fir_init0();
    #if MICROPY_PY_TV
    py_tv_init0();
    #endif
    #if MICROPY_PY_BUZZER
    py_buzzer_init0();
    #endif // MICROPY_PY_BUZZER
    imlib_init_all();
    readline_init0();
    #if MICROPY_HW_ENABLE_CAN
    can_init0();
    #endif
    fb_alloc_init0();
    framebuffer_init0();
    sensor_init0();
    //dma_alloc_init0();
    #ifdef IMLIB_ENABLE_IMAGE_FILE_IO
    file_buffer_init0();
    #endif
    #if MICROPY_HW_ENABLE_SERVO
    servo_init();
    #endif
    usbdbg_init();
    machine_adc_init();
    #if MICROPY_PY_MACHINE_SDCARD
    machine_sdcard_init0();
    #endif
    #if MICROPY_PY_MACHINE_I2S
    machine_i2s_init0();
    #endif
    machine_rtc_start();

    #if MICROPY_PY_LWIP
    // lwIP doesn't allow to reinitialise itself by subsequent calls to this function
    // because the system timeout list (next_timeout) is only ever reset by BSS clearing.
    // So for now we only init the lwIP stack once on power-up.
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
    if (first_soft_reset) {
        cyw43_init(&cyw43_state);
        uint8_t buf[8];
        memcpy(&buf[0], "PYBD", 4);
        mp_hal_get_mac_ascii(MP_HAL_MAC_WLAN0, 8, 4, (char *) &buf[4]);
        cyw43_wifi_ap_set_ssid(&cyw43_state, 8, buf);
        cyw43_wifi_ap_set_password(&cyw43_state, 8, (const uint8_t *) "pybd0123");
    }
    #endif

    #if MICROPY_PY_NETWORK
    mod_network_init();
    #endif

    #if MICROPY_PY_SENSOR
    if (first_soft_reset
        && sensor_init() != 0) {
        printf("sensor init failed!\n");
    }
    #endif

    // Mount or create a fresh filesystem.
    mp_obj_t mount_point = MP_OBJ_NEW_QSTR(MP_QSTR__slash_);
    #if MICROPY_PY_MACHINE_SDCARD
    mimxrt_sdcard_obj_t *sdcard = &mimxrt_sdcard_objs[MICROPY_HW_SDCARD_SDMMC - 1];
    if (!sdcard->state->initialized) {
        mp_hal_pin_input(sdcard->pins->cd_b.pin);
        sdcard_detected = !mp_hal_pin_read(sdcard->pins->cd_b.pin);
    } else {
        sdcard_detected = sdcard_detect(sdcard);
    }
    if (sdcard_detected) {
        mp_obj_t args[] = { MP_OBJ_NEW_SMALL_INT(MICROPY_HW_SDCARD_SDMMC) };
        mp_obj_t bdev = MP_OBJ_TYPE_GET_SLOT(&machine_sdcard_type, make_new) (&machine_sdcard_type, 1, 0, args);
        if (mp_vfs_mount_and_chdir_protected(bdev, mount_point) == 0) {
            mimxrt_msc_medium = &machine_sdcard_type;
            sdcard_mounted = true;
        }
    }
    #endif

    if (sdcard_mounted == false) {
        mp_obj_t bdev = MP_OBJ_TYPE_GET_SLOT(&mimxrt_flash_type, make_new) (&mimxrt_flash_type, 0, 0, NULL);
        if (mp_vfs_mount_and_chdir_protected(bdev, mount_point) == 0) {
            mimxrt_msc_medium = &mimxrt_flash_type;
        } else {
            // Create a fresh filesystem.
            fs_user_mount_t *vfs = MP_OBJ_TYPE_GET_SLOT(&mp_fat_vfs_type, make_new) (&mp_fat_vfs_type, 1, 0, &bdev);
            if (factoryreset_create_filesystem(vfs) == 0) {
                if (mp_vfs_mount_and_chdir_protected(bdev, mount_point) == 0) {
                    mimxrt_msc_medium = &mimxrt_flash_type;
                }
            }
        }
    }

    // Deferred tusb_init to allow the filesystem to be mounted/created before MSC.
    if (!tusb_inited()) {
        tusb_init();
    }

    // Execute user scripts.
    exec_boot_script("boot.py", false);

    if (first_soft_reset) {
        exec_boot_script("main.py", true);
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

        if (usbdbg_is_busy() && nlr_push(&nlr) == 0) {
            // Enable IDE interrupt
            usbdbg_set_irq_enabled(true);
            // Wait for the current command to finish.
            usbdbg_wait_for_command(1000);
            // Disable IDE interrupts
            usbdbg_set_irq_enabled(false);
            nlr_pop();
        }
    }

    mp_printf(MP_PYTHON_PRINTER, "MPY: soft reboot\n");
    machine_pin_irq_deinit();
    machine_rtc_irq_deinit();
    #if MICROPY_PY_LWIP
    systick_disable_dispatch(SYSTICK_DISPATCH_LWIP);
    #endif
    #if MICROPY_PY_BLUETOOTH
    mp_bluetooth_deinit();
    #endif
    #if MICROPY_PY_NETWORK
    mod_network_deinit();
    #endif
    #if MICROPY_PY_MACHINE_I2S
    machine_i2s_deinit_all();
    #endif
    machine_pwm_deinit_all();
    soft_timer_deinit();
    gc_sweep_all();
    mp_deinit();
    first_soft_reset = false;
    goto soft_reset;
    return 0;
}

void gc_collect(void) {
    gc_collect_start();
    gc_helper_collect_regs_and_stack();
    gc_collect_end();
}

void nlr_jump_fail(void *val) {
    for (;;) {
    }
}

void __fatal_error() {
    nlr_jump_fail(NULL);
}


#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    mp_printf(MP_PYTHON_PRINTER, "Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    for (;;) {
    }
}
#endif

const char mimxrt_help_text[] =
    "Welcome to MicroPython!\n"
    "\n"
    "For online help please visit https://micropython.org/help/.\n"
    "\n"
    "For access to the hardware use the 'machine' module. \n"
    "\n"
    "Quick overview of some objects:\n"
    "  machine.Pin(pin) -- get a pin, eg machine.Pin(0)\n"
    "  machine.Pin(pin, m, [p]) -- get a pin and configure it for IO mode m, pull mode p\n"
    "    methods: init(..), value([v]), high(), low())\n"
    "\n"
    "    Pins are numbered board specific, either 0-n, or 'D0'-'Dn', or 'A0' - 'An',\n"
    "    according to the boards's pinout sheet.\n"
    "    Pin IO modes are: Pin.IN, Pin.OUT, Pin.OPEN_DRAIN\n"
    "    Pin pull modes are: Pin.PULL_UP, Pin.PULL_UP_47K, Pin.PULL_UP_22K, Pin.PULL_DOWN, Pin.PULL_HOLD\n"
    "  machine.ADC(pin) -- make an analog object from a pin\n"
    "    methods: read_u16()\n"
    "  machine.UART(id, baudrate=115200) -- create an UART object (id=1 - 8, board-specific)\n"
    "    methods: init(), write(buf), any()\n"
    "             buf=read(n), readinto(buf), buf=readline()\n"
    "    The RX and TX pins are fixed and board-specific.\n"
    "  machine.SoftI2C() -- create a Soft I2C object\n"
    "  machine.I2C(id) -- create a HW I2C object\n"
    "    methods: readfrom(addr, buf, stop=True), writeto(addr, buf, stop=True)\n"
    "             readfrom_mem(addr, memaddr, arg), writeto_mem(addr, memaddr, arg)\n"
    "    SoftI2C allows to use any pin for sda and scl, HW I2C id's and pins are fixed\n"
    "  machine.SoftSPI(baudrate=1000000) -- create a Soft SPI object\n"
    "  machine.SPI(id, baudrate=1000000) -- create a HW SPI object\n"
    "    methods: read(nbytes, write=0x00), write(buf), write_readinto(wr_buf, rd_buf)\n"
    "    SoftSPI allows to use any pin for SPI, HW SPI id's and pins are fixed\n"
    "  machine.Timer(id, freq, callback) -- create a hardware timer object (id=0,1,2)\n"
    "    eg: machine.Timer(freq=1, callback=lambda t:print(t))\n"
    "  machine.RTC() -- create a Real Time Clock object\n"
    "    methods: init(), datetime([dateime_tuple]), now()\n"
    "  machine.PWM(pin, freq, duty_u16[, kw_opts]) -- create a PWM object\n"
    "    methods: init(), duty_u16([value]), duty_ns([value]), freq([value])\n"
    "\n"
    "Useful control commands:\n"
    "  CTRL-C -- interrupt a running program\n"
    "  CTRL-D -- on a blank line, do a soft reset of the board\n"
    "  CTRL-E -- on a blank line, enter paste mode\n"
    "\n"
    "For further help on a specific object, type help(obj)\n"
    "For a list of available modules, type help('modules')\n"
;
