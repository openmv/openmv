/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * CMOS sensor interface abstraction layer.
 */
#ifndef __OMV_CSI_H__
#define __OMV_CSI_H__
#include <stdarg.h>
#include "omv_i2c.h"
#include "imlib.h"

#define OV2640_SLV_ADDR         (0x60)
#define OV5640_SLV_ADDR         (0x78)
#define OV7725_SLV_ADDR         (0x42)
#define MT9V0XX_SLV_ADDR        (0xB8)
#define MT9M114_SLV_ADDR        (0x90)
#define BOSON_SLV_ADDR          (0xAE)
#define LEPTON_SLV_ADDR         (0x54)
#define HM0XX0_SLV_ADDR         (0x48)
#define GC2145_SLV_ADDR         (0x78)
#define GENX320_SLV_ADDR        (0x78)
#define FROGEYE2020_SLV_ADDR    (0x6E)
#define PAG7920_SLV_ADDR        (0x80)
#define PAG7936_SLV_ADDR        (0x80)

// Chip ID Registers
#define OV5640_CHIP_ID          (0x300A)
#define OV_CHIP_ID              (0x0A)
#define ON_CHIP_ID              (0x00)
#define HIMAX_CHIP_ID           (0x0001)
#define GC_CHIP_ID              (0xF0)
#define GENX320_CHIP_ID         (0x0014)
#define PIXART_CHIP_ID          (0x0000)

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
#define BOSON_ID                (0xAE)
#define BOSON_320_ID            (0xAE32)
#define BOSON_640_ID            (0xAE64)
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
#define GENX320_ID_ES           (0x30501C01)
#define GENX320_ID_MP           (0xB0602003)
#define PAG7920_ID              (0x7920)
#define PAG7936_ID              (0x7936)
#define PAJ6100_ID              (0x6100)
#define FROGEYE2020_ID          (0x2020)

#define OMV_CSI_TIMEOUT_MS      (3000)

typedef enum {
    OMV_CSI_ACTIVE_LOW  = 0,
    OMV_CSI_ACTIVE_HIGH = 1
} omv_csi_polarity_t;

typedef enum {
    OMV_CSI_CLK_SOURCE_MCO = 0U,
    OMV_CSI_CLK_SOURCE_TIM = 1U,
    OMV_CSI_CLK_SOURCE_OSC = 2U,
} omv_csi_clk_source_t;

typedef enum {
    OMV_CSI_CONFIG_INIT      = (1 << 0),
    OMV_CSI_CONFIG_FRAMESIZE = (1 << 1),
    OMV_CSI_CONFIG_PIXFORMAT = (1 << 2),
    OMV_CSI_CONFIG_WINDOWING = (1 << 3),
} omv_csi_config_t;

typedef enum {
    OMV_CSI_SDE_NORMAL,
    OMV_CSI_SDE_NEGATIVE,
} omv_csi_sde_t;

typedef enum {
    OMV_CSI_ATTR_CONTRAST=0,
    OMV_CSI_ATTR_BRIGHTNESS,
    OMV_CSI_ATTR_SATURATION,
    OMV_CSI_ATTR_GAINCEILING,
} omv_csi_attr_t;

typedef enum {
    OMV_CSI_GAINCEILING_2X,
    OMV_CSI_GAINCEILING_4X,
    OMV_CSI_GAINCEILING_8X,
    OMV_CSI_GAINCEILING_16X,
    OMV_CSI_GAINCEILING_32X,
    OMV_CSI_GAINCEILING_64X,
    OMV_CSI_GAINCEILING_128X,
} omv_csi_gainceiling_t;

typedef enum {
    OMV_CSI_FRAMESIZE_INVALID = 0,
    // C/SIF Resolutions
    OMV_CSI_FRAMESIZE_QQCIF,    // 88x72
    OMV_CSI_FRAMESIZE_QCIF,     // 176x144
    OMV_CSI_FRAMESIZE_CIF,      // 352x288
    OMV_CSI_FRAMESIZE_QQSIF,    // 88x60
    OMV_CSI_FRAMESIZE_QSIF,     // 176x120
    OMV_CSI_FRAMESIZE_SIF,      // 352x240
    // VGA Resolutions
    OMV_CSI_FRAMESIZE_QQQQVGA,  // 40x30
    OMV_CSI_FRAMESIZE_QQQVGA,   // 80x60
    OMV_CSI_FRAMESIZE_QQVGA,    // 160x120
    OMV_CSI_FRAMESIZE_QVGA,     // 320x240
    OMV_CSI_FRAMESIZE_VGA,      // 640x480
    OMV_CSI_FRAMESIZE_HQQQQVGA, // 30x20
    OMV_CSI_FRAMESIZE_HQQQVGA,  // 60x40
    OMV_CSI_FRAMESIZE_HQQVGA,   // 120x80
    OMV_CSI_FRAMESIZE_HQVGA,    // 240x160
    OMV_CSI_FRAMESIZE_HVGA,     // 480x320
    // FFT Resolutions
    OMV_CSI_FRAMESIZE_64X32,    // 64x32
    OMV_CSI_FRAMESIZE_64X64,    // 64x64
    OMV_CSI_FRAMESIZE_128X64,   // 128x64
    OMV_CSI_FRAMESIZE_128X128,  // 128x128
    // Himax Resolutions
    OMV_CSI_FRAMESIZE_160X160,  // 160x160
    OMV_CSI_FRAMESIZE_320X320,  // 320x320
    // Other
    OMV_CSI_FRAMESIZE_LCD,      // 128x160
    OMV_CSI_FRAMESIZE_QQVGA2,   // 128x160
    OMV_CSI_FRAMESIZE_WVGA,     // 720x480
    OMV_CSI_FRAMESIZE_WVGA2,    // 752x480
    OMV_CSI_FRAMESIZE_SVGA,     // 800x600
    OMV_CSI_FRAMESIZE_XGA,      // 1024x768
    OMV_CSI_FRAMESIZE_WXGA,     // 1280x768
    OMV_CSI_FRAMESIZE_SXGA,     // 1280x1024
    OMV_CSI_FRAMESIZE_SXGAM,    // 1280x960
    OMV_CSI_FRAMESIZE_UXGA,     // 1600x1200
    OMV_CSI_FRAMESIZE_HD,       // 1280x720
    OMV_CSI_FRAMESIZE_FHD,      // 1920x1080
    OMV_CSI_FRAMESIZE_QHD,      // 2560x1440
    OMV_CSI_FRAMESIZE_QXGA,     // 2048x1536
    OMV_CSI_FRAMESIZE_WQXGA,    // 2560x1600
    OMV_CSI_FRAMESIZE_WQXGA2,   // 2592x1944
} omv_csi_framesize_t;

typedef enum {
    OMV_CSI_IOCTL_FLAGS_ABORT = (1 << 8),
} omv_csi_ioctl_flags_t;

typedef enum {
    OMV_CSI_IOCTL_SET_READOUT_WINDOW    = 0x00 | OMV_CSI_IOCTL_FLAGS_ABORT,
    OMV_CSI_IOCTL_GET_READOUT_WINDOW    = 0x01,
    OMV_CSI_IOCTL_SET_TRIGGERED_MODE    = 0x02,
    OMV_CSI_IOCTL_GET_TRIGGERED_MODE    = 0x03,
    OMV_CSI_IOCTL_SET_FOV_WIDE          = 0x04,
    OMV_CSI_IOCTL_GET_FOV_WIDE          = 0x05,
    OMV_CSI_IOCTL_TRIGGER_AUTO_FOCUS    = 0x06,
    OMV_CSI_IOCTL_PAUSE_AUTO_FOCUS      = 0x07,
    OMV_CSI_IOCTL_RESET_AUTO_FOCUS      = 0x08,
    OMV_CSI_IOCTL_WAIT_ON_AUTO_FOCUS    = 0x09,
    OMV_CSI_IOCTL_SET_NIGHT_MODE        = 0x0A,
    OMV_CSI_IOCTL_GET_NIGHT_MODE        = 0x0B,
    OMV_CSI_IOCTL_LEPTON_GET_WIDTH      = 0x0C,
    OMV_CSI_IOCTL_LEPTON_GET_HEIGHT     = 0x0D,
    OMV_CSI_IOCTL_LEPTON_GET_RADIOMETRY = 0x0E,
    OMV_CSI_IOCTL_LEPTON_GET_REFRESH    = 0x0F,
    OMV_CSI_IOCTL_LEPTON_GET_RESOLUTION = 0x10,
    OMV_CSI_IOCTL_LEPTON_RUN_COMMAND    = 0x11,
    OMV_CSI_IOCTL_LEPTON_SET_ATTRIBUTE  = 0x12,
    OMV_CSI_IOCTL_LEPTON_GET_ATTRIBUTE  = 0x13,
    OMV_CSI_IOCTL_LEPTON_GET_FPA_TEMP   = 0x14,
    OMV_CSI_IOCTL_LEPTON_GET_AUX_TEMP   = 0x15,
    OMV_CSI_IOCTL_LEPTON_SET_MODE       = 0x16 | OMV_CSI_IOCTL_FLAGS_ABORT,
    OMV_CSI_IOCTL_LEPTON_GET_MODE       = 0x17,
    OMV_CSI_IOCTL_LEPTON_SET_RANGE      = 0x18 | OMV_CSI_IOCTL_FLAGS_ABORT,
    OMV_CSI_IOCTL_LEPTON_GET_RANGE      = 0x19,
    OMV_CSI_IOCTL_HIMAX_MD_ENABLE       = 0x1A,
    OMV_CSI_IOCTL_HIMAX_MD_CLEAR        = 0x1B,
    OMV_CSI_IOCTL_HIMAX_MD_WINDOW       = 0x1C | OMV_CSI_IOCTL_FLAGS_ABORT,
    OMV_CSI_IOCTL_HIMAX_MD_THRESHOLD    = 0x1D,
    OMV_CSI_IOCTL_HIMAX_OSC_ENABLE      = 0x1E | OMV_CSI_IOCTL_FLAGS_ABORT,
    OMV_CSI_IOCTL_GET_RGB_STATS         = 0x1F,
    OMV_CSI_IOCTL_GENX320_SET_BIASES    = 0x20,
    OMV_CSI_IOCTL_GENX320_SET_BIAS      = 0x21,
    OMV_CSI_IOCTL_GENX320_SET_AFK       = 0x22
} omv_csi_ioctl_t;

typedef enum {
    OMV_CSI_ERROR_NO_ERROR              =  0,
    OMV_CSI_ERROR_CTL_FAILED            = -1,
    OMV_CSI_ERROR_CTL_UNSUPPORTED       = -2,
    OMV_CSI_ERROR_ISC_UNDETECTED        = -3,
    OMV_CSI_ERROR_ISC_UNSUPPORTED       = -4,
    OMV_CSI_ERROR_ISC_INIT_FAILED       = -5,
    OMV_CSI_ERROR_TIM_INIT_FAILED       = -6,
    OMV_CSI_ERROR_DMA_INIT_FAILED       = -7,
    OMV_CSI_ERROR_CSI_INIT_FAILED       = -8,
    OMV_CSI_ERROR_IO_ERROR              = -9,
    OMV_CSI_ERROR_CAPTURE_FAILED        = -10,
    OMV_CSI_ERROR_CAPTURE_TIMEOUT       = -11,
    OMV_CSI_ERROR_INVALID_FRAMESIZE     = -12,
    OMV_CSI_ERROR_INVALID_PIXFORMAT     = -13,
    OMV_CSI_ERROR_INVALID_WINDOW        = -14,
    OMV_CSI_ERROR_INVALID_FRAMERATE     = -15,
    OMV_CSI_ERROR_INVALID_ARGUMENT      = -16,
    OMV_CSI_ERROR_PIXFORMAT_UNSUPPORTED = -17,
    OMV_CSI_ERROR_FRAMEBUFFER_ERROR     = -18,
    OMV_CSI_ERROR_FRAMEBUFFER_OVERFLOW  = -19,
    OMV_CSI_ERROR_JPEG_OVERFLOW         = -20,
} omv_csi_error_t;

#if (OMV_GENX320_ENABLE == 1)
typedef enum {
    OMV_CSI_GENX320_BIASES_DEFAULT,
    OMV_CSI_GENX320_BIASES_LOW_LIGHT,
    OMV_CSI_GENX320_BIASES_ACTIVE_MARKER,
    OMV_CSI_GENX320_BIASES_LOW_NOISE,
    OMV_CSI_GENX320_BIASES_HIGH_SPEED
} omv_csi_genx320_biases_preset_t;

typedef enum {
    OMV_CSI_GENX320_BIAS_DIFF_OFF,
    OMV_CSI_GENX320_BIAS_DIFF_ON,
    OMV_CSI_GENX320_BIAS_FO,
    OMV_CSI_GENX320_BIAS_HPF,
    OMV_CSI_GENX320_BIAS_REFR
} omv_csi_genx320_bias_t;
#endif

typedef void (*vsync_cb_t) (uint32_t vsync);
typedef void (*frame_cb_t) ();
typedef struct _omv_csi omv_csi_t;

typedef struct _omv_csi {
    uint32_t chip_id;           // Sensor ID 32 bits.
    uint8_t slv_addr;           // Sensor I2C slave address.

    // Hardware flags (clock polarities, hw capabilities etc..)
    struct {
        uint32_t reset_pol  : 1;  // Reset polarity.
        uint32_t power_pol  : 1;  // Power-down polarity.
        uint32_t vsync_pol  : 1;  // Vertical sync polarity.
        uint32_t hsync_pol  : 1;  // Horizontal sync polarity.
        uint32_t pixck_pol  : 1;  // Pixel clock edge.
        uint32_t frame_sync : 1;  // Hardware frame sync.
        uint32_t mono_bpp   : 2;  // Grayscale bytes per pixel output.
        uint32_t rgb_swap   : 1;  // Byte-swap 2BPP RGB formats after capture.
        uint32_t yuv_swap   : 1;  // Byte-swap 2BPP YUV formats after capture.
        uint32_t blc_size   : 4;  // Number of black level calibration registers.
        uint32_t raw_output : 1;  // The sensor supports raw output only.
        uint32_t yuv_format : 1;  // YUV/YVU output format.
        uint32_t jpg_format : 3;  // JPEG output format/mode.
        uint32_t cfa_format : 3;  // CFA format/pattern.
        uint32_t mipi_if    : 1;  // CSI-2 interface.
        uint32_t mipi_brate : 12; // CSI-2 interface bitrate.
    };

    const uint16_t *color_palette;    // Color palette used for color lookup.
    bool disable_delays;        // Set to true to disable all sensor settling time delays.
    bool disable_full_flush;    // Turn off default frame buffer flush policy when full.

    vsync_cb_t vsync_callback;  // VSYNC callback.
    frame_cb_t frame_callback;  // Frame callback.

    // Sensor state
    omv_csi_sde_t sde;          // Special digital effects
    pixformat_t pixformat;      // Pixel format
    omv_csi_framesize_t framesize;  // Frame size
    int framerate;              // Frame rate
    bool first_line;            // Set to true when the first line of the frame is being read.
    bool drop_frame;            // Set to true to drop the current frame.
    uint32_t last_frame_ms;     // Last sampled frame timestamp in milliseconds.
    bool last_frame_ms_valid;   // Last sampled frame timestamp in milliseconds valid.
    omv_csi_gainceiling_t gainceiling;  // AGC gainceiling
    bool hmirror;               // Horizontal Mirror
    bool vflip;                 // Vertical Flip
    bool transpose;             // Transpose Image
    bool auto_rotation;         // Rotate Image Automatically
    bool detected;              // Set to true when the sensor is initialized.

    omv_i2c_t i2c_bus;          // SCCB/I2C bus.

    #ifdef OMV_CSI_PORT_BITS
    // Additional port-specific members like device base pointer,
    // dma handles, more I/Os etc... are included directly here,
    // so that they can be accessible from this struct.
    OMV_CSI_PORT_BITS
    #endif

    // Sensor function pointers
    int (*reset) (omv_csi_t *csi);
    int (*sleep) (omv_csi_t *csi, int enable);
    int (*read_reg) (omv_csi_t *csi, uint16_t reg_addr);
    int (*write_reg) (omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data);
    int (*set_pixformat) (omv_csi_t *csi, pixformat_t pixformat);
    int (*set_framesize) (omv_csi_t *csi, omv_csi_framesize_t framesize);
    int (*set_framerate) (omv_csi_t *csi, int framerate);
    int (*set_contrast) (omv_csi_t *csi, int level);
    int (*set_brightness) (omv_csi_t *csi, int level);
    int (*set_saturation) (omv_csi_t *csi, int level);
    int (*set_gainceiling) (omv_csi_t *csi, omv_csi_gainceiling_t gainceiling);
    int (*set_quality) (omv_csi_t *csi, int quality);
    int (*set_colorbar) (omv_csi_t *csi, int enable);
    int (*set_auto_gain) (omv_csi_t *csi, int enable, float gain_db, float gain_db_ceiling);
    int (*get_gain_db) (omv_csi_t *csi, float *gain_db);
    int (*set_auto_exposure) (omv_csi_t *csi, int enable, int exposure_us);
    int (*get_exposure_us) (omv_csi_t *csi, int *exposure_us);
    int (*set_auto_whitebal) (omv_csi_t *csi, int enable, float r_gain_db, float g_gain_db, float b_gain_db);
    int (*get_rgb_gain_db) (omv_csi_t *csi, float *r_gain_db, float *g_gain_db, float *b_gain_db);
    int (*set_auto_blc) (omv_csi_t *csi, int enable, int *regs);
    int (*get_blc_regs) (omv_csi_t *csi, int *regs);
    int (*set_hmirror) (omv_csi_t *csi, int enable);
    int (*set_vflip) (omv_csi_t *csi, int enable);
    int (*set_special_effect) (omv_csi_t *csi, omv_csi_sde_t sde);
    int (*set_lens_correction) (omv_csi_t *csi, int enable, int radi, int coef);
    int (*ioctl) (omv_csi_t *csi, int request, va_list ap);
    int (*snapshot) (omv_csi_t *csi, image_t *image, uint32_t flags);
} omv_csi_t;

extern omv_csi_t csi;

// Resolution table
extern uint16_t resolution[][2];

// Initialize the sensor state.
void omv_csi_init0();

// Initialize the sensor and probe the image sensor.
int omv_csi_init();

// Detect and initialize the image sensor.
int omv_csi_probe_init(uint32_t bus_id, uint32_t bus_speed);

// This function is called after a setting that may require reconfiguring
// the hardware changes, such as window size, frame size, or pixel format.
int omv_csi_config(omv_csi_config_t config);

// Abort frame capture and disable IRQs, DMA etc..
int omv_csi_abort(bool fifo_flush, bool in_irq);

// Reset the sensor to its default state.
int omv_csi_reset();

// Return csi PID.
int omv_csi_get_id();

// Returns the xclk freq in hz.
uint32_t omv_csi_get_xclk_frequency();

// Returns the xclk freq in hz.
int omv_csi_set_clk_frequency(uint32_t frequency);

// Return true if the sensor was detected and initialized.
bool omv_csi_is_detected();

// Sleep mode.
int omv_csi_sleep(int enable);

// Shutdown mode.
int omv_csi_shutdown(int enable);

// Read a csi register.
int omv_csi_read_reg(uint16_t reg_addr);

// Write a csi register.
int omv_csi_write_reg(uint16_t reg_addr, uint16_t reg_data);

// Set the sensor pixel format.
int omv_csi_set_pixformat(pixformat_t pixformat);

// Set the sensor frame size.
int omv_csi_set_framesize(omv_csi_framesize_t framesize);

// Set the sensor frame rate.
int omv_csi_set_framerate(int framerate);

// Return the number of bytes per pixel to read from the image sensor.
uint32_t omv_csi_get_src_bpp();

// Return the number of bytes per pixel to write to memory.
uint32_t omv_csi_get_dst_bpp();

// Returns true if a crop is being applied to the frame buffer.
bool omv_csi_get_cropped();

// Set window size.
int omv_csi_set_windowing(int x, int y, int w, int h);

// Set the sensor contrast level (from -3 to +3).
int omv_csi_set_contrast(int level);

// Set the sensor brightness level (from -3 to +3).
int omv_csi_set_brightness(int level);

// Set the sensor saturation level (from -3 to +3).
int omv_csi_set_saturation(int level);

// Set the sensor AGC gain ceiling.
// Note: This function has no effect when AGC (Automatic Gain Control) is disabled.
int omv_csi_set_gainceiling(omv_csi_gainceiling_t gainceiling);

// Set the quantization scale factor, controls JPEG quality (quality 0-255).
int omv_csi_set_quality(int qs);

// Enable/disable the colorbar mode.
int omv_csi_set_colorbar(int enable);

// Enable auto gain or set value manually.
int omv_csi_set_auto_gain(int enable, float gain_db, float gain_db_ceiling);

// Get the gain value.
int omv_csi_get_gain_db(float *gain_db);

// Enable auto exposure or set value manually.
int omv_csi_set_auto_exposure(int enable, int exposure_us);

// Get the exposure value.
int omv_csi_get_exposure_us(int *get_exposure_us);

// Enable auto white balance or set value manually.
int omv_csi_set_auto_whitebal(int enable, float r_gain_db, float g_gain_db, float b_gain_db);

// Get the rgb gain values.
int omv_csi_get_rgb_gain_db(float *r_gain_db, float *g_gain_db, float *b_gain_db);

// Enable auto blc (black level calibration) or set from previous calibration.
int omv_csi_set_auto_blc(int enable, int *regs);

// Get black level valibration register values.
int omv_csi_get_blc_regs(int *regs);

// Enable/disable the hmirror mode.
int omv_csi_set_hmirror(int enable);

// Get hmirror status.
bool omv_csi_get_hmirror();

// Enable/disable the vflip mode.
int omv_csi_set_vflip(int enable);

// Get vflip status.
bool omv_csi_get_vflip();

// Enable/disable the transpose mode.
int omv_csi_set_transpose(bool enable);

// Get transpose mode state.
bool omv_csi_get_transpose();

// Enable/disable the auto rotation mode.
int omv_csi_set_auto_rotation(bool enable);

// Get transpose mode state.
bool omv_csi_get_auto_rotation();

// Set the number of virtual frame buffers.
int omv_csi_set_framebuffers(int count);

// Drop the next frame to match the current frame rate.
void omv_csi_throttle_framerate();

// Set special digital effects (SDE).
int omv_csi_set_special_effect(omv_csi_sde_t sde);

// Set lens shading correction
int omv_csi_set_lens_correction(int enable, int radi, int coef);

// IOCTL function
int omv_csi_ioctl(int request, ...);

// Set vsync callback function.
int omv_csi_set_vsync_callback(vsync_cb_t vsync_cb);

// Set frame callback function.
int omv_csi_set_frame_callback(frame_cb_t vsync_cb);

// Set color palette
int omv_csi_set_color_palette(const uint16_t *color_palette);

// Get color palette
const uint16_t *omv_csi_get_color_palette();

// Return true if the current frame size/format fits in RAM.
int omv_csi_check_framebuffer_size();

// Auto-crop frame buffer until it fits in RAM (may switch pixel format to BAYER).
int omv_csi_auto_crop_framebuffer();

// Copy a single line buffer to its destination. The copying process is
// DMA-accelerated, if available, and falls back to slow software if not.
int omv_csi_copy_line(void *dma, uint8_t *src, uint8_t *dst);

// Default snapshot function.
int omv_csi_snapshot(omv_csi_t *csi, image_t *image, uint32_t flags);

// Convert csi error codes to strings.
const char *omv_csi_strerror(int error);
#endif // __OMV_CSI_H__
