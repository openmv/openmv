/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * camera_vd66gy.c -- ST VD66GY / VD56G3 global-shutter sensor, bare-metal STM32 HAL version
 * (same register logic as drivers/sensors/vd66gy.c of the OpenMV build).
 *
 * Boot (firmware patch, VT patch, PLL setup) is done by ST's BSD-licensed VD6G driver
 * (Drivers/vd6g, from STMicroelectronics/stm32-mw-camera, unmodified). This file adds the
 * MIPI CSI-2 output, a centred ROI with optional digital binning, frame length, manual
 * exposure and analog gain, orientation, test pattern and the sensor GPIO (strobe) modes.
 *
 * Register widths and little-endian byte order follow ST's driver. The analog gain coding
 * (gain = 32 / (32 - code)) and the default minimum vertical blanking (110 lines) follow
 * the Linux vd56g3 driver; the blanking can be lowered at run time ("set vblank").
 */
#include <string.h>
#include <math.h>
#include "main.h"
#include "camera_vd66gy.h"
#include "vd6g.h"

// Registers (see Drivers/vd6g/vd6g.c).
#define REG_MODEL_ID                (0x0000)
#define REG_SYSTEM_FSM              (0x0028)
#define FSM_SW_STBY                 (0x02)
#define FSM_STREAMING               (0x03)
#define REG_STBY                    (0x0201)
#define REG_STREAMING               (0x0202)
#define CMD_ACK                     (0x00)
#define CMD_START_STREAM            (0x01)
#define CMD_STOP_STREAM             (0x01)
#define REG_LINE_LENGTH             (0x0300)
#define REG_ORIENTATION             (0x0302)
#define REG_FORMAT_CTRL             (0x030a)
#define REG_OIF_CTRL                (0x030c)
#define REG_OIF_IMG_CTRL            (0x030f)
#define RAW10_DATA_TYPE             (0x2b)
#define REG_OIF_CSI_BITRATE         (0x0312)
#define REG_DUSTER_CTRL             (0x0318)
#define REG_DARKCAL_CTRL            (0x0340)
#define REG_PATGEN_CTRL             (0x0400)
#define PATGEN_VER_COLOR_BAR        (0x0021)
#define REG_AE_COMPILER_CONTROL     (0x0430)
#define REG_EXP_MODE                (0x044c)
#define EXP_MANUAL                  (2)
#define REG_MANUAL_ANALOG_GAIN      (0x044d)
#define REG_MANUAL_COARSE_EXP       (0x044e)
#define REG_MANUAL_DIGITAL_GAIN(c)  (0x0450 + 2 * (c))
#define REG_FRAME_LENGTH            (0x0458)
#define REG_OUT_ROI_X_START         (0x045e)
#define REG_OUT_ROI_X_END           (0x0460)
#define REG_OUT_ROI_Y_START         (0x0462)
#define REG_OUT_ROI_Y_END           (0x0464)
#define REG_GPIO_CTRL(i)            (0x0467 + (i))
#define REG_READOUT_CTRL            (0x047e)
#define REG_EXP_COARSE_INTG_MARGIN  (0x0946)
#define REG_MAX_AG_CODED            (0x0960)
#define REG_MIN_AG_CODED            (0x097E)

#define PIXEL_CLOCK                 (160800000U)
#define ARRAY_X0                    (2)     // first active column (ST full-resolution mode)
#define MIN_EXP_LINES               (21)
#define EXP_OFFSET                  (7)
#define DEF_VBLANK                  (110)
#define GAIN_SCALE                  (32.0f)
#define I2C_CHUNK                   (256)

static I2C_HandleTypeDef s_hi2c;

typedef struct {
    VD6G_Ctx_t st;              // ST driver context (boot only)
    bool booted;
    bool streaming;
    bool mono;
    uint8_t ag_min, ag_max;
    uint8_t again;
    uint8_t bin;
    uint8_t gpio[VD6G_GPIO_NB];
    uint16_t model;
    uint16_t line_len;
    uint16_t vblank;
    uint16_t rows;
    uint16_t frame_len;
    uint32_t frame_us;
    uint32_t expo_us;
    cam_cfg_t cfg;
} cam_state_t;

static cam_state_t s;

// ---------------------------------------------------------------------------------------
// I2C: 16-bit big-endian register address, little-endian data.
// ---------------------------------------------------------------------------------------
static int i2c_init(void) {
    RCC_PeriphCLKInitTypeDef clk = { 0 };
    clk.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
    clk.I2c2ClockSelection = RCC_I2C2CLKSOURCE_HSI;
    if (HAL_RCCEx_PeriphCLKConfig(&clk) != HAL_OK) {
        return -1;
    }
    s_hi2c.Instance = CAM_I2C;
    s_hi2c.Init.Timing = CAM_I2C_TIMING;
    s_hi2c.Init.OwnAddress1 = 0;
    s_hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    s_hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    s_hi2c.Init.OwnAddress2 = 0;
    s_hi2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    s_hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    s_hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&s_hi2c) != HAL_OK) {
        return -1;
    }
    HAL_I2CEx_ConfigAnalogFilter(&s_hi2c, I2C_ANALOGFILTER_ENABLE);
    return 0;
}

static int vd_read(uint16_t reg, uint8_t *buf, uint16_t len) {
    return HAL_I2C_Mem_Read(&s_hi2c, CAM_I2C_ADDR, reg, I2C_MEMADD_SIZE_16BIT, buf, len, 20) == HAL_OK ? 0 : -1;
}

static int vd_write(uint16_t reg, const uint8_t *buf, uint32_t len) {
    while (len) {
        uint16_t n = len > I2C_CHUNK ? I2C_CHUNK : (uint16_t) len;
        if (HAL_I2C_Mem_Write(&s_hi2c, CAM_I2C_ADDR, reg, I2C_MEMADD_SIZE_16BIT,
                              (uint8_t *) buf, n, 50) != HAL_OK) {
            return -1;
        }
        reg += n;
        buf += n;
        len -= n;
    }
    return 0;
}

static int rd8(uint16_t reg, uint8_t *v) {
    return vd_read(reg, v, 1);
}

static int rd16(uint16_t reg, uint16_t *v) {
    uint8_t b[2] = { 0 };
    int ret = vd_read(reg, b, 2);
    *v = (uint16_t) (b[0] | (b[1] << 8));
    return ret;
}

static int wr8(uint16_t reg, uint8_t v) {
    return vd_write(reg, &v, 1);
}

static int wr16(uint16_t reg, uint16_t v) {
    uint8_t b[2] = { (uint8_t) (v & 0xFF), (uint8_t) (v >> 8) };
    return vd_write(reg, b, 2);
}

int cam_read_reg8(uint16_t reg, uint8_t *v) {
    return rd8(reg, v);
}

int cam_write_reg8(uint16_t reg, uint8_t v) {
    return wr8(reg, v);
}

// ---------------------------------------------------------------------------------------
// ST driver callbacks.
// ---------------------------------------------------------------------------------------
static void st_shutdown_pin(VD6G_Ctx_t *ctx, int value) {
    (void) ctx;
    HAL_GPIO_WritePin(CAM_NRST_PORT, CAM_NRST_PIN, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static int st_read8(VD6G_Ctx_t *ctx, uint16_t addr, uint8_t *value) {
    (void) ctx;
    return rd8(addr, value);
}

static int st_read16(VD6G_Ctx_t *ctx, uint16_t addr, uint16_t *value) {
    (void) ctx;
    return rd16(addr, value);
}

static int st_read32(VD6G_Ctx_t *ctx, uint16_t addr, uint32_t *value) {
    (void) ctx;
    uint8_t b[4] = { 0 };
    int ret = vd_read(addr, b, 4);
    *value = b[0] | (b[1] << 8) | (b[2] << 16) | ((uint32_t) b[3] << 24);
    return ret;
}

static int st_write8(VD6G_Ctx_t *ctx, uint16_t addr, uint8_t value) {
    (void) ctx;
    return wr8(addr, value);
}

static int st_write16(VD6G_Ctx_t *ctx, uint16_t addr, uint16_t value) {
    (void) ctx;
    return wr16(addr, value);
}

static int st_write32(VD6G_Ctx_t *ctx, uint16_t addr, uint32_t value) {
    (void) ctx;
    uint8_t b[4] = { (uint8_t) value, (uint8_t) (value >> 8), (uint8_t) (value >> 16), (uint8_t) (value >> 24) };
    return vd_write(addr, b, 4);
}

static int st_write_array(VD6G_Ctx_t *ctx, uint16_t addr, uint8_t *data, int data_len) {
    (void) ctx;
    return vd_write(addr, data, (uint32_t) data_len);
}

static void st_delay(VD6G_Ctx_t *ctx, uint32_t ms) {
    (void) ctx;
    HAL_Delay(ms);
}

// ---------------------------------------------------------------------------------------
// Helpers.
// ---------------------------------------------------------------------------------------
static int poll8(uint16_t reg, uint8_t val, uint32_t timeout_ms) {
    uint32_t start = HAL_GetTick();
    uint8_t v = 0xFF;
    do {
        if (rd8(reg, &v) != 0) {
            return -1;
        }
        if (v == val) {
            return 0;
        }
        HAL_Delay(1);
    } while ((HAL_GetTick() - start) < timeout_ms);
    return -1;
}

static uint32_t line_time_ns(void) {
    return (uint32_t) (((uint64_t) s.line_len * 1000000000ULL) / PIXEL_CLOCK);
}

static uint16_t min_frame_len(void) {
    uint32_t fl = (uint32_t) s.rows + s.vblank;
    return (fl > 0xFFFF) ? 0xFFFF : (uint16_t) fl;
}

static uint32_t lines_to_us(uint32_t lines) {
    return (uint32_t) (((uint64_t) lines * s.line_len * 1000000ULL) / PIXEL_CLOCK);
}

static uint16_t compute_frame_len(void) {
    uint64_t fl = ((uint64_t) s.frame_us * PIXEL_CLOCK) / (1000000ULL * s.line_len);
    if (fl < min_frame_len()) {
        fl = min_frame_len();
    }
    if (fl > 0xFFFF) {
        fl = 0xFFFF;
    }
    return (uint16_t) fl;
}

static uint16_t compute_expo_lines(void) {
    uint16_t margin = 68;
    rd16(REG_EXP_COARSE_INTG_MARGIN, &margin);
    uint32_t lt = line_time_ns();
    uint32_t lines = lt ? (uint32_t) (((uint64_t) s.expo_us * 1000 + lt - 1) / lt) : MIN_EXP_LINES;
    int32_t max_lines = (int32_t) s.frame_len - margin - EXP_OFFSET;
    if (max_lines < MIN_EXP_LINES) {
        max_lines = MIN_EXP_LINES;
    }
    if (lines < MIN_EXP_LINES) {
        lines = MIN_EXP_LINES;
    }
    if ((int32_t) lines > max_lines) {
        lines = (uint32_t) max_lines;
    }
    return (uint16_t) lines;
}

static int write_exposure(void) {
    int ret = 0;
    ret |= wr8(REG_MANUAL_ANALOG_GAIN, s.again);
    ret |= wr16(REG_MANUAL_COARSE_EXP, compute_expo_lines());
    for (int c = 0; c < 4; c++) {
        ret |= wr16(REG_MANUAL_DIGITAL_GAIN(c), 0x100);     // 1.0x (8.8 fixed point)
    }
    ret |= wr8(REG_EXP_MODE, EXP_MANUAL);
    return ret;
}

static int write_frame_len(void) {
    uint16_t old = s.frame_len;
    s.frame_len = compute_frame_len();
    int ret = 0;
    // The exposure must always fit in the frame: shorten it before a shorter frame,
    // lengthen the frame before a longer exposure.
    if (s.frame_len < old) {
        ret |= wr16(REG_MANUAL_COARSE_EXP, compute_expo_lines());
    }
    ret |= wr16(REG_FRAME_LENGTH, s.frame_len);
    if (s.frame_len >= old) {
        ret |= wr16(REG_MANUAL_COARSE_EXP, compute_expo_lines());
    }
    return ret;
}

static uint8_t gain_code(float db) {
    float gain = expf((db / 20.0f) * 2.302585093f);
    float code = GAIN_SCALE - (GAIN_SCALE / ((gain < 1.0f) ? 1.0f : gain));
    int c = (int) (code + 0.5f);
    return (uint8_t) ((c < s.ag_min) ? s.ag_min : (c > s.ag_max) ? s.ag_max : c);
}

static uint8_t cfa_of(bool hmirror, bool vflip) {
    // cz_cfa_t: 0 BGGR, 1 GBRG, 2 GRBG, 3 RGGB (ST vd6g.c VD6G_SetBayerType).
    if (hmirror && vflip) {
        return 1;
    } else if (hmirror) {
        return 3;
    } else if (vflip) {
        return 0;
    }
    return 2;
}

// ---------------------------------------------------------------------------------------
// Public API.
// ---------------------------------------------------------------------------------------
void cam_default_cfg(cam_cfg_t *c) {
    memset(c, 0, sizeof(*c));
    c->width = 320;
    c->height = 200;
    c->bin = 2;
    c->vblank = DEF_VBLANK;
    c->frame_us = 4166;         // 240 fps
    c->expo_us = 200;
    c->gain_db = 6.0f;
    c->strobe_gpio = -1;
}

int cam_stop(void) {
    uint8_t fsm = 0;
    rd8(REG_SYSTEM_FSM, &fsm);
    if (fsm != FSM_STREAMING) {
        s.streaming = false;
        return 0;
    }
    int ret = wr8(REG_STREAMING, CMD_STOP_STREAM);
    ret |= poll8(REG_STREAMING, CMD_ACK, 500);
    ret |= poll8(REG_SYSTEM_FSM, FSM_SW_STBY, 500);
    s.streaming = false;
    return ret;
}

int cam_start(void) {
    int ret = wr8(REG_STBY, CMD_START_STREAM);
    ret |= poll8(REG_STBY, CMD_ACK, 500);
    ret |= poll8(REG_SYSTEM_FSM, FSM_STREAMING, 500);
    s.streaming = (ret == 0);
    return ret;
}

bool cam_streaming(void) {
    return s.streaming;
}

int cam_init(void) {
    memset(&s, 0, sizeof(s));
    s.line_len = 1236;
    s.vblank = DEF_VBLANK;
    s.rows = VD6G_MAX_HEIGHT;

    // Power-up: module enable high, reset low, then let ST's driver pulse the reset.
    HAL_GPIO_WritePin(CAM_NRST_PORT, CAM_NRST_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CAM_EN_PORT, CAM_EN_PIN, GPIO_PIN_SET);
    HAL_Delay(20);

    if (i2c_init() != 0) {
        return -1;
    }

    VD6G_Config_t config = {
        .ext_clock_freq_in_hz = CAM_EXT_CLK_HZ,
        .resolution = VD6G_RES_QVGA_320_240,
        .frame_rate = 30,
        .flip_mirror_mode = VD6G_MIRROR_FLIP_NONE,
        .line_len = 0,
        .patgen = VD6G_PATGEN_DISABLE,
        .flicker = VD6G_FLICKER_FREE_NONE,
        .pixel_depth = 10,
        .out_itf = {
            .datalane_nb = 2,
            .clock_lane_swap_enable = 1,
            .data_lane0_swap_enable = 1,
            .data_lane1_swap_enable = 1,
            .data_lanes_mapping_swap_enable = 0,
        },
        .exposure_mode = VD6G_EXPOSURE_AUTO,
    };
    for (int i = 0; i < VD6G_GPIO_NB; i++) {
        config.gpio_ctrl[i] = VD6G_GPIO_GPIO_IN;
        s.gpio[i] = VD6G_GPIO_GPIO_IN;
    }
    s.st.shutdown_pin = st_shutdown_pin;
    s.st.read8 = st_read8;
    s.st.read16 = st_read16;
    s.st.read32 = st_read32;
    s.st.write8 = st_write8;
    s.st.write16 = st_write16;
    s.st.write32 = st_write32;
    s.st.write_array = st_write_array;
    s.st.delay = st_delay;
    s.st.log = NULL;

    if (VD6G_Init(&s.st, &config) != 0) {
        return -2;
    }
    rd16(REG_MODEL_ID, &s.model);
    s.mono = (s.st.bayer == VD6G_BAYER_NONE);
    rd16(REG_LINE_LENGTH, &s.line_len);
    rd8(REG_MIN_AG_CODED, &s.ag_min);
    rd8(REG_MAX_AG_CODED, &s.ag_max);
    s.ag_min &= 0x1f;
    s.ag_max &= 0x1f;
    if (s.ag_max == 0 || s.ag_max < s.ag_min) {
        s.ag_max = 28;
    }
    s.again = s.ag_min;
    s.booted = true;
    return 0;
}

int cam_configure(const cam_cfg_t *c) {
    if (!s.booted) {
        return -1;
    }
    uint32_t w = c->width, h = c->height;
    if (!w || !h || (w & 1) || (h & 1)) {
        return -1;
    }
    uint32_t bin = c->bin;
    if (bin == 0) {
        bin = ((2 * w) <= VD6G_MAX_WIDTH && (2 * h) <= VD6G_MAX_HEIGHT) ? 2 : 1;
    }
    if ((bin != 1 && bin != 2 && bin != 4) || (w * bin) > VD6G_MAX_WIDTH || (h * bin) > VD6G_MAX_HEIGHT) {
        return -1;
    }
    s.cfg = *c;
    s.bin = (uint8_t) bin;

    // Centred ROI, even start coordinates to keep the CFA phase.
    uint32_t cw = w * bin, ch = h * bin;
    uint32_t x0 = (ARRAY_X0 + (VD6G_MAX_WIDTH - cw) / 2) & ~1u;
    uint32_t y0 = ((VD6G_MAX_HEIGHT - ch) / 2) & ~1u;
    s.rows = (uint16_t) ch;
    s.vblank = c->vblank ? c->vblank : DEF_VBLANK;
    s.frame_us = c->frame_us;
    s.expo_us = c->expo_us;
    s.again = gain_code(c->gain_db);
    for (int i = 0; i < VD6G_GPIO_NB; i++) {
        s.gpio[i] = VD6G_GPIO_GPIO_IN;
    }
    if (c->strobe_gpio >= 0 && c->strobe_gpio < VD6G_GPIO_NB) {
        s.gpio[c->strobe_gpio] = VD6G_GPIO_STROBE | (c->strobe_inv ? VD6G_GPIO_INVERTED : VD6G_GPIO_NO_INVERSION);
    }

    int ret = cam_stop();
    ret |= rd16(REG_LINE_LENGTH, &s.line_len);
    if (!s.line_len) {
        return -1;
    }

    // MIPI CSI-2: 2 lanes, RAW10, lane swaps as used by ST for the STEVAL modules.
    const uint16_t oif_ctrl = (1 << 9) | (1 << 7) | (1 << 6) | (0 << 4) | (1 << 3) | (2 << 0);
    ret |= wr8(REG_FORMAT_CTRL, 10);
    ret |= wr16(REG_OIF_CTRL, oif_ctrl);
    ret |= wr16(REG_OIF_CSI_BITRATE, CAM_CSI_MBPS);
    ret |= wr8(REG_OIF_IMG_CTRL, RAW10_DATA_TYPE);

    // Readout window and binning.
    ret |= wr8(REG_READOUT_CTRL, (bin == 4) ? 2 : (bin == 2) ? 1 : 0);
    ret |= wr16(REG_OUT_ROI_X_START, (uint16_t) x0);
    ret |= wr16(REG_OUT_ROI_X_END, (uint16_t) (x0 + cw - 1));
    ret |= wr16(REG_OUT_ROI_Y_START, (uint16_t) y0);
    ret |= wr16(REG_OUT_ROI_Y_END, (uint16_t) (y0 + ch - 1));

    // Orientation and test pattern.
    ret |= wr8(REG_ORIENTATION, (c->hmirror ? 1 : 0) | (c->vflip ? 2 : 0));
    if (c->colorbar) {
        ret |= wr8(REG_DUSTER_CTRL, 0);
        ret |= wr8(REG_DARKCAL_CTRL, 2);
    }
    ret |= wr16(REG_PATGEN_CTRL, c->colorbar ? PATGEN_VER_COLOR_BAR : 0);

    // No flicker avoidance (strobe lighting).
    ret |= wr16(REG_AE_COMPILER_CONTROL, 0);

    for (int i = 0; i < VD6G_GPIO_NB; i++) {
        ret |= wr8(REG_GPIO_CTRL(i), s.gpio[i]);
    }

    // Frame length first, then the exposure (clamped to it).
    s.frame_len = 0xFFFF;
    ret |= write_frame_len();
    ret |= write_exposure();

    ret |= cam_start();
    return ret;
}

int cam_set_frame_us(uint32_t us) {
    s.frame_us = us;
    return s.streaming ? write_frame_len() : 0;
}

int cam_set_exposure_us(uint32_t us) {
    s.expo_us = us;
    return s.booted ? write_exposure() : 0;
}

int cam_set_gain_db(float db) {
    s.again = gain_code(db);
    return s.booted ? write_exposure() : 0;
}

int cam_set_vblank(uint16_t lines) {
    if (lines < 1 || lines > 4000) {
        return -1;
    }
    s.vblank = lines;
    return s.streaming ? write_frame_len() : 0;
}

void cam_get_info(cam_info_t *info) {
    memset(info, 0, sizeof(*info));
    uint16_t fl = s.frame_len, expo = 0;
    rd16(REG_FRAME_LENGTH, &fl);
    rd16(REG_MANUAL_COARSE_EXP, &expo);
    info->model = s.model;
    info->mono = s.mono;
    info->cfa = cfa_of(s.cfg.hmirror, s.cfg.vflip);
    info->line_len = s.line_len;
    info->frame_len = fl;
    info->rows = s.rows;
    info->bin = s.bin;
    info->line_ns = line_time_ns();
    info->min_frame_us = lines_to_us(min_frame_len());
    info->frame_us = lines_to_us(fl);
    info->expo_us = (uint32_t) (((uint64_t) expo * line_time_ns()) / 1000);
    info->again = s.again;
}
