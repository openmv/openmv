#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include STM32_HAL_H
#include "fmath.h"
#include "stm_pwm.h"

typedef struct _tim_info {
    uint32_t period;
    uint32_t pulse;
} tim_info_t;

static uint32_t stm_tim_get_source_clock(TIM_TypeDef *inst) {
    uint32_t source = 0;
    #if defined(STM32F4) || defined(STM32F7) || defined(STM32H7)
    uintptr_t base = ((uintptr_t) inst) & 0xFFFF0000u;
    #endif

    #if defined(STM32F4) || defined(STM32F7)
    // Timer clock on F4, F7, H7 == APBx * 2.
    if (base == APB1PERIPH_BASE) {
        source = HAL_RCC_GetPCLK1Freq() * 2;
    } else if (base == APB2PERIPH_BASE) {
        source = HAL_RCC_GetPCLK2Freq() * 2;
    }
    #elif defined(STM32H7)
    // Timer clock on F4, F7, H7 == APBx * 2.
    if (base == D2_APB1PERIPH_BASE) {
        source = HAL_RCC_GetPCLK1Freq() * 2;
    } else if (base == D2_APB2PERIPH_BASE) {
        source = HAL_RCC_GetPCLK2Freq() * 2;
    }
    #elif defined(STM32N6)
    source = HAL_RCC_GetSysClockFreq() >> LL_RCC_GetTIMPrescaler();
    #endif

    return source;
}

static void stm_tim_calc_period_pulse(TIM_TypeDef *inst, uint32_t frequency,
                                      uint32_t *period, uint32_t *pulse) {
    uint32_t tclk = stm_tim_get_source_clock(inst);

    *period = fast_ceilf(tclk / ((float) frequency)) - 1;
    *pulse = (*period + 1) / 2;
}

int stm_pwm_start(TIM_HandleTypeDef *tim, TIM_TypeDef *inst, uint32_t channel, uint32_t frequency) {
    if (frequency == 0) {
        // If frequency == 0, stop the timer.
        stm_pwm_stop(tim, channel);
    } else if (tim->Instance) {
        // The timer has been initialized, update the frequency and return.
        if (stm_pwm_set_frequency(tim, channel, frequency)) {
            return -1;
        }
    } else {
        // Otherwise, initialize timer and start it.
        uint32_t period, pulse;

        // Calculate period and pulse.
        stm_tim_calc_period_pulse(inst, frequency, &period, &pulse);

        // Timer base configuration
        tim->Instance = inst;
        tim->Init.Period = period;
        tim->Init.Prescaler = 0;
        tim->Init.CounterMode = TIM_COUNTERMODE_UP;
        tim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        tim->Init.RepetitionCounter = 0;
        tim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

        // Timer channel configuration
        TIM_OC_InitTypeDef TIMOCHandle;
        TIMOCHandle.Pulse = pulse;
        TIMOCHandle.OCMode = TIM_OCMODE_PWM1;
        TIMOCHandle.OCPolarity = TIM_OCPOLARITY_HIGH;
        TIMOCHandle.OCNPolarity = TIM_OCNPOLARITY_HIGH;
        TIMOCHandle.OCFastMode = TIM_OCFAST_DISABLE;
        TIMOCHandle.OCIdleState = TIM_OCIDLESTATE_RESET;
        TIMOCHandle.OCNIdleState = TIM_OCNIDLESTATE_RESET;

        if (HAL_TIM_PWM_Init(tim) != HAL_OK ||
            HAL_TIM_PWM_ConfigChannel(tim, &TIMOCHandle, channel) != HAL_OK ||
            HAL_TIM_PWM_Start(tim, channel) != HAL_OK) {
            return -1;
        }
    }
    return 0;
}

int stm_pwm_stop(TIM_HandleTypeDef *tim, uint32_t channel) {
    if (tim->Instance) {
        HAL_TIM_PWM_Stop(tim, channel);
        HAL_TIM_PWM_DeInit(tim);
        memset(tim, 0, sizeof(TIM_HandleTypeDef));
    }
    return 0;
}

int stm_pwm_set_frequency(TIM_HandleTypeDef *tim, uint32_t channel, uint32_t frequency) {
    uint32_t period, pulse;

    if (tim->Instance) {
        // Calculate period and pulse.
        stm_tim_calc_period_pulse(tim->Instance, frequency, &period, &pulse);

        __HAL_TIM_SET_AUTORELOAD(tim, period);
        __HAL_TIM_SET_COMPARE(tim, channel, pulse);
    }

    return 0;
}

uint32_t stm_pwm_get_frequency(TIM_HandleTypeDef *tim, uint32_t channel) {
    if (tim->Instance) {
        uint32_t tclk = stm_tim_get_source_clock(tim->Instance);
        return tclk / (tim->Init.Period + 1);
    }

    return 0;
}

