/*
 * camera_vd66gy.h -- ST VD66GY (colour) / VD56G3 (mono) global-shutter sensor, bare metal.
 *
 * Boot (firmware and VT patches, PLL) is done by ST's BSD-3 vd6g driver; this file adds
 * the MIPI CSI-2 RAW10 output, a centred ROI with optional digital binning, frame length,
 * manual exposure / analog gain and the sensor GPIO (strobe) modes.
 */
#ifndef CAMERA_VD66GY_H
#define CAMERA_VD66GY_H

#include <stdint.h>
#include <stdbool.h>

#define CAM_CSI_MBPS    804     // per lane, 2 lanes

typedef struct {
    uint16_t width, height;     // output frame (even; width a multiple of 32 for the detector)
    uint8_t  bin;               // 1, 2 or 4 (0 = 2 when it fits, else 1)
    bool     hmirror, vflip;
    bool     colorbar;          // sensor test pattern
    uint16_t vblank;            // minimum vertical blanking in lines (0 = 110)
    uint32_t frame_us;          // frame time (clamped to the sensor minimum)
    uint32_t expo_us;           // manual exposure (min 21 lines ~ 160 us)
    float    gain_db;           // manual analog gain, 0..18 dB (1x..8x)
    int8_t   strobe_gpio;       // sensor GPIO used as strobe output, -1 = none
    bool     strobe_inv;        // strobe active low
} cam_cfg_t;

typedef struct {
    uint16_t model;             // 0x5603 = VD66GY / VD56G3
    bool     mono;
    uint8_t  cfa;               // cz_cfa_t of pixel (0,0): 0 BGGR 1 GBRG 2 GRBG 3 RGGB
    uint16_t line_len;          // pixel clocks per line
    uint16_t frame_len;         // lines per frame
    uint16_t rows;              // sensor rows read (height * bin)
    uint8_t  bin;
    uint32_t line_ns;
    uint32_t min_frame_us;      // shortest frame for this ROI / binning / vblank
    uint32_t frame_us;          // frame time in use
    uint32_t expo_us;           // exposure in use (after rounding / clamping)
    uint8_t  again;             // analog gain code in use
} cam_info_t;

void cam_default_cfg(cam_cfg_t *c);
int  cam_init(void);                         // power up, reset, boot, identify (not streaming)
int  cam_configure(const cam_cfg_t *c);      // full setup in standby, then start streaming
int  cam_stop(void);
int  cam_start(void);
int  cam_set_frame_us(uint32_t us);          // while streaming
int  cam_set_exposure_us(uint32_t us);
int  cam_set_gain_db(float db);
int  cam_set_vblank(uint16_t lines);
void cam_get_info(cam_info_t *info);
bool cam_streaming(void);
int  cam_read_reg8(uint16_t reg, uint8_t *v);
int  cam_write_reg8(uint16_t reg, uint8_t v);

#endif
