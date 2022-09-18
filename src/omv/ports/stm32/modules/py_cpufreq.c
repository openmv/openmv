/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * CPU frequency scaling module.
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "py/obj.h"
#include "py/runtime.h"

#include "py_cpufreq.h"
#include "py_helper.h"
#include "omv_boardconfig.h"
#include STM32_HAL_H

#if defined(STM32F7) || defined(STM32H7)

#if defined(STM32H7)
#define N_FREQUENCIES   (4)
static const uint32_t CPUFREQ_FREQS_REV_V [N_FREQUENCIES] = {60, 120, 240, 480};
static const uint32_t CPUFREQ_FREQS_REV_XY[N_FREQUENCIES] = {50, 100, 200, 400};
#elif defined(STM32F7)
#define N_FREQUENCIES   (5)
static const uint32_t cpufreq_pllq[N_FREQUENCIES] = {5, 6, 7, 8, 9};
static const uint32_t cpufreq_freqs[N_FREQUENCIES] = {120, 144, 168, 192, 216};
static const uint32_t cpufreq_latency[N_FREQUENCIES] = { // Flash latency (see table 11)
    FLASH_LATENCY_3, FLASH_LATENCY_4, FLASH_LATENCY_5, FLASH_LATENCY_7, FLASH_LATENCY_7
};
#endif

#if defined(STM32H7)
static const uint32_t *cpufreq_get_frequencies()
{
    #if (OMV_MAX_CPU_FREQ == 400)
    (void)CPUFREQ_FREQS_REV_V;
    // If the maximum frequency is set to 400 use rev x/y frequencies.
    return CPUFREQ_FREQS_REV_XY;
    #else
    // Otherwise, determine the frequencies dynamically using the revid.
    if (HAL_GetREVID() >= 0x2003) {
        return CPUFREQ_FREQS_REV_V;
    } else {
        return CPUFREQ_FREQS_REV_XY;
    }
    #endif
}
#endif

static uint32_t cpufreq_get_cpuclk()
{
    uint32_t cpuclk = HAL_RCC_GetSysClockFreq();

    #if defined(STM32H7)
    uint32_t flatency;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &flatency);
    switch (RCC_ClkInitStruct.SYSCLKDivider) {
        case RCC_SYSCLK_DIV1:
            break;
        case RCC_SYSCLK_DIV2:
            cpuclk /= 2;
            break;
        case RCC_SYSCLK_DIV4:
            cpuclk /= 4;
            break;
        case RCC_SYSCLK_DIV8:
            cpuclk /= 8;
            break;
        default:
            break;
    }
    #endif
    return cpuclk;
}

mp_obj_t py_cpufreq_get_current_frequencies()
{
    mp_obj_t tuple[4] = {
        mp_obj_new_int(cpufreq_get_cpuclk()   / (1000000)),
        mp_obj_new_int(HAL_RCC_GetHCLKFreq()  / (1000000)),
        mp_obj_new_int(HAL_RCC_GetPCLK1Freq() / (1000000)),
        mp_obj_new_int(HAL_RCC_GetPCLK2Freq() / (1000000)),
    };
    return mp_obj_new_tuple(4, tuple);
}

mp_obj_t py_cpufreq_get_supported_frequencies()
{
    #if defined(STM32H7)
    const uint32_t *cpufreq_freqs = cpufreq_get_frequencies();
    #endif
    mp_obj_t freq_list = mp_obj_new_list(0, NULL);
    for (int i=0; i<N_FREQUENCIES; i++) {
        mp_obj_list_append(freq_list, mp_obj_new_int(cpufreq_freqs[i]));
    }
    return freq_list;
}

mp_obj_t py_cpufreq_set_frequency(mp_obj_t cpufreq_obj)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    #if defined(STM32F7)
    RCC_OscInitTypeDef RCC_OscInitStruct;
    #endif

    // Check if frequency is supported
    int cpufreq_idx = -1;
    uint32_t cpufreq = mp_obj_get_int(cpufreq_obj);
    #if defined(STM32H7)
    const uint32_t *cpufreq_freqs = cpufreq_get_frequencies();
    #endif
    for (int i=0; i<N_FREQUENCIES; i++) {
        if (cpufreq == cpufreq_freqs[i]) {
            cpufreq_idx = i;
            break;
        }
    }

    // Frequency is Not supported.
    if (cpufreq_idx == -1) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Unsupported frequency!"));
    }

    // Return if frequency hasn't changed.
    if (cpufreq == (cpufreq_get_cpuclk()/(1000000))) {
        return mp_const_true;
    }

    #if defined(STM32H7)
    uint32_t flatency = FLASH_LATENCY_2;
    RCC_ClkInitStruct.SYSCLKSource  = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.ClockType     = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
            RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

    switch (cpufreq) {
        case 50:
        case 60:
            RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV8; // D1CPRE
            RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV1;   // HPRE
            RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;   // D2PPRE1
            RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;   // D2PPRE2
            RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;   // D1PPRE
            RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;   // D3PPRE
            break;

        case 100:
        case 120:
            RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV4; // D1CPRE
            RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV1;   // HPRE
            RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;   // D2PPRE1
            RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;   // D2PPRE2
            RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;   // D1PPRE
            RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;   // D3PPRE
            break;

        case 200:
        case 240:
            RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV2; // D1CPRE
            RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV1;   // HPRE
            RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;   // D2PPRE1
            RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;   // D2PPRE2
            RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;   // D1PPRE
            RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;   // D3PPRE
            break;

        case 400:
        case 480:
            RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1; // D1CPRE
            RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;   // HPRE
            RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;   // D2PPRE1
            RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;   // D2PPRE2
            RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;   // D1PPRE
            RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;   // D3PPRE
            break;

        default:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Unsupported frequency!"));
            break;
    }

    #elif defined(STM32F7)
    // Select HSE as system clock source
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK |
            RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    // Configure the HCLK, PCLK1 and PCLK2 clocks dividers
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK) {
        // Initialization Error
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("RCC CLK Initialization Error!!"));
    }

    // Enable HSE Oscillator and activate PLL with HSE as source
    RCC_OscInitStruct.OscillatorType    = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState          = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState      = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource     = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM          = 12; // depends on HSE
    RCC_OscInitStruct.PLL.PLLN          = cpufreq_freqs[cpufreq_idx] * 2;
    RCC_OscInitStruct.PLL.PLLP          = 2;
    RCC_OscInitStruct.PLL.PLLQ          = cpufreq_pllq[cpufreq_idx];

    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        // Initialization Error
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("RCC OSC Initialization Error!!"));
    }

    // Select PLL as system clock source
    uint32_t flatency = cpufreq_latency[cpufreq_idx];
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    #endif

    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, flatency) != HAL_OK) {
        // Initialization Error
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("RCC CLK Initialization Error!!"));
    }
    return mp_const_true;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_cpufreq_set_frequency_obj, py_cpufreq_set_frequency);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_cpufreq_get_current_frequencies_obj, py_cpufreq_get_current_frequencies);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_cpufreq_get_supported_frequencies_obj, py_cpufreq_get_supported_frequencies);
#endif // defined(STM32F7) || defined(STM32H7)

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),                  MP_OBJ_NEW_QSTR(MP_QSTR_cpufreq) },
    #if defined(STM32F7) || defined(STM32H7)
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_frequency),             (mp_obj_t)&py_cpufreq_set_frequency_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_current_frequencies),   (mp_obj_t)&py_cpufreq_get_current_frequencies_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_supported_frequencies), (mp_obj_t)&py_cpufreq_get_supported_frequencies_obj },
    #else
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_frequency),             (mp_obj_t)&py_func_unavailable_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_current_frequencies),   (mp_obj_t)&py_func_unavailable_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_supported_frequencies), (mp_obj_t)&py_func_unavailable_obj },
    #endif
    { NULL, NULL },
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t cpufreq_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t)&globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_cpufreq, cpufreq_module);
