/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * ToF Module.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <mp.h>
#include <math.h>
#include STM32_HAL_H
#include <float.h>
#include "fb_alloc.h"
#include "xalloc.h"
#include "py_assert.h"
#include "py_image.h"
#include "py_helper.h"
#include "systick.h"
#include "py_tof.h"

#define I2C_TIMEOUT             (1000)
#define TOF_SLAVE_ADDR          (0x58<<1)

#define TOF_VDIN_PIN            (GPIO_PIN_12)
#define TOF_VDIN_PORT           (GPIOD)

#define TOF_STDBY_PIN           (GPIO_PIN_13)
#define TOF_STDBY_PORT          (GPIOD)

#define TOF_RESET_PIN           (GPIO_PIN_5)
#define TOF_RESET_PORT          (GPIOA)

#define GPIO_READ(port, pin)    HAL_GPIO_ReadPin(port, pin)
#define GPIO_LOW(port, pin)     HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
#define GPIO_HIGH(port, pin)    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)

static SPI_HandleTypeDef SPIHandle;
static I2C_HandleTypeDef I2CHandle;

static void spi_config()
{
    // Enable SPI clock
    __SPI2_CLK_ENABLE();

    // SPI pins configuration
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStructure.Alternate = GPIO_AF5_SPI2;
    
    GPIO_InitStructure.Pin = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = GPIO_PIN_15;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    // SPI configuration
    SPIHandle.Instance               = SPI2;
    SPIHandle.Init.Mode              = SPI_MODE_SLAVE;
    SPIHandle.Init.Direction         = SPI_DIRECTION_2LINES_RXONLY;
    SPIHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SPIHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
    SPIHandle.Init.CLKPhase          = SPI_PHASE_2EDGE;
    SPIHandle.Init.NSS               = SPI_NSS_HARD_INPUT;
    SPIHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    SPIHandle.Init.FirstBit          = SPI_FIRSTBIT_LSB;
    SPIHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
    SPIHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    SPIHandle.Init.CRCPolynomial     = 7;

    // Initialize the SPI
    if (HAL_SPI_Init(&SPIHandle) != HAL_OK) {
        // Initialization error
        nlr_jump(mp_obj_new_exception_msg(&mp_type_RuntimeError, "ToF SPI init failed!!"));
    }
}

static void i2c_init()
{
    // Enable I2C clock
    __I2C2_CLK_ENABLE();

    // I2C pins configuration
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStructure.Alternate = GPIO_AF4_I2C2;

    GPIO_InitStructure.Pin = GPIO_PIN_10;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = GPIO_PIN_11;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    // I2C configuration
    I2CHandle.Instance             = I2C2;
    I2CHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    #if defined(STM32F769xx)
    I2CHandle.Init.Timing          = 0x20404768; // 10KHz
    #else
    I2CHandle.Init.ClockSpeed      = 10000;
    I2CHandle.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    #endif
    I2CHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
    I2CHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    I2CHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLED;
    I2CHandle.Init.OwnAddress1     = 0xFE;
    I2CHandle.Init.OwnAddress2     = 0xFE;

    // Initialize I2C
    if (HAL_I2C_Init(&I2CHandle) != HAL_OK) {
        // Initialization error
        nlr_jump(mp_obj_new_exception_msg(&mp_type_RuntimeError, "ToF I2C init failed!!"));
    }
}

int i2c_write_bytes(uint8_t slv_addr, uint8_t *buf, int len, bool stop)
{
    int ret=0;

    __disable_irq();
    if(HAL_I2C_Master_Transmit(&I2CHandle, slv_addr, buf, len, I2C_TIMEOUT) != HAL_OK) {
        ret = -1;
    }
    __enable_irq();
    return ret;
}

int i2c_read_bytes(uint8_t slv_addr, uint8_t mem_addr, uint8_t *buf, int len, bool stop)
{
    int ret=0;

    __disable_irq();
    if (HAL_I2C_Mem_Read(&I2CHandle, slv_addr, mem_addr, I2C_MEMADD_SIZE_8BIT, buf, len, I2C_TIMEOUT) != HAL_OK) {
        ret = -1;
    }
    __enable_irq();
    return ret;
}

static mp_obj_t py_tof_deinit()
{
    return mp_const_none;
}

mp_obj_t py_tof_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    // Init I2C
    i2c_init();

    // Init SPI
    spi_config();

    // Configure GPIOs
    // VDIN pin
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pin = TOF_VDIN_PIN;
    GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
    GPIO_LOW(TOF_VDIN_PORT, TOF_VDIN_PIN);
    HAL_GPIO_Init(TOF_VDIN_PORT, &GPIO_InitStructure);

    // Standby
    GPIO_InitStructure.Pin = TOF_STDBY_PIN;
    GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
    GPIO_LOW(TOF_STDBY_PORT, TOF_STDBY_PIN);
    HAL_GPIO_Init(TOF_STDBY_PORT, &GPIO_InitStructure);

    // Reset
    GPIO_InitStructure.Pin = TOF_RESET_PIN;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
    GPIO_HIGH(TOF_RESET_PORT, TOF_RESET_PIN);
    HAL_GPIO_Init(TOF_RESET_PORT, &GPIO_InitStructure);

    // Reset cycle
    GPIO_LOW(TOF_RESET_PORT, TOF_RESET_PIN);
    systick_sleep(10);
    GPIO_HIGH(TOF_RESET_PORT, TOF_RESET_PIN);

    // Initialization
    // Enable timing generator
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x80, 0x01, 0x00, 0x00}, 4, true);
    // Disable standby mode
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x08, 0x00, 0x00, 0x00}, 4, true);
    systick_sleep(100);
    // Disable timing generator
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x80, 0x00, 0x00, 0x00}, 4, true);

    // Set defaults INIT_0, INIT_1, INIT_2
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x12, 0x0A, 0x00, 0x00}, 4, true);
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x1B, 0x0A, 0x00, 0x00}, 4, true);
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x0B, 0x00, 0x20, 0x00}, 4, true);

    // Config register (Update Select = 0x00, slave mode = 1)
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x81, 0x01, 0x00, 0x00}, 4, true);

    // Internal illumination
    // Step = 5ma, Curr = 20xsteps, Bias = 10xsteps (150mA on, 50mA off)
    //i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x39, 0xA0, 0x00, 0x00}, 4, true);
    //i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x3B, 0x00, 0x40, 0x0A}, 4, true);

    // Step = 5ma, Curr = 30xsteps, Bias = 0xsteps (150mA on, 0mA off)
    //i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x39, 0x00, 0x00, 0x00}, 4, true);
    //i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x3B, 0x00, 0x40, 0x0F}, 4, true);

    // Step = 5ma, Curr = 15xsteps, Bias = 15xsteps (150mA on, 75mA off)
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x39, 0xF0, 0x00, 0x00}, 4, true);
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x3B, 0x00, 0xC0, 0x07}, 4, true);

    // Enable dyn power, Re-arrange data
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x5C, 0x00, 0x10, 0x34}, 4, true);

    // Enable shutter and easy conf
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x5B, 0x00, 0x00, 0xC0}, 4, true);

    // De-Aliasing
    //i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0xFB, 0x09, 0xE8, 0x01}, 4, true);

    // Enable test pattern
    //i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x6C, 0x44, 0xA4, 0x00}, 4, true);
    //i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0xD9, 0x0C, 0x40, 0x00}, 4, true);

    // OP Mode: 1-lane SSI, OP_CLK(second byte) 24MHz = 0x00, 12MHz = 0x02, 6MHz = 0x04, 3MHz = 0x06
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0xDE, 0x10, 0x02, 0x00}, 4, true);

    // Enable timing generator
    i2c_write_bytes(TOF_SLAVE_ADDR, (uint8_t [4]){0x80, 0x01, 0x00, 0x00}, 4, true);

    uint8_t buf[3];
    i2c_read_bytes(TOF_SLAVE_ADDR, 0x6C, buf, 3, true);
    printf("buf: %x %x %x\n", buf[0], buf[1], buf[2]);
    return mp_const_none;
}

static mp_obj_t py_tof_write_reg(mp_obj_t addr, mp_obj_t vals_obj) {
    mp_obj_t *array;
    mp_obj_get_array_fixed_n(vals_obj, 3, &array);

    uint8_t vals[4] = {
        mp_obj_get_int(addr),
        mp_obj_get_int(array[0]),
        mp_obj_get_int(array[1]),
        mp_obj_get_int(array[2])
    };

    i2c_write_bytes(TOF_SLAVE_ADDR, vals, 4, true);
    return mp_const_none;
}

static void read_frame(uint8_t *buf, uint32_t buf_size)
{
    __disable_irq();
    // Pulse VDIN
    GPIO_HIGH(TOF_VDIN_PORT, TOF_VDIN_PIN);
    GPIO_LOW(TOF_VDIN_PORT, TOF_VDIN_PIN);

    do {
        if (HAL_SPI_Receive(&SPIHandle, buf, 1, 1000) != HAL_OK) {
            __enable_irq();
            nlr_jump(mp_obj_new_exception_msg(&mp_type_RuntimeError, "ToF error reading frame!!"));
        }
    } while (buf[0] != 0xFF);

    do {
        if (HAL_SPI_Receive(&SPIHandle, buf, 1, 1000) != HAL_OK) {
            __enable_irq();
            nlr_jump(mp_obj_new_exception_msg(&mp_type_RuntimeError, "ToF error reading frame!!"));
        }
    } while (buf[0] == 0xFF);

    if (HAL_SPI_Receive(&SPIHandle, buf+1, buf_size-1, 1000) != HAL_OK) {
        __enable_irq();
        nlr_jump(mp_obj_new_exception_msg(&mp_type_RuntimeError, "ToF error reading frame!!"));
    }

    __enable_irq();
}

mp_obj_t py_tof_read_frame()
{
    int buf_size = 80*60*4;
    uint8_t *buf = fb_alloc0(buf_size);

    read_frame(buf, buf_size);

    uint16_t *pix =(uint16_t*) buf;
    for (int y=0; y<6; y++) {
        for (int x=0; x<10; x++, pix+=2) {
            // Phase
            uint32_t p;
            p = (uint32_t)pix[1] & 0xFFF;
            printf("%lu,", p);
        }
        printf("\n");
    }
    fb_free();
    return mp_const_none;
}

mp_obj_t py_tof_draw_frame(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    int buf_size = 80*60*4;
    uint8_t *buf = fb_alloc0(buf_size);

    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img), "Operation not supported on JPEG");
    //int draw_phase = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_phase), 0);

    read_frame(buf, buf_size);

    uint16_t *pix =(uint16_t*) buf;
    for (int y=0; y<60; y++) {
        for (int x=0; x<80; x++, pix+=2) {
            // Phase
            uint32_t p;
            // Amplitude
            p = (uint32_t)(pix[0]&0x0FFF);
            IM_SET_GS_PIXEL(arg_img, x, y, (p*255)/4095);
            // Phase
            p = (uint32_t)(pix[1]&0x0FFF);
            IM_SET_GS_PIXEL(arg_img, x+80, y, ((p*255)/4095));
            // Ambient
            p = ((uint32_t)pix[0]&0xF000) >> 12;
            IM_SET_GS_PIXEL(arg_img, x, y+60, p*255/15);
        }
    }
    fb_free();
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tof_init_obj,  0,  py_tof_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tof_deinit_obj,     py_tof_deinit);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tof_read_frame_obj, py_tof_read_frame);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tof_draw_frame_obj, 1, py_tof_draw_frame);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_tof_write_reg_obj, py_tof_write_reg);
static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),    MP_OBJ_NEW_QSTR(MP_QSTR_tof)  },
    { MP_OBJ_NEW_QSTR(MP_QSTR_init),        (mp_obj_t)&py_tof_init_obj    },
    { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),      (mp_obj_t)&py_tof_deinit_obj  },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_frame),  (mp_obj_t)&py_tof_read_frame_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_draw_frame),  (mp_obj_t)&py_tof_draw_frame_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR___write_reg), (mp_obj_t)&py_tof_write_reg_obj },
    { NULL, NULL },
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t tof_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_tof,
    .globals = (mp_obj_t)&globals_dict,
};

void py_tof_init0()
{
}
