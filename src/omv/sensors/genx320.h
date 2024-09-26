/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * GENX320 driver.
 */
#ifndef __GENX320_H__
#define __GENX320_H__
#define OMV_GENX320_XCLK_FREQ    (24000000)
int genx320_init(sensor_t *sensor);
#endif // __GENX320_H__
