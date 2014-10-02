#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv)
{
    printf("#include <stdint.h>\n");
    printf("const float xyz_table[256] = {\n    ");
    for (int i=0; i<256; i++) {
        float t = i/255.0f;
        if (t > 0.04045f) {
            t = powf(((t+0.055f) / 1.055f), 2.4f);
        } else {
            t/= 12.92f;
        }
        t*=100.0f;

        printf("%.6ff, ", t);
        if (i==255) {
            printf("\n");
        } else if ((i+1)%8==0) {
            printf("\n    ");
        }
    }
    printf("};\n");
    return 0;
}
