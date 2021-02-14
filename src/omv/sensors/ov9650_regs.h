/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV9650 register definitions.
 */
#ifndef __REG_REGS_H__
#define __REG_REGS_H__

#define REG_GAIN       0x00
#define REG_BLUE       0x01
#define REG_RED        0x02
#define REG_VREF       0x03
#define REG_COM1       0x04
#define REG_BAVE       0x05
#define REG_GEAVE      0x06
#define REG_RAVE       0x08
#define REG_COM2       0x09
#define REG_PID        0x0A
#define REG_VER        0x0B
#define REG_COM3       0x0C
#define REG_COM4       0x0D
#define REG_COM5       0x0E
#define REG_COM6       0x0F
#define REG_AECH       0x10
#define REG_CLKRC      0x11
#define REG_COM7       0x12
#define REG_COM8       0x13
#define REG_COM9       0x14
#define REG_COM10      0x15
#define REG_RSVD16     0x16
#define REG_HSTART     0x17
#define REG_HSTOP      0x18
#define REG_VSTART     0x19
#define REG_VSTOP      0x1A
#define REG_PSHFT      0x1B
#define REG_MIDH       0x1C
#define REG_MIDL       0x1D
#define REG_MVFP       0x1E
#define REG_BOS        0x20
#define REG_GBOS       0x21
#define REG_GROS       0x22
#define REG_ROS        0x23
#define REG_AEW        0x24
#define REG_AEB        0x25
#define REG_VPT        0x26
#define REG_BBIAS      0x27
#define REG_GbBIAS     0x28
#define REG_GRCOM      0x29
#define REG_EXHCH      0x2A
#define REG_EXHCL      0x2B
#define REG_RBIAS      0x2C
#define REG_ADVFL      0x2D
#define REG_ADVFH      0x2E
#define REG_YAVE       0x2F
#define REG_HSYST      0x30
#define REG_HSYEN      0x31
#define REG_HREF       0x32
#define REG_CHLF       0x33
#define REG_ARBLM      0x34
#define REG_RSVD35     0x35
#define REG_RSVD36     0x36
#define REG_ADC        0x37
#define REG_ACOM38     0x38
#define REG_OFON       0x39
#define REG_TSLB       0x3A
#define REG_COM11      0x3B
#define REG_COM12      0x3C
#define REG_COM13      0x3D
#define REG_COM14      0x3E
#define REG_EDGE       0x3F
#define REG_COM15      0x40
#define REG_COM16      0x41
#define REG_COM17      0x42
#define REG_MTX1       0x4F
#define REG_MTX2       0x50
#define REG_MTX3       0x51
#define REG_MTX4       0x52
#define REG_MTX5       0x53
#define REG_MTX6       0x54
#define REG_MTX7       0x55
#define REG_MTX8       0x56
#define REG_MTX9       0x57
#define REG_MTXS       0x58
#define REG_LCC1       0x62
#define REG_LCC2       0x63
#define REG_LCC3       0x64
#define REG_LCC4       0x65
#define REG_LCC5       0x66
#define REG_MANU       0x67
#define REG_MANV       0x68
#define REG_HV         0x69
#define REG_MBD        0x6A
#define REG_DBLV       0x6B
#define REG_COM21      0x8B
#define REG_COM22      0x8C
#define REG_COM23      0x8D
#define REG_COM24      0x8E
#define REG_DBLC1      0x8F
#define REG_DBLC_B     0x90
#define REG_DBLC_R     0x91
#define REG_DMLNL      0x92
#define REG_DMLNH      0x93
#define REG_LCCFB      0x9D
#define REG_LCCFR      0x9E
#define REG_DBLC_GB    0x9F
#define REG_DBLC_GR    0xA0
#define REG_AECHM      0xA1
#define REG_COM25      0xA4
#define REG_COM26      0xA5
#define REG_GGAIN      0xA6
#define REG_VGAST      0xA7

/* register bits */

#define REG_COM1_QQCIF   (1<<5)
#define REG_COM1_QQVGA   (1<<5)
#define REG_COM1_SKIP2   (1<<2)
#define REG_COM1_SKIP3   (1<<3)
#define REG_COM7_RGB     (1<<2)
#define REG_COM7_QCIF    (1<<3)
#define REG_COM7_QVGA    (1<<4)
#define REG_COM7_CIF     (1<<5)
#define REG_COM7_VGA     (1<<6)
#define REG_COM8_AGC     (1<<2)
#define REG_COM8_AWB     (1<<1)
#define REG_COM8_AEC     (1<<0)
#define REG_MVFP_HMIRROR (1<<5)
#define REG_MVFP_VFLIP   (1<<4)
#define REG_CLKRC_DOUBLE (1<<8)
#define REG_CLKRC_DIVIDER_MASK (0x3F)

#endif //__REG_REGS_H__
