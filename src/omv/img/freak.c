//	Copyright (C) 2011-2012  Signal processing laboratory 2, EPFL,
//	Raphael Ortiz (raphael.ortiz@a3.epfl.ch),
//	Kirell Benzi (kirell.benzi@epfl.ch)
//	Alexandre Alahi (alexandre.alahi@epfl.ch)
//	and Pierre Vandergheynst (pierre.vandergheynst@epfl.ch)
//
//  Redistribution and use in source and binary forms, with or without modification,
//  are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
//  This software is provided by the copyright holders and contributors "as is" and
//  any express or implied warranties, including, but not limited to, the implied
//  warranties of merchantability and fitness for a particular purpose are disclaimed.
//  In no event shall the Intel Corporation or contributors be liable for any direct,
//  indirect, incidental, special, exemplary, or consequential damages
//  (including, but not limited to, procurement of substitute goods or services;
//  loss of use, data, or profits; or business interruption) however caused
//  and on any theory of liability, whether in contract, strict liability,
//  or tort (including negligence or otherwise) arising in any way out of
//  the use of this software, even if advised of the possibility of such damage.

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "fmath.h"
#include "arm_math.h"
#include "ff.h"
#include "imlib.h"
#include "xalloc.h"

#undef PI
#undef PI_2
#undef min
#undef max

#define PI    (3.14159265f)
#define PI_2  (3.14159265f*2.0f)

#define min(a,b) \
   ({ __typeof__ (a) _a = (a);  \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define max(a,b) \
   ({ __typeof__ (a) _a = (a);  \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define kNB_SCALES          (64)
#define kNB_ORIENTATION     (256)
#define kNB_POINTS          (43)
#define kNB_PAIRS           (512)
#define kSMALLEST_KP_SIZE   (7)
#define kNB_ORIENPAIRS      (45)
#define NUM_OCTAVES         (4)
#define BIG_R               (2.0f/3.0f)                 // bigger radius
#define SMALL_R             (2.0f/24.0f)                // smaller radius
#define UNIT_SPACE          ((BIG_R-SMALL_R)/21.0f)     // spaces between concentric circles (from center to outer: 1,2,3,4,5,6)

#define SCALE_IDX           (0)
#define KP_SIZE             (23)
#define SCALE_STEP          (1.0330248790212284f)       // 2 ^ ( (nbOctaves-1) /nbScales)
#define SCALE_FACTOR        (1.0f)                      // SCALE_STEP ^ SCALE_IDX
#define PATTERN_SCALE       (22)

#define MAX_KP_DIST         (512)
#define KPT_DESC_SIZE       (kNB_PAIRS/8)

// number of points on each concentric circle (from outer to inner)
const static int n[8] = {6,6,6,6,6,6,6,1};

// radii of the concentric cirles (from outer to inner)
const static float radius[8] = {
        BIG_R,
        BIG_R-6*UNIT_SPACE,
        BIG_R-11*UNIT_SPACE,
        BIG_R-15*UNIT_SPACE,
        BIG_R-18*UNIT_SPACE,
        BIG_R-20*UNIT_SPACE,
        SMALL_R,
        0.0f
};

// sigma of pattern points (each group of 6 points on a concentric cirle has the same sigma)
const static float sigma[8] = {
        (BIG_R/2.0f),
        (BIG_R-6*UNIT_SPACE)/2.0f,
        (BIG_R-11*UNIT_SPACE)/2.0f,
        (BIG_R-15*UNIT_SPACE)/2.0f,
        (BIG_R-18*UNIT_SPACE)/2.0f,
        (BIG_R-20*UNIT_SPACE)/2.0f,
        (SMALL_R)/2.0f,
        (SMALL_R)/2.0f
};

const static int ORIENTATION_PAIRS[45][4] = {
	{0, 3, 140, 0},	        {1, 4, 70, 121},
	{2, 5, -69, 121},	    {0, 2, 140, -80},
	{1, 3, 140, 81},	    {2, 4, 0, 161},
	{3, 5, -139, 81},	    {4, 0, -139, -80},
	{5, 1, 0, -160},	    {6, 9, 161, 93},
	{7, 10, 0, 186},	    {8, 11, -160, 93},
	{6, 8, 215, 0},	        {7, 9, 107, 186},
	{8, 10, -106, 186},	    {9, 11, -214, 0},	
	{10, 6, -106, -185},    {11, 7, 107, -185},	
	{12, 15, 258, 0},	    {13, 16, 129, 223},	
	{14, 17, -128, 223},    {12, 14, 258, -148},	
	{13, 15, 258, 149},	    {14, 16, 0, 298},	
	{15, 17, -257, 149},	{16, 12, -257, -148},	
	{17, 13, 0, -297},	    {18, 21, 322, 186},	
	{19, 22, 0, 372},	    {20, 23, -321, 186},	
	{18, 20, 430, 0},	    {19, 21, 215, 372},	
	{20, 22, -214, 372},	{21, 23, -429, 0},	
	{22, 18, -214, -371},	{23, 19, 215, -371},	
	{24, 27, 559, 0},	    {25, 28, 279, 484},	
	{26, 29, -278, 484},	{30, 33, 726, 419},	
	{31, 34, 0, 838},	    {32, 35, -725, 419},	
	{36, 39, 1117, 0},	    {37, 40, 559, 967},	
	{38, 41, -558, 967},	
};

const static uint8_t DESCRIPTION_PAIRS[512][2] = {
	{28, 26},	{29, 25},	{40, 38},	{32, 15},	{19, 10},	{10, 7},	{25, 11},	{42, 13},	
	{39, 33},	{33, 15},	{38, 16},	{21, 20},	{29, 11},	{20, 15},	{5, 1},	    {33, 32},	
	{17, 13},	{23, 12},	{9, 3},	    {25, 6},	{18, 12},	{41, 37},	{22, 19},	{4, 2},	
	{11, 6},	{6, 0},	    {11, 0},	{38, 14},	{9, 8},	    {29, 6},	{34, 31},	{16, 14},	
	{39, 20},	{37, 29},	{36, 30},	{40, 2},	{35, 30},	{31, 22},	{33, 21},	{32, 20},	
	{23, 18},	{36, 35},	{39, 21},	{28, 14},	{19, 7},	{40, 16},	{39, 32},	{8, 3},	
	{37, 6},	{41, 25},	{33, 20},	{40, 14},	{37, 11},	{36, 24},	{22, 10},	{41, 11},	
	{21, 15},	{22, 7},	{41, 29},	{13, 5},	{37, 25},	{31, 19},	{41, 6},	{38, 4},	
	{16, 2},	{32, 21},	{34, 22},	{38, 28},	{26, 3},	{26, 14},	{34, 10},	{31, 10},	
	{28, 16},	{31, 7},	{34, 19},	{28, 3},	{17, 1},	{14, 2},	{28, 2},	{26, 2},	
	{35, 24},	{38, 26},	{40, 28},	{21, 8},	{21, 3},	{30, 24},	{17, 5},	{40, 26},	
	{26, 16},	{14, 4},	{28, 4},	{34, 7},	{16, 4},	{39, 9},	{20, 3},	{39, 8},	
	{38, 3},	{41, 23},	{13, 1},	{20, 9},	{25, 17},	{26, 4},	{39, 27},	{20, 8},	
	{14, 9},	{31, 1},	{35, 18},	{13, 0},	{34, 1},	{40, 3},	{37, 23},	{17, 0},	
	{41, 18},	{14, 3},	{17, 6},	{18, 11},	{37, 13},	{21, 9},	{29, 13},	{27, 15},	
	{29, 12},	{29, 17},	{12, 11},	{13, 11},	{32, 27},	{23, 6},	{37, 17},	{25, 12},	
	{33, 27},	{6, 5},	    {31, 5},	{37, 18},	{16, 3},	{30, 23},	{30, 18},	{41, 13},	
	{12, 6},	{15, 8},	{23, 0},	{15, 3},	{25, 13},	{7, 4},	    {18, 0},	{36, 18},	
	{29, 5},	{35, 12},	{35, 23},	{16, 8},	{25, 5},	{22, 1},	{25, 1},	{13, 6},	
	{11, 1},	{23, 11},	{27, 20},	{10, 1},	{29, 1},	{27, 9},	{9, 2},	    {14, 8},	
	{19, 5},	{38, 7},	{15, 9},	{34, 17},	{12, 0},	{27, 21},	{36, 23},	{16, 9},	
	{27, 8},	{29, 18},	{18, 6},	{41, 1},	{5, 0},	    {25, 23},	{28, 15},	{3, 2},	
	{26, 15},	{42, 30},	{4, 3},	    {40, 10},	{10, 2},	{1, 0},	    {19, 4},	{26, 21},	
	{22, 5},	{7, 5},	    {19, 1},	{17, 11},	{34, 13},	{34, 0},	{8, 4},	    {24, 18},	
	{29, 23},	{38, 21},	{39, 14},	{28, 20},	{40, 7},	{24, 12},	{24, 23},	{39, 28},	
	{34, 4},	{39, 26},	{38, 19},	{39, 16},	{21, 14},	{31, 0},	{38, 20},	{32, 2},	
	{31, 2},	{22, 4},	{16, 7},	{40, 22},	{30, 11},	{22, 2},	{33, 16},	{31, 17},	
	{40, 20},	{25, 18},	{6, 1},	    {33, 4},	{40, 21},	{30, 6},	{33, 26},	{19, 2},	
	{11, 5},	{33, 2},	{38, 10},	{31, 4},	{8, 2},	    {21, 2},	{36, 0},	{42, 38},	
	{18, 17},	{23, 13},	{40, 19},	{13, 10},	{10, 4},	{32, 16},	{28, 21},	{7, 2},	
	{32, 4},	{15, 2},	{32, 28},	{13, 12},	{20, 4},	{17, 7},	{16, 15},	{20, 2},	
	{20, 16},	{26, 20},	{17, 12},	{12, 5},	{15, 14},	{14, 10},	{34, 2},	{42, 9},	
	{18, 5},	{23, 1},	{21, 4},	{24, 0},	{30, 29},	{26, 7},	{38, 22},	{19, 17},	
	{28, 7},	{7, 3},	    {31, 11},	{9, 4},	    {22, 0},	{35, 25},	{19, 0},	{23, 5},	
	{12, 1},	{15, 4},	{41, 24},	{22, 13},	{19, 16},	{28, 10},	{37, 35},	{37, 24},	
	{10, 5},	{4, 1},	    {41, 30},	{31, 14},	{10, 3},	{32, 26},	{7, 1},	    {18, 1},	
	{5, 2},	    {36, 29},	{38, 33},	{36, 25},	{34, 16},	{38, 34},	{41, 10},	{40, 31},	
	{19, 3},	{7, 0},	    {22, 6},	{26, 10},	{27, 2},	{22, 3},	{10, 8},	{23, 17},	
	{11, 7},	{19, 11},	{10, 0},	{19, 6},	{22, 14},	{40, 32},	{37, 7},	{27, 4},	
	{33, 28},	{35, 17},	{18, 13},	{20, 14},	{10, 9},	{22, 17},	{27, 14},	{21, 16},	
	{22, 11},	{30, 17},	{37, 34},	{37, 19},	{34, 12},	{5, 4},	    {41, 22},	{31, 16},	
	{31, 3},	{40, 1},	{34, 3},	{29, 10},	{19, 8},	{28, 27},	{8, 7},	    {40, 39},	
	{35, 13},	{35, 29},	{27, 16},	{14, 7},	{36, 13},	{30, 13},	{2, 1},	    {30, 25},	
	{37, 10},	{30, 5},	{22, 9},	{16, 10},	{17, 10},	{19, 13},	{19, 14},	{29, 24},	
	{11, 10},	{40, 27},	{27, 26},	{13, 4},	{16, 1},	{38, 5},	{22, 8},	{25, 10},	
	{17, 2},	{35, 1},	{38, 27},	{34, 14},	{31, 12},	{41, 31},	{40, 17},	{22, 16},	
	{7, 6},	    {13, 7},	{34, 25},	{25, 7},	{39, 38},	{26, 1},	{31, 29},	{41, 36},	
	{25, 24},	{41, 7},	{14, 5},	{39, 7},	{5, 3},	    {28, 19},	{16, 5},	{37, 22},	
	{37, 36},	{14, 1},	{24, 17},	{38, 13},	{24, 1},	{17, 4},	{15, 7},	{3, 1},	
	{13, 2},	{41, 35},	{41, 19},	{2, 0},	    {29, 7},	{26, 22},	{34, 23},	{31, 28},	
	{24, 13},	{37, 30},	{6, 4},	    {39, 10},	{28, 1},	{12, 10},	{12, 7},	{15, 10},	
	{4, 0},	    {34, 29},	{19, 12},	{38, 31},	{20, 7},	{31, 18},	{21, 7},	{26, 19},	
	{26, 5},	{28, 22},	{19, 15},	{22, 12},	{34, 26},	{21, 10},	{40, 0},	{20, 10},	
	{40, 13},	{22, 15},	{41, 4},	{9, 5},	    {38, 32},	{34, 18},	{13, 3},	{38, 0},	
	{25, 22},	{39, 19},	{38, 17},	{17, 3},	{31, 15},	{31, 25},	{14, 0},	{40, 34},	
	{40, 33},	{18, 10},	{17, 16},	{31, 23},	{39, 22},	{23, 10},	{29, 19},	{29, 4},	
	{34, 15},	{16, 0},	{25, 19},	{37, 2},	{17, 14},	{18, 7},	{25, 2},	{31, 26},	
	{32, 19},	{23, 7},	{17, 9},	{29, 22},	{14, 6},	{22, 20},	{28, 17},	{23, 19},	
	{22, 21},	{6, 3},	    {15, 1},	{27, 7},	{41, 34},	{31, 20},	{17, 8},	{33, 22},	
	{16, 11},	{16, 13},	{28, 0},	{12, 2},	{14, 11},	{14, 13},	{11, 3},	{27, 10},	
	{23, 22},	{20, 19},	{37, 31},	{34, 21},	{26, 13},	{39, 1},	{34, 28},	{26, 0},	
	{29, 2},	{21, 19},	{8, 0},	    {25, 4},	{20, 1},	{19, 18},	{15, 5},	{16, 6},	
	{31, 21},	{21, 1},	{33, 19},	{33, 5},	{12, 4},	{21, 5},	{37, 4},	{22, 18},	
	{9, 0},	    {34, 20},	{28, 11},	{35, 10},	{26, 6},	{32, 22},	{30, 7},	{41, 2}
};


// simply take average on a square patch, not even gaussian approx
static uint8_t mean_intensity(image_t *image, mw_image_t *i_image, int kp_x, int kp_y, uint32_t rot, uint32_t point)
{
    int ret_val;
    int pidx = (point/6)%8;
    float alpha, beta, theta = 0;

    if (rot) {
        theta = rot*PI_2/(float)kNB_ORIENTATION; // orientation of the pattern
    }
    beta = PI/n[pidx]*(pidx%2); // orientation offset so that groups of points on each circles are staggered
    alpha = (float)(point%n[pidx])*PI_2/(float)n[pidx]+beta+theta;

    float px = radius[pidx] * PATTERN_SCALE * arm_cos_f32(alpha);
    float py = radius[pidx] * PATTERN_SCALE * arm_sin_f32(alpha);
    float psigma = sigma[pidx] * PATTERN_SCALE;

    // get point position in image
    float xf = px+kp_x;
    float yf = py+kp_y;

    // calculate output:
    if (psigma < 0.5f) {
        int x = (int) xf;
        int y = (int) yf;
        int imagecols = image->w;

        // interpolation multipliers:
        int r_x = (xf-x)*1024;
        int r_y = (yf-y)*1024;
        int r_x_1 = (1024-r_x);
        int r_y_1 = (1024-r_y);
        uint8_t* ptr = image->data+(y*imagecols+x);

        // linear interpolation:
        ret_val = (r_x_1*r_y_1*(int)(*ptr++));
        ret_val += (r_x*r_y_1*(int)(*ptr));
        ptr += imagecols;
        ret_val += (r_x*r_y*(int)(*ptr--));
        ret_val += (r_x_1*r_y*(int)(*ptr));
        ret_val = (ret_val+512)/1024;
    } else {
        int x = (int) (xf-psigma+0.5f);
        int y = (int) (yf-psigma+0.5f);
        int w = (int) ((psigma+psigma)+0.5);
        int h = (int) ((psigma+psigma)+0.5);
        int shift = i_image->y_offs-i_image->h;
        ret_val = imlib_integral_mw_lookup(i_image, x, y-shift, w, h)/(w*h);
    }

    return ret_val;
}

array_t *freak_find_keypoints(image_t *image, bool normalized, int threshold, rectangle_t *roi)
{
    int thetaIdx=0;
    int direction0;
    int direction1;
    uint8_t pointsValue[kNB_POINTS];

    mw_image_t i_image;
    array_t *keypoints;
    array_alloc(&keypoints, xfree);

    // Find keypoints
    fast_detect(image, keypoints, threshold, roi);

    if (array_length(keypoints)) {
        int n_lines;
        if (image->h <= 144) {
            // Allocate and compute the whole integral image if
            // the image height is smaller than or equal to QCIF.
            n_lines = image->h/2; // Note: nlines is multiplied by 2
        } else {
            n_lines = (PATTERN_SCALE+1);
        }

        // Allocate 2 * (PATTERN_SCALE+1)
        // The lookup can access -(PATTERN_SCALE+1)...(PATTERN_SCALE+1),
        // keep (PATTERN_SCALE+1) rows before and after keypoint y.
        imlib_integral_mw_alloc(&i_image, image->w, n_lines*2);

        // Compute integral image
        imlib_integral_mw(image, &i_image);

        for (int i=0; i<array_length(keypoints); i++) {
            kp_t *kpt = array_at(keypoints, i);

            while (i_image.y_offs < (kpt->y+n_lines)) {
                // Shift image if needed.
                int shift = (kpt->y+n_lines) - i_image.y_offs;
                shift =  min(i_image.h-1, shift);
                imlib_integral_mw_shift(image, &i_image, shift);
            }

            // Estimate orientation (gradient)
            if (normalized) {
                thetaIdx = 0; // Assign 0° to all kpts
            } else {
                // Get the points intensity value in the un-rotated pattern
                for (int i=kNB_POINTS; i--;) {
                    pointsValue[i] = mean_intensity(image, &i_image, kpt->x, kpt->y, 0, i);
                }

                direction0 = 0;
                direction1 = 0;
                for (int m=45; m--;) {
                    // Iterate through the orientation pairs
                    int delta = (pointsValue[ORIENTATION_PAIRS[m][0]]
                                -pointsValue[ORIENTATION_PAIRS[m][1]]);
                    direction0 += delta*(ORIENTATION_PAIRS[m][2])/2048;
                    direction1 += delta*(ORIENTATION_PAIRS[m][3])/2048;
                }

                // Estimate orientation
                float angle = fast_atan2f((float)direction1, (float)direction0) * (180.0f/PI);
                thetaIdx = (int)(kNB_ORIENTATION * angle * (1.0f/360.0f) + 0.5f);

                if (thetaIdx < 0) {
                    thetaIdx += kNB_ORIENTATION;
                }

                if (thetaIdx >= kNB_ORIENTATION) {
                    thetaIdx -= kNB_ORIENTATION;
                }
            }

            // Extract descriptor at the computed orientation
            for (int i=kNB_POINTS; i--;) {
                pointsValue[i] = mean_intensity(image, &i_image, kpt->x, kpt->y, thetaIdx, i);
            }

            for (int m=kNB_PAIRS; m--;) {
                kpt->desc[m/8] |= (pointsValue[DESCRIPTION_PAIRS[m][0]]> pointsValue[DESCRIPTION_PAIRS[m][1]]) << (m%8);
            }
        }

        imlib_integral_mw_free(&i_image);
    }

    return keypoints;
}

int freak_match_keypoints(array_t *kpts1, array_t *kpts2, int threshold)
{
    int matches=0;
    int kpts1_size = array_length(kpts1);
    int kpts2_size = array_length(kpts2);

    // Reset the second set of keypoints.
    // Note: The first set will be cleared when matching.
    for (int x=0; x<kpts2_size; x++) {
        ((kp_t*)array_at(kpts2, x))->match = NULL;
    }

    // Match keypoints
    for (int x=0; x<kpts1_size; x++) {
        kp_t *min_kp=NULL;
        int min_dist = MAX_KP_DIST;

        kp_t *kp1 = array_at(kpts1, x);

        for (int y=0; y<kpts2_size; y++) {
            int dist = 0;
            kp_t *kp2 = array_at(kpts2, y);

            // If keypoint was matched skip it
            if (kp2->match != NULL) {
                continue;
            }

            // Check the first 128 bits of the descriptor
            for (int m=0; m<4; m++) { //128 bits
                uint32_t v = ((uint32_t*)(kp1->desc))[m] ^ ((uint32_t*)(kp2->desc))[m];
                while (v) {
                    dist++;
                    v &= v - 1;
                }
            }

            // FREAK descriptor works like a cascade, the first 128 bits are more
            // important, if the distance is bigger than a threshold, discard descriptor.
            if (dist > 32) {
                continue;
            }

            for (int m=4; m<16; m++) {
                uint32_t v = ((uint32_t*)(kp1->desc))[m] ^ ((uint32_t*)(kp2->desc))[m];
                while (v) {
                    dist++;
                    v &= v - 1;
                }
            }

            if (dist < min_dist) {
                min_kp = kp2;
                min_dist = dist;
            }
        }

        if ((((MAX_KP_DIST-min_dist)*100/MAX_KP_DIST)) < threshold) {
            kp1->match = NULL;
            // No match
        } else {
            matches++;
            kp1->match = min_kp;
            min_kp->match = kp1;
        }
    }
    return matches;
}

int freak_save_descriptor(FIL *fp, array_t *kpts)
{
    UINT bytes;
    FRESULT res;

    int kpts_size = array_length(kpts);

    // Write the number of keypoints
    res = f_write(fp, &kpts_size, sizeof(kpts_size), &bytes);
    if (res != FR_OK || bytes != sizeof(kpts_size)) {
        goto error;
    }

    // Write keypoints
    for (int i=0; i<kpts_size; i++) {
        kp_t *kp = array_at(kpts, i);

        // Write X
        res = f_write(fp, &kp->x, sizeof(kp->x), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->x)) {
            goto error;
        }

        // Write Y
        res = f_write(fp, &kp->y, sizeof(kp->y), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->y)) {
            goto error;
        }

        // Write descriptor
        res = f_write(fp, kp->desc, KPT_DESC_SIZE, &bytes);
        if (res != FR_OK || bytes != KPT_DESC_SIZE) {
            goto error;
        }
    }

error:
    return res;
}

int freak_load_descriptor(FIL *fp, array_t *kpts)
{
    UINT bytes;
    FRESULT res=FR_OK;

    int kpts_size=0;

    // Read number of keypoints
    res = f_read(fp, &kpts_size, sizeof(kpts_size), &bytes);
    if (res != FR_OK || bytes != sizeof(kpts_size)) {
        goto error;
    }

    // Read keypoints
    for (int i=0; i<kpts_size; i++) {
        kp_t *kp = xalloc(sizeof(*kp));

        // Read X
        res = f_read(fp, &kp->x, sizeof(kp->x), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->x)) {
            goto error;
        }

        // Read Y
        res = f_read(fp, &kp->y, sizeof(kp->y), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->y)) {
            goto error;
        }

        // Read descriptor
        res = f_read(fp, kp->desc,  KPT_DESC_SIZE, &bytes);
        if (res != FR_OK || bytes != KPT_DESC_SIZE) {
            goto error;
        }

        // Add keypoint to array
        array_push_back(kpts, kp);
    }

error:
    return res;
}
