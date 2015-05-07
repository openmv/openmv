/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MLX90620 Python module.
 *
 */
#include <mp.h>
#include <stdbool.h>
#include <float.h>
#include "systick.h"
#include "soft_i2c.h"
#include "mdefs.h"
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

#define CAL_ACP             0xD4
#define CAL_BCP             0xD5
#define CAL_TGC             0xD8
#define CAL_alphaCP_L       0xD6
#define CAL_alphaCP_H       0xD7
#define CAL_BI_SCALE        0xD9

#define VTH_L 0xDA
#define VTH_H 0xDB
#define KT1_L 0xDC
#define KT1_H 0xDD
#define KT2_L 0xDE
#define KT2_H 0xDF

//Common sensitivity coefficients
#define CAL_A0_L            0xE0
#define CAL_A0_H            0xE1
#define CAL_A0_SCALE        0xE2
#define CAL_DELTA_A_SCALE   0xE3
#define CAL_EMIS_L          0xE4
#define CAL_EMIS_H          0xE5

#define MAP(OldValue, OldMin, OldMax, NewMin, NewMax)\
    (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin

#define TIMEOUT             (10000)


enum image_type {
    RAINBOW,
    GRAYSCALE,
};

/* Grayscale [0..255] to rainbox lookup */
extern const uint16_t rainbow_table[256];

static const float alpha_ij[64] = {
  1.60499E-8f, 1.87856E-8f, 1.93677E-8f, 1.87856E-8f, 1.83782E-8f, 2.11139E-8f, 2.21035E-8f, 2.07647E-8f,
  2.01826E-8f, 2.30930E-8f, 2.38497E-8f, 2.23363E-8f, 2.19288E-8f, 2.52467E-8f, 2.58287E-8f, 2.46646E-8f,
  2.26855E-8f, 2.66436E-8f, 2.68183E-8f, 2.54213E-8f, 2.40825E-8f, 2.72257E-8f, 2.81570E-8f, 2.62362E-8f,
  2.48392E-8f, 2.81570E-8f, 2.89720E-8f, 2.68183E-8f, 2.50720E-8f, 2.83899E-8f, 2.87973E-8f, 2.72257E-8f,
  2.52467E-8f, 2.85645E-8f, 2.91466E-8f, 2.72257E-8f, 2.52467E-8f, 2.83899E-8f, 2.85645E-8f, 2.72257E-8f,
  2.58287E-8f, 2.81570E-8f, 2.83899E-8f, 2.64108E-8f, 2.46646E-8f, 2.74003E-8f, 2.81570E-8f, 2.58287E-8f,
  2.42571E-8f, 2.66436E-8f, 2.68183E-8f, 2.54213E-8f, 2.26855E-8f, 2.56541E-8f, 2.56541E-8f, 2.40825E-8f,
  2.15214E-8f, 2.38497E-8f, 2.40825E-8f, 2.21035E-8f, 1.99498E-8f, 2.19288E-8f, 2.16960E-8f, 2.01826E-8f,
};

// These are constants calculated from
// the calibration data stored in EEPROM
float k_t1, k_t1_sq, k_t2, emissivity;
int v_th, a_cp, b_cp, tgc, b_i_scale;
int8_t a_ij[64], b_ij[64];

static float calculate_TA(void)
{
    uint16_t ptat=0;
    uint8_t cmd_buf[4]={MLX_READ_REG, 0x90, 0x00, 0x01};
    soft_i2c_write_bytes(MLX_SLAVE_ADDR, cmd_buf, sizeof(cmd_buf), false);
    soft_i2c_read_bytes(MLX_SLAVE_ADDR, (uint8_t*)&ptat, 2, true);
    return (-k_t1 + fast_sqrtf(k_t1_sq - (4 * k_t2 * (v_th - ptat)))) / (2 * k_t2) + 25;
}

static void mlx90620_read_to(float *t, float Ta)
{
    float v_ir_comp;
    float v_ir_off_comp;
    float v_ir_tgc_comp;

    int16_t cpix;
    uint8_t cmd_buf[4];
    int16_t ir_data[64];

    // (T+273.15f)^4
    float Ta4 = (Ta + 273.15f) * (Ta + 273.15f) * (Ta + 273.15f) * (Ta + 273.15f);

    // Read IR data
    memcpy(cmd_buf, (uint8_t [4]){MLX_READ_REG, 0x00, 0x01, 0x40}, sizeof(cmd_buf)); //read 64*2 bytes
    soft_i2c_write_bytes(MLX_SLAVE_ADDR, cmd_buf, sizeof(cmd_buf), false);
    soft_i2c_read_bytes(MLX_SLAVE_ADDR, (uint8_t*)ir_data, 128, true);

    // Read compensation data
    memcpy(cmd_buf, (uint8_t [4]){MLX_READ_REG, 0x91, 0x00, 0x01}, sizeof(cmd_buf));
    soft_i2c_write_bytes(MLX_SLAVE_ADDR, cmd_buf, sizeof(cmd_buf), false);
    soft_i2c_read_bytes(MLX_SLAVE_ADDR, (uint8_t*)&cpix, 2, true);

    //Calculate the offset compensation for the one compensation pixel
    //This is a constant in the TO calculation, so calculate it here.
    float v_cp_off_comp = (float)cpix - (a_cp + (b_cp/(2<<(b_i_scale-1))) * (Ta - 25));

    for (int i=0; i<64; i++) {
        //#1: Calculate Offset Compensation
        v_ir_off_comp = ir_data[i] - (a_ij[i] + (float)(b_ij[i]/(2<<(b_i_scale-1))) * (Ta - 25));

        //#2: Calculate Thermal Gradien Compensation (TGC)
        v_ir_tgc_comp = v_ir_off_comp - ( ((float)tgc/32) * v_cp_off_comp);

        //#3: Calculate Emissivity Compensation
        v_ir_comp = v_ir_tgc_comp / emissivity;

        t[i] = fast_sqrtf(fast_sqrtf(v_ir_comp/alpha_ij[i] + Ta4)) - 273.15f;
    }

}

mp_obj_t mlx90620_read(mp_obj_t type_obj)
{
    float temp[64];
    float temp_flip[64];
    float max_temp = FLT_MIN;
    float min_temp = FLT_MAX;

    image_t *img;
    enum image_type img_type;

    //alloc image
    img = xalloc(sizeof(*img));
    img->w = 16;
    img->h = 4;

    // read image type
    img_type = mp_obj_get_int(type_obj);

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

    // get raw temperatures
    float ta = calculate_TA();
    mlx90620_read_to(temp, ta);

    // flip IR data, sensor memory read is column wise
    float *temp_p = temp_flip;
    memcpy(temp_p, temp, sizeof(temp));
    for (int x=15; x>=0; x--) {
        for (int y=0; y<4; y++) {
            temp[x+y*16] = *temp_p++;
        }
    }

    // normalize temp readings
    for (int i=0; i<64; i++) {
        temp[i] = temp[i]-ta;
        if (temp[i] > max_temp) {
            max_temp = temp[i];
        } else if (temp[i] < min_temp) {
            min_temp = temp[i];
        }
    }

    // map temps to rainbow or grayscale
    for (int i=0; i<64; i++) {
        uint16_t p = (uint16_t) MAP(temp[i], min_temp, max_temp, 0, 255);
        //uint16_t p = (((temp[i]-min_temp)/(max_temp-min_temp))*255.0f);

        switch (img_type) {
            case GRAYSCALE:
                img->pixels[i] = p;
                break;
            case RAINBOW:
                ((uint16_t*)img->pixels)[i] = rainbow_table[(uint8_t)p];
                break;
        }
    }

    return py_image_from_struct(img);
}

mp_obj_t mlx90620_read_raw()
{
    float *t = m_new(float, 64);
    mp_obj_t t_list = mp_obj_new_list(0, NULL);

    // get raw temperatures
    float ta = calculate_TA();
    mlx90620_read_to(t, ta);

    // normalize temp readings
    for (int i=0; i<64; i++) {
        mp_obj_list_append(t_list, mp_obj_new_float(t[i]));
    }

    return t_list;
}

mp_obj_t mlx90620_init()
{
    uint8_t cmd_buf[5];
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
    uint8_t lsb = 0x09; //32Hz
    uint8_t msb = 0x74;
    memcpy(cmd_buf, (uint8_t [5]){SET_CONFIG_DATA, (uint8_t)(lsb-0x55), lsb, (uint8_t)(msb-0x55), msb}, 5);
    soft_i2c_write_bytes(MLX_SLAVE_ADDR, cmd_buf, sizeof(cmd_buf), true);

    // Calculate Ta constants
    v_th = (256 * EEPROM_DATA[VTH_H] + EEPROM_DATA[VTH_L]);
    k_t1 = (256 * EEPROM_DATA[KT1_H] + EEPROM_DATA[KT1_L]) / 1024.0f;
    k_t2 = (256 * EEPROM_DATA[KT2_H] + EEPROM_DATA[KT2_L]) / 1048576.0f;
    emissivity = ((unsigned int)256 * EEPROM_DATA[CAL_EMIS_H] + EEPROM_DATA[CAL_EMIS_L]) / 32768.0f;
    k_t1_sq = k_t1 * k_t1;

    a_cp = (int8_t)EEPROM_DATA[CAL_ACP];
    b_cp = (int8_t)EEPROM_DATA[CAL_BCP];
    tgc  = (int8_t)EEPROM_DATA[CAL_TGC];
    b_i_scale = EEPROM_DATA[CAL_BI_SCALE];

    // Hack
    for (int i=0; i<8; i++) {
        EEPROM_DATA[i]=EEPROM_DATA[i+8];
        EEPROM_DATA[i+64]=EEPROM_DATA[i+8+64];
    }

    for (int i=0; i<64; i++) {
        // Read pixel offsets
        a_ij[i] = (int8_t)EEPROM_DATA[i];

        // Read slope coefficients
        b_ij[i] = (int8_t)EEPROM_DATA[i+4];
    }

    return mp_const_true;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(mlx90620_init_obj,     mlx90620_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mlx90620_read_obj,     mlx90620_read);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mlx90620_read_raw_obj, mlx90620_read_raw);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),    MP_OBJ_NEW_QSTR(MP_QSTR_mlx) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_HZ_8),      MP_OBJ_NEW_SMALL_INT(MLX_HZ_8)},
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_HZ_16),     MP_OBJ_NEW_SMALL_INT(MLX_HZ_16)},
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_HZ_32),     MP_OBJ_NEW_SMALL_INT(MLX_HZ_32)},
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_HZ_64),     MP_OBJ_NEW_SMALL_INT(MLX_HZ_64)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_RAINBOW),     MP_OBJ_NEW_SMALL_INT(RAINBOW)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_GRAYSCALE),   MP_OBJ_NEW_SMALL_INT(GRAYSCALE)},

    { MP_OBJ_NEW_QSTR(MP_QSTR_init),        (mp_obj_t)&mlx90620_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read),        (mp_obj_t)&mlx90620_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_raw),    (mp_obj_t)&mlx90620_read_raw_obj },
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
