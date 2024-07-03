/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Buzzer Python module.
 */
#include "py/obj.h"

#include "omv_boardconfig.h"

#if MICROPY_PY_BUZZER
#include STM32_HAL_H
#include "omv_gpio.h"

static TIM_HandleTypeDef buzzer_tim_handle = {};

static int buzzer_freq = OMV_BUZZER_FREQ;
static int buzzer_duty = 0;

static void buzzer_setup(int freq, int duty) {
    int tclk = OMV_BUZZER_TIM_PCLK_FREQ() * 2;
    int period = (tclk / freq) - 1;
    int pulse = (period * duty) / 510;

    if (((buzzer_duty <= 0) || (255 < buzzer_duty)) && (0 < duty) && (duty <= 255)) {
        omv_gpio_config(OMV_BUZZER_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);

        buzzer_tim_handle.Instance = OMV_BUZZER_TIM;
        buzzer_tim_handle.Init.Prescaler = 0;
        buzzer_tim_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
        buzzer_tim_handle.Init.Period = period;
        buzzer_tim_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        buzzer_tim_handle.Init.RepetitionCounter = 0;
        buzzer_tim_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

        TIM_OC_InitTypeDef buzzer_tim_oc_handle;
        buzzer_tim_oc_handle.Pulse = pulse;
        buzzer_tim_oc_handle.OCMode = TIM_OCMODE_PWM1;
        buzzer_tim_oc_handle.OCPolarity = TIM_OCPOLARITY_HIGH;
        buzzer_tim_oc_handle.OCNPolarity = TIM_OCNPOLARITY_HIGH;
        buzzer_tim_oc_handle.OCFastMode = TIM_OCFAST_DISABLE;
        buzzer_tim_oc_handle.OCIdleState = TIM_OCIDLESTATE_RESET;
        buzzer_tim_oc_handle.OCNIdleState = TIM_OCNIDLESTATE_RESET;

        HAL_TIM_PWM_Init(&buzzer_tim_handle);
        HAL_TIM_PWM_ConfigChannel(&buzzer_tim_handle, &buzzer_tim_oc_handle, OMV_BUZZER_TIM_CHANNEL);
        HAL_TIM_PWM_Start(&buzzer_tim_handle, OMV_BUZZER_TIM_CHANNEL);
    } else if ((0 < buzzer_duty) && (buzzer_duty <= 255) && ((duty <= 0) || (255 < duty))) {
        HAL_TIM_PWM_Stop(&buzzer_tim_handle, OMV_BUZZER_TIM_CHANNEL);
        HAL_TIM_PWM_DeInit(&buzzer_tim_handle);
        omv_gpio_deinit(OMV_BUZZER_PIN);
    } else if ((0 < buzzer_duty) && (buzzer_duty <= 255) && (0 < duty) && (duty <= 255)) {
        __HAL_TIM_SET_AUTORELOAD(&buzzer_tim_handle, period);
        __HAL_TIM_SET_COMPARE(&buzzer_tim_handle, OMV_BUZZER_TIM_CHANNEL, pulse);
    }

    buzzer_freq = freq;
    buzzer_duty = duty;
}

static mp_obj_t py_buzzer_freq(uint n_args, const mp_obj_t *args) {
    if (!n_args) {
        return mp_obj_new_int(buzzer_freq);
    } else {
        buzzer_setup(mp_obj_get_int(*args), buzzer_duty);
        return mp_const_none;
    }
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_buzzer_freq_obj, 0, 1, py_buzzer_freq);

static mp_obj_t py_buzzer_duty(uint n_args, const mp_obj_t *args) {
    if (!n_args) {
        return mp_obj_new_int(buzzer_duty);
    } else {
        buzzer_setup(buzzer_freq, mp_obj_get_int(*args));
        return mp_const_none;
    }
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_buzzer_duty_obj, 0, 1, py_buzzer_duty);

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_ROM_QSTR(MP_QSTR_buzzer) },
    { MP_ROM_QSTR(MP_QSTR_RESONANT_FREQ),   MP_ROM_INT(OMV_BUZZER_FREQ) },
    { MP_ROM_QSTR(MP_QSTR_freq),            MP_ROM_PTR(&py_buzzer_freq_obj) },
    { MP_ROM_QSTR(MP_QSTR_duty),            MP_ROM_PTR(&py_buzzer_duty_obj) }
};

static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t buzzer_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

void py_buzzer_init0() {
    buzzer_setup(OMV_BUZZER_FREQ, 0);
}

MP_REGISTER_MODULE(MP_QSTR_buzzer, buzzer_module);
#endif // MICROPY_PY_BUZZER
