#ifndef __SENSOR_CONFIG_H__
#define __SENSOR_CONFIG_H__

#include <stdint.h>

#include "omv_csi.h"
#include "omv_boardconfig.h"

typedef int (*sensor_init_t) (omv_csi_t *);

typedef struct {
    uint32_t chip_id;
    uint32_t clk_hz;
    sensor_init_t init_fun;
} sensor_config_t;

#ifndef OMV_OV2640_CLK_FREQ
#define OMV_OV2640_CLK_FREQ     (24000000)
#endif
extern int ov2640_init(omv_csi_t *csi);

#ifndef OMV_OV5640_CLK_FREQ
#define OMV_OV5640_CLK_FREQ     (24000000)
#endif
extern int ov5640_init(omv_csi_t *csi);

#ifndef OMV_OV7670_CLK_FREQ
#define OMV_OV7670_CLK_FREQ     (24000000)
#endif
extern int ov7670_init(omv_csi_t *csi);

#ifndef OMV_OV7690_CLK_FREQ
#define OMV_OV7690_CLK_FREQ     (24000000)
#endif
extern int ov7690_init(omv_csi_t *csi);

#ifndef OMV_OV7725_CLK_FREQ
#define OMV_OV7725_CLK_FREQ     (12000000)
#endif
extern int ov7725_init(omv_csi_t *csi);

#ifndef OMV_OV9650_CLK_FREQ
#define OMV_OV9650_CLK_FREQ     (12000000)
#endif
extern int ov9650_init(omv_csi_t *csi);

#ifndef OMV_MT9V0XX_CLK_FREQ
#define OMV_MT9V0XX_CLK_FREQ    (26666666)
#endif
extern int mt9v0xx_init(omv_csi_t *csi);

#ifndef OMV_MT9M114_CLK_FREQ
#define OMV_MT9M114_CLK_FREQ    (24000000)
#endif
extern int mt9m114_init(omv_csi_t *csi);

#ifndef OMV_BOSON_CLK_FREQ
#define OMV_BOSON_CLK_FREQ      (0)
#endif
extern int boson_init(omv_csi_t *csi);

#ifndef OMV_LEPTON_CLK_FREQ
#define OMV_LEPTON_CLK_FREQ     (24000000)
#endif
extern int lepton_init(omv_csi_t *csi);

#ifndef OMV_HM01B0_CLK_FREQ
#define OMV_HM01B0_CLK_FREQ     (6000000)
#endif
extern int hm01b0_init(omv_csi_t *csi);

#ifndef OMV_HM0360_CLK_FREQ
#define OMV_HM0360_CLK_FREQ     (0)
#endif
extern int hm0360_init(omv_csi_t *csi);

#ifndef OMV_GC2145_CLK_FREQ
#define OMV_GC2145_CLK_FREQ     (12000000)
#endif
extern int gc2145_init(omv_csi_t *csi);

#ifndef OMV_GENX320_CLK_FREQ
#define OMV_GENX320_CLK_FREQ    (24000000)
#endif
extern int genx320_init(omv_csi_t *csi);

#ifndef OMV_PAG7920_CLK_FREQ
#define OMV_PAG7920_CLK_FREQ    (24000000)
#endif
extern int pag7920_init(omv_csi_t *csi);

#ifndef OMV_PAG7936_CLK_FREQ
#define OMV_PAG7936_CLK_FREQ    (24000000)
#endif
extern int pag7936_init(omv_csi_t *csi);

#ifndef OMV_PAJ6100_CLK_FREQ
#define OMV_PAJ6100_CLK_FREQ    (6000000)
#endif
extern int paj6100_init(omv_csi_t *csi);
extern bool paj6100_detect(omv_csi_t *csi);

#ifndef OMV_PS5520_CLK_FREQ
#define OMV_PS5520_CLK_FREQ     (24000000)
#endif
extern int ps5520_init(omv_csi_t *csi);

#ifndef OMV_FROGEYE2020_CLK_FREQ
#define OMV_FROGEYE2020_CLK_FREQ    (5000000)
#endif
extern int frogeye2020_init(omv_csi_t *csi);

#ifndef OMV_SOFTCSI_CLK_FREQ
#define OMV_SOFTCSI_CLK_FREQ    (24000000)
#endif
extern int softcsi_init(omv_csi_t *csi);

// Sensor table *INDENT-OFF*
static const sensor_config_t sensor_config_table[] = {
    #if OMV_OV2640_ENABLE
    { OV2640_ID, OMV_OV2640_CLK_FREQ, ov2640_init },
    #endif

    #if OMV_OV5640_ENABLE
    { OV5640_ID, OMV_OV5640_CLK_FREQ, ov5640_init },
    #endif

    #if OMV_OV7670_ENABLE
    { OV7670_ID, OMV_OV7670_CLK_FREQ, ov7670_init },
    #endif

    #if OMV_OV7690_ENABLE
    { OV7690_ID, OMV_OV7690_CLK_FREQ, ov7690_init },
    #endif

    #if OMV_OV7725_ENABLE
    { OV7725_ID, OMV_OV7725_CLK_FREQ, ov7725_init },
    #endif

    #if OMV_OV9650_ENABLE
    { OV9650_ID, OMV_OV9650_CLK_FREQ, ov9650_init },
    #endif

    #if OMV_MT9V0XX_ENABLE
    { MT9V0X2_ID_V_1, OMV_MT9V0XX_CLK_FREQ, mt9v0xx_init },
    { MT9V0X2_ID_V_2, OMV_MT9V0XX_CLK_FREQ, mt9v0xx_init },
    { MT9V0X2_ID,     OMV_MT9V0XX_CLK_FREQ, mt9v0xx_init },
    { MT9V0X4_ID,     OMV_MT9V0XX_CLK_FREQ, mt9v0xx_init },
    #endif

    #if OMV_MT9M114_ENABLE
    { MT9M114_ID, OMV_MT9M114_CLK_FREQ, mt9m114_init },
    #endif

    #if OMV_BOSON_ENABLE
    { BOSON_ID,      OMV_BOSON_CLK_FREQ, boson_init },
    #endif

    #if OMV_LEPTON_ENABLE
    { LEPTON_ID,     OMV_LEPTON_CLK_FREQ, lepton_init },
    #endif

    #if OMV_HM01B0_ENABLE
    { HM01B0_ID, OMV_HM01B0_CLK_FREQ, hm01b0_init },
    #endif

    #if OMV_HM0360_ENABLE
    { HM0360_ID, OMV_HM0360_CLK_FREQ, hm0360_init },
    #endif

    #if OMV_GC2145_ENABLE
    { GC2145_ID, OMV_GC2145_CLK_FREQ, gc2145_init },
    #endif

    #if OMV_GENX320_ENABLE
    { GENX320_ID_ES, OMV_GENX320_CLK_FREQ, genx320_init },
    { GENX320_ID_MP, OMV_GENX320_CLK_FREQ, genx320_init },
    #endif

    #if OMV_PAG7920_ENABLE
    { PAG7920_ID, OMV_PAG7920_CLK_FREQ, pag7920_init },
    #endif

    #if OMV_PAG7936_ENABLE
    { PAG7936_ID, OMV_PAG7936_CLK_FREQ, pag7936_init },
    #endif

    #if OMV_PAJ6100_ENABLE
    { PAJ6100_ID, OMV_PAJ6100_CLK_FREQ, paj6100_init },
    #endif

    #if OMV_PS5520_ENABLE
    { PS5520_ID, OMV_PS5520_CLK_FREQ, ps5520_init },
    #endif

    #if OMV_FROGEYE2020_ENABLE
    { FROGEYE2020_ID, OMV_FROGEYE2020_CLK_FREQ, frogeye2020_init },
    #endif

    #if OMV_SOFTCSI_ENABLE
    { SOFTCSI_ID, OMV_SOFTCSI_CLK_FREQ, softcsi_init },
    #endif
};
#endif // __CSI_CONFIG_H__
