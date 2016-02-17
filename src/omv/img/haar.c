/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Viola-Jones face detector implementation.
 * Original Author: Francesco Comaschi (f.comaschi@tue.nl)
 *
 */
#include <ff.h>
#include <stdio.h>
#include "xalloc.h"
#include "imlib.h"
#include <arm_math.h>
// built-in cascades
#include "cascade.h"

static int imlib_std(image_t *image)
{
    int w=image->w;
    int h=image->h;
    int n = w*h;
    uint8_t *data = image->pixels;

    uint32_t s=0, sq=0;
    for (int i=0; i<n; i+=2) {
        s += data[i+0] + data[i+1];
        uint32_t v0 = __PKHBT(data[i+0],
                              data[i+1], 16);
        sq = __SMLAD(v0, v0, sq);
    }

    /* mean */
    int m = s/n;

    /* variance */
    uint32_t v = sq*n-(m*m);

    /* std */
    return fast_sqrtf(v);
}

static int evalWeakClassifier(cascade_t *cascade, int p_offset, int tree_index, int w_index, int r_index)
{
    int sumw=0;
    i_image_t *sum = cascade->sum;
    /* the node threshold is multiplied by the standard deviation of the image */
    int t = cascade->tree_thresh_array[tree_index] * cascade->std;

    for (int i=0; i<cascade->num_rectangles_array[tree_index]; i++) {
        int x = cascade->rectangles_array[r_index + (i<<2) + 0];
        int y = cascade->rectangles_array[r_index + (i<<2) + 1];
        int w = cascade->rectangles_array[r_index + (i<<2) + 2];
        int h = cascade->rectangles_array[r_index + (i<<2) + 3];

        int idx0=sum->w*y + x + p_offset;
        int idx1=sum->w*(y + h) + x + p_offset;
        sumw += ( sum->data[idx0]
                - sum->data[idx0 + w]
                - sum->data[idx1]
                + sum->data[idx1 + w])
                * (cascade->weights_array[w_index + i]<<12);
    }

    if (sumw >= t) {
        return cascade->alpha2_array[tree_index];
    }

    return cascade->alpha1_array[tree_index];
}

static int runCascadeClassifier(cascade_t* cascade, struct point pt, int start_stage)
{
    int w_index = 0;
    int r_index = 0;
    int tree_index = 0;
    int p_offset = pt.y * cascade->sum->w + pt.x;

    for (int i=start_stage; i<cascade->n_stages; i++) {
        int stage_sum = 0;
        for (int j=0; j<cascade->stages_array[i]; j++, tree_index++) {
            /* send the shifted window to a haar filter */
              stage_sum += evalWeakClassifier(cascade, p_offset, tree_index, w_index, r_index);
              w_index+=cascade->num_rectangles_array[tree_index];
              r_index+=4*cascade->num_rectangles_array[tree_index];
        }

        /* If the sum is below the stage threshold, no faces are detected */
        if (stage_sum < cascade->threshold*cascade->stages_thresh_array[i]) {
            return -i;
        }
    }

    return 1;
}

static void ScaleImageInvoker(cascade_t *cascade, float factor, int sum_row, int sum_col, array_t *vec)
{
    int result;
    struct point p;

    /* When filter window shifts to image boarder, some margin need to be kept */
    int y2 = sum_row - cascade->window.w;
    int x2 = sum_col - cascade->window.h;

    int win_w =  fast_roundf(cascade->window.w*factor);
    int win_h =  fast_roundf(cascade->window.h*factor);

    /* Shift the filter window over the image. */
    for (int x=0; x<=x2; x+=cascade->step) {
        for (int y=0; y<=y2; y+=cascade->step) {
            p.x = x;
            p.y = y;

            result = runCascadeClassifier(cascade, p, 0);

            /* If a face is detected, record the coordinates of the filter window */
            if (result > 0) {
                array_push_back(vec, rectangle_alloc(fast_roundf(x*factor), fast_roundf(y*factor), win_w, win_h));
            }
        }
    }
}

array_t *imlib_detect_objects(image_t *image, cascade_t *cascade)
{
    /* allocate the detections array */
    array_t *objects;
    array_alloc(&objects, xfree);

    /* allocate integral image */
    i_image_t sum;
    imlib_integral_image_alloc(&sum, image->w, image->h);

    /* set cascade image pointer */
    cascade->img = image;

    /* sets cascade integral image */
    cascade->sum = &sum;

    /* set image standard deviation */
    cascade->std = imlib_std(image);

    // Viola and Jones achieved best results using a scaling factor
    // of 1.25 and a scanning factor proportional to the current scale.
    float scale_factor = cascade->scale_factor;
    cascade->step = (image->w*75)/1000; //7.5% of the image width

    /* iterate over the image pyramid */
    for(float factor=1.0f; ; factor*=scale_factor) {
        /* Set the width and height of the images */
        sum.w = image->w/factor;
        sum.h = image->h/factor;
        cascade->step = cascade->step/factor;
        cascade->step = (cascade->step == 0) ? 1:cascade->step;

        /* Check if scaled image is smaller
           than the original detection window */
        if (sum.w < cascade->window.w ||
            sum.h < cascade->window.h) {
            break;
        }

        /* Compute a new scaled integral image */
        imlib_integral_image_scaled(image, &sum);

        /* Process the current scale */
        ScaleImageInvoker(cascade, factor, sum.h, sum.w, objects);
    }

    if (array_length(objects) >1)   {
        objects = rectangle_merge(objects);
    }
    return objects;
}

int imlib_load_cascade_from_file(cascade_t *cascade, const char *path)
{
    int i;
    UINT n_out;

    FIL fp;
    FRESULT res=FR_OK;

    res = f_open(&fp, path, FA_READ|FA_OPEN_EXISTING);
    if (res != FR_OK) {
        return res;
    }

    /* read detection window size */
    res = f_read(&fp, &cascade->window, sizeof(cascade->window), &n_out);
    if (res != FR_OK || n_out != sizeof(cascade->window)) {
        goto error;
    }

    /* read num stages */
    res = f_read(&fp, &cascade->n_stages, sizeof(cascade->n_stages), &n_out);
    if (res != FR_OK || n_out != sizeof(cascade->n_stages)) {
        goto error;
    }

    cascade->stages_array = xalloc (sizeof(*cascade->stages_array) * cascade->n_stages);
    cascade->stages_thresh_array = xalloc (sizeof(*cascade->stages_thresh_array) * cascade->n_stages);

    if (cascade->stages_array == NULL ||
        cascade->stages_thresh_array == NULL) {
        res = 20;
        goto error;
    }

    /* read num features in each stages */
    res = f_read(&fp, cascade->stages_array, sizeof(uint8_t) * cascade->n_stages, &n_out);
    if (res != FR_OK || n_out != sizeof(uint8_t) * cascade->n_stages) {
        goto error;
    }

    /* sum num of features in each stages*/
    for (i=0, cascade->n_features=0; i<cascade->n_stages; i++) {
        cascade->n_features += cascade->stages_array[i];
    }

    /* alloc features thresh array, alpha1, alpha 2,rects weights and rects*/
    cascade->tree_thresh_array = xalloc (sizeof(*cascade->tree_thresh_array) * cascade->n_features);
    cascade->alpha1_array = xalloc (sizeof(*cascade->alpha1_array) * cascade->n_features);
    cascade->alpha2_array = xalloc (sizeof(*cascade->alpha2_array) * cascade->n_features);
    cascade->num_rectangles_array = xalloc (sizeof(*cascade->num_rectangles_array) * cascade->n_features);

    if (cascade->tree_thresh_array == NULL ||
        cascade->alpha1_array   == NULL ||
        cascade->alpha2_array   == NULL ||
        cascade->num_rectangles_array == NULL) {
        res = 20;
        goto error;
    }

    /* read stages thresholds */
    res = f_read(&fp, cascade->stages_thresh_array, sizeof(int16_t)*cascade->n_stages, &n_out);
    if (res != FR_OK || n_out != sizeof(int16_t)*cascade->n_stages) {
        goto error;
    }

    /* read features thresholds */
    res = f_read(&fp, cascade->tree_thresh_array, sizeof(*cascade->tree_thresh_array)*cascade->n_features, &n_out);
    if (res != FR_OK || n_out != sizeof(*cascade->tree_thresh_array)*cascade->n_features) {
        goto error;
    }

    /* read alpha 1 */
    res = f_read(&fp, cascade->alpha1_array, sizeof(*cascade->alpha1_array)*cascade->n_features, &n_out);
    if (res != FR_OK || n_out != sizeof(*cascade->alpha1_array)*cascade->n_features) {
        goto error;
    }

    /* read alpha 2 */
    res = f_read(&fp, cascade->alpha2_array, sizeof(*cascade->alpha2_array)*cascade->n_features, &n_out);
    if (res != FR_OK || n_out != sizeof(*cascade->alpha2_array)*cascade->n_features) {
        goto error;
    }

    /* read num rectangles per feature*/
    res = f_read(&fp, cascade->num_rectangles_array, sizeof(*cascade->num_rectangles_array)*cascade->n_features, &n_out);
    if (res != FR_OK || n_out != sizeof(*cascade->num_rectangles_array)*cascade->n_features) {
        goto error;
    }

    /* sum num of recatngles per feature*/
    for (i=0, cascade->n_rectangles=0; i<cascade->n_features; i++) {
        cascade->n_rectangles += cascade->num_rectangles_array[i];
    }

    cascade->weights_array = xalloc (sizeof(*cascade->weights_array) * cascade->n_rectangles);
    cascade->rectangles_array = xalloc (sizeof(*cascade->rectangles_array) * cascade->n_rectangles * 4);

    if (cascade->weights_array  == NULL ||
        cascade->rectangles_array == NULL) {
        res = 20;
        goto error;
    }

    /* read rectangles weights */
    res =f_read(&fp, cascade->weights_array, sizeof(*cascade->weights_array)*cascade->n_rectangles, &n_out);
    if (res != FR_OK || n_out != sizeof(*cascade->weights_array)*cascade->n_rectangles) {
        goto error;
    }

    /* read rectangles num rectangles * 4 points */
    res = f_read(&fp, cascade->rectangles_array, sizeof(*cascade->rectangles_array)*cascade->n_rectangles *4, &n_out);
    if (res != FR_OK || n_out != sizeof(*cascade->rectangles_array)*cascade->n_rectangles *4) {
        goto error;
    }

error:
    f_close(&fp);
    return res;
}

int imlib_load_cascade_from_flash(cascade_t *cascade, const char *path)
{
    FRESULT res=FR_OK;

    if (strcmp(path, "frontalface") == 0) {
        cascade->window.w            = frontalface_window_w;
        cascade->window.h            = frontalface_window_h;
        cascade->n_stages            = frontalface_n_stages;
        cascade->stages_array        = (uint8_t *)frontalface_stages_array;
        cascade->stages_thresh_array = (int16_t *)frontalface_stages_thresh_array;
        cascade->tree_thresh_array   = (int16_t *)frontalface_tree_thresh_array;
        cascade->alpha1_array        = (int16_t *)frontalface_alpha1_array;
        cascade->alpha2_array        = (int16_t *)frontalface_alpha2_array;
        cascade->num_rectangles_array= (int8_t  *)frontalface_num_rectangles_array;
        cascade->weights_array       = (int8_t  *)frontalface_weights_array;
        cascade->rectangles_array    = (int8_t  *)frontalface_rectangles_array;
    } else if (strcmp(path, "eye") == 0) {
        cascade->window.w            = eye_window_w;
        cascade->window.h            = eye_window_h;
        cascade->n_stages            = eye_n_stages;
        cascade->stages_array        = (uint8_t *)eye_stages_array;
        cascade->stages_thresh_array = (int16_t *)eye_stages_thresh_array;
        cascade->tree_thresh_array   = (int16_t *)eye_tree_thresh_array;
        cascade->alpha1_array        = (int16_t *)eye_alpha1_array;
        cascade->alpha2_array        = (int16_t *)eye_alpha2_array;
        cascade->num_rectangles_array= (int8_t  *)eye_num_rectangles_array;
        cascade->weights_array       = (int8_t  *)eye_weights_array;
        cascade->rectangles_array    = (int8_t  *)eye_rectangles_array;
    } else {
        res = FR_NO_FILE;
    }

    if (res == FR_OK) {
        int i;
        // sum the number of features in all stages
        for (i=0, cascade->n_features=0; i<cascade->n_stages; i++) {
            cascade->n_features += cascade->stages_array[i];
        }

        // sum the number of recatngles in all features
        for (i=0, cascade->n_rectangles=0; i<cascade->n_features; i++) {
            cascade->n_rectangles += cascade->num_rectangles_array[i];
        }
    }

    return res;
}

int imlib_load_cascade(cascade_t *cascade, const char *path)
{
    if (path[0] != '/') {
        // built-in cascade
        return imlib_load_cascade_from_flash(cascade, path);
    } else {
        // xml cascade
        return imlib_load_cascade_from_file(cascade, path);
    }
}
