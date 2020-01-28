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
#include STM32_HAL_H
#include "lsm6ds3tr_c_reg.h"
#include "omv_boardconfig.h"
#include "py_helper.h"
#include "py_imu.h"

#if defined(OMV_ENABLE_IMU)

typedef union {
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union {
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;

static int32_t platform_write(void *handle, uint8_t Reg, uint8_t *Bufp,
                              uint16_t len)
{
    HAL_GPIO_WritePin(IMU_SPI_SSEL_PORT, IMU_SPI_SSEL_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &Reg, 1, 1000);
    HAL_SPI_Transmit(handle, Bufp, len, 1000);
    HAL_GPIO_WritePin(IMU_SPI_SSEL_PORT, IMU_SPI_SSEL_PIN, GPIO_PIN_SET);
    return 0;
}

static int32_t platform_read(void *handle, uint8_t Reg, uint8_t *Bufp,
                             uint16_t len)
{
    Reg |= 0x80;
    HAL_GPIO_WritePin(IMU_SPI_SSEL_PORT, IMU_SPI_SSEL_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &Reg, 1, 1000);
    HAL_SPI_Receive(handle, Bufp, len, 1000);
    HAL_GPIO_WritePin(IMU_SPI_SSEL_PORT, IMU_SPI_SSEL_PIN, GPIO_PIN_SET);
    return 0;
}

static SPI_HandleTypeDef SPIHandle = {
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

static stmdev_ctx_t dev_ctx = {
    .write_reg = platform_write,
    .read_reg = platform_read,
    .handle = &SPIHandle
};

STATIC mp_obj_t py_imu_acceleration_mg()
{
    if (HAL_SPI_GetState(&SPIHandle) != HAL_SPI_STATE_READY) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "IMU Not Ready!"));
    }

    axis3bit16_t data_raw_acceleration = {};
    lsm6ds3tr_c_acceleration_raw_get(&dev_ctx, data_raw_acceleration.u8bit);
    return mp_obj_new_tuple(3, (mp_obj_t [3]) {
            mp_obj_new_float(lsm6ds3tr_c_from_fs2g_to_mg(data_raw_acceleration.i16bit[0])),
            mp_obj_new_float(lsm6ds3tr_c_from_fs2g_to_mg(data_raw_acceleration.i16bit[1])),
            mp_obj_new_float(lsm6ds3tr_c_from_fs2g_to_mg(data_raw_acceleration.i16bit[2]))});
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_imu_acceleration_mg_obj, py_imu_acceleration_mg);

STATIC mp_obj_t py_imu_angular_rate_mdps()
{
    if (HAL_SPI_GetState(&SPIHandle) != HAL_SPI_STATE_READY) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "IMU Not Ready!"));
    }

    axis3bit16_t data_raw_angular_rate = {};
    lsm6ds3tr_c_angular_rate_raw_get(&dev_ctx, data_raw_angular_rate.u8bit);
    return mp_obj_new_tuple(3, (mp_obj_t [3]) {
            mp_obj_new_float(lsm6ds3tr_c_from_fs2000dps_to_mdps(data_raw_angular_rate.i16bit[0])),
            mp_obj_new_float(lsm6ds3tr_c_from_fs2000dps_to_mdps(data_raw_angular_rate.i16bit[1])),
            mp_obj_new_float(lsm6ds3tr_c_from_fs2000dps_to_mdps(data_raw_angular_rate.i16bit[2]))});
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_imu_angular_rate_mdps_obj, py_imu_angular_rate_mdps);

STATIC mp_obj_t py_imu_temperature_c()
{
    if (HAL_SPI_GetState(&SPIHandle) != HAL_SPI_STATE_READY) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "IMU Not Ready!"));
    }

    axis1bit16_t data_raw_temperature = {};
    lsm6ds3tr_c_temperature_raw_get(&dev_ctx, data_raw_temperature.u8bit);
    return mp_obj_new_float(lsm6ds3tr_c_from_lsb_to_celsius(data_raw_temperature.i16bit));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_imu_temperature_c_obj, py_imu_temperature_c);

#endif

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_OBJ_NEW_QSTR(MP_QSTR_imu) },
#if defined(OMV_ENABLE_IMU)
    { MP_ROM_QSTR(MP_QSTR_acceleration_mg),     MP_ROM_PTR(&py_imu_acceleration_mg_obj) },
    { MP_ROM_QSTR(MP_QSTR_angular_rate_mdps),   MP_ROM_PTR(&py_imu_angular_rate_mdps_obj) },
    { MP_ROM_QSTR(MP_QSTR_temperature_c),       MP_ROM_PTR(&py_imu_temperature_c_obj) },
#else
    { MP_ROM_QSTR(MP_QSTR_acceleration_mg),     MP_ROM_PTR(&py_func_unavailable_obj) },
    { MP_ROM_QSTR(MP_QSTR_angular_rate_mdps),   MP_ROM_PTR(&py_func_unavailable_obj) },
    { MP_ROM_QSTR(MP_QSTR_temperature_c),       MP_ROM_PTR(&py_func_unavailable_obj) },
#endif
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t imu_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict
};

void py_imu_init()
{
#if defined(OMV_ENABLE_IMU)
    HAL_SPI_Init(&SPIHandle);

    uint8_t whoamI = 0, rst = 1;

    // Try to read device id...
    for (int i = 0; (i < 10) && (whoamI != LSM6DS3TR_C_ID); i++) {
        lsm6ds3tr_c_device_id_get(&dev_ctx, &whoamI);
    }

    if (whoamI != LSM6DS3TR_C_ID) {
        HAL_SPI_DeInit(&SPIHandle);
        return;
    }

    /*
     *  Restore default configuration
     */

    lsm6ds3tr_c_reset_set(&dev_ctx, PROPERTY_ENABLE);

    for (int i = 0; (i < 10000) && rst; i++) {
        lsm6ds3tr_c_reset_get(&dev_ctx, &rst);
    }

    if (rst) {
        HAL_SPI_DeInit(&SPIHandle);
        return;
    }

    /*
     *  Enable Block Data Update
     */

    lsm6ds3tr_c_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

    /*
     * Set Output Data Rate
     */
    lsm6ds3tr_c_xl_data_rate_set(&dev_ctx, LSM6DS3TR_C_XL_ODR_52Hz);
    lsm6ds3tr_c_gy_data_rate_set(&dev_ctx, LSM6DS3TR_C_GY_ODR_52Hz);

    /*
     * Set full scale
     */
    lsm6ds3tr_c_xl_full_scale_set(&dev_ctx, LSM6DS3TR_C_8g);
    lsm6ds3tr_c_gy_full_scale_set(&dev_ctx, LSM6DS3TR_C_2000dps);

    /*
     * Configure filtering chain (No aux interface)
     */

    /* Accelerometer - analog filter */
    lsm6ds3tr_c_xl_filter_analog_set(&dev_ctx, LSM6DS3TR_C_XL_ANA_BW_400Hz);

    /* Accelerometer - LPF1 path ( LPF2 not used ) */
    //lsm6ds3tr_c_xl_lp1_bandwidth_set(&dev_ctx, LSM6DS3TR_C_XL_LP1_ODR_DIV_4);

    /* Accelerometer - LPF1 + LPF2 path */
    lsm6ds3tr_c_xl_lp2_bandwidth_set(&dev_ctx, LSM6DS3TR_C_XL_LOW_NOISE_LP_ODR_DIV_100);

    /* Accelerometer - High Pass / Slope path */
    //lsm6ds3tr_c_xl_reference_mode_set(&dev_ctx, PROPERTY_DISABLE);
    //lsm6ds3tr_c_xl_hp_bandwidth_set(&dev_ctx, LSM6DS3TR_C_XL_HP_ODR_DIV_100);

    /* Gyroscope - filtering chain */
    lsm6ds3tr_c_gy_band_pass_set(&dev_ctx, LSM6DS3TR_C_HP_260mHz_LP1_STRONG);
#endif
}

float py_imu_xy_rotation()
{
#if defined(OMV_ENABLE_IMU)
    if (HAL_SPI_GetState(&SPIHandle) != HAL_SPI_STATE_READY) {
        return 0;
    }

    axis3bit16_t data_raw_acceleration = {};
    lsm6ds3tr_c_acceleration_raw_get(&dev_ctx, data_raw_acceleration.u8bit);
    float x = lsm6ds3tr_c_from_fs2g_to_mg(data_raw_acceleration.i16bit[0]);
    float y = lsm6ds3tr_c_from_fs2g_to_mg(data_raw_acceleration.i16bit[1]);
    return fast_atan2f(y, x);
#else
    return 0;
#endif
}

float py_imu_yz_rotation()
{
#if defined(OMV_ENABLE_IMU)
    if (HAL_SPI_GetState(&SPIHandle) != HAL_SPI_STATE_READY) {
        return 0;
    }

    axis3bit16_t data_raw_acceleration = {};
    lsm6ds3tr_c_acceleration_raw_get(&dev_ctx, data_raw_acceleration.u8bit);
    float y = lsm6ds3tr_c_from_fs2g_to_mg(data_raw_acceleration.i16bit[1]);
    float z = lsm6ds3tr_c_from_fs2g_to_mg(data_raw_acceleration.i16bit[2]);
    return fast_atan2f(z, y);
#else
    return 0;
#endif
}

float py_imu_zx_rotation()
{
#if defined(OMV_ENABLE_IMU)
    if (HAL_SPI_GetState(&SPIHandle) != HAL_SPI_STATE_READY) {
        return 0;
    }

    axis3bit16_t data_raw_acceleration = {};
    lsm6ds3tr_c_acceleration_raw_get(&dev_ctx, data_raw_acceleration.u8bit);
    float z = lsm6ds3tr_c_from_fs2g_to_mg(data_raw_acceleration.i16bit[2]);
    float x = lsm6ds3tr_c_from_fs2g_to_mg(data_raw_acceleration.i16bit[0]);
    return fast_atan2f(x, z);
#else
    return 0;
#endif
}
