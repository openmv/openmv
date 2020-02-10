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
#ifndef __PY_IMU_H__
#define __PY_IMU_H__
void py_imu_init();
float py_imu_roll_rotation(); // in degrees
float py_imu_pitch_rotation(); // in degrees
#endif // __PY_IMU_H__
