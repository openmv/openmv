/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * main function.
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
#include "timer.h"
#include "pin.h"
#include "usb.h"
#include "rtc.h"
#include "storage.h"
#include "sdcard.h"
#include "ff.h"
#include "modnetwork.h"
#include "modmachine.h"

#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#include "shared/runtime/pyexec.h"
#include "shared/readline/readline.h"

#include "irq.h"
#include "rng.h"
#include "led.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "dac.h"
#include "can.h"
#include "extint.h"
#include "servo.h"

#include "cambus.h"
#include "sensor.h"
#include "usbdbg.h"
#include "wifidbg.h"
#include "sdram.h"
#include "fb_alloc.h"
#include "dma_alloc.h"
#include "ff_wrapper.h"

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc_msc_hid.h"
#include "usbd_cdc_interface.h"

#include "py_image.h"
#include "py_lcd.h"
#include "py_fir.h"
#include "py_tv.h"
#include "py_buzzer.h"
#include "py_imu.h"
#include "py_audio.h"

#include "framebuffer.h"

#include "ini.h"
#include "omv_boardconfig.h"

#if MICROPY_PY_LWIP
#include "lwip/init.h"
#include "lwip/apps/mdns.h"
#include "drivers/cyw43/cyw43.h"
#endif

#if MICROPY_PY_BLUETOOTH
#include "extmod/modbluetooth.h"
#include "mpbthciport.h"
#endif

int errno;
extern char _vfs_buf[];
static fs_user_mount_t *vfs_fat = (fs_user_mount_t *) &_vfs_buf;
#if MICROPY_PY_THREAD
pyb_thread_t pyb_thread_main;
#endif

static const char fresh_main_py[] =
"# main.py -- put your code here!\n"
"import pyb, time\n"
"led = pyb.LED(3)\n"
"usb = pyb.USB_VCP()\n"
"while (usb.isconnected()==False):\n"
"   led.on()\n"
"   time.sleep_ms(150)\n"
"   led.off()\n"
"   time.sleep_ms(100)\n"
"   led.on()\n"
"   time.sleep_ms(150)\n"
"   led.off()\n"
"   time.sleep_ms(600)\n"
;

static const char fresh_readme_txt[] =
"Thank you for supporting the OpenMV project!\r\n"
"\r\n"
"To download the IDE, please visit:\r\n"
"https://openmv.io/pages/download\r\n"
"\r\n"
"For tutorials and documentation, please visit:\r\n"
"http://docs.openmv.io/\r\n"
"\r\n"
"For technical support and projects, please visit the forums:\r\n"
"http://forums.openmv.io/\r\n"
"\r\n"
"Please use github to report bugs and issues:\r\n"
"https://github.com/openmv/openmv\r\n"
;

#if (OMV_ENABLE_SELFTEST == 1)
static const char fresh_selftest_py[] =
"import sensor, time, pyb\n"
"\n"
"def test_int_adc():\n"
"    adc  = pyb.ADCAll(12)\n"
"    # Test VBAT\n"
"    vbat = adc.read_core_vbat()\n"
"    vbat_diff = abs(vbat-"OMV_CORE_VBAT")\n"
"    if (vbat_diff > 0.15):\n"
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
"    if sensor.get_id() == sensor.OV7725: test_color_bars()\n"
"\n"
;
#endif

void flash_error(int n) {
    led_state(LED_RED, 0);
    led_state(LED_GREEN, 0);
    led_state(LED_BLUE, 0);
    for (int i = 0; i < n; i++) {
        led_state(LED_RED, 0);
        HAL_Delay(100);
        led_state(LED_RED, 1);
        HAL_Delay(100);
    }
    led_state(LED_RED, 0);
}

void NORETURN __fatal_error(const char *msg) {
    // Check if any storage has been initialized
    // before attempting to create the error log.
    if (pyb_usb_storage_medium) {
        FIL fp;
        if (f_open(&vfs_fat->fatfs, &fp, "ERROR.LOG",
                    FA_WRITE|FA_CREATE_ALWAYS) == FR_OK) {
            UINT bytes;
            const char *hdr = "FATAL ERROR:\n";
            f_write(&fp, hdr, strlen(hdr), &bytes);
            f_write(&fp, msg, strlen(msg), &bytes);
            f_close(&fp);
            storage_flush();
            // Initialize the USB device if it's not already initialize to allow
            // the host to mount the filesystem and access the error log.
            pyb_usb_dev_init(pyb_usb_dev_detect(), MICROPY_HW_USB_VID,
                    MICROPY_HW_USB_PID_CDC_MSC, USBD_MODE_CDC_MSC, 0, NULL, NULL);
        }
    }
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

void f_touch(const char *path)
{
    FIL fp;
    if (f_stat(&vfs_fat->fatfs, path, NULL) != FR_OK) {
        f_open(&vfs_fat->fatfs, &fp, path, FA_WRITE | FA_CREATE_ALWAYS);
        f_close(&fp);
    }
}

void make_flash_fs()
{
    FIL fp;
    UINT n;

    led_state(LED_RED, 1);

    uint8_t working_buf[_MAX_SS];
    if (f_mkfs(&vfs_fat->fatfs, FM_FAT, 0, working_buf, sizeof(working_buf)) != FR_OK) {
        __fatal_error("Could not create LFS");
    }

    // Mark FS as OpenMV disk.
    f_touch("/.openmv_disk");

    // Create default main.py
    f_open(&vfs_fat->fatfs, &fp, "/main.py", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
    f_close(&fp);

    // Create readme file
    f_open(&vfs_fat->fatfs, &fp, "/README.txt", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_readme_txt, sizeof(fresh_readme_txt) - 1 /* don't count null terminator */, &n);
    f_close(&fp);

    #if (OMV_ENABLE_SELFTEST == 1)
    // Create default selftest.py
    f_open(&vfs_fat->fatfs, &fp, "/selftest.py", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_selftest_py, sizeof(fresh_selftest_py) - 1 /* don't count null terminator */, &n);
    f_close(&fp);
    #endif

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

typedef struct openmv_config {
    bool wifidbg;
    wifidbg_config_t wifidbg_config;
} openmv_config_t;

int ini_handler_callback(void *user, const char *section, const char *name, const char *value)
{
    openmv_config_t *openmv_config = (openmv_config_t *) user;

    #define MATCH(s, n) ((strcmp(section, (s)) == 0) && (strcmp(name, (n)) == 0))

    if (MATCH("BoardConfig", "REPLUart")) {
        if (ini_is_true(value)) {
            mp_obj_t args[2] = {
                MP_OBJ_NEW_SMALL_INT(3), // UART Port
                MP_OBJ_NEW_SMALL_INT(115200) // Baud Rate
            };

            MP_STATE_PORT(pyb_stdio_uart) = pyb_uart_type.make_new((mp_obj_t) &pyb_uart_type, MP_ARRAY_SIZE(args), 0, args);
            uart_attach_to_repl(MP_STATE_PORT(pyb_stdio_uart), true);
        }
    } else if (MATCH("BoardConfig", "WiFiDebug")) {
        openmv_config->wifidbg = ini_is_true(value);
    } else if (MATCH("WiFiConfig", "Mode")) {
        openmv_config->wifidbg_config.mode = ini_atoi(value);
    } else if (MATCH("WiFiConfig", "ClientSSID")) {
        strncpy(openmv_config->wifidbg_config.client_ssid, value, WINC_MAX_SSID_LEN);
    } else if (MATCH("WiFiConfig", "ClientKey")) {
        strncpy(openmv_config->wifidbg_config.client_key,  value, WINC_MAX_PSK_LEN);
    } else if (MATCH("WiFiConfig", "ClientSecurity")) {
        openmv_config->wifidbg_config.client_security = ini_atoi(value);
    } else if (MATCH("WiFiConfig", "ClientChannel")) {
        openmv_config->wifidbg_config.client_channel = ini_atoi(value);
    } else if (MATCH("WiFiConfig", "AccessPointSSID")) {
        strncpy(openmv_config->wifidbg_config.access_point_ssid, value, WINC_MAX_SSID_LEN);
    } else if (MATCH("WiFiConfig", "AccessPointKey")) {
        strncpy(openmv_config->wifidbg_config.access_point_key,  value, WINC_MAX_PSK_LEN);
    } else if (MATCH("WiFiConfig", "AccessPointSecurity")) {
        openmv_config->wifidbg_config.access_point_security = ini_atoi(value);
    } else if (MATCH("WiFiConfig", "AccessPointChannel")) {
        openmv_config->wifidbg_config.access_point_channel = ini_atoi(value);
    } else if (MATCH("WiFiConfig", "BoardName")) {
        strncpy(openmv_config->wifidbg_config.board_name,  value, WINC_MAX_BOARD_NAME_LEN);
    } else {
        return 0;
    }

    return 1;

    #undef MATCH
}

FRESULT exec_boot_script(const char *path, bool selftest, bool interruptible, bool wifidbg_enabled)
{
    nlr_buf_t nlr;
    bool interrupted = false;
    FRESULT f_res = FR_NO_FILE;

    if (nlr_push(&nlr) == 0) {
        // Enable IDE interrupts if allowed.
        if (interruptible) {
            usbdbg_set_irq_enabled(true);
            usbdbg_set_script_running(true);
            #if OMV_ENABLE_WIFIDBG && MICROPY_PY_WINC1500
            wifidbg_set_irq_enabled(wifidbg_enabled);
            #endif
        }

        // Try to run the frozen module first.
        if (pyexec_frozen_module(path, true) == false) {
            // No frozen module, try the filesystem.
            f_res = f_stat(&vfs_fat->fatfs, path, NULL);
            if (f_res == FR_OK) {
                // Parse, compile and execute the script.
                pyexec_file(path, true);
            }
        }
        nlr_pop();
    } else {
        interrupted = true;
    }

    // Disable IDE interrupts
    usbdbg_set_irq_enabled(false);
    usbdbg_set_script_running(false);
    #if OMV_ENABLE_WIFIDBG && MICROPY_PY_WINC1500
    wifidbg_set_irq_enabled(false);
    #endif

    if (interrupted) {
        if (selftest) {
            // Get the exception message. TODO: might be a hack.
            mp_obj_str_t *str = mp_obj_exception_get_value((mp_obj_t)nlr.ret_val);
            // If any of the self-tests fail log the exception message
            // and loop forever. Note: IDE exceptions will not be caught.
            __fatal_error((const char*) str->data);
        } else {
            mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
            if (nlr_push(&nlr) == 0) {
                flash_error(3);
                nlr_pop();
            }// If this gets interrupted again ignore it.
        }
    }

    if (selftest && f_res == FR_OK) {
        // Remove self tests script and flush cache
        f_unlink(&vfs_fat->fatfs, path);
        storage_flush();

        #ifdef OMV_SELF_TEST_SWD_ADDR
        // Set flag for SWD debugger.
        // Note: main.py does not use the frame buffer.
        OMV_SELF_TEST_SWD_ADDR = 0xDEADBEEF;
        #endif
    }

    return f_res;
}

int main(void)
{
    #if MICROPY_HW_SDRAM_SIZE
    bool sdram_ok = false;
    #endif
    #if MICROPY_HW_ENABLE_SDCARD
    bool sdcard_mounted = false;
    #endif
    bool first_soft_reset = true;

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

    // Basic sub-system init
    led_init();
    pendsv_init();
    #if MICROPY_PY_THREAD
    pyb_thread_init(&pyb_thread_main);
    #endif

    // Re-enable IRQs (disabled by bootloader)
    __enable_irq();

soft_reset:
    #if defined(MICROPY_HW_LED4)
    led_state(LED_IR, 0);
    #endif
    led_state(LED_RED, 1);
    led_state(LED_GREEN, 1);
    led_state(LED_BLUE, 1);

    machine_init();

    #if MICROPY_PY_THREAD
    // Python threading init
    mp_thread_init();
    #endif

    // Stack limit should be less than real stack size, so we have a
    // chance to recover from limit hit. (Limit is measured in bytes)
    mp_stack_set_top(&_estack);
    mp_stack_set_limit((char*)&_estack - (char*)&_sstack - 1024);

    // GC init
    gc_init(&_heap_start, &_heap_end);

    #if MICROPY_ENABLE_PYSTACK
    static mp_obj_t pystack[384];
    mp_pystack_init(pystack, &pystack[384]);
    #endif

    // Micro Python init
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_init(mp_sys_argv, 0);

    // Initialise low-level sub-systems. Here we need to do the very basic
    // things like zeroing out memory and resetting any of the sub-systems.
    #if MICROPY_PY_LCD
    py_lcd_init0();
    #endif
    py_fir_init0();
    #if MICROPY_PY_TV
    py_tv_init0();
    #endif
    #if MICROPY_PY_BUZZER
    py_buzzer_init0();
    #endif // MICROPY_PY_BUZZER
    imlib_init_all();
    readline_init0();
    pin_init0();
    extint_init0();
    timer_init0();
    #if MICROPY_HW_ENABLE_CAN
    can_init0();
    #endif
    i2c_init0();
    spi_init0();
    uart_init0();
    fb_alloc_init0();
    framebuffer_init0();
    sensor_init0();
    dma_alloc_init0();
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
        mp_hal_get_mac_ascii(MP_HAL_MAC_WLAN0, 8, 4, (char *)&buf[4]);
        cyw43_wifi_ap_set_ssid(&cyw43_state, 8, buf);
        cyw43_wifi_ap_set_password(&cyw43_state, 8, (const uint8_t *)"pybd0123");
    }
    #endif

    pyb_usb_init0();
    MP_STATE_PORT(pyb_stdio_uart) = NULL;

    // Initialize the sensor and check the result after
    // mounting the file-system to log errors (if any).
    if (first_soft_reset) {
        sensor_init();
        #if MICROPY_PY_IMU
        py_imu_init();
        #endif // MICROPY_PY_IMU
    }

    mod_network_init();

    // Remove the BASEPRI masking (if any)
    irq_set_base_priority(0);

    #if MICROPY_HW_ENABLE_SDCARD
    // Initialize storage
    if (sdcard_is_present()) {
        // Init the vfs object
        vfs_fat->blockdev.flags = 0;
        sdcard_init_vfs(vfs_fat, 0);

        // Try to mount the SD card
        FRESULT res = f_mount(&vfs_fat->fatfs);
        if (res != FR_OK) {
            sdcard_mounted = false;
        } else {
            sdcard_mounted = true;
            // Set USB medium to SD
            pyb_usb_storage_medium = PYB_USB_STORAGE_MEDIUM_SDCARD;
        }
    }
    #endif

    #if MICROPY_HW_ENABLE_SDCARD
    if (sdcard_mounted == false) {
    #endif
        storage_init();

        // init the vfs object
        vfs_fat->blockdev.flags = 0;
        pyb_flash_init_vfs(vfs_fat);

        // Try to mount the flash
        FRESULT res = f_mount(&vfs_fat->fatfs);

        if (res == FR_NO_FILESYSTEM) {
            // Create a fresh fs
            make_flash_fs();
            // Flush storage
            storage_flush();
        } else if (res != FR_OK) {
            __fatal_error("Could not access LFS\n");
        }

        // Set USB medium to flash
        pyb_usb_storage_medium = PYB_USB_STORAGE_MEDIUM_FLASH;
    #if MICROPY_HW_ENABLE_SDCARD
    }
    #if MICROPY_HW_HAS_FLASH
    else {
        // The storage should always be initialized on boards that have
        // an external flash, to make sure the flash is memory-mapped.
        storage_init();
    }
    #endif
    #endif

    // Mark FS as OpenMV disk.
    f_touch("/.openmv_disk");

    // Mount the storage device (there should be no other devices mounted at this point)
    // we allocate this structure on the heap because vfs->next is a root pointer.
    mp_vfs_mount_t *vfs = m_new_obj_maybe(mp_vfs_mount_t);
    if (vfs == NULL) {
        __fatal_error("Failed to alloc memory for vfs mount\n");
    }

    vfs->str = "/";
    vfs->len = 1;
    vfs->obj = MP_OBJ_FROM_PTR(vfs_fat);
    vfs->next = NULL;
    MP_STATE_VM(vfs_mount_table) = vfs;
    MP_STATE_PORT(vfs_cur) = vfs;

    // Parse OpenMV configuration file.
    openmv_config_t openmv_config;
    memset(&openmv_config, 0, sizeof(openmv_config));
    ini_parse(&vfs_fat->fatfs, "/openmv.config", ini_handler_callback, &openmv_config);

    // Init wifi debugging if enabled and on first soft-reset only.
    #if OMV_ENABLE_WIFIDBG && MICROPY_PY_WINC1500
    if (openmv_config.wifidbg == true &&
            wifidbg_init(&openmv_config.wifidbg_config) != 0) {
        openmv_config.wifidbg = false;
    }
    #else
        openmv_config.wifidbg = false;
    #endif

    // Execute frozen _boot.py (if any) for early system setup.
    pyexec_frozen_module("_boot.py", false);

    // Run boot script(s)
    if (first_soft_reset) {
        // Execute the boot.py script before initializing the USB dev to
        // override the USB mode if required, otherwise VCP+MSC is used.
        exec_boot_script("boot.py", false, false, false);
        #if (OMV_ENABLE_SELFTEST == 1)
        // Execute the selftests.py script before the filesystem is mounted
        // to avoid corrupting the filesystem when selftests.py is removed.
        exec_boot_script("selftest.py", true, false, false);
        #endif
    }

    // Init USB device to default setting if it was not already configured
    if (!(pyb_usb_flags & PYB_USB_FLAG_USB_MODE_CALLED)) {
        pyb_usb_dev_init(pyb_usb_dev_detect(), MICROPY_HW_USB_VID,
                MICROPY_HW_USB_PID_CDC_MSC, USBD_MODE_CDC_MSC, 0, NULL, NULL);
    }

    // report if SDRAM failed
    #if MICROPY_HW_SDRAM_SIZE
    if (first_soft_reset && (!sdram_ok)) {
        char buf[512];
        snprintf(buf, sizeof(buf), "Failed to init sdram!");
        __fatal_error(buf);
    }
    #endif

    // Turn boot-up LEDs off
    led_state(LED_RED, 0);
    led_state(LED_GREEN, 0);
    led_state(LED_BLUE, 0);

    // Run main script if it exists.
    if (first_soft_reset) {
        exec_boot_script("main.py", false, true, openmv_config.wifidbg);
    }

    do {
        usbdbg_init();

        if (openmv_config.wifidbg == true) {
            // Need to reinit imlib in WiFi debug mode.
            imlib_deinit_all();
            imlib_init_all();
        }

        // If there's no script ready, just re-exec REPL
        while (!usbdbg_script_ready()) {
            nlr_buf_t nlr;

            if (nlr_push(&nlr) == 0) {
                // enable IDE interrupt
                usbdbg_set_irq_enabled(true);
                #if OMV_ENABLE_WIFIDBG && MICROPY_PY_WINC1500
                wifidbg_set_irq_enabled(openmv_config.wifidbg);
                #endif

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
                #if OMV_ENABLE_WIFIDBG && MICROPY_PY_WINC1500
                wifidbg_set_irq_enabled(openmv_config.wifidbg);
                #endif

                // Execute the script.
                pyexec_str(usbdbg_get_script(), true);
                nlr_pop();
            } else {
                mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
            }
        }
    } while (openmv_config.wifidbg == true);

    usbdbg_wait_for_command(1000);

    #if MICROPY_PY_LWIP
    // Must call GC sweep here to close open sockets.
    gc_sweep_all();
    systick_disable_dispatch(SYSTICK_DISPATCH_LWIP);
    #endif

    // soft reset
    storage_flush();

    // Disable all other IRQs except Systick
    irq_set_base_priority(IRQ_PRI_SYSTICK+1);

    #if MICROPY_PY_BLUETOOTH
    mp_bluetooth_deinit();
    #endif
    mod_network_deinit();
    timer_deinit();
    i2c_deinit_all();
    spi_deinit_all();
    uart_deinit_all();
    #if MICROPY_HW_ENABLE_CAN
    can_deinit_all();
    #endif
    #if MICROPY_PY_THREAD
    pyb_thread_deinit();
    #endif
    #if MICROPY_PY_AUDIO
    py_audio_deinit();
    #endif
    imlib_deinit_all();
    first_soft_reset = false;
    goto soft_reset;
}
