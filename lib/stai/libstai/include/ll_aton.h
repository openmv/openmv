/**
 ******************************************************************************
 * @file    ll_aton.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ATON LL module.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#ifndef __LL_ATON_H
#define __LL_ATON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "ll_aton_attributes.h"
#include "ll_aton_config.h"

#if (LL_ATON_PLATFORM != LL_ATON_PLAT_EC_TRACE)
#include "ll_aton_osal.h"
#include "ll_aton_platform.h"
#endif // LL_ATON_PLATFORM != LL_ATON_PLAT_EC_TRACE

/** @defgroup ATON_LL ATON_LL_Driver
 * @{
 */

/* LL ATON error codes */
#define LL_ATON_OK            (0)
#define LL_ATON_INVALID_ID    (-1)
#define LL_ATON_INVALID_PARAM (-2)
#define LL_ATON_TIMEOUT       (-3)

  /* this is needed to avoid some compilers (e.g. KEIL) that observe a strict semantic about conversion of
   * pointers to integers in const initializers
   */
  typedef union
  {
    unsigned char *p;
    uintptr_t i;
  } ll_aton_pointer;

  /* Method that translates an address from physical to virtual */
  unsigned char *LL_Address_Physical2Virtual(unsigned char *address);

  /* Method that translates an address from virtual to physical */
  unsigned char *LL_Address_Virtual2Physical(unsigned char *address);

/**
 * @brief ATON User Configuration macros
 */
/* Set beyond macro to 1 if you want to enable the generation of ATON event interrupts */
#ifndef LL_ATON_EN_EVENT_IRQ
#define LL_ATON_EN_EVENT_IRQ 1
#endif

/* Set beyond macro to 1 if you want to enable the generation of ATON configuration error interrupts */
#ifndef LL_ATON_EN_ERROR_IRQ
#define LL_ATON_EN_ERROR_IRQ 1
#endif

  /** @defgroup ATON_INIT ATON Global initialization/deinitialization functions
   * @{
   */
  int LL_ATON_Init(void);
  int LL_ATON_DeInit(void);
  /**
   * @}
   */

  /**
   * @brief List of acceleration units types
   */
  enum AccelUnitsType
  {
    STRENG = 0,
    STRENG64,
    CONVACC,
    DECUN,
    ACTIV,
    ARITH,
    POOL,
    IMC,
    RECBUF,
  };

  typedef struct
  {
    enum AccelUnitsType unit_type;
    unsigned short unit_num;
  } AccelUnits;

  /**
   * @brief  Converts an Aton unit cardinal group id into a global one
   * @param  type enum specifying the unit group
   * @param  id Cardinal id of the unit in the group
   * @retval ATON Unit index
   * @todo   Add boundary checks
   */
  static inline AccelUnits LL_ATON_GetUnit_From_Cardinal_ID(enum AccelUnitsType type, int id)
  {
    return (AccelUnits){type, (unsigned short)id};
  }

  /**
   * @brief Activation Unit function types
   */
  typedef enum
  {
    ACTIV_RELU = 1,
    ACTIV_PRELU,
    ACTIV_TRELU,
    ACTIV_FUNC,
    ACTIV_LUT
  } LL_Activacc_Op;

  /**
   * @brief Activation unit Accelerator configuration structure
   */
  typedef struct
  {
    unsigned rounding_f : 1;     /**< Input feature data rounding control: 1=enable,0=disable */
    unsigned saturation_f : 1;   /**< Input feature data saturation control: 1=enable,0=disable */
    unsigned round_mode_f : 2;   /**< Input feature round mode */
    unsigned inbytes_f : 2;      /**< Input data width in bytes. Valid values are 1, 2 or 3 bytes */
    unsigned outbytes_f : 2;     /**< Input feature output bytes after shift. Valid values are 1 or 2 bytes */
    unsigned rounding_o : 1;     /**< Output rounding control, 1=enable,0=disable */
    unsigned saturation_o : 1;   /**< Output saturation control,1=enable,0=disable */
    unsigned round_mode_o : 1;   /**< Output rounding mode */
    unsigned relu_mode_o : 1;    /**< Apply Relu operation before rounding */
    unsigned outbytes_o : 2;     /**< Number of output bytes: 1, 2 or 3 */
    unsigned signedop : 1;       /**< Signed/unsigned activations: 0: unsigned activations, 1 signed */
    unsigned char shift_f;       /**< Input feature data shift. Negative values represent left shifts */
    unsigned char shift_o;       /**< Optional right shift to be applied to the function evaluator final result */
    unsigned parameter;          /**< ReLU parameter */
    unsigned parameter_2;        /**< Zero offset for TRELU operation for use in scale/offset integer arithmetic.
                                  *   Needs zp alignment */
    unsigned nbytes;             /**< Number of bytes of input data  */
    ll_aton_pointer ROM0_vector; /**< Address of ROM0 coefficients table */
    ll_aton_pointer ROM1_vector; /**< Address of ROM1 coefficients table */
    ll_aton_pointer LUT_vector;  /**< Address of LUT coefficients table */
    unsigned ROM0_nbytes;        /**< Length of ROM0 table */
    unsigned ROM1_nbytes;        /**< Length of ROM1 table */
    unsigned char shift_b;       /**< Optional left shift to be applied to coefficient B */
    unsigned char shift_c;       /**< Optional left shift to be applied to coefficient C */
    unsigned char shift_norm;    /**< Function input range normalization left shift parameter */
    unsigned char bwidth;        /**< Number of MSB bits of the input activation to be used to address ROM0.
                                  *   This field configures the number of outer segments (max outer segments = 32).
                                  *   Valid values range = 0,1,2,3,4 and 5 corresponding to 1,2,4,8,16 and 32 outer segment(s)
                                  *   respectively */
    int fsub;                    /**< Feature data subtract value */
    LL_Activacc_Op operation;    /**< Activation type. See LL_Activacc_Op */
  } LL_Activacc_InitTypeDef;

  /** @defgroup LL_ACTIV Activation unit configuration functions
   * @{
   */
  int LL_Activacc_Init(int id, const LL_Activacc_InitTypeDef *Activacc_InitStruct);
  /**
   * @}
   */

  /**
   * @brief Arithmetic unit operations
   */
  typedef enum
  {
    ARITH_AFFINE = 1,
    ARITH_MIN,
    ARITH_MAX,
    ARITH_MUL,
    ARITH_X_AND_Y,
    ARITH_X_OR_Y,
    ARITH_NOT_X,
    ARITH_X_XOR_Y,
    ARITH_X_EQ_Y,
    ARITH_X_LT_Y,
    ARITH_X_LE_Y,
    ARITH_X_GT_Y,
    ARITH_X_GE_Y,
    ARITH_ABS_X,
    ARITH_SIGN_X,
    ARITH_CLIP
  } LL_Arithacc_Op;

  /**
   * @brief Arithmetic constant broadcast modes
   */
  typedef enum
  {
    ARITH_BCAST_NONE,
    ARITH_BCAST_CHAN,
    ARITH_BCAST_HEIGHT,
    ARITH_BCAST_WIDTH,
    ARITH_BCAST_HEIGHT_WIDTH,
    ARITH_BCAST_SCALAR,
  } LL_Arithacc_Bcast;

  /**
   *	@brief Arithmetic unit Accelerator configuration structure
   */
  typedef struct
  {
    unsigned rounding_x : 1;   /**< Input feature data rounding for stream X */
    unsigned saturation_x : 1; /**< Input feature data saturation for stream X */
    unsigned round_mode_x : 2; /**< Input feature data rounding mode for stream X */
    unsigned inbytes_x : 2;    /**< Input data width in bytes for stream X. Valid values are 1, 2 or 3 bytes */
    unsigned outbytes_x : 2;   /**< Number of output bytes to use for input feature data of stream X after rounding or
                                *  saturation. Valid values are 1 or 2 bytes */
    signed char shift_x;       /**< Input feature data shift for stream X. Use negative values for left shift */
    unsigned rounding_y : 1;   /**< Input feature data rounding for stream Y */
    unsigned saturation_y : 1; /**< Input feature data saturation for stream Y */
    unsigned round_mode_y : 2; /**< Input feature data rounding mode for stream Y */
    unsigned inbytes_y : 2;    /**< Input data width in bytes for stream Y. Valid values are 1, 2 or 3 bytes */
    unsigned outbytes_y : 2;   /**< Number of output bytes to use for input feature data of stream Y after rounding or
                                *  saturation. Valid values are 1 or 2 bytes */
    unsigned combinebc : 1;    /**< Combine coeff B and C to form a 32b coeff BC = {B[15:0],C[15:0]} */
    unsigned clipout : 1;      /**< Controls output clipping to range specified by clip range configuration, 1=enable,
                                *  0=disable */
    signed char shift_y;       /**< Input feature data shift for stream Y. Use negative values for left shift */
    unsigned rounding_o : 1;   /**< Rounding control, 1=enable, 0=disable */
    unsigned saturation_o : 1; /**< Saturation control, 1=enable, 0=disable */
    unsigned round_mode_o : 1; /**< Otput rounding mode control */
    unsigned relu_mode_o : 1;  /**< Apply Relu operation before rounding */
    unsigned outbytes_o : 2;   /**< Number of output bytes: 1 or 2 */
    unsigned char shift_o;     /**< Optional right shift to apply to final result of operation */
    unsigned scalar : 1;       /**< Set Scalar/Vector mode */
    unsigned dualinput : 1;    /**< Dual input control, 1=both X,Y streams valid, 0=only X stream is valid */
    LL_Arithacc_Op operation;  /**< Arithmetic operation to be applied. See LL_Arithacc_Op */
    LL_Arithacc_Bcast bcast;   /**< Set constant broadcast modes. See LL_Arithacc_Bcast */
    unsigned char Ax_shift;    /**< Optional right shift to result of Ax */
    unsigned char By_shift;    /**< Optional right shift to result of By */
    unsigned char C_shift;     /**< Optional left shift to apply to C */
    unsigned fWidth;           /**< Feature width */
    unsigned fHeight;          /**< Feature height */
    unsigned short fChannels;  /**< Number of feature channels */
    unsigned short batchDepth; /**< Batch depth */
    short clipmin;             /**< Signed 16b value specifying output clip min */
    short clipmax;             /**< Signed 16b value specifying output clip max */
    short A_scalar;            /**< Scalar coefficient A */
    short B_scalar;            /**< Scalar coefficient B */
    short C_scalar;            /**< Scalar coefficient C */
    ll_aton_pointer A_vector;  /**< Address of A vector table */
    ll_aton_pointer B_vector;  /**< Address of B vector table */
    ll_aton_pointer C_vector;  /**< Address of C vector table */
    unsigned char vec_precision[3]; /**< Number of bits for A, B and C vectors */
  } LL_Arithacc_InitTypeDef;

  /** @defgroup LL_ARITH Arithmetic Unit configuration functions
   * @{
   */
  int LL_Arithacc_Init(int id, const LL_Arithacc_InitTypeDef *Arithacc_InitStruct);
  /**
   * @}
   */

  typedef enum
  {
    AFILT_MODE_NONE = 0,
    AFILT_MODE_PIXELDROP = 1,
    AFILT_MODE_FRAMEDROP = 2,
    AFILT_MODE_FRAMEZERO = 3
  } LL_Convacc_Afilt_Mode;

  /**
   *	@brief Convolutional Accelerator configuration structure
   */
  typedef struct
  {
    unsigned rounding_f : 1;           /**< Input feature data rounding */
    unsigned saturation_f : 1;         /**< Input feature data saturation */
    unsigned round_mode_f : 2;         /**< Output data rounding mode */
    unsigned inbytes_f : 2;            /**< Input data width in bytes */
    unsigned rounding_o : 1;           /**< Output data rounding after right shift */
    unsigned saturation_o : 1;         /**< Output saturation */
    unsigned round_mode_o : 1;         /**< Output data rounding mode */
    unsigned relu_mode_o : 1;          /**< Apply Relu operation before rounding */
    unsigned outbytes_o : 2;           /**< Output data width in bytes */
    unsigned simd : 2;                 /**< Enable 8x8bit (1) or 16x8bit (2) SIMD mode */
    unsigned accumulate : 1;           /**< Sum and synchronize with stream link input #2 */
    unsigned accumulate_first : 1;     /**< No sum and synchronization with stream link input #2 for the first frame */
    unsigned accumulate_gen_first : 1; /**< Generate first accumulator input frame internally */
    unsigned fstat : 1;                /**< Feature data stationary */
    unsigned raw_o : 1;                /**< Use RAW file output format */
    unsigned kt1_mode : 1;             /**< Load kernel from T1 buffer */
    unsigned deepmode : 1;             /**< Enable Deep1x1 optimized mode */
    unsigned dss2mode : 1;             /**< Enable DSS2 (depth separable stride 2) optimized mode */
    unsigned f_unsigned : 1;           /**< Feature data unsigned */
    unsigned k_unsigned : 1;           /**< Kernel data unsigned */
    unsigned kseten : 2;               /**< Enable kernel set 0 (bit 0) or 1 (bit 1) if KT1 is 1,
                                        *   otherwise select byte 1 (0), byte 2 (1), byte 3 (2) or
                                        *   all bytes (Deep1x1 mode only) (3) of kernel stream in SIMD mode */
    unsigned char shift_f;             /**< Input feature data shift */
    unsigned char shift_a;             /**< Accumulator data input signed left shift */
    unsigned char shift_o;             /**< Result data output signed right shift */
    unsigned fWidth;                   /**< Feature data width */
    unsigned fHeight;                  /**< Feature data height */
    unsigned char kernelWidth;         /**< Kernel width */
    unsigned char kernelHeight;        /**< Kernel height */
    unsigned char nKernels;            /**< Total number of parallel kernels */
    unsigned short batchDepth;         /**< Batch Depth */
    unsigned char hstride;             /**< Horizontal stride */
    unsigned char vstride;             /**< Vertical stride */
    unsigned short left_padding;       /**< Number of vertical left dummy columns */
    unsigned short right_padding;      /**< Number of vertical right dummy columns */
    unsigned short top_padding;        /**< Number of horizontal top dummy lines */
    unsigned short bot_padding;        /**< Number of horizontal bottom dummy lines */
    unsigned short left_crop;          /**< Left feature data boundary */
    unsigned short right_crop;         /**< Right feature data boundary */
    unsigned short top_crop;           /**< Top feature data boundary */
    unsigned short bot_crop;           /**< Bottom feature data boundary */
    unsigned short fstatcnt;           /**< Number of frames before next reload of feature stationary frame */
    LL_Convacc_Afilt_Mode afilt_mode;  /**< Accumulator port filter mode. See LL_Convacc_Afilt_Mode */
    unsigned char afilt_tot;           /**< Total number of accumulation tensors */
    unsigned char afilt_first;         /**< First accumulation tensor */
    unsigned char afilt_last;          /**< Last accumulation tensor */
    unsigned char kfilt_tot;           /**< Total number of kernels */
    unsigned char kfilt_first;         /**< First kernel */
    unsigned char kfilt_last;          /**< Last kernel */
    int fsub;                          /**< Feature data subtract value */
    short zfbias;                      /**< Bias added to zero frames */
  } LL_Convacc_InitTypeDef;

  /** @defgroup LL_CONVACC Convolutional accelerator unit configuration functions
   * @{
   */
  int LL_Convacc_Init(int id, const LL_Convacc_InitTypeDef *Convacc_InitStruct);
  /**
   * @}
   */

  /**
   * @brief Pooling Acceleration supported operations
   */
  typedef enum
  {
    POOL_MAX = 1,
    POOL_MIN,
    POOL_AVG,
    POOL_GMAX,
    POOL_GMIN,
    POOL_GAVG
  } LL_Poolacc_Op;

  /**
   * @brief Pooling Accelerator configuration structure
   */
  typedef struct
  {
    LL_Poolacc_Op operation;   /**< Pooling operation type. See LL_Poolacc_Op */
    unsigned avgnopad : 1;     /**< Average pooling operation without padding */
    unsigned short inputX;     /**< Size X of the input feature */
    unsigned short inputY;     /**< Size Y of the input feature */
    unsigned short outputX;    /**< Size X of the output data */
    unsigned short outputY;    /**< Size Y of the output data */
    unsigned char poolWinX;    /**< Size X of pooling window */
    unsigned char poolWinY;    /**< Size Y of pooling window */
    unsigned char strideX;     /**< Stride value in X direction */
    unsigned char strideY;     /**< Stride value in Y direction */
    unsigned short topCrop;    /**< Top cropping size */
    unsigned short bottomCrop; /**< Bottom cropping size */
    unsigned short leftCrop;   /**< Left cropping size */
    unsigned short rightCrop;  /**< Right cropping size */
    unsigned short topPad;     /**< Top padding size */
    unsigned short bottomPad;  /**< Bottom padding size */
    unsigned short leftPad;    /**< Left padding size */
    unsigned short rightPad;   /**< Right padding size */
    unsigned short batchSize;  /**< Batch size */
    unsigned char shift_f;     /**< Input feature data shift */
    unsigned char shift_o;     /**< Optional right shift to apply the average pooling output */
    unsigned dualLine : 1;     /**< Enable dual line, allows each linebuffer line to work as 2 lines,
                                *   applicable for 8-bit data */
    unsigned nbytes : 2;       /**< input data number of bytes */
    unsigned rounding_f : 1;   /**< Input feature data rounding */
    unsigned saturation_f : 1; /**< Input feature data saturation */
    unsigned round_mode_f : 2; /**< Rounding mode to apply to input feature data */
    unsigned inbytes_f : 2;    /**< Input data width in bytes. Valid values are 1, 2 or 3 bytes */
    unsigned outbytes_f : 2;   /**< Number of output bytes to use for final result after rounding or saturation.
                                *   Valid values are 1 or 2 bytes */
    unsigned rounding_o : 1;   /**< Enable output rounding using round-to-nearest (round up)
                                *   (applicable to average pooling operations) */
    unsigned saturation_o : 1; /**< Enable output saturation (applicable to average pooling operations) */
    unsigned round_mode_o : 1; /**< Rounding mode to apply to output feature data */
    unsigned relu_mode_o : 1;  /**< Apply Relu operation before rounding */
    unsigned outbytes_o : 2;   /**< Number of output bytes to use for final result after rounding or saturation.
                                *   Valid values are 1 or 2 bytes */
    short mulval;              /**< constant to be multiplied to accumulated sum of pooling window.
                                * For average operation, it represents the reciprocal of the divisor in 16-bit fixed point.
                                * The average is computed by multiplying this constant with the accumulated sum and then applying the
                                * relevant right shift at the output. (Applicable to average pooling operations) */
    unsigned pad_val_en : 1;   /**< Enable padding value */
    short pad_val;             /**< Padding value to be used for padding operation */
  } LL_Poolacc_InitTypeDef;

  /**
   * @brief Epoch Controller configuration structure
   */
  typedef struct
  {
    uint32_t blobaddr;     /**< Blob code start address. Must be 8 byte aligned */
    unsigned stepmode : 1; /**< Enable step mode. Used for debugging purposes */
  } LL_EpochCtrl_InitTypeDef;

  /** @defgroup LL_POOL Pooling unit configuration functions
   * @{
   */
  int LL_Poolacc_Init(int id, const LL_Poolacc_InitTypeDef *conf);
  /**
   * @}
   */

  /**
   * @brief Streaming engine configuration structure
   */
  typedef struct
  {
    unsigned dir : 1;             /**< Stream Direction: 0 input, 1 output */
    unsigned raw : 1;             /**< Set RAW mode (1) or raster mode (0) */
    unsigned raw_out : 1;         /**< Force RAW output (bus to stream only)
                                   *   even if the engine is programmed in raster mode */
    unsigned continuous : 1;      /**< Do not restart address pointer at end of frame */
    unsigned noblk : 1;           /**< Do not use blocks wider that the native bus size */
    unsigned noinc : 1;           /**< Do not increment address */
    unsigned align_right : 1;     /**< Alignment for data on switch (default left) */
    unsigned mem_lsb : 1;         /**< For when nbits_in != nbits_out to decide which bits are read/written
                                   *   (default msb) */
    unsigned sync_with_other : 1; /**< Enable synchronizations signals between engines */
    unsigned nbits_unsigned : 1;  /**< Disable sign extension */
    unsigned bus_cid : 3;         /**< Set Compartment ID cache attribute */
    unsigned cacheable : 1;       /**< Set cacheable bus attribute */
    unsigned cache_allocate : 1;  /**< Set cache allocate bus attribute */
    unsigned bus_pfetch : 1;      /**< Enable bus prefetch */
    unsigned cache_linesize : 2;  /**< Cache Line size: 0 -> 64B, 1 -> 128B, 2 -> 256B, 3 -> 512B */
    unsigned cipher_en : 1;       /**< Enable ciphering: 0 -> disable, 1-> enable  */
    unsigned key_sel : 1;         /**< Bus Interface key to be used for ciphering (0, 1) */
    unsigned char sync_dma;       /**< Synchronization signals source engine */
    ll_aton_pointer addr_base;    /**< Source/Destination base address */
    unsigned offset_start;        /**< Offset of the Source/Destination start address from the base address */
    unsigned offset_end;          /**< Offset of the Source/Destination end address from the base address */
    unsigned offset_limit;        /**< Offset of the Stream engine address limit from the base address.
                                   *   Used to prevent prefetch beyond memory boundaries */
    unsigned frame_count;         /**< Number of frames to transfer */
    unsigned fwidth;              /**< Frame width (pixel per line) */
    unsigned fheight;             /**< Frame height (number of lines) */
    unsigned batch_depth;         /**< Batch depth (subpix per pixel) */
    unsigned batch_offset;        /**< Offset (bytes) between batches */
    unsigned frame_offset;        /**< Offset between multiple frames within frame repetition loop */
    unsigned line_offset;         /**< Offset between multiple frames within frame repetition loop.
                                   *   If set to zero it's derived from width and batch_offset */
    unsigned loop_offset;         /**< Offset between frame repetition loops */
    unsigned frame_loop_cnt;      /**< Number of frames to loop */
    unsigned frame_tot_cnt;       /**< Frame limit */
    unsigned char nbits_in;       /**< Data size in bits if reading */
    unsigned char nbits_out;      /**< Data size in bits if writing */
  } LL_Streng_TensorInitTypeDef;

  static inline unsigned char *LL_Streng_addr_start(const LL_Streng_TensorInitTypeDef *conf)
  {
    return conf->addr_base.p + conf->offset_start;
  }

  static inline unsigned char *LL_Streng_addr_end(const LL_Streng_TensorInitTypeDef *conf)
  {
    return conf->addr_base.p + conf->offset_end;
  }

  static inline unsigned char *LL_Streng_addr_limit(const LL_Streng_TensorInitTypeDef *conf)
  {
    return conf->addr_base.p + conf->offset_limit;
  }

  static inline uint32_t LL_Streng_len(const LL_Streng_TensorInitTypeDef *conf)
  {
    return conf->offset_end - conf->offset_start;
  }

  /**
   * @brief Streaming engine External Sync configuration structure
   */
  typedef struct
  {
    unsigned int enable : 1;      /**< Enable/disable external sync feature (0, 1) */
    unsigned int trig_source : 4; /**< Trigger source signal ID [0..3] */
    unsigned int lines;           /**< Number of lines associated to each trigger rising edge */
    unsigned int lines_offset;    /**< Number of lines after which the special offset will be applied */
    unsigned int offset;          /**< Special line offset */
  } LL_Streng_ExtSyncTypedef;

  /** @defgroup LL_STRENG Streaming Engine configuration and operation functions
   * @{
   */
  int LL_Streng_TensorInit(int id, const LL_Streng_TensorInitTypeDef *, int n);
  int LL_Streng_ExtSyncInit(int id, LL_Streng_ExtSyncTypedef *);
  int LL_Streng_Wait(uint32_t mask);
  /**
   * @}
   */

  /** @defgroup LL_BUSIF Bus Interface configuration functions
   * @{
   */
  int LL_Busif_SetKeys(int id, int key, uint64_t key_low, uint64_t key_hi);
  /**
   * @}
   */

  enum SwitchUnitsType
  {
    STRSWITCH = 0,
    STRSWITCH64,
    STRSWITCH_VC,
  };

  extern unsigned __atonn_getSrcPortID(enum SwitchUnitsType sut, unsigned char su_num, enum AccelUnitsType aut,
                                       unsigned char au_num, unsigned char port);
  extern unsigned __atonn_getDstPortID(enum SwitchUnitsType sut, unsigned char su_num, enum AccelUnitsType aut,
                                       unsigned char au_num, unsigned char port);

#if (LL_ATON_PLATFORM != LL_ATON_PLAT_EC_TRACE)

  typedef unsigned int SourcePort;
#define ATONN_SRCPORT(S, J, U, I, P) ATON_##S##_##J##_LINK_##U##_##I##_##P
// Convert SourcePort in ID needed to configure HW
#define ATONN_SRCPORT_ID(S) (S)

  typedef unsigned int DestPort;
#define ATONN_DSTPORT(S, J, U, I, P) ATON_##S##_DST_OFFSET(J, ATON_##S##_##J##_DST##U##_##I##_##P##_IDX)

// Convert DestPort in ID needed to configure HW
#define ATONN_DSTPORT_ID(D) (D)

#else

typedef struct
{
  enum SwitchUnitsType s;
  unsigned char s_num;
  enum AccelUnitsType u;
  unsigned char u_num;
  unsigned char port;
} SourcePort;
#define ATONN_SRCPORT(S, J, U, I, P)                                                                                   \
  {                                                                                                                    \
    .s = S, .s_num = J, .u = U, .u_num = I, .port = P                                                                  \
  }
static inline unsigned _atonn_getSrcPortID(SourcePort s)
{
  return __atonn_getSrcPortID(s.s, s.s_num, s.u, s.u_num, s.port);
}
#define ATONN_SRCPORT_ID(S) _atonn_getSrcPortID(S)

typedef struct
{
  enum SwitchUnitsType s;
  unsigned char s_num;
  enum AccelUnitsType u;
  unsigned char u_num;
  unsigned char port;
} DestPort;
#define ATONN_DSTPORT(S, J, U, I, P)                                                                                   \
  {                                                                                                                    \
    .s = S, .s_num = J, .u = U, .u_num = I, .port = P                                                                  \
  }
static inline unsigned _atonn_getDstPortID(DestPort d)
{
  return __atonn_getDstPortID(d.s, d.s_num, d.u, d.u_num, d.port);
}
#define ATONN_DSTPORT_ID(D) _atonn_getDstPortID(D)

#endif

/**
 * @brief Stream Switch source ports identifiers
 */
#define STRENG_SRC(I, P)  ATONN_SRCPORT(STRSWITCH, 0, STRENG, I, P)
#define CONVACC_SRC(I, P) ATONN_SRCPORT(STRSWITCH, 0, CONVACC, I, P)
#define DECUN_SRC(I, P)   ATONN_SRCPORT(STRSWITCH, 0, DECUN, I, P)
#define ACTIV_SRC(I, P)   ATONN_SRCPORT(STRSWITCH, 0, ACTIV, I, P)
#define ARITH_SRC(I, P)   ATONN_SRCPORT(STRSWITCH, 0, ARITH, I, P)
#define POOL_SRC(I, P)    ATONN_SRCPORT(STRSWITCH, 0, POOL, I, P)
#define RECBUF_SRC(I, P)  ATONN_SRCPORT(STRSWITCH, 0, RECBUF, I, P)

/**
 * @brief Stream Switch destination ports identifiers
 */
#define STRENG_DST(I, P)  ATONN_DSTPORT(STRSWITCH, 0, STRENG, I, P)
#define CONVACC_DST(I, P) ATONN_DSTPORT(STRSWITCH, 0, CONVACC, I, P)
#define DECUN_DST(I, P)   ATONN_DSTPORT(STRSWITCH, 0, DECUN, I, P)
#define ACTIV_DST(I, P)   ATONN_DSTPORT(STRSWITCH, 0, ACTIV, I, P)
#define ARITH_DST(I, P)   ATONN_DSTPORT(STRSWITCH, 0, ARITH, I, P)
#define POOL_DST(I, P)    ATONN_DSTPORT(STRSWITCH, 0, POOL, I, P)
#define RECBUF_DST(I, P)  ATONN_DSTPORT(STRSWITCH, 0, RECBUF, I, P)

/**
 * @brief Streaming switch configuration structure
 */
#define ATON_SWITCH_CONTEXT_NUM 2

#if ATON_SWITCH_CONTEXT_NUM == 2
#define LL_Switch_Init_Dest()     .dest
#define LL_Switch_Init_Source(x)  .source##x
#define LL_Switch_Init_Context(x) .context##x
#define LL_Switch_Init_Frames(x)  .frames##x
  typedef struct
  {
    SourcePort source0; /**< Must be one of SourcePort */
    SourcePort source1; /**< Must be one of SourcePort */
    DestPort dest;      /**< Must be one of DestPort */
    unsigned char frames0;
    unsigned char frames1;
    unsigned context0 : 1;
    unsigned context1 : 1;
  } LL_Switch_InitTypeDef;
#else
#define LL_Switch_Init_Dest()     .dest
#define LL_Switch_Init_Source(x)  .source[x]
#define LL_Switch_Init_Context(x) .context[x]
#define LL_Switch_Init_Frames(x)  .frames[x]
typedef struct
{
  SourcePort source[ATON_SWITCH_CONTEXT_NUM]; /**< Must be one of SourcePort */
  DestPort dest;                              /**< Must be one of DestPort */
  unsigned char context[ATON_SWITCH_CONTEXT_NUM];
  unsigned char frames[ATON_SWITCH_CONTEXT_NUM];
} LL_Switch_InitTypeDef;
#endif

  typedef LL_Switch_InitTypeDef LL_Switch_DeinitTypeDef;

  /**
   * @brief Streaming switch with virtual channels configuration structure
   */

#define LL_SwitchVC_Init_Dest()   .dest
#define LL_SwitchVC_Init_Source() .source
  typedef struct
  {
    SourcePort source; /**< Must be one of SourcePort */
    DestPort dest;     /**< Must be one of DestPort */
  } LL_SwitchVC_InitTypeDef;

  typedef LL_SwitchVC_InitTypeDef LL_SwitchVC_DeinitTypeDef;

  /** @defgroup STRSWTCH_VC Streaming Switch with virtual channels connection/disconnection functions
   * @{
   */
  int LL_SwitchVC_Init_NoReset(const LL_SwitchVC_InitTypeDef *LL_SwitchVC_InitStruct, int n);
  int LL_SwitchVC_Init(const LL_SwitchVC_InitTypeDef *LL_SwitchVC_InitStruct, int n);
  int LL_SwitchVC_Deinit(const LL_SwitchVC_DeinitTypeDef *LL_SwitchVC_DenitStruct, int n);
  int LL_SwitchVC_Deinit_Fine_Grained(const LL_SwitchVC_DeinitTypeDef *LL_SwitchVC_DenitStruct, int n);
  /**
   * @}
   */

  /** @defgroup STRSWTCH Streaming Switch connection/disconnection functions
   * @{
   */
  int LL_Switch_Init_NoReset(const LL_Switch_InitTypeDef *LL_Switch_InitStruct, int n);
  int LL_Switch_Init(const LL_Switch_InitTypeDef *LL_Switch_InitStruct, int n);
  int LL_Switch_Deinit(const LL_Switch_DeinitTypeDef *LL_Switch_DenitStruct, int n);
  int LL_Switch_Deinit_Fine_Grained(const LL_Switch_DeinitTypeDef *LL_Switch_DenitStruct, int n);
  /**
   * @}
   */

  /**
   * @brief Decompression Unint configuration structure
   */
  typedef struct
  {
    unsigned short nCVperCB;    /**< Number of CodeVectors per CodeBook */
    unsigned char nCWperCV;     /**< Number of CodeWords per CodeVector */
    unsigned char nRCWlastCV;   /**< Number of read CodeWords from the last CodeVector */
    unsigned char nFormatBytes; /**< Number of bytes of a CodeWord */
    unsigned short nBatches;    /**< Number of consecutive Batches used with a CodeBook */
    unsigned noDualInput : 1;   /**< Disable the CodeBook stream link */
    unsigned noOverWrite : 1;   /**< Disable CodeBooks overwriting */
    ll_aton_pointer CBs_vector; /**< Pointer to CodeBooks storage */
    unsigned CBs_size;          /**< Size of CodeBooks in Memory */
  } LL_Decun_InitTypeDef;

  /** @defgroup LL_DECUN Decompression Unit configuration functions
   * @{
   */
  int LL_Decun_Init(int id, const LL_Decun_InitTypeDef *LL_Decun_InitStruct);
  /**
   * @}
   */

  /** @defgroup LL_EPOCHCTRL Epoch controller functions
   * @{
   */
  int LL_EpochCtrl_Init(int id, const LL_EpochCtrl_InitTypeDef *conf);
  int LL_EpochCtrl_Step(int id);
  int LL_EpochCtrl_Wait(uint32_t mask);
  unsigned int LL_EpochCtrl_GetBlobSize(uint32_t *eb_addr);
  /**
   * @}
   */

  /**
   * @brief Structure defining a unit to be activated
   */
  typedef struct
  {
    AccelUnits unit; /**< Must be one of AccelUnits */
    // unsigned int flags;   // To be implemented e.g. clear, etc.
  } LL_ATON_EnableUnits_InitTypeDef;

  typedef LL_ATON_EnableUnits_InitTypeDef LL_ATON_DisableUnits_InitTypeDef;

  /** @addtogroup ATON_LL_UNITS ATON Units enabling/disabling functions
   * @{
   */
  int LL_ATON_EnableUnits_Init(const LL_ATON_EnableUnits_InitTypeDef *LL_ATON_EnableUnits_InitStruct, int n);
  int LL_ATON_DisableUnits_Init(const LL_ATON_DisableUnits_InitTypeDef *LL_ATON_DisableUnits_InitStruct, int n);
  /**
   * @}
   */

  /** @addtogroup ATON Clock Gating functions
   * @{
   */
  void LL_ATON_EnableClock(unsigned int clock);
  void LL_ATON_DisableClock(unsigned int clock);
  /**
   * @}
   */

  /** @defgroup Helper functions (use just for debug/testing purposes)
   * @{
   */

  /**
   * @brief  DMA version of a memcpy functionality, this function could be overloaded if a system DMA could be used
   * @param  dst destination memory address
   * @param  src source memory address
   * @param  src_limit memory pool end address of `src`
   * @param  n number of bytes to be transferred
   * @param dst_cached Destination under cache flag
   * @param dst_cached Source under cache flag
   * @retval Error code E.g.: Invalid ID, invalid parameters, not idle,..
   *
   * @note:  This function completely undermines any possibility for integrating correctly
   *         SW operators (or any other functionality which calls this function) in any of the three ATON runtime
   * scheduling modes. In other words, function `LL_ATON_Dma_memcpy()` and its usage are incompatible with the ATON
   * runtime. Therefore either `memcpy()` should be used in its place or calls to `LL_ATON_Dma_memcpy()` need to be
   * transformed in a sequence of "epoch blocks" which can be integrated with the ATON runtime (as an example see the
   * ATON-accelerated implementation of operator `Concat`)!
   */
  void *LL_ATON_Dma_memcpy(void *dst, void *src, void *src_limit, size_t n, int dst_cached, int src_cached);

  /**
   * @}
   */

  /** @defgroup Watchdog management functions. Used for polling mode only
   * @{
   */
  int startWatchdog(uint32_t timeout);
  int checkWatchdog(void);

  /**
   * @}
   */

  /** @defgroup External Trigger functions. Used to trigger external units (e.g. HSP) using an ATON interrupt lines
   * @{
   */
  int LL_TriggerHigh(int irq);
  int LL_TriggerLow(int irq);
  /**
   * @}
   */

#ifdef __cplusplus
}
#endif

#endif

/**
 * @}
 */
