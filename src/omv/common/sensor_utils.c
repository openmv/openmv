/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * This file contains image sensor driver utility functions and some default (weak)
 * implementations of common functions that can be replaced by port-specific drivers.
 */
#if MICROPY_PY_SENSOR
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mphal.h"
#include "cambus.h"
#include "sensor.h"
#include "ov2640.h"
#include "ov5640.h"
#include "ov7725.h"
#include "ov7670.h"
#include "ov7690.h"
#include "ov9650.h"
#include "mt9v0xx.h"
#include "mt9m114.h"
#include "lepton.h"
#include "hm01b0.h"
#include "paj6100.h"
#include "frogeye2020.h"
#include "gc2145.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"

#if (OMV_ENABLE_PAJ6100 == 1)
#define OMV_ENABLE_NONI2CIS
#endif

#ifndef __weak
#define __weak   __attribute__((weak))
#endif

// Sensor frame size/resolution table.
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

__weak void sensor_init0()
{
    // Reset the sesnor state
    memset(&sensor, 0, sizeof(sensor_t));
}

__weak int sensor_init()
{
    // Reset the sesnor state
    memset(&sensor, 0, sizeof(sensor_t));
    return SENSOR_ERROR_CTL_UNSUPPORTED;
}

__weak int sensor_abort()
{
    return SENSOR_ERROR_CTL_UNSUPPORTED;
}

__weak int sensor_reset()
{
    // Disable any ongoing frame capture.
    sensor_abort();

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
    sensor.auto_rotation        = (sensor.chip_id == OV7690_ID);
    #else
    sensor.auto_rotation        = false;
    #endif // MICROPY_PY_IMU
    sensor.vsync_callback       = NULL;
    sensor.frame_callback       = NULL;

    // Reset default color palette.
    sensor.color_palette        = rainbow_table;

    sensor.disable_full_flush   = false;

    // Restore shutdown state on reset.
    sensor_shutdown(false);

    // Disable the bus before reset.
    cambus_enable(&sensor.bus, false);

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

    // Re-enable the bus.
    cambus_enable(&sensor.bus, true);

    // Call sensor-specific reset function
    if (sensor.reset != NULL
            && sensor.reset(&sensor) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    // Reset framebuffers
    framebuffer_reset_buffers();

    return 0;
}

int sensor_probe_init(uint32_t bus_id, uint32_t bus_speed)
{
    int init_ret = 0;
    int freq;
    (void) freq;

    // Do a power cycle
    DCMI_PWDN_HIGH();
    mp_hal_delay_ms(10);

    DCMI_PWDN_LOW();
    mp_hal_delay_ms(10);

    /* Some sensors have different reset polarities, and we can't know which sensor
       is connected before initializing cambus and probing the sensor, which in turn
       requires pulling the sensor out of the reset state. So we try to probe the
       sensor with both polarities to determine line state. */
    sensor.pwdn_pol = ACTIVE_HIGH;
    sensor.reset_pol = ACTIVE_HIGH;

    // Reset the sensor
    DCMI_RESET_HIGH();
    mp_hal_delay_ms(10);

    DCMI_RESET_LOW();
    mp_hal_delay_ms(10);

    // Initialize the camera bus.
    cambus_init(&sensor.bus, bus_id, bus_speed);
    mp_hal_delay_ms(10);

    // Probe the sensor
    sensor.slv_addr = cambus_scan(&sensor.bus, NULL, 0);
    if (sensor.slv_addr == 0) {
        /* Sensor has been held in reset,
           so the reset line is active low */
        sensor.reset_pol = ACTIVE_LOW;

        // Pull the sensor out of the reset state.
        DCMI_RESET_HIGH();
        mp_hal_delay_ms(10);

        // Probe again to set the slave addr.
        sensor.slv_addr = cambus_scan(&sensor.bus, NULL, 0);
        if (sensor.slv_addr == 0) {
            sensor.pwdn_pol = ACTIVE_LOW;

            DCMI_PWDN_HIGH();
            mp_hal_delay_ms(10);

            sensor.slv_addr = cambus_scan(&sensor.bus, NULL, 0);
            if (sensor.slv_addr == 0) {
                sensor.reset_pol = ACTIVE_HIGH;

                DCMI_RESET_LOW();
                mp_hal_delay_ms(10);

                sensor.slv_addr = cambus_scan(&sensor.bus, NULL, 0);
                #ifndef OMV_ENABLE_NONI2CIS
                if (sensor.slv_addr == 0) {
                    return SENSOR_ERROR_ISC_UNDETECTED;
                }
                #endif
            }
        }
    }

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

        #if (OMV_ENABLE_MT9V0XX == 1)
        case MT9V0XX_SLV_ADDR:
            cambus_readw(&sensor.bus, sensor.slv_addr, ON_CHIP_ID, &sensor.chip_id_w);
            break;
        #endif //(OMV_ENABLE_MT9V0XX == 1)

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

        #if (OMV_ENABLE_GC2145 == 1)
        case GC2145_SLV_ADDR:
            cambus_readb(&sensor.bus, sensor.slv_addr, GC_CHIP_ID, &sensor.chip_id);
            break;
        #endif //(OMV_ENABLE_GC2145 == 1)

        #if (OMV_ENABLE_FROGEYE2020 == 1)
        case FROGEYE2020_SLV_ADDR:
            sensor.chip_id_w = FROGEYE2020_ID;
            sensor.pwdn_pol = ACTIVE_HIGH;
            sensor.reset_pol = ACTIVE_HIGH;
            break;
        #endif // (OMV_ENABLE_FROGEYE2020 == 1)

        #if (OMV_ENABLE_PAJ6100 == 1)
        case 0:
            if (paj6100_detect(&sensor)) {
                // Found PixArt PAJ6100
                sensor.chip_id_w = PAJ6100_ID;
                sensor.pwdn_pol = ACTIVE_LOW;
                sensor.reset_pol = ACTIVE_LOW;
                break;
            }
            return SENSOR_ERROR_ISC_UNDETECTED;
        #endif

        default:
            return SENSOR_ERROR_ISC_UNSUPPORTED;
            break;
    }

    switch (sensor.chip_id_w) {
        #if (OMV_ENABLE_OV2640 == 1)
        case OV2640_ID:
            if (sensor_set_xclk_frequency(OV2640_XCLK_FREQ) != 0) {
                return SENSOR_ERROR_TIM_INIT_FAILED;
            }
            init_ret = ov2640_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV2640 == 1)

        #if (OMV_ENABLE_OV5640 == 1)
        case OV5640_ID:
            freq = OMV_OV5640_XCLK_FREQ;
            #if (OMV_OV5640_REV_Y_CHECK == 1)
            if (HAL_GetREVID() < 0x2003) { // Is this REV Y?
                freq = OMV_OV5640_REV_Y_FREQ;
            }
            #endif
            if (sensor_set_xclk_frequency(freq) != 0) {
                return SENSOR_ERROR_TIM_INIT_FAILED;
            }
            init_ret = ov5640_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV5640 == 1)

        #if (OMV_ENABLE_OV7670 == 1)
        case OV7670_ID:
            if (sensor_set_xclk_frequency(OV7670_XCLK_FREQ) != 0) {
                return SENSOR_ERROR_TIM_INIT_FAILED;
            }
            init_ret = ov7670_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV7670 == 1)

        #if (OMV_ENABLE_OV7690 == 1)
        case OV7690_ID:
            if (sensor_set_xclk_frequency(OV7690_XCLK_FREQ) != 0) {
                return SENSOR_ERROR_TIM_INIT_FAILED;
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

        #if (OMV_ENABLE_MT9V0XX == 1)
        case MT9V0X2_ID_V_1:
        case MT9V0X2_ID_V_2:
            // Force old versions to the newest.
            sensor.chip_id_w = MT9V0X2_ID;
        case MT9V0X2_ID:
        case MT9V0X4_ID:
            if (sensor_set_xclk_frequency(MT9V0XX_XCLK_FREQ) != 0) {
                return SENSOR_ERROR_TIM_INIT_FAILED;
            }
            init_ret = mt9v0xx_init(&sensor);
            break;
        #endif //(OMV_ENABLE_MT9V0XX == 1)

        #if (OMV_ENABLE_MT9M114 == 1)
        case MT9M114_ID:
            if (sensor_set_xclk_frequency(MT9M114_XCLK_FREQ) != 0) {
                return SENSOR_ERROR_TIM_INIT_FAILED;
            }
            init_ret = mt9m114_init(&sensor);
            break;
        #endif //(OMV_ENABLE_MT9M114 == 1)

        #if (OMV_ENABLE_LEPTON == 1)
        case LEPTON_ID:
            if (sensor_set_xclk_frequency(LEPTON_XCLK_FREQ) != 0) {
                return SENSOR_ERROR_TIM_INIT_FAILED;
            }
            init_ret = lepton_init(&sensor);
            break;
        #endif // (OMV_ENABLE_LEPTON == 1)

        #if (OMV_ENABLE_HM01B0 == 1)
        case HM01B0_ID:
            if (sensor_set_xclk_frequency(HM01B0_XCLK_FREQ) != 0) {
                return SENSOR_ERROR_TIM_INIT_FAILED;
            }
            init_ret = hm01b0_init(&sensor);
            break;
        #endif //(OMV_ENABLE_HM01B0 == 1)

        #if (OMV_ENABLE_GC2145 == 1)
        case GC2145_ID:
            if (sensor_set_xclk_frequency(GC2145_XCLK_FREQ) != 0) {
                return SENSOR_ERROR_TIM_INIT_FAILED;
            }
            init_ret = gc2145_init(&sensor);
            break;
        #endif //(OMV_ENABLE_GC2145 == 1)

        #if (OMV_ENABLE_PAJ6100 == 1)
        case PAJ6100_ID:
            if (sensor_set_xclk_frequency(PAJ6100_XCLK_FREQ) != 0) {
                return SENSOR_ERROR_TIM_INIT_FAILED;
            }
            init_ret = paj6100_init(&sensor);
            break;
        #endif // (OMV_ENABLE_PAJ6100 == 1)

        #if (OMV_ENABLE_FROGEYE2020 == 1)
        case FROGEYE2020_ID:
            if (sensor_set_xclk_frequency(FROGEYE2020_XCLK_FREQ) != 0) {
                return SENSOR_ERROR_TIM_INIT_FAILED;
            }
            init_ret = frogeye2020_init(&sensor);
            break;
        #endif // (OMV_ENABLE_FROGEYE2020 == 1)

        default:
            return SENSOR_ERROR_ISC_UNSUPPORTED;
            break;
    }

    if (init_ret != 0 ) {
        // Sensor init failed.
        return SENSOR_ERROR_ISC_INIT_FAILED;
    }

    return 0;
}

__weak int sensor_get_id()
{
    return sensor.chip_id_w;
}

__weak uint32_t sensor_get_xclk_frequency()
{
    return SENSOR_ERROR_CTL_UNSUPPORTED;
}

__weak int sensor_set_xclk_frequency(uint32_t frequency)
{
    return SENSOR_ERROR_CTL_UNSUPPORTED;
}

__weak bool sensor_is_detected()
{
    return sensor.detected;
}

__weak int sensor_sleep(int enable)
{
    // Disable any ongoing frame capture.
    sensor_abort();

    // Check if the control is supported.
    if (sensor.sleep == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.sleep(&sensor, enable) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_shutdown(int enable)
{
    int ret = 0;

    // Disable any ongoing frame capture.
    sensor_abort();

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

__weak int sensor_read_reg(uint16_t reg_addr)
{
    int ret;

    // Check if the control is supported.
    if (sensor.read_reg == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if ((ret = sensor.read_reg(&sensor, reg_addr)) == -1) {
        return SENSOR_ERROR_IO_ERROR;
    }

    return ret;
}

__weak int sensor_write_reg(uint16_t reg_addr, uint16_t reg_data)
{
    // Check if the control is supported.
    if (sensor.write_reg == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.write_reg(&sensor, reg_addr, reg_data) == -1) {
        return SENSOR_ERROR_IO_ERROR;
    }

    return 0;
}

__weak int sensor_set_pixformat(pixformat_t pixformat)
{
    // Check if the value has changed.
    if (sensor.pixformat == pixformat) {
        return 0;
    }

    // Some sensor drivers automatically switch to BAYER to reduce the frame size if it does not fit in RAM.
    // If the current format is BAYER (1BPP), and the target format is color and (2BPP), and the frame does not
    // fit in RAM it will just be switched back again to BAYER, so we keep the current format unchanged.
    uint32_t size = framebuffer_get_buffer_size();
    if ((sensor.pixformat == PIXFORMAT_BAYER)
            && ((pixformat == PIXFORMAT_RGB565) || (pixformat == PIXFORMAT_YUV422))
            && (MAIN_FB()->u * MAIN_FB()->v * 2 > size)
            && (MAIN_FB()->u * MAIN_FB()->v * 1 <= size)) {
        return 0;
    }

    // Cropping and transposing (and thus auto rotation) don't work in JPEG mode.
    if ((pixformat == PIXFORMAT_JPEG)
            && (sensor_get_cropped() || sensor.transpose || sensor.auto_rotation)) {
        return SENSOR_ERROR_PIXFORMAT_UNSUPPORTED;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

    // Check if the control is supported.
    if (sensor.set_pixformat == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_pixformat(&sensor, pixformat) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    mp_hal_delay_ms(100); // wait for the camera to settle

    // Set pixel format
    sensor.pixformat = pixformat;

    // Skip the first frame.
    MAIN_FB()->pixfmt = PIXFORMAT_INVALID;

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers();

    // Reconfigure the DCMI if needed.
    return sensor_dcmi_config(pixformat);
}

__weak int sensor_set_framesize(framesize_t framesize)
{
    if (sensor.framesize == framesize) {
        // No change
        return 0;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

    // Call the sensor specific function
    if (sensor.set_framesize == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    if (sensor.set_framesize(&sensor, framesize) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    mp_hal_delay_ms(100); // wait for the camera to settle

    // Set framebuffer size
    sensor.framesize = framesize;

    // Skip the first frame.
    MAIN_FB()->pixfmt = PIXFORMAT_INVALID;

    // Set MAIN FB x offset, y offset, width, height, backup width, and backup height.
    MAIN_FB()->x = 0;
    MAIN_FB()->y = 0;
    MAIN_FB()->w = MAIN_FB()->u = resolution[framesize][0];
    MAIN_FB()->h = MAIN_FB()->v = resolution[framesize][1];

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers();

    return 0;
}

__weak int sensor_set_framerate(int framerate)
{
    if (sensor.framerate == framerate) {
        // No change
        return 0;
    }

    if (framerate < 0) {
        return SENSOR_ERROR_INVALID_ARGUMENT;
    }

    // If the sensor implements framerate control use it.
    if (sensor.set_framerate != NULL
            && sensor.set_framerate(&sensor, framerate) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    } else {
        // Otherwise use software framerate control.
        sensor.framerate = framerate;
    }
    return 0;
}

__weak bool sensor_get_cropped()
{
    if (sensor.framesize != FRAMESIZE_INVALID) {
        return (MAIN_FB()->x != 0)                                  // should be zero if not cropped.
            || (MAIN_FB()->y != 0)                                  // should be zero if not cropped.
            || (MAIN_FB()->u != resolution[sensor.framesize][0])    // should be equal to the resolution if not cropped.
            || (MAIN_FB()->v != resolution[sensor.framesize][1]);   // should be equal to the resolution if not cropped.
    }
    return false;
}


__weak uint32_t sensor_get_src_bpp()
{
    switch (sensor.pixformat) {
        case PIXFORMAT_GRAYSCALE:
            return sensor.hw_flags.gs_bpp;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            return 2;
        case PIXFORMAT_BAYER:
        case PIXFORMAT_JPEG:
            return 1;
        default:
            return 0;
    }
}

__weak uint32_t sensor_get_dst_bpp()
{
    switch (sensor.pixformat) {
        case PIXFORMAT_GRAYSCALE:
        case PIXFORMAT_BAYER:
            return 1;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            return 2;
        default:
            return 0;
    }
}

__weak int sensor_set_windowing(int x, int y, int w, int h)
{
    // Check if the value has changed.
    if ((MAIN_FB()->x == x) && (MAIN_FB()->y == y) &&
            (MAIN_FB()->u == w) && (MAIN_FB()->v == h)) {
        return 0;
    }

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return SENSOR_ERROR_PIXFORMAT_UNSUPPORTED;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

    // Skip the first frame.
    MAIN_FB()->pixfmt = PIXFORMAT_INVALID;

    MAIN_FB()->x = x;
    MAIN_FB()->y = y;
    MAIN_FB()->w = MAIN_FB()->u = w;
    MAIN_FB()->h = MAIN_FB()->v = h;

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers();

    return 0;
}

__weak int sensor_set_contrast(int level)
{
    // Check if the control is supported.
    if (sensor.set_contrast == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_contrast(&sensor, level) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_set_brightness(int level)
{
    // Check if the control is supported.
    if (sensor.set_brightness == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_brightness(&sensor, level) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_set_saturation(int level)
{
    // Check if the control is supported.
    if (sensor.set_saturation == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_saturation(&sensor, level) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_set_gainceiling(gainceiling_t gainceiling)
{
    // Check if the value has changed.
    if (sensor.gainceiling == gainceiling) {
        return 0;
    }

    // Check if the control is supported.
    if (sensor.set_gainceiling == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_gainceiling(&sensor, gainceiling) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    // Set the new control value.
    sensor.gainceiling = gainceiling;

    return 0;
}

__weak int sensor_set_quality(int qs)
{
    // Check if the control is supported.
    if (sensor.set_quality == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_quality(&sensor, qs) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_set_colorbar(int enable)
{
    // Check if the control is supported.
    if (sensor.set_colorbar == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_colorbar(&sensor, enable) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_set_auto_gain(int enable, float gain_db, float gain_db_ceiling)
{
    // Check if the control is supported.
    if (sensor.set_auto_gain == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_auto_gain(&sensor, enable, gain_db, gain_db_ceiling) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_get_gain_db(float *gain_db)
{
    // Check if the control is supported.
    if (sensor.get_gain_db == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.get_gain_db(&sensor, gain_db) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_set_auto_exposure(int enable, int exposure_us)
{
    // Check if the control is supported.
    if (sensor.set_auto_exposure == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_auto_exposure(&sensor, enable, exposure_us) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_get_exposure_us(int *exposure_us)
{
    // Check if the control is supported.
    if (sensor.get_exposure_us == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.get_exposure_us(&sensor, exposure_us) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_set_auto_whitebal(int enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
    // Check if the control is supported.
    if (sensor.set_auto_whitebal == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_auto_whitebal(&sensor, enable, r_gain_db, g_gain_db, b_gain_db) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_get_rgb_gain_db(float *r_gain_db, float *g_gain_db, float *b_gain_db)
{
    // Check if the control is supported.
    if (sensor.get_rgb_gain_db == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.get_rgb_gain_db(&sensor, r_gain_db, g_gain_db, b_gain_db) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_set_hmirror(int enable)
{
    // Check if the value has changed.
    if (sensor.hmirror == ((bool) enable)) {
        return 0;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    // Check if the control is supported.
    if (sensor.set_hmirror == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_hmirror(&sensor, enable) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    // Set the new control value.
    sensor.hmirror = enable;

    // Wait for the camera to settle
    mp_hal_delay_ms(100);

    return 0;
}

__weak bool sensor_get_hmirror()
{
    return sensor.hmirror;
}

__weak int sensor_set_vflip(int enable)
{
    // Check if the value has changed.
    if (sensor.vflip == ((bool) enable)) {
        return 0;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    // Check if the control is supported.
    if (sensor.set_vflip == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_vflip(&sensor, enable) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    // Set the new control value.
    sensor.vflip = enable;

    // Wait for the camera to settle
    mp_hal_delay_ms(100);

    return 0;
}

__weak bool sensor_get_vflip()
{
    return sensor.vflip;
}

__weak int sensor_set_transpose(bool enable)
{
    // Check if the value has changed.
    if (sensor.transpose == enable) {
        return 0;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return SENSOR_ERROR_PIXFORMAT_UNSUPPORTED;
    }

    // Set the new control value.
    sensor.transpose = enable;

    return 0;
}

__weak bool sensor_get_transpose()
{
    return sensor.transpose;
}

__weak int sensor_set_auto_rotation(bool enable)
{
    // Check if the value has changed.
    if (sensor.auto_rotation == enable) {
        return 0;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    // Operation not supported on JPEG images.
    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return SENSOR_ERROR_PIXFORMAT_UNSUPPORTED;
    }

    // Set the new control value.
    sensor.auto_rotation = enable;
    return 0;
}

__weak bool sensor_get_auto_rotation()
{
    return sensor.auto_rotation;
}

__weak int sensor_set_framebuffers(int count)
{
    // Disable any ongoing frame capture.
    sensor_abort();

    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

    return framebuffer_set_buffers(count);
}

__weak int sensor_set_special_effect(sde_t sde)
{
    // Check if the value has changed.
    if (sensor.sde == sde) {
        return 0;
    }

    // Check if the control is supported.
    if (sensor.set_special_effect == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_special_effect(&sensor, sde) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    // Set the new control value.
    sensor.sde = sde;

    return 0;
}

__weak int sensor_set_lens_correction(int enable, int radi, int coef)
{
    // Check if the control is supported.
    if (sensor.set_lens_correction == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_lens_correction(&sensor, enable, radi, coef) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int sensor_ioctl(int request, ... /* arg */)
{
    // Disable any ongoing frame capture.
    sensor_abort();

    // Check if the control is supported.
    if (sensor.ioctl == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    va_list ap;
    va_start(ap, request);
    // Call the sensor specific function.
    int ret = sensor.ioctl(&sensor, request, ap);
    va_end(ap);

    return ((ret != 0) ? SENSOR_ERROR_CTL_FAILED : 0);
}

__weak int sensor_set_vsync_callback(vsync_cb_t vsync_cb)
{
    sensor.vsync_callback = vsync_cb;
    return 0;
}

__weak int sensor_set_frame_callback(frame_cb_t vsync_cb)
{
    sensor.frame_callback = vsync_cb;
    return 0;
}

__weak int sensor_set_color_palette(const uint16_t *color_palette)
{
    sensor.color_palette = color_palette;
    return 0;
}

__weak const uint16_t *sensor_get_color_palette()
{
    return sensor.color_palette;
}

__weak int sensor_check_framebuffer_size()
{
    uint32_t bpp = sensor_get_dst_bpp();
    uint32_t size = framebuffer_get_buffer_size();
    return (((MAIN_FB()->u * MAIN_FB()->v * bpp) <= size) ? 0 : -1);
}

__weak int sensor_auto_crop_framebuffer()
{
    uint32_t bpp = sensor_get_dst_bpp();
    uint32_t size = framebuffer_get_buffer_size();

    // If the pixformat is NULL/JPEG there we can't do anything to check if it fits before hand.
    if (!bpp) {
        return 0;
    }

    // MAIN_FB() fits, we are done.
    if ((MAIN_FB()->u * MAIN_FB()->v * bpp) <= size) {
        return 0;
    }

    if ((sensor.pixformat == PIXFORMAT_RGB565) || (sensor.pixformat == PIXFORMAT_YUV422)) {
        // Switch to bayer for the quick 2x savings.
        sensor_set_pixformat(PIXFORMAT_BAYER);
        bpp = 1;

        // MAIN_FB() fits, we are done (bpp is 1).
        if ((MAIN_FB()->u * MAIN_FB()->v) <= size) {
            return 0;
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

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers();
    return 0;
}

const char *sensor_strerror(int error)
{
    static const char *sensor_errors[] = {
        "No error.",
        "Sensor control failed.",
        "The requested operation is not supported by the image sensor.",
        "Failed to detect the image sensor or image sensor is detached.",
        "The detected image sensor is not supported.",
        "Failed to initialize the image sensor.",
        "Failed to initialize the image sensor clock.",
        "Failed to initialize the image sensor DMA.",
        "Failed to initialize the image sensor DCMI.",
        "An low level I/O error has occurred.",
        "Frame capture has failed.",
        "Frame capture has timed out.",
        "Frame size is not supported or is not set.",
        "Pixel format is not supported or is not set.",
        "Window is not supported or is not set.",
        "Frame rate is not supported or is not set.",
        "An invalid argument is used.",
        "The requested operation is not supported on the current pixel format.",
        "Frame buffer error.",
        "Frame buffer overflow, try reducing the frame size.",
        "JPEG frame buffer overflow.",
    };

    // Sensor errors are negative.
    error = ((error < 0) ? (error * -1) : error);

    if (error > (sizeof(sensor_errors) / sizeof(sensor_errors[0]))) {
        return "Unknown error.";
    } else {
        return sensor_errors[error];
    }
}

__weak int sensor_snapshot(sensor_t *sensor, image_t *image, uint32_t flags)
{
    return -1;
}
#endif //MICROPY_PY_SENSOR
