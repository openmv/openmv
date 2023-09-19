/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * VOSPI driver.
 */
#ifndef __VOSPI_H__
#define __VOSPI_H__
int vospi_init(uint32_t n_packets, void *buffer);
int vospi_snapshot(uint32_t timeout_ms);
#endif // __VOSPI_H__
