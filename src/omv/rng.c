#include <stm32f4xx_hal.h>
#include "rng.h"

static RNG_HandleTypeDef RNGHandle;

void rng_init()
{
    __RNG_CLK_ENABLE();
    RNGHandle.Instance = RNG;
    HAL_RNG_Init(&RNGHandle);
}

uint32_t rng_randint(uint32_t min, uint32_t max)
{
    uint32_t rand=0;
    if (min==max) {
        return 0;
    }

    // Wait until the RNG is ready
    while (HAL_RNG_GetState(&RNGHandle) != RNG_FLAG_DRDY);

    rand = HAL_RNG_GetRandomNumber(&RNGHandle);
    return (rand%(max-min))+min;
}
