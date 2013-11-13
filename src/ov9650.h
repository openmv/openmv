#ifndef __OV9650_H__
#define __OV9650_H__
#include <stdint.h>
/* OV9650 Registers definition */
#define OV9650_GAIN       0x00
#define OV9650_BLUE       0x01
#define OV9650_RED        0x02
#define OV9650_VREF       0x03
#define OV9650_COM1       0x04
#define OV9650_BAVE       0x05
#define OV9650_GEAVE      0x06
#define OV9650_RAVE       0x08
#define OV9650_COM2       0x09
#define OV9650_PID        0x0A
#define OV9650_VER        0x0B
#define OV9650_COM3       0x0C
#define OV9650_COM4       0x0D
#define OV9650_COM5       0x0E
#define OV9650_COM6       0x0F
#define OV9650_AECH       0x10
#define OV9650_CLKRC      0x11
#define OV9650_COM7       0x12
#define OV9650_COM8       0x13
#define OV9650_COM9       0x14
#define OV9650_COM10      0x15
#define OV9650_RSVD16     0x16
#define OV9650_HSTART     0x17
#define OV9650_HSTOP      0x18
#define OV9650_VSTART     0x19
#define OV9650_VSTOP      0x1A
#define OV9650_PSHFT      0x1B
#define OV9650_MIDH       0x1C
#define OV9650_MIDL       0x1D
#define OV9650_MVFP       0x1E
#define OV9650_BOS        0x20
#define OV9650_GBOS       0x21
#define OV9650_GROS       0x22
#define OV9650_ROS        0x23
#define OV9650_AEW        0x24
#define OV9650_AEB        0x25
#define OV9650_VPT        0x26
#define OV9650_BBIAS      0x27
#define OV9650_GbBIAS     0x28
#define OV9650_GRCOM      0x29
#define OV9650_EXHCH      0x2A
#define OV9650_EXHCL      0x2B
#define OV9650_RBIAS      0x2C
#define OV9650_ADVFL      0x2D
#define OV9650_ADVFH      0x2E
#define OV9650_YAVE       0x2F
#define OV9650_HSYST      0x30
#define OV9650_HSYEN      0x31
#define OV9650_HREF       0x32
#define OV9650_CHLF       0x33
#define OV9650_ARBLM      0x34
#define OV9650_RSVD35     0x35
#define OV9650_RSVD36     0x36
#define OV9650_ADC        0x37
#define OV9650_ACOM38     0x38
#define OV9650_OFON       0x39
#define OV9650_TSLB       0x3A
#define OV9650_COM11      0x3B
#define OV9650_COM12      0x3C
#define OV9650_COM13      0x3D
#define OV9650_COM14      0x3E
#define OV9650_EDGE       0x3F
#define OV9650_COM15      0x40
#define OV9650_COM16      0x41
#define OV9650_COM17      0x42
#define OV9650_MTX1       0x4F
#define OV9650_MTX2       0x50
#define OV9650_MTX3       0x51
#define OV9650_MTX4       0x52
#define OV9650_MTX5       0x53
#define OV9650_MTX6       0x54
#define OV9650_MTX7       0x55
#define OV9650_MTX8       0x56
#define OV9650_MTX9       0x57
#define OV9650_MTXS       0x58
#define OV9650_LCC1       0x62
#define OV9650_LCC2       0x63
#define OV9650_LCC3       0x64
#define OV9650_LCC4       0x65
#define OV9650_LCC5       0x66
#define OV9650_MANU       0x67
#define OV9650_MANV       0x68
#define OV9650_HV         0x69
#define OV9650_MBD        0x6A
#define OV9650_DBLV       0x6B
#define OV9650_COM21      0x8B
#define OV9650_COM22      0x8C
#define OV9650_COM23      0x8D
#define OV9650_COM24      0x8E
#define OV9650_DBLC1      0x8F
#define OV9650_DBLC_B     0x90
#define OV9650_DBLC_R     0x91
#define OV9650_DMLNL      0x92
#define OV9650_DMLNH      0x93
#define OV9650_LCCFB      0x9D
#define OV9650_LCCFR      0x9E
#define OV9650_DBLC_GB    0x9F
#define OV9650_DBLC_GR    0xA0
#define OV9650_AECHM      0xA1
#define OV9650_COM25      0xA4
#define OV9650_COM26      0xA5
#define OV9650_GGAIN      0xA6
#define OV9650_VGAST      0xA7
enum ov9650_config_t { 
  QQVGA_RGB565, /* QQVGA160x120/RGB565*/
  QQVGA_YUV422, /* QQVGA160x120/YUV422*/
};

struct ov9650_id_t {
    uint8_t MIDH;
    uint8_t MIDL;
    uint8_t PID;
    uint8_t VER;
};

/* Hardware initialization */
int ov9650_init();
/* Reset sensor */
void ov9650_reset();
void ov9650_read_id(struct ov9650_id_t *id);
/* Configure sensor */
int ov9650_config(enum ov9650_config_t config);
int ov9650_set_brightness(int level);
void ov9650_set_exposure(uint16_t exposure);
int ov9650_snapshot();
uint8_t *get_frame_buffer();
void delay(uint32_t ntime);
#endif /* __OV9650_H__ */
