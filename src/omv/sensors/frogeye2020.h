/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * FrogEye2020 driver.
 */
#ifndef __FROGEYE2020_H__
#define __FROGEYE2020_H__
#define FROGEYE2020_XCLK_FREQ (5000000)
int frogeye2020_init(sensor_t *sensor);
#endif // __FROGEYE2020_H__
