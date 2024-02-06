/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2022 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2022 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * HM0360 driver.
 */
#ifndef __HM0360_H__
#define __HM0360_H__
// This sensor uses an internal oscillator.
#define OMV_HM0360_XCLK_FREQ    (0)
int hm0360_init(sensor_t *sensor);
#endif // __HM0360_H__
