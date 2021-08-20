#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define SINCN            3
#define DECIMATION_MAX 128
uint32_t div_const = 0;
uint32_t sinc[DECIMATION_MAX * SINCN];
uint32_t sinc1[DECIMATION_MAX];
uint32_t sinc2[DECIMATION_MAX * 2];
uint32_t coef[SINCN][DECIMATION_MAX];
int32_t lut[256][DECIMATION_MAX / 8][SINCN];

void convolve(uint32_t Signal[/* SignalLen */], unsigned short SignalLen,
              uint32_t Kernel[/* KernelLen */], unsigned short KernelLen,
              uint32_t Result[/* SignalLen + KernelLen - 1 */])
{
  uint16_t n;

  for (n = 0; n < SignalLen + KernelLen - 1; n++)
  {
    unsigned short kmin, kmax, k;
    
    Result[n] = 0;
    
    kmin = (n >= KernelLen - 1) ? n - (KernelLen - 1) : 0;
    kmax = (n < SignalLen - 1) ? n : SignalLen - 1;
    
    for (k = kmin; k <= kmax; k++) {
      Result[n] += Signal[k] * Kernel[n - k];
    }
  }
}


void gen_table(uint32_t decimation)
{
  uint16_t i, j;
  int64_t sum = 0;

  for (i = 0; i < decimation; i++) {
    sinc1[i] = 1;
  }

  sinc[0] = 0;
  sinc[decimation * SINCN - 1] = 0;      
  convolve(sinc1, decimation, sinc1, decimation, sinc2);
  convolve(sinc2, decimation * 2 - 1, sinc1, decimation, &sinc[1]);     
  for(j = 0; j < SINCN; j++) {
    for (i = 0; i < decimation; i++) {
      coef[j][i] = sinc[j * decimation + i];
      sum += sinc[j * decimation + i];
    }
  }

  /* Look-Up Table. */
  uint16_t c, d, s;
  for (s = 0; s < SINCN; s++)
  {
    uint32_t *coef_p = &coef[s][0];
    for (c = 0; c < 256; c++)
      for (d = 0; d < decimation / 8; d++)
        lut[c][d][s] = ((c >> 7)       ) * coef_p[d * 8    ] +
                       ((c >> 6) & 0x01) * coef_p[d * 8 + 1] +
                       ((c >> 5) & 0x01) * coef_p[d * 8 + 2] +
                       ((c >> 4) & 0x01) * coef_p[d * 8 + 3] +
                       ((c >> 3) & 0x01) * coef_p[d * 8 + 4] +
                       ((c >> 2) & 0x01) * coef_p[d * 8 + 5] +
                       ((c >> 1) & 0x01) * coef_p[d * 8 + 6] +
                       ((c     ) & 0x01) * coef_p[d * 8 + 7];
  }

  printf("const int32_t pdm_lut%d[256][DECIMATION_MAX / 8][SINCN] = {", decimation);

  for (int c = 0; c < 256; c++) {
      printf("\n\t{");
      for (d = 0; d < DECIMATION_MAX / 8; d++) {
        printf("{");
        for (int s = 0; s < SINCN; s++) {
            printf("%ld", lut[c][d][s]);
            if (s+1 < SINCN) {
                printf(", ");
            }
        }
        printf("}");
        if (d+1 < (DECIMATION_MAX / 8)) {
            printf(", ");
        }
      }
      printf("}");
      if (c+1 < 256) {
          printf(", ");
      }
  }

  printf("\n};\n");
}

int main(int argc, char **argv) { 
    printf("#ifdef USE_LUT\n");
    printf("#include <stdint.h>\n");
    gen_table(64);
    printf("\n");
    gen_table(128);
    printf("#endif //USE_LUT\n");
    return 0;
}
