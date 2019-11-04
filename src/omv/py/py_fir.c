/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * FIR Python module.
 */
#include <stdbool.h>
#include "soft_i2c.h"
#include "MLX90640_I2C_Driver.h"
#include "MLX90640_API.h"
#include "omv_boardconfig.h"
#include "framebuffer.h"
#include "sensor.h"
#include "py_helper.h"
#include "py_image.h"
#include "py_fir.h"

#define FIR_EEPROM_ADDR     0xA0
#define FIR_MODULE_ADDR     0xC0

#define FIR_READ_CMD        0x02
#define FIR_WR_CFG_REG      0x03
#define FIR_WR_TRIM_CMD     0x04

#define CAL_OSC_TRIM        0xF7

#define CAL_VTH_L           0xDA
#define CAL_VTH_H           0xDB

#define CAL_KT1_L           0xDC
#define CAL_KT1_H           0xDD
#define CAL_K_T1_SCALE      0xD2

#define CAL_KT2_L           0xDE
#define CAL_KT2_H           0xDF
#define CAL_K_T2_SCALE      0xD2

#define CAL_ACOMMON_L       0xD0
#define CAL_ACOMMON_H       0xD1

#define CAL_AI_SCALE        0xD9
#define CAL_AI_OFFSET       0x00

#define CAL_BI_SCALE        0xD9
#define CAL_BI_OFFSET       0x40

#define CAL_TGC             0xD8

#define CAL_EMISS_L         0xE4
#define CAL_EMISS_H         0xE5

#define CAL_KSTA_L          0xE6
#define CAL_KSTA_H          0xE7

#define CAL_A0_L            0xE0
#define CAL_A0_H            0xE1
#define CAL_A0_SCALE        0xE2
#define CAL_D_A_SCALE       0xE3
#define CAL_A_CP_OFFSET     0x80

#define CAL_ALPHA_CP_L      0xD6
#define CAL_ALPHA_CP_H      0xD7

#define CAL_KS_SCALE        0xC0
#define CAL_KS4_EE          0xC4

#define CAL_ACP_L           0xD3
#define CAL_ACP_H           0xD4

#define CAL_BCP             0xD5

#define MLX90640_ADDR       0x33

#define AMG8833_ADDR        0xD2

#define MAP(OldValue, OldMin, OldMax, NewMin, NewMax) \
    ({ __typeof__ (OldValue) _OldValue = (OldValue); \
       __typeof__ (OldMin) _OldMin = (OldMin); \
       __typeof__ (OldMax) _OldMax = (OldMax); \
       __typeof__ (NewMin) _NewMin = (NewMin); \
       __typeof__ (NewMax) _NewMax = (NewMax); \
       ((((_OldValue-_OldMin)*(_NewMax-_NewMin))+((_OldMax-OldMin)/2)) / \
       (_OldMax-OldMin))+_NewMin; })

// Grayscale to RGB565 conversion
extern const uint16_t rainbow_table[256];

// MLX variables
static float *a_ij = NULL;
static float *b_ij = NULL;
static float *alpha_ij = NULL;
static float v_th, k_t1, k_t2, tgc, emissivity, ksta, alpha_cp, ks4, a_cp, b_cp;

static uint8_t width = 0;
static uint8_t height = 0;
static enum { FIR_NONE, FIR_SHIELD, FIR_MLX90640, FIR_AMG8833 } type = FIR_NONE;
static uint8_t IR_refresh_rate = 0;
static uint8_t ADC_resolution = 0;

static void test_ack(int ret)
{
    PY_ASSERT_TRUE_MSG(ret == 0, "I2C Bus communication error - missing ACK!");
}

static float calculate_Ta() // ambient temp
{
    // Code to handle dealing with brown-out conditions.
    {
        uint16_t config_reg;
        test_ack(soft_i2c_write_bytes(FIR_MODULE_ADDR,
            (uint8_t [4]) {FIR_READ_CMD, 0x92, 0x00, 0x01}, 4, false));
        test_ack(soft_i2c_read_bytes(FIR_MODULE_ADDR,
            (uint8_t*) &config_reg, 2, true));

        if (!(config_reg & 0x0400)) { // brown out
            uint8_t eeprom;
            // Read the eeprom.
            test_ack(soft_i2c_write_bytes(FIR_EEPROM_ADDR,
                (uint8_t [1]){CAL_OSC_TRIM}, 1, false));
            test_ack(soft_i2c_read_bytes(FIR_EEPROM_ADDR,
                &eeprom, 1, true));

            // Write oscillator trimming value.
            soft_i2c_write_bytes(FIR_MODULE_ADDR,
                (uint8_t [5]){FIR_WR_TRIM_CMD,
                (uint8_t)(eeprom-0xAA), eeprom,
                (uint8_t)(0x00-0xAA), 0x00}, 5, true); // no ack here

            // Write device configuration value.
            uint8_t lsb = (ADC_resolution << 4) | IR_refresh_rate;
            // Normal Operation Mode - Continuous Measurment Mode
            uint8_t msb = 0x44;
            // ADC low reference enabled - EEPROM enabled
            // I2C FM+ enabled
            test_ack(soft_i2c_write_bytes(FIR_MODULE_ADDR,
                (uint8_t [5]){FIR_WR_CFG_REG,
                (uint8_t)(lsb-0x55), lsb,
                (uint8_t)(msb-0x55), msb}, 5, true));
        }
    }

    uint16_t ptat;
    test_ack(soft_i2c_write_bytes(FIR_MODULE_ADDR,
        (uint8_t [4]) {FIR_READ_CMD, 0x40, 0x00, 0x01}, 4, false));
    test_ack(soft_i2c_read_bytes(FIR_MODULE_ADDR,
        (uint8_t*) &ptat, 2, true));
    return (((-k_t1)+sqrtf((k_t1*k_t1)-(4*k_t2*(v_th-ptat))))/(2*k_t2))+25;
}

static void calculate_To(float Ta, float *To)
{
    fb_alloc_mark();
    int16_t *v_ir = fb_alloc(64 * sizeof(int16_t), FB_ALLOC_NO_HINT);
    // Read IR sensor result
    test_ack(soft_i2c_write_bytes(FIR_MODULE_ADDR,
        (uint8_t [4]){FIR_READ_CMD, 0x00, 0x01, 0x40}, 4, false));
    test_ack(soft_i2c_read_bytes(FIR_MODULE_ADDR,
        (uint8_t*) v_ir, 128, true));

    int16_t v_cp;
    // Read compensation pixel result
    test_ack(soft_i2c_write_bytes(FIR_MODULE_ADDR,
        (uint8_t [4]){FIR_READ_CMD, 0x41, 0x00, 0x01}, 4, false));
    test_ack(soft_i2c_read_bytes(FIR_MODULE_ADDR,
        (uint8_t*) &v_cp, 2, true));

    // Calculate Thermal Gradien Compensation (TGC)
    float v_ir_cp_off_comp = v_cp-(a_cp+(b_cp*(Ta-25)));

    // (Ta+273.15f)^4
    float Tak4 = (Ta+273.15f)*(Ta+273.15f)*(Ta+273.15f)*(Ta+273.15f);

    for (int i=0; i<64; i++) {
        // #1: Calculate Offset Compensation
        float v_ir_off_comp = v_ir[i]-(a_ij[i]+(b_ij[i]*(Ta-25)));

        // #2: Calculate Thermal Gradien Compensation (TGC)
        float v_ir_tgc_comp = v_ir_off_comp-(tgc*v_ir_cp_off_comp);

        // #3: Calculate Emissivity Compensation
        float v_ir_comp = v_ir_tgc_comp/emissivity;

        // #4: Calculate Sensitivity Compensation
        float alpha_comp_ij = (1+(ksta*(Ta-25)))*(alpha_ij[i]-(tgc*alpha_cp));

        // Ks4=0 for BAB and BAD sensors.
        // float sx = ks4*sqrtf(sqrtf((powf(alpha_comp_ij,3)*v_ir_comp)+(powf(alpha_comp_ij,4)*Ta4)));
        // To[i] = sqrtf(sqrtf((v_ir_comp/((alpha_comp_ij*(1-(ks4*273.15f)))+sx))+Ta4))-273.15f;
        To[i] = sqrtf(sqrtf((v_ir_comp/alpha_comp_ij)+Tak4))-273.15f;
    }
    fb_alloc_free_till_mark();
}

static mp_obj_t py_fir_deinit()
{
    switch (type) {
        case FIR_NONE:
            return mp_const_none;
        case FIR_SHIELD:
        case FIR_MLX90640:
        case FIR_AMG8833:
            soft_i2c_deinit();
            width = 0;
            height = 0;
            type = FIR_NONE;
            IR_refresh_rate = 0;
            ADC_resolution = 0;
            if (a_ij) {
                a_ij = NULL;
            }
            if (b_ij) {
                b_ij = NULL;
            }
            if (alpha_ij) {
                alpha_ij = NULL;
            }
            return mp_const_none;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_fir_deinit_obj, py_fir_deinit);

/*
Allows the refresh rate to be set in the range 1Hz and 512Hz, in powers of 2. (64Hz default)
The MLX90621 sensor is capable of a larger range but these extreme values are probably not useful with OpenMV.

Allows the ADC precision to be set in the range of 15 to 18 bits. (18 bit default).
Lower ADC precision allows a large maximum temperature within the scene without sensor overflow.
ADC 18-bits: max scene temperature ~450C, 15-bits: max scene temperature ~950C

calling:
 fir.init()
 fir.init(type=1, refresh=64, resolution=18)
*/
mp_obj_t py_fir_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    py_fir_deinit();
    switch (py_helper_keyword_int(n_args, args, 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_type), FIR_SHIELD)) {
        case FIR_NONE:
        {
            return mp_const_none;
        }
        case FIR_SHIELD:
        {
            width = 16;
            height = 4;
            type = FIR_SHIELD;
            soft_i2c_init();

            // parse refresh rate and ADC resolution
            IR_refresh_rate = py_helper_keyword_int(n_args, args, 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_refresh), 64);     // 64Hz
            ADC_resolution  = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_resolution), 18);  // 18-bits

            // sanitize values
            ADC_resolution  = ((ADC_resolution > 18) ? 18 : (ADC_resolution < 15) ? 15 : ADC_resolution) - 15;
            IR_refresh_rate = 14 - __CLZ(__RBIT((IR_refresh_rate > 512) ? 512 : (IR_refresh_rate < 1) ? 1 : IR_refresh_rate));

            a_ij = xalloc(64 * sizeof(*a_ij));
            b_ij = xalloc(64 * sizeof(*b_ij));
            alpha_ij = xalloc(64 * sizeof(*alpha_ij));

            fb_alloc_mark();
            uint8_t *eeprom = fb_alloc(256 * sizeof(uint8_t), FB_ALLOC_NO_HINT);
            // Read the whole eeprom.
            test_ack(soft_i2c_write_bytes(FIR_EEPROM_ADDR,
                (uint8_t [1]){0x00}, 1, false));
            test_ack(soft_i2c_read_bytes(FIR_EEPROM_ADDR,
                eeprom, 256, true));

            // Write oscillator trimming value.
            soft_i2c_write_bytes(FIR_MODULE_ADDR,
                (uint8_t [5]){FIR_WR_TRIM_CMD,
                (uint8_t)(eeprom[CAL_OSC_TRIM]-0xAA), eeprom[CAL_OSC_TRIM],
                (uint8_t)(0x00-0xAA), 0x00}, 5, true); // no ack here

            // Write device configuration value.
            // assignment of IR_refresh_rate and ADC_resolution now done above
            uint8_t lsb = (ADC_resolution << 4) | IR_refresh_rate;
            // Normal Operation Mode - Continuous Measurment Mode
            uint8_t msb = 0x44;
            // ADC low reference enabled - EEPROM enabled
            // I2C FM+ enabled
            test_ack(soft_i2c_write_bytes(FIR_MODULE_ADDR,
                (uint8_t [5]){FIR_WR_CFG_REG,
                (uint8_t)(lsb-0x55), lsb,
                (uint8_t)(msb-0x55), msb}, 5, true));

            v_th = ((int16_t)((eeprom[CAL_VTH_H]<<8)|eeprom[CAL_VTH_L])) /
                powf(2,3-ADC_resolution);

            uint8_t k_t1_scale = (uint8_t)((eeprom[CAL_K_T1_SCALE]&0xF0)>>4);
            k_t1 = ((int16_t)((eeprom[CAL_KT1_H]<<8)|eeprom[CAL_KT1_L])) /
                powf(2,k_t1_scale+(3-ADC_resolution));

            uint8_t k_t2_scale = (uint8_t)(eeprom[CAL_K_T2_SCALE]&0x0F);
            k_t2 = ((int16_t)((eeprom[CAL_KT2_H]<<8)|eeprom[CAL_KT2_L])) /
                powf(2,k_t2_scale+10+(3-ADC_resolution));

            int16_t a_common = (int16_t)((eeprom[CAL_ACOMMON_H]<<8)|eeprom[CAL_ACOMMON_L]);
            uint8_t a_i_scale = (uint8_t)((eeprom[CAL_AI_SCALE]&0xF0)>>4);
            uint8_t b_i_scale = (uint8_t)(eeprom[CAL_BI_SCALE]&0x0F);
            uint16_t alpha_0 = (uint16_t)((eeprom[CAL_A0_H]<<8)|eeprom[CAL_A0_L]);
            uint8_t alpha_0_scale = (uint8_t)eeprom[CAL_A0_SCALE];
            uint8_t delta_a_scale = (uint8_t)eeprom[CAL_D_A_SCALE];

            for (int i=0; i<64; i++) {
                // Pixel offset
                a_ij[i] = (a_common+(((uint8_t)eeprom[CAL_AI_OFFSET+i])*powf(2,a_i_scale))) /
                    powf(2,3-ADC_resolution);
                // Slope coefficient
                b_ij[i] = ((int8_t)eeprom[CAL_BI_OFFSET+i]) /
                    powf(2,b_i_scale+(3-ADC_resolution));
                // Sensitivity coefficient
                float t0 = alpha_0/powf(2,alpha_0_scale);
                float t1 = ((uint8_t)eeprom[CAL_A_CP_OFFSET+i])/powf(2,delta_a_scale);
                alpha_ij[i] = (t0+t1) /
                    powf(2,3-ADC_resolution);
            }

            tgc = ((int8_t)eeprom[CAL_TGC]) /
                32.0f;
            emissivity = ((uint16_t)((eeprom[CAL_EMISS_H]<<8)|eeprom[CAL_EMISS_L])) /
                32768.0f;
            ksta = ((int16_t)((eeprom[CAL_KSTA_H]<<8)|eeprom[CAL_KSTA_L])) /
                1048576.0f;
            alpha_cp = ((uint16_t)((eeprom[CAL_ALPHA_CP_H]<<8)|eeprom[CAL_ALPHA_CP_L])) /
                powf(2,alpha_0_scale+(3-ADC_resolution));

            uint8_t ks_scale = (uint8_t)(eeprom[CAL_KS_SCALE]&0x0F);
            ks4 = ((int8_t)eeprom[CAL_KS4_EE]) /
                powf(2,ks_scale+8);

            a_cp = ((int16_t)((eeprom[CAL_ACP_H]<<8)|eeprom[CAL_ACP_L])) /
                powf(2,3-ADC_resolution);
            b_cp = ((int8_t)eeprom[CAL_BCP]) /
                powf(2,b_i_scale+(3-ADC_resolution));

            fb_alloc_free_till_mark();
            return mp_const_none;
        }
        case FIR_MLX90640:
        {
            width = 32;
            height = 24;
            type = FIR_MLX90640;
            soft_i2c_init();

            // parse refresh rate and ADC resolution
            IR_refresh_rate = py_helper_keyword_int(n_args, args, 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_refresh), 32);     // 32Hz
            ADC_resolution  = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_resolution), 19);  // 19-bits

            // sanitize values
            ADC_resolution  = ((ADC_resolution > 19) ? 19 : (ADC_resolution < 16) ? 16 : ADC_resolution) - 16;
            IR_refresh_rate = __CLZ(__RBIT((IR_refresh_rate > 64) ? 64 : (IR_refresh_rate < 1) ? 1 : IR_refresh_rate)) + 1;

            alpha_ij = xalloc(sizeof(paramsMLX90640));

            MLX90640_I2CFreqSet(I2C_SPIN_DELAY);
            MLX90640_I2CInit();

            int error = 0;

            error |= MLX90640_SetResolution(MLX90640_ADDR, ADC_resolution);
            error |= MLX90640_SetRefreshRate(MLX90640_ADDR, IR_refresh_rate);

            fb_alloc_mark();
            uint16_t *eeprom = fb_alloc(832 * sizeof(uint16_t), FB_ALLOC_NO_HINT);
            error |= MLX90640_DumpEE(MLX90640_ADDR, eeprom);
            error |= MLX90640_ExtractParameters(eeprom, (paramsMLX90640 *) alpha_ij);

            PY_ASSERT_TRUE_MSG(error == 0, "Failed to init the MLX90640!");

            fb_alloc_free_till_mark();
            return mp_const_none;
        }
        case FIR_AMG8833:
        {
            width = 8;
            height = 8;
            type = FIR_AMG8833;
            soft_i2c_init();

            IR_refresh_rate = 10;
            ADC_resolution  = 12;

            test_ack(soft_i2c_write_bytes(AMG8833_ADDR, (uint8_t [2]){0x01, 0x3F}, 2, true));

            return mp_const_none;
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_fir_init_obj, 0, py_fir_init);

static mp_obj_t py_fir_width()
{
    if (type == FIR_NONE) return mp_const_none;
    return mp_obj_new_int(width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_fir_width_obj, py_fir_width);

static mp_obj_t py_fir_height()
{
    if (type == FIR_NONE) return mp_const_none;
    return mp_obj_new_int(height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_fir_height_obj, py_fir_height);

static mp_obj_t py_fir_type()
{
    if (type == FIR_NONE) return mp_const_none;
    return mp_obj_new_int(type);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_fir_type_obj, py_fir_type);

static mp_obj_t py_fir_refresh()
{
    const int mlx_90621_refresh_rates[16] = {512, 512, 512, 512, 512, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1, 0};
    const int mlx_90640_refresh_rates[8] = {0, 1, 2, 4, 8, 16, 32, 64};
    if (type == FIR_NONE) return mp_const_none;
    if (type == FIR_SHIELD) return mp_obj_new_int(mlx_90621_refresh_rates[IR_refresh_rate]);
    if (type == FIR_MLX90640) return mp_obj_new_int(mlx_90640_refresh_rates[IR_refresh_rate]);
    if (type == FIR_AMG8833) return mp_obj_new_int(IR_refresh_rate);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_fir_refresh_obj, py_fir_refresh);

static mp_obj_t py_fir_resolution()
{
    if (type == FIR_NONE) return mp_const_none;
    if (type == FIR_SHIELD) return mp_obj_new_int(ADC_resolution + 15);
    if (type == FIR_MLX90640) return mp_obj_new_int(ADC_resolution + 16);
    if (type == FIR_AMG8833) return mp_obj_new_int(ADC_resolution);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_fir_resolution_obj, py_fir_resolution);

mp_obj_t py_fir_read_ta()
{
    switch(type) {
        case FIR_NONE: return mp_const_none;
        case FIR_SHIELD: return mp_obj_new_float(calculate_Ta());
        case FIR_MLX90640:
        {
            fb_alloc_mark();
            uint16_t *data = fb_alloc(834 * sizeof(uint16_t), FB_ALLOC_NO_HINT);
            PY_ASSERT_TRUE_MSG(MLX90640_GetFrameData(MLX90640_ADDR, data) >= 0,
                               "Failed to read the MLX90640 sensor data!");
            mp_obj_t result = mp_obj_new_float(MLX90640_GetTa(data, (paramsMLX90640 *) alpha_ij));
            fb_alloc_free_till_mark();
            return result;
        }
        case FIR_AMG8833:
        {
            test_ack(soft_i2c_write_bytes(AMG8833_ADDR, (uint8_t [1]){0x0E}, 1, true));
            int16_t temp;
            test_ack(soft_i2c_read_bytes(AMG8833_ADDR, (uint8_t *) &temp, 2, true));
            if ((temp >> 11) & 1) temp |= 1 << 15;
            temp &= 0x87FF;
            return mp_obj_new_float(temp * 0.0625);
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_fir_read_ta_obj, py_fir_read_ta);

mp_obj_t py_fir_read_ir()
{
    switch(type) {
        case FIR_NONE: return mp_const_none;
        case FIR_SHIELD:
        {
            fb_alloc_mark();
            float *To = fb_alloc(64 * sizeof(float), FB_ALLOC_NO_HINT), *To_rot = fb_alloc(64 * sizeof(float), FB_ALLOC_NO_HINT);
            float Ta = calculate_Ta();
            float min = FLT_MAX, max = FLT_MIN;

            // Calculate object temperatures
            calculate_To(Ta, To_rot);

            // Rotate temperatures array (sensor memory is read column wise).
            for (int x=15, r=0; x>=0; x--) {
                for (int y=0; y<4; y++) {
                    float temp = To[y*16 + x] = To_rot[r++];
                    min = IM_MIN(min, temp);
                    max = IM_MAX(max, temp);
                }
            }

            mp_obj_t tuple[4];
            tuple[0] = mp_obj_new_float(Ta);
            tuple[1] = mp_obj_new_list(0, NULL);
            tuple[2] = mp_obj_new_float(min);
            tuple[3] = mp_obj_new_float(max);

            for (int i=0; i<64; i++) {
                mp_obj_list_append(tuple[1], mp_obj_new_float(To[i]));
            }

            fb_alloc_free_till_mark();
            return mp_obj_new_tuple(4, tuple);
        }
        case FIR_MLX90640:
        {
            fb_alloc_mark();
            uint16_t *data = fb_alloc(834 * sizeof(uint16_t), FB_ALLOC_NO_HINT);
            // Calculate 1st sub-frame...
            PY_ASSERT_TRUE_MSG(MLX90640_GetFrameData(MLX90640_ADDR, data) >= 0,
                               "Failed to read the MLX90640 sensor data!");
            float Ta = MLX90640_GetTa(data, (paramsMLX90640 *) alpha_ij);
            float *To = fb_alloc0(768 * sizeof(float), FB_ALLOC_NO_HINT);
            MLX90640_CalculateTo(data, (paramsMLX90640 *) alpha_ij, 0.95, Ta - 8, To);
            // Calculate 2nd sub-frame...
            PY_ASSERT_TRUE_MSG(MLX90640_GetFrameData(MLX90640_ADDR, data) >= 0,
                               "Failed to read the MLX90640 sensor data!");
            Ta = MLX90640_GetTa(data, (paramsMLX90640 *) alpha_ij);
            MLX90640_CalculateTo(data, (paramsMLX90640 *) alpha_ij, 0.95, Ta - 8, To);
            float min = FLT_MAX, max = FLT_MIN;

            for (int i=0; i<768; i++) {
                min = IM_MIN(min, To[i]);
                max = IM_MAX(max, To[i]);
            }

            mp_obj_t tuple[4];
            tuple[0] = mp_obj_new_float(Ta);
            tuple[1] = mp_obj_new_list(0, NULL);
            tuple[2] = mp_obj_new_float(min);
            tuple[3] = mp_obj_new_float(max);

            for (int i=0; i<24; i++) {
                for (int j=0; j<32; j++) {
                    mp_obj_list_append(tuple[1], mp_obj_new_float(To[(i*32)+(31-j)]));
                }
            }

            fb_alloc_free_till_mark();
            return mp_obj_new_tuple(4, tuple);
        }
        case FIR_AMG8833:
        {
            test_ack(soft_i2c_write_bytes(AMG8833_ADDR, (uint8_t [1]){0x0E}, 1, true));
            int16_t temp;
            test_ack(soft_i2c_read_bytes(AMG8833_ADDR, (uint8_t *) &temp, 2, true));
            if ((temp >> 11) & 1) temp |= 1 << 15;
            temp &= 0x87FF;
            float Ta = temp * 0.0625;

            test_ack(soft_i2c_write_bytes(AMG8833_ADDR, (uint8_t [1]){0x80}, 1, true));
            fb_alloc_mark();
            int16_t *data = fb_alloc(64 * sizeof(int16_t), FB_ALLOC_NO_HINT);
            test_ack(soft_i2c_read_bytes(AMG8833_ADDR, (uint8_t *) data, 128, true));
            float To[64], min = FLT_MAX, max = FLT_MIN;
            for (int i = 0; i < 64; i++) {
                if ((data[i] >> 11) & 1) data[i] |= 1 << 15;
                data[i] &= 0x87FF;
                To[i] = data[i] * 0.25;
                min = IM_MIN(min, To[i]);
                max = IM_MAX(max, To[i]);
            }

            mp_obj_t tuple[4];
            tuple[0] = mp_obj_new_float(Ta);
            tuple[1] = mp_obj_new_list(0, NULL);
            tuple[2] = mp_obj_new_float(min);
            tuple[3] = mp_obj_new_float(max);

            for (int i=0; i<8; i++) {
                for (int j=0; j<8; j++) {
                    mp_obj_list_append(tuple[1], mp_obj_new_float(To[((7-j)*8)+i]));
                }
            }

            fb_alloc_free_till_mark();
            return mp_obj_new_tuple(4, tuple);
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_fir_read_ir_obj,  py_fir_read_ir);

mp_obj_t py_fir_draw_ta(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    if (type == FIR_NONE) return mp_const_none;
    image_t *arg_img = py_helper_arg_to_image_mutable(args[0]);

    float Ta = mp_obj_get_float(args[1]);
    float min = -17.7778, max = 37.7778; // 0F to 100F

    int alpha = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_alpha), 128);
    PY_ASSERT_TRUE_MSG((0 <= alpha) && (alpha <= 256), "Error: 0 <= alpha <= 256!");

    mp_obj_t scale_obj = py_helper_keyword_object(n_args, args, 3, kw_args, 
                                                  MP_OBJ_NEW_QSTR(MP_QSTR_scale));
    if (scale_obj) {
        mp_obj_t *arg_scale;
        mp_obj_get_array_fixed_n(scale_obj, 2, &arg_scale);
        min = mp_obj_get_float(arg_scale[0]);
        max = mp_obj_get_float(arg_scale[1]);
    }

    uint8_t gs_ta = IM_MIN(IM_MAX(MAP(Ta, min, max, 0, 255), 0), 255);
    uint16_t r_ta = COLOR_RGB565_TO_R5(rainbow_table[gs_ta]);
    uint16_t g_ta = COLOR_RGB565_TO_G6(rainbow_table[gs_ta]);
    uint16_t b_ta = COLOR_RGB565_TO_B5(rainbow_table[gs_ta]);
    uint32_t va = __PKHBT((256-alpha), alpha, 16);
    for (int y=0; y<arg_img->h; y++) {
        for (int x=0; x<arg_img->w; x++) {
            switch (arg_img->bpp) {
                case IMAGE_BPP_BINARY:
                {
                    uint8_t pixel = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL(arg_img, x, y));
                    uint32_t vgs = __PKHBT(pixel, gs_ta, 16);
                    uint32_t gs = __SMUAD(va, vgs)>>8;
                    IMAGE_PUT_BINARY_PIXEL(arg_img, x, y, COLOR_GRAYSCALE_TO_BINARY(gs));
                    break;
                }
                case IMAGE_BPP_GRAYSCALE:
                {
                    uint8_t pixel = IMAGE_GET_GRAYSCALE_PIXEL(arg_img, x, y);
                    uint32_t vgs = __PKHBT(pixel, gs_ta, 16);
                    uint32_t gs = __SMUAD(va, vgs)>>8;
                    IMAGE_PUT_GRAYSCALE_PIXEL(arg_img, x, y, gs);
                    break;
                }
                case IMAGE_BPP_RGB565: {
                    uint16_t pixel = IMAGE_GET_RGB565_PIXEL(arg_img, x, y);
                    uint32_t vr = __PKHBT(COLOR_RGB565_TO_R5(pixel), r_ta, 16);
                    uint32_t vg = __PKHBT(COLOR_RGB565_TO_G6(pixel), g_ta, 16);
                    uint32_t vb = __PKHBT(COLOR_RGB565_TO_B5(pixel), b_ta, 16);
                    uint32_t r = __SMUAD(va, vr)>>8;
                    uint32_t g = __SMUAD(va, vg)>>8;
                    uint32_t b = __SMUAD(va, vb)>>8;
                    IMAGE_PUT_RGB565_PIXEL(arg_img, x, y, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                    break;
                }
                default: break;
            }
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_fir_draw_ta_obj, 2, py_fir_draw_ta);

mp_obj_t py_fir_draw_ir(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    if (type == FIR_NONE) return mp_const_none;
    image_t *arg_img = py_helper_arg_to_image_mutable(args[0]);

    mp_obj_t *arg_To;
    mp_obj_get_array_fixed_n(args[1], width*height, &arg_To);

    fb_alloc_mark();
    float *To = fb_alloc(width*height * sizeof(float), FB_ALLOC_NO_HINT), min = FLT_MAX, max = FLT_MIN;
    for (int i=0; i<width*height; i++) {
        float temp = To[i] = mp_obj_get_float(arg_To[i]);
        min = IM_MIN(min, temp);
        max = IM_MAX(max, temp);
    }

    int alpha = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_alpha), 128);
    PY_ASSERT_TRUE_MSG((0 <= alpha) && (alpha <= 256), "Error: 0 <= alpha <= 256!");

    mp_obj_t scale_obj = py_helper_keyword_object(n_args, args, 3, kw_args,
                                                  MP_OBJ_NEW_QSTR(MP_QSTR_scale));
    if (scale_obj) {
        mp_obj_t *arg_scale;
        mp_obj_get_array_fixed_n(scale_obj, 2, &arg_scale);
        min = mp_obj_get_float(arg_scale[0]);
        max = mp_obj_get_float(arg_scale[1]);
    }

    int x_scale = arg_img->w / width, y_scale = arg_img->h / height;
    int scale = IM_MIN(x_scale, y_scale);
    int x_offset = (arg_img->w - (width * scale)) / 2;
    int y_offset = (arg_img->h - (height * scale)) / 2;
    uint32_t va = __PKHBT((256-alpha), alpha, 16);
    for (int y=y_offset; y<y_offset+(height*scale); y++) {
        for (int x=x_offset; x<x_offset+(width*scale); x++) {
            int index = (((y-y_offset)/scale)*width)+((x-x_offset)/scale);
            uint8_t gs_to = IM_MIN(IM_MAX(MAP(To[index], min, max, 0, 255), 0), 255);
            uint16_t r_to = COLOR_RGB565_TO_R5(rainbow_table[gs_to]);
            uint16_t g_to = COLOR_RGB565_TO_G6(rainbow_table[gs_to]);
            uint16_t b_to = COLOR_RGB565_TO_B5(rainbow_table[gs_to]);
            switch (arg_img->bpp) {
                case IMAGE_BPP_BINARY:
                {
                    uint8_t pixel = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL(arg_img, x, y));
                    uint32_t vgs = __PKHBT(pixel, gs_to, 16);
                    uint32_t gs = __SMUAD(va, vgs)>>8;
                    IMAGE_PUT_BINARY_PIXEL(arg_img, x, y, COLOR_GRAYSCALE_TO_BINARY(gs));
                    break;
                }
                case IMAGE_BPP_GRAYSCALE:
                {
                    uint8_t pixel = IMAGE_GET_GRAYSCALE_PIXEL(arg_img, x, y);
                    uint32_t vgs = __PKHBT(pixel, gs_to, 16);
                    uint32_t gs = __SMUAD(va, vgs)>>8;
                    IMAGE_PUT_GRAYSCALE_PIXEL(arg_img, x, y, gs);
                    break;
                }
                case IMAGE_BPP_RGB565: {
                    uint16_t pixel = IMAGE_GET_RGB565_PIXEL(arg_img, x, y);
                    uint32_t vr = __PKHBT(COLOR_RGB565_TO_R5(pixel), r_to, 16);
                    uint32_t vg = __PKHBT(COLOR_RGB565_TO_G6(pixel), g_to, 16);
                    uint32_t vb = __PKHBT(COLOR_RGB565_TO_B5(pixel), b_to, 16);
                    uint32_t r = __SMUAD(va, vr)>>8;
                    uint32_t g = __SMUAD(va, vg)>>8;
                    uint32_t b = __SMUAD(va, vb)>>8;
                    IMAGE_PUT_RGB565_PIXEL(arg_img, x, y, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                    break;
                }
                default: break;
            }
        }
    }
    fb_alloc_free_till_mark();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_fir_draw_ir_obj, 2, py_fir_draw_ir);

mp_obj_t py_fir_snapshot(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    if (type == FIR_NONE) return mp_const_none;
    mp_obj_t ir = py_fir_read_ir();
    size_t len;
    mp_obj_t *items;
    mp_obj_tuple_get(ir, &len, &items);

    int pixformat = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_pixformat), PIXFORMAT_RGB565);
    PY_ASSERT_TRUE_MSG((pixformat == PIXFORMAT_GRAYSCALE) || (pixformat == PIXFORMAT_RGB565), "Invalid Pixformat!");

    mp_obj_t copy_to_fb_obj = py_helper_keyword_object(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_copy_to_fb));
    bool copy_to_fb = false;
    image_t *arg_other = NULL;

    if (copy_to_fb_obj) {
        if (mp_obj_is_integer(copy_to_fb_obj)) {
            copy_to_fb = mp_obj_get_int(copy_to_fb_obj);
        } else {
            arg_other = py_helper_arg_to_image_mutable(copy_to_fb_obj);
        }
    }

    if (copy_to_fb) {
        fb_update_jpeg_buffer();
    }

    image_t image;
    image.w = width;
    image.h = height;
    image.bpp = (pixformat == PIXFORMAT_RGB565) ? IMAGE_BPP_RGB565 : IMAGE_BPP_GRAYSCALE;
    image.data = NULL;

    if (copy_to_fb) {
        MAIN_FB()->w = 0;
        MAIN_FB()->h = 0;
        MAIN_FB()->bpp = 0;
        PY_ASSERT_TRUE_MSG((image_size(&image) <= fb_avail()), "The new image won't fit in the main frame buffer!");
        MAIN_FB()->w = image.w;
        MAIN_FB()->h = image.h;
        MAIN_FB()->bpp = image.bpp;
        image.data = MAIN_FB()->pixels;
    } else if (arg_other) {
        PY_ASSERT_TRUE_MSG((image_size(&image) <= image_size(arg_other)), "The new image won't fit in the target frame buffer!");
        image.data = arg_other->data;
    } else {
        image.data = xalloc(image_size(&image));
    }

    // Zero the image we are about to draw on.
    memset(image.data, 0, image_size(&image));

    if (MAIN_FB()->pixels == image.data) {
        MAIN_FB()->w = image.w;
        MAIN_FB()->h = image.h;
        MAIN_FB()->bpp = image.bpp;
    }

    if (arg_other) {
        arg_other->w = image.w;
        arg_other->h = image.h;
        arg_other->bpp = image.bpp;
    }
 
    mp_obj_t snapshot = py_image_from_struct(&image);

    mp_obj_t *new_args = xalloc((2 + n_args) * sizeof(mp_obj_t));
    new_args[0] = snapshot;
    new_args[1] = items[1]; // ir array

    for (uint i = 0; i < n_args; i++) {
        new_args[2+i] = args[i];
    }

    py_fir_draw_ir(2 + n_args, new_args, kw_args);
    gc_collect();

    return snapshot;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_fir_snapshot_obj, 0, py_fir_snapshot);

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_fir) },
    { MP_ROM_QSTR(MP_QSTR_FIR_NONE),        MP_ROM_INT(FIR_NONE) },
    { MP_ROM_QSTR(MP_QSTR_FIR_SHIELD),      MP_ROM_INT(FIR_SHIELD) },
    { MP_ROM_QSTR(MP_QSTR_FIR_MLX90620),    MP_ROM_INT(FIR_SHIELD) }, // == FIR_SHIELD
    { MP_ROM_QSTR(MP_QSTR_FIR_MLX90621),    MP_ROM_INT(FIR_SHIELD) }, // == FIR_SHIELD
    { MP_ROM_QSTR(MP_QSTR_FIR_MLX90640),    MP_ROM_INT(FIR_MLX90640) },
    { MP_ROM_QSTR(MP_QSTR_FIR_AMG8833),     MP_ROM_INT(FIR_AMG8833) },
    { MP_ROM_QSTR(MP_QSTR_init),            MP_ROM_PTR(&py_fir_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit),          MP_ROM_PTR(&py_fir_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_width),           MP_ROM_PTR(&py_fir_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height),          MP_ROM_PTR(&py_fir_height_obj) },
    { MP_ROM_QSTR(MP_QSTR_type),            MP_ROM_PTR(&py_fir_type_obj) },
    { MP_ROM_QSTR(MP_QSTR_refresh),         MP_ROM_PTR(&py_fir_refresh_obj) },
    { MP_ROM_QSTR(MP_QSTR_resolution),      MP_ROM_PTR(&py_fir_resolution_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_ta),         MP_ROM_PTR(&py_fir_read_ta_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_ir),         MP_ROM_PTR(&py_fir_read_ir_obj) },
    { MP_ROM_QSTR(MP_QSTR_draw_ta),         MP_ROM_PTR(&py_fir_draw_ta_obj) },
    { MP_ROM_QSTR(MP_QSTR_draw_ir),         MP_ROM_PTR(&py_fir_draw_ir_obj) },
    { MP_ROM_QSTR(MP_QSTR_snapshot),        MP_ROM_PTR(&py_fir_snapshot_obj) }
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t fir_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

void py_fir_init0()
{
    py_fir_deinit();
}
