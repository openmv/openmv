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
 * @file     dma.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     11-08-2023
 * @brief    Low Level Header file for DMA
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef DMA_H_
#define DMA_H_

#include <stdint.h>

#ifdef  __cplusplus
}
#endif

/**
  * @brief DMA_DMA_CHANNEL_RT_INFO_Type [DMA_CHANNEL_RT_INFO] ([0..7])
  */
typedef struct {
  volatile const  uint32_t  DMA_CSR;            /*!< (@ 0x00000000) Channel Status for DMA Channel (n) Register                */
  volatile const  uint32_t  DMA_CPC;            /*!< (@ 0x00000004) Channel PC for DMA Channel (n) Register                    */
} DMA_DMA_CHANNEL_RT_INFO_Type;                 /*!< Size = 8 (0x8)                                                            */


/**
  * @brief DMA_DMA_RT_CHANNEL_CFG_Type [DMA_RT_CHANNEL_CFG] ([0..7])
  */
typedef struct {
  volatile const  uint32_t  DMA_SAR;            /*!< (@ 0x00000000) Source Address for DMA Channel (n) Register                */
  volatile const  uint32_t  DMA_DAR;            /*!< (@ 0x00000004) Destination Address for DMA Channel (n) Register           */
  volatile const  uint32_t  DMA_CCR;            /*!< (@ 0x00000008) Channel Control for DMA Channel (n) Register               */
  volatile const  uint32_t  DMA_LC0;            /*!< (@ 0x0000000C) Loop Counter 0 for DMA Channel (n) Register                */
  volatile const  uint32_t  DMA_LC1;            /*!< (@ 0x00000010) Loop Counter 1 for DMA Channel (n) Register                */
  volatile const  uint32_t  RESERVED[3];
} DMA_DMA_RT_CHANNEL_CFG_Type;                  /*!< Size = 32 (0x20)                                                          */


/**
  * @brief DMA (DMA)
  */

typedef struct {                                /*!< (@ 0x00000000) DMA Structure                                         */
  volatile const  uint32_t  DMA_DSR;            /*!< (@ 0x00000000) DMA Manager Status Register                                */
  volatile const  uint32_t  DMA_DPC;            /*!< (@ 0x00000004) DMA Program Counter Register                               */
  volatile const  uint32_t  RESERVED[6];
  volatile        uint32_t  DMA_INTEN;          /*!< (@ 0x00000020) Interrupt Enable Register                                  */
  volatile const  uint32_t  DMA_INT_EVENT_RIS;  /*!< (@ 0x00000024) Event-Interrupt Raw Status Register                        */
  volatile const  uint32_t  DMA_INTMIS;         /*!< (@ 0x00000028) Interrupt Status Register                                  */
  volatile        uint32_t  DMA_INTCLR;         /*!< (@ 0x0000002C) Interrupt Clear Register                                   */
  volatile const  uint32_t  DMA_FSRD;           /*!< (@ 0x00000030) Fault Status DMA Manager Register                          */
  volatile const  uint32_t  DMA_FSRC;           /*!< (@ 0x00000034) Fault Status DMA Channel Register                          */
  volatile const  uint32_t  DMA_FTRD;           /*!< (@ 0x00000038) Fault Type DMA Manager Register                            */
  volatile const  uint32_t  RESERVED1;
  volatile const  uint32_t  DMA_FTR[8];         /*!< (@ 0x00000040) Fault Type for DMA Channel (n) Register                    */
  volatile const  uint32_t  RESERVED2[40];
  volatile DMA_DMA_CHANNEL_RT_INFO_Type DMA_CHANNEL_RT_INFO[8];/*!< (@ 0x00000100) [0..7]                                    */
  volatile const  uint32_t  RESERVED3[176];
  volatile DMA_DMA_RT_CHANNEL_CFG_Type DMA_RT_CHANNEL_CFG[8];/*!< (@ 0x00000400) [0..7]                                      */
  volatile const  uint32_t  RESERVED4[512];
  volatile const  uint32_t  DMA_DBGSTATUS;      /*!< (@ 0x00000D00) Debug Status Register                                      */
  volatile        uint32_t  DMA_DBGCMD;         /*!< (@ 0x00000D04) Debug Command Register                                     */
  volatile        uint32_t  DMA_DBGINST0;       /*!< (@ 0x00000D08) Debug Instruction Register 0                               */
  volatile        uint32_t  DMA_DBGINST1;       /*!< (@ 0x00000D0C) Debug Instruction Register 1                               */
  volatile const  uint32_t  RESERVED5[60];
  volatile const  uint32_t  DMA_CR0;            /*!< (@ 0x00000E00) Configuration Register 0                                   */
  volatile const  uint32_t  DMA_CR1;            /*!< (@ 0x00000E04) Configuration Register 1                                   */
  volatile const  uint32_t  DMA_CR2;            /*!< (@ 0x00000E08) Configuration Register 2                                   */
  volatile const  uint32_t  DMA_CR3;            /*!< (@ 0x00000E0C) Configuration Register 3                                   */
  volatile const  uint32_t  DMA_CR4;            /*!< (@ 0x00000E10) Configuration Register 4                                   */
  volatile const  uint32_t  DMA_CRD;            /*!< (@ 0x00000E14) DMA Configuration Register                                 */
  volatile const  uint32_t  RESERVED6[26];
  volatile        uint32_t  DMA_WD;             /*!< (@ 0x00000E80) Watchdog Register                                          */
} DMA_Type;                                     /*!< Size = 3716 (0xe84)                                                       */

/* Register fields and masks */

/* DMA DSR: Status of Manager thread */
#define DMA_DSR_DMA_STATUS_Pos               0U
#define DMA_DSR_DMA_STATUS_Msk               (0xFUL << DMA_DSR_DMA_STATUS_Pos)
#define DMA_DSR_WAKEUP_EVENT_Pos             4U
#define DMA_DSR_WAKEUP_EVENT_Msk             (0x1FUL << DMA_DSR_WAKEUP_EVENT_Pos)
#define DMA_DSR_DNS_Pos                      9U
#define DMA_DSR_DNS_Msk                      (0x1UL << DMA_DSR_DNS_Pos)

/* DMA FSRD: Fault Status of Manager */
#define DMA_FSRD_FS_MGR_Pos                  0U
#define DMA_FSRD_FS_MGR_Msk                  (0x1UL << DMA_FSRD_FS_MGR_Pos)

/* DMA FSRC: Fault Status of Channels */
#define DMA_FSRC_FAULT_STATUS_Pos            0U
#define DMA_FSRC_FAULT_STATUS_Msk            (0xFFUL << DMA_FSRC_FAULT_STATUS_Pos)

/* DMA CSR: Channel Status */
#define DMA_CSR_CHANNEL_STATUS_Pos           0U
#define DMA_CSR_CHANNEL_STATUS_Msk           (0xFUL << DMA_CSR_CHANNEL_STATUS_Pos)
#define DMA_CSR_WAKEUP_NUMBER_Pos            4U
#define DMA_CSR_WAKEUP_NUMBER_Msk            (0x1FUL << DMA_CSR_WAKEUP_NUMBER_Pos)
#define DMA_CSR_DMAWFP_B_NS_Pos              14U
#define DMA_CSR_DMAWFP_B_NS_Msk              (0x1UL << DMA_CSR_DMAWFP_B_NS_Pos)
#define DMA_CSR_DMAWFP_PERIPH_Pos            15U
#define DMA_CSR_DMAWFP_PERIPH_Msk            (0x1UL << DMA_CSR_DMAWFP_PERIPH_Pos)
#define DMA_CSR_CNS_Pos                      21U
#define DMA_CSR_CNS_Msk                      (0x1UL << DMA_CSR_CNS_Pos)

/* DMA DBGSTATUS: Debug Status */
#define DMA_DBGSTATUS_DBGSTATUS_Pos          0U
#define DMA_DBGSTATUS_DBGSTATUS_Msk          (0x1UL << DMA_DBGSTATUS_DBGSTATUS_Pos)

/* DMA WD.IRQ.ONLY: Watchdog IRQ Only */
#define DMA_WD_IRQ_ONLY_Pos                  0U
#define DMA_WD_IRQ_ONLY_Msk                  (0x1UL << DMA_WD_IRQ_ONLY_Pos)

#define DMA_MAX_CHANNELS     8                  /*!< Max 8 channels supported */
#define DMA_MAX_PERIPH_REQ   32                 /*!< Max 32 peripheral requests supported */
#define DMA_MAX_EVENTS       32                 /*!< Max 32 events supported */
#define DMA_MAX_IRQ          32                 /*!< Max 32 irqs supported */
#define DMA_MAX_BURST_LEN    16                 /*!< Max 16 xfers are supported */
#define DMA_MAX_BURST_SIZE   8                  /*!< Max 8bytes(DATA_WIDTH/8) */
#define DMA_MAX_BUFF_DEPTH   128                /*!< 32lines * 4bytes for each FIFO */

#define DMA_IRQ_ABORT_OFFSET 32                 /*!< Abort irq offset */

/**
 * enum DMA_THREAD
 * DMA Thread: Manager and Channel Thread
 */
typedef enum _DMA_THREAD {
    DMA_THREAD_MANAGER = 0,                     /*!< DMA Manager thread */
    DMA_THREAD_CHANNEL,                         /*!< DMA Channel thread */
} DMA_THREAD;

/**
 * enum DMA_THREAD_STATUS
 * DMA Thread Status: Covers both Manager and Channel Thread Status.
 */
typedef enum _DMA_THREAD_STATUS {
    DMA_THREAD_STATUS_STOPPED = 0,                /*!< MGR/CHNL Thread is Stopped */
    DMA_THREAD_STATUS_EXECUTING,                  /*!< MGR/CHNL Thread is executing */
    DMA_THREAD_STATUS_CACHE_MISS,                 /*!< MGR/CHNL Cache miss occurred */
    DMA_THREAD_STATUS_UPDATING_PC,                /*!< MGR/CHNL Thread is updating the program counter */
    DMA_THREAD_STATUS_WAITING_FOR_EVENT,          /*!< MGR/CHNL Thread is waiting for an event */
    DMA_THREAD_STATUS_AT_BARRIER,                 /*!< CHNL Thread is waiting at an barrier */
    DMA_THREAD_STATUS_WAITING_FOR_PERIPHERAL = 7, /*!< CHNL Thread is waiting for a peripheral */
    DMA_THREAD_STATUS_KILLING,                    /*!< CHNL Thread is at killing state */
    DMA_THREAD_STATUS_COMPLETING,                 /*!< CHNL Thread is at completing state */
    DMA_THREAD_STATUS_FAULTING_COMPLETING = 14,   /*!< CHNL Thread is in faulting completing state */
    DMA_THREAD_STATUS_FAULTING = 15,              /*!< MGR/CHNL Thread is in Faulting State */
} DMA_THREAD_STATUS;

/* Fault Type DMA  Manager Register */
typedef union _dma_ftrd_t {
    uint32_t ftrd;
    struct {
        uint32_t undef_instr     : 1;     /*!< Undefined Instruction */
        uint32_t operand_invalid : 1;     /*!< Invalid arguments passed */
        uint32_t RESERVED1       : 2;     /*!< Reserved */
        uint32_t dmago_err       : 1;     /*!< Security aspect while executing DMAGO */
        uint32_t mgr_evnt_err    : 1;     /*!< security aspect while executing DMAWFE or DMASEV */
        uint32_t RESERVED2       : 10;    /*!< Reserved */
        uint32_t instr_fetch_err : 1;     /*!< Instr fetch error on AXI */
        uint32_t RESERVED3       : 13;    /*!< Reserved */
        uint32_t dbg_instr       : 1;     /*!< Instr read from system or debug interface */
    } ftrd_b;
} dma_ftrd_t;

/* Fault Type DMA  Channel Register */
typedef union _dma_ftr {
    uint32_t ftr;
    struct {
        uint32_t undef_instr     : 1;     /*!< Undefined Instruction */
        uint32_t operand_invalid : 1;     /*!< Invalid arguments passed */
        uint32_t reserved1       : 3;     /*!< Reserved */
        uint32_t ch_evnt_err     : 1;     /*!< security aspect while executing DMAWFE or DMASEV */
        uint32_t ch_periph_err   : 1;     /*!< security aspect while executing DMAWFP/DMALDP/DMASTP/DMAFLUSHP */
        uint32_t ch_rdwr_err     : 1;     /*!< security aspect while programming CCRn */
        uint32_t reserved2       : 4;     /*!< Reserved */
        uint32_t mfifo_err       : 1;     /*!< mfifo prevented channel from executing DMALD/DMAST */
        uint32_t no_st_data      : 1;     /*!< mfifo didn't contain data to perform DMAST */
        uint32_t reserved3       : 2;     /*!< Reserved */
        uint32_t instr_fetch_err : 1;     /*!< AXI response on inst fetch */
        uint32_t data_write_err  : 1;     /*!< AXI response on data write */
        uint32_t data_read_err   : 1;     /*!< AXI response on data read */
        uint32_t reserved4       : 11;    /*!< Reserved */
        uint32_t dbg_instr       : 1;     /*!< Instr read from system or debug interface */
        uint32_t lockup_err      : 1;     /*!< channel lockedup due to resource starvation */
    } ftr_b;
} dma_ftr;

/* DMA  Instruction0 Register */
typedef union _dma_dbginst0_t {
    uint32_t dbginst0;
    struct {
        uint32_t dbg_thrd             : 1;     /*!< Debug Manager/Channel Thread */
        uint32_t reserved1            : 7;     /*!< Reserved */
        uint32_t chn_num              : 3;     /*!< Channel number */
        uint32_t reserved2            : 5;     /*!< Reserved */
        uint32_t ins_byte0            : 8;     /*!< Instruction byte 0 */
        uint32_t ins_byte1            : 8;     /*!< Instruction byte 1 */
    } dbginst0_b;
} dma_dbginst0_t;

/* DMA  Instruction1 Register */
typedef union _dma_dbginst1_t {
    uint32_t dbginst1;
    struct {
        uint32_t ins_byte2            : 8;     /*!< Instruction byte 2 */
        uint32_t ins_byte3            : 8;     /*!< Instruction byte 3 */
        uint32_t ins_byte4            : 8;     /*!< Instruction byte 4 */
        uint32_t ins_byte5            : 8;     /*!< Instruction byte 5 */
    } dbginst1_b;
} dma_dbginst1_t;

/**
  \fn          void dma_enable_interrupt(DMA_Type *dma,
                                         uint8_t   event_irq_select_index)
  \brief       Enable the interrupt for the requested interrupt line
  \param[in]   dma  Pointer to DMA register map
  \param[in]   event_irq_select_index  Index of the interrupt to be enabled
*/
static inline void dma_enable_interrupt(DMA_Type *dma,
                                        uint8_t   event_irq_select_index)
{
    dma->DMA_INTEN |= (1U << event_irq_select_index);
}

/**
  \fn          void dma_disable_interrupt(DMA_Type *dma,
                                          uint8_t event_irq_select_index)
  \brief       Disable the interrupt for the requested interrupt line
  \param[in]   dma  Pointer to DMA register map
  \param[in]   event_irq_select_index  Index of the interrupt to be disabled
*/
static inline void dma_disable_interrupt(DMA_Type *dma,
                                         uint8_t   event_irq_select_index)
{
    dma->DMA_INTEN &= ~(1U << event_irq_select_index);
}

/**
  \fn          void dma_clear_interrupt(DMA_Type *dma, uint8_t irq_clr_index)
  \brief       Clear the interrupt for the requested interrupt line
  \param[in]   dma  Pointer to DMA register map
  \param[in]   irq_clr_index  Index of the interrupt to be cleared
*/
static inline void dma_clear_interrupt(DMA_Type *dma, uint8_t irq_clr_index)
{
    dma->DMA_INTCLR = (1U << irq_clr_index);
}

/**
  \fn          bool dma_manager_is_nonsecure(DMA_Type *dma)
  \brief       Get Security status of the Manager thread
  \param[in]   dma    Pointer to DMA register map
  \return      bool True if Non-Secure
*/
static inline bool dma_manager_is_nonsecure(DMA_Type *dma)
{
    return (dma->DMA_DSR & DMA_DSR_DNS_Msk);
}

/**
  \fn          bool dma_manager_is_faulting(DMA_Type *dma)
  \brief       Get fault status of the Manager thread
  \param[in]   dma    Pointer to DMA register map
  \return      bool True if Manager thread is faulting
*/
static inline bool dma_manager_is_faulting(DMA_Type *dma)
{
    return (dma->DMA_FSRD & DMA_FSRD_FS_MGR_Msk);
}

/**
  \fn          bool dma_get_channel_fault_status(DMA_Type *dma,
                                                 uint8_t   channel_num)
  \brief       Get Fault status of the requested channel
  \param[in]   dma Pointer to DMA register map
  \param[in]   channel_num Channel Number
  \return      bool True if channel is faulted
*/
static inline bool dma_get_channel_fault_status(DMA_Type *dma,
                                                uint8_t   channel_num)
{
    return (dma->DMA_FSRC & (1 << channel_num));
}

/**
  \fn          DMA_THREAD_STATUS dma_get_channel_status(DMA_Type *dma,
                                                        uint8_t   channel_num)
  \brief       Get Current status of the Channel
  \param[in]   dma    Pointer to DMA register map
  \param[in]   channel_num Channel Number
  \return      DMA_THREAD_STATUS channel status
*/
static inline DMA_THREAD_STATUS dma_get_channel_status(DMA_Type *dma,
                                                       uint8_t   channel_num)
{
    return (DMA_THREAD_STATUS)(dma->DMA_CHANNEL_RT_INFO[channel_num].DMA_CSR
                               & DMA_CSR_CHANNEL_STATUS_Msk);
}

/**
  \fn          uint32_t dma_get_channel_src_addr(DMA_Type *dma,
                                                 uint8_t   channel_num)
  \brief       Get Source Address of the Channel
  \param[in]   dma    Pointer to DMA register map
  \param[in]   channel_num Channel Number
  \return      uint32_t Current Source Address
*/
static inline uint32_t dma_get_channel_src_addr(DMA_Type *dma,
                                                uint8_t   channel_num)
{
    return dma->DMA_RT_CHANNEL_CFG[channel_num].DMA_SAR;
}

/**
  \fn          uint32_t dma_get_channel_dest_addr(DMA_Type *dma,
                                                  uint8_t   channel_num)
  \brief       Get Destination Address of the Channel
  \param[in]   dma    Pointer to DMA register map
  \param[in]   channel_num Channel Number
  \return      uint32_t Current Destination Address
*/
static inline uint32_t dma_get_channel_dest_addr(DMA_Type *dma,
                                                 uint8_t   channel_num)
{
    return dma->DMA_RT_CHANNEL_CFG[channel_num].DMA_DAR;
}

/**
  \fn          bool dma_debug_is_busy(DMA_Type *dma)
  \brief       Get Debug Status of DMAC
  \param[in]   dma    Pointer to DMA register map
  \return      bool True if DMAC is busy in executing debug CMDS
*/
static inline bool dma_debug_is_busy(DMA_Type *dma)
{
    return (dma->DMA_DBGSTATUS & DMA_DBGSTATUS_DBGSTATUS_Msk);
}

/**
  \fn          void dma_execute(DMA_Type *dma,
                               uint32_t   dbginst0,
                               uint32_t   dbginst1)
  \brief       Execute command in DBGINST register
  \param[in]   dma    Pointer to DMA register map
  \param[in]   dbginst0   DBGINST0
  \param[in]   dbginst1   DBGINST1
  \return      void
*/
static inline void dma_execute(DMA_Type *dma,
                               uint32_t  dbginst0,
                               uint32_t  dbginst1)
{
    dma->DMA_DBGINST0 = dbginst0;
    dma->DMA_DBGINST1 = dbginst1;
    dma->DMA_DBGCMD   = 0;
}

#ifdef  __cplusplus
}
#endif

#endif /* DMA_H_ */
