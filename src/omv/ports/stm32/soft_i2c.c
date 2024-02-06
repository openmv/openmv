/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Software I2C implementation.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mphal.h"

#include "soft_i2c.h"
#include "omv_boardconfig.h"
#include "omv_gpio.h"

#if defined(OMV_SOFT_I2C_SIOC_PIN)
#define ACK     0
#define NACK    1

static void delay(void) {
    for (volatile int i = 0; i < OMV_SOFT_I2C_SPIN_DELAY; i++) {
        ;
    }
}

static void i2c_start(void) {
    /* The start of data transmission occurs when
       SIO_D is driven low while SIO_C is high */
    omv_gpio_write(OMV_SOFT_I2C_SIOD_PIN, 0);
    delay();
    omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 0);
    delay();
}

static void i2c_stop(void) {
    /* The stop of data transmission occurs when
       SIO_D is driven high while SIO_C is high */
    omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 1);
    delay();
    omv_gpio_write(OMV_SOFT_I2C_SIOD_PIN, 1);
    delay();
}

static uint8_t i2c_read_byte(char ack) {
    uint8_t data = 0;

    omv_gpio_write(OMV_SOFT_I2C_SIOD_PIN, 1);
    delay();

    for (char i = 0; i < 8; i++) {
        omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 1);
        delay();
        data += data + omv_gpio_read(OMV_SOFT_I2C_SIOD_PIN);
        delay();
        omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 0);
        delay();
    }

    /* Write ACK */
    omv_gpio_write(OMV_SOFT_I2C_SIOD_PIN, ack);
    delay();

    omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 1);
    delay();

    omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 0);
    delay();

    omv_gpio_write(OMV_SOFT_I2C_SIOD_PIN, 0);
    delay();
    return data;
}

static char i2c_write_byte(uint8_t data) {
    char i;

    for (i = 0; i < 8; i++) {
        omv_gpio_write(OMV_SOFT_I2C_SIOD_PIN, (data >> (7 - i)) & 1);
        delay();
        omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 1);
        delay();
        omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 0);
        delay();
    }

    omv_gpio_write(OMV_SOFT_I2C_SIOD_PIN, 1);
    delay();

    omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 1);
    delay();

    /* Read ACK */
    i = omv_gpio_read(OMV_SOFT_I2C_SIOD_PIN);
    delay();

    omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 0);
    delay();

    omv_gpio_write(OMV_SOFT_I2C_SIOD_PIN, 0);
    delay();
    return i;
}

int soft_i2c_read_bytes(uint8_t slv_addr, uint8_t *buf, int len, bool stop) {
    int ret = 0;
    mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    i2c_start();
    ret |= i2c_write_byte(slv_addr | 1);
    for (int i = 0; i < len; i++) {
        buf[i] = i2c_read_byte((i != (len - 1)) ? ACK : NACK);
    }
    if (stop) {
        i2c_stop();
    } else {
        omv_gpio_write(OMV_SOFT_I2C_SIOD_PIN, 1);
        delay();
        omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 1);
        delay();
    }
    MICROPY_END_ATOMIC_SECTION(atomic_state);
    return ret;
}

int soft_i2c_write_bytes(uint8_t slv_addr, uint8_t *buf, int len, bool stop) {
    int ret = 0;
    mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    i2c_start();
    ret |= i2c_write_byte(slv_addr);
    for (int i = 0; i < len; i++) {
        ret |= i2c_write_byte(buf[i]);
    }
    if (stop) {
        i2c_stop();
    } else {
        omv_gpio_write(OMV_SOFT_I2C_SIOD_PIN, 1);
        delay();
        omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 1);
        delay();
    }
    MICROPY_END_ATOMIC_SECTION(atomic_state);
    return ret;
}

void soft_i2c_init() {
    omv_gpio_write(OMV_SOFT_I2C_SIOC_PIN, 1); // Set first to prevent glitches.
    omv_gpio_config(OMV_SOFT_I2C_SIOC_PIN, OMV_GPIO_MODE_OUTPUT_OD, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);

    omv_gpio_write(OMV_SOFT_I2C_SIOD_PIN, 1); // Set first to prevent glitches.
    omv_gpio_config(OMV_SOFT_I2C_SIOD_PIN, OMV_GPIO_MODE_OUTPUT_OD, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);

    for (volatile int i = 0; i < 1000; i++) {
        ;
    }

    for (int j = 0; j < 127; j++) {
        // initialize bus
        soft_i2c_write_bytes(j << 1, NULL, 0, true);
    }
}

void soft_i2c_deinit() {
    for (volatile int i = 0; i < 1000; i++) {
        ;
    }
    omv_gpio_deinit(OMV_SOFT_I2C_SIOC_PIN);
    omv_gpio_deinit(OMV_SOFT_I2C_SIOD_PIN);
}
#endif // defined(OMV_SOFT_I2C_SIOC_PIN)
