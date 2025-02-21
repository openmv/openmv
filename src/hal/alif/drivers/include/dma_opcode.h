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
 * @file     dma_opcode.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     8-Aug-2023
 * @brief    DMA Opcode Generation Header File
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef DMA_OPCODE_H_
#define DMA_OPCODE_H_

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

#ifdef  __cplusplus
extern "C"
{
#endif

#define DMA_MAX_BACKWARD_JUMP   256                         /*!< Max Loop backward jump offset */
#define DMA_MAX_LP_CNT          256                         /*!< Max Loop count                */

/* 8 bit-opcode with variable data payload of 0, 8, 16 or 32bits */
#define DMA_OP_1BYTE_LEN         1
#define DMA_OP_2BYTE_LEN         2
#define DMA_OP_3BYTE_LEN         3
#define DMA_OP_6BYTE_LEN         6

#define OP_DMAADDH(ar)       (0x54 | (ar << 1))             /*!< Adds an immediate 16bit value to SARn or DARn                              */
#define OP_DMAADNH(ar)       (0x5C | (ar << 1))             /*!< Adds an immediate 16bit negative value to SARn or DARn                     */
#define OP_DMAEND            0x00                           /*!< End signal as the DMAC sequence is complete                                */
#define OP_DMAFLUSHP         0x35                           /*!< Flush the peripheral contents and sends message to resend its level status */
#define OP_DMAGO(ns)         (0xA0 | (ns << 1))             /*!< Execute thread in secure/non-secure mode                                   */
#define OP_DMAKILL           0x01                           /*!< Terminate Execution of a thread                                            */
#define OP_DMALD             0x04                           /*!< Performs DMA Load operation                                                */
#define OP_DMALDS            0x05                           /*!< Performs DMA Single Load operation                                         */
#define OP_DMALDB            0x07                           /*!< Performs DMA Burst Load operation                                          */
#define OP_DMALDP(bs)        (0x25 | (bs << 1))             /*!< Performs DMA Load & Notify Peripheral Single/Burst operation               */
#define OP_DMALP(lc)         (0x20 | (lc << 1))             /*!< Loop instruct DMAC to load 8bit val to LC0/LC1 reg                         */
#define OP_DMALPEND(nf, lc)  (0x28 | (nf << 4) | (lc << 2)) /*!< Loop End, nf=0, lc=1 if DMALPFE started loop                               */
#define OP_DMALPENDS(lc)     (0x39 | (lc << 2))             /*!< Loop End Single                                                            */
#define OP_DMALPENDB(lc)     (0x3B | (lc << 2))             /*!< Loop End Burst                                                             */
#define OP_DMAMOV            0xBC                           /*!< Move 32bit immediate into SAR/DAR/CCR                                      */
#define OP_DMANOP            0x18                           /*!< For code alignment                                                         */
#define OP_DMARMB            0x12                           /*!< Read Memory barrier, write-after-read sequence                             */
#define OP_DMASEV            0x34                           /*!< Send event                                                                 */
#define OP_DMAST             0x08                           /*!< Performs DMA Store operation                                               */
#define OP_DMASTS            0x09                           /*!< Performs DMA Single Store operation                                        */
#define OP_DMASTB            0x0B                           /*!< Performs DMA Burst Store operation                                         */
#define OP_DMASTP(bs)        (0x29 | (bs << 1))             /*!< Performs DMA Store & Notify Peripheral Single/Burst operation              */
#define OP_DMASTZ            0x0C                           /*!< Store Zeros                                                                */
#define OP_DMAWFE            0x36                           /*!< Wait for event                                                             */
#define OP_DMAWFP_P(p)       (0x30 | (p << 0))              /*!< Wait for peripheral with peripheral bit set                                */
#define OP_DMAWFP(bs)        (0x30 | (bs << 1))             /*!< Wait for peripheral in single/burst mode                                   */
#define OP_DMAWMB            0x13                           /*!< Write memory barrier                                                       */

/* SWAP SIZE */
typedef enum _DMA_SWAP {
    DMA_SWAP_NONE,          /*!< No swap, 8-bit data            */
    DMA_SWAP_16BIT,         /*!< Swap bytes within 16-bit data  */
    DMA_SWAP_32BIT,         /*!< Swap bytes within 32-bit data  */
    DMA_SWAP_64BIT,         /*!< Swap bytes within 64-bit data  */
    DMA_SWAP_128BIT,        /*!< Swap bytes within 128-bit data */
} DMA_SWAP;


/* Loop counters */
typedef enum _DMA_LC {
    DMA_LC_0,
    DMA_LC_1,
} DMA_LC;

/* Burst Type */
typedef enum _DMA_BURST {
    DMA_BURST_FIXED = 0,    /*!< Fixed Address burst            */
    DMA_BURST_INCREMENTING, /*!< Incrementing Address burst     */
} DMA_BURST;

/* Transfer type */
typedef enum _DMA_XFER {
    DMA_XFER_SINGLE     = 0,
    DMA_XFER_BURST      = 1,
    DMA_XFER_PERIPHERAL = 2,
    DMA_XFER_FORCE      = 2,
} DMA_XFER;

/* DMA Secure State */
typedef enum _DMA_SECURE_STATE {
    DMA_STATE_SECURE = 0,   /*!< Secure State                   */
    DMA_STATE_NON_SECURE,   /*!< Non-Secure State               */
} DMA_SECURE_STATE;

/* DMA registers */
typedef enum _DMA_REG {
    DMA_REG_SAR,
    DMA_REG_CCR,
    DMA_REG_DAR
} DMA_REG;

/* DMA channel control information */
typedef union _dma_ccr_t {
    uint32_t value;
    struct {
        uint32_t src_inc              : 1;     /*!< Source Fixed/Increment type burst                                */
        uint32_t src_burst_size       : 3;     /*!< No of bytes DMAC reads from source in a beat                     */
        uint32_t src_burst_len        : 4;     /*!< No of data transfers in a burst when DMAC read from source       */
        uint32_t src_prot_ctrl        : 3;     /*!< Protection control when DMAC reads from source                   */
        uint32_t src_cache_ctrl       : 3;     /*!< Cache control when DMAC reads from source                        */
        uint32_t dst_inc              : 1;     /*!< Destination Fixed/Increment type burst                           */
        uint32_t dst_burst_size       : 3;     /*!< No of bytes DMAC writes to destination in a beat                 */
        uint32_t dst_burst_len        : 4;     /*!< No of data transfers in a burst when DMAC writes to destination  */
        uint32_t dst_prot_ctrl        : 3;     /*!< Protection control when DMAC writes to destination               */
        uint32_t dst_cache_ctrl       : 3;     /*!< Cache control when DMAC writes to destination                    */
        uint32_t endian_swap_size     : 3;     /*!< swap size data                                                   */
    } value_b;
} dma_ccr_t;

/* DMA Loop control information */
typedef struct _dma_loop_t {
    DMA_XFER xfer_type;         /*!< Transfer Type : Single/Burst/Peripheral  */
    DMA_LC   lc;                /*!< Loop Register : LC0/LC1                  */
    uint8_t  jump;              /*!< Backward Jump offset                     */
    bool     nf;                /*!< Loop forever flag  t                     */
} dma_loop_t;

/* DMA Opcode buffer information */
typedef struct _dma_opcode_buf {
    uint8_t  *buf;             /*!< Start address of the opcode buffer   */
    uint32_t  off;             /*!< Current Offset from start address    */
    uint32_t  buf_size;        /*!< Total buffer size                    */
} dma_opcode_buf;

/**
  \fn          bool dma_construct_add(DMA_REG reg,
                                      uint16_t off,
                                      dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMAADDH
  \param[in]   reg Source or Destination Address Register
  \param[in]   off 16bit-immediate offset which needs to be added
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/

static inline bool dma_construct_add(DMA_REG reg,
                                     uint16_t off,
                                     dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_3BYTE_LEN) > op_buf->buf_size)
        return false;

    if (reg == DMA_REG_SAR)
      op_buf->buf[(op_buf->off)++] = OP_DMAADDH(0);
    else if (reg == DMA_REG_DAR)
      op_buf->buf[(op_buf->off)++] = OP_DMAADDH(1);
    else
      return false;

    op_buf->buf[(op_buf->off)++] = (uint8_t)off;
    op_buf->buf[(op_buf->off)++] = (uint8_t)(off >> 8);

    return true;
}

/**
  \fn          bool dma_construct_addneg(DMA_REG reg,
                                         int16_t off,
                                         dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMAADNH
  \param[in]   reg Source or Destination Address Register
  \param[in]   off 16bit-immediate value which needs to be subtracted
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_addneg(DMA_REG reg,
                                        int16_t off,
                                        dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_3BYTE_LEN) > op_buf->buf_size)
      return false;

    if (reg == DMA_REG_SAR)
      op_buf->buf[(op_buf->off)++] = OP_DMAADNH(0);
    else if (reg == DMA_REG_DAR)
      op_buf->buf[(op_buf->off)++] = OP_DMAADNH(1);
    else
      return false;

    off = off - 1;
    off = ~off;
    op_buf->buf[(op_buf->off)++] = (uint8_t)off;
    op_buf->buf[(op_buf->off)++] = (uint8_t)(off >> 8);

    return true;
}

/**
  \fn          bool dma_construct_end(dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMAEND
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_end(dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_1BYTE_LEN) > op_buf->buf_size)
      return false;

    op_buf->buf[(op_buf->off)++] = OP_DMAEND;

    return true;
}

/**
  \fn          bool dma_construct_flushperiph(uint8_t periph,
                                              dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMAFLUSHP
  \param[in]   periph peripheral number
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_flushperiph(uint8_t periph,
                                             dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_2BYTE_LEN) > op_buf->buf_size)
      return false;

    op_buf->buf[(op_buf->off)++] = OP_DMAFLUSHP;
    periph = periph & 0x1F;
    op_buf->buf[(op_buf->off)++] = (uint8_t)(periph << 3);

    return true;
}

/**
  \fn          bool dma_construct_go(DMA_SECURE_STATE ns,
                                     uint8_t channel_num,
                                     uint32_t imm,
                                     dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMAGO
  \param[in]   ns Defines the secure/Non-Secure State
  \param[in]   channel_num Defines the channel number
  \param[in]   imm 32bit address where the microcode resides
  \param[in]   op_buf opcode buf info
  \return      void
*/
static inline bool dma_construct_go(DMA_SECURE_STATE ns,
                                    uint8_t channel_num,
                                    uint32_t imm,
                                    dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_6BYTE_LEN) > op_buf->buf_size)
      return false;

    op_buf->buf[(op_buf->off)++] = (uint8_t)OP_DMAGO(ns);
    op_buf->buf[(op_buf->off)++] = channel_num & 0x7;
    op_buf->buf[(op_buf->off)++] = (uint8_t)imm;
    op_buf->buf[(op_buf->off)++] = (uint8_t)(imm >> 8);
    op_buf->buf[(op_buf->off)++] = (uint8_t)(imm >> 16);
    op_buf->buf[(op_buf->off)++] = (uint8_t)(imm >> 24);

    return true;
}

/**
  \fn          void dma_construct_kill(dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMAKILL
  \param[in]   op_buf opcode buf info
  \return      void
*/
static inline void dma_construct_kill(dma_opcode_buf *op_buf)
{
    *op_buf->buf = OP_DMAKILL;
}

/**
  \fn          bool dma_construct_load(DMA_XFER xfer_type, dma_opcode_buf *op_buf
  \brief       Build the opcode for DMALD
  \param[in]   xfer_type Burst/Single/Force Load operation
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_load(DMA_XFER xfer_type, dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_1BYTE_LEN) > op_buf->buf_size)
      return false;

    if (xfer_type == DMA_XFER_FORCE)
      op_buf->buf[(op_buf->off)++] = OP_DMALD;
    else if (xfer_type == DMA_XFER_BURST)
      op_buf->buf[(op_buf->off)++] = OP_DMALDB;
    else
      op_buf->buf[(op_buf->off)++] = OP_DMALDS;

    return true;
}

/**
  \fn          bool dma_construct_loadperiph(DMA_XFER xfer_type,
                                             uint8_t periph,
                                             dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMALDP
  \param[in]   xfer_type Burst or Single Load operation
  \param[in]   periph Peripheral number
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_loadperiph(DMA_XFER xfer_type,
                                            uint8_t periph,
                                            dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_2BYTE_LEN) > op_buf->buf_size)
      return false;

    if (xfer_type > DMA_XFER_BURST)
      return false;

    op_buf->buf[(op_buf->off)++] = (uint8_t)OP_DMALDP(xfer_type);
    periph = periph & 0x1F;
    op_buf->buf[(op_buf->off)++] = (uint8_t)(periph << 3);

    return true;
}

/**
  \fn          bool dma_construct_loop(DMA_LC lc,
                                       uint8_t iter,
                                       dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMALP
  \param[in]   lc Loop Counter register number
  \param[in]   iter 8bit loop value
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_loop(DMA_LC lc,
                                      uint8_t iter,
                                      dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_2BYTE_LEN) > op_buf->buf_size)
      return false;

    op_buf->buf[(op_buf->off)++] = (uint8_t)OP_DMALP(lc);
    op_buf->buf[(op_buf->off)++] = iter - 1;

    return true;
}

/**
  \fn          bool dma_construct_loopend(dma_loop_t lp_args,
                                          dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMALPEND
  \param[in]   lp_args loop arguments single/burst/force, lc, nf, jump
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_loopend(dma_loop_t *lp_args,
                                         dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_2BYTE_LEN) > op_buf->buf_size)
      return false;

    if (lp_args->nf == 0)
      op_buf->buf[(op_buf->off)++] = OP_DMALPEND(0, 1);
    else if (lp_args->xfer_type == DMA_XFER_FORCE)
      op_buf->buf[(op_buf->off)++] = (uint8_t)OP_DMALPEND(1, lp_args->lc);
    else  if (lp_args->xfer_type == DMA_XFER_BURST)
      op_buf->buf[(op_buf->off)++] = (uint8_t)OP_DMALPENDB(lp_args->lc);
    else
      op_buf->buf[(op_buf->off)++] = (uint8_t)OP_DMALPENDS(lp_args->lc);

    op_buf->buf[(op_buf->off)++] = lp_args->jump;

    return true;
}

/**
  \fn          bool dma_construct_move(uint32_t imm,
                                       DMA_REG reg,
                                       dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMAMOV
  \param[in]   imm 32bit immediate address
  \param[in]   reg SAR/CCR/DAR register
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_move(uint32_t imm,
                                      DMA_REG reg,
                                      dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_6BYTE_LEN) > op_buf->buf_size)
      return false;

    op_buf->buf[(op_buf->off)++] = OP_DMAMOV;
    op_buf->buf[(op_buf->off)++] = reg & 0x7;
    op_buf->buf[(op_buf->off)++] = (uint8_t)imm;
    op_buf->buf[(op_buf->off)++] = (uint8_t)(imm >> 8);
    op_buf->buf[(op_buf->off)++] = (uint8_t)(imm >> 16);
    op_buf->buf[(op_buf->off)++] = (uint8_t)(imm >> 24);

    return true;
}

/**
  \fn          bool dma_construct_nop(dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMANOP
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_nop(dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_1BYTE_LEN) > op_buf->buf_size)
      return false;

    op_buf->buf[(op_buf->off)++] = OP_DMANOP;

    return true;
}

/**
  \fn          bool dma_construct_rmb(dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMARMB
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_rmb(dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_1BYTE_LEN) > op_buf->buf_size)
      return false;

    op_buf->buf[(op_buf->off)++] = OP_DMARMB;

    return true;
}

/**
  \fn          bool dma_construct_send_event(uint8_t event_num,
                                             dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMASEV
  \param[in]   event_num Event number
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_send_event(uint8_t event_num,
                                            dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_2BYTE_LEN) > op_buf->buf_size)
      return false;

    op_buf->buf[(op_buf->off)++] = OP_DMASEV;
    event_num = event_num & 0x1F;
    op_buf->buf[(op_buf->off)++] = (uint8_t)(event_num << 3);

    return true;
}

/**
  \fn          bool dma_construct_store(DMA_XFER xfer_type,
                                        dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMAST
  \param[in]   xfer_type Burst/Single/Force Load operation
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_store(DMA_XFER xfer_type,
                                       dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_1BYTE_LEN) > op_buf->buf_size)
      return false;

    if (xfer_type == DMA_XFER_FORCE)
      op_buf->buf[(op_buf->off)++] = OP_DMAST;
    else if (xfer_type == DMA_XFER_BURST)
      op_buf->buf[(op_buf->off)++] = OP_DMASTB;
    else
      op_buf->buf[(op_buf->off)++] = OP_DMASTS;

    return true;
}

/**
  \fn          bool dma_construct_storeperiph(DMA_XFER xfer_type,
                                              uint8_t periph,
                                              dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMASTP
  \param[in]   xfer_type Burst or Single Store operation
  \param[in]   periph Peripheral number
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_storeperiph(DMA_XFER xfer_type,
                                             uint8_t periph,
                                             dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_2BYTE_LEN) > op_buf->buf_size)
      return false;

    if (xfer_type > DMA_XFER_BURST)
      return false;

    op_buf->buf[(op_buf->off)++] = (uint8_t)OP_DMASTP(xfer_type);
    periph = periph & 0x1F;
    op_buf->buf[(op_buf->off)++] = (uint8_t)(periph << 3);

    return true;
}

/**
  \fn          bool dma_construct_store_zeros(dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMASTZ
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_store_zeros(dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_1BYTE_LEN) > op_buf->buf_size)
      return false;

    op_buf->buf[(op_buf->off)++] = OP_DMASTZ;

    return true;
}

/**
  \fn          bool dma_construct_wfe(bool invalidate,
                                      uint8_t event_num,
                                      dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMAWFE
  \param[in]   invalidate Set for invalidating the Cache
  \param[in]   event_num Event number
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_wfe(bool invalidate,
                                     uint8_t event_num,
                                     dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_2BYTE_LEN) > op_buf->buf_size)
      return false;

    op_buf->buf[(op_buf->off)++] = OP_DMAWFE;
    event_num = event_num & 0x1F;
    op_buf->buf[(op_buf->off)++] = (uint8_t)((event_num << 3) | (invalidate << 1));

    return true;
}

/**
  \fn          bool dma_construct_wfp(DMA_XFER xfer_type,
                                      uint8_t periph_num,
                                      dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMAWFP
  \param[in]   xfer_type Single/Burst/Peripheral
  \param[in]   periph_num Peripheral number
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_wfp(DMA_XFER xfer_type,
                                     uint8_t periph_num,
                                     dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_2BYTE_LEN) > op_buf->buf_size)
      return false;

    if (xfer_type == DMA_XFER_PERIPHERAL)
        op_buf->buf[(op_buf->off)++] = OP_DMAWFP_P(1);
    else
        op_buf->buf[(op_buf->off)++] = (uint8_t)OP_DMAWFP(xfer_type);

    periph_num = periph_num & 0x1F;
    op_buf->buf[(op_buf->off)++] = (uint8_t)(periph_num << 3);

    return true;
}

/**
  \fn          bool dma_construct_wmb(dma_opcode_buf *op_buf)
  \brief       Build the opcode for DMAWMB
  \param[in]   op_buf opcode buf info
  \return      bool true if the opcode fits in the allocated space
*/
static inline bool dma_construct_wmb(dma_opcode_buf *op_buf)
{
    if ((op_buf->off + DMA_OP_1BYTE_LEN) > op_buf->buf_size)
      return false;

    op_buf->buf[(op_buf->off)++] = OP_DMAWMB;

    return true;
}


#ifdef  __cplusplus
}
#endif

#endif /* DMA_OPCODE_H_ */
