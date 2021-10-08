/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2020 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2020 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * IMU Python module.
 */
#include "omv_boardconfig.h"
#if MICROPY_PY_IMU

#include "py/obj.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "py_helper.h"
#include "py_imu.h"

#include STM32_HAL_H

#if defined(IMU_CHIP_LSM6DS3)
#include "lsm6ds3tr_c_reg.h"

typedef union {
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union {
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;


#define LSM_FUNC(f)     lsm6ds3tr_c_##f
#define LSM_CONST(c)    LSM6DS3TR_C_##c

#define lsm_from_fs8_to_mg          lsm6ds3tr_c_from_fs8g_to_mg
#define lsm_from_fs8_to_mg          lsm6ds3tr_c_from_fs8g_to_mg
#define lsm_from_fs2000_to_mdps     lsm6ds3tr_c_from_fs2000dps_to_mdps
#define lsm_from_lsb_to_celsius     lsm6ds3tr_c_from_lsb_to_celsius

#elif defined(IMU_CHIP_LSM6DSOX)
#include "lsm6dsox_reg.h"

typedef union {
  int16_t i16bit[3];
  int16_t u8bit[3];
} axis3bit16_t;

typedef union {
  int16_t i16bit;
  int16_t u8bit[1];
} axis1bit16_t;

#define LSM_FUNC(f)                 lsm6dsox_##f
#define LSM_CONST(c)                LSM6DSOX_##c

#define lsm_from_fs8_to_mg          lsm6dsox_from_fs8_to_mg
#define lsm_from_fs8_to_mg          lsm6dsox_from_fs8_to_mg
#define lsm_from_fs2000_to_mdps     lsm6dsox_from_fs2000_to_mdps
#define lsm_from_lsb_to_celsius     lsm6dsox_from_lsb_to_celsius
#else
#error "imu chip variant is not defined."
#endif

STATIC int32_t platform_write(void *handle, uint8_t Reg, uint8_t *Bufp,
                              uint16_t len)
{
    HAL_GPIO_WritePin(IMU_SPI_SSEL_PORT, IMU_SPI_SSEL_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &Reg, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(handle, Bufp, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(IMU_SPI_SSEL_PORT, IMU_SPI_SSEL_PIN, GPIO_PIN_SET);
    return 0;
}

STATIC int32_t platform_read(void *handle, uint8_t Reg, uint8_t *Bufp,
                             uint16_t len)
{
    Reg |= 0x80;
    HAL_GPIO_WritePin(IMU_SPI_SSEL_PORT, IMU_SPI_SSEL_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &Reg, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(handle, Bufp, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(IMU_SPI_SSEL_PORT, IMU_SPI_SSEL_PIN, GPIO_PIN_SET);
    return 0;
}

STATIC SPI_HandleTypeDef SPIHandle = {
    .Instance               = IMU_SPI,
    .Init.Mode              = SPI_MODE_MASTER,
    .Init.Direction         = SPI_DIRECTION_2LINES,
    .Init.DataSize          = SPI_DATASIZE_8BIT,
    .Init.CLKPolarity       = SPI_POLARITY_HIGH,
    .Init.CLKPhase          = SPI_PHASE_2EDGE,
    .Init.NSS               = SPI_NSS_SOFT,
    .Init.BaudRatePrescaler = IMU_SPI_PRESCALER,
    .Init.FirstBit          = SPI_FIRSTBIT_MSB,
};

STATIC stmdev_ctx_t dev_ctx = {
    .write_reg = platform_write,
    .read_reg = platform_read,
    .handle = &SPIHandle
};

STATIC bool test_not_ready()
{
    return HAL_SPI_GetState(&SPIHandle) != HAL_SPI_STATE_READY;
}

STATIC void error_on_not_ready()
{
    if (test_not_ready()) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("IMU Not Ready!"));
    }
}

STATIC mp_obj_t py_imu_tuple(float x, float y, float z)
{
    return mp_obj_new_tuple(3, (mp_obj_t [3]) {mp_obj_new_float(x),
                                               mp_obj_new_float(y),
                                               mp_obj_new_float(z)});
}

// For when the camera board is lying on a table face up.

// X points to the right of the camera sensor
// Y points down below the camera sensor
// Z points in the reverse direction of the camera sensor

// Thus (https://www.nxp.com/docs/en/application-note/AN3461.pdf):
//
// Roll = atan2(Y, Z)
// Pitch = atan2(-X, sqrt(Y^2, + Z^2)) -> assume Y=0 -> atan2(-X, Z)

// For when the camera board is standing right-side up.

// X points to the right of the camera sensor (still X)
// Y points down below the camera sensor (now Z)
// Z points in the reverse direction of the camera sensor (now -Y)

// So:
//
// Roll = atan2(-X, sqrt(Z^2, + Y^2)) -> assume Z=0 -> atan2(-X, Y)
// Pitch = atan2(Z, -Y)

#if (OMV_IMU_ROTATION_DEGREES != 0) && \
    (OMV_IMU_ROTATION_DEGREES != 90) && \
    (OMV_IMU_ROTATION_DEGREES != 180) && \
    (OMV_IMU_ROTATION_DEGREES != 270)
#error "OMV_IMU_ROTATION_DEGREES must be 0, 90, 180, or 270!"
#endif

#if (OMV_IMU_MOUNTING_Z_DIRECTION != -1) && \
    (OMV_IMU_MOUNTING_Z_DIRECTION != 1)
#error "OMV_IMU_MOUNTING_Z_DIRECTION must be -1 or 1!"
#endif

STATIC float py_imu_get_roll()
{
    axis3bit16_t data_raw_acceleration = {};
    LSM_FUNC(acceleration_raw_get)(&dev_ctx, data_raw_acceleration.u8bit);
    #if OMV_IMU_ROTATION_DEGREES == 0
    float xr = lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[0]); // x
    float yr = lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[1]); // y
    #elif OMV_IMU_X_Y_ROTATION_DEGREES == 90
    float xr = -lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[1]); // y
    float yr = lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[0]); // x
    #elif OMV_IMU_X_Y_ROTATION_DEGREES == 180
    float xr = -lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[0]); // x
    float yr = -lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[1]); // y
    #elif OMV_IMU_X_Y_ROTATION_DEGREES == 270
    float xr = lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[1]); // y
    float yr = -lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[0]); // x
    #endif
    #if OMV_IMU_MOUNTING_Z_DIRECTION == 1 // default is -1 (IMU pointing reverse of camera)
    xr = -xr;
    yr = -yr;
    #endif
    return fmodf((IM_RAD2DEG(fast_atan2f(-xr, yr)) + 180), 360); // rotate 180
}

STATIC float py_imu_get_pitch()
{
    axis3bit16_t data_raw_acceleration = {};
    LSM_FUNC(acceleration_raw_get)(&dev_ctx, data_raw_acceleration.u8bit);
    #if OMV_IMU_ROTATION_DEGREES == 0
    float yr = lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[1]); // y
    float zr = lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[2]); // z
    #elif OMV_IMU_X_Y_ROTATION_DEGREES == 90
    float yr = lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[0]); // x
    float zr = lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[2]); // z
    #elif OMV_IMU_X_Y_ROTATION_DEGREES == 180
    float yr = -lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[1]); // y
    float zr = lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[2]); // z
    #elif OMV_IMU_X_Y_ROTATION_DEGREES == 270
    float yr = -lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[0]); // x
    float zr = lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[2]); // z
    #endif
    #if OMV_IMU_MOUNTING_Z_DIRECTION == 1 // default is -1 (IMU pointing reverse of camera)
    yr = -yr;
    zr = -zr;
    #endif
    return IM_RAD2DEG(fast_atan2f(zr, -yr));
}

STATIC mp_obj_t py_imu_acceleration_mg()
{
    error_on_not_ready();

    axis3bit16_t data_raw_acceleration = {};
    LSM_FUNC(acceleration_raw_get)(&dev_ctx, data_raw_acceleration.u8bit);
    return py_imu_tuple(lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[0]),
                        lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[1]),
                        lsm_from_fs8_to_mg(data_raw_acceleration.i16bit[2]));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_imu_acceleration_mg_obj, py_imu_acceleration_mg);

STATIC mp_obj_t py_imu_angular_rate_mdps()
{
    error_on_not_ready();

    axis3bit16_t data_raw_angular_rate = {};
    LSM_FUNC(angular_rate_raw_get)(&dev_ctx, data_raw_angular_rate.u8bit);
    return py_imu_tuple(lsm_from_fs2000_to_mdps(data_raw_angular_rate.i16bit[0]),
                        lsm_from_fs2000_to_mdps(data_raw_angular_rate.i16bit[1]),
                        lsm_from_fs2000_to_mdps(data_raw_angular_rate.i16bit[2]));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_imu_angular_rate_mdps_obj, py_imu_angular_rate_mdps);

STATIC mp_obj_t py_imu_temperature_c()
{
    error_on_not_ready();

    axis1bit16_t data_raw_temperature = {};
    LSM_FUNC(temperature_raw_get)(&dev_ctx, data_raw_temperature.u8bit);
    return mp_obj_new_float(LSM_FUNC(from_lsb_to_celsius)(data_raw_temperature.i16bit));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_imu_temperature_c_obj, py_imu_temperature_c);

STATIC mp_obj_t py_imu_roll()
{
    error_on_not_ready();

    return mp_obj_new_float(py_imu_get_roll());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_imu_roll_obj, py_imu_roll);

STATIC mp_obj_t py_imu_pitch()
{
    error_on_not_ready();

    return mp_obj_new_float(py_imu_get_pitch());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_imu_pitch_obj, py_imu_pitch);

STATIC mp_obj_t py_imu_sleep(mp_obj_t enable)
{
    error_on_not_ready();

    bool en = mp_obj_get_int(enable);
    LSM_FUNC(xl_data_rate_set)(&dev_ctx, en ? LSM_CONST(XL_ODR_OFF) : LSM_CONST(XL_ODR_52Hz));
    LSM_FUNC(gy_data_rate_set)(&dev_ctx, en ? LSM_CONST(GY_ODR_OFF) : LSM_CONST(GY_ODR_52Hz));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_imu_sleep_obj, py_imu_sleep);

STATIC mp_obj_t py_imu_write_reg(mp_obj_t addr, mp_obj_t val)
{
    error_on_not_ready();

    uint8_t v = mp_obj_get_int(val);
    LSM_FUNC(write_reg)(&dev_ctx, mp_obj_get_int(addr), &v, sizeof(v));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_imu_write_reg_obj, py_imu_write_reg);

STATIC mp_obj_t py_imu_read_reg(mp_obj_t addr)
{
    error_on_not_ready();

    uint8_t v;
    LSM_FUNC(read_reg)(&dev_ctx, mp_obj_get_int(addr), &v, sizeof(v));
    return mp_obj_new_int(v);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_imu_read_reg_obj, py_imu_read_reg);

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_OBJ_NEW_QSTR(MP_QSTR_imu) },
    { MP_ROM_QSTR(MP_QSTR_acceleration_mg),     MP_ROM_PTR(&py_imu_acceleration_mg_obj) },
    { MP_ROM_QSTR(MP_QSTR_angular_rate_mdps),   MP_ROM_PTR(&py_imu_angular_rate_mdps_obj) },
    { MP_ROM_QSTR(MP_QSTR_temperature_c),       MP_ROM_PTR(&py_imu_temperature_c_obj) },
    { MP_ROM_QSTR(MP_QSTR_roll),                MP_ROM_PTR(&py_imu_roll_obj) },
    { MP_ROM_QSTR(MP_QSTR_pitch),               MP_ROM_PTR(&py_imu_pitch_obj) },
    { MP_ROM_QSTR(MP_QSTR_sleep),               MP_ROM_PTR(&py_imu_sleep_obj) },
    { MP_ROM_QSTR(MP_QSTR___write_reg),         MP_ROM_PTR(&py_imu_write_reg_obj) },
    { MP_ROM_QSTR(MP_QSTR___read_reg),          MP_ROM_PTR(&py_imu_read_reg_obj) },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t imu_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict
};

void py_imu_init()
{
    HAL_SPI_Init(&SPIHandle);

    uint8_t whoamI = 0, rst = 1;

    // Try to read device id...
    for (int i = 0; (i < 10) && (whoamI != LSM_CONST(ID)); i++) {
        LSM_FUNC(device_id_get)(&dev_ctx, &whoamI);
    }

    if (whoamI != LSM_CONST(ID)) {
        HAL_SPI_DeInit(&SPIHandle);
        return;
    }

    LSM_FUNC(reset_set)(&dev_ctx, PROPERTY_ENABLE);

    for (int i = 0; (i < 10000) && rst; i++) {
        LSM_FUNC(reset_get)(&dev_ctx, &rst);
    }

    if (rst) {
        HAL_SPI_DeInit(&SPIHandle);
        return;
    }

    LSM_FUNC(block_data_update_set)(&dev_ctx, PROPERTY_ENABLE);

    LSM_FUNC(xl_data_rate_set)(&dev_ctx, LSM_CONST(XL_ODR_52Hz));
    LSM_FUNC(gy_data_rate_set)(&dev_ctx, LSM_CONST(GY_ODR_52Hz));

    LSM_FUNC(xl_full_scale_set)(&dev_ctx, LSM_CONST(8g));
    LSM_FUNC(gy_full_scale_set)(&dev_ctx, LSM_CONST(2000dps));
}

float py_imu_roll_rotation()
{
    if (test_not_ready()) {
        return 0; // output when the camera is mounted upright
    }

    return py_imu_get_roll();
}

float py_imu_pitch_rotation()
{
    if (test_not_ready()) {
        return 0; // output when the camera is mounted upright
    }

    return py_imu_get_pitch();
}

MP_REGISTER_MODULE(MP_QSTR_imu, imu_module, MICROPY_PY_IMU);
#endif // MICROPY_PY_IMU
