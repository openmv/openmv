/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Lake Fu <lake_fu@pixart.com> for PixArt Inc.
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * PAG7920 driver.
 */
#ifndef __PAG7920_H__
#define __PAG7920_H__

#define OMV_PAG7920_XCLK_FREQ 24000000

int pag7920_init(sensor_t *sensor);
#endif
