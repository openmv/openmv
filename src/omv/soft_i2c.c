/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Software I2C implementation.
 *
 */
#include <stdbool.h>
#include <stm32f4xx_hal.h>
#include "soft_i2c.h"

#define I2C_PORT            GPIOA
#define I2C_SIOC_PIN        GPIO_PIN_2
#define I2C_SIOD_PIN        GPIO_PIN_3

#define I2C_SIOC_H()        HAL_GPIO_WritePin(I2C_PORT, I2C_SIOC_PIN, GPIO_PIN_SET)
#define I2C_SIOC_L()        HAL_GPIO_WritePin(I2C_PORT, I2C_SIOC_PIN, GPIO_PIN_RESET)

#define I2C_SIOD_H()        HAL_GPIO_WritePin(I2C_PORT, I2C_SIOD_PIN, GPIO_PIN_SET)
#define I2C_SIOD_L()        HAL_GPIO_WritePin(I2C_PORT, I2C_SIOD_PIN, GPIO_PIN_RESET)

#define I2C_SIOD_READ()     HAL_GPIO_ReadPin(I2C_PORT, I2C_SIOD_PIN)
#define I2C_SIOD_WRITE(bit) HAL_GPIO_WritePin(I2C_PORT, I2C_SIOD_PIN, bit);

#define ACK 0
#define NACK 1

static void delay(void)
{
    volatile uint32_t i;
    for (i=0; i<18; i++);
}

static void i2c_start(void)
{
    /* The start of data transmission occurs when
       SIO_D is driven low while SIO_C is high */
    I2C_SIOC_H();
    I2C_SIOD_H();
    delay();

    I2C_SIOD_L();
    delay();

    I2C_SIOC_L();
    delay();
}

static void i2c_stop(void)
{
    /* The stop of data transmission occurs when
       SIO_D is driven high while SIO_C is high */
    I2C_SIOC_L();
    I2C_SIOD_L();
    delay();

    I2C_SIOC_H();
    delay();

    I2C_SIOD_H();
    delay();
}

static uint8_t i2c_read_byte(char ack)
{
    uint8_t data = 0;

    for(char i = 0; i < 8; i++) {
        I2C_SIOC_H();
        delay();

        data |= I2C_SIOD_READ()&0x01;
        data <<= (i != 7);

        I2C_SIOC_L();
        if (i == 7) {
            /* Write ACK */
            I2C_SIOD_WRITE(ack);
        }
        delay();
    }

    I2C_SIOC_H();
    delay();

    I2C_SIOC_L();
    I2C_SIOD_H();
    delay();
    return data;
}

static char i2c_write_byte(uint8_t data)
{
    char i;

    /* Shift the 8 bits out */
    for (i=0; i<8; i++) {
        if (data & 0x80) {
            I2C_SIOD_H();
        } else {
            I2C_SIOD_L();
        }

        I2C_SIOC_H();
        delay();

        I2C_SIOC_L();
        delay();

        data <<= 1;
    }

    I2C_SIOC_H();
    delay();

    /* Read ACK */
    i = I2C_SIOD_READ()&0x01;

    I2C_SIOC_L();
    delay();

    I2C_SIOD_H();
    return i;
}

int soft_i2c_read_bytes(uint8_t slv_addr, uint8_t *buf, int len, bool stop)
{
    int ret = 0;

    __disable_irq();
    i2c_start();
    ret |= i2c_write_byte(slv_addr | 0x01);
    for (int i=0; i<len; i++) {
        buf[i] = i2c_read_byte(ACK);
    }
    if (stop) {
        i2c_stop();
    }
    __enable_irq();
    return ret;
}

int soft_i2c_write_bytes(uint8_t slv_addr, uint8_t *buf, int len, bool stop)
{
    uint8_t ret = 0;

    __disable_irq();
    i2c_start();
    ret |= i2c_write_byte(slv_addr);
    for (int i=0; i<len; i++) {
        ret |= i2c_write_byte(*buf++);
    }

    if (stop) {
        i2c_stop();
    }
    __enable_irq();
    return ret;
}

void soft_i2c_init()
{
    /* Conigure I2C GPIOs */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_OD;

    GPIO_InitStructure.Pin = I2C_SIOC_PIN;
    HAL_GPIO_Init(I2C_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = I2C_SIOD_PIN;
    HAL_GPIO_Init(I2C_PORT, &GPIO_InitStructure);

    I2C_SIOC_H();
    I2C_SIOD_H();
    delay();
}
