/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     Driver_PDM.h
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     15-Jan-2023
 * @brief    CMSIS-Driver for PDM.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef DRIVER_PDM_H_
#define DRIVER_PDM_H_

#include "Driver_Common.h"

#ifdef _cplusplus
extern "c"
{
#endif

#define ARM_PDM_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

#define ARM_PDM_MODE                                        0x00UL

/* Control code for PDM */
#define ARM_PDM_MODE_MICROPHONE_SLEEP                       0x00UL
#define ARM_PDM_MODE_AUDIOFREQ_8K_DECM_64                   0x01UL
#define ARM_PDM_MODE_AUDIOFREQ_16K_DECM_32                  0x02UL
#define ARM_PDM_MODE_AUDIOFREQ_16K_DECM_48                  0x03UL
#define ARM_PDM_MODE_AUDIOFREQ_16K_DECM_64                  0x04UL
#define ARM_PDM_MODE_AUDIOFREQ_32K_DECM_48                  0x05UL
#define ARM_PDM_MODE_AUDIOFREQ_48K_DECM_50                  0x06UL
#define ARM_PDM_MODE_AUDIOFREQ_48K_DECM_64                  0x07UL
#define ARM_PDM_MODE_AUDIOFREQ_96K_DECM_50                  0x08UL
#define ARM_PDM_MODE_AUDIOFREQ_192K_DECM_25                 0x09UL

/* Control code for PDM. Old definitions, to be deprecated in future releases */
#define ARM_PDM_MODE_STANDARD_VOICE_512_CLK_FRQ             ARM_PDM_MODE_AUDIOFREQ_8K_DECM_64
#define ARM_PDM_MODE_HIGH_QUALITY_512_CLK_FRQ               ARM_PDM_MODE_AUDIOFREQ_16K_DECM_32
#define ARM_PDM_MODE_HIGH_QUALITY_768_CLK_FRQ               ARM_PDM_MODE_AUDIOFREQ_16K_DECM_48
#define ARM_PDM_MODE_HIGH_QUALITY_1024_CLK_FRQ              ARM_PDM_MODE_AUDIOFREQ_16K_DECM_64
#define ARM_PDM_MODE_WIDE_BANDWIDTH_AUDIO_1536_CLK_FRQ      ARM_PDM_MODE_AUDIOFREQ_32K_DECM_48
#define ARM_PDM_MODE_FULL_BANDWIDTH_AUDIO_2400_CLK_FRQ      ARM_PDM_MODE_AUDIOFREQ_48K_DECM_50
/* Typo in macro name, should be '3072' KHz instead of '3071' */
#define ARM_PDM_MODE_FULL_BANDWIDTH_AUDIO_3071_CLK_FRQ      ARM_PDM_MODE_AUDIOFREQ_48K_DECM_64
#define ARM_PDM_MODE_ULTRASOUND_4800_CLOCK_FRQ              ARM_PDM_MODE_AUDIOFREQ_96K_DECM_50
#define ARM_PDM_MODE_ULTRASOUND_96_SAMPLING_RATE            ARM_PDM_MODE_AUDIOFREQ_192K_DECM_25

#define ARM_PDM_BYPASS_IIR_FILTER                           0x0AUL
#define ARM_PDM_BYPASS_FIR_FILTER                           0x0BUL

#define ARM_PDM_PEAK_DETECTION_NODE                         0x0CUL
#define ARM_PDM_SAMPLE_ADVANCE                              0x0DUL
#define ARM_PDM_CHANNEL_PHASE                               0x0EUL
#define ARM_PDM_CHANNEL_GAIN                                0x0FUL
#define ARM_PDM_CHANNEL_PEAK_DETECT_TH                      0x10UL
#define ARM_PDM_CHANNEL_PEAK_DETECT_ITV                     0x11UL

/* PDM event */
#define ARM_PDM_EVENT_ERROR                                (1UL << 0)
#define ARM_PDM_EVENT_CAPTURE_COMPLETE                     (1UL << 1)
#define ARM_PDM_EVENT_AUDIO_DETECTION                      (1UL << 2)

#define ARM_PDM_SELECT_RESOLUTION                          (1UL << 3)

#define ARM_PDM_16BIT_RESOLUTION                           (1UL << 4)
#define ARM_PDM_32BIT_RESOLUTION                           (1UL << 5)

/* PDM channel FIR length */
#define PDM_MAX_FIR_COEFFICIENT                             18

#define ARM_PDM_SELECT_CHANNEL                              0x01UL

/* PDM channels */
#define ARM_PDM_AUDIO_CHANNEL_0                            (0x00)
#define ARM_PDM_AUDIO_CHANNEL_1                            (0x01)
#define ARM_PDM_AUDIO_CHANNEL_2                            (0x02)
#define ARM_PDM_AUDIO_CHANNEL_3                            (0x03)
#define ARM_PDM_AUDIO_CHANNEL_4                            (0x04)
#define ARM_PDM_AUDIO_CHANNEL_5                            (0x05)
#define ARM_PDM_AUDIO_CHANNEL_6                            (0x06)
#define ARM_PDM_AUDIO_CHANNEL_7                            (0x07)

/* PDM mask channels */
#define ARM_PDM_MASK_CHANNEL_0                             (1 << ARM_PDM_AUDIO_CHANNEL_0)
#define ARM_PDM_MASK_CHANNEL_1                             (1 << ARM_PDM_AUDIO_CHANNEL_1)
#define ARM_PDM_MASK_CHANNEL_2                             (1 << ARM_PDM_AUDIO_CHANNEL_2)
#define ARM_PDM_MASK_CHANNEL_3                             (1 << ARM_PDM_AUDIO_CHANNEL_3)
#define ARM_PDM_MASK_CHANNEL_4                             (1 << ARM_PDM_AUDIO_CHANNEL_4)
#define ARM_PDM_MASK_CHANNEL_5                             (1 << ARM_PDM_AUDIO_CHANNEL_5)
#define ARM_PDM_MASK_CHANNEL_6                             (1 << ARM_PDM_AUDIO_CHANNEL_6)
#define ARM_PDM_MASK_CHANNEL_7                             (1 << ARM_PDM_AUDIO_CHANNEL_7)

typedef void (*ARM_PDM_SignalEvent_t) (uint32_t event);  /*Pointer to \ref PDM_SignalEvent : Signal PDM Event*/

/**
 @brief: These channel configurations are specific to each channels
 */
typedef struct _PDM_CH_CONFIG {
    uint8_t ch_num;                 /* Channel number */
    uint32_t ch_fir_coef[PDM_MAX_FIR_COEFFICIENT]; /* Channel FIR filter Coefficient */
    uint32_t ch_iir_coef;           /* Channel IIR Filter Coefficient */
}PDM_CH_CONFIG;

/**
 * @brief: PDM Status
 */
typedef struct _ARM_PDM_STATUS {
    uint32_t rx_busy          : 1;  /* Receiver busy flag             */
    uint32_t rx_overflow      : 1;  /* Receive data overflow detected */
    uint32_t reserved         : 30; /* Reserved (must be Zero)        */
}ARM_PDM_STATUS;

/**
 @brief : PDM Driver Capabilities
 */
typedef struct _ARM_PDM_CAPABILITIES{
    uint32_t mono_mode     :1;   /* supports Mono mode           */
    uint32_t synchronous   :1;   /* supports synchronous Receive */
    uint32_t reserved      :29;  /* Reserved (must be Zero)      */
}ARM_PDM_CAPABILITIES;

/**
 @brief  Access Structure of PDM Driver
*/
typedef struct ARM_DRIVER_PDM{
    ARM_DRIVER_VERSION            (*GetVersion)        (void);                                 /* pointer is pointing to PDM_GetVersion : used to get the driver version        */
    ARM_PDM_CAPABILITIES          (*GetCapabilities)   (void);                                 /* pointer is pointing to PDM_Capabilities : used to get the driver capabilities */
    int32_t                       (*Initialize)        (ARM_PDM_SignalEvent_t cb_event);       /* Pointer pointing to \ref PDM_intialize                                        */
    int32_t                       (*Uninitialize)      (void);                                 /* Pointer to PDM_Uninitialize : Un-initialize comparator Interface              */
    int32_t                       (*PowerControl)      (ARM_POWER_STATE state);                /* Pointer to PDM_PowerControl : Control Comparator Interface Power              */
    int32_t                       (*Control)           (uint32_t control, uint32_t arg1, uint32_t arg2);  /* Pointer to PDM_Control : Control Comparator Interface              */
    int32_t                       (*Config)            (PDM_CH_CONFIG *cnfg);                  /* Pointer to PDM Config: Channel configurations specific to each channel        */
    int32_t                       (*Receive)           (void *data, uint32_t num);             /* Pointer to Receive : PDM Receive Configuration                                */
    ARM_PDM_STATUS                (*GetStatus)         (void);                                 /* Pointer to GetStatus: Get the PDM status                                      */
}const ARM_DRIVER_PDM;

#endif /* DRIVER_PDM_H_ */
