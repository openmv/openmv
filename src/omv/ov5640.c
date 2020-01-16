/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV5640 driver.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include STM32_HAL_H
#include "cambus.h"
#include "ov5640.h"
#include "ov5640_regs.h"
#include "systick.h"
#include "omv_boardconfig.h"

#define BLANK_LINES             8
#define DUMMY_LINES             6

#define BLANK_COLUMNS           0
#define DUMMY_COLUMNS           16

#define SENSOR_WIDTH            2624
#define SENSOR_HEIGHT           1964

#define ACTIVE_SENSOR_WIDTH     (SENSOR_WIDTH - BLANK_COLUMNS - (2 * DUMMY_COLUMNS))
#define ACTIVE_SENSOR_HEIGHT    (SENSOR_HEIGHT - BLANK_LINES - (2 * DUMMY_LINES))

#define DUMMY_WIDTH_BUFFER      16
#define DUMMY_HEIGHT_BUFFER     8

#define HSYNC_TIME              252
#define VYSNC_TIME              24

static const uint8_t default_regs[][3] = {

// https://github.com/ArduCAM/Arduino/blob/master/ArduCAM/ov5640_regs.h

    { 0x47, 0x40, 0x20 },
    { 0x40, 0x50, 0x6e },
    { 0x40, 0x51, 0x8f },
    { 0x30, 0x08, 0x42 },
    { 0x31, 0x03, 0x03 },
    { 0x30, 0x17, 0xff }, // { 0x30, 0x17, 0x7f },
    { 0x30, 0x18, 0xff },
    { 0x30, 0x2c, 0x02 },
    { 0x31, 0x08, 0x01 },
    { 0x36, 0x30, 0x2e },
    { 0x36, 0x32, 0xe2 },
    { 0x36, 0x33, 0x23 },
    { 0x36, 0x21, 0xe0 },
    { 0x37, 0x04, 0xa0 },
    { 0x37, 0x03, 0x5a },
    { 0x37, 0x15, 0x78 },
    { 0x37, 0x17, 0x01 },
    { 0x37, 0x0b, 0x60 },
    { 0x37, 0x05, 0x1a },
    { 0x39, 0x05, 0x02 },
    { 0x39, 0x06, 0x10 },
    { 0x39, 0x01, 0x0a },
    { 0x37, 0x31, 0x12 },
    { 0x36, 0x00, 0x08 },
    { 0x36, 0x01, 0x33 },
    { 0x30, 0x2d, 0x60 },
    { 0x36, 0x20, 0x52 },
    { 0x37, 0x1b, 0x20 },
    { 0x47, 0x1c, 0x50 },
    { 0x3a, 0x18, 0x00 },
    { 0x3a, 0x19, 0xf8 },
    { 0x36, 0x35, 0x1c },
    { 0x36, 0x34, 0x40 },
    { 0x36, 0x22, 0x01 },
    { 0x3c, 0x04, 0x28 },
    { 0x3c, 0x05, 0x98 },
    { 0x3c, 0x06, 0x00 },
    { 0x3c, 0x07, 0x08 },
    { 0x3c, 0x08, 0x00 },
    { 0x3c, 0x09, 0x1c },
    { 0x3c, 0x0a, 0x9c },
    { 0x3c, 0x0b, 0x40 },
    { 0x38, 0x20, 0x47 }, // { 0x38, 0x20, 0x41 },
    { 0x38, 0x21, 0x01 },
    { 0x38, 0x00, 0x00 },
    { 0x38, 0x01, 0x00 },
    { 0x38, 0x02, 0x00 },
    { 0x38, 0x03, 0x04 },
    { 0x38, 0x04, 0x0a },
    { 0x38, 0x05, 0x3f },
    { 0x38, 0x06, 0x07 },
    { 0x38, 0x07, 0x9b },
    { 0x38, 0x08, 0x05 },
    { 0x38, 0x09, 0x00 },
    { 0x38, 0x0a, 0x03 },
    { 0x38, 0x0b, 0xc0 },
    { 0x38, 0x10, 0x00 },
    { 0x38, 0x11, 0x10 },
    { 0x38, 0x12, 0x00 },
    { 0x38, 0x13, 0x06 },
    { 0x38, 0x14, 0x31 },
    { 0x38, 0x15, 0x31 },
    { 0x30, 0x34, 0x1a },
    { 0x30, 0x35, 0x11 }, // { 0x30, 0x35, 0x21 },
    { 0x30, 0x36, 0x50 }, // { 0x30, 0x36, 0x46 },
    { 0x30, 0x37, 0x13 },
    { 0x30, 0x38, 0x00 },
    { 0x30, 0x39, 0x00 },
    { 0x38, 0x0c, 0x07 },
    { 0x38, 0x0d, 0x68 },
    { 0x38, 0x0e, 0x03 },
    { 0x38, 0x0f, 0xd8 },
    { 0x3c, 0x01, 0xb4 },
    { 0x3c, 0x00, 0x04 },
    { 0x3a, 0x08, 0x00 },
    { 0x3a, 0x09, 0x93 },
    { 0x3a, 0x0e, 0x06 },
    { 0x3a, 0x0a, 0x00 },
    { 0x3a, 0x0b, 0x7b },
    { 0x3a, 0x0d, 0x08 },
    { 0x3a, 0x00, 0x38 }, // { 0x3a, 0x00, 0x3c },
    { 0x3a, 0x02, 0x05 },
    { 0x3a, 0x03, 0xc4 },
    { 0x3a, 0x14, 0x05 },
    { 0x3a, 0x15, 0xc4 },
    { 0x36, 0x18, 0x00 },
    { 0x36, 0x12, 0x29 },
    { 0x37, 0x08, 0x64 },
    { 0x37, 0x09, 0x52 },
    { 0x37, 0x0c, 0x03 },
    { 0x40, 0x01, 0x02 },
    { 0x40, 0x04, 0x02 },
    { 0x30, 0x00, 0x00 },
    { 0x30, 0x02, 0x1c },
    { 0x30, 0x04, 0xff },
    { 0x30, 0x06, 0xc3 },
    { 0x30, 0x0e, 0x58 },
    { 0x30, 0x2e, 0x00 },
    { 0x43, 0x00, 0x30 },
    { 0x50, 0x1f, 0x00 },
    { 0x47, 0x13, 0x03 },
    { 0x44, 0x07, 0x04 },
    { 0x46, 0x0b, 0x35 },
    { 0x46, 0x0c, 0x22 },
    { 0x38, 0x24, 0x02 }, // { 0x38, 0x24, 0x01 },
    { 0x50, 0x01, 0xa3 },
    { 0x34, 0x06, 0x01 },
    { 0x34, 0x00, 0x06 },
    { 0x34, 0x01, 0x80 },
    { 0x34, 0x02, 0x04 },
    { 0x34, 0x03, 0x00 },
    { 0x34, 0x04, 0x06 },
    { 0x34, 0x05, 0x00 },
    { 0x51, 0x80, 0xff },
    { 0x51, 0x81, 0xf2 },
    { 0x51, 0x82, 0x00 },
    { 0x51, 0x83, 0x14 },
    { 0x51, 0x84, 0x25 },
    { 0x51, 0x85, 0x24 },
    { 0x51, 0x86, 0x16 },
    { 0x51, 0x87, 0x16 },
    { 0x51, 0x88, 0x16 },
    { 0x51, 0x89, 0x62 },
    { 0x51, 0x8a, 0x62 },
    { 0x51, 0x8b, 0xf0 },
    { 0x51, 0x8c, 0xb2 },
    { 0x51, 0x8d, 0x50 },
    { 0x51, 0x8e, 0x30 },
    { 0x51, 0x8f, 0x30 },
    { 0x51, 0x90, 0x50 },
    { 0x51, 0x91, 0xf8 },
    { 0x51, 0x92, 0x04 },
    { 0x51, 0x93, 0x70 },
    { 0x51, 0x94, 0xf0 },
    { 0x51, 0x95, 0xf0 },
    { 0x51, 0x96, 0x03 },
    { 0x51, 0x97, 0x01 },
    { 0x51, 0x98, 0x04 },
    { 0x51, 0x99, 0x12 },
    { 0x51, 0x9a, 0x04 },
    { 0x51, 0x9b, 0x00 },
    { 0x51, 0x9c, 0x06 },
    { 0x51, 0x9d, 0x82 },
    { 0x51, 0x9e, 0x38 },
    { 0x53, 0x81, 0x1e },
    { 0x53, 0x82, 0x5b },
    { 0x53, 0x83, 0x14 },
    { 0x53, 0x84, 0x06 },
    { 0x53, 0x85, 0x82 },
    { 0x53, 0x86, 0x88 },
    { 0x53, 0x87, 0x7c },
    { 0x53, 0x88, 0x60 },
    { 0x53, 0x89, 0x1c },
    { 0x53, 0x8a, 0x01 },
    { 0x53, 0x8b, 0x98 },
    { 0x53, 0x00, 0x08 },
    { 0x53, 0x01, 0x30 },
    { 0x53, 0x02, 0x3f },
    { 0x53, 0x03, 0x10 },
    { 0x53, 0x04, 0x08 },
    { 0x53, 0x05, 0x30 },
    { 0x53, 0x06, 0x18 },
    { 0x53, 0x07, 0x28 },
    { 0x53, 0x09, 0x08 },
    { 0x53, 0x0a, 0x30 },
    { 0x53, 0x0b, 0x04 },
    { 0x53, 0x0c, 0x06 },
    { 0x54, 0x80, 0x01 },
    { 0x54, 0x81, 0x06 },
    { 0x54, 0x82, 0x12 },
    { 0x54, 0x83, 0x24 },
    { 0x54, 0x84, 0x4a },
    { 0x54, 0x85, 0x58 },
    { 0x54, 0x86, 0x65 },
    { 0x54, 0x87, 0x72 },
    { 0x54, 0x88, 0x7d },
    { 0x54, 0x89, 0x88 },
    { 0x54, 0x8a, 0x92 },
    { 0x54, 0x8b, 0xa3 },
    { 0x54, 0x8c, 0xb2 },
    { 0x54, 0x8d, 0xc8 },
    { 0x54, 0x8e, 0xdd },
    { 0x54, 0x8f, 0xf0 },
    { 0x54, 0x90, 0x15 },
    { 0x55, 0x80, 0x06 },
    { 0x55, 0x83, 0x40 },
    { 0x55, 0x84, 0x20 },
    { 0x55, 0x89, 0x10 },
    { 0x55, 0x8a, 0x00 },
    { 0x55, 0x8b, 0xf8 },
    { 0x50, 0x00, 0xa7 },
    { 0x58, 0x00, 0x20 },
    { 0x58, 0x01, 0x19 },
    { 0x58, 0x02, 0x17 },
    { 0x58, 0x03, 0x16 },
    { 0x58, 0x04, 0x18 },
    { 0x58, 0x05, 0x21 },
    { 0x58, 0x06, 0x0F },
    { 0x58, 0x07, 0x0A },
    { 0x58, 0x08, 0x07 },
    { 0x58, 0x09, 0x07 },
    { 0x58, 0x0a, 0x0A },
    { 0x58, 0x0b, 0x0C },
    { 0x58, 0x0c, 0x0A },
    { 0x58, 0x0d, 0x03 },
    { 0x58, 0x0e, 0x01 },
    { 0x58, 0x0f, 0x01 },
    { 0x58, 0x10, 0x03 },
    { 0x58, 0x11, 0x09 },
    { 0x58, 0x12, 0x0A },
    { 0x58, 0x13, 0x03 },
    { 0x58, 0x14, 0x01 },
    { 0x58, 0x15, 0x01 },
    { 0x58, 0x16, 0x03 },
    { 0x58, 0x17, 0x08 },
    { 0x58, 0x18, 0x10 },
    { 0x58, 0x19, 0x0A },
    { 0x58, 0x1a, 0x06 },
    { 0x58, 0x1b, 0x06 },
    { 0x58, 0x1c, 0x08 },
    { 0x58, 0x1d, 0x0E },
    { 0x58, 0x1e, 0x22 },
    { 0x58, 0x1f, 0x18 },
    { 0x58, 0x20, 0x13 },
    { 0x58, 0x21, 0x12 },
    { 0x58, 0x22, 0x16 },
    { 0x58, 0x23, 0x1E },
    { 0x58, 0x24, 0x64 },
    { 0x58, 0x25, 0x2A },
    { 0x58, 0x26, 0x2C },
    { 0x58, 0x27, 0x2A },
    { 0x58, 0x28, 0x46 },
    { 0x58, 0x29, 0x2A },
    { 0x58, 0x2a, 0x26 },
    { 0x58, 0x2b, 0x24 },
    { 0x58, 0x2c, 0x26 },
    { 0x58, 0x2d, 0x2A },
    { 0x58, 0x2e, 0x28 },
    { 0x58, 0x2f, 0x42 },
    { 0x58, 0x30, 0x40 },
    { 0x58, 0x31, 0x42 },
    { 0x58, 0x32, 0x08 },
    { 0x58, 0x33, 0x28 },
    { 0x58, 0x34, 0x26 },
    { 0x58, 0x35, 0x24 },
    { 0x58, 0x36, 0x26 },
    { 0x58, 0x37, 0x2A },
    { 0x58, 0x38, 0x44 },
    { 0x58, 0x39, 0x4A },
    { 0x58, 0x3a, 0x2C },
    { 0x58, 0x3b, 0x2a },
    { 0x58, 0x3c, 0x46 },
    { 0x58, 0x3d, 0xCE },
    { 0x56, 0x88, 0x22 },
    { 0x56, 0x89, 0x22 },
    { 0x56, 0x8a, 0x42 },
    { 0x56, 0x8b, 0x24 },
    { 0x56, 0x8c, 0x42 },
    { 0x56, 0x8d, 0x24 },
    { 0x56, 0x8e, 0x22 },
    { 0x56, 0x8f, 0x22 },
    { 0x50, 0x25, 0x00 },
    { 0x3a, 0x0f, 0x30 },
    { 0x3a, 0x10, 0x28 },
    { 0x3a, 0x1b, 0x30 },
    { 0x3a, 0x1e, 0x28 },
    { 0x3a, 0x11, 0x61 },
    { 0x3a, 0x1f, 0x10 },
    { 0x40, 0x05, 0x1a },
    { 0x34, 0x06, 0x00 },
    { 0x35, 0x03, 0x00 },
    { 0x30, 0x08, 0x02 },

// OpenMV Custom.

    { 0x3a, 0x02, 0x07 },
    { 0x3a, 0x03, 0xae },
    { 0x3a, 0x08, 0x01 },
    { 0x3a, 0x09, 0x27 },
    { 0x3a, 0x0a, 0x00 },
    { 0x3a, 0x0b, 0xf6 },
    { 0x3a, 0x0e, 0x06 },
    { 0x3a, 0x0d, 0x08 },
    { 0x3a, 0x14, 0x07 },
    { 0x3a, 0x15, 0xae },

// End.

    { 0x00, 0x00, 0x00 }
};

#define NUM_BRIGHTNESS_LEVELS (9)

#define NUM_CONTRAST_LEVELS (7)
static const uint8_t contrast_regs[NUM_CONTRAST_LEVELS][1] = {
    {0x14}, /* -3 */
    {0x18}, /* -2 */
    {0x1C}, /* -1 */
    {0x00}, /* +0 */
    {0x10}, /* +1 */
    {0x18}, /* +2 */
    {0x1C}, /* +3 */
};

#define NUM_SATURATION_LEVELS (7)
static const uint8_t saturation_regs[NUM_SATURATION_LEVELS][6] = {
    {0x0c, 0x30, 0x3d, 0x3e, 0x3d, 0x01}, /* -3 */
    {0x10, 0x3d, 0x4d, 0x4e, 0x4d, 0x01}, /* -2 */
    {0x15, 0x52, 0x66, 0x68, 0x66, 0x02}, /* -1 */
    {0x1a, 0x66, 0x80, 0x82, 0x80, 0x02}, /* +0 */
    {0x1f, 0x7a, 0x9a, 0x9c, 0x9a, 0x02}, /* +1 */
    {0x24, 0x8f, 0xb3, 0xb6, 0xb3, 0x03}, /* +2 */
    {0x2b, 0xab, 0xd6, 0xda, 0xd6, 0x04}, /* +3 */
};

static int reset(sensor_t *sensor)
{
    // Reset all registers
    int ret = cambus_writeb2(sensor->slv_addr, SCCB_SYSTEM_CTRL_1, 0x11);
    ret |= cambus_writeb2(sensor->slv_addr, SYSTEM_CTROL0, 0x82);

    // Delay 5 ms
    systick_sleep(5);

    // Write default regsiters
    for (int i = 0; default_regs[i][0]; i++) {
        ret |= cambus_writeb2(sensor->slv_addr, (default_regs[i][0] << 8) | (default_regs[i][1] << 0), default_regs[i][2]);
    }

    // Delay 300 ms
    systick_sleep(300);

    return ret;
}

static int sleep(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = cambus_readb2(sensor->slv_addr, SYSTEM_CTROL0, &reg);

    if (enable) {
        reg |= 0x40;
    } else {
        reg &= ~0x40;
    }

    return cambus_writeb2(sensor->slv_addr, SYSTEM_CTROL0, reg) | ret;
}

static int read_reg(sensor_t *sensor, uint16_t reg_addr)
{
    uint8_t reg_data;
    if (cambus_readb2(sensor->slv_addr, reg_addr, &reg_data) != 0) {
        return -1;
    }
    return reg_data;
}

static int write_reg(sensor_t *sensor, uint16_t reg_addr, uint16_t reg_data)
{
    return cambus_writeb2(sensor->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    uint8_t pll, reg, reg2;
    int ret = 0;

    if (((pixformat == PIXFORMAT_BAYER) || (pixformat == PIXFORMAT_JPEG))
    && ((sensor->framesize == FRAMESIZE_QQCIF) // doesn't work with bayer/jpeg, unknown why
    || (sensor->framesize == FRAMESIZE_QQSIF) // doesn't work with bayer/jpeg, not a multiple of 8
    || (sensor->framesize == FRAMESIZE_HQQQVGA) // doesn't work with bayer/jpeg, unknown why
    || (sensor->framesize == FRAMESIZE_HQQVGA) // doesn't work with bayer/jpeg, unknown why
    || (resolution[sensor->framesize][0] % 8) // w/h must be divisble by 8
    || (resolution[sensor->framesize][1] % 8))) {
        return -1;
    }

    switch (pixformat) {
        case PIXFORMAT_RGB565:
            ret |= cambus_writeb2(sensor->slv_addr, FORMAT_CONTROL, 0x61);
            ret |= cambus_writeb2(sensor->slv_addr, FORMAT_CONTROL_MUX, 0x01);
            pll = (resolution[sensor->framesize][0] > 2048) ? 0x50 : 0x64; // 32 MHz vs 40 MHz
            break;
        case PIXFORMAT_GRAYSCALE:
        case PIXFORMAT_YUV422:
            ret |= cambus_writeb2(sensor->slv_addr, FORMAT_CONTROL, 0x30);
            ret |= cambus_writeb2(sensor->slv_addr, FORMAT_CONTROL_MUX, 0x00);
            pll = (resolution[sensor->framesize][0] > 2048) ? 0x50 : 0x64; // 32 MHz vs 40 MHz
            break;
        case PIXFORMAT_BAYER:
            ret |= cambus_writeb2(sensor->slv_addr, FORMAT_CONTROL, 0x00);
            ret |= cambus_writeb2(sensor->slv_addr, FORMAT_CONTROL_MUX, 0x01);
            pll = (resolution[sensor->framesize][0] > 2048) ? 0x64 : 0x50; // 40 MHz vs 32 MHz (jpeg can go faster at higher reses)
            break;
        case PIXFORMAT_JPEG:
            ret |= cambus_writeb2(sensor->slv_addr, FORMAT_CONTROL, 0x30);
            ret |= cambus_writeb2(sensor->slv_addr, FORMAT_CONTROL_MUX, 0x00);
            pll = (resolution[sensor->framesize][0] > 2048) ? 0x50 : 0x64; // 32 MHz vs 40 MHz
            break;
        default:
            return -1;
    }

    ret |= cambus_readb2(sensor->slv_addr, TIMING_TC_REG_21, &reg);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_TC_REG_21, (reg & 0xDF) | ((pixformat == PIXFORMAT_JPEG) ? 0x20 : 0x00));

    ret |= cambus_readb2(sensor->slv_addr, SYSTEM_RESET_02, &reg);
    ret |= cambus_writeb2(sensor->slv_addr, SYSTEM_RESET_02, (reg & 0xE3) | ((pixformat == PIXFORMAT_JPEG) ? 0x00 : 0x1C));

    ret |= cambus_readb2(sensor->slv_addr, CLOCK_ENABLE_02, &reg);
    ret |= cambus_writeb2(sensor->slv_addr, CLOCK_ENABLE_02, (reg & 0xD7) | ((pixformat == PIXFORMAT_JPEG) ? 0x28 : 0x00));

    // Adjust JPEG quality.

    if (pixformat == PIXFORMAT_JPEG) {
        bool high_res = (resolution[sensor->framesize][0] > 1280) || (resolution[sensor->framesize][1] > 960);
        ret |= cambus_writeb2(sensor->slv_addr, JPEG_CTRL07, high_res ? 0x10 : 0x4);

        if (sensor->pixformat != PIXFORMAT_JPEG) { // We have to double this.
            ret |= cambus_readb2(sensor->slv_addr, TIMING_HTS_H, &reg);
            ret |= cambus_readb2(sensor->slv_addr, TIMING_HTS_L, &reg2);
            uint16_t sensor_hts = ((((reg << 8) | reg2) - HSYNC_TIME) * 2) + HSYNC_TIME;
            ret |= cambus_writeb2(sensor->slv_addr, TIMING_HTS_H, sensor_hts >> 8);
            ret |= cambus_writeb2(sensor->slv_addr, TIMING_HTS_L, sensor_hts);
        }
    }

    return cambus_writeb2(sensor->slv_addr, SC_PLL_CONTRL2, pll) | ret;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    uint8_t pll, reg;
    uint16_t sensor_w = 0;
    uint16_t sensor_h = 0;
    uint16_t sensor_div = 0;
    int ret = 0;
    uint16_t w = resolution[framesize][0];
    uint16_t h = resolution[framesize][1];

    if (((sensor->pixformat == PIXFORMAT_BAYER) || (sensor->pixformat == PIXFORMAT_JPEG))
    && ((framesize == FRAMESIZE_QQCIF) // doesn't work with bayer/jpeg, unknown why
    || (framesize == FRAMESIZE_QQSIF) // doesn't work with bayer/jpeg, not a multiple of 8
    || (framesize == FRAMESIZE_HQQQVGA) // doesn't work with bayer/jpeg, unknown why
    || (framesize == FRAMESIZE_HQQVGA) // doesn't work with bayer/jpeg, unknown why
    || (w % 8) // w/h must be divisble by 8
    || (h % 8))) {
        return -1;
    }

    // Step 1: Determine readout area and subsampling amount.

    if ((w <= 320) && (h <= 240)) {
        sensor_w = 2560;
        sensor_h = 1440;
        sensor_div = 2;
    } else if ((w <= 640) && (h <= 480)) {
        sensor_w = 2560;
        sensor_h = 1440;
        sensor_div = 2;
    } else if ((w < 1280) && (h < 960)) { // < and not <=
        sensor_w = 2560;
        sensor_h = 1600;
        sensor_div = 2;
    } else if ((w <= 2560) && (h <= 1920)) {
        sensor_w = 2560;
        sensor_h = 1600;
        sensor_div = 1;
    } else {
        sensor_w = ACTIVE_SENSOR_WIDTH;
        sensor_h = ACTIVE_SENSOR_HEIGHT;
        sensor_div = 1;
    }

    // Step 2: Determine horizontal and vertical start and end points.

    uint16_t old_sensor_w = sensor_w;
    uint16_t old_sensor_h = sensor_h;

    sensor_w += DUMMY_WIDTH_BUFFER; // camera hardware needs dummy pixels to sync
    sensor_h += DUMMY_HEIGHT_BUFFER; // camera hardware needs dummy lines to sync

    uint16_t sensor_ws = (((ACTIVE_SENSOR_WIDTH - sensor_w) / 4) * 2) + DUMMY_COLUMNS; // must be multiple of 2
    uint16_t sensor_we = sensor_ws + sensor_w - 1;

    uint16_t sensor_hs = (((ACTIVE_SENSOR_HEIGHT - sensor_h) / 4) * 2) + DUMMY_LINES; // must be multiple of 2
    uint16_t sensor_he = sensor_hs + sensor_h - 1;

    // Step 3: Apply the division.

    old_sensor_w /= sensor_div;
    old_sensor_h /= sensor_div;

    sensor_w /= sensor_div;
    sensor_h /= sensor_div;

    // Step 4: Determine scaling window offset.

    float ratio = IM_MIN(old_sensor_w / ((float) w), old_sensor_h / ((float) h));
    uint16_t w_mul = w * ratio;
    uint16_t h_mul = h * ratio;
    uint16_t x_off = (sensor_w - w_mul) / 2;
    uint16_t y_off = (sensor_h - h_mul) / 2;

    // Step 5: Compute total frame time.

    uint16_t sensor_hts = (sensor_w * ((sensor->pixformat == PIXFORMAT_JPEG) ? 1 : 2)) + HSYNC_TIME;
    uint16_t sensor_vts = sensor_h + VYSNC_TIME;

    uint16_t sensor_x_inc = (((sensor_div * 2) - 1) << 4) | (1 << 0); // odd[7:4]/even[3:0] pixel inc on the bayer pattern
    uint16_t sensor_y_inc = (((sensor_div * 2) - 1) << 4) | (1 << 0); // odd[7:4]/even[3:0] pixel inc on the bayer pattern

    // Step 6: Write regs.

    ret |= cambus_writeb2(sensor->slv_addr, TIMING_HS_H, sensor_ws >> 8);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_HS_L, sensor_ws);

    ret |= cambus_writeb2(sensor->slv_addr, TIMING_VS_H, sensor_hs >> 8);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_VS_L, sensor_hs);
    
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_HW_H, sensor_we >> 8);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_HW_L, sensor_we);

    ret |= cambus_writeb2(sensor->slv_addr, TIMING_VH_H, sensor_he >> 8);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_VH_L, sensor_he);

    ret |= cambus_writeb2(sensor->slv_addr, TIMING_DVPHO_H, w >> 8);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_DVPHO_L, w);

    ret |= cambus_writeb2(sensor->slv_addr, TIMING_DVPVO_H, h >> 8);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_DVPVO_L, h);

    ret |= cambus_writeb2(sensor->slv_addr, TIMING_HTS_H, sensor_hts >> 8);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_HTS_L, sensor_hts);

    ret |= cambus_writeb2(sensor->slv_addr, TIMING_VTS_H, sensor_vts >> 8);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_VTS_L, sensor_vts);

    ret |= cambus_writeb2(sensor->slv_addr, TIMING_HOFFSET_H, x_off >> 8);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_HOFFSET_L, x_off);

    ret |= cambus_writeb2(sensor->slv_addr, TIMING_VOFFSET_H, y_off >> 8);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_VOFFSET_L, y_off);

    ret |= cambus_writeb2(sensor->slv_addr, TIMING_X_INC, sensor_x_inc);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_Y_INC, sensor_y_inc);

    ret |= cambus_readb2(sensor->slv_addr, TIMING_TC_REG_20, &reg);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_TC_REG_20, (reg & 0xFE) | (sensor_div > 1));

    ret |= cambus_readb2(sensor->slv_addr, TIMING_TC_REG_21, &reg);
    ret |= cambus_writeb2(sensor->slv_addr, TIMING_TC_REG_21, (reg & 0xFE) | (sensor_div > 1));

    ret |= cambus_writeb2(sensor->slv_addr, VFIFO_HSIZE_H, w >> 8);
    ret |= cambus_writeb2(sensor->slv_addr, VFIFO_HSIZE_L, w);

    ret |= cambus_writeb2(sensor->slv_addr, VFIFO_VSIZE_H, h >> 8);
    ret |= cambus_writeb2(sensor->slv_addr, VFIFO_VSIZE_L, h);

    // Step 7: Adjust JPEG quality.

    ret |= cambus_writeb2(sensor->slv_addr, JPEG_CTRL07, (sensor_div > 1) ? 0x4 : 0x10);

    // Step 8: Adjust PLL freq.

    switch (sensor->pixformat) {
        case PIXFORMAT_RGB565:
            pll = (w > 2048) ? 0x50 : 0x64; // 32 MHz vs 40 MHz
            break;
        case PIXFORMAT_GRAYSCALE:
        case PIXFORMAT_YUV422:
            pll = (w > 2048) ? 0x50 : 0x64; // 32 MHz vs 40 MHz
            break;
        case PIXFORMAT_BAYER:
            pll = (w > 2048) ? 0x64 : 0x50; // 40 MHz vs 32 MHz (jpeg can go faster at higher reses)
            break;
        case PIXFORMAT_JPEG:
            pll = (w > 2048) ? 0x50 : 0x64; // 32 MHz vs 40 MHz
            break;
        default:
            return -1;
    }

    ret |= cambus_writeb2(sensor->slv_addr, SC_PLL_CONTRL2, pll);

    // Delay 300 ms
    systick_sleep(300);

    return ret;
}

static int set_framerate(sensor_t *sensor, framerate_t framerate)
{
    return 0;
}

static int set_contrast(sensor_t *sensor, int level)
{
    int ret = 0;

    int new_level = level + (NUM_CONTRAST_LEVELS / 2);
    if (new_level < 0 || new_level >= NUM_CONTRAST_LEVELS) {
        return -1;
    }

    ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0x03); // start group 3
    ret |= cambus_writeb2(sensor->slv_addr, 0x5586, (new_level + 5) << 2);
    ret |= cambus_writeb2(sensor->slv_addr, 0x5585, contrast_regs[new_level][0]);
    ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0x13); // end group 3
    ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0xa3); // launch group 3

    return ret;
}

static int set_brightness(sensor_t *sensor, int level)
{
    int ret = 0;

    int new_level = level + (NUM_BRIGHTNESS_LEVELS / 2);
    if (new_level < 0 || new_level >= NUM_BRIGHTNESS_LEVELS) {
        return -1;
    }

    ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0x03); // start group 3
    ret |= cambus_writeb2(sensor->slv_addr, 0x5587, abs(level) << 4);
    ret |= cambus_writeb2(sensor->slv_addr, 0x5588, (level < 0) ? 0x09 : 0x01);
    ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0x13); // end group 3
    ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0xa3); // launch group 3

    return ret;
}

static int set_saturation(sensor_t *sensor, int level)
{
    int ret = 0;

    int new_level = level + (NUM_SATURATION_LEVELS / 2);
    if (new_level < 0 || new_level >= NUM_SATURATION_LEVELS) {
        return -1;
    }

    ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0x03); // start group 3
    ret |= cambus_writeb2(sensor->slv_addr, 0x5581, 0x1c);
    ret |= cambus_writeb2(sensor->slv_addr, 0x5582, 0x5a);
    ret |= cambus_writeb2(sensor->slv_addr, 0x5583, 0x06);
    ret |= cambus_writeb2(sensor->slv_addr, 0x5584, saturation_regs[new_level][0]);
    ret |= cambus_writeb2(sensor->slv_addr, 0x5585, saturation_regs[new_level][1]);
    ret |= cambus_writeb2(sensor->slv_addr, 0x5586, saturation_regs[new_level][2]);
    ret |= cambus_writeb2(sensor->slv_addr, 0x5587, saturation_regs[new_level][3]);
    ret |= cambus_writeb2(sensor->slv_addr, 0x5588, saturation_regs[new_level][4]);
    ret |= cambus_writeb2(sensor->slv_addr, 0x5589, saturation_regs[new_level][5]);
    ret |= cambus_writeb2(sensor->slv_addr, 0x558b, 0x98);
    ret |= cambus_writeb2(sensor->slv_addr, 0x558a, 0x01);
    ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0x13); // end group 3
    ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0xa3); // launch group 3

    return ret;
}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling)
{
    uint8_t reg;
    int ret = 0;

    int new_gainceiling = 16 << (gainceiling + 1);
    if (new_gainceiling >= 1024) {
        return -1;
    }

    ret |= cambus_readb2(sensor->slv_addr, AEC_GAIN_CEILING_H, &reg);
    ret |= cambus_writeb2(sensor->slv_addr, AEC_GAIN_CEILING_H, (reg & 0xFC) | (new_gainceiling >> 8));
    ret |= cambus_writeb2(sensor->slv_addr, AEC_GAIN_CEILING_L, new_gainceiling);

    return ret;
}

static int set_quality(sensor_t *sensor, int qs)
{
    uint8_t reg;
    int ret = cambus_readb2(sensor->slv_addr, JPEG_CTRL07, &reg);
    ret |= cambus_writeb2(sensor->slv_addr, JPEG_CTRL07, (reg & 0xC0) | (qs >> 2));

    return ret;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = cambus_readb2(sensor->slv_addr, PRE_ISP_TEST, &reg);
    return cambus_writeb2(sensor->slv_addr, PRE_ISP_TEST, (reg & 0x7F) | (enable ? 0x80 : 0x00)) | ret;
}

static int set_auto_gain(sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling)
{
    uint8_t reg;
    int ret = cambus_readb2(sensor->slv_addr, AEC_PK_MANUAL, &reg);
    ret |= cambus_writeb2(sensor->slv_addr, AEC_PK_MANUAL, (reg & 0xFD) | ((enable == 0) << 1));

    if ((enable == 0) && (!isnanf(gain_db)) && (!isinff(gain_db))) {
        int gain = IM_MAX(IM_MIN(fast_expf((gain_db / 20.0) * fast_log(10.0)) * 16.0, 1023), 0);

        ret |= cambus_readb2(sensor->slv_addr, AEC_PK_REAL_GAIN_H, &reg);
        ret |= cambus_writeb2(sensor->slv_addr, AEC_PK_REAL_GAIN_H, (reg & 0xFC) | (gain >> 8));
        ret |= cambus_writeb2(sensor->slv_addr, AEC_PK_REAL_GAIN_L, gain);
    } else if ((enable != 0) && (!isnanf(gain_db_ceiling)) && (!isinff(gain_db_ceiling))) {
        int gain_ceiling = IM_MAX(IM_MIN(fast_expf((gain_db_ceiling / 20.0) * fast_log(10.0)) * 16.0, 1023), 0);

        ret |= cambus_readb2(sensor->slv_addr, AEC_GAIN_CEILING_H, &reg);
        ret |= cambus_writeb2(sensor->slv_addr, AEC_GAIN_CEILING_H, (reg & 0xFC) | (gain_ceiling >> 8));
        ret |= cambus_writeb2(sensor->slv_addr, AEC_GAIN_CEILING_L, gain_ceiling);
    }

    return ret;
}

static int get_gain_db(sensor_t *sensor, float *gain_db)
{
    uint8_t gainh, gainl;

    int ret = cambus_readb2(sensor->slv_addr, AEC_PK_REAL_GAIN_H, &gainh);
    ret |= cambus_readb2(sensor->slv_addr, AEC_PK_REAL_GAIN_L, &gainl);

    *gain_db = 20.0 * (fast_log((((gainh & 0x3) << 8) | gainl) / 16.0) / fast_log(10.0));

    return ret;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure_us)
{
    uint8_t reg, pll, hts_h, hts_l, vts_h, vts_l;
    int ret = cambus_readb2(sensor->slv_addr, AEC_PK_MANUAL, &reg);
    ret |= cambus_writeb2(sensor->slv_addr, AEC_PK_MANUAL, (reg & 0xFE) | ((enable == 0) << 0));

    if ((enable == 0) && (exposure_us >= 0)) {
        ret |= cambus_readb2(sensor->slv_addr, SC_PLL_CONTRL2, &pll);

        ret |= cambus_readb2(sensor->slv_addr, TIMING_HTS_H, &hts_h);
        ret |= cambus_readb2(sensor->slv_addr, TIMING_HTS_L, &hts_l);

        ret |= cambus_readb2(sensor->slv_addr, TIMING_VTS_H, &vts_h);
        ret |= cambus_readb2(sensor->slv_addr, TIMING_VTS_L, &vts_l);

        uint16_t hts = (hts_h << 8) | hts_l;
        uint16_t vts = (vts_h << 8) | vts_l;

        // "/ 3" -> SC_PLL_CONTRL3[3:0] (pll pre-divider)
        // "/ 1" -> SC_PLL_CONTRL1[7:4] (system clock divider)
        // "/ 2" -> SC_PLL_CONTRL3[4] (pll root divider)
        // "/ 10" -> SYSTEM_CTROL0[3:0] (bit mode)
        // "/ 1" -> SYSTEM_ROOT_DIVIDER[5:4] (pclk root divider)
        int pclk_freq = (((((OV5640_XCLK_FREQ / 3) * pll) / 1) / 2) / 10) / 1;
        int clocks_per_us = pclk_freq / 1000000;
        int exposure = IM_MAX(IM_MIN((exposure_us * clocks_per_us) / hts, 0xFFFF), 0x0000);

        int new_vts = IM_MAX(exposure, vts);

        ret |= cambus_writeb2(sensor->slv_addr, AEC_PK_EXPOSURE_0, exposure >> 12);
        ret |= cambus_writeb2(sensor->slv_addr, AEC_PK_EXPOSURE_1, exposure >> 4);
        ret |= cambus_writeb2(sensor->slv_addr, AEC_PK_EXPOSURE_2, exposure << 4);

        ret |= cambus_writeb2(sensor->slv_addr, TIMING_VTS_H, new_vts >> 8);
        ret |= cambus_writeb2(sensor->slv_addr, TIMING_VTS_L, new_vts);
    }

    return ret;
}

static int get_exposure_us(sensor_t *sensor, int *exposure_us)
{
    uint8_t pll, aec_0, aec_1, aec_2, hts_h, hts_l;
    int ret = cambus_readb2(sensor->slv_addr, SC_PLL_CONTRL2, &pll);

    ret |= cambus_readb2(sensor->slv_addr, AEC_PK_EXPOSURE_0, &aec_0);
    ret |= cambus_readb2(sensor->slv_addr, AEC_PK_EXPOSURE_1, &aec_1);
    ret |= cambus_readb2(sensor->slv_addr, AEC_PK_EXPOSURE_2, &aec_2);

    ret |= cambus_readb2(sensor->slv_addr, TIMING_HTS_H, &hts_h);
    ret |= cambus_readb2(sensor->slv_addr, TIMING_HTS_L, &hts_l);

    uint32_t aec = ((aec_0 << 16) | (aec_1 << 8) | aec_2) >> 4;
    uint16_t hts = (hts_h << 8) | hts_l;

    // "/ 3" -> SC_PLL_CONTRL3[3:0] (pll pre-divider)
    // "/ 1" -> SC_PLL_CONTRL1[7:4] (system clock divider)
    // "/ 2" -> SC_PLL_CONTRL3[4] (pll root divider)
    // "/ 10" -> SYSTEM_CTROL0[3:0] (bit mode)
    // "/ 1" -> SYSTEM_ROOT_DIVIDER[5:4] (pclk root divider)
    int pclk_freq = (((((OV5640_XCLK_FREQ / 3) * pll) / 1) / 2) / 10) / 1;
    int clocks_per_us = pclk_freq / 1000000;
    *exposure_us = (aec * hts) / clocks_per_us;

    return ret;
}

static int set_auto_whitebal(sensor_t *sensor, int enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
    uint8_t reg;
    int ret = cambus_readb2(sensor->slv_addr, AWB_MANUAL_CONTROL, &reg);
    ret |= cambus_writeb2(sensor->slv_addr, AWB_MANUAL_CONTROL, (reg & 0xFE) | (enable == 0));

    if ((enable == 0) && (!isnanf(r_gain_db)) && (!isnanf(g_gain_db)) && (!isnanf(b_gain_db))
                      && (!isinff(r_gain_db)) && (!isinff(g_gain_db)) && (!isinff(b_gain_db))) {

        int r_gain = IM_MAX(IM_MIN(fast_roundf(fast_expf((r_gain_db / 20.0) * fast_log(10.0))), 4095), 0);
        int g_gain = IM_MAX(IM_MIN(fast_roundf(fast_expf((g_gain_db / 20.0) * fast_log(10.0))), 4095), 0);
        int b_gain = IM_MAX(IM_MIN(fast_roundf(fast_expf((b_gain_db / 20.0) * fast_log(10.0))), 4095), 0);

        ret |= cambus_writeb2(sensor->slv_addr, AWB_R_GAIN_H, r_gain >> 8);
        ret |= cambus_writeb2(sensor->slv_addr, AWB_R_GAIN_L, r_gain);
        ret |= cambus_writeb2(sensor->slv_addr, AWB_G_GAIN_H, g_gain >> 8);
        ret |= cambus_writeb2(sensor->slv_addr, AWB_G_GAIN_L, g_gain);
        ret |= cambus_writeb2(sensor->slv_addr, AWB_B_GAIN_H, b_gain >> 8);
        ret |= cambus_writeb2(sensor->slv_addr, AWB_B_GAIN_L, b_gain);
    }

    return ret;
}

static int get_rgb_gain_db(sensor_t *sensor, float *r_gain_db, float *g_gain_db, float *b_gain_db)
{
    uint8_t redh, redl, greenh, greenl, blueh, bluel;

    int ret = cambus_readb2(sensor->slv_addr, AWB_R_GAIN_H, &redh);
    ret |= cambus_readb2(sensor->slv_addr, AWB_R_GAIN_L, &redl);
    ret |= cambus_readb2(sensor->slv_addr, AWB_G_GAIN_H, &greenh);
    ret |= cambus_readb2(sensor->slv_addr, AWB_G_GAIN_L, &greenl);
    ret |= cambus_readb2(sensor->slv_addr, AWB_B_GAIN_H, &blueh);
    ret |= cambus_readb2(sensor->slv_addr, AWB_B_GAIN_L, &bluel);

    *r_gain_db = 20.0 * (fast_log(((redh & 0xF) << 8) | redl) / fast_log(10.0));
    *g_gain_db = 20.0 * (fast_log(((greenh & 0xF) << 8) | greenl) / fast_log(10.0));
    *b_gain_db = 20.0 * (fast_log(((blueh & 0xF) << 8) | bluel) / fast_log(10.0));

    return ret;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = cambus_readb2(sensor->slv_addr, TIMING_TC_REG_21, &reg);
    if (enable){
        ret |= cambus_writeb2(sensor->slv_addr, TIMING_TC_REG_21, reg|0x06);
    } else {
        ret |= cambus_writeb2(sensor->slv_addr, TIMING_TC_REG_21, reg&0xF9);
    }
    return ret;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = cambus_readb2(sensor->slv_addr, TIMING_TC_REG_20, &reg);
    if (!enable){
        ret |= cambus_writeb2(sensor->slv_addr, TIMING_TC_REG_20, reg|0x06);
    } else {
        ret |= cambus_writeb2(sensor->slv_addr, TIMING_TC_REG_20, reg&0xF9);
    }
    return ret;
}

static int set_special_effect(sensor_t *sensor, sde_t sde)
{
    int ret = 0;

    switch (sde) {
        case SDE_NEGATIVE:
            ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0x03); // start group 3
            ret |= cambus_writeb2(sensor->slv_addr, 0x5580, 0x40);
            ret |= cambus_writeb2(sensor->slv_addr, 0x5003, 0x08);
            ret |= cambus_writeb2(sensor->slv_addr, 0x5583, 0x40); // sat U
            ret |= cambus_writeb2(sensor->slv_addr, 0x5584, 0x10); // sat V
            ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0x13); // end group 3
            ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0xa3); // latch group 3
            break;
        case SDE_NORMAL:
            ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0x03); // start group 3
            ret |= cambus_writeb2(sensor->slv_addr, 0x5580, 0x06);
            ret |= cambus_writeb2(sensor->slv_addr, 0x5583, 0x40); // sat U
            ret |= cambus_writeb2(sensor->slv_addr, 0x5584, 0x10); // sat V
            ret |= cambus_writeb2(sensor->slv_addr, 0x5003, 0x08);
            ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0x13); // end group 3
            ret |= cambus_writeb2(sensor->slv_addr, 0x3212, 0xa3); // latch group 3
            break;
        default:
            return -1;
    }

    return ret;
}

static int set_lens_correction(sensor_t *sensor, int enable, int radi, int coef)
{
    uint8_t reg;
    int ret = cambus_readb2(sensor->slv_addr, ISP_CONTROL_00, &reg);
    return cambus_writeb2(sensor->slv_addr, ISP_CONTROL_00, (reg & 0x7F) | (enable ? 0x80 : 0x00)) | ret;
}

int ov5640_init(sensor_t *sensor)
{
    // Initialize sensor structure.
    sensor->gs_bpp              = 2;
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
    sensor->set_quality         = set_quality;
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
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_VSYNC, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_HSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_PIXCK, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_FSYNC, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_JPEGE, 1);

    return 0;
}
