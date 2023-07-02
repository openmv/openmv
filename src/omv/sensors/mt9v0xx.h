/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MT9V0XX driver.
 */
#ifndef __MT9V0XX_H__
#define __MT9V0XX_H__
#ifndef MT9V0XX_XCLK_FREQ
#define MT9V0XX_XCLK_FREQ    26666666
#endif
int mt9v0xx_init(sensor_t *sensor);
#endif // __MT9V0XX_H__
