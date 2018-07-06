/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2018 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Lepton driver.
 *
 */

#include STM32_HAL_H
#include "cambus.h"
#include "sensor.h"
#include "systick.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"

#if defined(OMV_ENABLE_LEPTON)
#include "crc16.h"
#include "LEPTON_SDK.h"
#include "LEPTON_AGC.h"
#include "LEPTON_SYS.h"
#include "LEPTON_VID.h"
#include "LEPTON_OEM.h"
#include "LEPTON_RAD.h"
#include "LEPTON_I2C_Reg.h"

#define VOSPI_LINE_PIXELS       (80)
#define VOSPI_NUMBER_PACKETS    (60)
#define VOSPI_SPECIAL_LINE      (20)

static int h_res = 0;
static int v_res = 0;
static bool h_mirror = false;
static bool v_flip = false;

static SPI_HandleTypeDef SPIHandle;
static LEP_CAMERA_PORT_DESC_T handle;
extern const uint16_t rainbow_table[256];

static int reset(sensor_t *sensor)
{
    memset(&handle, 0, sizeof(handle));

    h_res = 0;
    v_res = 0;
    h_mirror = false;
    v_flip = false;

    DCMI_PWDN_LOW();
    systick_sleep(10);

    DCMI_PWDN_HIGH();
    systick_sleep(10);

    DCMI_RESET_LOW();
    systick_sleep(10);

    DCMI_RESET_HIGH();
    systick_sleep(1000);

    LEP_CAMERA_PORT_DESC_T tmp_handle;

    bool okay = false;

    for (int i = 0; i < 1000; i++) {
        LEP_RESULT result = LEP_OpenPort(0, LEP_CCI_TWI, 0, &tmp_handle);

        if (result == LEP_OK) {
            okay = true;
            break;
        } else {
            systick_sleep(1);
        }
    }

    if (!okay) {
        return -1;
    }

    bool booted = false;

    for (int i = 0; i < 1000; i++) {
        LEP_SDK_BOOT_STATUS_E status;
        if (LEP_GetCameraBootStatus(&tmp_handle, &status) != LEP_OK) {
            return -1;
        }

        if (status == LEP_BOOT_STATUS_BOOTED) {
            booted = true;
            break;
        } else {
            systick_sleep(1);
        }
    }

    if (!booted) {
        return -1;
    }

    bool busy = true;

    for (int i = 0; i < 1000; i++) {
        LEP_UINT16 status;
        if (LEP_DirectReadRegister(&tmp_handle, LEP_I2C_STATUS_REG, &status) != LEP_OK) {
            return -1;
        }

        if (!(status & LEP_I2C_STATUS_BUSY_BIT_MASK)) {
            busy = false;
            break;
        } else {
            systick_sleep(1);
        }
    }

    if (busy) {
        return -1;
    }

    busy = true;

    for (int i = 0; i < 5000; i++) {
        LEP_SYS_STATUS_E status;
        if (LEP_GetSysFFCStatus(&tmp_handle, &status) != LEP_OK) {
            return -1;
        }

        if (status == LEP_SYS_STATUS_READY) {
            busy = false;
            break;
        } else {
            systick_sleep(1);
        }
    }

    if (busy) {
        return -1;
    }

    if (LEP_SetRadEnableState(&tmp_handle, LEP_RAD_DISABLE) != LEP_OK) {
        return -1;
    }

    LEP_AGC_ROI_T roi;

    if (LEP_GetAgcROI(&tmp_handle, &roi) != LEP_OK) {
        return -1;
    }

    int tmp_h_res = roi.endCol + 1;
    int tmp_v_res = roi.endRow + 1;

    if (LEP_SetAgcEnableState(&tmp_handle, LEP_AGC_ENABLE) != LEP_OK) {
        return -1;
    }

    if (LEP_SetAgcCalcEnableState(&tmp_handle, LEP_AGC_ENABLE) != LEP_OK) {
        return -1;
    }

    handle = tmp_handle;
    h_res = tmp_h_res;
    v_res = tmp_v_res;
    return 0;
}

static int sleep(sensor_t *sensor, int enable)
{
    if (enable) {
        DCMI_PWDN_LOW();
        systick_sleep(100);
    } else {
        DCMI_PWDN_HIGH();
        systick_sleep(100);
    }

    return 0;
}

static int read_reg(sensor_t *sensor, uint8_t reg_addr)
{
    uint16_t reg_data;
    if (cambus_readw2(sensor->slv_addr, reg_addr, &reg_data)) {
        return -1;
    }
    return reg_data;
}

static int write_reg(sensor_t *sensor, uint8_t reg_addr, uint16_t reg_data)
{
    return cambus_writew2(sensor->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    return 0;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
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

static int set_quality(sensor_t *sensor, int quality)
{
    return 0;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    return 0;
}

static int set_special_effect(sensor_t *sensor, sde_t sde)
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
    h_mirror = enable;
    return 0;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    v_flip = enable;
    return 0;
}

static int set_lens_correction(sensor_t *sensor, int enable, int radi, int coef)
{
    return 0;
}

static int snapshot(sensor_t *sensor, image_t *image)
{
    fb_update_jpeg_buffer();

    if ((!h_res) || (!v_res) || (!sensor->framesize) || (!sensor->pixformat)) {
        return -1;
    }

    int y_scaler = h_res / VOSPI_LINE_PIXELS;
    int vospi_line_size = sizeof(uint16_t) + sizeof(uint16_t) + (VOSPI_LINE_PIXELS * sizeof(uint16_t));
    uint8_t buffer[vospi_line_size];

    bool reset = false;
    uint32_t time = systick_current_millis();
    for (int y = 0; y < v_res;) {
        for (int x = 0; x < h_res;) {
            if (sys_tick_has_passed(time, (h_res > VOSPI_LINE_PIXELS) ? 3000 : 1000)) {
                return -1;
            }

            int state = __get_PRIMASK();
            __disable_irq();
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
            HAL_StatusTypeDef status = HAL_SPI_Receive(&SPIHandle, buffer, vospi_line_size, 1000);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
            __set_PRIMASK(state);

            if (status != HAL_OK) {
                return -1;
            }

            if ((buffer[0] & 0xF) == 0xF) {
                continue;
            }

            int ttt = (buffer[0] >> 4) & 0x7;
            int packet_num = ((buffer[0] << 8) | (buffer[1] << 0)) & 0xFFF;
            int crc = (buffer[2] << 8) | (buffer[3] << 0);

            buffer[0] &= 0x0F;
            buffer[1] &= 0xFF;
            buffer[2] = 0;
            buffer[3] = 0;

            if (CalcCRC16Bytes(vospi_line_size, (char *) buffer) != crc) {
                systick_sleep(200);
                y = x = 0;
                reset = false;
                continue;
            }

            if ((y == 0) && (x == 0) && (packet_num != 0)) {
                systick_sleep(200);
                y = x = 0;
                reset = false;
                continue;
            }

            if ((((y % (VOSPI_NUMBER_PACKETS / y_scaler)) * y_scaler) + (x / VOSPI_LINE_PIXELS)) != packet_num) {
                systick_sleep(200);
                y = x = 0;
                reset = false;
                continue;
            }

            if ((h_res > VOSPI_LINE_PIXELS)
            && (packet_num == VOSPI_SPECIAL_LINE)
            && (((y / (VOSPI_NUMBER_PACKETS / y_scaler)) + 1) != ttt)) {
                if (!ttt) {
                    reset = true;
                } else {
                    systick_sleep(200);
                    y = x = 0;
                    reset = false;
                    continue;
                }
            }

            image_t img;
            img.w = MAIN_FB()->u;
            img.h = MAIN_FB()->v;
            img.bpp = MAIN_FB()->bpp; // invalid
            img.data = MAIN_FB()->pixels; // valid

            float x_scale = resolution[sensor->framesize][0] / ((float) h_res);
            float y_scale = resolution[sensor->framesize][1] / ((float) v_res);
            // MAX == KeepAspectRationByExpanding - MIN == KeepAspectRatio
            float scale = IM_MAX(x_scale, y_scale);
            int x_offset = (resolution[sensor->framesize][0] - (h_res * scale)) / 2;
            int y_offset = (resolution[sensor->framesize][1] - (v_res * scale)) / 2;
            // The code below upscales the source image to the requested frame size
            // and then crops it to the window set by the user.

            for (int yyy = fast_floorf(y * scale) + y_offset,
                 yyyy = fast_ceilf((y + 1) * scale) + y_offset; yyy < yyyy; yyy++) {
                if ((MAIN_FB()->y <= yyy) && (yyy < (MAIN_FB()->y + MAIN_FB()->v))) {

                    for (int xxx = fast_floorf(x * scale) + x_offset,
                         xxxx = fast_ceilf((x + VOSPI_LINE_PIXELS) * scale) + x_offset; xxx < xxxx; xxx++) {
                        if ((MAIN_FB()->x <= xxx) && (xxx < (MAIN_FB()->x + MAIN_FB()->u))) {

                            int i = (xxx / scale) - x;
                            // Value is the 14-bit value from the FLIR IR camera.
                            // However, with AGC enabled only the bottom 8-bits are non-zero.
                            int value = ((buffer[(i*2)+4] << 8) | (buffer[(i*2)+5] << 0)) & 0x3FFF;

                            int t_x = xxx - MAIN_FB()->x;
                            int t_y = yyy - MAIN_FB()->y;

                            if (h_mirror) t_x = MAIN_FB()->u - t_x - 1;
                            if (v_flip) t_y = MAIN_FB()->v - t_y - 1;

                            switch (sensor->pixformat) {
                                case PIXFORMAT_RGB565: {
                                    IMAGE_PUT_RGB565_PIXEL(&img, t_x, t_y, rainbow_table[value & 0xFF]);
                                    break;
                                }
                                case PIXFORMAT_GRAYSCALE: {
                                    IMAGE_PUT_GRAYSCALE_PIXEL(&img, t_x, t_y, value & 0xFF);
                                    break;
                                }
                                default: {
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            x += VOSPI_LINE_PIXELS;
        }

        y += 1;

        if (reset && (!(y % (VOSPI_NUMBER_PACKETS / y_scaler)))) {
            y -= VOSPI_NUMBER_PACKETS / y_scaler;
            reset = false;
        }
    }

    MAIN_FB()->w = MAIN_FB()->u;
    MAIN_FB()->h = MAIN_FB()->v;

    switch (sensor->pixformat) {
        case PIXFORMAT_RGB565: {
            MAIN_FB()->bpp = sizeof(uint16_t);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            MAIN_FB()->bpp = sizeof(uint8_t);
            break;
        }
        default: {
            break;
        }
    }

    image->w = MAIN_FB()->w;
    image->h = MAIN_FB()->h;
    image->bpp = MAIN_FB()->bpp;
    image->data = MAIN_FB()->pixels;

    return 0;
}

int lepton_init(sensor_t *sensor)
{
    memset(&SPIHandle, 0, sizeof(SPIHandle));

    SPIHandle.Instance               = SPI3;
    SPIHandle.Init.NSS               = SPI_NSS_SOFT;
    SPIHandle.Init.Mode              = SPI_MODE_MASTER;
    SPIHandle.Init.Direction         = SPI_DIRECTION_2LINES_RXONLY;
    SPIHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SPIHandle.Init.CLKPhase          = SPI_PHASE_2EDGE;
    SPIHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
    SPIHandle.Init.BaudRatePrescaler = LEPTON_SPI_PRESCALER;

    __HAL_RCC_SPI3_CLK_ENABLE();
    if (HAL_SPI_Init(&SPIHandle) != HAL_OK) {
        __HAL_RCC_SPI3_FORCE_RESET();
        __HAL_RCC_SPI3_RELEASE_RESET();
        __HAL_RCC_SPI3_CLK_DISABLE();
        return -1;
    }

    GPIO_InitTypeDef GPIO_InitTypeDefSS;
    GPIO_InitTypeDefSS.Pin = GPIO_PIN_15;
    GPIO_InitTypeDefSS.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitTypeDefSS.Pull = GPIO_PULLUP;
    GPIO_InitTypeDefSS.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitTypeDefSS.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitTypeDefSS);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);

    GPIO_InitTypeDef GPIO_InitTypeDefSCLK;
    GPIO_InitTypeDefSCLK.Pin = GPIO_PIN_3;
    GPIO_InitTypeDefSCLK.Mode = GPIO_MODE_AF_PP;
    GPIO_InitTypeDefSCLK.Pull = GPIO_PULLUP;
    GPIO_InitTypeDefSCLK.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitTypeDefSCLK.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitTypeDefSCLK);

    GPIO_InitTypeDef GPIO_InitTypeDefMISO;
    GPIO_InitTypeDefMISO.Pin = GPIO_PIN_4;
    GPIO_InitTypeDefMISO.Mode = GPIO_MODE_AF_PP;
    GPIO_InitTypeDefMISO.Pull = GPIO_PULLUP;
    GPIO_InitTypeDefMISO.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitTypeDefMISO.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitTypeDefMISO);

    GPIO_InitTypeDef GPIO_InitTypeDefMOSI;
    GPIO_InitTypeDefMOSI.Pin = GPIO_PIN_5;
    GPIO_InitTypeDefMOSI.Mode = GPIO_MODE_AF_PP;
    GPIO_InitTypeDefMOSI.Pull = GPIO_PULLUP;
    GPIO_InitTypeDefMOSI.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitTypeDefMOSI.Alternate = GPIO_AF7_SPI3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitTypeDefMOSI);

    sensor->gs_bpp              = sizeof(uint8_t);
    sensor->reset               = reset;
    sensor->sleep               = sleep;
    sensor->snapshot            = snapshot;
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
    sensor->set_special_effect  = set_special_effect;
    sensor->set_auto_gain       = set_auto_gain;
    sensor->get_gain_db         = get_gain_db;
    sensor->set_auto_exposure   = set_auto_exposure;
    sensor->get_exposure_us     = get_exposure_us;
    sensor->set_auto_whitebal   = set_auto_whitebal;
    sensor->get_rgb_gain_db     = get_rgb_gain_db;
    sensor->set_hmirror         = set_hmirror;
    sensor->set_vflip           = set_vflip;
    sensor->set_lens_correction = set_lens_correction;

    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_VSYNC, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_HSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_PIXCK, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_FSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_JPEGE, 0);

    return 0;
}
#else
int lepton_init(sensor_t *sensor)
{
    return -1;
}
#endif //defined(OMV_ENABLE_LEPTON)
