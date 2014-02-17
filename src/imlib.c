#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <arm_math.h>
#include <stm32f4xx.h>
#include "array.h"
#include "imlib.h"
#include "ff.h"

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MAX_GRAY_LEVEL (255)

float imlib_distance(struct color *c0, struct color *c1)
{
    float sum=0.0f;
    sum += (c0->r - c1->r) * (c0->r - c1->r);
    sum += (c0->g - c1->g) * (c0->g - c1->g);
    sum += (c0->b - c1->b) * (c0->b - c1->b);
    return sqrtf(sum);
}

void imlib_rgb_to_hsv(struct color *rgb, struct color *hsv)
{
    int min;
    int max;
    int r,g,b;
    int delta;

    /* 0..100 */
    r = rgb->r*100/255;
    g = rgb->g*100/255;
    b = rgb->b*100/255;

    min = MIN(r, MIN(g, b));
    max = MAX(r, MAX(g, b));

    if (min == max) {
        /* Black/gray/white */
        hsv->h = 0;
        hsv->s = 0;
        hsv->v = min;
    } else {
        delta = max-min;
        //scaled to avoid floats
        if (r==max) {
            hsv->h = (g-b)*100/delta;
        } else if (g==max) {
            hsv->h = 200 + (b-r)*100/delta;
        } else {
            hsv->h = 400 + (r-g)*100/delta;
        }
        hsv->h = hsv->h*60/100;
        if (hsv->h<0) {
            hsv->h+=360;
        }
        hsv->s = delta*100/max;
        hsv->v = max;
    }
}

/* converts a grayscale buffer to RGB565 to display on LCDs */
void imlib_grayscale_to_rgb565(struct image *image)
{
#if 0
    int i;
    for (i=0; i<(image->w * image->h * image->bpp); i++) {
        uint8_t y = image->pixels[i];
        uint8_t r = y*31/255;
        uint8_t g = y*63/255;
        uint8_t b = y*31/255;
        //uint16_t rgb = (r << 11) | (g << 5) | b;
    }
#endif
}

void imlib_detect_color(struct image *image, struct color *color, struct rectangle *rectangle, int threshold)
{
    int x,y;
    uint8_t p0,p1;
    struct color rgb;
    struct color hsv;

    int pixels = 1;
    rectangle->w = 0;
    rectangle->h = 0;
    rectangle->x = image->w;
    rectangle->y = image->h;

    //to avoid sqrt we use squared values
    threshold *= threshold;

    for (y=0; y<image->h; y++) {
        for (x=0; x<image->w; x++) {
            int i=y*image->w*image->bpp+x*image->bpp;
            p0 = image->pixels[i];
            p1 = image->pixels[i+1];

            /* map RGB565 to RGB888 */
            rgb.r = (uint8_t) (p0>>3) * 255/31;
            rgb.g = (uint8_t) (((p0&0x07)<<3) | (p1>>5)) * 255/63;
            rgb.b = (uint8_t) (p1&0x1F) * 255/31;

            /* convert RGB to HSV */
            imlib_rgb_to_hsv(&rgb, &hsv);

            /* difference between target Hue and pixel Hue squared */
            hsv.h = (hsv.h - color->h) * (hsv.h - color->h);

            /* add pixel if within threshold */
            if (hsv.h < threshold && hsv.s > color->s && hsv.v > color->v) { //s==pale
                pixels++;
                if (x < rectangle->x) {
                    rectangle->x = x;
                }
                if (y < rectangle->y) {
                    rectangle->y = y;
                }

                if (x > rectangle->w) {
                    rectangle->w = x;
                }
                if (y > rectangle->h) {
                    rectangle->h = y;
                }
            }
        }
    }

    rectangle->w = rectangle->w-rectangle->x;
    rectangle->h = rectangle->h-rectangle->y;
}

void imlib_erosion_filter(struct image *src, uint8_t *kernel, int k_size)
{
    int x, y, j, k;
    int w = src->w;
    int h = src->h;
    /* TODO */
    uint8_t *dst = calloc(w*h, 1);

    for (y=0; y<h-k_size; y++) {
        for (x=0; x<w-k_size; x++) {
            dst[w*(y+1)+x+1] = 255;
            for (j=0; j<k_size; j++) {
                for (k=0; k<k_size; k++) {
                  /* (y*w+x)+(j*w+k) */
                  if (src->pixels[w*(y+j)+x+k] != (kernel[j*k_size+k]*255)) {
                    dst[w*(y+1)+x+1] = 0;
                    j=k_size;
                    break;
                  }
                }
            }
        }
    }
    memcpy(src->pixels, dst, w*h);
    free(dst);
}

int imlib_image_mean(struct image *src)
{
    int s=0;
    int x,y;
    int n = src->w*src->h;

    for (y=0; y<src->h; y++) {
        for (x=0; x<src->w; x++) {
            s += src->data[src->w*y+x];
        }
    }

    /* mean */
    return s/n;
}

void imlib_subimage(struct image *src_img, struct image *dst_img, int x_off, int y_off)
{
    int x, y;
    typeof(*src_img->data) *src = src_img->data;
    typeof(*dst_img->data) *dst = dst_img->data;

    for (y=y_off; y<dst_img->h+y_off; y++) {
        for (x=x_off; x<dst_img->w+x_off; x++) {
            *dst++ = src[y*src_img->w+x];
        }
    }
}

void imlib_blit(struct image *dst_img, struct image *src_img, int x_off, int y_off)
{
    int x, y;
    typeof(*src_img->data) *src = src_img->data;
    typeof(*dst_img->data) *dst = dst_img->data;

    for (y=y_off; y<src_img->h+y_off; y++) {
        for (x=x_off; x<src_img->w+x_off; x++) {
            dst[y*dst_img->w+x]=*src++;
        }
    }
}

void imlib_integral_image(struct image *src, struct integral_image *sum)
{
    int x, y, s,t;
    unsigned char *data = src->pixels;
    typeof(*sum->data) *sumData = sum->data;

    for (y=0; y<src->h; y++) {
        s = 0;
        /* loop over the number of columns */
        for (x=0; x<src->w; x++) {
            /* sum of the current row (integer)*/
            s += data[y*src->w+x];
            t = s;
            if (y != 0) {
                t += sumData[(y-1)*src->w+x];
            }
            sumData[y*src->w+x]=t;
        }
    }
}

void imlib_scale_image(struct image *src, struct image *dst)
{
    int x, y, i, j;
    uint8_t *t, *p;
    int w1 = src->w;
    int h1 = src->h;
    int w2 = dst->w;
    int h2 = dst->h;

    int rat = 0;

    uint8_t *src_data = src->pixels;
    uint8_t *dst_data = dst->pixels;

    int x_ratio = (int)((w1<<16)/w2) +1;
    int y_ratio = (int)((h1<<16)/h2) +1;

    for (i=0;i<h2;i++){
        t = dst_data + i*w2;
        y = ((i*y_ratio)>>16);
        p = src_data + y*w1;
        rat = 0;
        for (j=0;j<w2;j++) {
            x = (rat>>16);
            *t++ = p[x];
            rat += x_ratio;
        }
    }
}

void imlib_draw_rectangle(struct image *image, struct rectangle *r)
{
    int i;
    uint8_t c=0xFF;
    int x = MIN(MAX(r->x, 0), image->w);
    int y = MIN(MAX(r->y, 0), image->h);
    int w = (x+r->w) > image->w ? (image->w-x):r->w;
    int h = (y+r->h) > image->h ? (image->h-y):r->h;

    x *= image->bpp;
    w *= image->bpp;
    int col = image->w*image->bpp;

    for (i=0; i<w; i++) {
        image->pixels[y*col + x + i] = c;
        image->pixels[(y+h)*col + x + i] = c;
    }

    for (i=0; i<h; i++) {
        image->pixels[(y+i)*col + x] = c;
        image->pixels[(y+i)*col + x + w-2] = c;
        if (image->bpp>1) {
            image->pixels[(y+i)*col + x+1] = c;
            image->pixels[(y+i)*col + x + w-1] = c;
        }
    }
}

void imlib_histeq(struct image *src)
{
    int i, sum;
    int a = src->w*src->h;
    uint32_t hist[MAX_GRAY_LEVEL+1]={0};

    /* compute image histogram */
    for (i=0; i<a; i++) {
        hist[src->pixels[i]]+=1;
    }

    /* compute the CDF */
    for (i=0, sum=0; i<MAX_GRAY_LEVEL+1; i++) {
        sum += hist[i];
        hist[i] = sum;
    }

    for (i=0; i<a; i++) {
        src->pixels[i] = (uint8_t) ((MAX_GRAY_LEVEL/(float)a) * hist[src->pixels[i]]);
    }
}

/* Viola-Jones face detector implementation
 * Original Author:   Francesco Comaschi (f.comaschi@tue.nl)
 */
static int evalWeakClassifier(struct cascade *cascade, int std, int p_offset, int tree_index, int w_index, int r_index )
{
    int i, sumw=0;
    struct rectangle tr;
    struct integral_image *sum = &cascade->sum;
    /* the node threshold is multiplied by the standard deviation of the image */
    int t = cascade->tree_thresh_array[tree_index] * std;

    for (i=0; i<cascade->num_rectangles_array[tree_index]; i++) {
        tr.x = cascade->rectangles_array[r_index + i*4 + 0];
        tr.y = cascade->rectangles_array[r_index + i*4 + 1];
        tr.w = cascade->rectangles_array[r_index + i*4 + 2];
        tr.h = cascade->rectangles_array[r_index + i*4 + 3];

        sumw += (
             *((sum->data + sum->w*(tr.y ) + (tr.x ))               + p_offset)
           - *((sum->data + sum->w*(tr.y ) + (tr.x  + tr.w))        + p_offset)
           - *((sum->data + sum->w*(tr.y  + tr.h) + (tr.x ))        + p_offset)
           + *((sum->data + sum->w*(tr.y  + tr.h) + (tr.x  + tr.w)) + p_offset))
           * cascade->weights_array[w_index + i]*4096;
    }

    if (sumw >= t) {
        return cascade->alpha2_array[tree_index];
    }

    return cascade->alpha1_array[tree_index];
}

static int runCascadeClassifier(struct cascade* cascade, struct point pt, int start_stage)
{
    int i, j;
    int p_offset;
    int32_t mean;
    int32_t std;

    int w_index = 0;
    int r_index = 0;
    int stage_sum;
    int tree_index = 0;

    int x,y,offset;
    uint32_t sumsq=0;
    vec_t v0, v1;

    for (y=pt.y; y<cascade->window.w; y++) {
      for (x=pt.x; x<cascade->window.w; x+=2) {
          offset = y*cascade->img->w+x;
          v0.s0 = cascade->img->pixels[offset+0];
          v0.s1 = cascade->img->pixels[offset+1];

          v1.s0 = cascade->img->pixels[offset+0];
          v1.s1 = cascade->img->pixels[offset+1];
          sumsq = __SMLAD(v0.i, v1.i, sumsq);
      }
    }

    /* Image normalization */
    int win_w = cascade->window.w - 1;
    int win_h = cascade->window.h - 1;

    p_offset = pt.y * (cascade->sum.w) + pt.x;

    mean = cascade->sum.data[p_offset]
         - cascade->sum.data[win_w + p_offset]
         - cascade->sum.data[cascade->sum.w * win_h + p_offset]
         + cascade->sum.data[cascade->sum.w * win_h + win_w + p_offset];

    std = sqrtf(sumsq * cascade->window.w * cascade->window.h - mean * mean);

    for (i=start_stage; i<cascade->n_stages; i++) {
        stage_sum = 0;
        for (j=0; j<cascade->stages_array[i]; j++, tree_index++) {
            /* send the shifted window to a haar filter */
              stage_sum += evalWeakClassifier(cascade, std, p_offset, tree_index, w_index, r_index);
              w_index+=cascade->num_rectangles_array[tree_index];
              r_index+=4 * cascade->num_rectangles_array[tree_index];
        }

        /* If the sum is below the stage threshold, no faces are detected */
        if (stage_sum < 0.4*cascade->stages_thresh_array[i]) {
            return -i;
        }
    }

    return 1;
}

static void ScaleImageInvoker(struct cascade *cascade, float factor, int sum_row, int sum_col, struct array *vec)
{
    int result;
    int x, y, x2, y2;

    struct point p;
    struct size win_size;

    win_size.w =  roundf(cascade->window.w*factor);
    win_size.h =  roundf(cascade->window.h*factor);

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
                struct rectangle *r = malloc(sizeof(struct rectangle));
                r->x = roundf(x*factor);
                r->y = roundf(y*factor);
                r->w = win_size.w;
                r->h = win_size.h;
                array_push_back(vec, r);
            }
        }
    }
}

struct rectangle *rectangle_clone(struct rectangle *rect)
{
    struct rectangle *rectangle;
    rectangle = malloc(sizeof(struct rectangle));
    memcpy(rectangle, rect, sizeof(struct rectangle));
    return rectangle;
}

void rectangle_add(struct rectangle *rect0, struct rectangle *rect1)
{
    rect0->x += rect1->x;
    rect0->y += rect1->y;
    rect0->w += rect1->w;
    rect0->h += rect1->h;
}

void rectangle_div(struct rectangle *rect0, int c)
{
    rect0->x /= c;
    rect0->y /= c;
    rect0->w /= c;
    rect0->h /= c;
}

void rectangle_merge(struct rectangle *rect0, struct rectangle *rect1)
{
    rect0->x = (rect0->x < rect1->x)? rect0->x:rect1->x;
    rect0->y = (rect0->y < rect1->y)? rect0->y:rect1->y;
    rect0->w = (rect0->w > rect1->w)? rect0->w:rect1->w;
    rect0->h = (rect0->h > rect1->h)? rect0->h:rect1->h;

}

int rectangle_intersects(struct rectangle *rect0, struct rectangle *rect1)
{
    return  ((rect0->x < (rect1->x+rect1->w)) &&
             (rect0->y < (rect1->y+rect1->h)) &&
             ((rect0->x+rect0->w) > rect1->x) &&
             ((rect0->y+rect0->h) > rect1->y));
}

struct array *imlib_merge_detections(struct array *rectangles)
{
    int j;
    struct array *objects;
    struct array *overlap;
    struct rectangle *rect1, *rect2;

    array_alloc(&objects, free);
    array_alloc(&overlap, free);

    /* merge overlaping detections */
    while (array_length(rectangles)) {
        /* check for overlaping detections */
        rect1 = (struct rectangle *) array_at(rectangles, 0);
        for (j=1; j<array_length(rectangles); j++) {
            rect2 = (struct rectangle *) array_at(rectangles, j);
            if (rectangle_intersects(rect1, rect2)) {
                array_push_back(overlap, rectangle_clone(rect2));
                array_erase(rectangles, j--);
            }
        }

        /* add the overlaping detections */
        int count = array_length(overlap)+1;
        while (array_length(overlap)) {
            rect2 = (struct rectangle *) array_at(overlap, 0);
            rectangle_add(rect1, rect2);
            array_erase(overlap, 0);
        }

        /* average the overlaping detections */
        rectangle_div(rect1, count);
        array_push_back(objects, rectangle_clone(rect1));
        array_erase(rectangles, 0);
    }

    array_free(overlap);
    array_free(rectangles);
    return objects;
}

struct array *imlib_detect_objects(struct image *image, struct cascade *cascade)
{
    /* scaling factor */
    float factor;

    struct array *objects;

    struct image img;
    struct integral_image sum;

    /* allocate buffer for scaled image */
    img.w = image->w;
    img.h = image->h;
    img.bpp = image->bpp;
    /* use the second half of the framebuffer */
    img.pixels = image->pixels+(image->w * image->h);

    /* allocate buffer for integral image */
    sum.w = image->w;
    sum.h = image->h;
    //sum.data   = malloc(image->w *image->h*sizeof(*sum.data));
    sum.data = (uint32_t*) (image->pixels+(image->w * image->h * 2));

    /* allocate the detections array */
    array_alloc(&objects, free);

    /* set cascade image pointer */
    cascade->img = &img;

    /* iterate over the image pyramid */
    for(factor=1.0f; ; factor*=cascade->scale_factor) {
        /* size of the scaled image */
        struct size sz = {
            (image->w/factor),
            (image->h/factor)
        };

        /* if scaled image is smaller than the original detection window, break */
        if ((sz.w  - cascade->window.w)  <= 0 ||
            (sz.h - cascade->window.h) <= 0) {
            break;
        }

        /* Set the width and height of the images */
        img.w = sz.w;
        img.h = sz.h;

        sum.w = sz.w;
        sum.h = sz.h;

        /* downsample using nearest neighbor */
        imlib_scale_image(image, &img);

        /* compute a new integral image */
        imlib_integral_image(&img, &sum);

        /* sets images for haar classifier cascade */
        cascade->sum = sum;

        /* process the current scale with the cascaded fitler. */
        ScaleImageInvoker(cascade, factor, sum.h, sum.w, objects);
    }

    //free(sum.data);

    objects = imlib_merge_detections(objects);
    return objects;
}

int imlib_load_cascade(struct cascade* cascade, const char *path)
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

    cascade->stages_array = malloc (sizeof(*cascade->stages_array) * cascade->n_stages);
    cascade->stages_thresh_array = malloc (sizeof(*cascade->stages_thresh_array) * cascade->n_stages);

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
    cascade->tree_thresh_array = malloc (sizeof(*cascade->tree_thresh_array) * cascade->n_features);
    cascade->alpha1_array = malloc (sizeof(*cascade->alpha1_array) * cascade->n_features);
    cascade->alpha2_array = malloc (sizeof(*cascade->alpha2_array) * cascade->n_features);
    cascade->num_rectangles_array = malloc (sizeof(*cascade->num_rectangles_array) * cascade->n_features);

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

    cascade->weights_array = malloc (sizeof(*cascade->weights_array) * cascade->n_rectangles);
    cascade->rectangles_array = malloc (sizeof(*cascade->rectangles_array) * cascade->n_rectangles * 4);

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

uint32_t imlib_integral_lookup(struct integral_image *src, int x, int y, int w, int h)
{
#define PIXEL_AT(x,y)\
    (src->data[src->w*(y-1)+(x-1)])

    if (x==0 && y==0) {
        return PIXEL_AT(w,h);
    } else if (y==0) {
        return PIXEL_AT(w+x, h+y) - PIXEL_AT(x, h+y);
    } else if (x==0) {
        return PIXEL_AT(w+x, h+y) - PIXEL_AT(w+x, y);
    } else {
        return PIXEL_AT(w+x, h+y) + PIXEL_AT(x, y) - PIXEL_AT(w+x, y) - PIXEL_AT(x, h+y);
    }
#undef  PIXEL_AT
}
