/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor abstraction layer.
 *
 */
#ifndef __SENSOR_H__
#define __SENSOR_H__
#include <stdint.h>
#include "imlib.h"

#define OV9650_ID       (0x96)
#define OV2640_ID       (0x26)
#define OV7725_ID       (0x77)
#define MT9V034_ID      (0x13)

typedef enum {
    PIXFORMAT_BAYER,     // 1BPP/RAW
    PIXFORMAT_RGB565,    // 2BPP/RGB565
    PIXFORMAT_YUV422,    // 2BPP/YUV422
    PIXFORMAT_GRAYSCALE, // 1BPP/GRAYSCALE
    PIXFORMAT_JPEG,      // JPEG/COMPRESSED
} pixformat_t;

typedef enum {
    // C/SIF Resolutions
    FRAMESIZE_QQCIF,    // 88x72
    FRAMESIZE_QCIF,     // 176x144
    FRAMESIZE_CIF,      // 352x288
    FRAMESIZE_QQSIF,    // 88x60
    FRAMESIZE_QSIF,     // 176x120
    FRAMESIZE_SIF,      // 352x240
    // VGA Resolutions
    FRAMESIZE_QQQQVGA,  // 40x30
    FRAMESIZE_QQQVGA,   // 80x60
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QVGA,     // 320x240
    FRAMESIZE_VGA,      // 640x480
    FRAMESIZE_HQQQVGA,  // 60x40
    FRAMESIZE_HQQVGA,   // 120x80
    FRAMESIZE_HQVGA,    // 240x160
    // FFT Resolutions
    FRAMESIZE_64X32,    // 64x32
    FRAMESIZE_64X64,    // 64x64
    FRAMESIZE_128X64,   // 128x64
    FRAMESIZE_128X128,  // 128x128
    // Other
    FRAMESIZE_LCD,      // 128x160
    FRAMESIZE_QQVGA2,   // 128x160
    FRAMESIZE_SVGA,     // 800x600
    FRAMESIZE_SXGA,     // 1280x1024
    FRAMESIZE_UXGA,     // 1600x1200
} framesize_t;

typedef enum {
    FRAMERATE_2FPS =0x9F,
    FRAMERATE_8FPS =0x87,
    FRAMERATE_15FPS=0x83,
    FRAMERATE_30FPS=0x81,
    FRAMERATE_60FPS=0x80,
} framerate_t;

typedef enum {
    GAINCEILING_2X,
    GAINCEILING_4X,
    GAINCEILING_8X,
    GAINCEILING_16X,
    GAINCEILING_32X,
    GAINCEILING_64X,
    GAINCEILING_128X,
} gainceiling_t;

typedef enum {
    SDE_NORMAL,
    SDE_NEGATIVE,
} sde_t;

typedef enum {
    ATTR_CONTRAST=0,
    ATTR_BRIGHTNESS,
    ATTR_SATURATION,
    ATTR_GAINCEILING,
} sensor_attr_t;

typedef enum {
    ACTIVE_LOW,
    ACTIVE_HIGH
} reset_polarity_t;

typedef void (*line_filter_t) (uint8_t *src, int src_stride, uint8_t *dst, int dst_stride, void *args);

#define SENSOR_HW_FLAGS_VSYNC        (0) // vertical sync polarity.
#define SENSOR_HW_FLAGS_HSYNC        (1) // horizontal sync polarity.
#define SENSOR_HW_FLAGS_PIXCK        (2) // pixel clock edge.
#define SENSOR_HW_FLAGS_FSYNC        (3) // hardware frame sync.
#define SENSOR_HW_FLAGS_JPEGE        (4) // hardware JPEG encoder.
#define SENSOR_HW_FLAGS_GET(s, x)    ((s)->hw_flags &  (1<<x))
#define SENSOR_HW_FLAGS_SET(s, x, v) ((s)->hw_flags |= (v<<x))
#define SENSOR_HW_FLAGS_CLR(s, x)    ((s)->hw_flags &= ~(1<<x))

typedef struct _sensor sensor_t;
typedef struct _sensor {
    uint8_t  chip_id;           // Sensor ID.
    uint8_t  slv_addr;          // Sensor I2C slave address.
    uint16_t gs_bpp;            // Grayscale bytes per pixel.
    uint32_t hw_flags;          // Hardware flags (clock polarities/hw capabilities)
    uint32_t vsync_pin;
    GPIO_TypeDef *vsync_gpio;
    int fb_w, fb_h;             // Backup for MAIN_FB().

    // Line pre-processing function and args
    void *line_filter_args;
    line_filter_t line_filter_func;

    reset_polarity_t reset_pol; // Reset polarity (TODO move to hw_flags)

    // Sensor state
    sde_t sde;                  // Special digital effects
    pixformat_t pixformat;      // Pixel format
    framesize_t framesize;      // Frame size
    framerate_t framerate;      // Frame rate
    gainceiling_t gainceiling;  // AGC gainceiling

    // Sensor function pointers
    int  (*reset)               (sensor_t *sensor);
    int  (*sleep)               (sensor_t *sensor, int enable);
    int  (*read_reg)            (sensor_t *sensor, uint8_t reg_addr);
    int  (*write_reg)           (sensor_t *sensor, uint8_t reg_addr, uint16_t reg_data);
    int  (*set_pixformat)       (sensor_t *sensor, pixformat_t pixformat);
    int  (*set_framesize)       (sensor_t *sensor, framesize_t framesize);
    int  (*set_framerate)       (sensor_t *sensor, framerate_t framerate);
    int  (*set_contrast)        (sensor_t *sensor, int level);
    int  (*set_brightness)      (sensor_t *sensor, int level);
    int  (*set_saturation)      (sensor_t *sensor, int level);
    int  (*set_gainceiling)     (sensor_t *sensor, gainceiling_t gainceiling);
    int  (*set_quality)         (sensor_t *sensor, int quality);
    int  (*set_colorbar)        (sensor_t *sensor, int enable);
    int  (*set_auto_gain)       (sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling);
    int  (*get_gain_db)         (sensor_t *sensor, float *gain_db);
    int  (*set_auto_exposure)   (sensor_t *sensor, int enable, int exposure_us);
    int  (*get_exposure_us)     (sensor_t *sensor, int *exposure_us);
    int  (*set_auto_whitebal)   (sensor_t *sensor, int enable, float r_gain_db, float g_gain_db, float b_gain_db);
    int  (*get_rgb_gain_db)     (sensor_t *sensor, float *r_gain_db, float *g_gain_db, float *b_gain_db);
    int  (*set_hmirror)         (sensor_t *sensor, int enable);
    int  (*set_vflip)           (sensor_t *sensor, int enable);
    int  (*set_special_effect)  (sensor_t *sensor, sde_t sde);
    int  (*set_lens_correction) (sensor_t *sensor, int enable, int radi, int coef);
} sensor_t;

// Resolution table
extern const int resolution[][2];

// Initialize the sensor hardware and probe the image sensor.
int sensor_init();

// Initialize the sensor state.
void sensor_init0();

// Reset the sensor to its default state.
int sensor_reset();

// Return sensor PID.
int sensor_get_id();

// Sleep mode.
int sensor_sleep(int enable);

// Read a sensor register.
int sensor_read_reg(uint8_t reg_addr);

// Write a sensor register.
int sensor_write_reg(uint8_t reg_addr, uint16_t reg_data);

// Set the sensor pixel format.
int sensor_set_pixformat(pixformat_t pixformat);

// Set the sensor frame size.
int sensor_set_framesize(framesize_t framesize);

// Set the sensor frame rate.
int sensor_set_framerate(framerate_t framerate);

// Set window size.
int sensor_set_windowing(int x, int y, int w, int h);

// Set the sensor contrast level (from -3 to +3).
int sensor_set_contrast(int level);

// Set the sensor brightness level (from -3 to +3).
int sensor_set_brightness(int level);

// Set the sensor saturation level (from -3 to +3).
int sensor_set_saturation(int level);

// Set the sensor AGC gain ceiling.
// Note: This function has no effect when AGC (Automatic Gain Control) is disabled.
int sensor_set_gainceiling(gainceiling_t gainceiling);

// Set the quantization scale factor, controls JPEG quality (quality 0-255).
int sensor_set_quality(int qs);

// Enable/disable the colorbar mode.
int sensor_set_colorbar(int enable);

// Enable auto gain or set value manually.
int sensor_set_auto_gain(int enable, float gain_db, float gain_db_ceiling);

// Get the gain value.
int sensor_get_gain_db(float *gain_db);

// Enable auto exposure or set value manually.
int sensor_set_auto_exposure(int enable, int exposure_us);

// Get the exposure value.
int sensor_get_exposure_us(int *get_exposure_us);

// Enable auto white balance or set value manually.
int sensor_set_auto_whitebal(int enable, float r_gain_db, float g_gain_db, float b_gain_db);

// Get the rgb gain values.
int sensor_get_rgb_gain_db(float *r_gain_db, float *g_gain_db, float *b_gain_db);

// Enable/disable the hmirror mode.
int sensor_set_hmirror(int enable);

// Enable/disable the vflip mode.
int sensor_set_vflip(int enable);

// Set special digital effects (SDE).
int sensor_set_special_effect(sde_t sde);

// Set lens shading correction
int sensor_set_lens_correction(int enable, int radi, int coef);

// Set filter function.
int sensor_set_line_filter(line_filter_t line_filter_func, void *line_filter_args);

// Set vsync output pin
int sensor_set_vsync_output(GPIO_TypeDef *gpio, uint32_t pin);

// Capture a Snapshot.
int sensor_snapshot(image_t *image, line_filter_t line_filter_func, void *line_filter_args);
#endif /* __SENSOR_H__ */
