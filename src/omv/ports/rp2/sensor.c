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
#if MICROPY_PY_SENSOR
#include <stdio.h>
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

#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "omv_boardconfig.h"
#include "unaligned_memcpy.h"
#include "dcmi.pio.h"

sensor_t sensor = {0};
extern void __fatal_error(const char *msg);
static void dma_irq_handler();

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
    {30,   20  },    /* HQQQQVGA  */
    {60,   40  },    /* HQQQVGA   */
    {120,  80  },    /* HQQVGA    */
    {240,  160 },    /* HQVGA     */
    {480,  320 },    /* HVGA      */
    // FFT Resolutions
    {64,   32  },    /* 64x32     */
    {64,   64  },    /* 64x64     */
    {128,  64  },    /* 128x64    */
    {128,  128 },    /* 128x128   */
    // Himax Resolutions
    {160,  160 },    /* 160x160   */
    {320,  320 },    /* 320x320   */
    // Other
    {128,  160 },    /* LCD       */
    {128,  160 },    /* QQVGA2    */
    {720,  480 },    /* WVGA      */
    {752,  480 },    /* WVGA2     */
    {800,  600 },    /* SVGA      */
    {1024, 768 },    /* XGA       */
    {1280, 768 },    /* WXGA      */
    {1280, 1024},    /* SXGA      */
    {1280, 960 },    /* SXGAM     */
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
    uint32_t p = 4;

    // Allocate pin to the PWM
    gpio_set_function(DCMI_XCLK_PIN, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to the GPIO
    uint slice_num = pwm_gpio_to_slice_num(DCMI_XCLK_PIN);

    // Set period to p cycles
    pwm_set_wrap(slice_num, p-1);

    // Set channel A 50% duty cycle.
    pwm_set_chan_level(slice_num, PWM_CHAN_A, p/2);

    // Set sysclk divider
    // f = 125000000 / (p * (1 + (p/16)))
    pwm_set_clkdiv_int_frac(slice_num, 1, p);

    // Set the PWM running
    pwm_set_enabled(slice_num, true);

    return 0;
}

static void dma_config(int w, int h, int bpp, uint32_t *capture_buf, bool rev_bytes)
{
    dma_channel_abort(DCMI_DMA_CHANNEL);
    dma_irqn_set_channel_enabled(DCMI_DMA, DCMI_DMA_CHANNEL, false);

    dma_channel_config c = dma_channel_get_default_config(DCMI_DMA_CHANNEL);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(DCMI_PIO, DCMI_SM, false));
    channel_config_set_bswap(&c, rev_bytes);

    dma_channel_configure(DCMI_DMA_CHANNEL, &c,
        capture_buf,                // Destinatinon pointer.
        &DCMI_PIO->rxf[DCMI_SM],    // Source pointer.
        (w*h*bpp)>>2,               // Number of transfers in words.
        true                        // Start immediately, will block on SM.
    );

    // Re-enable DMA IRQs.
    dma_irqn_set_channel_enabled(DCMI_DMA, DCMI_DMA_CHANNEL, true);
}

static int dcmi_config(uint32_t pixformat)
{
    uint offset;
    pio_sm_config config;

    pio_sm_set_enabled(DCMI_PIO, DCMI_SM, false);
    pio_sm_clear_fifos(DCMI_PIO, DCMI_SM);

    for(uint i=DCMI_D0_PIN; i<DCMI_D0_PIN+8; i++) {
        pio_gpio_init(DCMI_PIO, i);
    }
    pio_sm_set_consecutive_pindirs(DCMI_PIO, DCMI_SM, DCMI_D0_PIN, 8, false);

    if (pixformat == PIXFORMAT_GRAYSCALE) {
        offset = pio_add_program(DCMI_PIO, &dcmi_odd_byte_program);
        config = dcmi_odd_byte_program_get_default_config(offset);
    } else {
        offset = pio_add_program(DCMI_PIO, &dcmi_default_program);
        config = dcmi_default_program_get_default_config(offset);
    }

    sm_config_set_clkdiv(&config, 1);
    sm_config_set_in_pins(&config, DCMI_D0_PIN);
    sm_config_set_in_shift(&config, true, true, 32);
    pio_sm_init(DCMI_PIO, DCMI_SM, offset, &config);
    pio_sm_set_enabled(DCMI_PIO, DCMI_SM, true);
    return 0;
}

void dcmi_abort()
{
    // Disable DMA channel
    dma_channel_abort(DCMI_DMA_CHANNEL);
    dma_irqn_set_channel_enabled(DCMI_DMA, DCMI_DMA_CHANNEL, false);

    // Disable state machine.
    pio_sm_set_enabled(DCMI_PIO, DCMI_SM, false);
    pio_sm_clear_fifos(DCMI_PIO, DCMI_SM);

    // Clear bpp flag.
    MAIN_FB()->bpp = -1;
}

int sensor_init()
{
    int init_ret = 0;

    // PIXCLK
    gpio_init(DCMI_PXCLK_PIN);
    gpio_set_dir(DCMI_PXCLK_PIN, GPIO_IN);

    // HSYNC
    gpio_init(DCMI_HSYNC_PIN);
    gpio_set_dir(DCMI_HSYNC_PIN, GPIO_IN);

    // VSYNC
    gpio_init(DCMI_VSYNC_PIN);
    gpio_set_dir(DCMI_VSYNC_PIN, GPIO_IN);

    #if defined(DCMI_PWDN_PIN)
    gpio_init(DCMI_PWDN_PIN);
    gpio_set_dir(DCMI_PWDN_PIN, GPIO_OUT);
    gpio_pull_down(DCMI_PWDN_PIN);
    DCMI_PWDN_HIGH();
    #endif

    #if defined(DCMI_RESET_PIN)
    gpio_init(DCMI_RESET_PIN);
    gpio_set_dir(DCMI_RESET_PIN, GPIO_OUT);
    gpio_pull_up(DCMI_RESET_PIN);
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

    // Set default color palette.
    sensor.color_palette = rainbow_table;

    // Disable VSYNC IRQ and callback
    sensor_set_vsync_callback(NULL);

    // Set new DMA IRQ handler.
    // Disable IRQs.
    irq_set_enabled(DCMI_DMA_IRQ, false);

    // Clear DMA interrupts.
    dma_irqn_acknowledge_channel(DCMI_DMA, DCMI_DMA_CHANNEL);

    // Remove current handler if any
    irq_handler_t irq_handler = irq_get_exclusive_handler(DCMI_DMA_IRQ);
    if (irq_handler != NULL) {
        irq_remove_handler(DCMI_DMA_IRQ, irq_handler);
    }

    // Set new exclusive IRQ handler.
    irq_set_exclusive_handler(DCMI_DMA_IRQ, dma_irq_handler);
    // Or set shared IRQ handler, but this needs to be called once.
    // irq_add_shared_handler(DCMI_DMA_IRQ, dma_irq_handler, PICO_DEFAULT_IRQ_PRIORITY);

    irq_set_enabled(DCMI_DMA_IRQ, true);

    /* All good! */
    sensor.detected = true;

    return 0;
}

int sensor_reset()
{
    dcmi_abort();

    // Reset the sensor state
    sensor.sde           = 0;
    sensor.pixformat     = 0;
    sensor.framesize     = 0;
    sensor.framerate     = 0;
    sensor.gainceiling   = 0;
    sensor.hmirror       = false;
    sensor.vflip         = false;
    sensor.transpose     = false;
    #if MICROPY_PY_IMU
    sensor.auto_rotation = sensor.chip_id == OV7690_ID;
    #else
    sensor.auto_rotation = false;
    #endif // MICROPY_PY_IMU
    sensor.vsync_callback= NULL;
    sensor.frame_callback= NULL;

    // Reset default color palette.
    sensor.color_palette = rainbow_table;

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

    // Reset framebuffers
    framebuffer_reset_buffers();
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

    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

    if (sensor.set_pixformat == NULL
        || sensor.set_pixformat(&sensor, pixformat) != 0) {
        // Operation not supported
        return -1;
    }

    // wait for the camera to settle
    mp_hal_delay_ms(100);

    // Set pixel format
    sensor.pixformat = pixformat;

    // Skip the first frame.
    MAIN_FB()->bpp = -1;

    // Reconfigure PIO DCMI program.
    dcmi_config(pixformat);

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

    // wait for the camera to settle
    mp_hal_delay_ms(100);

    // Set framebuffer size
    sensor.framesize = framesize;

    // Skip the first frame.
    MAIN_FB()->bpp = -1;

    // Set MAIN FB x offset, y offset, width, height, backup width, and backup height.
    MAIN_FB()->x = 0;
    MAIN_FB()->y = 0;
    MAIN_FB()->w = MAIN_FB()->u = resolution[framesize][0];
    MAIN_FB()->h = MAIN_FB()->v = resolution[framesize][1];

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers();
    return 0;
}

int sensor_set_framerate(int framerate)
{
    if (sensor.framerate == framerate) {
        // No change
        return 0;
    }

    // Call the sensor specific function
    if (sensor.set_framerate == NULL
        || sensor.set_framerate(&sensor, framerate) != 0) {
        // Operation not supported
        return -1;
    }

    return 0;
}

int sensor_set_windowing(int x, int y, int w, int h)
{
    return -1;
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
int sensor_check_buffsize()
{
    uint32_t bpp;
    uint32_t size = framebuffer_get_buffer_size();

    switch (sensor.pixformat) {
        case PIXFORMAT_GRAYSCALE:
        case PIXFORMAT_BAYER:
            bpp = 1;
            break;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            bpp = 2;
            break;
        default:
            return -1;
    }

    // This driver doesn't support windowing or anything like that.
    if ((MAIN_FB()->u * MAIN_FB()->v * bpp) > size) {
        return -1;
    }

    return 0;
}

static void dma_irq_handler()
{
    // Clear the interrupt request.
    dma_irqn_acknowledge_channel(DCMI_DMA, DCMI_DMA_CHANNEL);

    framebuffer_get_tail(FB_NO_FLAGS);
    vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);
    if (buffer != NULL) {
        // Set next buffer and retrigger the DMA channel.
        dma_channel_set_write_addr(DCMI_DMA_CHANNEL, buffer->data, true);

        // Unblock the state machine
        pio_sm_restart(DCMI_PIO, DCMI_SM);
        pio_sm_clear_fifos(DCMI_PIO, DCMI_SM);
        pio_sm_put_blocking(DCMI_PIO, DCMI_SM, (MAIN_FB()->v - 1));
        pio_sm_put_blocking(DCMI_PIO, DCMI_SM, (MAIN_FB()->u * MAIN_FB()->bpp) - 1);
    }
}

// This is the default snapshot function, which can be replaced in sensor_init functions.
int sensor_snapshot(sensor_t *sensor, image_t *image, uint32_t flags)
{
    // Compress the framebuffer for the IDE preview.
    framebuffer_update_jpeg_buffer();

    if (sensor_check_buffsize() != 0) {
        return -1;
    }

    // Free the current FB head.
    framebuffer_free_current_buffer();

    switch (sensor->pixformat) {
        case PIXFORMAT_BAYER:
        case PIXFORMAT_GRAYSCALE:
            MAIN_FB()->bpp = 1;
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_RGB565:
            MAIN_FB()->bpp = 2;
            break;
        default:
            return -1;
    }

    vbuffer_t *buffer = framebuffer_get_head(FB_NO_FLAGS);

    // If there's no ready buffer in the fifo, and the DMA is Not currently
    // transferring a new buffer, reconfigure and restart the DMA transfer.
    if (buffer == NULL && !dma_channel_is_busy(DCMI_DMA_CHANNEL)) {
        buffer = framebuffer_get_tail(FB_PEEK);
        if (buffer == NULL) {
            return -1;
        }

        // Configure the DMA on the first frame, for later frames only the write is changed.
        dma_config(MAIN_FB()->u, MAIN_FB()->v, MAIN_FB()->bpp, (void *) buffer->data,
                (SENSOR_HW_FLAGS_GET(sensor, SENSOR_HW_FLAGS_RGB565_REV) && MAIN_FB()->bpp == 2));

        // Unblock the state machine
        pio_sm_put_blocking(DCMI_PIO, DCMI_SM, (MAIN_FB()->v - 1));
        pio_sm_put_blocking(DCMI_PIO, DCMI_SM, (MAIN_FB()->u * MAIN_FB()->bpp) - 1);
    }

    // Wait for the DMA to finish the transfer.
    for (mp_uint_t ticks = mp_hal_ticks_ms(); buffer == NULL;) {
        buffer = framebuffer_get_head(FB_NO_FLAGS);
        if ((mp_hal_ticks_ms() - ticks) > 3000) {
            dcmi_abort();
            return -1;
        }
    }

    MAIN_FB()->w = MAIN_FB()->u;
    MAIN_FB()->h = MAIN_FB()->v;

    // Set the user image.
    if (image != NULL) {
        image->w      = MAIN_FB()->w;
        image->h      = MAIN_FB()->h;
        image->bpp    = MAIN_FB()->bpp;
        image->pixels = buffer->data;
    }

    return 0;
}
#endif
