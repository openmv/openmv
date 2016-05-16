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

#define OV9650_PID     (0x96)
#define OV2640_PID     (0x26)
#define OV7725_PID     (0x77)

typedef struct {
    uint8_t MIDH;
    uint8_t MIDL;
    uint8_t PID;
    uint8_t VER;
} sensor_id_t;

typedef enum {
    PIXFORMAT_RGB565,    // 2BPP/RGB565
    PIXFORMAT_YUV422,    // 2BPP/YUV422
    PIXFORMAT_GRAYSCALE, // 1BPP/GRAYSCALE
    PIXFORMAT_JPEG,      // JPEG/COMPRESSED
} pixformat_t;

typedef enum {
    FRAMESIZE_QQCIF,    // 88x72
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QQVGA2,   // 128x160
    FRAMESIZE_QCIF,     // 176x144
    FRAMESIZE_HQVGA,    // 220x160
    FRAMESIZE_QVGA,     // 320x240
    FRAMESIZE_CIF,      // 352x288
    FRAMESIZE_VGA,      // 640x480
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
#define SENSOR_HW_FLAGS_HW_JPEG      (4) // hardware JPEG encoder.
#define SENSOR_HW_FLAGS_SW_JPEG      (5) // software JPEG encoder enable/disable.
#define SENSOR_HW_FLAGS_GET(s, x)    ((s)->hw_flags &  (1<<x))
#define SENSOR_HW_FLAGS_SET(s, x, v) ((s)->hw_flags |= (v<<x))
#define SENSOR_HW_FLAGS_CLR(s, x)    ((s)->hw_flags &= ~(1<<x))

typedef struct _sensor sensor_t;
typedef struct _sensor {
    sensor_id_t id;             // Sensor ID.
    uint8_t  slv_addr;          // Sensor I2C slave address.
    uint32_t hw_flags;          // Hardware flags (clock polarities/hw capabilities)

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
    int  (*set_pixformat)       (sensor_t *sensor, pixformat_t pixformat);
    int  (*set_framesize)       (sensor_t *sensor, framesize_t framesize);
    int  (*set_framerate)       (sensor_t *sensor, framerate_t framerate);
    int  (*set_contrast)        (sensor_t *sensor, int level);
    int  (*set_brightness)      (sensor_t *sensor, int level);
    int  (*set_saturation)      (sensor_t *sensor, int level);
    int  (*set_gainceiling)     (sensor_t *sensor, gainceiling_t gainceiling);
    int  (*set_quality)         (sensor_t *sensor, int quality);
    int  (*set_colorbar)        (sensor_t *sensor, int enable);
    int  (*set_whitebal)        (sensor_t *sensor, int enable);
    int  (*set_gain_ctrl)       (sensor_t *sensor, int enable);
    int  (*set_exposure_ctrl)   (sensor_t *sensor, int enable);
    int  (*set_hmirror)         (sensor_t *sensor, int enable);
    int  (*set_vflip)           (sensor_t *sensor, int enable);
    int  (*set_special_effect)  (sensor_t *sensor, sde_t sde);
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

// Read a sensor register.
int sensor_read_reg(uint8_t reg);

// Write a sensor register.
int sensor_write_reg(uint8_t reg, uint8_t val);

// Enable/disable framebuffer JPEG compression.
// Note: Has nothing to do with HW JPEG.
int sensor_enable_jpeg(bool enable);

// Capture a Snapshot.
int sensor_snapshot(image_t *image, line_filter_t line_filter_func, void *line_filter_args);

// Capture the frame buffer.
int sensor_get_fb(image_t *img);

// Set the sensor pixel format.
int sensor_set_pixformat(pixformat_t pixformat);

// Set the sensor frame size.
int sensor_set_framesize(framesize_t framesize);

// Set the sensor frame rate.
int sensor_set_framerate(framerate_t framerate);

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

// Enable/disable the whitebal mode.
int sensor_set_whitebal(int enable);

// Enable/disable the agc mode.
int sensor_set_gain_ctrl(int enable);

// Enable/disable the aec mode.
int sensor_set_exposure_ctrl(int enable);

// Enable/disable the hmirror mode.
int sensor_set_hmirror(int enable);

// Enable/disable the vflip mode.
int sensor_set_vflip(int enable);

// Set special digital effects (SDE).
int sensor_set_special_effect(sde_t sde);

// Set filter function.
int sensor_set_line_filter(line_filter_t line_filter_func, void *line_filter_args);
#endif /* __SENSOR_H__ */
