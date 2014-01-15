#ifndef __RCC_CTRL_H__
#define __RCC_CTRL_H__
enum sysclk_freq {
    SYSCLK_42_MHZ=0,
    SYSCLK_84_MHZ,
    SYSCLK_168_MHZ,
    SYSCLK_200_MHZ,
    SYSCLK_240_MHZ,
};
void rcc_ctrl_set_frequency(enum sysclk_freq);
#endif //__RCC_CTRL_H__
