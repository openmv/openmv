/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV2640 driver.
 */
#ifndef __OV2640_H__
#define __OV2640_H__
#define OMV_OV2640_XCLK_FREQ    24000000
int ov2640_init(sensor_t *sensor);
#endif // __OV2640_H__
