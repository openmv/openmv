/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor abstraction layer.
 */
#ifndef __SENSOR_H__
#define __SENSOR_H__
#include <stdarg.h>
#include "omv_i2c.h"
#include "imlib.h"

#define OV2640_SLV_ADDR         (0x60)
#define OV5640_SLV_ADDR         (0x78)
#define OV7725_SLV_ADDR         (0x42)
#define MT9V0XX_SLV_ADDR        (0xB8)
#define MT9M114_SLV_ADDR        (0x90)
#define LEPTON_SLV_ADDR         (0x54)
#define HM0XX0_SLV_ADDR         (0x48)
#define GC2145_SLV_ADDR         (0x78)
#define FROGEYE2020_SLV_ADDR    (0x6E)

// Chip ID Registers
#define OV5640_CHIP_ID          (0x300A)
#define OV_CHIP_ID              (0x0A)
#define ON_CHIP_ID              (0x00)
#define HIMAX_CHIP_ID           (0x0001)
#define GC_CHIP_ID              (0xF0)

// Chip ID Values
#define OV2640_ID               (0x26)
#define OV5640_ID               (0x56)
#define OV7670_ID               (0x76)
#define OV7690_ID               (0x76)
#define OV7725_ID               (0x77)
#define OV9650_ID               (0x96)
#define MT9V0X2_ID_V_1          (0x1311)
#define MT9V0X2_ID_V_2          (0x1312)
#define MT9V0X2_ID              (0x1313)
#define MT9V0X2_C_ID            (0x1413)
#define MT9V0X4_ID              (0x1324)
#define MT9V0X4_C_ID            (0x1424)
#define MT9M114_ID              (0x2481)
#define LEPTON_ID               (0x54)
#define LEPTON_1_5              (0x5415)
#define LEPTON_1_6              (0x5416)
#define LEPTON_2_0              (0x5420)
#define LEPTON_2_5              (0x5425)
#define LEPTON_3_0              (0x5430)
#define LEPTON_3_5              (0x5435)
#define HM01B0_ID               (0xB0)
#define HM0360_ID               (0x60)
#define GC2145_ID               (0x21)
#define PAJ6100_ID              (0x6100)
#define FROGEYE2020_ID          (0x2020)

typedef enum {
    FRAMESIZE_INVALID = 0,
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
    FRAMESIZE_HQQQQVGA, // 30x20
    FRAMESIZE_HQQQVGA,  // 60x40
    FRAMESIZE_HQQVGA,   // 120x80
    FRAMESIZE_HQVGA,    // 240x160
    FRAMESIZE_HVGA,     // 480x320
    // FFT Resolutions
    FRAMESIZE_64X32,    // 64x32
    FRAMESIZE_64X64,    // 64x64
    FRAMESIZE_128X64,   // 128x64
    FRAMESIZE_128X128,  // 128x128
    // Himax Resolutions
    FRAMESIZE_160X160,  // 160x160
    FRAMESIZE_320X320,  // 320x320
    // Other
    FRAMESIZE_LCD,      // 128x160
    FRAMESIZE_QQVGA2,   // 128x160
    FRAMESIZE_WVGA,     // 720x480
    FRAMESIZE_WVGA2,    // 752x480
    FRAMESIZE_SVGA,     // 800x600
    FRAMESIZE_XGA,      // 1024x768
    FRAMESIZE_WXGA,     // 1280x768
    FRAMESIZE_SXGA,     // 1280x1024
    FRAMESIZE_SXGAM,    // 1280x960
    FRAMESIZE_UXGA,     // 1600x1200
    FRAMESIZE_HD,       // 1280x720
    FRAMESIZE_FHD,      // 1920x1080
    FRAMESIZE_QHD,      // 2560x1440
    FRAMESIZE_QXGA,     // 2048x1536
    FRAMESIZE_WQXGA,    // 2560x1600
    FRAMESIZE_WQXGA2,   // 2592x1944
} framesize_t;

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
} polarity_t;

typedef enum {
    XCLK_SOURCE_MCO = 0U,
    XCLK_SOURCE_TIM = 1U,
    XCLK_SOURCE_OSC = 2U,
} xclk_source_t;

typedef enum {
    IOCTL_SET_READOUT_WINDOW,
    IOCTL_GET_READOUT_WINDOW,
    IOCTL_SET_TRIGGERED_MODE,
    IOCTL_GET_TRIGGERED_MODE,
    IOCTL_SET_FOV_WIDE,
    IOCTL_GET_FOV_WIDE,
    IOCTL_TRIGGER_AUTO_FOCUS,
    IOCTL_PAUSE_AUTO_FOCUS,
    IOCTL_RESET_AUTO_FOCUS,
    IOCTL_WAIT_ON_AUTO_FOCUS,
    IOCTL_SET_NIGHT_MODE,
    IOCTL_GET_NIGHT_MODE,
    IOCTL_LEPTON_GET_WIDTH,
    IOCTL_LEPTON_GET_HEIGHT,
    IOCTL_LEPTON_GET_RADIOMETRY,
    IOCTL_LEPTON_GET_REFRESH,
    IOCTL_LEPTON_GET_RESOLUTION,
    IOCTL_LEPTON_RUN_COMMAND,
    IOCTL_LEPTON_SET_ATTRIBUTE,
    IOCTL_LEPTON_GET_ATTRIBUTE,
    IOCTL_LEPTON_GET_FPA_TEMPERATURE,
    IOCTL_LEPTON_GET_AUX_TEMPERATURE,
    IOCTL_LEPTON_SET_MEASUREMENT_MODE,
    IOCTL_LEPTON_GET_MEASUREMENT_MODE,
    IOCTL_LEPTON_SET_MEASUREMENT_RANGE,
    IOCTL_LEPTON_GET_MEASUREMENT_RANGE,
    IOCTL_HIMAX_MD_ENABLE,
    IOCTL_HIMAX_MD_CLEAR,
    IOCTL_HIMAX_MD_WINDOW,
    IOCTL_HIMAX_MD_THRESHOLD,
    IOCTL_HIMAX_OSC_ENABLE,
} ioctl_t;

typedef enum {
    SENSOR_ERROR_NO_ERROR              =  0,
    SENSOR_ERROR_CTL_FAILED            = -1,
    SENSOR_ERROR_CTL_UNSUPPORTED       = -2,
    SENSOR_ERROR_ISC_UNDETECTED        = -3,
    SENSOR_ERROR_ISC_UNSUPPORTED       = -4,
    SENSOR_ERROR_ISC_INIT_FAILED       = -5,
    SENSOR_ERROR_TIM_INIT_FAILED       = -6,
    SENSOR_ERROR_DMA_INIT_FAILED       = -7,
    SENSOR_ERROR_CSI_INIT_FAILED       = -8,
    SENSOR_ERROR_IO_ERROR              = -9,
    SENSOR_ERROR_CAPTURE_FAILED        = -10,
    SENSOR_ERROR_CAPTURE_TIMEOUT       = -11,
    SENSOR_ERROR_INVALID_FRAMESIZE     = -12,
    SENSOR_ERROR_INVALID_PIXFORMAT     = -13,
    SENSOR_ERROR_INVALID_WINDOW        = -14,
    SENSOR_ERROR_INVALID_FRAMERATE     = -15,
    SENSOR_ERROR_INVALID_ARGUMENT      = -16,
    SENSOR_ERROR_PIXFORMAT_UNSUPPORTED = -17,
    SENSOR_ERROR_FRAMEBUFFER_ERROR     = -18,
    SENSOR_ERROR_FRAMEBUFFER_OVERFLOW  = -19,
    SENSOR_ERROR_JPEG_OVERFLOW         = -20,
} sensor_error_t;

typedef enum {
    SENSOR_CONFIG_INIT      = (1 << 0),
    SENSOR_CONFIG_FRAMESIZE = (1 << 1),
    SENSOR_CONFIG_PIXFORMAT = (1 << 2),
    SENSOR_CONFIG_WINDOWING = (1 << 3),
} sensor_config_t;

// Bayer patterns.
// NOTE: These must match the Bayer subformats in imlib.h
//
// BGGR matches the bayer pattern of BGBG...
//                                   GRGR...
//
// GBRG matches the bayer pattern of GBGB...
//                                   RGRG...
//
// GRBG matches the bayer pattern of GRGR...
//                                   BGBG...
//
// RGGB matches the bayer pattern of RGRG...
//                                   GBGB...
//
#define SENSOR_HW_FLAGS_BAYER_BGGR    (SUBFORMAT_ID_BGGR)
#define SENSOR_HW_FLAGS_BAYER_GBRG    (SUBFORMAT_ID_GBRG)
#define SENSOR_HW_FLAGS_BAYER_GRBG    (SUBFORMAT_ID_GRBG)
#define SENSOR_HW_FLAGS_BAYER_RGGB    (SUBFORMAT_ID_RGGB)

// YUV patterns.
// NOTE: These must match the YUV subformats in imlib.h
//
// YUV422 matches the YUV pattern of YUYV... etc. coming out of the sensor.
//
// YVU422 matches the YUV pattern of YVYU... etc. coming out of the sensor.
//
#define SENSOR_HW_FLAGS_YUV422        (SUBFORMAT_ID_YUV422)
#define SENSOR_HW_FLAGS_YVU422        (SUBFORMAT_ID_YVU422)

typedef void (*vsync_cb_t) (uint32_t vsync);
typedef void (*frame_cb_t) ();

typedef struct _sensor sensor_t;
typedef struct _sensor {
    union {
        uint8_t chip_id;        // Sensor ID.
        uint16_t chip_id_w;     // Sensor ID 16 bits.
    };
    uint8_t slv_addr;           // Sensor I2C slave address.

    // Hardware flags (clock polarities, hw capabilities etc..)
    struct {
        uint32_t vsync : 1;       // Vertical sync polarity.
        uint32_t hsync : 1;       // Horizontal sync polarity.
        uint32_t pixck : 1;       // Pixel clock edge.
        uint32_t fsync : 1;       // Hardware frame sync.
        uint32_t jpege : 1;       // Hardware jpeg encoder.
        uint32_t jpeg_mode : 3;   // JPEG mode.
        uint32_t gs_bpp : 2;      // Grayscale bytes per pixel output.
        uint32_t rgb_swap : 1;    // Byte-swap 2BPP RGB formats after capture.
        uint32_t yuv_swap : 1;    // Byte-swap 2BPP YUV formats after capture.
        uint32_t bayer : 3;       // Bayer/CFA pattern.
        uint32_t yuv_order : 1;   // YUV/YVU order.
        uint32_t blc_size : 4;    // Number of black level calibration registers.
    } hw_flags;

    const uint16_t *color_palette;    // Color palette used for color lookup.
    bool disable_delays;        // Set to true to disable all sensor settling time delays.
    bool disable_full_flush;    // Turn off default frame buffer flush policy when full.

    vsync_cb_t vsync_callback;  // VSYNC callback.
    frame_cb_t frame_callback;  // Frame callback.
    polarity_t pwdn_pol;        // PWDN polarity (TODO move to hw_flags)
    polarity_t reset_pol;       // Reset polarity (TODO move to hw_flags)

    // Sensor state
    sde_t sde;                  // Special digital effects
    pixformat_t pixformat;      // Pixel format
    framesize_t framesize;      // Frame size
    int framerate;              // Frame rate
    bool first_line;            // Set to true when the first line of the frame is being read.
    bool drop_frame;            // Set to true to drop the current frame.
    uint32_t last_frame_ms;     // Last sampled frame timestamp in milliseconds.
    bool last_frame_ms_valid;   // Last sampled frame timestamp in milliseconds valid.
    gainceiling_t gainceiling;  // AGC gainceiling
    bool hmirror;               // Horizontal Mirror
    bool vflip;                 // Vertical Flip
    bool transpose;             // Transpose Image
    bool auto_rotation;         // Rotate Image Automatically
    bool detected;              // Set to true when the sensor is initialized.

    omv_i2c_t i2c_bus;          // SCCB/I2C bus.

    // Sensor function pointers
    int (*reset) (sensor_t *sensor);
    int (*sleep) (sensor_t *sensor, int enable);
    int (*read_reg) (sensor_t *sensor, uint16_t reg_addr);
    int (*write_reg) (sensor_t *sensor, uint16_t reg_addr, uint16_t reg_data);
    int (*set_pixformat) (sensor_t *sensor, pixformat_t pixformat);
    int (*set_framesize) (sensor_t *sensor, framesize_t framesize);
    int (*set_framerate) (sensor_t *sensor, int framerate);
    int (*set_contrast) (sensor_t *sensor, int level);
    int (*set_brightness) (sensor_t *sensor, int level);
    int (*set_saturation) (sensor_t *sensor, int level);
    int (*set_gainceiling) (sensor_t *sensor, gainceiling_t gainceiling);
    int (*set_quality) (sensor_t *sensor, int quality);
    int (*set_colorbar) (sensor_t *sensor, int enable);
    int (*set_auto_gain) (sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling);
    int (*get_gain_db) (sensor_t *sensor, float *gain_db);
    int (*set_auto_exposure) (sensor_t *sensor, int enable, int exposure_us);
    int (*get_exposure_us) (sensor_t *sensor, int *exposure_us);
    int (*set_auto_whitebal) (sensor_t *sensor, int enable, float r_gain_db, float g_gain_db, float b_gain_db);
    int (*get_rgb_gain_db) (sensor_t *sensor, float *r_gain_db, float *g_gain_db, float *b_gain_db);
    int (*set_auto_blc) (sensor_t *sensor, int enable, int *regs);
    int (*get_blc_regs) (sensor_t *sensor, int *regs);
    int (*set_hmirror) (sensor_t *sensor, int enable);
    int (*set_vflip) (sensor_t *sensor, int enable);
    int (*set_special_effect) (sensor_t *sensor, sde_t sde);
    int (*set_lens_correction) (sensor_t *sensor, int enable, int radi, int coef);
    int (*ioctl) (sensor_t *sensor, int request, va_list ap);
    int (*snapshot) (sensor_t *sensor, image_t *image, uint32_t flags);
} sensor_t;

extern sensor_t sensor;

// Resolution table
extern const int resolution[][2];

// Initialize the sensor state.
void sensor_init0();

// Initialize the sensor hardware and probe the image sensor.
int sensor_init();

// Detect and initialize the image sensor.
int sensor_probe_init(uint32_t bus_id, uint32_t bus_speed);

// This function is called after a setting that may require reconfiguring
// the hardware changes, such as window size, frame size, or pixel format.
int sensor_config(sensor_config_t config);

// Abort frame capture and disable IRQs, DMA etc..
int sensor_abort(bool fifo_flush, bool in_irq);

// Reset the sensor to its default state.
int sensor_reset();

// Return sensor PID.
int sensor_get_id();

// Returns the xclk freq in hz.
uint32_t sensor_get_xclk_frequency();

// Returns the xclk freq in hz.
int sensor_set_xclk_frequency(uint32_t frequency);

// Return true if the sensor was detected and initialized.
bool sensor_is_detected();

// Sleep mode.
int sensor_sleep(int enable);

// Shutdown mode.
int sensor_shutdown(int enable);

// Read a sensor register.
int sensor_read_reg(uint16_t reg_addr);

// Write a sensor register.
int sensor_write_reg(uint16_t reg_addr, uint16_t reg_data);

// Set the sensor pixel format.
int sensor_set_pixformat(pixformat_t pixformat);

// Set the sensor frame size.
int sensor_set_framesize(framesize_t framesize);

// Set the sensor frame rate.
int sensor_set_framerate(int framerate);

// Return the number of bytes per pixel to read from the image sensor.
uint32_t sensor_get_src_bpp();

// Return the number of bytes per pixel to write to memory.
uint32_t sensor_get_dst_bpp();

// Returns true if a crop is being applied to the frame buffer.
bool sensor_get_cropped();

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

// Enable auto blc (black level calibration) or set from previous calibration.
int sensor_set_auto_blc(int enable, int *regs);

// Get black level valibration register values.
int sensor_get_blc_regs(int *regs);

// Enable/disable the hmirror mode.
int sensor_set_hmirror(int enable);

// Get hmirror status.
bool sensor_get_hmirror();

// Enable/disable the vflip mode.
int sensor_set_vflip(int enable);

// Get vflip status.
bool sensor_get_vflip();

// Enable/disable the transpose mode.
int sensor_set_transpose(bool enable);

// Get transpose mode state.
bool sensor_get_transpose();

// Enable/disable the auto rotation mode.
int sensor_set_auto_rotation(bool enable);

// Get transpose mode state.
bool sensor_get_auto_rotation();

// Set the number of virtual frame buffers.
int sensor_set_framebuffers(int count);

// Drop the next frame to match the current frame rate.
void sensor_throttle_framerate();

// Set special digital effects (SDE).
int sensor_set_special_effect(sde_t sde);

// Set lens shading correction
int sensor_set_lens_correction(int enable, int radi, int coef);

// IOCTL function
int sensor_ioctl(int request, ...);

// Set vsync callback function.
int sensor_set_vsync_callback(vsync_cb_t vsync_cb);

// Set frame callback function.
int sensor_set_frame_callback(frame_cb_t vsync_cb);

// Set color palette
int sensor_set_color_palette(const uint16_t *color_palette);

// Get color palette
const uint16_t *sensor_get_color_palette();

// Return true if the current frame size/format fits in RAM.
int sensor_check_framebuffer_size();

// Auto-crop frame buffer until it fits in RAM (may switch pixel format to BAYER).
int sensor_auto_crop_framebuffer();

// Copy a single line buffer to its destination. The copying process is
// DMA-accelerated, if available, and falls back to slow software if not.
int sensor_copy_line(void *dma, uint8_t *src, uint8_t *dst);

// Default snapshot function.
int sensor_snapshot(sensor_t *sensor, image_t *image, uint32_t flags);

// Convert sensor error codes to strings.
const char *sensor_strerror(int error);
#endif /* __SENSOR_H__ */
