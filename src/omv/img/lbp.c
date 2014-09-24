/*
* This file is part of the OpenMV project.
* Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
* This work is licensed under the MIT license, see the file LICENSE for details.
*
* LBPu2⁄8,2 Operator.
*
*/
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
#define LBP_NUM_REGIONS (7)     //7x7 regions
#define LBP_DESC_SIZE   (LBP_NUM_REGIONS*LBP_NUM_REGIONS*LBP_HIST_SIZE)

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

uint8_t *imlib_lbp_desc(image_t *image)
{
    int w = image->w;
    int h = image->h;
    int RY = h/LBP_NUM_REGIONS;
    int RX = w/LBP_NUM_REGIONS;
    uint8_t *data = image->data;
    uint8_t *desc = xalloc0(LBP_DESC_SIZE);

    for (int y=0; y<h-3; y++) {
        int y_idx = (y/RY*(w/RX));
        for (int x=0; x<w-3; x++) {
            uint8_t lbp=0;
            int hist_idx = y_idx+x/RX;
            uint8_t p = data[(y+1)*w+x+1];

            lbp |= (data[(y+0)*w+x+0] >= p) << 0;
            lbp |= (data[(y+0)*w+x+1] >= p) << 1;
            lbp |= (data[(y+0)*w+x+2] >= p) << 2;
            lbp |= (data[(y+1)*w+x+2] >= p) << 3;
            lbp |= (data[(y+2)*w+x+2] >= p) << 4;
            lbp |= (data[(y+2)*w+x+1] >= p) << 5;
            lbp |= (data[(y+2)*w+x+0] >= p) << 6;
            lbp |= (data[(y+1)*w+x+0] >= p) << 7;

            desc[hist_idx*LBP_HIST_SIZE+uniform_tbl[lbp]]++;
        }
    }
    return desc;
}

int imlib_lbp_desc_distance(uint8_t *d0, uint8_t *d1)
{
    uint32_t sum = 0;
    for (int i=0; i<LBP_DESC_SIZE; i++) {
        sum += (((d0[i] - d1[i]) * (d0[i] - d1[i]))/max((d0[i] + d1[i]),1));
    }
    return sum;
}

int imlib_lbp_desc_load(const char *path, uint8_t **desc)
{
    FIL fp;
    UINT bytes;
    FRESULT res=FR_OK;

    *desc = NULL;
    uint8_t *hist = xalloc0(LBP_DESC_SIZE);

    /* open LBP desc file */
    res = f_open(&fp, path, FA_READ|FA_OPEN_EXISTING);
    if (res != FR_OK) {
        return res;
    }

    /* read descriptor */
    res = f_read(&fp, hist, LBP_DESC_SIZE, &bytes);
    if (res != FR_OK || bytes != LBP_DESC_SIZE) {
        *desc = NULL;
        xfree(hist);
    } else {
        *desc = hist;
    }

    f_close(&fp);
    return res;
}
