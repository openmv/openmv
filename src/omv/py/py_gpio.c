/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * GPIO Python module.
 *
 */
#include "mp.h"
#include "pincfg.h"
#include "py_gpio.h"
#include "py_assert.h"
#define EXTI_MAX 16

typedef struct _py_gpio_obj_t {
    mp_obj_base_t base;
    const gpio_t *info;
} py_gpio_obj_t;

static mp_obj_t extint_vector[EXTI_MAX];
static const uint8_t nvic_irq_channel[EXTI_MAX] = {
    EXTI0_IRQn,
    EXTI1_IRQn,
    EXTI2_IRQn,
    EXTI3_IRQn,
    EXTI4_IRQn,
    EXTI9_5_IRQn,
    EXTI9_5_IRQn,
    EXTI9_5_IRQn,
    EXTI9_5_IRQn,
    EXTI9_5_IRQn,
    EXTI15_10_IRQn,
    EXTI15_10_IRQn,
    EXTI15_10_IRQn,
    EXTI15_10_IRQn,
    EXTI15_10_IRQn,
    EXTI15_10_IRQn,
};

static void gpio_init(const gpio_t *gpio)
{
    /* Configure the GPIO pin */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pin   = gpio->pin;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(gpio->port, &GPIO_InitStructure);
}

void gpio_init_exti(const gpio_t *gpio, mp_obj_t cb, uint32_t priority, uint32_t sub_priority)
{
    /* Configure the GPIO pin */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pin   = gpio->pin;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_IT_FALLING;
    HAL_GPIO_Init(gpio->port, &GPIO_InitStructure);

    int line = 32-__CLZ(gpio->pin)-1;
    printf("line %d\n", line);
    // add cb to exti vector
    extint_vector[line] = cb;

    /* Enable and set NVIC Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(nvic_irq_channel[line], priority, sub_priority);
    HAL_NVIC_EnableIRQ(nvic_irq_channel[line]);
}

const mp_obj_module_t *py_gpio_init()
{
    for (int i=0; i<EXTI_MAX; i++) {
        extint_vector[i] = mp_const_none;
    }
    return &gpio_module;
}

static mp_obj_t py_gpio_low(py_gpio_obj_t *gpio)
{
    HAL_GPIO_WritePin(gpio->info->port, gpio->info->pin, GPIO_PIN_RESET);
    return mp_const_none;
}

static mp_obj_t py_gpio_high(py_gpio_obj_t *gpio)
{
    HAL_GPIO_WritePin(gpio->info->port, gpio->info->pin, GPIO_PIN_SET);
    return mp_const_none;
}

static void py_gpio_print(void (*print)(void *env, const char *fmt, ...), void *env, mp_obj_t self_in, mp_print_kind_t kind)
{
    print(env, "<gpio>");
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_gpio_low_obj,  py_gpio_low);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_gpio_high_obj, py_gpio_high);

static const mp_map_elem_t locals_dict_table[] = {
    {MP_OBJ_NEW_QSTR(MP_QSTR_low),   (mp_obj_t)&py_gpio_low_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_high),  (mp_obj_t)&py_gpio_high_obj},
    {NULL, NULL },
};

STATIC MP_DEFINE_CONST_DICT(locals_dict, locals_dict_table);

static const mp_obj_type_t py_gpio_type = {
    { &mp_type_type },
    .name  = MP_QSTR_gpio,
    .print = py_gpio_print,
    .locals_dict = (mp_obj_t)&locals_dict,
};

mp_obj_t py_gpio_new(mp_obj_t id_obj)
{
    gpio_id_t id = mp_obj_get_int(id_obj);
    py_gpio_obj_t *gpio_obj=mp_const_none;

    if (id < GPIO_ID_MAX) {
        gpio_init(&gpio_pins[id]);
        gpio_obj = m_new_obj(py_gpio_obj_t);
        gpio_obj->base.type = &py_gpio_type;
        gpio_obj->info = &gpio_pins[id];
    }
    return gpio_obj;
}

mp_obj_t py_gpio_exti(mp_obj_t id_obj, mp_obj_t cb_obj)
{
    gpio_id_t id = mp_obj_get_int(id_obj);
    if (id < GPIO_ID_MAX) {
        gpio_init_exti(&gpio_pins[id], cb_obj, 0x0F, 0x0F);
    }
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_gpio_new_obj,   py_gpio_new);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_gpio_exti_obj,   py_gpio_exti);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_gpio) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_GPIO),    (mp_obj_t)&py_gpio_new_obj  },
    { MP_OBJ_NEW_QSTR(MP_QSTR_EXTI),    (mp_obj_t)&py_gpio_exti_obj },
      GPIO_PINS_QSTR, //exports pin names
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t gpio_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_gpio,
    .globals = (mp_obj_t)&globals_dict,
};

// EXTI handler
void Handle_EXTI_Irq(uint32_t line)
{
    if (__HAL_GPIO_EXTI_GET_FLAG(1 << line)) {
        __HAL_GPIO_EXTI_CLEAR_FLAG(1 << line);
        mp_obj_t cb = extint_vector[line];
        if (cb != mp_const_none) {
            // When executing code within a handler we must lock the GC to prevent
            // any memory allocations.  We must also catch any exceptions.
            gc_lock();
            nlr_buf_t nlr;
            if (nlr_push(&nlr) == 0) {
                mp_call_function_1(cb, MP_OBJ_NEW_SMALL_INT(line));
                nlr_pop();
            } else {
                // Uncaught exception; disable the callback so it doesn't run again.
                extint_vector[line] = mp_const_none;
                HAL_NVIC_DisableIRQ(nvic_irq_channel[line]);
                printf("Uncaught exception in ExtInt interrupt handler line %lu\n", line);
                mp_obj_print_exception((mp_obj_t)nlr.ret_val);
            }
            gc_unlock();
        }
    }
}
