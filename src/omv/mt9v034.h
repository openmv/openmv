/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MT9V034 driver.
 *
 */
#ifndef __MT9V034_H__
#define __MT9V034_H__
#include "sensor.h"
#define MT9V034_XCLK_FREQ 27000000
int mt9v034_init(sensor_t *sensor);
#endif // __MT9V034_H__
