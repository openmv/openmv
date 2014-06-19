#include <stdio.h>
#include <string.h>
#include <stm32f4xx_hal.h>
#include "misc.h"
#include "systick.h"
#include "pendsv.h"
#include "mpconfig.h"
#include "qstr.h"
#include "misc.h"
#include "nlr.h"
#include "lexer.h"
#include "parse.h"
#include "obj.h"
#include "objmodule.h"
#include "runtime.h"
#include "gc.h"
#include "gccollect.h"
#include "pybstdio.h"
#include "readline.h"
#include "pyexec.h"
#include "uart.h"
#include "timer.h"
#include "pin.h"
#include "extint.h"
#include "usb.h"
#include "rtc.h"
#include "storage.h"
#include "sdcard.h"
#include "ff.h"
#include "mdefs.h"

#include "led.h"
#include "rng.h"
#include "sensor.h"
#include "usbdbg.h"

#include "py_led.h"
#include "py_time.h"
#include "py_sensor.h"
#include "py_image.h"

int errno;
static FATFS fatfs0;
//static FATFS fatfs1;

void SystemClock_Config(void);

void flash_error(int n) {
    for (int i = 0; i < n; i++) {
        led_state(LED_RED, 0);
        HAL_Delay(250);
        led_state(LED_RED, 1);
        HAL_Delay(250);
    }
    led_state(LED_RED, 0);
}

void __fatal_error(const char *msg) {
    stdout_tx_strn("\nFATAL ERROR:\n", 14);
    stdout_tx_strn(msg, strlen(msg));
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
STATIC mp_obj_t pyb_config_usb_mode = MP_OBJ_NULL;

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

STATIC mp_obj_t pyb_usb_mode(mp_obj_t usb_mode) {
    if (MP_OBJ_IS_STR(usb_mode)) {
        pyb_config_usb_mode = usb_mode;
    }
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_1(pyb_usb_mode_obj, pyb_usb_mode);

STATIC mp_obj_t py_vcp_is_connected(void ) {
    return MP_BOOL(usb_vcp_is_connected());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_vcp_is_connected_obj, py_vcp_is_connected);

static const char fresh_main_py[] =
"# main.py -- put your code here!\n"
"import led, sensor\n"
"led.on(led.BLUE)\n"
"sensor.set_pixformat(sensor.RGB565)\n"
"while (vcp_is_connected()==False):\n"
"  image = sensor.snapshot()\n"
;

static const char fresh_pybcdc_inf[] =
#include "genhdr/pybcdc_inf.h"
;

static const char fresh_readme_txt[] =
"This is a Micro Python board\r\n"
"\r\n"
"You can get started right away by writing your Python code in 'main.py'.\r\n"
"\r\n"
"For a serial prompt:\r\n"
" - Windows: you need to go to 'Device manager', right click on the unknown device,\r\n"
"   then update the driver software, using the 'pybcdc.inf' file found on this drive.\r\n"
"   Then use a terminal program like Hyperterminal or putty.\r\n"
" - Mac OS X: use the command: screen /dev/tty.usbmodem*\r\n"
" - Linux: use the command: screen /dev/ttyACM0\r\n"
"\r\n"
"Please visit http://micropython.org/help/ for further help.\r\n"
;

typedef struct {
    qstr name;
    const mp_obj_module_t *(*init)(void);
} module_t;

static const module_t exported_modules[] ={
    {MP_QSTR_sensor,py_sensor_init},
    {MP_QSTR_led,   py_led_init},
    {MP_QSTR_time,  py_time_init},
//    {MP_QSTR_gpio,  py_gpio_init},
//    {MP_QSTR_spi,   py_spi_init},
    {0, NULL}
};

int main(void) {
    /* STM32F4xx HAL library initialization:
         - Configure the Flash prefetch, instruction and Data caches
         - Configure the Systick to generate an interrupt each 1 msec
         - Set NVIC Group Priority to 4
         - Global MSP (MCU Support Package) initialization
       */
    HAL_Init();

    // set the system clock to be HSE
    SystemClock_Config();

    // enable GPIO clocks
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();

    // enable the CCM RAM
    __CCMDATARAMEN_CLK_ENABLE();

    // basic sub-system init
    pendsv_init();
    timer_tim3_init();
    int first_soft_reset = true;

soft_reset:
    // check if user switch held to select the reset mode
    led_state(LED_RED, 1);
    led_state(LED_GREEN, 1);
    led_state(LED_BLUE, 1);
    uint reset_mode = 1;

#if MICROPY_HW_ENABLE_RTC
    if (first_soft_reset) {
        rtc_init();
    }
#endif

    // more sub-system init
#if MICROPY_HW_HAS_SDCARD
    if (first_soft_reset) {
        sdcard_init();
    }
#endif

    if (first_soft_reset) {
        storage_init();
    }

    // GC init
    gc_init(&_heap_start, &_heap_end);

#if 0
    // Change #if 0 to #if 1 if you want REPL on UART_6 (or another uart)
    // as well as on USB VCP
    mp_obj_t args[2] = {
        MP_OBJ_NEW_SMALL_INT(PYB_UART_6),
        MP_OBJ_NEW_SMALL_INT(115200),
    };
    pyb_uart_global_debug = pyb_uart_type.make_new((mp_obj_t)&pyb_uart_type,
                                                   ARRAY_SIZE(args),
                                                   0, args);
#else
    pyb_uart_global_debug = NULL;
#endif

    // Micro Python init
    qstr_init();
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_0_colon__slash_));
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_0_colon__slash_lib));
    mp_obj_list_init(mp_sys_argv, 0);

    readline_init();

    pin_init();
    extint_init();

    // local filesystem init
    // try to mount the flash
    FRESULT res = f_mount(&fatfs0, "0:", 1);
    if (reset_mode == 3 || res == FR_NO_FILESYSTEM) {
        // no filesystem, or asked to reset it, so create a fresh one

        // LED on to indicate creation of LFS
        led_state(LED_RED, 1);

        res = f_mkfs("0:", 0, 0);
        if (res == FR_OK) {
            // success creating fresh LFS
        } else {
            __fatal_error("could not create LFS");
        }

        // create empty main.py
        FIL fp;
        f_open(&fp, "0:/main.py", FA_WRITE | FA_CREATE_ALWAYS);
        UINT n;
        f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
        // TODO check we could write n bytes
        f_close(&fp);

        // create .inf driver file
        f_open(&fp, "0:/pybcdc.inf", FA_WRITE | FA_CREATE_ALWAYS);
        f_write(&fp, fresh_pybcdc_inf, sizeof(fresh_pybcdc_inf) - 1 /* don't count null terminator */, &n);
        f_close(&fp);

        // create readme file
        f_open(&fp, "0:/README.txt", FA_WRITE | FA_CREATE_ALWAYS);
        f_write(&fp, fresh_readme_txt, sizeof(fresh_readme_txt) - 1 /* don't count null terminator */, &n);
        f_close(&fp);

        // keep LED on for at least 200ms
        led_state(LED_RED, 0);
    } else if (res == FR_OK) {
        // mount sucessful
    } else {
        __fatal_error("could not access LFS");
    }

    // root device defaults to internal flash filesystem
    uint root_device = 0;

#if defined(USE_DEVICE_MODE)
    usb_storage_medium_t usb_medium = USB_STORAGE_MEDIUM_FLASH;
#endif

#if MICROPY_HW_HAS_SDCARD
    /* prepare workarea for sdcard fs */
    //f_mount(&fatfs1, "1:", 0);

    // if an SD card is present then mount it on 1:/
    if (reset_mode == 1 && sdcard_is_present()) {
        FRESULT res = f_mount(&fatfs1, "1:", 1);
        if (res != FR_OK) {
            printf("[SD] could not mount SD card\n");
        } else {
            // use SD card as root device
            root_device = 1;

            if (first_soft_reset) {
                // use SD card as medium for the USB MSD
#if defined(USE_DEVICE_MODE)
                usb_medium = USB_STORAGE_MEDIUM_SDCARD;
#endif
            }
        }
    }
#else
    // Get rid of compiler warning if no SDCARD is configured.
    (void)first_soft_reset;
#endif

    // turn boot-up LEDs off
    led_state(LED_RED, 0);
    led_state(LED_GREEN, 0);
    led_state(LED_BLUE, 0);

#if defined(USE_HOST_MODE)
    // USB host
    pyb_usb_host_init();
#elif defined(USE_DEVICE_MODE)
    // USB device
    if (reset_mode == 1) {
        usb_device_mode_t usb_mode = USB_DEVICE_MODE_CDC_MSC;
        if (pyb_config_usb_mode != MP_OBJ_NULL) {
            if (strcmp(mp_obj_str_get_str(pyb_config_usb_mode), "CDC+HID") == 0) {
                usb_mode = USB_DEVICE_MODE_CDC_HID;
            }
        }
        pyb_usb_dev_init(usb_mode, usb_medium);
    } else {
        pyb_usb_dev_init(USB_DEVICE_MODE_CDC_MSC, usb_medium);
    }
#endif

    timer_init0();

    rng_init();
    usbdbg_init();

    /* Add functions to the global python namespace */
//    mp_store_global(qstr_from_str("open"), mp_make_function_n(2, py_file_open));
    mp_store_global(qstr_from_str("vcp_is_connected"), mp_make_function_n(0, py_vcp_is_connected));
//    mp_store_global(qstr_from_str("info"), mp_make_function_n(0, py_info));
//    mp_store_global(qstr_from_str("gc_collect"), mp_make_function_n(0, py_gc_collect));
    mp_store_global(qstr_from_str("Image"), mp_make_function_n(1, py_image_load_image));
    mp_store_global(qstr_from_str("HaarCascade"), mp_make_function_n(1, py_image_load_cascade));

    /* Export Python modules to the global python namespace */
    for (const module_t *p = exported_modules; p->name; p++) {
        const mp_obj_module_t *module = p->init();
        if (module == NULL) {
            __fatal_error("failed to init module");
        } else {
            mp_module_register(p->name, (mp_obj_t)module);
        }
    }


    // now that everything is initialised, run main script
    if (reset_mode == 1 && pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
        vstr_t *vstr = vstr_new();
        vstr_printf(vstr, "%d:/", root_device);
        if (pyb_config_main == MP_OBJ_NULL) {
            vstr_add_str(vstr, "main.py");
        } else {
            vstr_add_str(vstr, mp_obj_str_get_str(pyb_config_main));
        }
        FRESULT res = f_stat(vstr_str(vstr), NULL);
        if (res == FR_OK) {
            if (!pyexec_file(vstr_str(vstr))) {
                flash_error(3);
            }
        }
        vstr_free(vstr);
    }

    // Enter REPL
    // REPL mode can change, or it can request a soft reset
    nlr_buf_t nlr;
    for (;;) {
        if (nlr_push(&nlr) == 0) {
            if (usbdbg_script_ready()) {
                pyexec_push_scope();
                pyexec_str(usbdbg_get_script());
                pyexec_pop_scope();
            }
            usbdbg_clr_script();

            // no script run repl
            pyexec_friendly_repl();
            nlr_pop();
        }
    }

    printf("PYB: sync filesystems\n");
    storage_flush();

    printf("PYB: soft reboot\n");

    first_soft_reset = false;
    goto soft_reset;
}

static NORETURN mp_obj_t mp_sys_exit(uint n_args, const mp_obj_t *args) {
    int rc = 0;
    if (n_args > 0) {
        rc = mp_obj_get_int(args[0]);
    }
    nlr_raise(mp_obj_new_exception_arg1(&mp_type_SystemExit, mp_obj_new_int(rc)));
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_sys_exit_obj, 0, 1, mp_sys_exit);
