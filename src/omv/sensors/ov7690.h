/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2020 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2020 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV7690 driver.
 */
#ifndef __OV7690_H__
#define __OV7690_H__
#define OV7690_XCLK_FREQ 24000000
int ov7690_init(sensor_t *sensor);
#endif // __OV7690_H__
