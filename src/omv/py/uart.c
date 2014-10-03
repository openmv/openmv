/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * UART Python module.
 *
 */
#include "mp.h"
#include "pincfg.h"
#include "uart.h"
#define UART_TIMEOUT   (1000)
UART_HandleTypeDef UARTHandle;

static mp_obj_t py_uart_read(mp_obj_t obj)
{
    mp_buffer_info_t bufinfo;

    if (MP_OBJ_IS_INT(obj)) {
        bufinfo.len = mp_obj_get_int(obj);
        bufinfo.typecode = 'B';
        mp_obj_str_builder_start(&mp_type_bytes, bufinfo.len, (byte**)&(bufinfo.buf));
    } else {
        mp_get_buffer_raise(obj, &bufinfo, MP_BUFFER_WRITE);
    }

    mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    if (HAL_UART_Receive(&UARTHandle, bufinfo.buf, bufinfo.len, UART_TIMEOUT) != HAL_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_Exception, "HAL_UART_Receive failed"));
    }
    MICROPY_END_ATOMIC_SECTION(atomic_state);

    return mp_obj_new_int(bufinfo.len);
}

static mp_obj_t py_uart_write(mp_obj_t obj)
{
    byte buf[1];
    mp_buffer_info_t bufinfo;

    if (MP_OBJ_IS_INT(obj)) {
        buf[0] = mp_obj_get_int(obj);
        bufinfo.buf = buf;
        bufinfo.len = 1;
        bufinfo.typecode = 'B';
    } else {
        mp_get_buffer_raise(obj, &bufinfo, MP_BUFFER_READ);
    }

    mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    if (HAL_UART_Transmit(&UARTHandle, bufinfo.buf, bufinfo.len, UART_TIMEOUT) != HAL_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_Exception, "HAL_UART_Transmit failed"));
    }
    MICROPY_END_ATOMIC_SECTION(atomic_state);

    return mp_obj_new_int(bufinfo.len);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_uart_read_obj,   py_uart_read);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_uart_write_obj,  py_uart_write);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_UART) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read),      (mp_obj_t)&py_uart_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write),     (mp_obj_t)&py_uart_write_obj },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t uart_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_UART,
    .globals = (mp_obj_t)&globals_dict,
};

const mp_obj_module_t *py_uart_init()
{
    UARTHandle.Instance        = UARTx;
    UARTHandle.Init.BaudRate   = 38400;
    UARTHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UARTHandle.Init.StopBits   = UART_STOPBITS_1;
    UARTHandle.Init.Parity     = UART_PARITY_NONE;
    UARTHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UARTHandle.Init.Mode       = UART_MODE_TX_RX;

    if (HAL_UART_Init(&UARTHandle) != HAL_OK) {
        return NULL;
    }

    return &uart_module;
}
