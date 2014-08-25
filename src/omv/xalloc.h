#ifndef __XALLOC_H__
#define __XALLOC_H__
#include <stdint.h>
void xalloc_init();
void *xalloc(uint32_t size);
void *xalloc0(uint32_t size);
void xfree(void *ptr);
void *xrealloc(void *ptr, uint32_t size);
#endif /* __XALLOC_H__ */
