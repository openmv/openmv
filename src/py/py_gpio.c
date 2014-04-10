#include <stm32f4xx.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_spi.h>
#include <stm32f4xx_gpio.h>
#include <libmp.h>
#include "py_assert.h"
#include "py_gpio.h"
typedef enum {
    GPIO_PC9,
    GPIO_PC10,
    GPIO_PC11,
    GPIO_PC12,
    GPIO_PA8,
    GPIO_PA15,
    GPIO_ID_MAX,
} gpio_id_t;

typedef struct {
    GPIO_TypeDef* port;
    uint32_t pin;
} gpio_t;

/* GPIOs */
static const gpio_t gpio_constants[] = {
    {GPIOC, GPIO_Pin_9 },
    {GPIOC, GPIO_Pin_10},
    {GPIOC, GPIO_Pin_11},
    {GPIOC, GPIO_Pin_12},
    {GPIOA, GPIO_Pin_8 },
    {GPIOA, GPIO_Pin_15},
    {NULL, 0}
};

typedef struct _py_gpio_obj_t {
    mp_obj_base_t base;
    const gpio_t *info;
} py_gpio_obj_t;

static mp_obj_t py_gpio_low(py_gpio_obj_t *gpio)
{
    GPIO_ResetBits(gpio->info->port, gpio->info->pin);
    return mp_const_none;
}

static mp_obj_t py_gpio_high(py_gpio_obj_t *gpio)
{
    GPIO_SetBits(gpio->info->port, gpio->info->pin);
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
    GPIO_InitTypeDef GPIO_InitStructure;
    /* Configure the GPIO pin */
    GPIO_InitStructure.GPIO_Pin   = gpio->pin;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(gpio->port, &GPIO_InitStructure);
}

mp_obj_t py_gpio_new(mp_obj_t id_obj)
{
    gpio_id_t id = mp_obj_get_int(id_obj);
    py_gpio_obj_t *gpio_obj=mp_const_none;

    if (id < GPIO_ID_MAX) {
        gpio_init(&gpio_constants[id]);
        gpio_obj = m_new_obj(py_gpio_obj_t);
        gpio_obj->base.type = &py_gpio_type;
        gpio_obj->info = &gpio_constants[id];
    }
    return gpio_obj;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_gpio_new_obj,   py_gpio_new);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_gpio) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P1),      MP_OBJ_NEW_SMALL_INT(GPIO_PC10)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_P2),      MP_OBJ_NEW_SMALL_INT(GPIO_PC11)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_P3),      MP_OBJ_NEW_SMALL_INT(GPIO_PC12)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_P4),      MP_OBJ_NEW_SMALL_INT(GPIO_PA15)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_P5),      MP_OBJ_NEW_SMALL_INT(GPIO_PC9 )},
    { MP_OBJ_NEW_QSTR(MP_QSTR_P6),      MP_OBJ_NEW_SMALL_INT(GPIO_PA8 )},
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
    /* Enable GPIO clocks */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    return &py_gpio_module;
}
