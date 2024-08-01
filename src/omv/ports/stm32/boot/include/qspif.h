/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * QSPI Flash driver.
 */
#ifndef __QSPIF_H__
#define __QSPIF_H__
int qspif_init();
int qspif_deinit();
int qspif_reset();
int qspif_read(uint8_t *buf, uint32_t addr, uint32_t size);
int qspif_write(uint8_t *buf, uint32_t addr, uint32_t size);
int qspif_erase_block(uint32_t addr);
int qspif_erase_chip();
int qspif_memory_test();
#endif //__QSPIF_H__
