#ifndef __XALLOC_H__
#define __XALLOC_H__
#include <stdint.h>
void *xalloc(size_t size);
void *xalloc0(size_t size);
void xfree(void *ptr);
void *xrealloc(void *ptr, size_t size);
#endif /* __XALLOC_H__ */
