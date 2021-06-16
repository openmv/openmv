/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2019 Lake Fu <lake_fu@pixart.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * PAJ6100 driver.
 *
 */
#ifndef __PAJ6100_H__
#define __PAJ6100_H__
#include <stdbool.h>
#include "sensor.h"
#define PAJ6100_XCLK_FREQ 6000000

bool paj6100_detect(sensor_t *sensor);
int paj6100_init(sensor_t *sensor);
#endif
