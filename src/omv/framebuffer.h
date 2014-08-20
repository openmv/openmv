#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__
#include "mutex.h"
extern char _fb_base;
static struct framebuffer {
    int w;
    int h;
    int bpp;
    int ready;
    mutex_t lock;
    uint8_t pixels[];
}*fb = (struct framebuffer *) &_fb_base;
#endif /* __FRAMEBUFFER_H__ */
