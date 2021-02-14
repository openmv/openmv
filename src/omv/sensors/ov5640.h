/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV5640 driver.
 */
#ifndef __OV5640_H__
#define __OV5640_H__
#define OV5640_XCLK_FREQ 24000000
int ov5640_init(sensor_t *sensor);
#endif // __OV5640_H__
