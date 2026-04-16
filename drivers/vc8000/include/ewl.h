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
 *  Abstract : Hantro Encoder Wrapper Layer for OS services
 *
 ********************************************************************************
 */

#ifndef __EWL_H__
#define __EWL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "basetype.h"
/* Return values */
#define EWL_OK                      0
#define EWL_ERROR                  -1

#define EWL_HW_WAIT_OK              EWL_OK
#define EWL_HW_WAIT_ERROR           EWL_ERROR
#define EWL_HW_WAIT_TIMEOUT         1

/* HW configuration values */
#define EWL_HW_BUS_TYPE_UNKNOWN     0
#define EWL_HW_BUS_TYPE_AHB         1
#define EWL_HW_BUS_TYPE_OCP         2
#define EWL_HW_BUS_TYPE_AXI         3
#define EWL_HW_BUS_TYPE_PCI         4

#define EWL_HW_BUS_WIDTH_UNKNOWN     0
#define EWL_HW_BUS_WIDTH_32BITS      1
#define EWL_HW_BUS_WIDTH_64BITS      2
#define EWL_HW_BUS_WIDTH_128BITS     3

#define EWL_HW_SYNTHESIS_LANGUAGE_UNKNOWN     0
#define EWL_HW_SYNTHESIS_LANGUAGE_VHDL        1
#define EWL_HW_SYNTHESIS_LANGUAGE_VERILOG     2

#define EWL_HW_CONFIG_NOT_SUPPORTED    0
#define EWL_HW_CONFIG_ENABLED          1

#define ASIC_STATUS_RFC_BUFF_OVERFLOW   0x800U
#define ASIC_STATUS_LINE_BUFFER_DONE    0x400U
#define ASIC_STATUS_FUSE                0x200U
#define ASIC_STATUS_SLICE_READY         0x100U
#define ASIC_STATUS_HW_TIMEOUT          0x040U
#define ASIC_STATUS_BUFF_FULL           0x020U
#define ASIC_STATUS_HW_RESET            0x010U
#define ASIC_STATUS_ERROR               0x008U
#define ASIC_STATUS_FRAME_READY         0x004U
#define ASIC_IRQ_LINE                   0x001U

#define ASIC_STATUS_ALL     (ASIC_STATUS_RFC_BUFF_OVERFLOW   |\
                             ASIC_STATUS_LINE_BUFFER_DONE    |\
                             ASIC_STATUS_FUSE                |\
                             ASIC_STATUS_SLICE_READY         |\
                             ASIC_STATUS_HW_TIMEOUT          |\
                             ASIC_STATUS_BUFF_FULL           |\
                             ASIC_STATUS_HW_RESET            |\
                             ASIC_STATUS_ERROR               |\
                             ASIC_STATUS_FRAME_READY)

/* new added fuse 2 bitmap */
#define HWCFGAddress64Bits     0x80000000U
#define HWCFGDnfSupport        0x40000000U
#define HWCFGRfcSupport        0x30000000U
#define HWCFGQEnhanceSupport   0x08000000U
#define HWCFGInstantSupport    0x04000000U
#define HWCFGSvctSupport       0x02000000U
#define HWCFGAxiIdInSupport    0x01000000U
#define HWCFGIrqClearSupport   0x00800000U
#define HWCFGInLoopbackSupport 0x00400000U

/* Hardware configuration description */
    typedef struct EWLHwConfig
    {
        /* first part in reg 63 */
        u32 maxEncodedWidth; /* Maximum supported width for video encoding (not JPEG) */
        u32 h264Enabled;     /* HW supports H.264 */
        u32 jpegEnabled;     /* HW supports JPEG */
        u32 vp8Enabled;      /* HW supports VP8 */
        u32 vsEnabled;       /* HW supports video stabilization */
        u32 rgbEnabled;      /* HW supports RGB input */
        u32 searchAreaSmall; /* HW search area reduced */
        u32 scalingEnabled;  /* HW supports down-scaling */
        u32 busType;         /* HW bus type in use */
        u32 busWidth;
        u32 synthesisLanguage;

        /* second part in reg 296 */
        u32 addr64Support;  /* HW supports 64bit bus address */
        u32 dnfSupport;     /* HW supports Denoise filter */
        u32 enhanceSupport; /* HW supports quality enhancement */
        u32 rfcSupport;     /* HW supports reference frame compression */
        u32 instantSupport; /* HW supports low latency control */
        u32 svctSupport;    /* HW supports SVCT */
        u32 inAxiIdSupport; /* HW supports configurable AXI Read ID for Input picture */
        u32 irqEnhanceSupport; /* HW supports to clear irq by write-one */
        u32 inLoopbackSupport;  /* HW support input buffer loopback. */
    } EWLHwConfig_t;

/* Allocated linear memory area information */
    typedef struct EWLLinearMem
    {
        u32 *virtualAddress;
        u32 size;
        ptr_t busAddress;
    } EWLLinearMem_t;

/* EWLInitParam is used to pass parameters when initializing the EWL */
    typedef struct EWLInitParam
    {
        u32 clientType;
    } EWLInitParam_t;

#define EWL_CLIENT_TYPE_H264_ENC         1U
#define EWL_CLIENT_TYPE_VP8_ENC          2U
#define EWL_CLIENT_TYPE_JPEG_ENC         3U
#define EWL_CLIENT_TYPE_VIDEOSTAB        4U

extern u32 (*pollInputLineBufTestFunc)(void);

/*------------------------------------------------------------------------------
    4.  Function prototypes
------------------------------------------------------------------------------*/

/* Read and return the HW ID register value, static implementation */
    u32 EWLReadAsicID(void);

/* Read and return HW configuration info, static implementation */
    EWLHwConfig_t EWLReadAsicConfig(void);

/* Initialize the EWL instance 
 * Returns a wrapper instance or NULL for error
 * EWLInit is called when the encoder instance is initialized */
    const void *EWLInit(EWLInitParam_t * param);

/* Release the EWL instance
 * Returns EWL_OK or EWL_ERROR
 * EWLRelease is called when the encoder instance is released */
    i32 EWLRelease(const void *inst);

/* Reserve the HW resource for one codec instance
 * EWLReserveHw is called when beginning a frame encoding
 * The function may block until the resource is available. 
 * Returns EWL_OK if the resource was successfully reserved for this instance
 * or EWL_ERROR if unable to reserve the resource. */
    i32 EWLReserveHw(const void *inst);

/* Release the HW resource
 * EWLReleaseHw is called when the HW has finished the frame encoding.
 * The codec SW will continue the frame encoding but the HW can
 * be used by another codec.*/
    void EWLReleaseHw(const void *inst);

/* Frame buffers memory */
    i32 EWLMallocRefFrm(const void *inst, u32 size, EWLLinearMem_t * info);
    void EWLFreeRefFrm(const void *inst, EWLLinearMem_t * info);

/* SW/HW shared memory */
    i32 EWLMallocLinear(const void *inst, u32 size, EWLLinearMem_t * info);
    void EWLFreeLinear(const void *inst, EWLLinearMem_t * info);

    /* D-Cache coherence *//* Not in use currently */
    void EWLDCacheRangeFlush(const void *instance, EWLLinearMem_t * info);
    void EWLDCacheRangeRefresh(const void *instance, EWLLinearMem_t * info);

/* Write value to a HW register
 * All registers are written at once at the beginning of frame encoding 
 * Offset is relative to the the HW ID register (#0) in bytes 
 * Enable indicates when the HW is enabled. If shadow registers are used then
 * they must be flushed to the HW registers when enable is '1' before
 * writing the register that enables the HW */
    void EWLWriteReg(const void *inst, u32 offset, u32 val);

/* Read and return the value of a HW register
 * The status register is read after every macroblock encoding by SW
 * The other registers which may be updated by the HW are read after
 * BUFFER_FULL or FRAME_READY interrupt
 * Offset is relative to the the HW ID register (#0) in bytes */
    u32 EWLReadReg(const void *inst, u32 offset);

    /* Writing all registers in one call *//*Not in use currently */
    void EWLWriteRegAll(const void *inst, const u32 * table, u32 size);
    /* Reading all registers in one call *//*Not in use currently */
    void EWLReadRegAll(const void *inst, u32 * table, u32 size);

/* HW enable/disable. This will write <val> to register <offset> and by */
/* this enablig/disabling the hardware. */
    void EWLEnableHW(const void *inst, u32 offset, u32 val);
    void EWLDisableHW(const void *inst, u32 offset, u32 val);

/* Synchronize SW with HW
 * Returns EWL_HW_WAIT_OK, EWL_HW_WAIT_ERROR or EWL_HW_WAIT_TIMEOUT
 * EWLWaitHwRdy is called after enabling the HW to wait for IRQ from HW.
 * If slicesReady pointer is given, at input it should contain the number
 * of slicesReady received. The function will return when the HW has finished
 * encoding next slice. Upon return the slicesReady pointer will contain
 * the number of slices that are ready and available in the HW output buffer.
 */
    i32 EWLWaitHwRdy(const void *inst, u32 *slicesReady);

/* SW/SW shared memory handling */
    void *EWLmalloc(u32 n);
    void *EWLcalloc(u32 n, u32 s);
    void EWLfree(void *p);
    void *EWLmemcpy(void *d, const void *s, u32 n);
    void *EWLmemset(void *d, i32 c, u32 n);
    int EWLmemcmp(const void *s1, const void *s2, u32 n);

/* Get the base address of on-chip sram used for input MB line buffer. */
i32 EWLGetInputLineBufferBase(const void *instance, EWLLinearMem_t * info);

#ifdef __cplusplus
}
#endif
#endif /*__EWL_H__*/
