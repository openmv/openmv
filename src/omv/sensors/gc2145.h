/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * GC2145 driver.
 */
#ifndef __GC2145_H__
#define __GC2145_H__
#define GC2145_XCLK_FREQ (12000000)
int gc2145_init(sensor_t *sensor);
#endif // __GC2145_H__
