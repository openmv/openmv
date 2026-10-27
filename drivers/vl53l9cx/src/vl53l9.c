/**
 ******************************************************************************
 * @file    vl53l9.c
 * @author  IMD Software Team
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 * @warning
 *
 * Use of control adjustments, or procedures other than those specified in
 * product datasheet may result in hazardous product behavior.
 *
 */

/* private headers ***********************************************************/

#include "vl53l9.h"
#include "vl53l9_patch.h"
#include "vl53l9_platform.h"
#include "vl53l9_reg.h"

#include <errno.h>
#include <math.h>   // pow
#include <stddef.h> // NULL
#include <stdint.h>

/* private macros ***********************************************************/

#ifdef __GNUC__
// make use of '__builtin_ffsl' provided by GCC
#define __ffsl(x) (__builtin_ffsl(x))
// Alternate implementation based on '__builtin_clz'
// #define __ffsl(x) (32U - __builtin_clz(x & -x))
#elif __ICCARM__
#include "intrinsics.h"
#define __ffsl(x) (32U - __CLZ(x & -x))
#else
// Not yet tested with other compilers
#endif

#define __bf_shf(x) (__ffsl(x) - 1U)

#define _FIELD_PREP(_mask, _val) ({ ((uint32_t)(_val) << __bf_shf(_mask)) & (_mask); })
#define _FIELD_GET(_mask, _reg)  ({ (uint32_t)(((_reg) & (_mask)) >> __bf_shf(_mask)); })

#define CHECK_NULL_PTR(ptr)                    \
    do {                                       \
        if (ptr == NULL) {                     \
            return VL53L9_ERROR_INVALID_PARAM; \
        }                                      \
    } while (0)

#define CHECK_RET(ret)  \
    do {                \
        if (ret != 0) { \
            return ret; \
        }               \
    } while (0)

// raw frame size (including cropped pixels)
#define FRAME_SIZE_BINNING_2  (54U * 42U)
#define FRAME_SIZE_BINNING_4  (24U * 24U) // cropped: 24 * 20
#define FRAME_SIZE_BINNING_6  (18U * 14U)
#define FRAME_SIZE_BINNING_8  (12U * 10U)
#define FRAME_SIZE_BINNING_12 (8U * 8U) // cropped: 8 * 6
#define FRAME_SIZE_BINNING_24 (4U * 4U)
#define FRAME_SIZE(binning)   FRAME_SIZE_BINNING_##binning

// dss size
#define DSS_SIZE_BINNING_2  (1134U)
#define DSS_SIZE_BINNING_4  (288U)
#define DSS_SIZE_BINNING_6  (126U)
#define DSS_SIZE_BINNING_8  (60U)
#define DSS_SIZE_BINNING_12 (32U)
#define DSS_SIZE_BINNING_24 (8U)
#define DSS_SIZE(binning)   DSS_SIZE_BINNING_##binning

// raw buffer size (3 frames + dss + status)
#define RAW_BUFFER_SIZE(binning) ((FRAME_SIZE(binning) * 3U * 2U) + DSS_SIZE(binning) + VL53L9_STATUS_SIZE)

/* private types *************************************************************/

typedef enum {
    FSM_STATE_NONE = 0U,
    FSM_STATE_READY_TO_BOOT = 1U,
    FSM_STATE_STANDBY = 2U,
    FSM_STATE_STREAMING = 3U,
} _fsm_state_t;

typedef enum {
    COMMAND_NONE = 0x0,
    COMMAND_BOOT = 0x1,                 // request fw patch setup
    COMMAND_START_STREAM = 0x2,         // request start streaming
    COMMAND_STOP_STREAM = 0x3,          // request stop streaming
    COMMAND_TRIGGER_NEXT_FRAME = 0x5,   // trigger next frame in manual mode
    COMMAND_ACK_FRAME_READ = 0x6,       // acknowledge frame read back through i3c
    COMMAND_SWITCH_TO_FAST_CLOCK = 0x7, // turn on the pll and switch the system clock to the fast clock
    COMMAND_SWITCH_TO_EXT_CLOCK = 0x8,  // turn off the pll and switch the system clock to the external clock
    COMMAND_OTP_CUSTOM_WRITE = 0x9,     // write OTP custom from OTP_CUSTOM mirror page
    COMMAND_DSS_LUT_MAP = 0x0A,         // map DSS LUT buffer
    COMMAND_DSS_LUT_UNMAP = 0x0B,       // remap default output buffer
    COMMAND_OTP_UNPACK = 0x12,          // apply OTP settings from UI OTP mirror page
} _command_t;

typedef enum {
    DSS_DISABLE = 0U,
    DSS_LONG = 1U,
    DSS_SHORT = 2U,
} _dss_mode_t;

typedef enum {
    FORMAT_SQUARE = 0U,
    FORMAT_WIDE = 1U,
} _format_t;

typedef struct {
    uint32_t x_size : 6;
    uint32_t y_size : 6;
    uint32_t x_offset : 6;
    uint32_t y_offset : 6;
    uint32_t enable : 1;
} _crop_config_t;

/* private global variables **************************************************/

static const uint8_t tx_channel_0_short[7] = { 1, 3, 5, 11, 9, 9, 0 };
static const uint8_t tx_channel_1_short[7] = { 2, 4, 6, 12, 10, 10, 0 };
static const uint16_t blanking_short[7] = { 296, 116, 26, 20, 18, 18, 0 };
static const uint8_t dithering_short[7] = { 31, 31, 31, 31, 31, 31, 0 };

static const uint8_t tx_channel_0_long[7] = { 1, 3, 13, 11, 0, 11, 0 };
static const uint8_t tx_channel_1_long[7] = { 2, 4, 14, 12, 0, 12, 0 };
static const uint16_t blanking_long[7] = { 296, 116, 54, 20, 0, 20, 0 };
static const uint8_t dithering_long[7] = { 31, 31, 31, 31, 31, 31, 0 };

/* private functions prototypes **********************************************/

static _fsm_state_t _get_fsm_state(void *const p_dev);
static int _wait_for_state(void *const p_dev, _fsm_state_t state, uint32_t timeout_ms);
static int _write_cmd(void *const p_dev, _command_t cmd, uint32_t timeout_ms);
static int _init_default_config(void *const p_dev);
static int _is_valid_csi_config(void *const p_dev);
static int _write_crop_config(void *const p_dev, _crop_config_t *p_crop);
static unsigned int _get_raw_frame_resolution(uint8_t binning);

/* public functions implementation *******************************************/

int vl53l9_init(void *const p_dev) {
    int ret;
    vl53l9_vddio_t voltage_vddio;
    vl53l9_vdda_t voltage_vdda;
    uint32_t ext_clock;

    CHECK_NULL_PTR(p_dev);
    ret = _wait_for_state(p_dev, FSM_STATE_READY_TO_BOOT, 4);
    CHECK_RET(ret);

    ret = vl53l9_get_config_ext_clock(p_dev, &ext_clock);
    CHECK_RET(ret);
    ret = vl53l9_write32(p_dev, VL53L9_REGADDR_EXT_CLOCK, ext_clock);
    CHECK_RET(ret);

    ret = vl53l9_get_config_vddio(p_dev, &voltage_vddio);
    CHECK_RET(ret);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_VDDIO_CFG, (uint8_t)voltage_vddio);
    CHECK_RET(ret);

    ret = vl53l9_get_config_vdda(p_dev, &voltage_vdda);
    CHECK_RET(ret);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_VDDA_CFG, (uint8_t)voltage_vdda);
    CHECK_RET(ret);

    // load firmware patch
    ret = vl53l9_write(p_dev, VL53L9_REGADDR_FWPATCH, (uint8_t *)&g_vl53l9_fw_patch[0], sizeof(g_vl53l9_fw_patch));
    CHECK_RET(ret);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_SETTING_INSTALL_PATCH, 1);
    CHECK_RET(ret);
    ret = _write_cmd(p_dev, COMMAND_BOOT, 71);
    CHECK_RET(ret);
    ret = _wait_for_state(p_dev, FSM_STATE_STANDBY, 4);
    CHECK_RET(ret);

    // check firmware version
    uint8_t fw_patch_ver[2];
    ret = vl53l9_read(p_dev, VL53L9_REGADDR_PATCH_REVISION, fw_patch_ver, 2);
    CHECK_RET(ret);
    ret = ((fw_patch_ver[1] == VL53L9_FW_PATCH_VER_MAJOR) && (fw_patch_ver[0] == VL53L9_FW_PATCH_VER_MINOR))
              ? VL53L9_ERROR_NONE
              : VL53L9_ERROR_INTERNAL;
    CHECK_RET(ret);

    // apply default settings
    ret = _init_default_config(p_dev);
    CHECK_RET(ret);

    return ret;
}

int vl53l9_set_com_config(void *const p_dev, uint8_t address, uint8_t instance_id) {
    int ret;
    uint8_t dynamic_address_active = 0;
    CHECK_NULL_PTR(p_dev);
    ret = vl53l9_read8(p_dev, 0xd20A, &dynamic_address_active);
    CHECK_RET(ret);

    ret = vl53l9_write8(p_dev, 0xd238, instance_id & (uint8_t)0xf);
    CHECK_RET(ret);

    ret = vl53l9_write8(p_dev, 0xd208, address >> 1);
    CHECK_RET(ret);

    if (dynamic_address_active == (uint8_t)0) {
        ret = vl53l9_write8(p_dev, 0xd20A, 0x1);
        CHECK_RET(ret);
    }
    return ret;
}

int vl53l9_get_com_config(void *const p_dev, uint8_t *p_address, uint8_t *p_instance_id) {
    int ret;
    uint8_t data;
    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(p_address);
    CHECK_NULL_PTR(p_instance_id);

    ret = vl53l9_read8(p_dev, 0xd238, &data);
    CHECK_RET(ret);
    *p_instance_id = data & (uint8_t)0xf;

    ret = vl53l9_read8(p_dev, 0xd208, &data);
    CHECK_RET(ret);
    *p_address = (uint8_t)((data & (uint8_t)0x7f) << 1);

    return ret;
}

int vl53l9_get_device_id(void *const p_dev, uint32_t *p_id) {
    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(p_id);
    return vl53l9_read32(p_dev, VL53L9_REGADDR_MODEL_ID, p_id);
}

int vl53l9_get_calib_data(void *const p_dev, uint8_t *p_buffer) {
    int ret;

    CHECK_NULL_PTR(p_buffer);

    // enable fast clock mode to be able to perform burst read when the device is in standby
    ret = _write_cmd(p_dev, COMMAND_SWITCH_TO_FAST_CLOCK, 5);
    CHECK_RET(ret);

    // retrieve calibration data
    ret = vl53l9_read(p_dev, VL53L9_REGBASE_DEBUG_SETTINGS, p_buffer, VL53L9_CALIB_DATA_SIZE);
    CHECK_RET(ret);

    // restore external clock mode
    ret = _write_cmd(p_dev, COMMAND_SWITCH_TO_EXT_CLOCK, 5);
    CHECK_RET(ret);

    return ret;
}

int vl53l9_get_hw_config(void *const p_dev, vl53l9_hw_config_t *p_config) {
    int ret;
    uint32_t data = 0;

    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(p_config);
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_OUTPUT_IF, (uint8_t *)&data);
    CHECK_RET(ret);
    p_config->output_interface = _FIELD_GET(VL53L9_REGFIELD_OUTPUT_IF, data);

    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_FRAME_SIGNALING_MODE, (uint8_t *)&data);
    CHECK_RET(ret);
    p_config->signaling_mode = _FIELD_GET(VL53L9_REGFIELD_FRAME_SIGNALING_MODE, data);

    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_INTR_OUTPUT_MODE, (uint8_t *)&data);
    CHECK_RET(ret);
    p_config->interrupt_pad_mode = _FIELD_GET(VL53L9_REGFIELD_INTR_OUTPUT_MODE, data);

    ret = vl53l9_read32(p_dev, VL53L9_REGADDR_CSI2_DATA_RATE, &data);
    CHECK_RET(ret);
    p_config->csi_data_rate = data;

    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_CSI2_VIRTUAL_CHANNEL, (uint8_t *)&data);
    CHECK_RET(ret);
    p_config->csi_virtual_channel = _FIELD_GET(VL53L9_REGFIELD_CSI2_VIRTUAL_CHANNEL, data);

    ret = vl53l9_read16(p_dev, VL53L9_REGADDR_CSI2_ISL, (uint16_t *)&data);
    CHECK_RET(ret);
    p_config->csi_status_line_datatype = _FIELD_GET(VL53L9_REGFIELD_CSI2_ISL_DATA_TYPE, data);
    p_config->csi_status_line_force_width = _FIELD_GET(VL53L9_REGFIELD_CSI2_ISL_FORCE_WIDTH, data);

    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_CSI2_FRAME_DATA_TYPE, (uint8_t *)&data);
    CHECK_RET(ret);
    p_config->csi_frame_datatype = _FIELD_GET(VL53L9_REGFIELD_CSI2_FRAME_DATA_TYPE, data);

    ret = vl53l9_read16(p_dev, VL53L9_REGADDR_CSI2_FRAME_HEIGHT, (uint16_t *)&data);
    CHECK_RET(ret);
    p_config->csi_frame_height = (uint16_t)data;

    ret = vl53l9_read16(p_dev, VL53L9_REGADDR_CSI2_FRAME_WIDTH, (uint16_t *)&data);
    CHECK_RET(ret);
    p_config->csi_frame_width = (uint16_t)data;

    return ret;
}

int vl53l9_set_hw_config(void *const p_dev, vl53l9_hw_config_t config) {
    int ret;
    uint16_t status_line_cfg;

    CHECK_NULL_PTR(p_dev);
    if (_get_fsm_state(p_dev) != FSM_STATE_STANDBY) {
        return VL53L9_ERROR_INVALID_STATE;
    }

    if ((config.output_interface == VL53L9_OUTPUT_CSI2) && (config.csi_frame_width < VL53L9_STATUS_SIZE)) {
        return VL53L9_ERROR_INVALID_PARAM;
    }

    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_OUTPUT_IF, (uint8_t)config.output_interface);
    CHECK_RET(ret);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_FRAME_SIGNALING_MODE, (uint8_t)config.signaling_mode);
    CHECK_RET(ret);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_INTR_OUTPUT_MODE, (uint8_t)config.interrupt_pad_mode);
    CHECK_RET(ret);

    ret = vl53l9_write32(p_dev, VL53L9_REGADDR_CSI2_DATA_RATE, config.csi_data_rate);
    CHECK_RET(ret);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_CSI2_VIRTUAL_CHANNEL, config.csi_virtual_channel);
    CHECK_RET(ret);
    status_line_cfg = (((uint16_t)config.csi_status_line_datatype & 0x2fU) |
                       (((uint16_t)config.csi_status_line_force_width & 1U) << 6));
    ret = vl53l9_write16(p_dev, VL53L9_REGADDR_CSI2_ISL, status_line_cfg);
    CHECK_RET(ret);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_CSI2_FRAME_DATA_TYPE, config.csi_frame_datatype);
    CHECK_RET(ret);
    ret = vl53l9_write16(p_dev, VL53L9_REGADDR_CSI2_FRAME_HEIGHT, config.csi_frame_height);
    CHECK_RET(ret);
    ret = vl53l9_write16(p_dev, VL53L9_REGADDR_CSI2_FRAME_WIDTH, config.csi_frame_width);
    CHECK_RET(ret);

    return ret;
}

int vl53l9_get_power_mode(void *const p_dev, vl53l9_power_mode_t *p_mode) {
    int ret;
    uint8_t data;

    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(p_mode);
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_POWER_MODE, &data);
    *p_mode = (vl53l9_power_mode_t)_FIELD_GET(VL53L9_REGFIELD_POWER_MODE, data);
    return ret;
}

int vl53l9_set_power_mode(void *const p_dev, vl53l9_power_mode_t mode) {
    CHECK_NULL_PTR(p_dev);
    if (_get_fsm_state(p_dev) != FSM_STATE_STANDBY) {
        return VL53L9_ERROR_INVALID_STATE;
    }
    return vl53l9_write8(p_dev, VL53L9_REGADDR_POWER_MODE, (uint8_t)mode);
}

int vl53l9_get_sync_mode(void *const p_dev, vl53l9_sync_mode_t *p_mode) {
    int ret;
    uint8_t data;

    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(p_mode);
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_SYNCHRO, &data);
    CHECK_RET(ret);
    *p_mode = (vl53l9_sync_mode_t)_FIELD_GET(VL53L9_REGFIELD_SYNCHRO, data);
    return ret;
}

int vl53l9_set_sync_mode(void *const p_dev, vl53l9_sync_mode_t mode) {
    CHECK_NULL_PTR(p_dev);
    if (_get_fsm_state(p_dev) != FSM_STATE_STANDBY) {
        return VL53L9_ERROR_INVALID_STATE;
    }
    return vl53l9_write8(p_dev, VL53L9_REGADDR_SYNCHRO, (uint8_t)mode);
}

int vl53l9_get_frame_period(void *const p_dev, uint32_t *p_period_us) {
    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(p_period_us);
    return vl53l9_read32(p_dev, VL53L9_REGADDR_FRAME_PERIOD, p_period_us);
}

int vl53l9_set_frame_period(void *const p_dev, uint32_t period_us) {
    CHECK_NULL_PTR(p_dev);
    if (_get_fsm_state(p_dev) != FSM_STATE_STANDBY) {
        return VL53L9_ERROR_INVALID_STATE;
    }
    if ((period_us < (10U * 1000U)) || (period_us > (1U * 1000U * 1000U))) {
        return VL53L9_ERROR_INVALID_PARAM;
    }
    return vl53l9_write32(p_dev, VL53L9_REGADDR_FRAME_PERIOD, period_us);
}

int vl53l9_get_context(void *const p_dev, vl53l9_context_t *p_context) {
    int ret;
    uint8_t data;

    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(p_context);
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_CONTEXT_SELECTION, &data);
    CHECK_RET(ret);
    *p_context = (vl53l9_context_t)_FIELD_GET(VL53L9_REGFIELD_CONTEXT_SELECTION, data);
    return ret;
}

int vl53l9_set_context(void *const p_dev, vl53l9_context_t context) {
    int ret;

    CHECK_NULL_PTR(p_dev);
    if (_get_fsm_state(p_dev) != FSM_STATE_STANDBY) {
        return VL53L9_ERROR_INVALID_STATE;
    }

    int16_t cal_prog_offset_1_5 = (context == VL53L9_CONTEXT_SHORT) ? 1 : -8;
    int16_t cal_prog_offset_6 = (context == VL53L9_CONTEXT_SHORT) ? 5 : -1;

    ret = vl53l9_write16(p_dev, VL53L9_REGADDR_CAL_PROG_OFFSET_1TO5, (uint16_t)cal_prog_offset_1_5);
    CHECK_RET(ret);
    ret = vl53l9_write16(p_dev, VL53L9_REGADDR_CAL_PROG_OFFSET_6, (uint16_t)cal_prog_offset_6);
    CHECK_RET(ret);

    uint16_t cab_short_dist_scale = (context == VL53L9_CONTEXT_SHORT) ? 256U : 683U;
    uint16_t cab_long_dist_scale = 2048U;

    uint32_t cab_dist_scale = 0U;
    cab_dist_scale |= _FIELD_PREP(VL53L9_REGFIELD_CAB_SHORT_SCALE, cab_short_dist_scale);
    cab_dist_scale |= _FIELD_PREP(VL53L9_REGFIELD_CAB_LONG_SCALE, cab_long_dist_scale);

    ret = vl53l9_write32(p_dev, VL53L9_REGADDR_CAB_DIST_SCALE, cab_dist_scale);
    CHECK_RET(ret);

    return vl53l9_write8(p_dev, VL53L9_REGADDR_CONTEXT_SELECTION, (uint8_t)context);
}

int vl53l9_get_binning(void *const p_dev, vl53l9_context_t context, uint8_t *p_binning) {
    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(p_binning);
    return vl53l9_read8(p_dev, VL53L9_REGADDR_STANDBY_BINNING((uint16_t)context), p_binning);
}

int vl53l9_set_binning(void *const p_dev, vl53l9_context_t context, uint8_t binning) {
    int ret;
    _dss_mode_t dss_mode = (context == VL53L9_CONTEXT_SHORT) ? DSS_SHORT : DSS_LONG;
    _format_t format = FORMAT_WIDE;
    _crop_config_t crop = { 0 };

    CHECK_NULL_PTR(p_dev);
    if (_get_fsm_state(p_dev) != FSM_STATE_STANDBY) {
        return VL53L9_ERROR_INVALID_STATE;
    }

    switch (binning) {
    case 2: // 54x42
        break;
    case 4: // 24x20
        format = FORMAT_SQUARE;
        crop.x_size = 24;
        crop.y_size = 20;
        crop.x_offset = 0;
        crop.y_offset = 2;
        crop.enable = 1;
        break;
    case 6: // 18x14
        break;
    case 8: // 12x10
        break;
    case 12: // 8x6
        format = FORMAT_SQUARE;
        crop.x_size = 8;
        crop.y_size = 6;
        crop.x_offset = 0;
        crop.y_offset = 1;
        crop.enable = 1;
        break;
    case 24: // 4x4
        format = FORMAT_SQUARE;
        break;
    default:
        return VL53L9_ERROR_INVALID_PARAM;
        break;
    }

    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_STANDBY_BINNING((uint16_t)context), binning);
    CHECK_RET(ret);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_STANDBY_DSS_MODE((uint16_t)context), (uint8_t)dss_mode);
    CHECK_RET(ret);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_FORMAT, (uint8_t)format);
    CHECK_RET(ret);

    ret = _write_crop_config(p_dev, &crop);
    return ret;
}

int vl53l9_get_exposure(void *const p_dev, vl53l9_context_t context, uint16_t *p_exposure_ms) {
    int ret = VL53L9_ERROR_NONE;

    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(p_exposure_ms);

    float expo_acc = 0.0f;
    uint32_t shots[7] = { 0 };
    const uint16_t *blanking = (context == VL53L9_CONTEXT_SHORT) ? blanking_short : blanking_long;

    // NOTE: perform a single read instead of burst (which would require to switch to fast clock mode when in standby)
    for (uint16_t i = 0U; i < 7U; i++) {
        ret = vl53l9_read32(p_dev, VL53L9_REGADDR_STREAM_NB_SHOT_STEP(i + 1U, (uint16_t)context), &shots[i]);
        CHECK_RET(ret);
    }

    for (uint16_t i = 0U; i < 7U; i++) {
        uint32_t num = 2U * (64U + (uint32_t)blanking[i]) * shots[i];
        float den = 1000000.0f;
        expo_acc += (float)num / den;
    }

    errno = 0; // MISRA-C 2012 Rule 22.8 requires to clear errno before errno-setting functions
    *p_exposure_ms = (uint16_t)(ceilf(expo_acc));

    return ret;
}

int vl53l9_set_exposure(void *const p_dev, vl53l9_context_t context, uint16_t exposure_ms) {

    uint32_t shots[7] = { 0 };

    static const uint16_t short_ratios[7] = { 100, 200, 400, 615, 1231, 1231, 100 };
    static const uint16_t long_ratios[7] = { 100, 200, 308, 615, 0, 615, 100 };
    const uint16_t *ratios = (context == VL53L9_CONTEXT_SHORT) ? short_ratios : long_ratios;

    uint32_t shots_base;
    uint32_t step_shot;

    const uint16_t *blanking = (context == VL53L9_CONTEXT_SHORT) ? blanking_short : blanking_long;
    uint32_t blank_sum = 0;

    if ((exposure_ms < 1U) || (exposure_ms > 30U)) {
        return VL53L9_ERROR_INVALID_PARAM;
    }

    for (uint8_t i = 0U; i < 7U; i++) {
        blank_sum += ((64U + (uint32_t)blanking[i]) * ratios[i]);
    }

    if (blank_sum == 0U) {
        return VL53L9_ERROR_INTERNAL;
    }

    shots_base = (500000UL * exposure_ms) / blank_sum;
    for (uint8_t i = 0U; i < 7U; i++) {
        step_shot = shots_base * ratios[i];
        shots[i] = step_shot & GENMASK(23, 0);
    }

    return vl53l9_write(p_dev, VL53L9_REGADDR_STREAM_NB_SHOT_STEP(1U, (uint16_t)context), (uint8_t *)shots,
                        sizeof(shots));
}

int vl53l9_start(void *const p_dev) {
    int ret;
    uint8_t output_interface;
    CHECK_NULL_PTR(p_dev);
    if (_get_fsm_state(p_dev) != FSM_STATE_STANDBY) {
        return VL53L9_ERROR_INVALID_STATE;
    }

    // make sure csi settings are consistent with binning
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_OUTPUT_IF, &output_interface);
    CHECK_RET(ret);
    output_interface = _FIELD_GET(VL53L9_REGFIELD_OUTPUT_IF, output_interface);
    if (output_interface == VL53L9_OUTPUT_CSI2) {
        ret = _is_valid_csi_config(p_dev);
        CHECK_RET(ret);
    }
    return _write_cmd(p_dev, COMMAND_START_STREAM, 60);
}

int vl53l9_stop(void *const p_dev) {
    CHECK_NULL_PTR(p_dev);
    if (_get_fsm_state(p_dev) != FSM_STATE_STREAMING) {
        return VL53L9_ERROR_INVALID_STATE;
    }
    return _write_cmd(p_dev, COMMAND_STOP_STREAM, 14);
}

int vl53l9_trigger_frame(void *const p_dev) {
    int ret;
    vl53l9_sync_mode_t sync_mode;

    CHECK_NULL_PTR(p_dev);
    if (_get_fsm_state(p_dev) != FSM_STATE_STREAMING) {
        return VL53L9_ERROR_INVALID_STATE;
    }
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_SYNCHRO, (uint8_t *)&sync_mode);
    CHECK_RET(ret);
    if (sync_mode != VL53L9_SYNC_MANUAL) {
        return VL53L9_ERROR_INVALID_OPERATION;
    }
    return _write_cmd(p_dev, COMMAND_TRIGGER_NEXT_FRAME, 30);
}

int vl53l9_poll_frame(void *const p_dev, uint8_t *p_is_ready) {
    int ret;
    uint8_t data;

    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(p_is_ready);
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_FRAME_READY, &data);
    CHECK_RET(ret);
    *p_is_ready = _FIELD_GET(VL53L9_REGFIELD_FRAME_READY, (uint32_t)data);
    return ret;
}

int vl53l9_get_frame(void *const p_dev, uint8_t *p_buffer, uint16_t size) {
    int ret;
    uint8_t data;

    CHECK_NULL_PTR(p_dev);

    // if provided size is 0, acknowledge the frame without reading it
    if (size == 0U) {
        return _write_cmd(p_dev, COMMAND_ACK_FRAME_READ, 30);
    }

    CHECK_NULL_PTR(p_buffer);

    // return error if the frame is not ready
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_FRAME_READY, &data);
    CHECK_RET(ret);
    data = _FIELD_GET(VL53L9_REGFIELD_FRAME_READY, (uint32_t)data);
    if (data != 1U) {
        return VL53L9_ERROR_INVALID_STATE;
    }

    // retrieve current context and binning
    vl53l9_context_t context;
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_CONTEXT_SELECTION, (uint8_t *)&context);
    CHECK_RET(ret);
    context = (vl53l9_context_t)_FIELD_GET(VL53L9_REGFIELD_CONTEXT_SELECTION, (uint8_t)context);
    uint8_t binning;
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_STANDBY_BINNING((uint16_t)context), &binning);
    CHECK_RET(ret);

    // return error if the requsted size doesn't match the configuration
    uint16_t expected_size;
    ret = vl53l9_get_raw_buffer_size(binning, &expected_size);
    CHECK_RET(ret);

    if (size != expected_size) {
        return VL53L9_ERROR_INVALID_PARAM;
    }

    uint16_t resolution = (uint16_t)_get_raw_frame_resolution(binning);
    if (resolution == 0U) {
        return VL53L9_ERROR_INTERNAL;
    }

    uint32_t block_size = (uint32_t)resolution * 2U;
    uint16_t dss_block_size = (uint16_t)(resolution / 2U);
    uint32_t offset = 0U;

    // read depth, amplitude and ambient
    ret = vl53l9_read(p_dev, VL53L9_REGADDR_FB_DEPTH, &p_buffer[offset], 3U * block_size);
    CHECK_RET(ret);
    offset = 3U * block_size;

    // read dss indexes for lut (1 entry per binned pixel, 2 indexes per byte)
    // NOTE: in current implementation, the dss is always enabled so no check needed
    ret = _write_cmd(p_dev, COMMAND_DSS_LUT_MAP, 5);
    CHECK_RET(ret);
    ret = vl53l9_read(p_dev, VL53L9_REGADDR_FB_DEPTH, &p_buffer[offset], dss_block_size);
    CHECK_RET(ret);
    ret = _write_cmd(p_dev, COMMAND_DSS_LUT_UNMAP, 5);
    CHECK_RET(ret);
    offset += dss_block_size;

    // read status line
    ret = vl53l9_read(p_dev, VL53L9_REGBASE_SENSOR_STATUS, &p_buffer[offset], VL53L9_STATUS_SIZE);
    CHECK_RET(ret);

    // clear the frame_ready flag and release the interrupt pin
    return _write_cmd(p_dev, COMMAND_ACK_FRAME_READ, 30);
}

int vl53l9_get_frame_async(void *const p_dev, uint8_t *p_buffer, uint16_t size) {
    int ret;
    uint8_t data;

    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(p_buffer);

    // return error if the frame is not ready
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_FRAME_READY, &data);
    CHECK_RET(ret);
    data = _FIELD_GET(VL53L9_REGFIELD_FRAME_READY, (uint32_t)data);
    if (data != 1U) {
        return VL53L9_ERROR_INVALID_STATE;
    }

    // retrieve current context and binning
    vl53l9_context_t context;
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_CONTEXT_SELECTION, (uint8_t *)&context);
    CHECK_RET(ret);
    context = (vl53l9_context_t)_FIELD_GET(VL53L9_REGFIELD_CONTEXT_SELECTION, (uint8_t)context);
    uint8_t binning;
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_STANDBY_BINNING((uint16_t)context), &binning);
    CHECK_RET(ret);

    // return error if the requsted size doesn't match the configuration
    uint16_t expected_size;
    ret = vl53l9_get_raw_buffer_size(binning, &expected_size);
    CHECK_RET(ret);

    if (size != expected_size) {
        return VL53L9_ERROR_INVALID_PARAM;
    }

    uint16_t resolution = (uint16_t)_get_raw_frame_resolution(binning);
    if (resolution == 0U) {
        return VL53L9_ERROR_INTERNAL;
    }

    uint32_t block_size = (uint32_t)resolution * 2U;
    uint32_t offset = 0U;

    // read depth, amplitude and ambient
    ret = vl53l9_read_async(p_dev, VL53L9_REGADDR_FB_DEPTH, &p_buffer[offset], 3U * block_size);
    CHECK_RET(ret);

    return ret;
}

int vl53l9_get_frame_async_ack(void *const p_dev, uint8_t *p_buffer, uint16_t size) {
    int ret;
    uint8_t data;

    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(p_buffer);

    // return error if the frame is not ready
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_FRAME_READY, &data);
    CHECK_RET(ret);
    data = _FIELD_GET(VL53L9_REGFIELD_FRAME_READY, (uint32_t)data);
    if (data != 1U) {
        return VL53L9_ERROR_INVALID_STATE;
    }

    // retrieve current context and binning
    vl53l9_context_t context;
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_CONTEXT_SELECTION, (uint8_t *)&context);
    CHECK_RET(ret);
    context = (vl53l9_context_t)_FIELD_GET(VL53L9_REGFIELD_CONTEXT_SELECTION, (uint8_t)context);
    uint8_t binning;
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_STANDBY_BINNING((uint16_t)context), &binning);
    CHECK_RET(ret);

    // return error if the requsted size doesn't match the configuration
    uint16_t expected_size;
    ret = vl53l9_get_raw_buffer_size(binning, &expected_size);
    CHECK_RET(ret);

    if (size != expected_size) {
        return VL53L9_ERROR_INVALID_PARAM;
    }

    uint16_t resolution = (uint16_t)_get_raw_frame_resolution(binning);
    if (resolution == 0U) {
        return VL53L9_ERROR_INTERNAL;
    }

    uint16_t block_size = resolution * 2U;
    uint16_t dss_block_size = (uint16_t)(resolution / 2U);
    uint16_t offset = 3U * block_size;

    // read dss indexes for lut (1 entry per binned pixel, 2 indexes per byte)
    // NOTE: in current implementation, the dss is always enabled so no check needed
    ret = _write_cmd(p_dev, COMMAND_DSS_LUT_MAP, 5);
    CHECK_RET(ret);
    ret = vl53l9_read(p_dev, VL53L9_REGADDR_FB_DEPTH, &p_buffer[offset], dss_block_size);
    CHECK_RET(ret);
    ret = _write_cmd(p_dev, COMMAND_DSS_LUT_UNMAP, 5);
    CHECK_RET(ret);
    offset += dss_block_size;

    // read status line
    ret = vl53l9_read(p_dev, VL53L9_REGBASE_SENSOR_STATUS, &p_buffer[offset], VL53L9_STATUS_SIZE);
    CHECK_RET(ret);

    // clear the frame_ready flag and release the interrupt pin
    return _write_cmd(p_dev, COMMAND_ACK_FRAME_READ, 30);
}

int vl53l9_get_status(void *const p_dev, vl53l9_status_t *status) {
    int ret;

    CHECK_NULL_PTR(p_dev);
    CHECK_NULL_PTR(status);

    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_SYSTEM_FSM, (uint8_t *)&status->fsm);
    CHECK_RET(ret);
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_COMMAND_ERROR, (uint8_t *)&status->command);
    CHECK_RET(ret);
    ret = vl53l9_read16(p_dev, VL53L9_REGADDR_ERROR_CODE, (uint16_t *)&status->firmware);
    CHECK_RET(ret);
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_ERROR_STATUS, (uint8_t *)&status->error);
    CHECK_RET(ret);
    for (uint16_t i = 0U; i < 5U; i++) {
        ret = vl53l9_read8(p_dev, VL53L9_REGADDR_LDD_STATUS(i), (uint8_t *)status->laser_driver);
        CHECK_RET(ret);
    }

    return ret;
}

int vl53l9_get_raw_buffer_size(uint8_t binning, uint16_t *p_size) {

    int ret = VL53L9_ERROR_NONE;

    if (p_size == NULL) {
        return VL53L9_ERROR_INVALID_PARAM;
    }

    switch (binning) {
    case 2:
        *p_size = RAW_BUFFER_SIZE(2);
        break;
    case 4:
        *p_size = RAW_BUFFER_SIZE(4);
        break;
    case 6:
        *p_size = RAW_BUFFER_SIZE(6);
        break;
    case 8:
        *p_size = RAW_BUFFER_SIZE(8);
        break;
    case 12:
        *p_size = RAW_BUFFER_SIZE(12);
        break;
    case 24:
        *p_size = RAW_BUFFER_SIZE(24);
        break;

    default:
        ret = VL53L9_ERROR_INVALID_PARAM;
        break;
    }

    return ret;
}

/* private functions implementation ******************************************/

static _fsm_state_t _get_fsm_state(void *const p_dev) {
    _fsm_state_t state;
    (void)vl53l9_read8(p_dev, VL53L9_REGADDR_SYSTEM_FSM, (uint8_t *)&state);
    return state;
}

static int _wait_for_state(void *const p_dev, _fsm_state_t state, uint32_t timeout_ms) {
    int ret = VL53L9_ERROR_NONE;
    uint32_t elapsed_time_ms = 0;
    _fsm_state_t current_state;

    CHECK_NULL_PTR(p_dev);
    do {
        (void)vl53l9_wait_ms(p_dev, 1);
        elapsed_time_ms++;
        current_state = _get_fsm_state(p_dev);
    } while ((current_state != state) && (elapsed_time_ms < timeout_ms));

    if (elapsed_time_ms >= timeout_ms) {
        ret = VL53L9_ERROR_TIMEOUT;
    }

    return ret;
}

static int _write_cmd(void *const p_dev, _command_t cmd, uint32_t timeout_ms) {
    int ret;
    uint32_t elapsed_time_ms = 0;
    _command_t current_cmd;

    CHECK_NULL_PTR(p_dev);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_COMMAND, (uint8_t)cmd);
    CHECK_RET(ret);

    do {
        ret = vl53l9_read8(p_dev, VL53L9_REGADDR_COMMAND, (uint8_t *)&current_cmd);
        CHECK_RET(ret);
        if (current_cmd == COMMAND_NONE) { // avoid delay if not nessesary
            break;
        }
        (void)vl53l9_wait_ms(p_dev, 1);
        elapsed_time_ms++;
    } while ((elapsed_time_ms < timeout_ms));

    if (elapsed_time_ms >= timeout_ms) {
        return VL53L9_ERROR_TIMEOUT;
    }

    return ret;
}

static int _init_default_config(void *const p_dev) {
    int ret;
    uint16_t context;
    CHECK_NULL_PTR(p_dev);

    // short context
    context = (uint16_t)VL53L9_CONTEXT_SHORT;
    ret = vl53l9_write(p_dev, VL53L9_REGADDR_STREAM_VCSEL_CH0_STEP(1U, context), (uint8_t *)tx_channel_0_short,
                       sizeof(tx_channel_0_short));
    CHECK_RET(ret);
    ret = vl53l9_write(p_dev, VL53L9_REGADDR_STREAM_VCSEL_CH1_STEP(1U, context), (uint8_t *)tx_channel_1_short,
                       sizeof(tx_channel_1_short));
    CHECK_RET(ret);
    ret = vl53l9_write(p_dev, VL53L9_REGADDR_STREAM_BLANKING_STEP(1U, context), (uint8_t *)blanking_short,
                       sizeof(blanking_short));
    CHECK_RET(ret);
    ret = vl53l9_write(p_dev, VL53L9_REGADDR_STREAM_MAX_DITHERING_STEP(1U, context), (uint8_t *)dithering_short,
                       sizeof(dithering_short));
    CHECK_RET(ret);

    // long context
    context = (uint16_t)VL53L9_CONTEXT_LONG;
    ret = vl53l9_write(p_dev, VL53L9_REGADDR_STREAM_VCSEL_CH0_STEP(1U, context), (uint8_t *)tx_channel_0_long,
                       sizeof(tx_channel_0_long));
    CHECK_RET(ret);
    ret = vl53l9_write(p_dev, VL53L9_REGADDR_STREAM_VCSEL_CH1_STEP(1U, context), (uint8_t *)tx_channel_1_long,
                       sizeof(tx_channel_1_long));
    CHECK_RET(ret);
    ret = vl53l9_write(p_dev, VL53L9_REGADDR_STREAM_BLANKING_STEP(1U, context), (uint8_t *)blanking_long,
                       sizeof(blanking_long));
    CHECK_RET(ret);
    ret = vl53l9_write(p_dev, VL53L9_REGADDR_STREAM_MAX_DITHERING_STEP(1U, context), (uint8_t *)dithering_long,
                       sizeof(dithering_long));
    CHECK_RET(ret);

    // common
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_OUTPUT_IF, VL53L9_OUTPUT_I3C);
    CHECK_RET(ret);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_FRAME_SIGNALING_MODE, VL53L9_REGFIELD_INTR_OUTPUT_MODE);
    CHECK_RET(ret);

    // dss
    ret = vl53l9_write32(p_dev, VL53L9_REGADDR_DSS_SHORT_OR_TREE_LIMIT, 656);
    CHECK_RET(ret);
    ret = vl53l9_write32(p_dev, VL53L9_REGADDR_DSS_LONG_OR_TREE_LIMIT, 656);
    CHECK_RET(ret);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_DSS_SHORT_AMB_WEIGHT, 12);
    CHECK_RET(ret);
    ret = vl53l9_write8(p_dev, VL53L9_REGADDR_DSS_LONG_AMB_WEIGHT, 12);
    CHECK_RET(ret);

    // set cal_prog_offset values according to default context selection (short)
    ret = vl53l9_write16(p_dev, VL53L9_REGADDR_CAL_PROG_OFFSET_1TO5, (uint16_t)1);
    CHECK_RET(ret);
    ret = vl53l9_write16(p_dev, VL53L9_REGADDR_CAL_PROG_OFFSET_6, (uint16_t)5);
    CHECK_RET(ret);

    // custom analog block
    uint32_t data;
    ret = vl53l9_read32(p_dev, VL53L9_REGADDR_CAB_ALGO_SCALE, &data);
    CHECK_RET(ret);
    data &= ~VL53L9_REGFIELD_CAB_AMBIENT_ATTENUATION;
    data |= _FIELD_PREP(VL53L9_REGFIELD_CAB_AMBIENT_ATTENUATION, 4);
    ret = vl53l9_write32(p_dev, VL53L9_REGADDR_CAB_ALGO_SCALE, data);
    CHECK_RET(ret);

    // set cab_dist_scale according to default context selection (short)
    data = 0x01000800; // short 256 - long 2048
    return vl53l9_read32(p_dev, VL53L9_REGADDR_CAB_DIST_SCALE, &data);
}

static int _is_valid_csi_config(void *const p_dev) {
    int ret;
    vl53l9_context_t context;
    uint8_t binning = 0;
    _dss_mode_t dss_mode = DSS_DISABLE;
    uint16_t csi_width, csi_height;
    uint32_t frame_size_expected;

    // retrieve current configuration
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_CONTEXT_SELECTION, (uint8_t *)&context);
    CHECK_RET(ret);
    context = (vl53l9_context_t)_FIELD_GET(VL53L9_REGFIELD_CONTEXT_SELECTION, (uint8_t)context);
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_STANDBY_BINNING((uint16_t)context), &binning);
    CHECK_RET(ret);
    ret = vl53l9_read8(p_dev, VL53L9_REGADDR_STANDBY_DSS_MODE((uint16_t)context), (uint8_t *)&dss_mode);
    CHECK_RET(ret);
    ret = vl53l9_read16(p_dev, VL53L9_REGADDR_CSI2_FRAME_HEIGHT, (uint16_t *)&csi_height);
    CHECK_RET(ret);
    ret = vl53l9_read16(p_dev, VL53L9_REGADDR_CSI2_FRAME_WIDTH, (uint16_t *)&csi_width);
    CHECK_RET(ret);

    // make sure the csi2 frame size is large enough to contain the expected data
    unsigned int resolution = _get_raw_frame_resolution(binning);
    frame_size_expected = resolution * 6U;
    if (dss_mode != DSS_DISABLE) {
        frame_size_expected += resolution / 2U;
    }

    uint32_t csi_frame_size = (uint32_t)csi_height * csi_width;
    if (csi_frame_size < frame_size_expected) {
        return VL53L9_ERROR_INVALID_STATE;
    }
    return ret;
}

static int _write_crop_config(void *const p_dev, _crop_config_t *p_crop) {

    uint32_t data = 0;
    data |= _FIELD_PREP(VL53L9_REGFIELD_CROP_ENABLE, p_crop->enable);
    data |= _FIELD_PREP(VL53L9_REGFIELD_CROP_X_OFFSET, p_crop->x_offset);
    data |= _FIELD_PREP(VL53L9_REGFIELD_CROP_Y_OFFSET, p_crop->y_offset);
    data |= _FIELD_PREP(VL53L9_REGFIELD_CROP_X_SIZE, p_crop->x_size);
    data |= _FIELD_PREP(VL53L9_REGFIELD_CROP_Y_SIZE, p_crop->y_size);

    return vl53l9_write32(p_dev, VL53L9_REGADDR_CROP_PARAMS, data);
}

static unsigned int _get_raw_frame_resolution(uint8_t binning) {

    unsigned int ret = 0;

    switch (binning) {
    case 2:
        ret = FRAME_SIZE_BINNING_2;
        break;
    case 4:
        ret = FRAME_SIZE_BINNING_4;
        break;
    case 6:
        ret = FRAME_SIZE_BINNING_6;
        break;
    case 8:
        ret = FRAME_SIZE_BINNING_8;
        break;
    case 12:
        ret = FRAME_SIZE_BINNING_12;
        break;
    case 24:
        ret = FRAME_SIZE_BINNING_24;
        break;
    default:
        ret = 0;
        break;
    }

    return ret;
}
