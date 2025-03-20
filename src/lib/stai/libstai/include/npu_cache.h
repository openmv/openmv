#ifndef NPU_CACHE_H
#define NPU_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32n6xx_hal.h"

void npu_cache_init(void);
void npu_cache_enable(void);
void npu_cache_disable(void);
void npu_cache_invalidate(void);
void npu_cache_clean_invalidate_range(uint32_t start_addr, uint32_t end_addr);
void npu_cache_clean_range(uint32_t start_addr, uint32_t end_addr);

#ifdef __cplusplus
}
#endif

#endif /* NPU_CACHE_H */
