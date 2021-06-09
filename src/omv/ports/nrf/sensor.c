/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor abstraction layer for nRF port.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mphal.h"
#include "cambus.h"
#include "sensor.h"
#include "ov2640.h"
#include "ov5640.h"
#include "ov7725.h"
#include "ov7690.h"
#include "ov7670.h"
#include "ov9650.h"
#include "mt9v034.h"
#include "lepton.h"
#include "hm01b0.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"
#include "unaligned_memcpy.h"
#include "nrf_i2s.h"
#include "hal/nrf_gpio.h"

extern void __fatal_error(const char *msg);

sensor_t sensor = {0};

static uint32_t _vsyncMask;
static uint32_t _hrefMask;
static uint32_t _pclkMask;
static const volatile uint32_t *_vsyncPort;
static const volatile uint32_t *_hrefPort;
static const volatile uint32_t *_pclkPort;

#define interrupts()        __enable_irq()
#define noInterrupts()      __disable_irq()

#define digitalPinToPort(P)		(P/32)

#ifndef digitalPinToBitMask
#define digitalPinToBitMask(P) (1 << (P % 32))
#endif

#ifndef portInputRegister
#define portInputRegister(P) ((P == 0) ? &NRF_P0->IN : &NRF_P1->IN)
#endif

const int resolution[][2] = {
    {0,    0   },
    // C/SIF Resolutions
    {88,   72  },    /* QQCIF     */
    {176,  144 },    /* QCIF      */
    {352,  288 },    /* CIF       */
    {88,   60  },    /* QQSIF     */
    {176,  120 },    /* QSIF      */
    {352,  240 },    /* SIF       */
    // VGA Resolutions
    {40,   30  },    /* QQQQVGA   */
    {80,   60  },    /* QQQVGA    */
    {160,  120 },    /* QQVGA     */
    {320,  240 },    /* QVGA      */
    {640,  480 },    /* VGA       */
    {60,   40  },    /* HQQQVGA   */
    {120,  80  },    /* HQQVGA    */
    {240,  160 },    /* HQVGA     */
    // FFT Resolutions
    {64,   32  },    /* 64x32     */
    {64,   64  },    /* 64x64     */
    {128,  64  },    /* 128x64    */
    {128,  128 },    /* 128x128   */
    // Other
    {128,  160 },    /* LCD       */
    {128,  160 },    /* QQVGA2    */
    {720,  480 },    /* WVGA      */
    {752,  480 },    /* WVGA2     */
    {800,  600 },    /* SVGA      */
    {1024, 768 },    /* XGA       */
    {1280, 1024},    /* SXGA      */
    {1600, 1200},    /* UXGA      */
    {1280, 720 },    /* HD        */
    {1920, 1080},    /* FHD       */
    {2560, 1440},    /* QHD       */
    {2048, 1536},    /* QXGA      */
    {2560, 1600},    /* WQXGA     */
    {2592, 1944},    /* WQXGA2    */
};

static int extclk_config(int frequency)
{
    #ifndef I2S_CONFIG_MCKFREQ_MCKFREQ_32MDIV2
    // Note this define is out of spec and has been removed from hal.
    #define I2S_CONFIG_MCKFREQ_MCKFREQ_32MDIV2 (0x80000000UL) /*!< 32 MHz / 2 = 16.0 MHz */
    #endif

    nrf_gpio_cfg_output(DCMI_XCLK_PIN);

    // Generates 16 MHz signal using I2S peripheral
    NRF_I2S->CONFIG.MCKEN = (I2S_CONFIG_MCKEN_MCKEN_ENABLE << I2S_CONFIG_MCKEN_MCKEN_Pos);
    NRF_I2S->CONFIG.MCKFREQ = I2S_CONFIG_MCKFREQ_MCKFREQ_32MDIV2  << I2S_CONFIG_MCKFREQ_MCKFREQ_Pos;
    NRF_I2S->CONFIG.MODE = I2S_CONFIG_MODE_MODE_MASTER << I2S_CONFIG_MODE_MODE_Pos;

    NRF_I2S->PSEL.MCK = (DCMI_XCLK_PIN << I2S_PSEL_MCK_PIN_Pos);

    NRF_I2S->ENABLE = 1;
    NRF_I2S->TASKS_START = 1;

    return 0;
}

static int dcmi_config()
{
    uint32_t dcmi_pins[] = {
        DCMI_D0_PIN,
        DCMI_D1_PIN,
        DCMI_D2_PIN,
        DCMI_D3_PIN,
        DCMI_D4_PIN,
        DCMI_D5_PIN,
        DCMI_D6_PIN,
        DCMI_D7_PIN,
        DCMI_VSYNC_PIN,
        DCMI_HSYNC_PIN,
        DCMI_PXCLK_PIN,
    };

    // Configure DCMI input pins
    for (int i=0; i<sizeof(dcmi_pins)/sizeof(dcmi_pins[0]); i++) {
        nrf_gpio_cfg_input(dcmi_pins[i], NRF_GPIO_PIN_PULLUP);
    }

    _vsyncMask = digitalPinToBitMask(DCMI_VSYNC_PIN);
    _hrefMask = digitalPinToBitMask(DCMI_HSYNC_PIN);
    _pclkMask = digitalPinToBitMask(DCMI_PXCLK_PIN);

    _vsyncPort = portInputRegister(digitalPinToPort(DCMI_VSYNC_PIN));
    _hrefPort = portInputRegister(digitalPinToPort(DCMI_HSYNC_PIN));
    _pclkPort = portInputRegister(digitalPinToPort(DCMI_PXCLK_PIN));

    return 0;
}

int sensor_init()
{
    int init_ret = 0;

    #if defined(DCMI_PWDN_PIN)
    nrf_gpio_cfg_output(DCMI_PWDN_PIN);
    DCMI_PWDN_HIGH();
    #endif

    #if defined(DCMI_RESET_PIN)
    nrf_gpio_cfg_output(DCMI_RESET_PIN);
    DCMI_RESET_HIGH();
    #endif

    /* Do a power cycle */
    DCMI_PWDN_HIGH();
    mp_hal_delay_ms(10);

    DCMI_PWDN_LOW();
    mp_hal_delay_ms(10);

    // Configure the sensor external clock (XCLK) to XCLK_FREQ.
    #if (OMV_XCLK_SOURCE == OMV_XCLK_TIM)
    // Configure external clock timer.
    if (extclk_config(OMV_XCLK_FREQUENCY) != 0) {
        // Timer problem
        return -1;
    }
    #elif (OMV_XCLK_SOURCE == OMV_XCLK_OSC)
    // An external oscillator is used for the sensor clock.
    // Nothing to do.
    #else
    #error "OMV_XCLK_SOURCE is not set!"
    #endif

    /* Reset the sesnor state */
    memset(&sensor, 0, sizeof(sensor_t));

    /* Some sensors have different reset polarities, and we can't know which sensor
       is connected before initializing cambus and probing the sensor, which in turn
       requires pulling the sensor out of the reset state. So we try to probe the
       sensor with both polarities to determine line state. */
    sensor.pwdn_pol = ACTIVE_HIGH;
    sensor.reset_pol = ACTIVE_HIGH;

    /* Reset the sensor */
    DCMI_RESET_HIGH();
    mp_hal_delay_ms(10);

    DCMI_RESET_LOW();
    mp_hal_delay_ms(10);

    // Initialize the camera bus.
    cambus_init(&sensor.bus, ISC_I2C_ID, ISC_I2C_SPEED);
    mp_hal_delay_ms(10);

    /* Probe the sensor */
    sensor.slv_addr = cambus_scan(&sensor.bus);
    if (sensor.slv_addr == 0) {
        /* Sensor has been held in reset,
           so the reset line is active low */
        sensor.reset_pol = ACTIVE_LOW;

        /* Pull the sensor out of the reset state */
        DCMI_RESET_HIGH();
        mp_hal_delay_ms(10);

        /* Probe again to set the slave addr */
        sensor.slv_addr = cambus_scan(&sensor.bus);
        if (sensor.slv_addr == 0) {
            sensor.pwdn_pol = ACTIVE_LOW;

            DCMI_PWDN_HIGH();
            mp_hal_delay_ms(10);

            sensor.slv_addr = cambus_scan(&sensor.bus);
            if (sensor.slv_addr == 0) {
                sensor.reset_pol = ACTIVE_HIGH;

                DCMI_RESET_LOW();
                mp_hal_delay_ms(10);

                sensor.slv_addr = cambus_scan(&sensor.bus);
                if (sensor.slv_addr == 0) {
                    return -2;
                }
            }
        }
    }

    // Clear sensor chip ID.
    sensor.chip_id = 0;

    // Set default snapshot function.
    sensor.snapshot = sensor_snapshot;

    switch (sensor.slv_addr) {
        #if (OMV_ENABLE_OV2640 == 1)
        case OV2640_SLV_ADDR: // Or OV9650.
            cambus_readb(&sensor.bus, sensor.slv_addr, OV_CHIP_ID, &sensor.chip_id);
            break;
        #endif // (OMV_ENABLE_OV2640 == 1)

        #if (OMV_ENABLE_OV5640 == 1)
        case OV5640_SLV_ADDR:
            cambus_readb2(&sensor.bus, sensor.slv_addr, OV5640_CHIP_ID, &sensor.chip_id);
            break;
        #endif // (OMV_ENABLE_OV5640 == 1)

        #if (OMV_ENABLE_OV7725 == 1) || (OMV_ENABLE_OV7670 == 1) || (OMV_ENABLE_OV7690 == 1)
        case OV7725_SLV_ADDR: // Or OV7690 or OV7670.
            cambus_readb(&sensor.bus, sensor.slv_addr, OV_CHIP_ID, &sensor.chip_id);
            break;
        #endif //(OMV_ENABLE_OV7725 == 1) || (OMV_ENABLE_OV7670 == 1) || (OMV_ENABLE_OV7690 == 1)

        #if (OMV_ENABLE_MT9V034 == 1)
        case MT9V034_SLV_ADDR:
            cambus_readb(&sensor.bus, sensor.slv_addr, ON_CHIP_ID, &sensor.chip_id);
            break;
        #endif //(OMV_ENABLE_MT9V034 == 1)

        #if (OMV_ENABLE_MT9M114 == 1)
        case MT9M114_SLV_ADDR:
            cambus_readw2(&sensor.bus, sensor.slv_addr, ON_CHIP_ID, &sensor.chip_id_w);
            break;
        #endif // (OMV_ENABLE_MT9M114 == 1)

        #if (OMV_ENABLE_LEPTON == 1)
        case LEPTON_SLV_ADDR:
            sensor.chip_id = LEPTON_ID;
            break;
        #endif // (OMV_ENABLE_LEPTON == 1)

        #if (OMV_ENABLE_HM01B0 == 1)
        case HM01B0_SLV_ADDR:
            cambus_readb2(&sensor.bus, sensor.slv_addr, HIMAX_CHIP_ID, &sensor.chip_id);
            break;
        #endif //(OMV_ENABLE_HM01B0 == 1)
        default:
            return -3;
            break;
    }

    switch (sensor.chip_id) {
        #if (OMV_ENABLE_OV2640 == 1)
        case OV2640_ID:
            if (extclk_config(OV2640_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = ov2640_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV2640 == 1)

        #if (OMV_ENABLE_OV5640 == 1)
        case OV5640_ID:
            if (extclk_config(OV5640_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = ov5640_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV5640 == 1)

        #if (OMV_ENABLE_OV7670 == 1)
        case OV7670_ID:
            if (extclk_config(OV7670_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = ov7670_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV7670 == 1)

        #if (OMV_ENABLE_OV7690 == 1)
        case OV7690_ID:
            if (extclk_config(OV7690_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = ov7690_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV7690 == 1)

        #if (OMV_ENABLE_OV7725 == 1)
        case OV7725_ID:
            init_ret = ov7725_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV7725 == 1)

        #if (OMV_ENABLE_OV9650 == 1)
        case OV9650_ID:
            init_ret = ov9650_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV9650 == 1)

        #if (OMV_ENABLE_MT9V034 == 1)
        case MT9V034_ID:
            if (extclk_config(MT9V034_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = mt9v034_init(&sensor);
            break;
        #endif //(OMV_ENABLE_MT9V034 == 1)

        #if (OMV_ENABLE_MT9M114 == 1)
        case MT9M114_ID:
            if (extclk_config(MT9M114_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = mt9m114_init(&sensor);
            break;
        #endif //(OMV_ENABLE_MT9M114 == 1)

        #if (OMV_ENABLE_LEPTON == 1)
        case LEPTON_ID:
            if (extclk_config(LEPTON_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = lepton_init(&sensor);
            break;
        #endif // (OMV_ENABLE_LEPTON == 1)

        #if (OMV_ENABLE_HM01B0 == 1)
        case HM01B0_ID:
            init_ret = hm01b0_init(&sensor);
            break;
        #endif //(OMV_ENABLE_HM01B0 == 1)

        default:
            return -3;
            break;
    }

    if (init_ret != 0 ) {
        // Sensor init failed.
        return -4;
    }

    /* Configure the DCMI interface. */
    if (dcmi_config() != 0){
        // DCMI config failed
        return -6;
    }

    // Clear fb_enabled flag
    // This is executed only once to initialize the FB enabled flag.
    // TODO
    //JPEG_FB()->enabled = 0;

    // Set default color palette.
    sensor.color_palette = rainbow_table;

    sensor.detected = true;

    // Disable VSYNC IRQ and callback
    sensor_set_vsync_callback(NULL);

    /* All good! */
    return 0;
}

int sensor_reset()
{
    framebuffer_reset_buffers();

    // Reset the sensor state
    sensor.sde                  = 0;
    sensor.pixformat            = 0;
    sensor.framesize            = 0;
    sensor.framerate            = 0;
    sensor.last_frame_ms        = 0;
    sensor.last_frame_ms_valid  = false;
    sensor.gainceiling          = 0;
    sensor.hmirror              = false;
    sensor.vflip                = false;
    sensor.transpose            = false;
    #if MICROPY_PY_IMU
    sensor.auto_rotation        = sensor.chip_id == OV7690_ID;
    #else
    sensor.auto_rotation        = false;
    #endif // MICROPY_PY_IMU
    sensor.vsync_callback       = NULL;
    sensor.frame_callback       = NULL;

    // Reset default color palette.
    sensor.color_palette        = rainbow_table;

    // Restore shutdown state on reset.
    sensor_shutdown(false);

    // Hard-reset the sensor
    if (sensor.reset_pol == ACTIVE_HIGH) {
        DCMI_RESET_HIGH();
        mp_hal_delay_ms(10);
        DCMI_RESET_LOW();
    } else {
        DCMI_RESET_LOW();
        mp_hal_delay_ms(10);
        DCMI_RESET_HIGH();
    }
    mp_hal_delay_ms(20);

    // Call sensor-specific reset function
    if (sensor.reset(&sensor) != 0) {
        return -1;
    }

    return 0;
}

int sensor_get_id()
{
    return sensor.chip_id;
}

bool sensor_is_detected()
{
    return sensor.detected;
}

int sensor_sleep(int enable)
{
    if (sensor.sleep == NULL
        || sensor.sleep(&sensor, enable) != 0) {
        // Operation not supported
        return -1;
    }
    return 0;
}

int sensor_shutdown(int enable)
{
    int ret = 0;
    if (enable) {
        if (sensor.pwdn_pol == ACTIVE_HIGH) {
            DCMI_PWDN_HIGH();
        } else {
            DCMI_PWDN_LOW();
        }
    } else {
        if (sensor.pwdn_pol == ACTIVE_HIGH) {
            DCMI_PWDN_LOW();
        } else {
            DCMI_PWDN_HIGH();
        }
    }

    mp_hal_delay_ms(10);
    return ret;
}

int sensor_read_reg(uint16_t reg_addr)
{
    if (sensor.read_reg == NULL) {
        // Operation not supported
        return -1;
    }
    return sensor.read_reg(&sensor, reg_addr);
}

int sensor_write_reg(uint16_t reg_addr, uint16_t reg_data)
{
    if (sensor.write_reg == NULL) {
        // Operation not supported
        return -1;
    }
    return sensor.write_reg(&sensor, reg_addr, reg_data);
}

int sensor_set_pixformat(pixformat_t pixformat)
{
    if (sensor.pixformat == pixformat) {
        // No change
        return 0;
    }

    // sensor_check_buffsize() will switch from PIXFORMAT_BAYER to PIXFORMAT_RGB565 to try to fit
    // the MAIN_FB() in RAM as a first step optimization. If the user tries to switch back to RGB565
    // and that would be bigger than the RAM buffer we would just switch back.
    //
    // So, just short-circuit doing any work.
    //
    // This code is explicitly here to allow users to set the resolution to RGB565 and have it
    // switch to BAYER only once even though they are setting the resolution to RGB565 repeatedly
    // in a loop. Only RGB565->BAYER has this problem and needs this fix because of sensor_check_buffsize().
    uint32_t size = framebuffer_get_buffer_size();
    if ((sensor.pixformat == PIXFORMAT_BAYER)
    &&  (pixformat == PIXFORMAT_RGB565)
    &&  (MAIN_FB()->u * MAIN_FB()->v * 2 > size)
    &&  (MAIN_FB()->u * MAIN_FB()->v * 1 <= size)) {
        // No change
        return 0;
    }

    // Cropping and transposing (and thus auto rotation) don't work in JPEG mode.
    //if ((pixformat == PIXFORMAT_JPEG) && (cropped() || sensor.transpose || sensor.auto_rotation)) {
    //    return -1;
    //}

    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

    if (sensor.set_pixformat == NULL
        || sensor.set_pixformat(&sensor, pixformat) != 0) {
        // Operation not supported
        return -1;
    }

    mp_hal_delay_ms(100); // wait for the camera to settle

    // Set pixel format
    sensor.pixformat = pixformat;

    // Skip the first frame.
    MAIN_FB()->bpp = -1;

    return 0;
}

int sensor_set_framesize(framesize_t framesize)
{
    if (sensor.framesize == framesize) {
        // No change
        return 0;
    }

    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

    // Call the sensor specific function
    if (sensor.set_framesize == NULL
        || sensor.set_framesize(&sensor, framesize) != 0) {
        // Operation not supported
        return -1;
    }

    mp_hal_delay_ms(100); // wait for the camera to settle

    // Set framebuffer size
    sensor.framesize = framesize;

    // Skip the first frame.
    MAIN_FB()->bpp = -1;

    // Set MAIN FB x offset, y offset, width, height, backup width, and backup height.
    MAIN_FB()->x = 0;
    MAIN_FB()->y = 0;
    MAIN_FB()->w = MAIN_FB()->u = resolution[framesize][0];
    MAIN_FB()->h = MAIN_FB()->v = resolution[framesize][1];

    return 0;
}

int sensor_set_framerate(int framerate)
{
    if (sensor.framerate == framerate) {
        // No change
        return 0;
    }

    if (framerate < 0) {
        return -1;
    }

    // Call the sensor specific function (does not fail if function is not set)
    if (sensor.set_framerate != NULL) {
        if (sensor.set_framerate(&sensor, framerate) != 0) {
            // Operation not supported
            return -1;
        }
    }

    // Set framerate
    sensor.framerate = framerate;
    return 0;
}

int sensor_set_windowing(int x, int y, int w, int h)
{
    if ((MAIN_FB()->x == x) && (MAIN_FB()->y == y) && (MAIN_FB()->u == w) && (MAIN_FB()->v == h)) {
        // No change
        return 0;
    }

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return -1;
    }

    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

    // Skip the first frame.
    MAIN_FB()->bpp = -1;

    MAIN_FB()->x = x;
    MAIN_FB()->y = y;
    MAIN_FB()->w = MAIN_FB()->u = w;
    MAIN_FB()->h = MAIN_FB()->v = h;

    return 0;
}

int sensor_set_contrast(int level)
{
    if (sensor.set_contrast != NULL) {
        return sensor.set_contrast(&sensor, level);
    }
    return -1;
}

int sensor_set_brightness(int level)
{
    if (sensor.set_brightness != NULL) {
        return sensor.set_brightness(&sensor, level);
    }
    return -1;
}

int sensor_set_saturation(int level)
{
    if (sensor.set_saturation != NULL) {
        return sensor.set_saturation(&sensor, level);
    }
    return -1;
}

int sensor_set_gainceiling(gainceiling_t gainceiling)
{
    if (sensor.gainceiling == gainceiling) {
        /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_gainceiling == NULL
        || sensor.set_gainceiling(&sensor, gainceiling) != 0) {
        /* operation not supported */
        return -1;
    }

    sensor.gainceiling = gainceiling;
    return 0;
}

int sensor_set_quality(int qs)
{
    /* call the sensor specific function */
    if (sensor.set_quality == NULL
        || sensor.set_quality(&sensor, qs) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_colorbar(int enable)
{
    /* call the sensor specific function */
    if (sensor.set_colorbar == NULL
        || sensor.set_colorbar(&sensor, enable) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_auto_gain(int enable, float gain_db, float gain_db_ceiling)
{
    /* call the sensor specific function */
    if (sensor.set_auto_gain == NULL
        || sensor.set_auto_gain(&sensor, enable, gain_db, gain_db_ceiling) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_get_gain_db(float *gain_db)
{
    /* call the sensor specific function */
    if (sensor.get_gain_db == NULL
        || sensor.get_gain_db(&sensor, gain_db) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_auto_exposure(int enable, int exposure_us)
{
    /* call the sensor specific function */
    if (sensor.set_auto_exposure == NULL
        || sensor.set_auto_exposure(&sensor, enable, exposure_us) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_get_exposure_us(int *exposure_us)
{
    /* call the sensor specific function */
    if (sensor.get_exposure_us == NULL
        || sensor.get_exposure_us(&sensor, exposure_us) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_auto_whitebal(int enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
    /* call the sensor specific function */
    if (sensor.set_auto_whitebal == NULL
        || sensor.set_auto_whitebal(&sensor, enable, r_gain_db, g_gain_db, b_gain_db) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_get_rgb_gain_db(float *r_gain_db, float *g_gain_db, float *b_gain_db)
{
    /* call the sensor specific function */
    if (sensor.get_rgb_gain_db == NULL
        || sensor.get_rgb_gain_db(&sensor, r_gain_db, g_gain_db, b_gain_db) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_hmirror(int enable)
{
    if (sensor.hmirror == ((bool) enable)) {
        /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_hmirror == NULL
        || sensor.set_hmirror(&sensor, enable) != 0) {
        /* operation not supported */
        return -1;
    }
    sensor.hmirror = enable;
    mp_hal_delay_ms(100); // wait for the camera to settle
    return 0;
}

bool sensor_get_hmirror()
{
    return sensor.hmirror;
}

int sensor_set_vflip(int enable)
{
    if (sensor.vflip == ((bool) enable)) {
        /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_vflip == NULL
        || sensor.set_vflip(&sensor, enable) != 0) {
        /* operation not supported */
        return -1;
    }
    sensor.vflip = enable;
    mp_hal_delay_ms(100); // wait for the camera to settle
    return 0;
}

bool sensor_get_vflip()
{
    return sensor.vflip;
}

int sensor_set_transpose(bool enable)
{
    if (sensor.transpose == enable) {
        /* no change */
        return 0;
    }

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return -1;
    }

    sensor.transpose = enable;
    return 0;
}

bool sensor_get_transpose()
{
    return sensor.transpose;
}

int sensor_set_auto_rotation(bool enable)
{
    if (sensor.auto_rotation == enable) {
        /* no change */
        return 0;
    }

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return -1;
    }

    sensor.auto_rotation = enable;
    return 0;
}

bool sensor_get_auto_rotation()
{
    return sensor.auto_rotation;
}

int sensor_set_framebuffers(int count)
{
    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

    return framebuffer_set_buffers(count);
}

int sensor_set_special_effect(sde_t sde)
{
    if (sensor.sde == sde) {
        /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_special_effect == NULL
        || sensor.set_special_effect(&sensor, sde) != 0) {
        /* operation not supported */
        return -1;
    }

    sensor.sde = sde;
    return 0;
}

int sensor_set_lens_correction(int enable, int radi, int coef)
{
    /* call the sensor specific function */
    if (sensor.set_lens_correction == NULL
        || sensor.set_lens_correction(&sensor, enable, radi, coef) != 0) {
        /* operation not supported */
        return -1;
    }

    return 0;
}

int sensor_ioctl(int request, ... /* arg */)
{
    int ret = -1;

    if (sensor.ioctl != NULL) {
        va_list ap;
        va_start(ap, request);
        /* call the sensor specific function */
        ret = sensor.ioctl(&sensor, request, ap);
        va_end(ap);
    }

    return ret;
}

int sensor_set_vsync_callback(vsync_cb_t vsync_cb)
{
    sensor.vsync_callback = vsync_cb;
    if (sensor.vsync_callback == NULL) {
        // Disable VSYNC EXTI IRQ
    } else {
        // Enable VSYNC EXTI IRQ
    }
    return 0;
}

int sensor_set_frame_callback(frame_cb_t vsync_cb)
{
    sensor.frame_callback = vsync_cb;
    return 0;
}

int sensor_set_color_palette(const uint16_t *color_palette)
{
    sensor.color_palette = color_palette;
    return 0;
}

const uint16_t *sensor_get_color_palette()
{
    return sensor.color_palette;
}

void VsyncExtiCallback()
{
    if (sensor.vsync_callback != NULL) {
        //sensor.vsync_callback(HAL_GPIO_ReadPin(DCMI_VSYNC_PORT, DCMI_VSYNC_PIN));
    }
}

// To make the user experience better we automatically shrink the size of the MAIN_FB() to fit
// within the RAM we have onboard the system.
void sensor_check_buffsize()
{
    if (MAIN_FB()->n_buffers != 1) {
        framebuffer_set_buffers(1);
    }

    uint32_t size = framebuffer_get_buffer_size();
    uint32_t bpp;

    switch (sensor.pixformat) {
        case PIXFORMAT_GRAYSCALE:
        case PIXFORMAT_BAYER:
            bpp = 1;
            break;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            bpp = 2;
            break;
        // If the pixformat is NULL/JPEG there we can't do anything to check if it fits before hand.
        default:
            return;
    }

    // MAIN_FB() fits, we are done.
    if ((MAIN_FB()->u * MAIN_FB()->v * bpp) <= size) {
        return;
    }

    if (sensor.pixformat == PIXFORMAT_RGB565) {
        // Switch to bayer for the quick 2x savings.
        sensor_set_pixformat(PIXFORMAT_BAYER);
        bpp = 1;

        // MAIN_FB() fits, we are done (bpp is 1).
        if (MAIN_FB()->u * MAIN_FB()->v <= size) {
            return;
        }
    }

    int window_w = MAIN_FB()->u;
    int window_h = MAIN_FB()->v;

    // We need to shrink the frame buffer. We can do this by cropping. So, we will subtract columns
    // and rows from the frame buffer until it fits within the frame buffer.
    int max = IM_MAX(window_w, window_h);
    int min = IM_MIN(window_w, window_h);
    float aspect_ratio = max / ((float) min);
    float r = aspect_ratio, best_r = r;
    int c = 1, best_c = c;
    float best_err = FLT_MAX;

    // Find the width/height ratio that's within 1% of the aspect ratio with a loop limit.
    for (int i = 100; i; i--) {
        float err = fast_fabsf(r - fast_roundf(r));

        if (err <= best_err) {
            best_err = err;
            best_r = r;
            best_c = c;
        }

        if (best_err <= 0.01f) {
            break;
        }

        r += aspect_ratio;
        c += 1;
    }

    // Select the larger geometry to map the aspect ratio to.
    int u_sub, v_sub;

    if (window_w > window_h) {
        u_sub = fast_roundf(best_r);
        v_sub = best_c;
    } else {
        u_sub = best_c;
        v_sub = fast_roundf(best_r);
    }

    // Crop the frame buffer while keeping the aspect ratio and keeping the width/height even.
    while (((MAIN_FB()->u * MAIN_FB()->v * bpp) > size) || (MAIN_FB()->u % 2)  || (MAIN_FB()->v % 2)) {
        MAIN_FB()->u -= u_sub;
        MAIN_FB()->v -= v_sub;
    }

    // Center the new window using the previous offset and keep the offset even.
    MAIN_FB()->x += (window_w - MAIN_FB()->u) / 2;
    MAIN_FB()->y += (window_h - MAIN_FB()->v) / 2;

    if (MAIN_FB()->x % 2) {
        MAIN_FB()->x -= 1;
    }

    if (MAIN_FB()->y % 2) {
        MAIN_FB()->y -= 1;
    }
}

// This is the default snapshot function, which can be replaced in sensor_init functions.
int sensor_snapshot(sensor_t *sensor, image_t *image, uint32_t flags)
{
    // Compress the framebuffer for the IDE preview, only if it's not the first frame,
    // the framebuffer is enabled and the image sensor does not support JPEG encoding.
    // Note: This doesn't run unless the IDE is connected and the framebuffer is enabled.
    framebuffer_update_jpeg_buffer();

    framebuffer_free_current_buffer();
    vbuffer_t *buffer = framebuffer_get_tail(FB_NO_FLAGS);

    if (!buffer) {
        return -1;
    }

    uint8_t *b = buffer->data;
    uint32_t _width  = MAIN_FB()->w;
    uint32_t _height = MAIN_FB()->h;
    int bytesPerRow  = _width * 2; // Always read 2 BPP
    bool _grayscale = (sensor->pixformat == PIXFORMAT_GRAYSCALE);

    uint32_t ulPin = 32; // P1.xx set of GPIO is in 'pin' 32 and above
    NRF_GPIO_Type *port = nrf_gpio_pin_port_decode(&ulPin);

    noInterrupts();

    // Falling edge indicates start of frame
    while ((*_vsyncPort & _vsyncMask) == 0); // wait for HIGH
    while ((*_vsyncPort & _vsyncMask) != 0); // wait for LOW

    for (int i = 0; i < _height; i++) {
        // rising edge indicates start of line
        while ((*_hrefPort & _hrefMask) == 0); // wait for HIGH

        for (int j = 0; j < bytesPerRow; j++) {
            // rising edges clock each data byte
            while ((*_pclkPort & _pclkMask) != 0); // wait for LOW

            uint32_t in = port->IN; // read all bits in parallel
            //in = (in >> 8) | ((in>>2) & 3);
            in >>= 2; // place bits 0 and 1 at the "bottom" of the register
            in &= 0x3f03; // isolate the 8 bits we care about
            in |= (in >> 6); // combine the upper 6 and lower 2 bits

            if (!(j & 1) || !_grayscale) {
                *b++ = in;
            }
            while ((*_pclkPort & _pclkMask) == 0); // wait for HIGH
        }
        while ((*_hrefPort & _hrefMask) != 0); // wait for LOW
    }

    interrupts();

    // Not useful for the NRF but must call to keep API the same.
    if (sensor->frame_callback) {
        sensor->frame_callback();
    }

    // Fix the BPP.
    switch (sensor->pixformat) {
        case PIXFORMAT_GRAYSCALE:
            MAIN_FB()->bpp = 1;
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_RGB565: {
            MAIN_FB()->bpp = 2;
            if (SENSOR_HW_FLAGS_GET(sensor, SWNSOR_HW_FLAGS_RGB565_REV)) {
                unaligned_memcpy_rev16(buffer->data, buffer->data, _width*_height);
            }
            break;
        }
        case PIXFORMAT_BAYER:
            MAIN_FB()->bpp = 3;
            break;
        default:
            MAIN_FB()->bpp = -1;
            break;
    }

    // Set the user image.
    if (image != NULL) {
        image->w = MAIN_FB()->w;
        image->h = MAIN_FB()->h;
        image->bpp = MAIN_FB()->bpp;
        image->pixels = buffer->data;
    }

    return 0;
}
