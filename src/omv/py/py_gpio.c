#include "mp.h"
#include "pincfg.h"
#include "py_gpio.h"
#include "py_assert.h"

typedef struct _py_gpio_obj_t {
    mp_obj_base_t base;
    const gpio_t *info;
} py_gpio_obj_t;

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

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_gpio_new_obj,   py_gpio_new);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_gpio) },
      GPIO_PINS_QSTR,
    { MP_OBJ_NEW_QSTR(MP_QSTR_GPIO),    (mp_obj_t)&py_gpio_new_obj },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t py_gpio_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_gpio,
    .globals = (mp_obj_t)&globals_dict,
};

const mp_obj_module_t *py_gpio_init()
{
    /* no init required */
    return &py_gpio_module;
}
