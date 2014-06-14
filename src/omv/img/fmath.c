#include "mdefs.h"
#include "fmath.h"
#define M_PI_2  1.57079632f
#define M_PI_4  0.78539816f

const float __atanf_lut[4] = {
    -0.0443265554792128,    //p7
    -0.3258083974640975,    //p3
    +0.1555786518463281,    //p5
    +0.9997878412794807     //p1
};

float ALWAYS_INLINE fast_sqrtf(float x)
{
    asm volatile (
            "vsqrt.f32  %[r], %[x]\n"
            : [r] "=t" (x)
            : [x] "t"  (x));
    return x;
}

int ALWAYS_INLINE fast_floorf(float x)
{
    int i;
    asm volatile (
            "vcvt.S32.f32  %[r], %[x]\n"
            : [r] "=t" (i)
            : [x] "t"  (x));
    return i;
}

int ALWAYS_INLINE fast_roundf(float x)
{
    int i;
    asm volatile (
            "vcvtr.s32.f32  %[r], %[x]\n"
            : [r] "=t" (i)
            : [x] "t"  (x));
    return i;
}

#if 0
float ALWAYS_INLINE fast_atanf(float x)
{
    //return (0.97239f * x) - (0.19195f * x*x*x);
   return M_PI_4*x - x*(fast_fabsf(x) - 1.0f)*(0.2447f + 0.0663f*fast_fabsf(x));
}
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
typedef union{
    uint32_t l;
    struct {
        uint32_t m : 20;
        uint32_t e : 11;
        uint32_t s : 1;
    };
}exp_t;

float fast_expf(float x)
{
    exp_t e;
    e.l = (uint32_t)(1512775 * x + 1072632447);
    // IEEE binary32 format
    e.e = (e.e -1023 + 127) &0xFF; // rebase

    uint32_t packed = (e.s << 31) | (e.e << 23) | e.m <<3;
    return *((float*)&packed);
}
#pragma GCC diagnostic pop

float fast_cbrtf(float f)
{
    unsigned int* p = (unsigned int *) &f;
    *p = *p/3 + 709921077;
    return f;
}

float ALWAYS_INLINE fast_fabsf(float x)
{
    asm volatile (
            "vabs.f32  %[r], %[x]\n"
            : [r] "=t" (x)
            : [x] "t"  (x));
    return x;
}

float fast_atanf(float x)
{

    float a, b, r, xx;
    int m;

    union {
        float f;
        int i;
    } xinv, ax;

    ax.f = fast_fabsf(x);

    //fast inverse approximation (2x newton)
    xinv.f = ax.f;
    m = 0x3F800000 - (xinv.i & 0x7F800000);
    xinv.i = xinv.i + m;
    xinv.f = 1.41176471f - 0.47058824f * xinv.f;
    xinv.i = xinv.i + m;
    b = 2.0 - xinv.f * ax.f;
    xinv.f = xinv.f * b;
    b = 2.0 - xinv.f * ax.f;
    xinv.f = xinv.f * b;

    //if |x| > 1.0 -> ax = -1/ax, r = pi/2
    xinv.f = xinv.f + ax.f;
    a = (ax.f > 1.0f);
    ax.f = ax.f - a * xinv.f;
    r = a * M_PI_2;

    //polynomial evaluation
    xx = ax.f * ax.f;
    a = (__atanf_lut[0] * ax.f) * xx + (__atanf_lut[2] * ax.f);
    b = (__atanf_lut[1] * ax.f) * xx + (__atanf_lut[3] * ax.f);
    xx = xx * xx;
    b = b + a * xx;
    r = r + b;

    //if x < 0 -> r = -r
    a = 2 * r;
    b = (x < 0.0f);
    r = r - a * b;

    return r;
}
