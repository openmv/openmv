/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * main function.
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include STM32_HAL_H
#include "mpconfig.h"
#include "systick.h"
#include "pendsv.h"
#include "qstr.h"
#include "nlr.h"
#include "lexer.h"
#include "parse.h"
#include "compile.h"
#include "runtime.h"
#include "obj.h"
#include "objmodule.h"
#include "objstr.h"
#include "gc.h"
#include "stackctrl.h"
#include "gccollect.h"
#include "readline.h"
#include "timer.h"
#include "pin.h"
#include "usb.h"
#include "rtc.h"
#include "storage.h"
#include "sdcard.h"
#include "ff.h"
#include "modnetwork.h"

#include "lib/utils/pyexec.h"
#include "lib/fatfs/ff.h"
#include "extmod/fsusermount.h"

#include "irq.h"
#include "rng.h"
#include "led.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "dac.h"
#include "extint.h"
#include "servo.h"

#include "sensor.h"
#include "usbdbg.h"
#include "sdram.h"
#include "fb_alloc.h"
#include "ff_wrapper.h"

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc_msc_hid.h"
#include "usbd_cdc_interface.h"
#include "usbd_msc_storage.h"

#include "py_sensor.h"
#include "py_image.h"
#include "py_lcd.h"
#include "py_fir.h"

int errno;
extern char _vfs_buf;
extern char _stack_size;
static fs_user_mount_t *vfs = (fs_user_mount_t *) &_vfs_buf;

static const char fresh_main_py[] =
"# main.py -- put your code here!\n"
"import pyb, time\n"
"led = pyb.LED(3)\n"
"usb = pyb.USB_VCP()\n"
"while (usb.isconnected()==False):\n"
"   led.on()\n"
"   time.sleep(150)\n"
"   led.off()\n"
"   time.sleep(100)\n"
"   led.on()\n"
"   time.sleep(150)\n"
"   led.off()\n"
"   time.sleep(600)\n"
;

static const char fresh_openmv_inf[] =
#include "genhdr/openmv_inf.h"
;

static const char fresh_readme_txt[] =
"This is a Micro Python board\r\n"
"\r\n"
"You can get started right away by writing your Python code in 'main.py'.\r\n"
"\r\n"
"For a serial prompt:\r\n"
" - Windows: you need to go to 'Device manager', right click on the unknown device,\r\n"
"   then update the driver software, using the 'openmv.inf' file found on this drive.\r\n"
"   Then use a terminal program like Hyperterminal or putty.\r\n"
" - Mac OS X: use the command: screen /dev/tty.usbmodem*\r\n"
" - Linux: use the command: screen /dev/ttyACM0\r\n"
"\r\n"
"Please visit https://openmv.io/ or http://micropython.org/help/ for further help.\r\n"
;

#ifdef OPENMV1
static const char fresh_selftest_py[] ="";
#else
static const char fresh_selftest_py[] =
"import sensor, time, pyb\n"
"\n"
"def test_int_adc():\n"
"    adc  = pyb.ADCAll(12)\n"
"    # Test VBAT\n"
"    vbat = adc.read_core_vbat()\n"
"    vbat_diff = abs(vbat-3.3)\n"
"    if (vbat_diff > 0.1):\n"
"        raise Exception('INTERNAL ADC TEST FAILED VBAT=%fv'%vbat)\n"
"\n"
"    # Test VREF\n"
"    vref = adc.read_core_vref()\n"
"    vref_diff = abs(vref-1.2)\n"
"    if (vref_diff > 0.1):\n"
"        raise Exception('INTERNAL ADC TEST FAILED VREF=%fv'%vref)\n"
"    adc = None\n"
"    print('INTERNAL ADC TEST PASSED...')\n"
"\n"
"def test_color_bars():\n"
"    sensor.reset()\n"
"    # Set sensor settings\n"
"    sensor.set_brightness(0)\n"
"    sensor.set_saturation(3)\n"
"    sensor.set_gainceiling(8)\n"
"    sensor.set_contrast(2)\n"
"\n"
"    # Set sensor pixel format\n"
"    sensor.set_framesize(sensor.QVGA)\n"
"    sensor.set_pixformat(sensor.RGB565)\n"
"\n"
"    # Enable colorbar test mode\n"
"    sensor.set_colorbar(True)\n"
"\n"
"    # Skip a few frames to allow the sensor settle down\n"
"    for i in range(0, 100):\n"
"        image = sensor.snapshot()\n"
"\n"
"    #color bars thresholds\n"
"    t = [lambda r, g, b: r < 70  and g < 70  and b < 70,   # Black\n"
"         lambda r, g, b: r < 70  and g < 70  and b > 200,  # Blue\n"
"         lambda r, g, b: r > 200 and g < 70  and b < 70,   # Red\n"
"         lambda r, g, b: r > 200 and g < 70  and b > 200,  # Purple\n"
"         lambda r, g, b: r < 70  and g > 200 and b < 70,   # Green\n"
"         lambda r, g, b: r < 70  and g > 200 and b > 200,  # Aqua\n"
"         lambda r, g, b: r > 200 and g > 200 and b < 70,   # Yellow\n"
"         lambda r, g, b: r > 200 and g > 200 and b > 200]  # White\n"
"\n"
"    # color bars are inverted for OV7725\n"
"    if (sensor.get_id() == sensor.OV7725):\n"
"        t = t[::-1]\n"
"\n"
"    #320x240 image with 8 color bars each one is approx 40 pixels.\n"
"    #we start from the center of the frame buffer, and average the\n"
"    #values of 10 sample pixels from the center of each color bar.\n"
"    for i in range(0, 8):\n"
"        avg = (0, 0, 0)\n"
"        idx = 40*i+20 #center of colorbars\n"
"        for off in range(0, 10): #avg 10 pixels\n"
"            rgb = image.get_pixel(idx+off, 120)\n"
"            avg = tuple(map(sum, zip(avg, rgb)))\n"
"\n"
"        if not t[i](avg[0]/10, avg[1]/10, avg[2]/10):\n"
"            raise Exception('COLOR BARS TEST FAILED.'\n"
"            'BAR#(%d): RGB(%d,%d,%d)'%(i+1, avg[0]/10, avg[1]/10, avg[2]/10))\n"
"\n"
"    print('COLOR BARS TEST PASSED...')\n"
"\n"
"if __name__ == '__main__':\n"
"    print('')\n"
"    test_int_adc()\n"
"    test_color_bars()\n"
;
#endif

void flash_error(int n) {
    for (int i = 0; i < n; i++) {
        led_state(LED_RED, 0);
        HAL_Delay(100);
        led_state(LED_RED, 1);
        HAL_Delay(100);
    }
    led_state(LED_RED, 0);
}

void NORETURN __fatal_error(const char *msg) {
    FIL fp;
    if (f_open(&fp, "ERROR.LOG",
               FA_WRITE|FA_CREATE_ALWAYS) == FR_OK) {
        f_printf(&fp, "\nFATAL ERROR:\n%s\n", msg);
    }
    f_close(&fp);
    storage_flush();

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
void __attribute__((weak))
    __assert_func(const char *file, int line, const char *func, const char *expr) {
    (void)func;
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("");
}
#endif

STATIC mp_obj_t pyb_config_source_dir = MP_OBJ_NULL;
STATIC mp_obj_t pyb_config_main = MP_OBJ_NULL;

STATIC mp_obj_t pyb_source_dir(mp_obj_t source_dir) {
    if (MP_OBJ_IS_STR(source_dir)) {
        pyb_config_source_dir = source_dir;
    }
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_1(pyb_source_dir_obj, pyb_source_dir);

STATIC mp_obj_t pyb_main(mp_obj_t main) {
    if (MP_OBJ_IS_STR(main)) {
        pyb_config_main = main;
    }
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_1(pyb_main_obj, pyb_main);

static void make_flash_fs()
{
    FIL fp;
    UINT n;

    led_state(LED_RED, 1);

    if (f_mkfs("0:", 0, 0) != FR_OK) {
        __fatal_error("could not create LFS");
    }

    // create default main.py
    f_open(&fp, "main.py", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
    f_close(&fp);

    // create .inf driver file
    f_open(&fp, "openmv.inf", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_openmv_inf, sizeof(fresh_openmv_inf) - 1 /* don't count null terminator */, &n);
    f_close(&fp);

    // create readme file
    f_open(&fp, "README.txt", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_readme_txt, sizeof(fresh_readme_txt) - 1 /* don't count null terminator */, &n);
    f_close(&fp);

    // create default selftest.py
    f_open(&fp, "selftest.py", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_selftest_py, sizeof(fresh_selftest_py) - 1 /* don't count null terminator */, &n);
    f_close(&fp);

    led_state(LED_RED, 0);
}

#ifdef STACK_PROTECTOR
uint32_t __stack_chk_guard=0xDEADBEEF;

void NORETURN __stack_chk_fail(void)
{
    while (1) {
        flash_error(100);
    }
}
#endif

int main(void)
{
    FRESULT f_res;
    int sensor_init_ret = 0;
    bool first_soft_reset = true;

    // Uncomment to disable write buffer to get precise faults.
    // NOTE: Cache should be disabled on M7.
    //SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;

    // STM32F4xx HAL library initialization:
    //  - Set NVIC Group Priority to 4
    //  - Configure the Flash prefetch, instruction and Data caches
    //  - Configure the Systick to generate an interrupt each 1 msec
    //  Note: The bootloader enables the CCM/DTCM memory.
    HAL_Init();

    // Stack limit should be less than real stack size, so we have a chance
    // to recover from limit hit.  (Limit is measured in bytes.)
    mp_stack_ctrl_init();
    mp_stack_set_limit((char*)&_ram_end - (char*)&_heap_end - 1024);

    // basic sub-system init
    led_init();
    pendsv_init();

    // Re-enable IRQs (disabled by bootloader)
    __enable_irq();

soft_reset:
    led_state(LED_IR, 0);
    led_state(LED_RED, 1);
    led_state(LED_GREEN, 1);
    led_state(LED_BLUE, 1);

    // GC init
    gc_init(&_heap_start, &_heap_end);

    // Micro Python init
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_init(mp_sys_argv, 0);

    // zero out the pointers to the mounted devices
    memset(MP_STATE_PORT(fs_user_mount), 0, sizeof(MP_STATE_PORT(fs_user_mount)));

    // Initialise low-level sub-systems.  Here we need to very basic things like
    // zeroing out memory and resetting any of the sub-systems.  Following this
    // we can run Python scripts (eg main.py).

    readline_init0();
    pin_init0();
    extint_init0();
    timer_init0();
    rng_init0();
    i2c_init0();
    spi_init0();
    uart_init0();
    dac_init();
    pyb_usb_init0();
    sensor_init0();
    fb_alloc_init0();
    file_buffer_init0();
    py_lcd_init0();
    py_fir_init0();

#if MICROPY_HW_ENABLE_RTC
    if (first_soft_reset) {
        rtc_init();
    }
#endif

    // Initialize the sensor and check the result after
    // mounting the file-system to log errors (if any).
    if (first_soft_reset) {
        sensor_init_ret = sensor_init();
    }

    servo_init();
    usbdbg_init();
    mod_network_init();

    // Remove the BASEPRI masking (if any)
    irq_set_base_priority(0);

    // Initialize storage
    if (sdcard_is_present()) {
        sdcard_init();
        // init the vfs object
        vfs->str = "1:";
        vfs->len = 2;
        vfs->flags = 0;
        sdcard_init_vfs(vfs);

        // put the sdcard device in slot 1 (it will be unused at this point)
        MP_STATE_PORT(fs_user_mount)[1] = vfs;

        // try to mount the flash
        FRESULT res = f_mount(&vfs->fatfs, vfs->str, 1);
        if (res != FR_OK) {
            __fatal_error("could not mount SD\n");
        }
        // Set CWD and USB medium to SD
        f_chdrive("1:");
        pyb_usb_storage_medium = PYB_USB_STORAGE_MEDIUM_SDCARD;
    } else {
        storage_init();

        // init the vfs object
        vfs->str = "0:";
        vfs->len = 2;
        vfs->flags = 0;
        pyb_flash_init_vfs(vfs);

        // put the flash device in slot 0 (it will be unused at this point)
        MP_STATE_PORT(fs_user_mount)[0] = vfs;

        // try to mount the flash
        FRESULT res = f_mount(&vfs->fatfs, vfs->str, 1);

        if (res == FR_NO_FILESYSTEM) {
            // create a fresh fs
            make_flash_fs();
        } else if (res != FR_OK) {
            __fatal_error("could not access LFS\n");
        }

        // Set CWD and USB medium to flash
        f_chdrive("0:");
        pyb_usb_storage_medium = PYB_USB_STORAGE_MEDIUM_FLASH;
    }

    // turn boot-up LEDs off
    led_state(LED_RED, 0);
    led_state(LED_GREEN, 0);
    led_state(LED_BLUE, 0);

    // init USB device to default setting if it was not already configured
    if (!(pyb_usb_flags & PYB_USB_FLAG_USB_MODE_CALLED)) {
        pyb_usb_dev_init(USBD_VID, USBD_PID_CDC_MSC, USBD_MODE_CDC_MSC, NULL);
    }

    // check sensor init result
    if (first_soft_reset && sensor_init_ret != 0) {
        char buf[512];
        snprintf(buf, sizeof(buf), "Failed to init sensor, error:%d", sensor_init_ret);
        __fatal_error(buf);
    }

    // Run self tests the first time only
    f_res = f_stat("selftest.py", NULL);
    if (first_soft_reset && f_res == FR_OK) {
        nlr_buf_t nlr;
        if (nlr_push(&nlr) == 0) {
            // Parse, compile and execute the self-tests script.
            pyexec_file("selftest.py");
            nlr_pop();
        } else {
            // Get the exception message. TODO: might be a hack.
            mp_obj_str_t *str = mp_obj_exception_get_value((mp_obj_t)nlr.ret_val);
            // If any of the self-tests fail log the exception message
            // and loop forever. Note: IDE exceptions will not be caught.
            __fatal_error((const char*) str->data);
        }
        // Success: remove self tests script and flush cache
        f_unlink("selftest.py");
        storage_flush();
    }

    // Run the main script from the current directory.
    f_res = f_stat("main.py", NULL);
    if (first_soft_reset && f_res == FR_OK) {
        nlr_buf_t nlr;
        if (nlr_push(&nlr) == 0) {
            // Parse, compile and execute the main script.
            pyexec_file("main.py");
            nlr_pop();
        } else {
            mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
            if (nlr_push(&nlr) == 0) {
                flash_error(3);
                nlr_pop();
            }// if this gets interrupted again ignore it.
        }
    }

    // If there's no script ready, just re-exec REPL
    while (!usbdbg_script_ready()) {
        nlr_buf_t nlr;

        if (nlr_push(&nlr) == 0) {
            // enable IDE interrupt
            usbdbg_set_irq_enabled(true);

            // run REPL
            pyexec_friendly_repl();

            nlr_pop();
        }
    }

    if (usbdbg_script_ready()) {
        nlr_buf_t nlr;

        // execute the script
        if (nlr_push(&nlr) == 0) {
            // enable IDE interrupt
            usbdbg_set_irq_enabled(true);

            pyexec_str(usbdbg_get_script());
            nlr_pop();
        } else {
            mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
        }
    }

    // Disable all other IRQs except Systick and Flash IRQs
    // Note: FS IRQ is disable, since we're going for a soft-reset.
    irq_set_base_priority(IRQ_PRI_FLASH+1);

    // soft reset
    storage_flush();
    timer_deinit();
    uart_deinit();

    first_soft_reset = false;
    goto soft_reset;

}
