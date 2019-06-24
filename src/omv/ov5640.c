/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV5640 driver.
 *
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

static int reset(sensor_t *sensor)
{
    int i=0;
    const uint16_t (*regs)[2];
    // Reset all registers
    cambus_writeb2(sensor->slv_addr, 0x3008, 0x42);
    // Delay 10 ms
    systick_sleep(10);
    // Write default regsiters
    for (i=0, regs = default_regs; regs[i][0]; i++) {
        cambus_writeb2(sensor->slv_addr, regs[i][0], regs[i][1]);
    }
    cambus_writeb2(sensor->slv_addr, 0x3008, 0x02);
    systick_sleep(30);
    // Write auto focus firmware
    for (i=0, regs = OV5640_AF_REG; regs[i][0]; i++) {
        cambus_writeb2(sensor->slv_addr, regs[i][0], regs[i][1]);
    }
    // Delay
    systick_sleep(10);
    // Enable auto focus
    cambus_writeb2(sensor->slv_addr, 0x3023, 0x01);
    cambus_writeb2(sensor->slv_addr, 0x3022, 0x04);

    systick_sleep(30);
    return 0;
}

static int sleep(sensor_t *sensor, int enable)
{
    uint8_t reg;
    if (enable) {
        reg = 0x42;
    } else {
        reg = 0x02;
    }
    // Write back register
    return cambus_writeb2(sensor->slv_addr, 0x3008, reg);
}

static int read_reg(sensor_t *sensor, uint16_t reg_addr)
{
    uint8_t reg_data;
    if (cambus_readb(sensor->slv_addr, reg_addr, &reg_data) != 0) {
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
//    uint8_t reg;
    int ret=0;
    switch (pixformat) {
        case PIXFORMAT_RGB565:
            cambus_writeb2(sensor->slv_addr, 0x4300, 0x61);//RGB565
            cambus_writeb2(sensor->slv_addr, 0x501f, 0x01);//ISP RGB
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_GRAYSCALE:
            cambus_writeb2(sensor->slv_addr, 0x4300, 0x10);//Y8
            cambus_writeb2(sensor->slv_addr, 0x501f, 0x00);//ISP YUV
            break;
        case PIXFORMAT_BAYER:
//        	reg = 0x00;//TODO: fix order
            break;
        default:
            return -1;
    }
    return ret;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    int ret=0;
    uint16_t w = resolution[framesize][0];
    uint16_t h = resolution[framesize][1];

    ret |= cambus_writeb2(sensor->slv_addr, 0x3808, w>>8);
    ret |= cambus_writeb2(sensor->slv_addr, 0x3809,  w);
    ret |= cambus_writeb2(sensor->slv_addr, 0x380a, h>>8);
    ret |= cambus_writeb2(sensor->slv_addr, 0x380b,  h);

    return ret;
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
    return 0;
}

static int get_gain_db(sensor_t *sensor, float *gain_db)
{
    return 0;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure_us)
{
    return 0;
}

static int get_exposure_us(sensor_t *sensor, int *exposure_us)
{
    return 0;
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
    uint8_t reg;
    int ret = cambus_readb2(sensor->slv_addr, 0x3821, &reg);
    if (enable){
    	ret |= cambus_writeb2(sensor->slv_addr, 0x3821, reg&0x06);
    } else {
    	ret |= cambus_writeb2(sensor->slv_addr, 0x3821, reg|0xF9);
    }
    return ret;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = cambus_readb2(sensor->slv_addr, 0x3820, &reg);
    if (enable){
    	ret |= cambus_writeb2(sensor->slv_addr, 0x3820, reg&0xF9);
    } else {
    	ret |= cambus_writeb2(sensor->slv_addr, 0x3820, reg|0x06);
    }
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

int ov5640_init(sensor_t *sensor)
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
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_PIXCK, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_FSYNC, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_JPEGE, 0);

    return 0;
}
