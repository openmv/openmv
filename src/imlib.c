#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <arm_math.h>
#include <stm32f4xx.h>
#include "array.h"
#include "imlib.h"
#include "cascade.h"

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

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
void imlib_grayscale_to_rgb565(struct frame_buffer *fb)
{
#if 0
    int i;
    for (i=0; i<(fb->width * fb->height * fb->bpp); i++) {
        uint8_t y = fb->pixels[i];
        uint8_t r = y*31/255;
        uint8_t g = y*63/255;
        uint8_t b = y*31/255;
        //uint16_t rgb = (r << 11) | (g << 5) | b;
    }
#endif
}

void imlib_color_track(struct frame_buffer *fb, struct color *color, struct point *point, int threshold)
{
    int x,y;
    uint8_t p0,p1;
    struct color rgb;
    struct color hsv;

    int pixels = 1;
    point->x = 0;
    point->y = 0;

    //to avoid sqrt we use squared values
    threshold *= threshold;

    for (y=0; y<fb->height; y++) {
        for (x=0; x<fb->width; x++) {
            int i=y*fb->width*fb->bpp+x*fb->bpp;
            p0 = fb->pixels[i];
            p1 = fb->pixels[i+1];

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
                point->x += x;
                point->y += y;
            }
        }
    }

    if (pixels < 10) {
        point->x = 0;
        point->y = 0;
    } else {
        point->x /= pixels;
        point->y /= pixels;
    }
}

void imlib_erosion_filter(struct frame_buffer *fb, uint8_t *kernel, int k_size)
{
    int x, y, j, k;
    int w = fb->width;
    int h = fb->height;
    uint8_t *dst = calloc(w*h, 1);
    
    for (y=0; y<h-k_size; y++) {
        for (x=0; x<w-k_size; x++) {
            dst[w*(y+1)+x+1] = 255;
            for (j=0; j<k_size; j++) {
                for (k=0; k<k_size; k++) {
                  /* (y*w+x)+(j*w+k) */
                  if (fb->pixels[w*(y+j)+x+k] != (kernel[j*k_size+k]*255)) {
                    dst[w*(y+1)+x+1] = 0;
                    j=k_size;
                    break;
                  }
                }
            }
        }   
    }
    memcpy(fb->pixels, dst, w*h);
    free(dst);
}

void imlib_integral_image(struct frame_buffer *src, struct integral_image *sum)
{
  int x, y, s,t;
  unsigned char *data = src->data;
  typeof(*sum->data) *sumData = sum->data;
  for (y=0; y<src->height; y++) {
      s = 0;
      /* loop over the number of columns */
      for (x=0; x<src->width; x++) {
	      /* sum of the current row (integer)*/
     	  s += data[y*src->width+x];
    	  t = s;
    	  if (y != 0) {
	          t += sumData[(y-1)*src->width+x];
	      }
	      sumData[y*src->width+x]=t;
	 }
  }
}

void imlib_scale_image(struct frame_buffer *src, struct frame_buffer *dst)
{
    int x, y, i, j;
    uint8_t *t, *p;
    int w1 = src->width;
    int h1 = src->height;
    int w2 = dst->width;
    int h2 = dst->height;

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

void imlib_draw_rectangle(struct frame_buffer* image, struct rectangle *r)
{
	int i;
    int bpp = image->bpp;
	int col = image->width*image->bpp;
    uint8_t c=0xff;
	for (i = 0; i < r->width*bpp; i++) {
        image->data[r->y*col + r->x*bpp + i] = c;
	}

	for (i = 0; i < r->height; i++) {
		image->data[col*(r->y+i) + r->x*bpp + r->width*bpp] = c;
	}

	for (i = 0; i < r->width*bpp; i++) {
		image->data[col*(r->y + r->height) + r->x*bpp + r->width*bpp - i] = c;
	}

	for (i = 0; i < r->height; i++) {
		image->data[col*(r->y + r->height - i) + r->x*bpp] =c;
	}
}

#define MAX_GRAY_LEVEL (255)
void imlib_histeq(struct frame_buffer *fb)
{
    int i, sum;
    int a = fb->width*fb->height;
    uint32_t hist[MAX_GRAY_LEVEL+1]={0}; 

    /* compute image histogram */
    for (i=0; i<a; i++) {
        hist[fb->pixels[i]]+=1;
    }

    /* compute the CDF */ 
    for (i=0, sum=0; i<MAX_GRAY_LEVEL+1; i++) {
        sum += hist[i];
        hist[i] = sum;
    }

    for (i=0; i<a; i++) {
        fb->pixels[i] = (uint8_t) ((MAX_GRAY_LEVEL/(float)a) * hist[fb->pixels[i]]);
    }
}

/* Viola-Jones face detector implementation 
 * Original Author:   Francesco Comaschi (f.comaschi@tue.nl)
 */
static int evalWeakClassifier(struct integral_image *sum, int std, int p_offset, int tree_index, int w_index, int r_index )
{
    struct rectangle tr;

    /* the node threshold is multiplied by the standard deviation of the image */
    int t = tree_thresh_array[tree_index] * std;

    /* this is just a hack will be removed when
       we add the number of rects to the classifier */
    tr.x = rectangles_array[r_index + 8];
    tr.y = rectangles_array[r_index + 9];
    tr.w = rectangles_array[r_index + 10];
    tr.h = rectangles_array[r_index + 11];

    int i,k, sumw=0;

    if ((tr.x)&& (tr.y) &&(tr.width) &&(tr.height)) {
        k = 3;
    } else {
        k = 2;
    }

    for (i=0; i<k; i++) {
        tr.x = rectangles_array[r_index + i*4 + 0];
        tr.y = rectangles_array[r_index + i*4 + 1];
        tr.w = rectangles_array[r_index + i*4 + 2];
        tr.h = rectangles_array[r_index + i*4 + 3];

        sumw += (
             *((sum->data + sum->width*(tr.y ) + (tr.x ))                         + p_offset)
           - *((sum->data + sum->width*(tr.y ) + (tr.x  + tr.width))              + p_offset)
           - *((sum->data + sum->width*(tr.y  + tr.height) + (tr.x ))             + p_offset)
           + *((sum->data + sum->width*(tr.y  + tr.height) + (tr.x  + tr.width))  + p_offset))
           * weights_array[w_index + i];
    }
    
    if (sumw >= t) {
        return alpha2_array[tree_index];
    }

    return alpha1_array[tree_index];
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

    for (y=pt.y; y<24; y++) {
      for (x=pt.x; x<24; x+=2) {
          offset = y*cascade->img->width+x;
          v0.s0 = cascade->img->data[offset+0]; 
          v0.s1 = cascade->img->data[offset+1]; 

          v1.s0 = cascade->img->data[offset+0]; 
          v1.s1 = cascade->img->data[offset+1]; 
          sumsq = __SMLAD(v0.i, v1.i, sumsq);
      }
    }

    /* Image normalization */
    int win_w = cascade->window.width - 1;
    int win_h = cascade->window.height - 1;

    p_offset = pt.y * (cascade->sum.width) + pt.x;

    mean = cascade->sum.data[p_offset]
         - cascade->sum.data[win_w + p_offset]
         - cascade->sum.data[cascade->sum.width * win_h + p_offset]
         + cascade->sum.data[cascade->sum.width * win_h + win_w + p_offset];

    std = sqrtf(sumsq * cascade->window.width * cascade->window.height - mean * mean);

    for (i=start_stage; i<cascade->n_stages; i++) {
        stage_sum = 0;
        for (j=0; j<stages_array[i]; j++, tree_index++, w_index+=3, r_index+=12) {
            /* send the shifted window to a haar filter */
              stage_sum += evalWeakClassifier(&cascade->sum, std, p_offset, tree_index, w_index, r_index);
        } 

        /* If the sum is below the stage threshold, no faces are detected */
        if (stage_sum < 0.4*stages_thresh_array[i]) {
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

    win_size.width =  roundf(cascade->window.width*factor);
    win_size.height =  roundf(cascade->window.height*factor);

    /* When filter window shifts to image boarder, some margin need to be kept */
    y2 = sum_row - win_size.height;
    x2 = sum_col - win_size.width;

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
                r->w = win_size.width;
                r->h = win_size.height;
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


struct array *imlib_detect_objects(struct cascade *cascade, struct frame_buffer *fb)
{
    /* scaling factor */
    float factor;

    /* group overlaping windows */
    const float GROUP_EPS = 0.4f;

    struct array *objects;

    struct frame_buffer img;
    struct integral_image sum;

    /* allocate buffer for scaled image */
    img.width    = fb->width;
    img.height   = fb->height;
    /* use the second half of the frame_buffer */
    img.pixels   = fb->pixels+(fb->width * fb->height);

    /* allocate buffer for integral image */
    sum.width    = fb->width;
    sum.height   = fb->height;
    sum.data   = malloc(fb->width *fb->height*sizeof(*sum.data));

    /* allocate the detections array */
    array_alloc(&objects, free);

    /* set cascade image pointer */
    cascade->img = &img;

    /* iterate over the image pyramid */
    for(factor=1.0f; ; factor*=cascade->scale_factor) {
        /* size of the scaled image */
        struct size sz = { 
            (fb->width/factor), 
            (fb->height/factor) 
        };

        /* if scaled image is smaller than the original detection window, break */
        if ((sz.width  - cascade->window.width)  <= 0 ||
            (sz.height - cascade->window.height) <= 0) {
            break;
        }

        /* Set the width and height of the images */ 
        img.width  = sz.width;
        img.height = sz.height;

        sum.width  = sz.width;
        sum.height = sz.height;

        /* downsample using nearest neighbor */
        imlib_scale_image(fb, &img);

        /* compute a new integral image */
        imlib_integral_image(&img, &sum);

        /* sets images for haar classifier cascade */
        cascade->sum = sum;

        /* process the current scale with the cascaded fitler. */
        ScaleImageInvoker(cascade, factor, sum.height, sum.width, objects);
    } 

    free(sum.data);

    objects = imlib_merge_detections(objects);
    return objects;
}


