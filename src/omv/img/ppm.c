/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * PPM/PGM reader/writer.
 *
 */
#include <ff.h>
#include <stdio.h>
#include "xalloc.h"
#include "imlib.h"
#include "mdefs.h"

#define BUF_LEN   (1024*3)

#define R8(p) \
    rb_tbl[((p>>3)&0x1F)]

#define G8(p) \
    g_tbl[((p&0x07)<<3)|(p>>13)]

#define B8(p) \
    rb_tbl[((p>>8)&0x1F)]

static uint8_t rb_tbl []= {
    0, 8, 16, 24, 32, 41, 49, 57, 65, 74,
    82, 90, 98, 106, 115, 123, 131, 139,
    148, 156, 164, 172, 180, 189, 197, 205,
    213, 222, 230, 238, 246, 255
};

static uint8_t g_tbl []= {
    0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44,
    48, 52, 56, 60, 64, 68, 72, 76, 80, 85, 89,
    93, 97, 101, 105, 109, 113, 117, 121, 125, 129,
    133, 137, 141, 145, 149, 153, 157, 161, 165, 170,
    174, 178, 182, 186, 190, 194, 198, 202, 206, 210,
    214, 218, 222, 226, 230, 234, 238, 242, 246, 250, 255
};

static int isspace(int c)
{
    return (c=='\n'||c=='\r'||c=='\t'||c==' ');
}

static char f_getc(FIL *fp)
{
    char c;
    UINT bytes;
    FRESULT res = f_read(fp, &c, 1, &bytes);
    if (res != FR_OK || bytes != 1) {
        c = EOF;
    }
    return c;
}

static DISABLE_OPT int read_num(FIL *fp)
{
    int x=0, c=0;
    while (1) {
        c = f_getc(fp);
        if (c == '#') {
            /* skip comments */
            while ((c = f_getc(fp)) != EOF && c != '\n');
        } else if (c >= 48 && c <= 57) {
            x *= 10;
            x += (c-48);
        } else {
            break;
        }
    }
    return x;
}

int ppm_write(image_t *img, const char *path)
{
    FIL fp;
    UINT bytes;
    FRESULT res=FR_OK;

    int idx=0;
    uint8_t buf[BUF_LEN];

    res = f_open(&fp, path, FA_WRITE|FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        return res;
    }

    if (img->bpp == 1) {
        bytes = f_printf(&fp, "P5\n%d %d\n255\n", img->w, img->h);
    } else {
        bytes = f_printf(&fp, "P6\n%d %d\n255\n", img->w, img->h);
    }

    if (bytes == -1) {
        res = FR_DENIED;
        goto error;
    }

    if (img->bpp == 1) {
        res = f_write(&fp, img->data, img->w*img->h, &bytes);
    } else {
        uint16_t *pixels = (uint16_t*)img->data;
        for (int i=0; i<img->w*img->h; i++) {
            uint16_t p = pixels[i];
            buf[idx++]=R8(p); buf[idx++]=G8(p); buf[idx++]=B8(p); //RGB565->RGB888
            if (idx == BUF_LEN) {
                idx = 0;
                res = f_write(&fp, buf, BUF_LEN, &bytes);
                if (res != FR_OK || bytes != BUF_LEN) {
                    goto error;
                }
            }
        }
        if (idx) {
            res = f_write(&fp, buf, idx, &bytes);
            if (res != FR_OK || bytes != idx) {
                goto error;
            }
        }
    }

error:
    f_close(&fp);
    return res;
}

int ppm_read(image_t *img, const char *path)
{
    FIL fp;
    UINT bytes;
    FRESULT res=FR_OK;

    res = f_open(&fp, path, FA_READ|FA_OPEN_EXISTING);
    if (res != FR_OK) {
        return res;
    }

    /* read image header */
    if (f_getc(&fp) != 'P') {
        printf("ppm:image format not supported\n");
        res = -1;
        goto error;
    }

    int type = f_getc(&fp);
    if (type != '5' && type != '6') {
        printf("ppm:image format not supported\n");
        res = -1;
        goto error;
    }

    if (!isspace(f_getc(&fp))) {
        printf("ppm:image format not supported\n");
        res = -1;
        goto error;
    }

    img->w = read_num(&fp); /* read image width */
    img->h = read_num(&fp); /* read image height */
    int max_gray = read_num(&fp); /* read image max gray */
    if (img->w == -1 || img->h == -1 || max_gray != 255) {
        printf("ppm:image format not supported\n");
        res = -1;
        goto error;
    }

    img->bpp = (type=='5')? 1:2;
    int size = img->w*img->h*img->bpp;

    /* alloc image */
    img->data = xalloc(size);
    if (img->data == NULL) {
        printf("ppm: out of memory\n");
        res = -1;
        goto error;
    }

    /* read image data */
    res = f_read(&fp, img->data, size, &bytes);
    if (res != FR_OK || bytes != size) {
        goto error;
    }

error:
    f_close(&fp);
    return res;
}

int ppm_write_subimg(image_t *img, const char *path, rectangle_t *r)
{
    FIL fp;
    UINT bytes;
    FRESULT res=FR_OK;

    int idx=0;
    uint8_t buf[BUF_LEN];

    res = f_open(&fp, path, FA_WRITE|FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        return res;
    }

    if (img->bpp == 1) {
        bytes = f_printf(&fp, "P5\n%d %d\n255\n", r->w, r->h);
    } else {
        bytes = f_printf(&fp, "P6\n%d %d\n255\n", r->w, r->h);
    }

    if (bytes == -1) {
        res = FR_DENIED;
        goto error;
    }

    if (img->bpp == 1) {
        for (int j=r->y; j<r->y+r->h; j++) {
            for (int i=r->x; i<r->x+r->w; i++) {
                uint8_t c = img->pixels[j*img->w+i];
                res = f_write(&fp, &c, 1, &bytes);
                if (res != FR_OK || bytes != 1) {
                    goto error;
                }
            }
        }
    } else {
        uint16_t *pixels = (uint16_t*)img->data;
        for (int j=r->y; j<r->y+r->h; j++) {
            for (int i=r->x; i<r->x+r->w; i++) {
                uint16_t p = pixels[j*img->w+i];
                buf[idx++]=R8(p); buf[idx++]=G8(p); buf[idx++]=B8(p); //RGB565->RGB888
                if (idx == BUF_LEN) {
                    idx = 0;
                    res = f_write(&fp, buf, BUF_LEN, &bytes);
                    if (res != FR_OK || bytes != BUF_LEN) {
                        goto error;
                    }
                }
            }
        }

        if (idx) {
            res = f_write(&fp, buf, idx, &bytes);
            if (res != FR_OK || bytes != idx) {
                goto error;
            }
        }
    }

error:
    f_close(&fp);
    return res;
}
