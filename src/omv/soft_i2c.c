/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Software I2C implementation.
 *
 */
#include <mp.h>
#include "soft_i2c.h"

#define I2C_PORT            GPIOB
#define I2C_SIOC_PIN        GPIO_PIN_10
#define I2C_SIOD_PIN        GPIO_PIN_11

#define I2C_SIOC_H()        HAL_GPIO_WritePin(I2C_PORT, I2C_SIOC_PIN, GPIO_PIN_SET)
#define I2C_SIOC_L()        HAL_GPIO_WritePin(I2C_PORT, I2C_SIOC_PIN, GPIO_PIN_RESET)

#define I2C_SIOD_H()        HAL_GPIO_WritePin(I2C_PORT, I2C_SIOD_PIN, GPIO_PIN_SET)
#define I2C_SIOD_L()        HAL_GPIO_WritePin(I2C_PORT, I2C_SIOD_PIN, GPIO_PIN_RESET)

#define I2C_SIOD_READ()     HAL_GPIO_ReadPin(I2C_PORT, I2C_SIOD_PIN)
#define I2C_SIOD_WRITE(bit) HAL_GPIO_WritePin(I2C_PORT, I2C_SIOD_PIN, bit);

#define ACK 0
#define NACK 1

static void delay(void) // TODO: Update with clock speed knowledge for M7.
{
    for(volatile int i=0; i<16; i++);
}

static void i2c_start(void)
{
    /* The start of data transmission occurs when
       SIO_D is driven low while SIO_C is high */
    I2C_SIOD_L();
    delay();
    I2C_SIOC_L();
    delay();
}

static void i2c_stop(void)
{
    /* The stop of data transmission occurs when
       SIO_D is driven high while SIO_C is high */
    I2C_SIOC_H();
    delay();
    I2C_SIOD_H();
    delay();
}

static uint8_t i2c_read_byte(char ack)
{
    uint8_t data = 0;

    I2C_SIOD_H();
    delay();

    for(char i=0; i<8; i++) {
        I2C_SIOC_H();
        delay();
        data += data + I2C_SIOD_READ();
        delay();
        I2C_SIOC_L();
        delay();
    }

    /* Write ACK */
    I2C_SIOD_WRITE(ack);
    delay();

    I2C_SIOC_H();
    delay();

    I2C_SIOC_L();
    delay();

    I2C_SIOD_L();
    delay();
    return data;
}

static char i2c_write_byte(uint8_t data)
{
    char i;

    for(i=0; i<8; i++) {
        I2C_SIOD_WRITE((data >> (7 - i)) & 1);
        delay();
        I2C_SIOC_H();
        delay();
        I2C_SIOC_L();
        delay();
    }

    I2C_SIOD_H();
    delay();

    I2C_SIOC_H();
    delay();

    /* Read ACK */
    i = I2C_SIOD_READ();
    delay();

    I2C_SIOC_L();
    delay();

    I2C_SIOD_L();
    delay();
    return i;
}

int soft_i2c_read_bytes(uint8_t slv_addr, uint8_t *buf, int len, bool stop)
{
    int ret = 0;
    mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    i2c_start();
    ret |= i2c_write_byte(slv_addr | 1);
    for(int i=0; i<len; i++) {
        buf[i] = i2c_read_byte(ACK);
    }
    if (stop) {
        i2c_stop();
    } else {
        I2C_SIOD_H();
        delay();
        I2C_SIOC_H();
        delay();
    }
    MICROPY_END_ATOMIC_SECTION(atomic_state);
    return ret;
}

int soft_i2c_write_bytes(uint8_t slv_addr, uint8_t *buf, int len, bool stop)
{
    int ret = 0;
    mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    i2c_start();
    ret |= i2c_write_byte(slv_addr);
    for(int i=0; i<len; i++) {
        ret |= i2c_write_byte(buf[i]);
    }
    if (stop) {
        i2c_stop();
    } else {
        I2C_SIOD_H();
        delay();
        I2C_SIOC_H();
        delay();
    }
    MICROPY_END_ATOMIC_SECTION(atomic_state);
    return ret;
}

void soft_i2c_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_OD;

    GPIO_InitStructure.Pin = I2C_SIOC_PIN;
    I2C_SIOC_H(); // Set first to prevent glitches.
    HAL_GPIO_Init(I2C_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = I2C_SIOD_PIN;
    I2C_SIOD_H(); // Set first to prevent glitches.
    HAL_GPIO_Init(I2C_PORT, &GPIO_InitStructure);

    for(volatile int i=0; i<1000; i++);

    for(int j=0; j<127; j++) { // initialize bus
        soft_i2c_write_bytes(j << 1, NULL, 0, true);
    }
}

void soft_i2c_deinit()
{
    for(volatile int i=0; i<1000; i++);
    HAL_GPIO_DeInit(I2C_PORT, I2C_SIOC_PIN);
    HAL_GPIO_DeInit(I2C_PORT, I2C_SIOD_PIN);
}
