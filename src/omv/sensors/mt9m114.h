/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MT9M114 driver.
 */
#ifndef __MT9M114_H__
#define __MT9M114_H__
#define OMV_MT9M114_XCLK_FREQ    (24000000)
int mt9m114_init(sensor_t *sensor);
#endif // __MT9M114_H__
