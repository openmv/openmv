/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * GC2145 register definitions.
 */
#ifndef __GC2145_REGS_H__
#define __GC2145_REGS_H__

#define REG_AMODE1                      (0x17)
#define REG_AMODE1_DEF                  (0x14)
#define REG_AMODE1_SET_HMIRROR(r, x)    ((r&0xFE)|((x&1)<<0))
#define REG_AMODE1_SET_VMIRROR(r, x)    ((r&0xFD)|((x&1)<<1))

#define REG_OUTPUT_FMT                  (0x84)
#define REG_OUTPUT_FMT_RGB565           (0x06)
#define REG_OUTPUT_FMT_YCBYCR           (0x02)
#define REG_OUTPUT_FMT_BAYER            (0x17)
#define REG_OUTPUT_SET_FMT(r, x)        ((r&0xE0)|(x))

#define REG_SYNC_MODE                   (0x86)
#define REG_SYNC_MODE_DEF               (0x03)
#define REG_SYNC_MODE_COL_SWITCH        (0x10)
#define REG_SYNC_MODE_ROW_SWITCH        (0x20)
#endif //__GC2145_REGS_H__
