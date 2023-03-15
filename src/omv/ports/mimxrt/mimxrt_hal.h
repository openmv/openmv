/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MIMXRT HAL.
 */
#ifndef __MIMXRT_HAL_H__
#define __MIMXRT_HAL_H__

void mimxrt_hal_init();
void mimxrt_hal_bootloader();
int mimxrt_hal_csi_init(CSI_Type *inst);
int mimxrt_hal_i2c_init(uint32_t bus_id);
int mimxrt_hal_spi_init(uint32_t bus_id, bool nss_enable, uint32_t nss_pol);
#endif //__MIMXRT_HAL_H__
