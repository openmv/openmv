#include "xalloc.h"
#include "imlib.h"
#include <arm_math.h>
float imlib_template_match(struct image *f,  struct image *t_orig, struct rectangle *r)
{
    int den_b=0;
    float corr =0.0;

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
    f_imgs->data = (uint32_t*) (f->data+(f->w * f->h*2));/* after the framebuffer */

    /* get integeral image */
    imlib_integral_image(f, f_imgs);

    /* copy template */
    t->w = (int) t_orig->w;
    t->h = (int) t_orig->h;
    memcpy(t->data, t_orig->data,(sizeof(*t->data)* t_orig->w * t_orig->h));

    /* get normalized template sum of squares */
    int t_mean = imlib_image_mean(t);
    for (int i=0; i < (t->w*t->h); i++) {
        int c = (int8_t)t->data[i]-t_mean;
        den_b += c*c;
        t->data[i]=(int8_t)c;
    }

    for (int v=0; v < f->h - t->h; v+=3) {
    for (int u=0; u < f->w - t->w; u+=3) {
        int num = 0;
        int den_a=0;
        uint32_t f_sum  =imlib_integral_lookup(f_imgs, u, v, t->w, t->h);
        int f_mean = f_sum / (t->w*t->h);

        for (int y=v; y<t->h+v; y++) {
            for (int x=u; x<t->w+u; x++) {
                int a = (int8_t)f->data[y*f->w+x]-f_mean;
                int b = (int8_t)t->data[(y-v)*t->w+(x-u)];
                num += a*b;
                den_a += a*a;
            }
        }

        /* this overflows */
        float c = num/(fast_sqrtf(den_a) *fast_sqrtf(den_b));

        if (c > corr) {
            corr = c;
            r->x = u;
            r->y = v;
            r->w = t->w;
            r->h = t->h;
        }
    }
    }

    xfree(t->data);
    return corr;
}
