#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE inline __attribute__((always_inline))
#endif

#ifndef BREAK
#define BREAK() __asm__ volatile ("BKPT")
#endif

#ifndef DISABLE_OPT
#define DISABLE_OPT __attribute__((optimize("O0")))
#endif
