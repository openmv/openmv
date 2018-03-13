/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * CPU frequency module.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <mp.h>
#include <math.h>
#include STM32_HAL_H
#include "py_cpufreq.h"

enum cpufreq_freqs {
    CPUFREQ_120MHZ=0,
    CPUFREQ_144MHZ=1,
    CPUFREQ_168MHZ=2,
    CPUFREQ_192MHZ=3,
    CPUFREQ_216MHZ=4,
    CPUFREQ_MAX
};

static const uint32_t cpufreq_pllq[] = {5, 6, 7, 8, 9};
static const uint32_t cpufreq_freq[] = {120, 144, 168, 192, 216};
static const uint32_t cpufreq_latency[] = { // Flash latency (see table 11)
    FLASH_LATENCY_3, FLASH_LATENCY_4, FLASH_LATENCY_5, FLASH_LATENCY_7, FLASH_LATENCY_7
};

void py_cpufreq_init0()
{

}

mp_obj_t py_cpufreq_get_frequency()
{
    mp_obj_t tuple[4] = {
        mp_obj_new_int(HAL_RCC_GetSysClockFreq()),
        mp_obj_new_int(HAL_RCC_GetHCLKFreq()),
        mp_obj_new_int(HAL_RCC_GetPCLK1Freq()),
        mp_obj_new_int(HAL_RCC_GetPCLK2Freq()),
    };
    return mp_obj_new_tuple(4, tuple);
}

mp_obj_t py_cpufreq_set_frequency(mp_obj_t cpufreq_idx_obj)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    // Check CPU frequency index range
    uint32_t cpufreq_idx = mp_obj_get_int(cpufreq_idx_obj);
    if (cpufreq_idx < 0 || cpufreq_idx >= CPUFREQ_MAX) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Unsupported frequency!"));
    }

    // Return if frequency hasn't changed 
    if (cpufreq_freq[cpufreq_idx] == (HAL_RCC_GetSysClockFreq()/1000000)) {
        return mp_const_true;
    }

    // Select HSE as system clock source
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK |
            RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    // Configure the HCLK, PCLK1 and PCLK2 clocks dividers
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK) {
        // Initialization Error
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "RCC CLK Initialization Error!!"));
    }

    // Enable HSE Oscillator and activate PLL with HSE as source
    RCC_OscInitStruct.OscillatorType    = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState          = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState      = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource     = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM          = 12; // depends on HSE
    RCC_OscInitStruct.PLL.PLLN          = cpufreq_freq[cpufreq_idx] * 2;
    RCC_OscInitStruct.PLL.PLLP          = 2;
    RCC_OscInitStruct.PLL.PLLQ          = cpufreq_pllq[cpufreq_idx];

    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        // Initialization Error
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "RCC OSC Initialization Error!!"));
    }

    // Select PLL as system clock source
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, cpufreq_latency[cpufreq_idx]) != HAL_OK) {
        // Initialization Error
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "RCC CLK Initialization Error!!"));
    }

    // Do a soft-reset ?
    //nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Frequency is set!"));
    return mp_const_true;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_cpufreq_get_frequency_obj, py_cpufreq_get_frequency);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_cpufreq_set_frequency_obj, py_cpufreq_set_frequency);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_cpufreq)        },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CPUFREQ_120MHZ),  MP_OBJ_NEW_SMALL_INT(CPUFREQ_120MHZ)    },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CPUFREQ_144MHZ),  MP_OBJ_NEW_SMALL_INT(CPUFREQ_144MHZ)    },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CPUFREQ_168MHZ),  MP_OBJ_NEW_SMALL_INT(CPUFREQ_168MHZ)    },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CPUFREQ_192MHZ),  MP_OBJ_NEW_SMALL_INT(CPUFREQ_192MHZ)    },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CPUFREQ_216MHZ),  MP_OBJ_NEW_SMALL_INT(CPUFREQ_216MHZ)    },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_frequency),   (mp_obj_t)&py_cpufreq_get_frequency_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_frequency),   (mp_obj_t)&py_cpufreq_set_frequency_obj },
    { NULL, NULL },
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t cpufreq_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t)&globals_dict,
};
