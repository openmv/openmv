#include <stdio.h>
#include <string.h>
#include <stm32f4xx.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_pwr.h>
#include <stm32f4xx_rtc.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_rng.h>
#include <stm32f4xx_misc.h>
#include <libmp.h>
#include "systick.h"
#include "rcc_ctrl.h"
#include "led.h"
#include "rng.h"
#include "sensor.h"
#include "usbdbg.h"
#include "py_led.h"
#include "py_sensor.h"
#include "py_imlib.h"
#include "py_file.h"
#include "py_time.h"

int errno;

static FATFS fatfs0;

void __fatal_error(const char *msg) {
    printf("%s\n", msg);
    while (1) {
        led_state(LED_RED, 1);
        systick_sleep(250);
        led_state(LED_RED, 0);
        systick_sleep(250);
    }
}

// sync all file systems
mp_obj_t py_sync(void) {
    storage_flush();
    return mp_const_none;
}

mp_obj_t py_vcp_connected() {
    bool connected = usb_vcp_is_connected();
    return mp_obj_new_int(connected);
}

static const char fresh_main_py[] =
"# main.py -- put your code here!\n"
"import led, time\n"
"while(vcp_connected()==0):\n"
" led.on(led.BLUE)\n"
" time.sleep(500)\n"
" led.off(led.BLUE)\n"
" time.sleep(500)\n"
;

static const char *help_text =
"Welcome to Micro Python!\n\n"
"This is a *very* early version of Micro Python and has minimal functionality.\n\n"
"Specific commands for the board:\n"
"    pyb.info()     -- print some general information\n"
"    pyb.gc()       -- run the garbage collector\n"
"    pyb.repl_info(<val>) -- enable/disable printing of info after each command\n"
"    pyb.delay(<n>) -- wait for n milliseconds\n"
"    pyb.Led(<n>)   -- create Led object for LED n (n=1,2)\n"
"                      Led methods: on(), off()\n"
"    pyb.Servo(<n>) -- create Servo object for servo n (n=1,2,3,4)\n"
"                      Servo methods: angle(<x>)\n"
"    pyb.switch()   -- return True/False if switch pressed or not\n"
"    pyb.accel()    -- get accelerometer values\n"
"    pyb.rand()     -- get a 16-bit random number\n"
"    pyb.gpio(<port>)           -- get port value (port='A4' for example)\n"
"    pyb.gpio(<port>, <val>)    -- set port value, True or False, 1 or 0\n"
"    pyb.ADC(<port>) -- make an analog port object (port='C0' for example)\n"
"                       ADC methods: read()\n"
;

// get some help about available functions
static mp_obj_t py_help(void) {
    printf("%s", help_text);
    return mp_const_none;
}

// get lots of info about the board
static mp_obj_t py_info(void) {
    // get and print unique id; 96 bits
    {
        byte *id = (byte*)0x1fff7a10;
        printf("ID=%02x%02x%02x%02x:%02x%02x%02x%02x:%02x%02x%02x%02x\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8], id[9], id[10], id[11]);
    }

    // get and print clock speeds
    // SYSCLK=168MHz, HCLK=168MHz, PCLK1=42MHz, PCLK2=84MHz
    {
        RCC_ClocksTypeDef rcc_clocks;
        RCC_GetClocksFreq(&rcc_clocks);
        printf("S=%lu\nH=%lu\nP1=%lu\nP2=%lu\n", rcc_clocks.SYSCLK_Frequency, rcc_clocks.HCLK_Frequency, rcc_clocks.PCLK1_Frequency, rcc_clocks.PCLK2_Frequency);
    }

    // to print info about memory
    {
        extern void *_sidata;
        extern void *_sdata;
        extern void *_edata;
        extern void *_sbss;
        extern void *_ebss;
        extern void *_estack;
        extern void *_etext;
        printf("_etext=%p\n", &_etext);
        printf("_sidata=%p\n", &_sidata);
        printf("_sdata=%p\n", &_sdata);
        printf("_edata=%p\n", &_edata);
        printf("_sbss=%p\n", &_sbss);
        printf("_ebss=%p\n", &_ebss);
        printf("_estack=%p\n", &_estack);
        printf("_ram_start=%p\n", &_ram_start);
        printf("_heap_start=%p\n", &_heap_start);
        printf("_heap_end=%p\n", &_heap_end);
        printf("_ram_end=%p\n", &_ram_end);
    }

    // qstr info
    {
        uint n_pool, n_qstr, n_str_data_bytes, n_total_bytes;
        qstr_pool_info(&n_pool, &n_qstr, &n_str_data_bytes, &n_total_bytes);
        printf("qstr:\n  n_pool=%u\n  n_qstr=%u\n  n_str_data_bytes=%u\n  n_total_bytes=%u\n", n_pool, n_qstr, n_str_data_bytes, n_total_bytes);
    }

    // GC info
    {
        gc_info_t info;
        gc_info(&info);
        printf("GC:\n");
        printf("  %lu total\n", info.total);
        printf("used:  %lu free: %lu\n", info.used, info.free);
        printf("  1=%lu 2=%lu m=%lu\n", info.num_1block, info.num_2block, info.max_block);
    }

    // free space on flash
    {
        DWORD nclst;
        FATFS *fatfs;
        f_getfree("0:", &nclst, &fatfs);
        printf("LFS free: %u bytes\n", (uint)(nclst * fatfs->csize * 512));
    }

    return mp_const_none;
}

gc_info_t get_gc_info()
{
    gc_info_t info;
    gc_info(&info);
    return info;
}

static void SYSCLKConfig_STOP(void) {
    /* After wake-up from STOP reconfigure the system clock */
    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) {
    }

    /* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while (RCC_GetSYSCLKSource() != 0x08) {
    }
}

static mp_obj_t py_stop(void) {
    PWR_EnterSTANDBYMode();
    //PWR_FlashPowerDownCmd(ENABLE); don't know what the logic is with this

    /* Enter Stop Mode */
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

    /* Configures system clock after wake-up from STOP: enable HSE, PLL and select
     *        PLL as system clock source (HSE and PLL are disabled in STOP mode) */
    SYSCLKConfig_STOP();

    //PWR_FlashPowerDownCmd(DISABLE);

    return mp_const_none;
}

static mp_obj_t py_standby(void) {
    PWR_EnterSTANDBYMode();
    return mp_const_none;
}

void __libc_init_array(void)
{

}

int main(void)
{
    rcc_ctrl_set_frequency(SYSCLK_200_MHZ);

    /* Init SysTick timer */
    systick_init();

    /* Init MicroPython */
    libmp_init();

    /* init USB debug */
    usbdbg_init();

    /* init rng */
    rng_init();

    /* Add functions to the global python namespace */
    rt_store_name(MP_QSTR_help, rt_make_function_n(0, py_help));
    rt_store_name(qstr_from_str("open"), rt_make_function_n(2, py_file_open));
    rt_store_name(qstr_from_str("vcp_connected"), rt_make_function_n(0, py_vcp_connected));
    rt_store_name(MP_QSTR_info, rt_make_function_n(0, py_info));
    rt_store_name(MP_QSTR_gc, (mp_obj_t)&pyb_gc_obj);
    rt_store_name(MP_QSTR_stop, rt_make_function_n(0, py_stop));
    rt_store_name(MP_QSTR_standby, rt_make_function_n(0, py_standby));
    rt_store_name(MP_QSTR_sync, rt_make_function_n(0, py_sync));

    /* Add modules to the global python namespace */
    mp_obj_t time_module = py_time_init();
    rt_store_name(qstr_from_str("time"), time_module);

    mp_obj_t led_module = py_led_init();
    rt_store_name(qstr_from_str("led"), led_module);

    mp_obj_t sensor_module = py_sensor_init();
    if (!sensor_module) {
        __fatal_error("sensor init failed");
    }
    rt_store_name(qstr_from_str("sensor"), sensor_module);

    mp_obj_t imlib_module = py_imlib_init();
    rt_store_name(qstr_from_str("imlib"), imlib_module);

    /* Try to mount the flash fs */
    bool reset_filesystem = false;
    FRESULT res = f_mount(&fatfs0, "0:", 1);
    if (!reset_filesystem && res == FR_OK) {
        /* Mount sucessful */
    } else if (reset_filesystem || res == FR_NO_FILESYSTEM) {
        /* No filesystem, so create a fresh one */
        res = f_mkfs("0:", 0, 0);
        if (res != FR_OK) {
            __fatal_error("could not create LFS");
        }

        /* Create main.py */
        FIL fp;
        f_open(&fp, "0:/main.py", FA_WRITE | FA_CREATE_ALWAYS);
        UINT n;
        f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
        // TODO check we could write n bytes
        f_close(&fp);
    } else {
        __fatal_error("could not access LFS");
    }

    pyb_usb_dev_init(PYB_USB_DEV_VCP_MSC);

    /* Try to run user script first */
    if (!libmp_do_file("0:/user.py")) {
        /* no user script */
    }

    /* Fall back to main script */
    if (!libmp_do_file("0:/main.py")) {
        __fatal_error("failed to run main script");
    }

    libmp_do_repl();

    while(1);
}
