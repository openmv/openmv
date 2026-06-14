/*
 * Copyright (c) 2015-2022, Verisilicon Inc. - All Rights Reserved
 * Copyright (c) 2011-2014, Google Inc. - All Rights Reserved
 *
 *
 ********************************************************************************
 *
 * This software is distributed under the terms of
 * BSD-3-Clause. The following provisions apply :
 *
 ********************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ********************************************************************************
 *
 *  Description : Encoder common configuration parameters
 *
 ********************************************************************************
 */
#ifndef __ENCCFG_H__
#define __ENCCFG_H__

#include "basetype.h"

/* Here is defined the default values for the encoder build-time configuration.
 * You can override these settings by defining the values as compiler flags 
 * in the Makefile.
 */

#ifdef PCIE_FPGA_VERIFICATION
#define ENCH1_INPUT_SWAP_32_YUV                   1
#define ENCH1_INPUT_SWAP_32_RGB16                 1
#define ENCH1_INPUT_SWAP_32_RGB32                 1
#define ENCH1_OUTPUT_SWAP_32                      1
#endif

/* The input image's 32-bit swap: 0 or 1
 * This defines the 32-bit endianess of the ASIC input YUV
 * 1 = 64-bit endianess */
#ifndef ENCH1_INPUT_SWAP_32_YUV
#define ENCH1_INPUT_SWAP_32_YUV                   1
#endif

/* The input image's 16-bit swap: 0 or 1
 * This defines the 16-bit endianess of the ASIC input YUV
 */
#ifndef ENCH1_INPUT_SWAP_16_YUV
#define ENCH1_INPUT_SWAP_16_YUV                   1
#endif

/* The input image's 8-bit swap: 0 or 1
 * This defines the byte endianess of the ASIC input YUV
 */
#ifndef ENCH1_INPUT_SWAP_8_YUV
#define ENCH1_INPUT_SWAP_8_YUV                    1
#endif

/* The input image's 32-bit swap: 0 or 1
 * This defines the 32-bit endianess of the ASIC input RGB16
 * 1 = 64-bit endianess */
#ifndef ENCH1_INPUT_SWAP_32_RGB16
#define ENCH1_INPUT_SWAP_32_RGB16                 1
#endif

/* The input image's 16-bit swap: 0 or 1
 * This defines the 16-bit endianess of the ASIC input RGB16
 */
#ifndef ENCH1_INPUT_SWAP_16_RGB16
#define ENCH1_INPUT_SWAP_16_RGB16                 1
#endif

/* The input image's byte swap: 0 or 1
 * This defines the byte endianess of the ASIC input RGB16
 */
#ifndef ENCH1_INPUT_SWAP_8_RGB16
#define ENCH1_INPUT_SWAP_8_RGB16                  0
#endif

/* The input image's 32-bit swap: 0 or 1
 * This defines the 32-bit endianess of the ASIC input RGB32
 * 1 = 64-bit endianess */
#ifndef ENCH1_INPUT_SWAP_32_RGB32
#define ENCH1_INPUT_SWAP_32_RGB32                 1
#endif

/* The input image's 16-bit swap: 0 or 1
 * This defines the 16-bit endianess of the ASIC input RGB32
 */
#ifndef ENCH1_INPUT_SWAP_16_RGB32
#define ENCH1_INPUT_SWAP_16_RGB32                 0
#endif

/* The input image's byte swap: 0 or 1
 * This defines the byte endianess of the ASIC input RGB32
 */
#ifndef ENCH1_INPUT_SWAP_8_RGB32
#define ENCH1_INPUT_SWAP_8_RGB32                  0
#endif

/* ENCH1_OUTPUT_SWAP_XX define the byte endianess of the ASIC output stream.
 * This MUST be configured to be the same as the native system endianess,
 * because the control software relies on system endianess when reading
 * the data from the memory. */

/* The output stream's 32-bit swap: 0 or 1
 * This defines the 32-bit endianess of the ASIC output stream
 * 1 = 64-bit endianess */
#ifndef ENCH1_OUTPUT_SWAP_32
#define ENCH1_OUTPUT_SWAP_32                      1
#endif

/* The output stream's 16-bit swap: 0 or 1
 * This defines the 16-bit endianess of the ASIC output stream.
 */
#ifndef ENCH1_OUTPUT_SWAP_16
#define ENCH1_OUTPUT_SWAP_16                      1
#endif

/* The output stream's 8-bit swap: 0 or 1
 * This defines the byte endianess of the ASIC output stream.
 */
#ifndef ENCH1_OUTPUT_SWAP_8
#define ENCH1_OUTPUT_SWAP_8                       1
#endif

/* The output down-scaled image's swapping is defined the same way as
 * output stream swapping. Typically these should be the same values
 * as for output stream. */
#ifndef ENCH1_SCALE_OUTPUT_SWAP_32
#define ENCH1_SCALE_OUTPUT_SWAP_32                  ENCH1_OUTPUT_SWAP_32
#endif
#ifndef ENCH1_SCALE_OUTPUT_SWAP_16
#define ENCH1_SCALE_OUTPUT_SWAP_16                  ENCH1_OUTPUT_SWAP_16
#endif
#ifndef ENCH1_SCALE_OUTPUT_SWAP_8
#define ENCH1_SCALE_OUTPUT_SWAP_8                   ENCH1_OUTPUT_SWAP_8
#endif

/* The output MV and macroblock info swapping is defined the same way as
 * output stream swapping. Typically these should be the same values
 * as for output stream. */
#ifndef ENCH1_MV_OUTPUT_SWAP_32
#define ENCH1_MV_OUTPUT_SWAP_32                     ENCH1_OUTPUT_SWAP_32
#endif
#ifndef ENCH1_MV_OUTPUT_SWAP_16
#define ENCH1_MV_OUTPUT_SWAP_16                     ENCH1_OUTPUT_SWAP_16
#endif
#ifndef ENCH1_MV_OUTPUT_SWAP_8
#define ENCH1_MV_OUTPUT_SWAP_8                      ENCH1_OUTPUT_SWAP_8
#endif

/* ASIC interrupt enable.
 * This enables/disables the ASIC to generate interrupts
 * If this is '1', the EWL must poll the registers to find out
 * when the HW is ready.
 */
#ifndef ENCH1_IRQ_DISABLE
#define ENCH1_IRQ_DISABLE                         0
#endif

/* ASIC bus interface configuration values                  */
/* DO NOT CHANGE IF NOT FAMILIAR WITH THE CONCEPTS INVOLVED */

/* Burst length. This sets the maximum length of a single ASIC burst in addresses.
 * Allowed values are:
 *          AHB {0, 4, 8, 16} ( 0 means incremental burst type INCR)
 *          OCP [1,63]
 *          AXI [1,16]
 */
#ifndef ENCH1_BURST_LENGTH
#define ENCH1_BURST_LENGTH                               16
#endif

/* SCMD burst mode disable                                                    */
/* 0 - enable SCMD burst mode                                                 */
/* 1 - disable SCMD burst mode                                                */
#ifndef ENCH1_BURST_SCMD_DISABLE
#define ENCH1_BURST_SCMD_DISABLE                         0
#endif

/* INCR type burst mode                                                       */
/* 0 - enable INCR type bursts                                                */
/* 1 - disable INCR type and use SINGLE instead                               */
#ifndef ENCH1_BURST_INCR_TYPE_ENABLED
#define ENCH1_BURST_INCR_TYPE_ENABLED                    0
#endif

/* Data discard mode. When enabled read bursts of length 2 or 3 are converted */
/* to BURST4 and  useless data is discarded. Otherwise use INCR type for that */
/* kind  of read bursts */
/* 0 - disable data discard                                                   */
/* 1 - enable data discard                                                    */
#ifndef ENCH1_BURST_DATA_DISCARD_ENABLED
#define ENCH1_BURST_DATA_DISCARD_ENABLED                 0
#endif

/* AXI bus read and write ID values used by HW. 0 - 255 */
#ifndef ENCH1_AXI_READ_ID
#define ENCH1_AXI_READ_ID                                0
#endif

#define ENCH1_AXI_READ_ID_C0                              1
#define ENCH1_AXI_READ_ID_C1                              2
#define ENCH1_AXI_READ_ID_C2                              3
#ifndef ENCH1_AXI_READ_ID_EN
#define ENCH1_AXI_READ_ID_EN                              1
#endif

#ifndef ENCH1_AXI_WRITE_ID
#define ENCH1_AXI_WRITE_ID                               0
#endif

/* End of "ASIC bus interface configuration values"                           */

/* ASIC internal clock gating control. 0 - disabled, 1 - enabled              */
#ifndef ENCH1_ASIC_CLOCK_GATING_ENABLED
#define ENCH1_ASIC_CLOCK_GATING_ENABLED                  0
#endif

/* ASIC timeout interrupt enable/disable                                      */
#ifndef ENCH1_TIMEOUT_INTERRUPT
#define ENCH1_TIMEOUT_INTERRUPT                          1
#endif

/* H.264 slice ready interrupt enable/disable. When enabled the HW will raise */
/* interrupt after every completed slice creating several IRQ per frame.      */
/* When disabled the HW will raise interrupt only when the frame encoding is  */
/* finished.                                                                  */
#ifndef ENCH1_SLICE_READY_INTERRUPT
#define ENCH1_SLICE_READY_INTERRUPT                      1
#endif

/* ASIC input picture read chunk size, 0=4 MBs, 1=1 MB                        */
#ifndef ENCH1_INPUT_READ_CHUNK
#define ENCH1_INPUT_READ_CHUNK                           0
#endif

/* AXI bus dual channel disable, 0=use two channels, 1=use single channel.   */
#ifndef ENCH1_AXI_2CH_DISABLE
#define ENCH1_AXI_2CH_DISABLE                           1   /* for STM Panther 0=>1 */
#endif

/* Reconstructed picture output writing, 0=1 MB (write burst for every MB),
   1=4MBs (write burst for every fourth MB).                                 */
#ifndef ENCH1_REC_WRITE_BUFFER
#define ENCH1_REC_WRITE_BUFFER                          1
#endif

/* How often to update VP8 entropy probabilities, higher value lowers SW load.
   1=update for every frame, 2=update for every second frame, etc.           */
#ifndef ENCH1_VP8_ENTROPY_UPDATE_DISTANCE
#define ENCH1_VP8_ENTROPY_UPDATE_DISTANCE               1
#endif

/* ASIC RFC overflow interrupt enable/disable                                      */
#ifndef ENCH1_RFC_OVERFLOW_INTERRUPT
#define ENCH1_RFC_OVERFLOW_INTERRUPT                    1
#endif

#endif
