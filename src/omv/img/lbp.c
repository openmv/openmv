/*
* This file is part of the OpenMV project.
* Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
* This work is licensed under the MIT license, see the file LICENSE for details.
*
* LBPu2⁄8,2 Operator.
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "imlib.h"
#include "xalloc.h"
#include "ff.h"

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define LBP_HIST_SIZE   (59)    //58 uniform hist + 1
#define LBP_DESC_SIZE   (4956)

//const static float lbp_weights [49]= {
//    2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f,
//    2.0f, 4.0f, 4.0f, 1.0f, 4.0f, 4.0f, 2.0f,
//    1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
//    0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
//    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
//    0.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 0.0f,
//    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
//};

const static uint8_t uniform_tbl[256] = {
    0, 1, 2, 3, 4, 58, 5, 6, 7, 58, 58, 58, 8, 58, 9, 10,
    11, 58, 58, 58, 58, 58, 58, 58, 12, 58, 58, 58, 13, 58, 14, 15,
    16, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    17, 58, 58, 58, 58, 58, 58, 58, 18, 58, 58, 58, 19, 58, 20, 21,
    22, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    23, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    24, 58, 58, 58, 58, 58, 58, 58, 25, 58, 58, 58, 26, 58, 27, 28,
    29, 30, 58, 31, 58, 58, 58, 32, 58, 58, 58, 58, 58, 58, 58, 33,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 34,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 35,
    36, 37, 58, 38, 58, 58, 58, 39, 58, 58, 58, 58, 58, 58, 58, 40,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 41,
    42, 43, 58, 44, 58, 58, 58, 45, 58, 58, 58, 58, 58, 58, 58, 46,
    47, 48, 58, 49, 58, 58, 58, 50, 51, 52, 58, 53, 54, 55, 56, 57
};

//(56, 94, 24, 24)
void imlib_lbp_desc(image_t *image, int div, uint8_t *desc, rectangle_t *roi)
{
    int RY = roi->h/div;
    int RX = roi->w/div;
    int s = image->w; //stride
    int h = (roi->y+RY*div)-3;
    int w = (roi->x+RX*div)-3;
    uint8_t *data = image->data;

    for (int y=roi->y; y<h; y++) {
        int y_idx = (y-roi->y)/RY*div;
        for (int x=roi->x; x<w; x++) {
            uint8_t lbp=0;
            int hist_idx = y_idx+(x-roi->x)/RX;
            uint8_t p = data[(y+1)*s+x+1];

            lbp |= (data[(y+0)*s+x+0] >= p) << 0;
            lbp |= (data[(y+0)*s+x+1] >= p) << 1;
            lbp |= (data[(y+0)*s+x+2] >= p) << 2;
            lbp |= (data[(y+1)*s+x+2] >= p) << 3;
            lbp |= (data[(y+2)*s+x+2] >= p) << 4;
            lbp |= (data[(y+2)*s+x+1] >= p) << 5;
            lbp |= (data[(y+2)*s+x+0] >= p) << 6;
            lbp |= (data[(y+1)*s+x+0] >= p) << 7;

            desc[hist_idx*LBP_HIST_SIZE+uniform_tbl[lbp]]++;
        }
    }
}

uint8_t *imlib_lbp_cascade(image_t *image, rectangle_t *roi)
{
    uint8_t *desc = xalloc0(LBP_DESC_SIZE);

    int offset =0;
    for (int i=1, c=0; i<=7; i+=2, c++) {
        imlib_lbp_desc(image, i, desc+offset, roi);
        offset += i*i*LBP_HIST_SIZE;
    }

    return desc;
}

#if 0
int imlib_lbp_desc_distance(uint8_t *d0, uint8_t *d1)
{
    uint32_t sum = 0;
    float thresh = 0.15f;
    int stages[] = {1*1*59, 3*3*59, 5*5*59, 7*7*59};

    for (int i=0, s=0; i<LBP_DESC_SIZE; i++) {
        if (i== stages[s]) {
            s++;
            thresh += 0.25f;
        }
        sum += (((d0[i] - d1[i]) * (d0[i] - d1[i]))/max((d0[i] + d1[i]), 1)) * thresh;
    }
    return sum;
}
#else
int imlib_lbp_desc_distance(uint8_t *d0, uint8_t *d1)
{
    int size=LBP_DESC_SIZE;

    int32_t num = 0;
    int32_t den_a=0;
    int32_t den_b=0;
    int32_t m_a = 0;
    int32_t m_b = 0;

    for (int i=0; i<size; i++) {
        m_a += d0[i];
        m_b += d1[i];
    }

    m_a /= size;
    m_b /= size;

    for (int i=0; i<size; i++) {
        int32_t x =((int)d0[i]) - m_a;
        int32_t y =((int)d1[i]) - m_b;
        num += x*y;
        den_a += x*x;
        den_b += y*y;
    }
    return (num/(fast_sqrtf(den_a) * fast_sqrtf(den_b)))*100;
}
#endif

int imlib_lbp_desc_save(FIL *fp, uint8_t *desc)
{
    UINT bytes;
    // Write descriptor
    return f_write(fp, desc, LBP_DESC_SIZE, &bytes);
}

int imlib_lbp_desc_load(FIL *fp, uint8_t **desc)
{
    UINT bytes;
    FRESULT res=FR_OK;

    *desc = NULL;
    uint8_t *hist = xalloc(LBP_DESC_SIZE);

    // Read descriptor
    res = f_read(fp, hist, LBP_DESC_SIZE, &bytes);
    if (res != FR_OK || bytes != LBP_DESC_SIZE) {
        *desc = NULL;
        xfree(hist);
    } else {
        *desc = hist;
    }

    return res;
}
