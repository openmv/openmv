#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

//255,50,10 -> [55.6477, 73.3233, 65.9473]
struct color {
    union {
        uint8_t v[3];
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        struct {
            int h;
            int s;
            int v;
        };
        struct {
            float L;
            float A;
            float B;
        };
        struct {
            float x;
            float y;
            float z;
        };
    };
};

float lab_distance(struct color *c0, struct color *c1)
{
    float sum=0.0f;
    sum += (c0->L - c1->L) * (c0->L - c1->L);
    sum += (c0->A - c1->A) * (c0->A - c1->A);
    sum += (c0->B - c1->B) * (c0->B - c1->B);
    return sqrtf(sum);
}


void rgb2lab (struct color *rgb, struct color *lab)
{
    float t;
    float v[3];
    float xyz[3];

    for (int i=0; i<3; i++) {
        t = rgb->v[i]/255.0f;
       if (t > 0.04045f) {
           t = powf(((t+0.055f) / 1.055f), 2.4);
       } else {
           t/= 12.92f;
       }
       v[i]=t*100.0f;
    }


    xyz[0] = (v[0] * 0.4124f + v[1] * 0.3576f + v[2] * 0.1805f) / 95.047f  ;
    xyz[1] = (v[0] * 0.2126f + v[1] * 0.7152f + v[2] * 0.0722f) / 100.0f   ;
    xyz[2] = (v[0] * 0.0193f + v[1] * 0.1192f + v[2] * 0.9505f) / 108.883f ;

   for (int i=0; i<3; i++) {
       t = xyz[i];
       if (t > 0.008856f) {
           t = powf(t, 0.333333f);
       } else {
            t = (7.787f * t) + (16.0f/ 116.0f);
       }
       xyz[i]=t;
    }

   lab->L = (116.0f * xyz[1]-16.0f);
   lab->A = (500.0f * (xyz[0]-xyz[1]));
   lab->B = (200.0f * (xyz[1]-xyz[2]));
}

static inline int fround(float flt)
{
  return (int) floor(flt+0.5f);
}

int main(int argc, char **argv)
{
    struct color lab1={0};
    struct color rgb1={0};

    uint8_t p0,p1;
    uint16_t c = 0;

    printf("#include <stdint.h>\n");
    printf("const int8_t lab_table[196608] = {\n");
    for (int i=0; i<65536; i++, c++) {
        p0 = ((char*)&c)[0];
        p1 = ((char*)&c)[1];

        /* map RGB565 to RGB888 */
        rgb1.r = (uint8_t) (p0>>3) * 255/31;
        rgb1.g = (uint8_t) (((p0&0x07)<<3) | (p1>>5)) * 255/63;
        rgb1.b = (uint8_t) (p1&0x1F) * 255/31;

        rgb2lab(&rgb1, &lab1);
        printf("%d, %d, %d, ", fround(lab1.L), fround(lab1.A), fround(lab1.B));
        if ((i+1)%9==0) {
            printf("\n");
        }
    }
    printf("};\n");
    return 0;
}
