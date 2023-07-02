/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * FrogEye2020 driver.
 */
#include "omv_boardconfig.h"
#if (OMV_ENABLE_FROGEYE2020 == 1)

#include "sensor.h"

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat) {
    return (pixformat == PIXFORMAT_GRAYSCALE) ? 0 : -1;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize) {
    return (framesize == FRAMESIZE_QVGA) ? 0 : -1;
}

int frogeye2020_init(sensor_t *sensor) {
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    sensor->hw_flags.pixck = 1;
    sensor->hw_flags.gs_bpp = 1;
    return 0;
}

#endif // (OMV_ENABLE_FROGEYE2020 == 1)
