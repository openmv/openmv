/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Lepton driver.
 */
#ifndef __LEPTON_H__
#define __LEPTON_H__
#include "sensor.h"
#define LEPTON_XCLK_FREQ 24000000
int lepton_init(sensor_t *sensor);
#endif // __LEPTON_H__
