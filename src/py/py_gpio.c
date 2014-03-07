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
    const char *sym;
    int val;
    GPIO_TypeDef* port;
    uint32_t pin;
} gpio_t;

/* GPIOs */
static gpio_t gpio_constants[] = {
    {"PC9",   GPIO_PC9 , GPIOC, GPIO_Pin_9 },
    {"PC10",  GPIO_PC10, GPIOC, GPIO_Pin_10},
    {"PC11",  GPIO_PC11, GPIOC, GPIO_Pin_11},
    {"PC12",  GPIO_PC12, GPIOC, GPIO_Pin_12},
    {"PA8",   GPIO_PA8 , GPIOA, GPIO_Pin_8 },
    {"PA15",  GPIO_PA15, GPIOA, GPIO_Pin_15},
    {NULL, 0}
};

typedef struct _py_gpio_obj_t {
    mp_obj_base_t base;
    gpio_t *info;
} py_gpio_obj_t;

mp_obj_t py_gpio_low(py_gpio_obj_t *gpio)
{
    GPIO_ResetBits(gpio->info->port, gpio->info->pin);
    return mp_const_none;
}

mp_obj_t py_gpio_high(py_gpio_obj_t *gpio)
{
    GPIO_SetBits(gpio->info->port, gpio->info->pin);
    return mp_const_none;
}

void py_gpio_print(void (*print)(void *env, const char *fmt, ...), void *env, mp_obj_t self_in, mp_print_kind_t kind)
{
    print(env, "<gpio>");
}

static MP_DEFINE_CONST_FUN_OBJ_1(py_gpio_low_obj,  py_gpio_low);
static MP_DEFINE_CONST_FUN_OBJ_1(py_gpio_high_obj, py_gpio_high);

static const mp_method_t py_gpio_methods[] = {
    { "low",   &py_gpio_low_obj},
    { "high",  &py_gpio_high_obj},
    { NULL, NULL },
};

static const mp_obj_type_t py_gpio_type = {
    { &mp_type_type },
    .name       = MP_QSTR_gpio,
    .print      = py_gpio_print,
    .methods    = py_gpio_methods,
};

static void gpio_init(gpio_t *gpio)
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

mp_obj_t py_gpio_init()
{
    /* Enable GPIO clocks */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    /* Create module */
    mp_obj_t m = mp_obj_new_module(qstr_from_str("gpio"));

    /* Store module functions */
    rt_store_attr(m, qstr_from_str("GPIO"), rt_make_function_n(1, py_gpio_new));

    /* Store module constants */
    for (gpio_t *p = gpio_constants; p->sym != NULL; p++) {
        rt_store_attr(m, QSTR_FROM_STR_STATIC(p->sym), MP_OBJ_NEW_SMALL_INT((machine_int_t)p->val));
    }
    return m;
}
