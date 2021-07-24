/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * NINA-W10 driver BSP implementation.
 */
#if MICROPY_PY_NINAW10
#include <stdint.h>
#include <string.h>
#include "py/mphal.h"

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "omv_boardconfig.h"

#include "nina.h"
#include "nina_bsp.h"

#define debug_printf(...) mp_printf(&mp_plat_print, __VA_ARGS__)

int nina_bsp_init()
{
    gpio_init(WIFI_CS_PIN);
    gpio_set_dir(WIFI_CS_PIN, GPIO_OUT);

    gpio_init(WIFI_ACK_PIN);
    gpio_set_dir(WIFI_ACK_PIN, GPIO_IN);
    gpio_set_pulls(WIFI_ACK_PIN, true, false);

    gpio_init(WIFI_RST_PIN);
    gpio_set_dir(WIFI_RST_PIN, GPIO_OUT);

    gpio_init(WIFI_GPIO0_PIN);
    gpio_set_dir(WIFI_GPIO0_PIN, GPIO_OUT);

    gpio_init(WIFI_SCLK_PIN);
    gpio_set_function(WIFI_SCLK_PIN, GPIO_FUNC_SPI);

    gpio_init(WIFI_MOSI_PIN);
    gpio_set_function(WIFI_MOSI_PIN, GPIO_FUNC_SPI);

    gpio_init(WIFI_MISO_PIN);
    gpio_set_function(WIFI_MISO_PIN, GPIO_FUNC_SPI);

    // Reset module in WiFi mode
    gpio_put(WIFI_CS_PIN, 1);
    gpio_put(WIFI_GPIO0_PIN, 1);

    gpio_put(WIFI_RST_PIN, 0);
    mp_hal_delay_ms(100);

    gpio_put(WIFI_RST_PIN, 1);
    mp_hal_delay_ms(750);

    gpio_put(WIFI_GPIO0_PIN, 0);
    gpio_set_dir(WIFI_GPIO0_PIN, GPIO_IN);
    gpio_set_pulls(WIFI_GPIO0_PIN, true, false);

    // Initialize SPI.
    spi_init(WIFI_SPI, 8 * 1000 * 1000);
    spi_set_slave(WIFI_SPI, false);
    spi_set_format(WIFI_SPI, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    return 0;
}

int nina_bsp_reset()
{
    return 0;
}

int nina_bsp_spi_slave_select(uint32_t timeout)
{
    // Wait for ACK to go low.
    for (mp_uint_t start = mp_hal_ticks_ms(); gpio_get(WIFI_ACK_PIN) == 1; mp_hal_delay_ms(1)) {
        if ((mp_hal_ticks_ms() - start) >= timeout) {
            return -1;
        }
    }

    // Chip select.
    gpio_put(WIFI_CS_PIN, 0);

    // Wait for ACK to go high.
    for (mp_uint_t start = mp_hal_ticks_ms(); gpio_get(WIFI_ACK_PIN) == 0; mp_hal_delay_ms(1)) {
        if ((mp_hal_ticks_ms() - start) >= 100) {
            gpio_put(WIFI_CS_PIN, 1);
            return -1;
        }
    }

    return 0;
}

int nina_bsp_spi_slave_deselect()
{
    gpio_put(WIFI_CS_PIN, 1);
    return 0;
}

int nina_bsp_spi_transfer(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t size)
{
    int rsize = 0;
    if (tx_buf && rx_buf) {
        rsize = spi_write_read_blocking(WIFI_SPI, tx_buf, rx_buf, size);
    } else if (tx_buf) {
        rsize = spi_write_blocking(WIFI_SPI, tx_buf, size);
    } else if (rx_buf) {
        rsize = spi_read_blocking(WIFI_SPI, 0xFF, rx_buf, size);
    }
    #if NINA_DEBUG
    for (int i=0; i<size; i++) {
        if (tx_buf) {
            debug_printf("0x%x ", tx_buf[i]);
        } else {
            debug_printf("0x%x ", rx_buf[i]);
        }
    }
    #endif
    return ((rsize == size) ? 0 : -1);
}
#endif //MICROPY_PY_NINAW10
