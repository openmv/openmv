#ifndef __FONT_H__
#define __FONT_H__
typedef struct {
    int w;
    int h;
    uint8_t data[10];
} glyph_t;
extern const glyph_t font[95];
#endif //__FONT_H__
