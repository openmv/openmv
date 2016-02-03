#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv)
{
    int r, g, b;
    int y, u, v;

    uint8_t p0, p1;
    uint16_t c = 0;

    printf("#include <stdint.h>\n");
    printf("const int8_t yuv_table[196608] = {\n");
    for (int i=0; i<65536; i++, c++) {
        p0 = ((char*)&c)[0];
        p1 = ((char*)&c)[1];

        // Convert RGB565 to RGB888
        r = (p0>>3) * 255/31;
        g = (((p0&0x07)<<3) | (p1>>5)) * 255/63;
        b = (p1&0x1F) * 255/31;

        // Convert RGB888 to YUV
        // ITU-R 601 coefficients
        // Note this will generate signed YUV values
        y = r * +0.29900f + g * +0.58700f + b * +0.11400f-128;
        u = r * -0.16874f + g * -0.33126f + b * +0.50000f;
        v = r * +0.50000f + g * -0.41869f + b * -0.08131f;

        printf("%d, %d, %d, ", y, u, v);

        if ((i+1)%9==0) {
            printf("\n");
        }
    }
    printf("};\n");
    return 0;
}
