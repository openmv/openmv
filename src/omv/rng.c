#include <stm32f4xx_rng.h>
#include <stm32f4xx_rcc.h>
#include "rng.h"

void rng_init()
{
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
    RNG_Cmd(ENABLE);
}

uint32_t rng_randint(uint32_t min, uint32_t max)
{
    uint32_t rand;
    if (min==max) {
        return 0;
    }
    rand = RNG_GetRandomNumber();
    return (rand%(max-min))+min;
}
