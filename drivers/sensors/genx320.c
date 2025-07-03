/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2025 OpenMV, LLC.
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
 * GENX320 driver.
 */
#include "omv_boardconfig.h"
#if (OMV_GENX320_ENABLE == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "py/mphal.h"
#include "framebuffer.h"
#include "omv_i2c.h"
#include "omv_csi.h"

#include "evt_2_0.h"
#include "psee_genx320.h"

#define BLANK_LINES                     4
#define BLANK_COLUMNS                   4

#define SENSOR_WIDTH                    324
#define SENSOR_HEIGHT                   324

#define ACTIVE_SENSOR_WIDTH             (SENSOR_WIDTH - BLANK_COLUMNS)
#define ACTIVE_SENSOR_HEIGHT            (SENSOR_HEIGHT - BLANK_LINES)
#define ACTIVE_SENSOR_SIZE              (ACTIVE_SENSOR_WIDTH * ACTIVE_SENSOR_HEIGHT)

#define HISTO_HSYNC_CLOCK_CYCLES        880 // 320 + 880 = 1200 cycles -> ~122 FPS
#define EVENT_HSYNC_CLOCK_CYCLES        280 // 320 + 280 = 600 cycles
#define VSYNC_CLOCK_CYCLES              8

#define CONTRAST_DEFAULT                16
#define BRIGHTNESS_DEFAULT              128

#define I2C_TIMEOUT                     1000

#define INTEGRATION_DEF_PREIOD          20000 // 20ms (50 FPS)
#define INTEGRATION_MIN_PREIOD          0x10 // 16us
#define INTEGRATION_MAX_PREIOD          0x1FFF0 // 131056us

#define FPS_TO_US(fps)                  (1000000 / (fps))

#define EVT_CLK_MULTIPLIER              (2)
#define EVT_CLK_FREQ \
    (((omv_csi_get_clk_frequency(csi, false) * EVT_CLK_MULTIPLIER) + 500000) / 1000000)

#define AFK_50_HZ                       (50)
#define AFK_60_HZ                       (60)
#define AFK_LOW_FREQ                    IM_MIN(AFK_50_HZ, AFK_60_HZ)
#define AFK_HIGH_FREQ                   IM_MAX(AFK_50_HZ, AFK_60_HZ)
#define AFK_LOW_BAND                    ((AFK_LOW_FREQ * 2) - 10)
#define AFK_HIGH_BAND                   ((AFK_HIGH_FREQ * 2) + 10)

#define EHC_DIFF3D_N_BITS_SIZE          (7) // signed 8-bit value

static bool reset_called = false;
static bool event_mode = false;
static int32_t contrast = CONTRAST_DEFAULT;
static int32_t brightness = BRIGHTNESS_DEFAULT;
static uint64_t event_time = 0;

static AFK_HandleTypeDef psee_afk;

static int set_event_mode(omv_csi_t *csi, bool enable) {
    if (reset_called) {
        if (event_mode == enable) {
            return 0;
        }

        psee_sensor_stop(csi, event_mode ? &dcmi_evt : &dcmi_histo);
        psee_sensor_destroy(csi);
    }

    BIAS_Params_t biases = (csi->chip_id == SAPHIR_ES_ID) ? genx320es_default_biases : genx320mp_default_biases;

    // Force CPI with chicken bits
    psee_sensor_write(csi, TOP_CHICKEN, TOP_CHICKEN_OVERRIDE_MIPI_MODE_EN |
                      TOP_CHICKEN_OVERRIDE_HISTO_MODE_EN |
                      (!enable) << TOP_CHICKEN_OVERRIDE_HISTO_MODE_Pos |
                      I2C_TIMEOUT << TOP_CHICKEN_I2C_TIMEOUT_Pos);

    // Start the Init sequence
    psee_sensor_init(csi, enable ? &dcmi_evt : &dcmi_histo);

    if (enable) {
        // Set EVT20 mode
        psee_sensor_write(csi, EDF_CONTROL, 0);
    }

    // Configure Packet and Frame sizes
    int packet_width = enable ? 128 : ACTIVE_SENSOR_WIDTH;
    int packet_height = enable ? 64 : ACTIVE_SENSOR_HEIGHT;
    int packet_hsync = enable ? EVENT_HSYNC_CLOCK_CYCLES : HISTO_HSYNC_CLOCK_CYCLES;
    psee_sensor_write(csi, CPI_PACKET_SIZE_CONTROL, packet_width);
    psee_sensor_write(csi, CPI_PACKET_TIME_CONTROL, packet_width << CPI_PACKET_TIME_CONTROL_PERIOD_Pos |
                      packet_hsync << CPI_PACKET_TIME_CONTROL_BLANKING_Pos);
    psee_sensor_write(csi, CPI_FRAME_SIZE_CONTROL, packet_height);
    psee_sensor_write(csi, CPI_FRAME_TIME_CONTROL, VSYNC_CLOCK_CYCLES);

    // Enable dropping
    psee_sensor_write(csi, RO_READOUT_CTRL, RO_READOUT_CTRL_DIGITAL_PIPE_EN |
                      RO_READOUT_CTRL_AVOID_BPRESS_TD |
                      RO_READOUT_CTRL_DROP_EN |
                      RO_READOUT_CTRL_DROP_ON_FULL_EN);

    // Enable the Anti-FlicKering filter
    if (psee_afk_init(csi, &psee_afk) != AFK_OK) {
        return -1;
    }

    if (psee_afk_activate(&psee_afk, AFK_LOW_BAND, AFK_HIGH_BAND, EVT_CLK_FREQ) != AFK_OK) {
        return -1;
    }

    if (enable) {
        // Operation Mode Configuration
        psee_PM3C_config(csi);
    } else {
        // Operation Mode Configuration
        psee_PM3C_Histo_config(csi);
    }

    // Set the default border for the Activity map
    psee_set_default_XY_borders(csi, &genx320mp_default_am_borders);

    // Configure the activity map
    psee_configure_activity_map(csi);

    // Set Standard biases
    psee_sensor_set_biases(csi, &biases);

    if (enable) {
        // Start the csi
        psee_sensor_start(csi, &dcmi_evt);
    } else {
        // Start the csi
        psee_sensor_start(csi, &dcmi_histo);

        EHC_HandleTypeDef psee_ehc;

        if (psee_ehc_init(csi, &psee_ehc) != EHC_OK) {
            return -1;
        }

        if (psee_ehc_activate(&psee_ehc, EHC_ALGO_DIFF3D, 0, EHC_DIFF3D_N_BITS_SIZE,
                              INTEGRATION_DEF_PREIOD, EHC_WITHOUT_PADDING) != EHC_OK) {
            return -1;
        }
    }

    reset_called = true;
    event_mode = enable;

    return 0;
}

static int reset(omv_csi_t *csi) {
    csi->color_palette = NULL;

    reset_called = false;
    event_mode = false;
    contrast = CONTRAST_DEFAULT;
    brightness = BRIGHTNESS_DEFAULT;
    event_time = 0;

    // Set histogram mode by default.
    return set_event_mode(csi, false);
}

static int sleep(omv_csi_t *csi, int enable) {
    if (enable) {
        if (!event_mode) {
            psee_PM2_Histo_config(csi);
        } else {
            psee_PM2_config(csi);
        }
    } else {
        if (!event_mode) {
            psee_PM3C_Histo_config(csi);
        } else {
            psee_PM3C_config(csi);
        }
    }
    return 0;
}

static int read_reg(omv_csi_t *csi, uint16_t reg_addr) {
    uint32_t reg_data;
    uint8_t addr[] = {(reg_addr >> 8), reg_addr};
    if (omv_i2c_write_bytes(csi->i2c, csi->slv_addr, addr, 2, OMV_I2C_XFER_NO_STOP) != 0) {
        return -1;
    }
    if (omv_i2c_read_bytes(csi->i2c, csi->slv_addr, (uint8_t *) &reg_data, 4, OMV_I2C_XFER_NO_FLAGS) != 0) {
        return -1;
    }
    reg_data = __REV(reg_data);
    return reg_data;
}

static int write_reg(omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data) {
    uint8_t buf[] = {(reg_addr >> 8), reg_addr, (reg_data >> 24), (reg_data >> 16), (reg_data >> 8), reg_data};
    return omv_i2c_write_bytes(csi->i2c, csi->slv_addr, buf, 6, OMV_I2C_XFER_NO_FLAGS);
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    return (pixformat == PIXFORMAT_GRAYSCALE) ? 0 : -1;
}

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    return ((framesize == OMV_CSI_FRAMESIZE_128X64) || (framesize == OMV_CSI_FRAMESIZE_320X320)) ? 0 : -1;
}

static int set_framerate(omv_csi_t *csi, int framerate) {
    if (event_mode) {
        return -1;
    }

    int us = FPS_TO_US(framerate);

    if (us < INTEGRATION_MIN_PREIOD) {
        return -1;
    }

    if (us > INTEGRATION_MAX_PREIOD) {
        return -1;
    }

    int lines = ACTIVE_SENSOR_HEIGHT + VSYNC_CLOCK_CYCLES;
    int clocks_per_frame = (omv_csi_get_clk_frequency(csi, false) * EVT_CLK_MULTIPLIER) / framerate;
    int hsync_clocks = (clocks_per_frame / lines) - ACTIVE_SENSOR_WIDTH;

    if (hsync_clocks <= 0) {
        return -1;
    }

    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    psee_sensor_write(csi, EHC_INTEGRATION_PERIOD, us);
    psee_sensor_write(csi, CPI_PACKET_TIME_CONTROL, ACTIVE_SENSOR_WIDTH << CPI_PACKET_TIME_CONTROL_PERIOD_Pos |
                      hsync_clocks << CPI_PACKET_TIME_CONTROL_BLANKING_Pos);

    // Wait for the camera to settle
    if (!csi->disable_delays) {
        mp_hal_delay_ms(100);
    }

    return 0;
}

static int set_contrast(omv_csi_t *csi, int level) {
    contrast = __USAT(level, UINT8_T_BITS);
    return 0;
}

static int set_brightness(omv_csi_t *csi, int level) {
    brightness = __USAT(level, UINT8_T_BITS);
    return 0;
}

static int set_colorbar(omv_csi_t *csi, int enable) {
    if (event_mode) {
        return -1;
    }

    uint32_t reg;
    psee_sensor_read(csi, RO_READOUT_CTRL, &reg);
    reg = (reg & ~RO_READOUT_CTRL_ERC_SELF_TEST_EN) | (enable ? RO_READOUT_CTRL_ERC_SELF_TEST_EN : 0);
    psee_sensor_write(csi, RO_READOUT_CTRL, reg);
    return 0;
}

static int set_hmirror(omv_csi_t *csi, int enable) {
    psee_sensor_set_flip(csi, enable, csi->vflip);
    return 0;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    psee_sensor_set_flip(csi, csi->hmirror, enable);
    return 0;
}

static int disable_hot_pixels(omv_csi_t *csi, uint8_t *histogram, float sigma) {
    // Compute average
    int32_t avg = 0;

    for (uint32_t i = 0; i < ACTIVE_SENSOR_SIZE; i++) {
        avg += histogram[i];
    }

    avg /= ACTIVE_SENSOR_SIZE;

    // Compute std
    int64_t std = 0;

    for (uint32_t i = 0; i < ACTIVE_SENSOR_SIZE; i++) {
        int32_t diff = histogram[i] - avg;
        std += diff * diff;
    }

    std = fast_sqrtf(std / ((float) ACTIVE_SENSOR_SIZE));

    int32_t threshold = fast_roundf(avg + (std * sigma));
    int ret = 0;

    for (uint32_t y = 0; y < ACTIVE_SENSOR_HEIGHT; y++) {
        // Reset all blocks
        for (uint32_t i = 0; i < (ACTIVE_SENSOR_WIDTH / UINT32_T_BITS); i++) {
            psee_write_ROI_X(csi, i * sizeof(uint32_t), 0);
        }

        // Select line
        uint32_t offset = y / UINT32_T_BITS;
        psee_write_ROI_Y(csi, offset * sizeof(uint32_t), 1 << (y % UINT32_T_BITS));

        // Trigger shadow
        psee_write_ROI_CTRL(csi, ROI_CTRL_PX_SW_RSTN | ROI_CTRL_TD_SHADOW_TRIGGER);

        uint32_t tmp[ACTIVE_SENSOR_WIDTH / UINT32_T_BITS] = {};
        for (uint32_t x = 0; x < ACTIVE_SENSOR_WIDTH; x++) {
            if (histogram[(y * ACTIVE_SENSOR_WIDTH) + x] > threshold) {
                tmp[x / UINT32_T_BITS] |= 1 << (x % UINT32_T_BITS);
                ret += 1;
            }
        }

        // Write x values to disable
        for (uint32_t i = 0; i < (ACTIVE_SENSOR_WIDTH / UINT32_T_BITS); i++) {
            psee_write_ROI_X(csi, i * sizeof(uint32_t), tmp[i]);
        }

        // Activate block
        psee_write_ROI_CTRL(csi, ROI_CTRL_PX_SW_RSTN | ROI_CTRL_TD_SHADOW_TRIGGER | ROI_CTRL_TD_EN);

        // Disable roi block
        psee_write_ROI_CTRL(csi, ROI_CTRL_PX_SW_RSTN);
        psee_write_ROI_Y(csi, offset * sizeof(uint32_t), 0);

        mp_printf(MP_PYTHON_PRINTER, "CSI: Calibrating - %d%%\n", ((y * 50) / ACTIVE_SENSOR_HEIGHT) + 50);
    }

    return ret;
}

static void histo_post_process(omv_csi_t *csi, image_t *image) {
    for (uint32_t i = 0; i < ACTIVE_SENSOR_SIZE; i++) {
        image->data[i] = __USAT((((int8_t *) image->data)[i] * contrast) + brightness, UINT8_T_BITS);
    }

    if (csi->color_palette &&
        (framebuffer_get_buffer_size(csi->fb) >= (ACTIVE_SENSOR_SIZE * sizeof(uint16_t)))) {
        for (int32_t i = ACTIVE_SENSOR_SIZE - 1; i >= 0; i--) {
            ((uint16_t *) image->data)[i] = csi->color_palette[image->data[i]];
        }
        image->pixfmt = PIXFORMAT_RGB565;
        csi->fb->pixfmt = PIXFORMAT_RGB565;
    }
}

static int event_post_process(ec_event_t *array, image_t *image) {
    int event_count = (image->w * image->h) / sizeof(uint32_t);
    int valid_count = 0;

    for (int i = 0; i < event_count; i++) {
        uint32_t val = ((uint32_t *) image->data)[i];
        uint32_t type = __EVT20_TYPE(val);
        switch (type) {
            case TD_LOW:
            case TD_HIGH: {
                ec_event_t *event = array + valid_count++;
                uint64_t t = __EVT20_TIME(event_time, __EVT20_TS(val));
                event->type = EC_PIXEL_EVENT(type);
                event->ts_s = EC_TS_S(t);
                event->ts_ms = EC_TS_MS(t);
                event->ts_us = EC_TS_US(t);
                event->x = __EVT20_X(val);
                event->y = __EVT20_Y(val);
                break;
            }
            case EV_TIME_HIGH: {
                event_time = __EVT20_TIME_HIGH(val);
                break;
            }
            case EXT_TRIGGER: {
                ec_event_t *event = array + valid_count++;
                uint64_t t = __EVT20_TIME(event_time, __EVT20_TS(val));
                event->type = EC_TRIGGER_EVENT(__EVT20_TRIGGER_ID(val), __EVT20_TRIGGER_POLARITY(val));
                event->ts_s = EC_TS_S(t);
                event->ts_ms = EC_TS_MS(t);
                event->ts_us = EC_TS_US(t);
                event->x = 0;
                event->y = 0;
                break;
            }
            default: {
                break;
            }
        }
    }

    return valid_count;
}

static int snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    // Set histogram mode if not already set.
    int ret = set_event_mode(csi, false);
    if (ret < 0) {
        return ret;
    }

    ret = omv_csi_set_pixformat(csi, PIXFORMAT_GRAYSCALE);
    if (ret < 0) {
        return ret;
    }

    ret = omv_csi_set_framesize(csi, OMV_CSI_FRAMESIZE_320X320);
    if (ret < 0) {
        return ret;
    }

    if (csi->transpose) {
        return OMV_CSI_ERROR_CAPTURE_FAILED;
    }

    ret = ((omv_csi_snapshot_t) csi->priv)(csi, image, flags);
    if (ret < 0) {
        return ret;
    }

    histo_post_process(csi, image);
    return ret;
}

static int ioctl(omv_csi_t *csi, int request, va_list ap) {
    int ret = 0;

    switch (request) {
        // Setting a preset of biases tuned for a particular application/condition
        case OMV_CSI_IOCTL_GENX320_SET_BIASES: {
            int mode = va_arg(ap, int);
            switch (mode) {
                case OMV_CSI_GENX320_BIASES_DEFAULT: {
                    // Set default biases V2.0.0
                    psee_sensor_set_bias(csi, DIFF, 51);
                    psee_sensor_set_bias(csi, DIFF_OFF, 28);
                    psee_sensor_set_bias(csi, DIFF_ON, 25);
                    psee_sensor_set_bias(csi, FO, 34);
                    psee_sensor_set_bias(csi, HPF, 40);
                    psee_sensor_set_bias(csi, REFR, 10);
                    break;
                }
                case OMV_CSI_GENX320_BIASES_LOW_LIGHT: {
                    // Set biases tuned for low light
                    psee_sensor_set_bias(csi, DIFF, 51);
                    psee_sensor_set_bias(csi, DIFF_OFF, 19);
                    psee_sensor_set_bias(csi, DIFF_ON, 24);
                    psee_sensor_set_bias(csi, FO, 19);
                    psee_sensor_set_bias(csi, HPF, 0);
                    psee_sensor_set_bias(csi, REFR, 10);
                    break;
                }
                case OMV_CSI_GENX320_BIASES_ACTIVE_MARKER: {
                    // Set biases tuned for active marker
                    psee_sensor_set_bias(csi, DIFF, 51);
                    psee_sensor_set_bias(csi, DIFF_OFF, 45); //127
                    psee_sensor_set_bias(csi, DIFF_ON, 55); //78
                    psee_sensor_set_bias(csi, FO, 50);
                    psee_sensor_set_bias(csi, HPF, 127);
                    psee_sensor_set_bias(csi, REFR, 0);
                    break;
                }
                case OMV_CSI_GENX320_BIASES_LOW_NOISE: {
                    // Set low sensitivity low noise biases
                    psee_sensor_set_bias(csi, DIFF, 51);
                    psee_sensor_set_bias(csi, DIFF_OFF, 38);
                    psee_sensor_set_bias(csi, DIFF_ON, 35);
                    psee_sensor_set_bias(csi, FO, 24);
                    psee_sensor_set_bias(csi, HPF, 40);
                    psee_sensor_set_bias(csi, REFR, 10);
                    break;
                }
                case OMV_CSI_GENX320_BIASES_HIGH_SPEED: {
                    // Set biases tuned for high speed motion
                    psee_sensor_set_bias(csi, DIFF, 51);
                    psee_sensor_set_bias(csi, DIFF_OFF, 26);
                    psee_sensor_set_bias(csi, DIFF_ON, 37);
                    psee_sensor_set_bias(csi, FO, 38);
                    psee_sensor_set_bias(csi, HPF, 74);
                    psee_sensor_set_bias(csi, REFR, 25);
                    break;
                }
                default: {
                    ret = -1;
                    break;
                }
            }
            break;
        }
        // Setting biases one by one
        case OMV_CSI_IOCTL_GENX320_SET_BIAS: {
            int bias_name = va_arg(ap, int);
            int bias_value = va_arg(ap, int);
            switch (bias_name) {
                case OMV_CSI_GENX320_BIAS_DIFF_OFF: {
                    psee_sensor_set_bias(csi, DIFF_OFF, bias_value);
                    break;
                }
                case OMV_CSI_GENX320_BIAS_DIFF_ON: {
                    psee_sensor_set_bias(csi, DIFF_ON, bias_value);
                    break;
                }
                case OMV_CSI_GENX320_BIAS_FO: {
                    psee_sensor_set_bias(csi, FO, bias_value);
                    break;
                }
                case OMV_CSI_GENX320_BIAS_HPF: {
                    psee_sensor_set_bias(csi, HPF, bias_value);
                    break;
                }
                case OMV_CSI_GENX320_BIAS_REFR: {
                    psee_sensor_set_bias(csi, REFR, bias_value);
                    break;
                }
                default: {
                    ret = -1;
                    break;
                }
            }
            break;
        }
        // Controlling AFK filter
        case OMV_CSI_IOCTL_GENX320_SET_AFK: {
            int mode = va_arg(ap, int);
            if (mode == 0) {
                // Disable AFK
                if (psee_afk_get_state(&psee_afk) != AFK_STATE_RESET) {
                    if (psee_afk_deactivate(&psee_afk) != AFK_OK) {
                        ret = -1;
                    }
                }
            } else {
                // Enable AFK
                int freq_min = va_arg(ap, int);
                int freq_max = va_arg(ap, int);
                if (psee_afk_init(csi, &psee_afk) != AFK_OK) {
                    ret = -1;
                }
                if (psee_afk_activate(&psee_afk, freq_min, freq_max, EVT_CLK_FREQ) != AFK_OK) {
                    ret = -1;
                }
            }
            break;
        }
        case OMV_CSI_IOCTL_GENX320_READ_EVENTS: {
            // Set event mode if not already set.
            ret = set_event_mode(csi, true);
            if (ret < 0) {
                break;
            }

            // 1 byte per pixel.
            ret = omv_csi_set_pixformat(csi, PIXFORMAT_GRAYSCALE);
            if (ret < 0) {
                break;
            }

            // 128*64 bytes / 4 bytes per event = 2048 events.
            ret = omv_csi_set_framesize(csi, OMV_CSI_FRAMESIZE_128X64);
            if (ret < 0) {
                break;
            }

            if (csi->transpose) {
                return OMV_CSI_ERROR_CAPTURE_FAILED;
            }

            image_t image;
            ret = ((omv_csi_snapshot_t) csi->priv)(csi, &image, 0);
            if (ret < 0) {
                break;
            }

            // Invalidate frame.
            csi->fb->pixfmt = PIXFORMAT_INVALID;

            ret = event_post_process((ec_event_t *) va_arg(ap, ec_event_t *), &image);
            break;
        }
        case OMV_CSI_IOCTL_GENX320_CALIBRATE: {
            uint32_t event_count = va_arg(ap, uint32_t);
            float sigma = va_arg(ap, double);

            if (!reset_called) {
                return OMV_CSI_ERROR_CAPTURE_FAILED;
            }

            if (csi->pixformat != PIXFORMAT_GRAYSCALE) {
                return OMV_CSI_ERROR_INVALID_PIXFORMAT;
            }

            if ((csi->framesize != OMV_CSI_FRAMESIZE_128X64) && (csi->framesize != OMV_CSI_FRAMESIZE_320X320)) {
                return OMV_CSI_ERROR_INVALID_FRAMESIZE;
            }

            if (csi->transpose) {
                return OMV_CSI_ERROR_CAPTURE_FAILED;
            }

            uint8_t *histogram = fb_alloc0(ACTIVE_SENSOR_SIZE, FB_ALLOC_NO_HINT);

            // Collect events to calibrate hot pixels.
            for (uint32_t i = 0; i < event_count; ) {
                image_t image;
                ret = ((omv_csi_snapshot_t) csi->priv)(csi, &image, 0);
                if (ret < 0) {
                    return ret;
                }

                // Invalidate frame.
                csi->fb->pixfmt = PIXFORMAT_INVALID;

                if (!event_mode) {
                    for (uint32_t j = 0; j < ACTIVE_SENSOR_SIZE; j++) {
                        int32_t val = abs(((int8_t *) image.data)[j]);
                        histogram[j] = val;
                        i += val;
                    }
                } else {
                    // Build histogram of events.
                    for (uint32_t j = 0; j < 2048; j++) {
                        uint32_t val = ((uint32_t *) image.data)[j];
                        switch (__EVT20_TYPE(val)) {
                            case TD_LOW:
                            case TD_HIGH: {
                                uint32_t x = __EVT20_X(val);
                                uint32_t y = __EVT20_Y(val);
                                if ((x < ACTIVE_SENSOR_WIDTH) && (y < ACTIVE_SENSOR_HEIGHT)) {
                                    uint32_t index = (y * ACTIVE_SENSOR_WIDTH) + x;
                                    histogram[index] = __USAT(histogram[index] + 1, UINT8_T_BITS);
                                    i++;
                                }
                                break;
                            }
                            default: {
                                break;
                            }
                        }
                    }
                }

                // Print something to prevent the user from thinking the camera is stuck.
                mp_printf(MP_PYTHON_PRINTER, "CSI: Calibrating - %d%%\n", ((i * 50) / event_count));
            }

            ret = disable_hot_pixels(csi, histogram, sigma);
            fb_free();
            break;
        }
        default: {
            ret = -1;
            break;
        }
    }

    return ret;
}

static int match(omv_csi_t *csi, size_t id) {
    id &= 0x7FFFFFFF;
    return (id == (GENX320_ID_ES & 0x7FFFFFFF)) ||
           (id == (GENX320_ID_MP & 0x7FFFFFFF));
}

int genx320_init(omv_csi_t *csi) {
    // Initialize csi structure
    csi->reset = reset;
    csi->sleep = sleep;
    csi->match = match;
    csi->read_reg = read_reg;
    csi->write_reg = write_reg;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_framerate = set_framerate;
    csi->set_contrast = set_contrast;
    csi->set_brightness = set_brightness;
    csi->set_colorbar = set_colorbar;
    csi->set_hmirror = set_hmirror;
    csi->set_vflip = set_vflip;
    csi->priv = csi->snapshot;
    csi->snapshot = snapshot;
    csi->ioctl = ioctl;

    // Set csi flags
    csi->mono_bpp = sizeof(uint8_t);

    return 0;
}
#endif // (OMV_GENX320_ENABLE == 1)
