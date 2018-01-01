/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MT9V034 driver.
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include STM32_HAL_H
#include "cambus.h"
#include "sensor.h"
#include "systick.h"
#include "omv_boardconfig.h"

#define MT9V034_MAX_HEIGHT				        (480)
#define MT9V034_MAX_WIDTH					    (752)
#define MT9V034_CHIP_VERSION                    (0x00)
#define MT9V034_COL_START                       (0x01)
#define MT9V034_COL_START_MIN                   (1)
#define MT9V034_COL_START_MAX                   (752)
#define MT9V034_ROW_START                       (0x02)
#define MT9V034_ROW_START_MIN                   (4)
#define MT9V034_ROW_START_MAX                   (482)
#define MT9V034_WINDOW_HEIGHT                   (0x03)
#define MT9V034_WINDOW_HEIGHT_MIN               (1)
#define MT9V034_WINDOW_HEIGHT_MAX               (480)
#define MT9V034_WINDOW_WIDTH                    (0x04)
#define MT9V034_WINDOW_WIDTH_MIN                (1)
#define MT9V034_WINDOW_WIDTH_MAX                (752)
#define MT9V034_HORIZONTAL_BLANKING             (0x05)
#define MT9V034_HORIZONTAL_BLANKING_MIN         (43)
#define MT9V034_HORIZONTAL_BLANKING_MAX         (1023)
#define MT9V034_VERTICAL_BLANKING               (0x06)
#define MT9V034_VERTICAL_BLANKING_MIN           (4)
#define MT9V034_VERTICAL_BLANKING_MAX           (3000)
#define MT9V034_CHIP_CONTROL                    (0x07)
#define MT9V034_CHIP_CONTROL_MASTER_MODE        (1 << 3)
#define MT9V034_CHIP_CONTROL_DOUT_ENABLE        (1 << 7)
#define MT9V034_CHIP_CONTROL_SEQUENTIAL         (1 << 8)
#define MT9V034_SHUTTER_WIDTH1                  (0x08)
#define MT9V034_SHUTTER_WIDTH2                  (0x09)
#define MT9V034_SHUTTER_WIDTH_CONTROL           (0x0A)
#define MT9V034_TOTAL_SHUTTER_WIDTH             (0x0B)
#define MT9V034_TOTAL_SHUTTER_WIDTH_MIN         (1)
#define MT9V034_TOTAL_SHUTTER_WIDTH_MAX         (32767)
#define MT9V034_RESET                           (0x0C)
#define MT9V034_READ_MODE                       (0x0D)
#define MT9V034_READ_MODE_ROW_BIN_2             (1 << 0)
#define MT9V034_READ_MODE_ROW_BIN_4             (1 << 1)
#define MT9V034_READ_MODE_COL_BIN_2             (1 << 2)
#define MT9V034_READ_MODE_COL_BIN_4             (1 << 3)
#define MT9V034_READ_MODE_ROW_FLIP              (1 << 4)
#define MT9V034_READ_MODE_COL_FLIP              (1 << 5)
#define MT9V034_READ_MODE_DARK_COLS             (1 << 6)
#define MT9V034_READ_MODE_DARK_ROWS             (1 << 7)
#define MT9V034_PIXEL_OPERATION_MODE            (0x0F)
#define MT9V034_PIXEL_OPERATION_MODE_COLOR      (1 << 2)
#define MT9V034_PIXEL_OPERATION_MODE_HDR        (1 << 6)
#define MT9V034_ANALOG_GAIN                     (0x35)
#define MT9V034_ANALOG_GAIN_MIN                 (16)
#define MT9V034_ANALOG_GAIN_MAX                 (64)
#define MT9V034_MAX_ANALOG_GAIN                 (0x36)
#define MT9V034_MAX_ANALOG_GAIN_MAX             (127)
#define MT9V034_FRAME_DARK_AVERAGE              (0x42)
#define MT9V034_DARK_AVG_THRESH                 (0x46)
#define MT9V034_DARK_AVG_LOW_THRESH_MASK        (255 << 0)
#define MT9V034_DARK_AVG_LOW_THRESH_SHIFT       (0)
#define MT9V034_DARK_AVG_HIGH_THRESH_MASK       (255 << 8)
#define MT9V034_DARK_AVG_HIGH_THRESH_SHIFT      (8)
#define MT9V034_ROW_NOISE_CORR_CONTROL          (0x70)
#define MT9V034_ROW_NOISE_CORR_ENABLE           (1 << 5)
#define MT9V034_ROW_NOISE_CORR_USE_BLK_AVG      (1 << 7)
#define MT9V034_PIXEL_CLOCK                     (0x72)
#define MT9V034_PIXEL_CLOCK_INV_LINE            (1 << 0)
#define MT9V034_PIXEL_CLOCK_INV_FRAME           (1 << 1)
#define MT9V034_PIXEL_CLOCK_XOR_LINE            (1 << 2)
#define MT9V034_PIXEL_CLOCK_CONT_LINE           (1 << 3)
#define MT9V034_PIXEL_CLOCK_INV_PXL_CLK         (1 << 4)
#define MT9V034_TEST_PATTERN                    (0x7F)
#define MT9V034_TEST_PATTERN_DATA_MASK          (1023 << 0)
#define MT9V034_TEST_PATTERN_DATA_SHIFT         (0)
#define MT9V034_TEST_PATTERN_USE_DATA           (1 << 10)
#define MT9V034_TEST_PATTERN_GRAY_MASK          (3 << 11)
#define MT9V034_TEST_PATTERN_GRAY_NONE          (0 << 11)
#define MT9V034_TEST_PATTERN_GRAY_VERTICAL      (1 << 11)
#define MT9V034_TEST_PATTERN_GRAY_HORIZONTAL    (2 << 11)
#define MT9V034_TEST_PATTERN_GRAY_DIAGONAL      (3 << 11)
#define MT9V034_TEST_PATTERN_ENABLE             (1 << 13)
#define MT9V034_TEST_PATTERN_FLIP               (1 << 14)
#define MT9V034_AEC_AGC_ENABLE                  (0xAF)
#define MT9V034_AEC_ENABLE                      (1 << 0)
#define MT9V034_AGC_ENABLE                      (1 << 1)
#define MT9V034_THERMAL_INFO                    (0xC1)

#define MT9V034_WINDOW_HEIGHT_A                 (0x03)
#define MT9V034_WINDOW_WIDTH_A                  (0x04)
#define MT9V034_HORIZONTAL_BLANKING_A           (0x05)
#define MT9V034_VERTICAL_BLANKING_A             (0x06)
#define MT9V034_COARSE_SHUTTER_WIDTH_TOTAL_A    (0x0B)
#define MT9V034_ANALOG_GAIN_CONTROL             (0x35)
#define MT9V034_MAX_GAIN                        (0xAB)
#define MT9V034_FINE_SHUTTER_WIDTH_TOTAL_A      (0xD5)

static int reset(sensor_t *sensor)
{
    // NOTE: TODO This doesn't reset register configuration.
    cambus_writew(sensor->slv_addr, MT9V034_RESET, 1);

    // Delay 10 ms
    systick_sleep(10);

	cambus_writew(sensor->slv_addr, MT9V034_CHIP_CONTROL, 0x0088);
    cambus_writew(sensor->slv_addr, MT9V034_READ_MODE, 
            (MT9V034_READ_MODE_ROW_FLIP | MT9V034_READ_MODE_COL_FLIP));
    cambus_writew(sensor->slv_addr, MT9V034_RESET, 1);
    return 0;
}

static int sleep(sensor_t *sensor, int enable)
{
    return 0;
}

static int read_reg(sensor_t *sensor, uint8_t reg_addr)
{
    uint16_t reg_data;
    if (cambus_readw(sensor->slv_addr, reg_addr, &reg_data) != 0) {
        return -1;
    }
    return reg_data;
}

static int write_reg(sensor_t *sensor, uint8_t reg_addr, uint16_t reg_data)
{
    return cambus_writew(sensor->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    return 0;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    int ret=0;
    uint16_t readmode = 0;
    uint16_t width  = resolution[framesize][0]; 
    uint16_t height = resolution[framesize][1];

    if ((width * 4) <= MT9V034_MAX_WIDTH && (height * 4) <= MT9V034_MAX_HEIGHT) {
        width  *= 4; height *= 4;
        readmode |= MT9V034_READ_MODE_ROW_BIN_4;
        readmode |= MT9V034_READ_MODE_COL_BIN_4;
    } else if ((width * 2) <= MT9V034_MAX_WIDTH && (height * 2) <= MT9V034_MAX_HEIGHT) {
        width  *= 2; height *= 2;
        readmode |= MT9V034_READ_MODE_ROW_BIN_2;
        readmode |= MT9V034_READ_MODE_COL_BIN_2;
    }

    readmode |= MT9V034_READ_MODE_ROW_FLIP;
    readmode |= MT9V034_READ_MODE_COL_FLIP;
    cambus_writew(sensor->slv_addr, MT9V034_READ_MODE, readmode);

	ret |= cambus_writew(sensor->slv_addr, MT9V034_WINDOW_WIDTH, width);
	ret |= cambus_writew(sensor->slv_addr, MT9V034_COL_START, (MT9V034_MAX_WIDTH  - width ) / 2 + MT9V034_COL_START_MIN);

	ret |= cambus_writew(sensor->slv_addr, MT9V034_WINDOW_HEIGHT, height);
	ret |= cambus_writew(sensor->slv_addr, MT9V034_ROW_START, (MT9V034_MAX_HEIGHT - height) / 2 + MT9V034_ROW_START_MIN);
    return 0;
}

static int set_framerate(sensor_t *sensor, framerate_t framerate)
{
    return 0;
}

static int set_contrast(sensor_t *sensor, int level)
{
    return 0;
}

static int set_brightness(sensor_t *sensor, int level)
{
    return 0;

}

static int set_saturation(sensor_t *sensor, int level)
{
    return 0;
}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling)
{
    return 0;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    return 0;
}

static int set_auto_gain(sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling)
{
    uint16_t reg;
    int ret = cambus_readw(sensor->slv_addr, MT9V034_AEC_AGC_ENABLE, &reg);
    ret |= cambus_writew(sensor->slv_addr, MT9V034_AEC_AGC_ENABLE,
            (reg & (~MT9V034_AGC_ENABLE)) | ((enable != 0) ? MT9V034_AGC_ENABLE : 0));

    if ((enable == 0) && (!isnanf(gain_db))) {
        int gain = IM_MAX(IM_MIN(fast_roundf(fast_expf((gain_db / 20.0) * fast_log(10.0)) * 16.0), 127), 0);

        ret |= cambus_readw(sensor->slv_addr, MT9V034_ANALOG_GAIN_CONTROL, &reg);
        ret |= cambus_writew(sensor->slv_addr, MT9V034_ANALOG_GAIN_CONTROL, (reg & 0xFF80) | gain);
    } else if ((enable != 0) && (!isnanf(gain_db_ceiling))) {
        int gain_ceiling = IM_MAX(IM_MIN(fast_roundf(fast_expf((gain_db_ceiling / 20.0) * fast_log(10.0)) * 16.0), 127), 16);

        ret |= cambus_readw(sensor->slv_addr, MT9V034_MAX_GAIN, &reg);
        ret |= cambus_writew(sensor->slv_addr, MT9V034_MAX_GAIN, (reg & 0xFF80) | gain_ceiling);
    }

    return ret;
}

static int get_gain_db(sensor_t *sensor, float *gain_db)
{
    uint16_t gain;
    int ret = cambus_readw(sensor->slv_addr, MT9V034_ANALOG_GAIN_CONTROL, &gain);

    *gain_db = 20.0 * (fast_log((gain & 0x7F) / 16.0) / fast_log(10.0));

    return ret;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure_us)
{
    uint16_t reg, row_limit_0, row_limit_1, row_time_0, row_time_1;
    int ret = cambus_readw(sensor->slv_addr, MT9V034_AEC_AGC_ENABLE, &reg);
    ret |= cambus_writew(sensor->slv_addr, MT9V034_AEC_AGC_ENABLE,
            (reg & (~MT9V034_AEC_ENABLE)) | ((enable != 0) ? MT9V034_AEC_ENABLE : 0));

    if ((enable == 0) && (exposure_us >= 0)) {
        ret |= cambus_readw(sensor->slv_addr, MT9V034_WINDOW_HEIGHT_A, &row_limit_0);
        ret |= cambus_readw(sensor->slv_addr, MT9V034_VERTICAL_BLANKING_A, &row_limit_1);
        ret |= cambus_readw(sensor->slv_addr, MT9V034_WINDOW_WIDTH_A, &row_time_0);
        ret |= cambus_readw(sensor->slv_addr, MT9V034_HORIZONTAL_BLANKING_A, &row_time_1);

        int exposure = exposure_us * (27000000 / 1000000);
        int row_time = row_time_0 + row_time_1;
        int coarse_time = IM_MIN((exposure / row_time), (row_limit_0 + row_limit_1));
        int fine_time = exposure % row_time;

        ret |= cambus_writew(sensor->slv_addr, MT9V034_COARSE_SHUTTER_WIDTH_TOTAL_A, coarse_time);
        ret |= cambus_writew(sensor->slv_addr, MT9V034_FINE_SHUTTER_WIDTH_TOTAL_A, fine_time);
    }

    return ret;
}

static int get_exposure_us(sensor_t *sensor, int *exposure_us)
{
    uint16_t int_rows, int_pixels, row_time_0, row_time_1;
    int ret = cambus_readw(sensor->slv_addr, MT9V034_COARSE_SHUTTER_WIDTH_TOTAL_A, &int_rows);
    ret |= cambus_readw(sensor->slv_addr, MT9V034_FINE_SHUTTER_WIDTH_TOTAL_A, &int_pixels);
    ret |= cambus_readw(sensor->slv_addr, MT9V034_WINDOW_WIDTH_A, &row_time_0);
    ret |= cambus_readw(sensor->slv_addr, MT9V034_HORIZONTAL_BLANKING_A, &row_time_1);

    *exposure_us = ((int_rows * (row_time_0 + row_time_1)) + int_pixels) / (27000000 / 1000000);

    return ret;
}

static int set_auto_whitebal(sensor_t *sensor, int enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
    return 0;
}

static int get_rgb_gain_db(sensor_t *sensor, float *r_gain_db, float *g_gain_db, float *b_gain_db)
{
    return 0;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
    uint16_t read_mode;
    int ret = cambus_readw(sensor->slv_addr, MT9V034_READ_MODE, &read_mode);
    ret |= cambus_writew(sensor->slv_addr, MT9V034_ANALOG_GAIN_CONTROL,
            (read_mode & (~MT9V034_READ_MODE_COL_FLIP)) | ((enable != 0) ? MT9V034_READ_MODE_COL_FLIP : 0));

    return ret;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    uint16_t read_mode;
    int ret = cambus_readw(sensor->slv_addr, MT9V034_READ_MODE, &read_mode);
    ret |= cambus_writew(sensor->slv_addr, MT9V034_ANALOG_GAIN_CONTROL,
            (read_mode & (~MT9V034_READ_MODE_ROW_FLIP)) | ((enable != 0) ? MT9V034_READ_MODE_ROW_FLIP : 0));

    return ret;
}

static int set_special_effect(sensor_t *sensor, sde_t sde)
{
    return 0;
}

static int set_lens_correction(sensor_t *sensor, int enable, int radi, int coef)
{
    return 0;
}

int mt9v034_init(sensor_t *sensor)
{
    // Initialize sensor structure.
    sensor->gs_bpp              = 1;
    sensor->reset               = reset;
    sensor->sleep               = sleep;
    sensor->read_reg            = read_reg;
    sensor->write_reg           = write_reg;
    sensor->set_pixformat       = set_pixformat;
    sensor->set_framesize       = set_framesize;
    sensor->set_framerate       = set_framerate;
    sensor->set_contrast        = set_contrast;
    sensor->set_brightness      = set_brightness;
    sensor->set_saturation      = set_saturation;
    sensor->set_gainceiling     = set_gainceiling;
    sensor->set_colorbar        = set_colorbar;
    sensor->set_auto_gain       = set_auto_gain;
    sensor->get_gain_db         = get_gain_db;
    sensor->set_auto_exposure   = set_auto_exposure;
    sensor->get_exposure_us     = get_exposure_us;
    sensor->set_auto_whitebal   = set_auto_whitebal;
    sensor->get_rgb_gain_db     = get_rgb_gain_db;
    sensor->set_hmirror         = set_hmirror;
    sensor->set_vflip           = set_vflip;
    sensor->set_special_effect  = set_special_effect;
    sensor->set_lens_correction = set_lens_correction;

    // Set sensor flags
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_VSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_HSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_PIXCK, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_FSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_JPEGE, 0);

    return 0;
}
