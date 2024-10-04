/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * GC2145 register definitions.
 */
#ifndef __GC2145_REGS_H__
#define __GC2145_REGS_H__

#define REG_AMODE1                      (0x17)
#define REG_AMODE1_DEF                  (0x14)
#define REG_AMODE1_SET_HMIRROR(r, x)    ((r & 0xFE) | ((x & 1) << 0))
#define REG_AMODE1_SET_VMIRROR(r, x)    ((r & 0xFD) | ((x & 1) << 1))

#define REG_OUTPUT_FMT                  (0x84)
#define REG_OUTPUT_FMT_RGB565           (0x06)
#define REG_OUTPUT_FMT_YCBYCR           (0x02)
#define REG_OUTPUT_FMT_BAYER            (0x17)
#define REG_OUTPUT_SET_FMT(r, x)        ((r & 0xE0) | (x))

#define REG_SYNC_MODE                   (0x86)
#define REG_SYNC_MODE_DEF               (0x03)
#define REG_SYNC_MODE_COL_SWITCH        (0x10)
#define REG_SYNC_MODE_ROW_SWITCH        (0x20)
#endif //__GC2145_REGS_H__
