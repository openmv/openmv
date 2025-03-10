#ifndef __CMSIS_MCU_H__
#define __CMSIS_MCU_H__

#ifndef PI
#define PI               3.14159265358979f
#endif

#ifndef M_PI
#define M_PI             3.14159265f
#define M_PI_2           1.57079632f
#define M_PI_4           0.78539816f
#endif

#define typeof __typeof__

inline void __builtin_arm_wfi() {
};

inline void __builtin_arm_dmb(uint32_t v) {
};

inline void __enable_irq() {
};

inline void __disable_irq() {
};

inline uint32_t __builtin_arm_rbit(uint32_t v) {
    uint8_t sc = 31U;
    uint32_t r = v;
    for (v >>= 1U; v; v >>= 1U) {
        r <<= 1U;
        r |= v & 1U;
        sc--;
    }
    return (r << sc);
}

#endif