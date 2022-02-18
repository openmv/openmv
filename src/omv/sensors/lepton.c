/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Lepton driver.
 */
#include "omv_boardconfig.h"
#if (OMV_ENABLE_LEPTON == 1)

#include STM32_HAL_H
#include "irq.h"
#include "cambus.h"
#include "sensor.h"
#include "py/mphal.h"
#include "framebuffer.h"
#include "common.h"

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
#define VOSPI_SPECIAL_PACKET    (20)
#define VOSPI_LINE_SIZE         (80 * 2)
#define VOSPI_HEADER_SIZE       (4)
#define VOSPI_PACKET_SIZE       (VOSPI_HEADER_SIZE + VOSPI_LINE_SIZE)
#define VOSPI_HEADER_SEG(buf)   (((buf[0] >> 4) & 0x7))
#define VOSPI_HEADER_PID(buf)   (((buf[0] << 8) | (buf[1] << 0)) & 0x0FFF)
#define VOSPI_HEADER_CRC(buf)   (((buf[2] << 8) | (buf[3] << 0)))
#define VOSPI_FIRST_PACKET      (0)
#define VOSPI_FIRST_SEGMENT     (1)
#define LEPTON_TIMEOUT          (1000)
// Temperatures in Celsius
#define DEFAULT_MIN_TEMP        (-10.0f)
#define DEFAULT_MAX_TEMP        (40.0f)
#define LEPTON_MIN_TEMP_NORM    (-10.0f)
#define LEPTON_MAX_TEMP_NORM    (140.0f)
#define LEPTON_MIN_TEMP_HIGH    (-10.0f)
#define LEPTON_MAX_TEMP_HIGH    (600.0f)

static bool radiometry = false;
static int h_res = 0;
static int v_res = 0;
static bool v_flip = false;
static bool h_mirror = false;
static bool measurement_mode = false;
static bool high_temp_mode = false;
static float min_temp = DEFAULT_MIN_TEMP;
static float max_temp = DEFAULT_MAX_TEMP;

extern SPI_HandleTypeDef ISC_SPIHandle;
static DMA_HandleTypeDef DMAHandle;
LEP_CAMERA_PORT_DESC_T   LEPHandle;
extern uint8_t _line_buf[];
extern uint8_t _vospi_buf[];

static bool vospi_resync = true;
static uint8_t *vospi_packet = _line_buf;
static uint8_t *vospi_buffer = _vospi_buf;
static volatile uint32_t vospi_pid = 0;
static volatile uint32_t vospi_seg = 1;
static uint32_t vospi_packets = 60;
static int lepton_reset(sensor_t *sensor, bool measurement_mode, bool high_temp_mode);

static void lepton_sync()
{
    HAL_SPI_Abort(&ISC_SPIHandle);

    // Disable DMA IRQ
    HAL_NVIC_DisableIRQ(ISC_SPI_DMA_IRQn);

    debug_printf("resync...\n");
    mp_hal_delay_ms(200);

    vospi_resync = false;
    vospi_pid = VOSPI_FIRST_PACKET;
    vospi_seg = VOSPI_FIRST_SEGMENT;

    HAL_NVIC_EnableIRQ(ISC_SPI_DMA_IRQn);
    HAL_SPI_Receive_DMA(&ISC_SPIHandle, vospi_packet, VOSPI_PACKET_SIZE);
}

static uint16_t lepton_calc_crc(uint8_t *buf)
{
    buf[0] &= 0x0F;
    buf[1] &= 0xFF;
    buf[2] = 0;
    buf[3] = 0;
    return CalcCRC16Bytes(VOSPI_PACKET_SIZE, (char *) buf);
}

static int sleep(sensor_t *sensor, int enable)
{
    if (enable) {
        DCMI_PWDN_LOW();
        mp_hal_delay_ms(100);
    } else {
        DCMI_PWDN_HIGH();
        mp_hal_delay_ms(100);
    }

    return 0;
}

static int read_reg(sensor_t *sensor, uint16_t reg_addr)
{
    uint16_t reg_data;
    if (cambus_readw2(&sensor->bus, sensor->slv_addr, reg_addr, &reg_data)) {
        return -1;
    }
    return reg_data;
}

static int write_reg(sensor_t *sensor, uint16_t reg_addr, uint16_t reg_data)
{
    return cambus_writew2(&sensor->bus, sensor->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    return ((pixformat != PIXFORMAT_GRAYSCALE) && (pixformat != PIXFORMAT_RGB565)) ? - 1 : 0;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
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

static int ioctl(sensor_t *sensor, int request, va_list ap)
{
    int ret = 0;

    if ((!h_res) || (!v_res)) {
        return -1;
    }

    switch (request) {
        case IOCTL_LEPTON_GET_WIDTH: {
            int *width = va_arg(ap, int *);
            *width = h_res;
            break;
        }
        case IOCTL_LEPTON_GET_HEIGHT: {
            int *height = va_arg(ap, int *);
            *height = v_res;
            break;
        }
        case IOCTL_LEPTON_GET_RADIOMETRY: {
            int *type = va_arg(ap, int *);
            *type = radiometry;
            break;
        }
        case IOCTL_LEPTON_GET_REFRESH: {
            int *refresh = va_arg(ap, int *);
            *refresh = (h_res == 80) ? 27 : 9;
            break;
        }
        case IOCTL_LEPTON_GET_RESOLUTION: {
            int *resolution = va_arg(ap, int *);
            *resolution = 14;
            break;
        }
        case IOCTL_LEPTON_RUN_COMMAND: {
            int command = va_arg(ap, int);
            ret = (LEP_RunCommand(&LEPHandle, command) == LEP_OK) ? 0 : -1;
            break;
        }
        case IOCTL_LEPTON_SET_ATTRIBUTE: {
            int command = va_arg(ap, int);
            uint16_t *data = va_arg(ap, uint16_t *);
            size_t data_len = va_arg(ap, size_t);
            ret = (LEP_SetAttribute(&LEPHandle, command, (LEP_ATTRIBUTE_T_PTR) data, data_len) == LEP_OK) ? 0 : -1;
            break;
        }
        case IOCTL_LEPTON_GET_ATTRIBUTE: {
            int command = va_arg(ap, int);
            uint16_t *data = va_arg(ap, uint16_t *);
            size_t data_len = va_arg(ap, size_t);
            ret = (LEP_GetAttribute(&LEPHandle, command, (LEP_ATTRIBUTE_T_PTR) data, data_len) == LEP_OK) ? 0 : -1;
            break;
        }
        case IOCTL_LEPTON_GET_FPA_TEMPERATURE: {
            int *temp = va_arg(ap, int *);
            LEP_SYS_FPA_TEMPERATURE_KELVIN_T tfpa;
            ret = (LEP_GetSysFpaTemperatureKelvin(&LEPHandle, &tfpa) == LEP_OK) ? 0 : -1;
            *temp = tfpa;
            break;
        }
        case IOCTL_LEPTON_GET_AUX_TEMPERATURE: {
            int *temp = va_arg(ap, int *);
            LEP_SYS_AUX_TEMPERATURE_KELVIN_T taux;
            ret = (LEP_GetSysAuxTemperatureKelvin(&LEPHandle, &taux) == LEP_OK) ? 0 : -1;
            *temp = taux;
            break;
        }
        case IOCTL_LEPTON_SET_MEASUREMENT_MODE: {
            int measurement_mode_in = va_arg(ap, int);
            int high_temp_mode_in = va_arg(ap, int);
            if (measurement_mode != measurement_mode_in) {
                measurement_mode = measurement_mode_in;
                high_temp_mode = high_temp_mode_in;
                ret = lepton_reset(sensor, measurement_mode, high_temp_mode);
            }
            break;
        }
        case IOCTL_LEPTON_GET_MEASUREMENT_MODE: {
            int *measurement_mode_out = va_arg(ap, int *);
            int *high_temp_mode_out = va_arg(ap, int *);
            *measurement_mode_out = measurement_mode;
            *high_temp_mode_out = high_temp_mode;
            break;
        }
        case IOCTL_LEPTON_SET_MEASUREMENT_RANGE: {
            float *arg_min_temp = va_arg(ap, float *);
            float *arg_max_temp = va_arg(ap, float *);
            float min_temp_range = (high_temp_mode) ? LEPTON_MIN_TEMP_HIGH : LEPTON_MIN_TEMP_NORM;
            float max_temp_range = (high_temp_mode) ? LEPTON_MAX_TEMP_HIGH : LEPTON_MAX_TEMP_NORM;
            min_temp = IM_MAX(IM_MIN(*arg_min_temp, *arg_max_temp), min_temp_range);
            max_temp = IM_MIN(IM_MAX(*arg_max_temp, *arg_min_temp), max_temp_range);
            break;
        }
        case IOCTL_LEPTON_GET_MEASUREMENT_RANGE: {
            float *ptr_min_temp = va_arg(ap, float *);
            float *ptr_max_temp = va_arg(ap, float *);
            *ptr_min_temp = min_temp;
            *ptr_max_temp = max_temp;
            break;
        }
        default: {
            ret = -1;
            break;
        }
    }

    return ret;
}


static int lepton_reset(sensor_t *sensor, bool measurement_mode, bool high_temp_mode)
{
    DCMI_PWDN_LOW();
    mp_hal_delay_ms(10);

    DCMI_PWDN_HIGH();
    mp_hal_delay_ms(10);

    DCMI_RESET_LOW();
    mp_hal_delay_ms(10);

    DCMI_RESET_HIGH();
    mp_hal_delay_ms(1000);

    LEP_RAD_ENABLE_E rad;
    LEP_AGC_ROI_T roi;
    memset(&LEPHandle, 0, sizeof(LEP_CAMERA_PORT_DESC_T));

    for (mp_uint_t start = mp_hal_ticks_ms(); ;mp_hal_delay_ms(1)) {
        if (LEP_OpenPort(&sensor->bus, LEP_CCI_TWI, 0, &LEPHandle) == LEP_OK) {
            break;
        }
        if ((mp_hal_ticks_ms() - start) >= LEPTON_TIMEOUT) {
            return -1;
        }
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); ;mp_hal_delay_ms(1)) {
        LEP_SDK_BOOT_STATUS_E status;
        if (LEP_GetCameraBootStatus(&LEPHandle, &status) != LEP_OK) {
            return -1;
        }
        if (status == LEP_BOOT_STATUS_BOOTED) {
            break;
        }
        if ((mp_hal_ticks_ms() - start) >= LEPTON_TIMEOUT) {
            return -1;
        }
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); ;mp_hal_delay_ms(1)) {
        LEP_UINT16 status;
        if (LEP_DirectReadRegister(&LEPHandle, LEP_I2C_STATUS_REG, &status) != LEP_OK) {
            return -1;
        }
        if (!(status & LEP_I2C_STATUS_BUSY_BIT_MASK)) {
            break;
        }
        if ((mp_hal_ticks_ms() - start) >= LEPTON_TIMEOUT) {
            return -1;
        }
    }

    if (LEP_GetRadEnableState(&LEPHandle, &rad) != LEP_OK
        || LEP_GetAgcROI(&LEPHandle, &roi) != LEP_OK) {
        return -1;
    }

    // Use the low gain mode to enable high temperature readings (~450C) on Lepton 3.5
    LEP_SYS_GAIN_MODE_E gain_mode = high_temp_mode ? LEP_SYS_GAIN_MODE_LOW : LEP_SYS_GAIN_MODE_HIGH;
    if (LEP_SetSysGainMode(&LEPHandle, gain_mode) != LEP_OK) {
        return -1;
    }

    if (!measurement_mode) {
        if (LEP_SetRadEnableState(&LEPHandle, LEP_RAD_DISABLE) != LEP_OK
            || LEP_SetAgcEnableState(&LEPHandle, LEP_AGC_ENABLE) != LEP_OK
            || LEP_SetAgcCalcEnableState(&LEPHandle, LEP_AGC_ENABLE) != LEP_OK) {
            return -1;
        }
    }

    h_res = roi.endCol + 1;
    v_res = roi.endRow + 1;
    radiometry = (rad == LEP_RAD_ENABLE);

    if (v_res > 60) {
        vospi_packets = 240;
    } else {
        vospi_packets = 60;
    }

    // resync and enable DMA before the first snapshot.
    vospi_resync = true;
    return 0;
}

static int reset(sensor_t *sensor)
{
    h_res = 0;
    v_res = 0;
    v_flip = false;
    h_mirror = false;
    radiometry = false;
    measurement_mode = false;
    high_temp_mode = false;
    min_temp = DEFAULT_MIN_TEMP;
    max_temp = DEFAULT_MAX_TEMP;
    return lepton_reset(sensor, false, false);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    (void) lepton_calc_crc; // to shut the compiler up.

    if (vospi_resync == true) {
        return; // nothing to do here
    }

    if (vospi_pid < vospi_packets && (vospi_packet[0] & 0xF) != 0xF) {
        uint32_t pid = VOSPI_HEADER_PID(vospi_packet);
        uint32_t seg = VOSPI_HEADER_SEG(vospi_packet);
        if (pid != (vospi_pid % VOSPI_NUMBER_PACKETS)) {
            if (vospi_pid == VOSPI_FIRST_PACKET) {
                // Wait for the first packet of the first segement.
                vospi_pid = VOSPI_FIRST_PACKET;
                vospi_seg = VOSPI_FIRST_SEGMENT;
            } else { // lost sync
                vospi_resync = true;
                debug_printf("lost sync, packet id:%lu expected id:%lu \n", pid, vospi_pid);
            }
        } else if (vospi_packets > 60 && pid == VOSPI_SPECIAL_PACKET && seg != vospi_seg ) {
            if (vospi_seg == VOSPI_FIRST_SEGMENT) {
                // Wait for the first packet of the first segement.
                vospi_pid = VOSPI_FIRST_PACKET;
                vospi_seg = VOSPI_FIRST_SEGMENT;
            } else { // lost sync
                vospi_resync = true;
                debug_printf("lost sync, segment id:%lu expected id:%lu\n", seg, vospi_seg);
            }
        } else {
            memcpy(vospi_buffer + vospi_pid * VOSPI_LINE_SIZE,
                    vospi_packet + VOSPI_HEADER_SIZE, VOSPI_LINE_SIZE);
            if ((++vospi_pid % VOSPI_NUMBER_PACKETS) == 0) {
                vospi_seg++;
            }
        }
    }
}

static int snapshot(sensor_t *sensor, image_t *image, uint32_t flags)
{
    framebuffer_update_jpeg_buffer();

    if (MAIN_FB()->n_buffers != 1) {
        framebuffer_set_buffers(1);
    }

    if (sensor_check_framebuffer_size(sensor) == -1) {
        return -1;
    }

    if ((!h_res) || (!v_res) || (!sensor->framesize) || (!sensor->pixformat)) {
        return -1;
    }

    framebuffer_free_current_buffer();
    vbuffer_t *buffer = framebuffer_get_tail(FB_NO_FLAGS);

    if (!buffer) {
        return -1;
    }

    // The SPI DMA device is always clocking the FLIR Lepton in the background.
    // The code below resets the vospi control values to let data be pulled in.
    // If we need to re-sync we do it. Otherwise, after we finish pulling data
    // in we exit and let the SPI bus keep running. Then on the next call to
    // snapshot we read in more data and pull in the next frame.
    HAL_NVIC_DisableIRQ(ISC_SPI_DMA_IRQn);
    vospi_pid = VOSPI_FIRST_PACKET;
    vospi_seg = VOSPI_FIRST_SEGMENT;
    HAL_NVIC_EnableIRQ(ISC_SPI_DMA_IRQn);

    // Snapshot start tick
    mp_uint_t tick_start = mp_hal_ticks_ms();
    bool reset_tried = false;

    do {
        if (vospi_resync == true) {
            lepton_sync();
        }

        __WFI();

        if ((mp_hal_ticks_ms() - tick_start) >= 20000) {
            // Timeout error.
            return -1;
        }

        if ((!reset_tried) && ((mp_hal_ticks_ms() - tick_start) >= 10000)) {
            reset_tried = true;

            // The FLIR lepton might have crashed so reset it (it does this).
            bool temp_h_mirror = h_mirror;
            bool temp_v_flip = v_flip;
            int ret = lepton_reset(sensor, measurement_mode, high_temp_mode);
            h_mirror = temp_h_mirror;
            v_flip = temp_v_flip;

            if (ret < 0) {
                return -1;
            }

            // Reset the VOSPI interface again.
            HAL_NVIC_DisableIRQ(ISC_SPI_DMA_IRQn);
            vospi_pid = VOSPI_FIRST_PACKET;
            vospi_seg = VOSPI_FIRST_SEGMENT;
            HAL_NVIC_EnableIRQ(ISC_SPI_DMA_IRQn);
        }
    } while (vospi_pid < vospi_packets); // only checking one volatile var so atomic.

    MAIN_FB()->w        = MAIN_FB()->u;
    MAIN_FB()->h        = MAIN_FB()->v;
    MAIN_FB()->pixfmt   = sensor->pixformat;

    framebuffer_init_image(image);

    uint16_t *src = (uint16_t*) vospi_buffer;

    float x_scale = resolution[sensor->framesize][0] / ((float) h_res);
    float y_scale = resolution[sensor->framesize][1] / ((float) v_res);
    // MAX == KeepAspectRationByExpanding - MIN == KeepAspectRatio
    float scale = IM_MAX(x_scale, y_scale), scale_inv = 1.0f / scale;
    int x_offset = (resolution[sensor->framesize][0] - (h_res * scale)) / 2;
    int y_offset = (resolution[sensor->framesize][1] - (v_res * scale)) / 2;
    // The code below upscales the source image to the requested frame size
    // and then crops it to the window set by the user.

    LEP_SYS_FPA_TEMPERATURE_KELVIN_T kelvin;
    if (measurement_mode && (!radiometry)) {
        if (LEP_GetSysFpaTemperatureKelvin(&LEPHandle, &kelvin) != LEP_OK) {
            return -1;
        }
    }

    for (int y = y_offset, yy = fast_ceilf(v_res * scale) + y_offset; y < yy; y++) {
        if ((MAIN_FB()->y <= y) && (y < (MAIN_FB()->y + MAIN_FB()->v))) { // user window cropping

            uint16_t *row_ptr = src + (fast_floorf(y * scale_inv) * h_res);

            for (int x = x_offset, xx = fast_ceilf(h_res * scale) + x_offset; x < xx; x++) {
                if ((MAIN_FB()->x <= x) && (x < (MAIN_FB()->x + MAIN_FB()->u))) { // user window cropping

                    // Value is the 14/16-bit value from the FLIR IR camera.
                    // However, with AGC enabled only the bottom 8-bits are non-zero.
                    int value = __REV16(row_ptr[fast_floorf(x * scale_inv)]);

                    if (measurement_mode) {
                        // Need to convert 14/16-bits to 8-bits ourselves...
                        if (!radiometry) value = (value - 8192) + kelvin;
                        float celsius = (value * 0.01f) - 273.15f;
                        celsius = IM_MAX(IM_MIN(celsius, max_temp), min_temp);
                        value = IM_MAX(IM_MIN(IM_DIV(((celsius - min_temp) * 255), (max_temp - min_temp)), 255), 0);
                    }

                    int t_x = x - MAIN_FB()->x;
                    int t_y = y - MAIN_FB()->y;

                    if (h_mirror) t_x = MAIN_FB()->u - t_x - 1;
                    if (v_flip) t_y = MAIN_FB()->v - t_y - 1;

                    switch (sensor->pixformat) {
                        case PIXFORMAT_GRAYSCALE: {
                            IMAGE_PUT_GRAYSCALE_PIXEL(image, t_x, t_y, value & 0xFF);
                            break;
                        }
                        case PIXFORMAT_RGB565: {
                            IMAGE_PUT_RGB565_PIXEL(image, t_x, t_y, sensor->color_palette[value & 0xFF]);
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

    return 0;
}

int lepton_init(sensor_t *sensor)
{
    sensor->reset               = reset;
    sensor->sleep               = sleep;
    sensor->snapshot            = snapshot;
    sensor->read_reg            = read_reg;
    sensor->write_reg           = write_reg;
    sensor->set_pixformat       = set_pixformat;
    sensor->set_framesize       = set_framesize;
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
    sensor->ioctl               = ioctl;

    sensor->hw_flags.vsync      = 1;
    sensor->hw_flags.hsync      = 0;
    sensor->hw_flags.pixck      = 0;
    sensor->hw_flags.fsync      = 0;
    sensor->hw_flags.jpege      = 0;
    sensor->hw_flags.gs_bpp     = 1;

    // Configure the DMA handler for Transmission process
    DMAHandle.Instance                 = ISC_SPI_DMA_STREAM;
    DMAHandle.Init.Request             = ISC_SPI_DMA_REQUEST;
    DMAHandle.Init.Mode                = DMA_CIRCULAR;
    DMAHandle.Init.Priority            = DMA_PRIORITY_HIGH;
    DMAHandle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    // When the DMA is configured in direct mode (the FIFO is disabled), the source and
    // destination transfer widths are equal, and both defined by PSIZE (MSIZE is ignored).
    // Additionally, burst transfers are not possible (MBURST and PBURST are both ignored).
    DMAHandle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    DMAHandle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    // Note MBURST and PBURST are ignored.
    DMAHandle.Init.MemBurst            = DMA_MBURST_INC4;
    DMAHandle.Init.PeriphBurst         = DMA_PBURST_INC4;
    DMAHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    DMAHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    DMAHandle.Init.MemInc              = DMA_MINC_ENABLE;
    DMAHandle.Init.PeriphInc           = DMA_PINC_DISABLE;

    // NVIC configuration for DMA transfer complete interrupt
    NVIC_SetPriority(ISC_SPI_DMA_IRQn, IRQ_PRI_DMA21);
    HAL_NVIC_DisableIRQ(ISC_SPI_DMA_IRQn);

    HAL_DMA_DeInit(&DMAHandle);
    if (HAL_DMA_Init(&DMAHandle) != HAL_OK) {
        // Initialization Error
        return -1;
    }

    memset(&ISC_SPIHandle, 0, sizeof(ISC_SPIHandle));
    ISC_SPIHandle.Instance               = ISC_SPI;
    ISC_SPIHandle.Init.NSS               = SPI_NSS_HARD_OUTPUT;
    ISC_SPIHandle.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
    ISC_SPIHandle.Init.NSSPolarity       = SPI_NSS_POLARITY_LOW;
    ISC_SPIHandle.Init.Mode              = SPI_MODE_MASTER;
    ISC_SPIHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
    ISC_SPIHandle.Init.Direction         = SPI_DIRECTION_2LINES_RXONLY;
    ISC_SPIHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    ISC_SPIHandle.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_04DATA;
    ISC_SPIHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    ISC_SPIHandle.Init.CLKPhase          = SPI_PHASE_2EDGE;
    ISC_SPIHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
    ISC_SPIHandle.Init.BaudRatePrescaler = ISC_SPI_PRESCALER;
    // Recommanded setting to avoid glitches
    ISC_SPIHandle.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;

    if (HAL_SPI_Init(&ISC_SPIHandle) != HAL_OK) {
        ISC_SPI_RESET();
        ISC_SPI_RELEASE();
        ISC_SPI_CLK_DISABLE();
        return -1;
    }

    // Associate the initialized DMA handle to the the SPI handle
    __HAL_LINKDMA(&ISC_SPIHandle, hdmarx, DMAHandle);

    // NVIC configuration for SPI transfer complete interrupt
    NVIC_SetPriority(ISC_SPI_IRQn, IRQ_PRI_DCMI);
    HAL_NVIC_EnableIRQ(ISC_SPI_IRQn);

    return 0;
}
#endif // (OMV_ENABLE_LEPTON == 1)
