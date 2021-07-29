/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * NINA-W10 driver BSP.
 */
#ifndef __NINA_BSP_H__
#define __NINA_BSP_H__
int nina_bsp_init();
int nina_bsp_deinit();
int nina_bsp_read_irq();
int nina_bsp_spi_slave_select(uint32_t timeout);
int nina_bsp_spi_slave_deselect();
int nina_bsp_spi_transfer(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t size);
#endif //define __NINA_BSP_H__
