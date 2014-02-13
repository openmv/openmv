#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__
extern char _main_ram_start;
static struct framebuffer {
    int w;
    int h;
    int bpp;
    uint8_t pixels[];
}*fb = (struct framebuffer *) &_main_ram_start;
#endif /* __FRAMEBUFFER_H__ */
