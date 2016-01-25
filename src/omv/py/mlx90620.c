/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MLX90621 Python module.
 *
 */
#include <mp.h>
#include <stdbool.h>
#include <float.h>
#include "systick.h"
#include "soft_i2c.h"
#include "mdefs.h"
#include "math.h"
#include "fmath.h"
#include "xalloc.h"
#include "py_image.h"
#include "mlx90620.h"

#ifdef OPENMV2

#define MLX_SLAVE_ADDR      (0xC0)
#define MLX_EEPROM_ADDR     (0xA0)
#define OSC_TRIM_OFFSET     (0xF7)
#define REG_EEPROM_DATA     (0x00)

// MLX commands
#define WRITE_OSC_TRIM      (0x04)
#define SET_CONFIG_DATA     (0x03)
#define MLX_READ_REG        (0x02)

//Common sensitivity coefficients
#define CAL_ACOMMON_L       0xD0
#define CAL_ACOMMON_H       0xD1
#define KT_SCALE            0xD2
#define CAL_ACP_L           0xD3
#define CAL_ACP_H           0xD4
#define CAL_BCP             0xD5
#define CAL_ALPHACP_L       0xD6
#define CAL_ALPHACP_H       0xD7
#define CAL_TGC             0xD8
#define CAL_AI_SCALE        0xD9
#define CAL_BI_SCALE        0xD9
#define VTH_L               0xDA
#define VTH_H               0xDB
#define KT1_L               0xDC
#define KT1_H               0xDD
#define KT2_L               0xDE
#define KT2_H               0xDF
#define CAL_A0_L            0xE0
#define CAL_A0_H            0xE1
#define CAL_A0_SCALE        0xE2
#define CAL_DELTA_A_SCALE   0xE3
#define CAL_EMIS_L          0xE4
#define CAL_EMIS_H          0xE5
#define CAL_KSTA_L          0xE6
#define CAL_KSTA_H          0xE7
#define CAL_KS_SCALE        0xC0
#define CAL_KS4_EE          0xC4
#define TA0                 (25)

#define MAP(OldValue, OldMin, OldMax, NewMin, NewMax)\
    (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin

#define TIMEOUT             (10000)

enum image_type {
    RAINBOW,
    GRAYSCALE,
};

enum ir_refresh_rate {
    IR_REFRESH_512HZ = 5,
    IR_REFRESH_256HZ,
    IR_REFRESH_128HZ,
    IR_REFRESH_64HZ,
    IR_REFRESH_32HZ,
    IR_REFRESH_16HZ,
    IR_REFRESH_8HZ,
};

/* Temp [0..99] to rainbow lookup */
extern const uint16_t rainbow_table[256];

// These are constants calculated from
// the calibration data stored in EEPROM
float a_ij[64], b_ij[64];
float v_th, k_t1, k_t2, ks4, ksta, tgc;
float emissivity, alpha_cp, a_cp, b_cp;

// Alpha(i,j) table
static const float alpha_ij[64] = {
    6.0415914049e-08f, 6.6935172072e-08f, 6.7866494646e-08f, 6.2045728555e-08f, 6.6003849497e-08f, 7.4618583312e-08f, 7.5084244600e-08f, 6.9729139796e-08f, 
    7.2057446232e-08f, 8.1370671978e-08f, 8.3466147771e-08f, 7.6946889749e-08f, 7.7179720392e-08f, 8.8821252575e-08f, 9.1382389655e-08f, 8.1603502622e-08f, 
    8.1603502622e-08f, 9.3477865448e-08f, 9.5806171885e-08f, 8.8588421931e-08f, 8.5328792920e-08f, 9.5806171885e-08f, 9.9065800896e-08f, 9.4409188023e-08f, 
    8.8355591288e-08f, 1.0069561540e-07f, 1.0255826055e-07f, 9.5806171885e-08f, 8.9286913862e-08f, 1.0139410733e-07f, 1.0628355085e-07f, 9.8367308965e-08f, 
    8.8588421931e-08f, 1.0139410733e-07f, 1.0581788956e-07f, 9.8134478321e-08f, 8.9054083219e-08f, 1.0069561540e-07f, 1.0535222827e-07f, 9.6504663816e-08f, 
    8.6958607426e-08f, 9.9531462183e-08f, 1.0325675248e-07f, 9.5340510597e-08f, 8.6958607426e-08f, 9.6970325103e-08f, 1.0139410733e-07f, 9.5340510597e-08f, 
    8.3000486484e-08f, 9.4409188023e-08f, 9.7668817034e-08f, 8.9054083219e-08f, 7.9042365542e-08f, 9.0683897724e-08f, 9.2546542874e-08f, 8.4863131633e-08f, 
    7.5084244600e-08f, 8.3233317127e-08f, 8.7424268713e-08f, 8.0672180047e-08f, 6.7168002715e-08f, 7.5782736531e-08f, 7.9508026829e-08f, 7.5084244600e-08f,
};

static float calculate_Ta()
{
    uint16_t ptat=0;
    uint8_t cmd_buf[4]={MLX_READ_REG, 0x40, 0x00, 0x01};
    soft_i2c_write_bytes(MLX_SLAVE_ADDR, cmd_buf, sizeof(cmd_buf), false);
    soft_i2c_read_bytes(MLX_SLAVE_ADDR, (uint8_t*)&ptat, 2, true);
    return (-k_t1 + fast_sqrtf(k_t1 * k_t1 - (4 * k_t2 * (v_th - ptat)))) / (2 * k_t2) + TA0;
}

static void calculate_To(float Ta, float *To)
{
    float v_ir_comp;
    float v_ir_off_comp;
    float v_ir_tgc_comp;

    int16_t v_cp;
    uint8_t cmd_buf[4];
    int16_t ir_data[64];

    // (T+273.15f)^4
    float Ta4 = (Ta + 273.15f) * (Ta + 273.15f) * (Ta + 273.15f) * (Ta + 273.15f);

    // Read IR data
    memcpy(cmd_buf, (uint8_t [4]){MLX_READ_REG, 0x00, 0x01, 0x40}, sizeof(cmd_buf)); //read 64*2 bytes
    soft_i2c_write_bytes(MLX_SLAVE_ADDR, cmd_buf, sizeof(cmd_buf), false);
    soft_i2c_read_bytes(MLX_SLAVE_ADDR, (uint8_t*)ir_data, 128, true);

    // Read compensation data
    memcpy(cmd_buf, (uint8_t [4]){MLX_READ_REG, 0x41, 0x00, 0x01}, sizeof(cmd_buf));
    soft_i2c_write_bytes(MLX_SLAVE_ADDR, cmd_buf, sizeof(cmd_buf), false);
    soft_i2c_read_bytes(MLX_SLAVE_ADDR, (uint8_t*)&v_cp, 2, true);

    //Calculate the offset compensation for the one compensation pixel
    //This is a constant in the TO calculation, so calculate it here.
    float v_ir_cp_off_comp = (float)v_cp - ((a_cp + b_cp) * (Ta - TA0));

    for (int i=0; i<64; i++) {
        //#1: Calculate Offset Compensation
        v_ir_off_comp = ir_data[i] - (a_ij[i] + b_ij[i] * (Ta - TA0));

        //#2: Calculate Thermal Gradien Compensation (TGC)
        v_ir_tgc_comp = v_ir_off_comp - tgc * v_ir_cp_off_comp;

        //#3: Calculate Emissivity Compensation
		v_ir_comp = v_ir_tgc_comp / emissivity;

        float alpha_comp_ij = (1 + ksta * (Ta - TA0)) * (alpha_ij[i] - tgc * alpha_cp);
        // Ks4=0 for 40 and 60 FOV sensors.
        //float sx = ks4 * sqrtf(sqrtf(powf(alpha_comp_ij, 3) * v_ir_comp  + powf(alpha_comp_ij, 4) * Ta4));
        //To[i] = sqrtf(sqrtf((v_ir_comp/alpha_comp_ij * (1-ks4*273.15f)+sx) + Ta4)) - 273.15f;
        To[i] = sqrtf(sqrtf(v_ir_comp/alpha_comp_ij + Ta4)) - 273.15f;
        //printf ("%f, ", (double) To[i]);
    }
    //printf ("\n\n");
}

mp_obj_t mlx90620_read_ta()
{
    return mp_obj_new_float(calculate_Ta());
    
}

mp_obj_t mlx90620_read_ir(mp_obj_t type_obj, mp_obj_t t_obj, mp_obj_t p_obj)
{
    float Ta, To[64];
    float To_rot[64];
    float max_To = FLT_MIN;
    float min_To = FLT_MAX;
    
    image_t *img;
    enum image_type img_type;

    // Alloc image
    img = xalloc(sizeof(*img));
    img->w = 16;
    img->h = 4;

    // Read image type
    img_type = mp_obj_get_int(type_obj);

    // Read params
    float t = mp_obj_get_float(t_obj);
    float p = mp_obj_get_float(p_obj);

    switch (img_type) {
        case GRAYSCALE:
            img->bpp = 1;
            img->pixels = xalloc(img->w*img->h*1);
            break;
        case RAINBOW:
            img->bpp = 2;
            img->pixels = xalloc(img->w*img->h*2);
            break;
    }

    // Calculate ambient temperature
    Ta = calculate_Ta();

    // Calculate object temperatures
    calculate_To(Ta, To);

    // Copy temperatures
    float *To_p = To_rot;
    memcpy(To_p, To, sizeof(To));

    // Rotate object temperatures.
    // Note: sensor memory is read column wise.
    for (int x=15; x>=0; x--) {
        for (int y=0; y<4; y++) {
            float to = To[x+y*16] = *To_p++;
            // Find min and max object temperature.
            if (to > max_To) {
                max_To = to;
            } else if (to < min_To) {
                min_To = to;
            }
        }
    }

    // Map object temperature to rainbow or grayscale
    for (int i=0; i<64; i++) {
        int to= atanf((To[i]/t)*tanf(p*(3.1415f/2)))*(512/3.1415f);

        switch (img_type) {
            case GRAYSCALE:
                img->pixels[i] = to;
                break;
            case RAINBOW:
                ((uint16_t*)img->pixels)[i] = rainbow_table[to];
                break;
        }
    }

    mp_obj_t ret_obj[] = {
        mp_obj_new_float(Ta),
        mp_obj_new_float(min_To),
        mp_obj_new_float(max_To),
        py_image_from_struct(img),
    };

    return mp_obj_new_tuple(sizeof(ret_obj)/sizeof(mp_obj_t), ret_obj);
}

mp_obj_t mlx90620_read_raw()
{
    float Ta, To[64], To_rot[64];
    mp_obj_t t_list = mp_obj_new_list(0, NULL);

    // Calculate ambient temperature
    Ta = calculate_Ta();

    // Calculate object temperatures
    calculate_To(Ta, To);

    // Copy temperatures
    float *To_p = To_rot;
    memcpy(To_p, To, sizeof(To));

    // Rotate object temperatures.
    // Note: sensor memory is read column wise.
    for (int x=15; x>=0; x--) {
        for (int y=0; y<4; y++) {
            To[x+y*16] = *To_p++;
        }
    }

    for (int i=0; i<64; i++) {
        mp_obj_list_append(t_list, mp_obj_new_float(To[i]));
    }

    return t_list;
}

mp_obj_t mlx90620_init(mp_obj_t refresh_rate)
{
    uint8_t cmd_buf[5];

    // EEPROM data for quick lookup
    uint8_t EEPROM_DATA[256];

    // Init I2C
    soft_i2c_init();

    // Read EEPROM data
    cmd_buf[0]=REG_EEPROM_DATA;
    soft_i2c_write_bytes(MLX_EEPROM_ADDR, cmd_buf, 1, false);
    soft_i2c_read_bytes(MLX_EEPROM_ADDR, EEPROM_DATA, 256, true);
    
    // Write oscillator trimming value
    uint8_t trim = EEPROM_DATA[OSC_TRIM_OFFSET];
    memcpy(cmd_buf, (uint8_t [5]){WRITE_OSC_TRIM, (uint8_t)(trim-0xAA), trim, 0x56, 0x00}, 5);
    soft_i2c_write_bytes(MLX_SLAVE_ADDR, cmd_buf, sizeof(cmd_buf), true);

    // Write configuration register
    uint8_t lsb = 0x30 | (mp_obj_get_int(refresh_rate) & 0x0F);
    uint8_t msb = 0x44;
    memcpy(cmd_buf, (uint8_t [5]){SET_CONFIG_DATA, (uint8_t)(lsb-0x55), lsb, (uint8_t)(msb-0x55), msb}, 5);
    soft_i2c_write_bytes(MLX_SLAVE_ADDR, cmd_buf, sizeof(cmd_buf), true);

    // Calculate To/Ta constants
    // Note the ADC is set to the highest resolution, the following
    // calculations can omit the (2^3-ConfigReg[5:4]) value which is equal to 1.
    int resolution = 3; //TODO read resolution
    int k_t1_scale = (EEPROM_DATA[KT_SCALE] & 0xF0) >> 4;
    int k_t2_scale = (EEPROM_DATA[KT_SCALE] & 0x0F) + 10;
    int a_i_scale = (EEPROM_DATA[CAL_AI_SCALE] & 0xF0) >> 4;
	int b_i_scale = (EEPROM_DATA[CAL_BI_SCALE] & 0x0F);
    int ks_scale = (EEPROM_DATA[CAL_KS_SCALE] & 0x0F) + 8;
    int a_common = (int16_t) (EEPROM_DATA[CAL_ACOMMON_H] << 8 | EEPROM_DATA[CAL_ACOMMON_L]);

    v_th = (int16_t) (EEPROM_DATA[VTH_H] << 8 | EEPROM_DATA[VTH_L]) / (float) (1 << (3 - resolution));
    k_t1 = (int16_t) (EEPROM_DATA[KT1_H] << 8 | EEPROM_DATA[KT1_L]) / (float) (1 << (k_t1_scale + (3 - resolution)));
    k_t2 = (int16_t) (EEPROM_DATA[KT2_H] << 8 | EEPROM_DATA[KT2_L]) / (float) (1 << (k_t2_scale + (3 - resolution)));           

    emissivity = (EEPROM_DATA[CAL_EMIS_H] << 8 | EEPROM_DATA[CAL_EMIS_L]) >> 15;
    alpha_cp =  (EEPROM_DATA[CAL_ALPHACP_H] << 8 | EEPROM_DATA[CAL_ALPHACP_L]) / powf(2, (EEPROM_DATA[CAL_A0_SCALE] + (3 - resolution)));

	a_cp = (int16_t) (EEPROM_DATA[CAL_ACP_H] << 8 | EEPROM_DATA[CAL_ACP_L]) / (float) (1 << (3 - resolution));
	b_cp = (int16_t) EEPROM_DATA[CAL_BCP] / (float) (1 << (b_i_scale + (3 - resolution)));
    tgc  = (int8_t) EEPROM_DATA[CAL_TGC] / 32.0f;
    ks4 = (int8_t) EEPROM_DATA[CAL_KS4_EE] / (float) (1 << ks_scale);
    ksta = (int16_t) (EEPROM_DATA[CAL_KSTA_H] << 8 | EEPROM_DATA[CAL_KSTA_L])/ (float) (1 << 20);

    printf("vth: %f kt1: %f kt2: %f a_common: %d emissivity: %f\n",
          (double) v_th, (double) k_t1, (double)k_t2, a_common, (double) emissivity);
    printf("a_scale: %d b_scale: %d alpha_cp:%f a_cp: %f b_cp: %f tgc: %f \n",
          a_i_scale, b_i_scale, (double) alpha_cp, (double) a_cp, (double) b_cp, (double) tgc);

    printf("a_ij, b_ij\n");
    for (int i=0; i<64; i++) {
        // Pixel offsets
        a_ij[i] = (a_common + EEPROM_DATA[i] * (1 << a_i_scale)) / (float) (1 << (3 - resolution));

        // Slope coefficients
        b_ij[i] = EEPROM_DATA[0x40 + i] / (float) (1 << (b_i_scale + (3 - resolution)));
        printf("a_ij %f b_ij %f\n", (double) a_ij[i], (double) b_ij[i]);
    }
 
    /*
    // prints alpha_ij table
    for (int i=0; i<64; i++) {
        printf ("%d, %d,", (EEPROM_DATA[CAL_A0_H] << 8 | EEPROM_DATA[CAL_A0_L]), (EEPROM_DATA[0X80 + i])); 
    }
    printf("\n");
    */

    printf("Ta %f\n", (double)calculate_Ta());
    return mp_const_true;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(mlx90620_init_obj,     mlx90620_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mlx90620_read_ta_obj,  mlx90620_read_ta);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mlx90620_read_ir_obj,  mlx90620_read_ir);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mlx90620_read_raw_obj, mlx90620_read_raw);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),         MP_OBJ_NEW_QSTR(MP_QSTR_mlx) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_IR_REFRESH_8HZ),   MP_OBJ_NEW_SMALL_INT(IR_REFRESH_8HZ)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_IR_REFRESH_16HZ),  MP_OBJ_NEW_SMALL_INT(IR_REFRESH_16HZ)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_IR_REFRESH_32HZ),  MP_OBJ_NEW_SMALL_INT(IR_REFRESH_32HZ)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_IR_REFRESH_64HZ),  MP_OBJ_NEW_SMALL_INT(IR_REFRESH_64HZ)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_IR_REFRESH_128HZ), MP_OBJ_NEW_SMALL_INT(IR_REFRESH_128HZ)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_IR_REFRESH_256HZ), MP_OBJ_NEW_SMALL_INT(IR_REFRESH_256HZ)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_IR_REFRESH_512HZ), MP_OBJ_NEW_SMALL_INT(IR_REFRESH_512HZ)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_RAINBOW),          MP_OBJ_NEW_SMALL_INT(RAINBOW)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_GRAYSCALE),        MP_OBJ_NEW_SMALL_INT(GRAYSCALE)},

    { MP_OBJ_NEW_QSTR(MP_QSTR_init),             (mp_obj_t)&mlx90620_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_ta),          (mp_obj_t)&mlx90620_read_ta_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_ir),          (mp_obj_t)&mlx90620_read_ir_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_raw),         (mp_obj_t)&mlx90620_read_raw_obj },
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t mlx_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_mlx,
    .globals = (mp_obj_t)&globals_dict,
};

#else
const mp_obj_module_t mlx_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_mlx,
};

#endif //OPENMV2
