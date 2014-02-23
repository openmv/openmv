#include <libmp.h>
#include "xalloc.h"
#include "imlib.h"
#include <math.h>
#include <arm_math.h>

int imlib_save_template(struct image *image, const char *path)
{
    UINT n_out;

    FIL fp;
    FRESULT res=FR_OK;

    res = f_open(&fp, path, FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
    if (res != FR_OK) {
        return res;
    }

    /* write template width */
    res = f_write(&fp, &image->w, sizeof(image->w), &n_out);
    if (res != FR_OK || n_out != sizeof(image->w)) {
        goto error;
    }

    /* write template height */
    res = f_write(&fp, &image->h, sizeof(image->h), &n_out);
    if (res != FR_OK || n_out != sizeof(image->h)) {
        goto error;
    }

    /* write template data */
    res = f_write(&fp, image->data, image->w*image->h*sizeof(*image->data), &n_out);
    if (res != FR_OK || n_out != image->w*image->h*sizeof(*image->data)) {
        goto error;
    }

error:
    f_close(&fp);
    return res;
}

int imlib_load_template(struct image *image, const char *path)
{
    UINT n_out;

    FIL fp;
    FRESULT res=FR_OK;

    res = f_open(&fp, path, FA_READ|FA_OPEN_EXISTING);
    if (res != FR_OK) {
        return res;
    }

    /* read template width */
    res = f_read(&fp, &image->w, sizeof(image->w), &n_out);
    if (res != FR_OK || n_out != sizeof(image->w)) {
        goto error;
    }

    /* read template height */
    res = f_read(&fp, &image->h, sizeof(image->h), &n_out);
    if (res != FR_OK || n_out != sizeof(image->h)) {
        goto error;
    }

    printf("temp:%d %d \n", image->w, image->h);
    image->data = xalloc(sizeof(*image->data)*image->w*image->h);
    if (image->data == NULL) {
        goto error;
    }
    /* read template data */
    res = f_read(&fp, image->data, image->w*image->h*sizeof(*image->data), &n_out);
    if (res != FR_OK || n_out != image->w*image->h*sizeof(*image->data)) {
        goto error;
    }

error:
    f_close(&fp);
    return res;
}

float imlib_template_match(struct image *f,  struct image *t_orig, struct rectangle *r)
{
    int x,y,u,v;
    float corr =0.0;
    float factor,scale_factor = 0.5f;

    struct image img;
    struct image *t=&img;

    struct integral_image sum;
    struct integral_image *f_imgs=&sum;

    /* allocate buffer for scaled template */
    t->w = t_orig->w;
    t->h = t_orig->h;
    t->bpp = t_orig->bpp;
    t->data = xalloc(sizeof(*t->data)* t_orig->w * t_orig->h);

    /* allocate buffer for integral image */
    f_imgs->w = f->w;
    f_imgs->h = f->h;
    /* after the framebuffer */
    f_imgs->data = (uint32_t*) (f->data+(f->w * f->h*2));

    /* get integeral image */
    imlib_integral_image(f, f_imgs);

    for(factor=1.0f; (factor>0.25f); factor*=scale_factor) {
        t->w = (int) t_orig->w*factor;
        t->h = (int) t_orig->h*factor;
        imlib_scale_image(t_orig, t);

        int i;
        long b_den=0;
        int t_mean = imlib_image_mean(t);

        /* get normalized template sum of squares */
        for (i=0; i < (t->w*t->h); i++) {
            int c = t->data[i]-t_mean;
            b_den += c*c;
            t->data[i]=(int8_t)c;
        }

        for (v=0; v < f->h - t->h; v+=3) {
        for (u=0; u < f->w - t->w; u+=3) {
            int sum_shift = 0;
            int f_sum = imlib_integral_lookup(f_imgs, u, v, t->w, t->h);
            int f_mean = f_sum / (t->w*t->h);
            //int f_sumsq = imlib_integral_lookup(f_imgsq, u, v, t->w, t->h);
            int f_sumsq=0;

            for (y=v; y<t->h+v; y++) {
                for (x=u; x<t->w+u; x++) {
                    int a = f->data[y*f->w+x]-f_mean;
                    int b = (int8_t) t->data[(y-v)*t->w+(x-u)];
                    sum_shift += a*b;
                    f_sumsq += f->data[y*f->w+x] * f->data[y*f->w+x];
                }
            }

            int a_den = f_sumsq-(f_sum*(long)f_sum)/(t->w*t->h);
            /* this overflows */
            //corr = sum_shift/sqrtf(a_den*b_den);
            float c = sum_shift/(sqrtf(a_den)*sqrtf(b_den));

            if (c > corr) {
                corr = c;
                r->x = u;
                r->y = v;
                r->w = t->w;
                r->h = t->h;
            }
        }
        }
    }

    xfree(t->data);
    return corr;
}
