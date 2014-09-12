#include <ff.h>
#include <stdio.h>
#include "xalloc.h"
#include "imlib.h"
#include <arm_math.h>

/* Viola-Jones face detector implementation
 * Original Author:   Francesco Comaschi (f.comaschi@tue.nl)
 */
static int evalWeakClassifier(cascade_t *cascade, int std, int p_offset, int tree_index, int w_index, int r_index)
{
    int sumw=0;
    i_image_t *sum = cascade->sum;
    /* the node threshold is multiplied by the standard deviation of the image */
    int t = cascade->tree_thresh_array[tree_index] * std;

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

    uint32_t sumsq = 0;
    int32_t std, mean;

    int x_ratio = cascade->x_ratio;
    int y_ratio = cascade->y_ratio;
    image_t *image = cascade->img;

    for (int y=pt.y; y<cascade->window.h; y++) {
          int sy = ((y*y_ratio)>>16)*image->w;
      for (int x=pt.x; x<cascade->window.w; x+=2) {
          int sx = sy+((x*x_ratio)>>16);
          uint32_t v0 = __PKHBT(image->pixels[sx+0],
                                image->pixels[sx+1], 16);
          sumsq = __SMLAD(v0, v0, sumsq);
      }
    }

    /* Image normalization */
    i_image_t *sum = cascade->sum;
    int win_w = cascade->window.w - 1;
    int win_h = cascade->window.h - 1;
    int p_offset = pt.y * (sum->w) + pt.x;

    mean = sum->data[p_offset]
         - sum->data[win_w + p_offset]
         - sum->data[sum->w * win_h + p_offset]
         + sum->data[sum->w * win_h + win_w + p_offset];

    std = fast_sqrtf(sumsq * cascade->window.w * cascade->window.h - mean * mean);

    for (int i=start_stage; i<cascade->n_stages; i++) {
        int stage_sum = 0;
        for (int j=0; j<cascade->stages_array[i]; j++, tree_index++) {
            /* send the shifted window to a haar filter */
              stage_sum += evalWeakClassifier(cascade, std, p_offset, tree_index, w_index, r_index);
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
    int x, y, x2, y2;

    struct point p;
    struct size win_size;

    win_size.w =  fast_roundf(cascade->window.w*factor);
    win_size.h =  fast_roundf(cascade->window.h*factor);

    /* When filter window shifts to image boarder, some margin need to be kept */
    y2 = sum_row - win_size.h;
    x2 = sum_col - win_size.w;

    /* Shift the filter window over the image. */
    for (x=0; x<=x2; x+=cascade->step) {
        for (y=0; y<=y2; y+=cascade->step) {
            p.x = x;
            p.y = y;

            result = runCascadeClassifier(cascade, p, 0);

            /* If a face is detected, record the coordinates of the filter window */
            if (result > 0) {
                array_push_back(vec, rectangle_alloc(fast_roundf(x*factor),
                            fast_roundf(y*factor), win_size.w, win_size.h));
            }
        }
    }
}

array_t *imlib_detect_objects(image_t *image, cascade_t *cascade)
{
    float factor = 1.0f;
    array_t *objects;

    /* allocate the detections array */
    array_alloc(&objects, xfree);

    /* allocate integral image */
    i_image_t sum;
    imlib_integral_image_alloc(&sum, image->w, image->h);

    /* set cascade image pointer */
    cascade->img = image;

    /* sets cascade integral image */
    cascade->sum = &sum;

    /* iterate over the image pyramid */
    while (true) {
        /* Set the width and height of the images */
        sum.w = (image->w*factor);
        sum.h = (image->h*factor);

        /* if scaled image is smaller than the original detection window, break */
        if (sum.w <= cascade->window.w ||
            sum.h <= cascade->window.h) {
            break;
        }

        cascade->x_ratio = (int)((image->w<<16)/sum.w) +1;
        cascade->y_ratio = (int)((image->h<<16)/sum.h) +1;

        /* compute a new scaled integral image */
        imlib_integral_image_scaled(image, &sum);

        /* process the current scale with the cascaded fitler. */
        ScaleImageInvoker(cascade, factor, sum.h, sum.w, objects);

        factor *= cascade->scale_factor;
    }

    if (array_length(objects) >1)   {
        objects = rectangle_merge(objects);
    }
    return objects;
}

int imlib_load_cascade(cascade_t *cascade, const char *path)
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


