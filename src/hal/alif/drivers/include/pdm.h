/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef PDM_H_
#define PDM_H_

#include <stdint.h>
#include <stdbool.h>

/**
  * @brief PDM_PDM_CHANNEL_CFG [PDM_CHANNEL_CFG] ([0..7])
  */
typedef struct {
  volatile uint32_t  PDM_CH_FIR_COEF_0;            /*!< (@ 0x00000000) Channel (n) FIR Filter Coefficient 0 Register              */
  volatile uint32_t  PDM_CH_FIR_COEF_1;            /*!< (@ 0x00000004) Channel (n) FIR Filter Coefficient 1 Register              */
  volatile uint32_t  PDM_CH_FIR_COEF_2;            /*!< (@ 0x00000008) Channel (n) FIR Filter Coefficient 2 Register              */
  volatile uint32_t  PDM_CH_FIR_COEF_3;            /*!< (@ 0x0000000C) Channel (n) FIR Filter Coefficient 3 Register              */
  volatile uint32_t  PDM_CH_FIR_COEF_4;            /*!< (@ 0x00000010) Channel (n) FIR Filter Coefficient 4 Register              */
  volatile uint32_t  PDM_CH_FIR_COEF_5;            /*!< (@ 0x00000014) Channel (n) FIR Filter Coefficient 5 Register              */
  volatile uint32_t  PDM_CH_FIR_COEF_6;            /*!< (@ 0x00000018) Channel (n) FIR Filter Coefficient 6 Register              */
  volatile uint32_t  PDM_CH_FIR_COEF_7;            /*!< (@ 0x0000001C) Channel (n) FIR Filter Coefficient 7 Register              */
  volatile uint32_t  PDM_CH_FIR_COEF_8;            /*!< (@ 0x00000020) Channel (n) FIR Filter Coefficient 8 Register              */
  volatile uint32_t  PDM_CH_FIR_COEF_9;            /*!< (@ 0x00000024) Channel (n) FIR Filter Coefficient 9 Register              */
  volatile uint32_t  PDM_CH_FIR_COEF_10;           /*!< (@ 0x00000028) Channel (n) FIR Filter Coefficient 10 Register             */
  volatile uint32_t  PDM_CH_FIR_COEF_11;           /*!< (@ 0x0000002C) Channel (n) FIR Filter Coefficient 11 Register             */
  volatile uint32_t  PDM_CH_FIR_COEF_12;           /*!< (@ 0x00000030) Channel (n) FIR Filter Coefficient 12 Register             */
  volatile uint32_t  PDM_CH_FIR_COEF_13;           /*!< (@ 0x00000034) Channel (n) FIR Filter Coefficient 13 Register             */
  volatile uint32_t  PDM_CH_FIR_COEF_14;           /*!< (@ 0x00000038) Channel (n) FIR Filter Coefficient 14 Register             */
  volatile uint32_t  PDM_CH_FIR_COEF_15;           /*!< (@ 0x0000003C) Channel (n) FIR Filter Coefficient 15 Register             */
  volatile uint32_t  PDM_CH_FIR_COEF_16;           /*!< (@ 0x00000040) Channel (n) FIR Filter Coefficient 16 Register             */
  volatile uint32_t  PDM_CH_FIR_COEF_17;           /*!< (@ 0x00000044) Channel (n) FIR Filter Coefficient 17 Register             */
  volatile const  uint32_t  RESERVED[14];
  volatile uint32_t  PDM_CH_IIR_COEF_SEL;          /*!< (@ 0x00000080) Channel (n) IIR Filter Coefficient Selection Register      */
  volatile uint32_t  PDM_CH_PHASE;                 /*!< (@ 0x00000084) Channel (n) Phase Control Register                         */
  volatile uint32_t  PDM_CH_GAIN;                  /*!< (@ 0x00000088) Channel (n) Gain Control Register                          */
  volatile uint32_t  PDM_CH_PKDET_TH;              /*!< (@ 0x0000008C) Channel (n) Peak Detector Threshold Register               */
  volatile uint32_t  PDM_CH_PKDET_ITV;             /*!< (@ 0x00000090) Channel (n) Peak Detector Interval Register                */
  volatile const  uint32_t  PDM_CH_PKDET_STAT;     /*!< (@ 0x00000094) Channel (n) Peak Detector Status Register                  */
  volatile const  uint32_t  RESERVED1[26];
} PDM_PDM_CHANNEL_CFG_Type;                        /*!< Size = 256 (0x100)                                                        */

/**
  * @brief PDM (PDM)
  */
typedef struct {                                         /*!< (@ 0x43002000) PDM Structure                                              */
  volatile uint32_t  PDM_CTL0;                           /*!< (@ 0x00000000) PDM Audio Control Register 0                               */
  volatile uint32_t  PDM_CTL1;                           /*!< (@ 0x00000004) PDM Audio Control Register 1                               */
  volatile uint32_t  PDM_FIFO_WATERMARK_H;               /*!< (@ 0x00000008) FIFO Watermark Register                                    */
  volatile const  uint32_t  PDM_FIFO_STAT;               /*!< (@ 0x0000000C) FIFO Status Register                                       */
  volatile const  uint32_t  PDM_ERROR_IRQ;               /*!< (@ 0x00000010) FIFO Error Interrupt Status Register                       */
  volatile const  uint32_t  PDM_WARN_IRQ;                /*!< (@ 0x00000014) FIFO Warning Interrupt Status Register                     */
  volatile const  uint32_t  PDM_AUDIO_DETECT_IRQ;        /*!< (@ 0x00000018) Audio Detection Interrupt Status Register                  */
  volatile uint32_t  PDM_IRQ_ENABLE;                     /*!< (@ 0x0000001C) Interrupt Enable Register                                  */
  volatile uint32_t  PDM_CH0_CH1_AUDIO_OUT;              /*!< (@ 0x00000020) Channels 0 and 1 Audio Output Register                     */
  volatile uint32_t  PDM_CH2_CH3_AUDIO_OUT;              /*!< (@ 0x00000024) Channels 2 and 3 Audio Output Register                     */
  volatile uint32_t  PDM_CH4_CH5_AUDIO_OUT;              /*!< (@ 0x00000028) Channels 4 and 5 Audio Output Register                     */
  volatile uint32_t  PDM_CH6_CH7_AUDIO_OUT;              /*!< (@ 0x0000002C) Channels 6 and 7 Audio Output Register                     */
  volatile const  uint32_t  RESERVED[4];
  volatile PDM_PDM_CHANNEL_CFG_Type PDM_CHANNEL_CFG[8];  /*!< (@ 0x00000040) [0..7]                                                     */
} PDM_Type;                                              /*!< Size = 2112 (0x840)                                                       */

#define PDM0_IRQ_ENABLE               (0xFF03U)                   /* To enable the interrupt status register */
#define PDM_BYPASS_IIR                (1U << 2U)                  /* Bypass DC blocking IIR filter           */
#define PDM_BYPASS_FIR                (1U << 3U)                  /* Bypass FIR filter                       */
#define PDM_PEAK_DETECT_NODE          (1U << 4U)                  /* Peak detection node                     */
#define PDM_DMA_HANDSHAKE             (1U << 24U)                 /* DMA handshaking signals for flow control*/
#define PDM_SAMPLE_ADV                (1U << 17U)                 /* Sample advance                          */

#define PDM_INTERRUPT_STATUS_VALUE    (0x1U)                      /* To check the interrupt status           */

#define PDM_FIFO_ALMOST_FULL_IRQ      (0x1U << 0U)                /* FIFO almost full Interrupt              */
#define PDM_FIFO_OVERFLOW_IRQ         (0x1U << 1U)                /* FIFO overflow interrupt                 */
#define PDM_AUDIO_DETECT_IRQ_STAT     (0xFFU << 8U)               /* Audio detect interrupt                  */
#define PDM_CHANNEL_ENABLE            (0xFFU)                     /* To check the which channel is enabled   */
#define PDM_MODES                     (0xFFU << 16U)              /* To check for the PDM modes              */

#define PDM_FIFO_CLEAR                (1U << 31U)                 /* To clear FIFO clear bit                 */

#define PDM_MAX_FIR_COEFFICIENT       18                          /* PDM channel FIR length                  */
#define PDM_MAX_DMA_CHANNEL           1U                          /* PDM DMA maximum channel                 */

#define PDM_AUDIO_CH_0_1              0U                          /* PDM audio channel 0 and 1               */
#define PDM_AUDIO_CH_2_3              1U                          /* PDM audio channel 2 and 3               */
#define PDM_AUDIO_CH_4_5              2U                          /* PDM audio channel 4 and 5               */
#define PDM_AUDIO_CH_6_7              3U                          /* PDM audio channel 6 and 7               */
#define PDM_CLK_MODE                  16U                         /* PDM clock frequency mode                */

#define PDM_MAX_PHASE_CTRL            0x3FU                       /* PDM phase maximum value                 */
#define PDM_MAX_GAIN_CTRL             0xFFFU                      /* PDM gain maximum value                  */
#define PDM_MAX_CHANNEL               8U                          /* PDM supports a maximum of 8 channels    */

#define PDM_AUDIO_CHANNEL             (0x3U)
#define PDM_CHANNEL_0_1               (PDM_AUDIO_CHANNEL << 0U)   /* check for channel 0 and 1               */
#define PDM_CHANNEL_2_3               (PDM_AUDIO_CHANNEL << 2U)   /* check for channel 2 and 3               */
#define PDM_CHANNEL_4_5               (PDM_AUDIO_CHANNEL << 4U)   /* check for channel 4 and 5               */
#define PDM_CHANNEL_6_7               (PDM_AUDIO_CHANNEL << 6U)   /* check for channel 6 and 7               */
#define PDM_CHANNEL_0                 (1U << 0U)
#define PDM_CHANNEL_1                 (1U << 1U)
#define PDM_CHANNEL_2                 (1U << 2U)
#define PDM_CHANNEL_3                 (1U << 3U)
#define PDM_CHANNEL_4                 (1U << 4U)
#define PDM_CHANNEL_5                 (1U << 5U)
#define PDM_CHANNEL_6                 (1U << 6U)
#define PDM_CHANNEL_7                 (1U << 7U)

typedef enum _PDM_TRANSFER_STATUS
{
    PDM_CAPTURE_STATUS_NONE,        /* PDM capture status none     */
    PDM_AUDIO_STATUS_DETECTION,     /* PDM status audio detection  */
    PDM_CAPTURE_STATUS_COMPLETE,    /* PDM capture status complete */
    PDM_ERROR_DETECT,               /* PDM error detection status  */
}PDM_TRANSFER_STATUS;

/**
 @brief struct pdm_transfer_t:- To store PDM Receive Configuration
 */
typedef struct{
    uint32_t curr_cnt;                    /* Current count value                                      */
    uint32_t total_cnt;                   /* Total count value                                        */
    void *buf;                            /* Channel audio output values are stored in this address   */
    volatile PDM_TRANSFER_STATUS status;  /* transfer status                                          */
}pdm_transfer_t;

/**
 @fn          void pdm_bypass_iir(PDM_Type *pdm, bool arg)
 @brief       Select the Bypass DC blocking IIR filter
 @param[in]   pdm : Pointer to the PDM register map
 @param[in]   arg : Enable or disable the bypass IIR filter
 @return      none
 */
static inline void pdm_bypass_iir(PDM_Type *pdm, bool arg)
{
    if(arg)  /* Enable the bypass IIR filter */
        pdm->PDM_CTL1 |= PDM_BYPASS_IIR;

    else  /* Disable the bypass IIR filter */
        pdm->PDM_CTL1 &= ~(PDM_BYPASS_IIR);
}

/**
 @fn          void pdm_bypass_fir(PDM_Type *pdm, bool arg)
 @brief       To select the Bypass FIR filter
 @param[in]   pdm : Pointer to the PDM register map
 @param[in]   arg : Enable or disable the bypass FIR filter
 @return      none
 */
static inline void pdm_bypass_fir(PDM_Type *pdm, bool arg)
{
    if(arg)  /* Enable the bypass FIR filter */
        pdm->PDM_CTL1 |= PDM_BYPASS_FIR;

    else  /* Disable the bypass FIR filter */
        pdm->PDM_CTL1 &= ~(PDM_BYPASS_FIR);
}

/**
 @fn          void pdm_peak_detect(PDM_Type *pdm, bool arg)
 @brief       To select the Bypass FIR filter
 @param[in]   pdm : Pointer to the PDM register map
 @param[in]   arg : Enable or disable the peak detection node
 @return      none
 */
static inline void pdm_peak_detect(PDM_Type *pdm, bool arg)
{
    if(arg)  /* peak detection after gain stage */
        pdm->PDM_CTL1 |= PDM_PEAK_DETECT_NODE;

    else  /* peak detection before gain stage */
        pdm->PDM_CTL1 &= ~(PDM_PEAK_DETECT_NODE);
}

/**
 @fn          void pdm_sample_advance(PDM_Type *pdm, bool arg)
 @brief       To select the Sample advance
 @param[in]   pdm : Pointer to the PDM register map
 @param[in]   arg : Enable or disable the Sample advance
 @return      none
 */
static inline void pdm_sample_advance(PDM_Type *pdm, bool arg)
{
    if(arg)  /* Enable the Sample advance */
        pdm->PDM_CTL1 |= PDM_SAMPLE_ADV;

    else  /* Disable the Sample advance */
        pdm->PDM_CTL1 &= ~(PDM_SAMPLE_ADV);
}

/**
 @fn          void pdm_dma_handshake(PDM_Type *pdm, bool arg)
 @brief       To Use DMA handshaking signals for flow control
              (Not yet implemented)
 @param[in]   pdm : Pointer to the PDM register map
 @param[in]   arg : Enable or disable the DMA Handshake
 @return      none
 */
static inline void pdm_dma_handshake(PDM_Type *pdm, bool arg)
{
    if(arg)  /* Enable the DMA handshake */
        pdm->PDM_CTL1 |= PDM_DMA_HANDSHAKE;

    else  /* Disable the DMA handshake */
        pdm->PDM_CTL1 &= ~(PDM_DMA_HANDSHAKE);
}

/**
 @fn          void pdm_enable_irq(PDM_Type *pdm)
 @brief       Enable the Fifo overflow IRQ
 @param[in]   pdm : Pointer to the PDM register map
 @return      None
 */
static inline void pdm_dma_enable_irq(PDM_Type *pdm)
{

    pdm->PDM_IRQ_ENABLE &= ~(PDM0_IRQ_ENABLE); /* Clear IRQ */

    /* Enable the Interrupt */
    pdm->PDM_IRQ_ENABLE |= (PDM_FIFO_OVERFLOW_IRQ);
}

/**
 @fn          void pdm_enable_irq(PDM_Type *pdm)
 @brief       Enable the IRQ
 @param[in]   pdm : Pointer to the PDM register map
 @return      None
 */
static inline void pdm_enable_irq(PDM_Type *pdm)
{
    uint32_t audio_ch;

    pdm->PDM_IRQ_ENABLE &= ~(PDM0_IRQ_ENABLE); /* Clear IRQ */

    /* get user enabled channel */
    audio_ch = ((pdm->PDM_CTL0)) & PDM_CHANNEL_ENABLE;

    /* Enable the Interrupt */
    pdm->PDM_IRQ_ENABLE |= (( audio_ch  << 8) | (PDM_FIFO_ALMOST_FULL_IRQ | PDM_FIFO_OVERFLOW_IRQ));
}

/**
 @fn          void pdm_enable_fifo_clear(PDM_Type *pdm)
 @brief       Enable the fifo clear bit
 @param[in]   pdm : Pointer to the PDM register map
 @return      None
 */
static inline void pdm_enable_fifo_clear(PDM_Type *pdm)
{
    pdm->PDM_CTL0 |= PDM_FIFO_CLEAR;
}

/**
 @fn          void pdm_disable_fifo_clear(PDM_Type *pdm)
 @brief       Disable the fifo clear bit
 @param[in]   pdm : Pointer to the PDM register map
 @return      None
 */
static inline void pdm_disable_fifo_clear(PDM_Type *pdm)
{
    pdm->PDM_CTL0 &= ~(PDM_FIFO_CLEAR);
}

/**
 @fn          void pdm_clear_modes(PDM_Type *pdm)
 @brief       Clear the PDM modes
 @param[in]   pdm : Pointer to the PDM register map
 @return      None
 */
static inline void pdm_clear_modes(PDM_Type *pdm)
{
    pdm->PDM_CTL0 &= ~(PDM_MODES);
}

/**
 @fn          void pdm_enable_modes(PDM_Type *pdm, uint32_t arg))
 @brief       Enable the PDM modes
 @param[in]   pdm : Pointer to the PDM register map
 @param[in]   arg : Select the pdm frequency modes
 @return      None
 */
static inline void pdm_enable_modes(PDM_Type *pdm, uint32_t arg)
{
    pdm->PDM_CTL0 |= (arg << PDM_CLK_MODE);
}

/**
 @fn          void pdm_clear_channel(PDM_Type *pdm)
 @brief       Clear the PDM channels
 @param[in]   pdm : Pointer to the PDM register map
 @return      None
 */
static inline void pdm_clear_channel(PDM_Type *pdm)
{
    pdm->PDM_CTL0 &= ~(PDM_CHANNEL_ENABLE);
}

/**
 @fn          void pdm_set_fifo_watermark(PDM_Type *pdm, uint32_t fifo_watermark))
 @brief       Set the pdm fifo watermark value
 @param[in]   pdm            : Pointer to the PDM register map
 @param[in]   fifo_watermark : Threshold to trigger FIFO almost full warning interrupt
 @return      None
 */
static inline void pdm_set_fifo_watermark(PDM_Type *pdm, uint32_t fifo_watermark)
{
    pdm->PDM_FIFO_WATERMARK_H |= fifo_watermark;
}

/**
 @fn          void pdm_enable_multi_ch(PDM_Type *pdm, uint32_t arg))
 @brief       Enable the PDM multiple channels
 @param[in]   pdm : Pointer to the PDM register map
 @param[in]   arg : Enable the pdm multiple channels
 @return      None
 */
static inline void pdm_enable_multi_ch(PDM_Type *pdm, uint32_t arg)
{
    pdm->PDM_CTL0 |= arg;
}

/**
 @fn          void pdm_set_ch_iir_coef(PDM_Type *pdm, uint8_t ch_num, uint32_t ch_iir_coef)
 @brief       Set the pdm channel IIR filter coefficient value
 @param[in]   pdm         : Pointer to the PDM register map
 @param[in]   ch_num      : Select the pdm channel
 @param[in]   ch_iir_coef : Set the pdm channel IIR filter coefficient value
 @return      None
 */
static inline void pdm_set_ch_iir_coef(PDM_Type *pdm, uint8_t ch_num, uint32_t ch_iir_coef)
{
    pdm->PDM_CHANNEL_CFG[ch_num].PDM_CH_IIR_COEF_SEL = ch_iir_coef;
}

/**
 @fn          void pdm_set_ch_phase(PDM_Type *pdm, uint8_t ch_num, uint32_t ch_phase)
 @brief       Set the pdm channel phase control value
 @param[in]   pdm      : Pointer to the PDM register map
 @param[in]   ch_num   : Select the pdm channel
 @param[in]   ch_phase : Set the pdm channel phase control value
 @return      None
 */
static inline void pdm_set_ch_phase(PDM_Type *pdm, uint8_t ch_num, uint32_t ch_phase)
{
    pdm->PDM_CHANNEL_CFG[ch_num].PDM_CH_PHASE = ch_phase;
}

/**
 @fn          void pdm_set_ch_gain(PDM_Type *pdm, uint8_t ch_num, uint32_t ch_gain)
 @brief       Set the pdm channel gain control value
 @param[in]   pdm      : Pointer to the PDM register map
 @param[in]   ch_num   : Select the pdm channel
 @param[in]   ch_gain  : Set the pdm channel gain control value
 @return      None
 */
static inline void pdm_set_ch_gain(PDM_Type *pdm, uint8_t ch_num, uint32_t ch_gain)
{
    pdm->PDM_CHANNEL_CFG[ch_num].PDM_CH_GAIN = ch_gain;
}

/**
 @fn          void pdm_set_peak_detect_th(PDM_Type *pdm, uint8_t ch_num,
                                          uint32_t ch_peak_detect_th)
 @brief       Set the pdm channel Peak detector threshold value
 @param[in]   pdm      : Pointer to the PDM register map
 @param[in]   ch_num   : Select the pdm channel
 @param[in]   ch_peak_detect_th  : Set the pdm channel Peak detector
                                   threshold value
 @return      None
 */
static inline void pdm_set_peak_detect_th(PDM_Type *pdm, uint8_t ch_num,
                                          uint32_t ch_peak_detect_th)
{
    pdm->PDM_CHANNEL_CFG[ch_num].PDM_CH_PKDET_TH = ch_peak_detect_th;
}

/**
 @fn          void pdm_set_peak_detect_th(PDM_Type *pdm, uint8_t ch_num,
                                          uint32_t ch_peak_detect_itv)
 @brief       Set the pdm channel Peak detector interval value
 @param[in]   pdm      : Pointer to the PDM register map
 @param[in]   ch_num   : Select the pdm channel
 @param[in]   ch_peak_detect_itv  : Set the pdm channel Peak detector
                                    interval value
 @return      None
 */
static inline void pdm_set_peak_detect_itv(PDM_Type *pdm, uint8_t ch_num,
                                           uint32_t ch_peak_detect_itv)
{
    pdm->PDM_CHANNEL_CFG[ch_num].PDM_CH_PKDET_ITV = ch_peak_detect_itv;
}

/**
 @fn          void pdm_set_fir_coeff(PDM_Type *pdm, uint8_t ch_num,
                                     uint32_t ch_fir_coef[PDM_MAX_FIR_COEFFICIENT])
 @brief       Set the pdm channel Peak detector interval value
 @param[in]   pdm      : Pointer to the PDM register map
 @param[in]   ch_num   : Select the pdm channel
 @param[in]   ch_fir_coef  : Set the pdm channel Fir coefficient values
 @return      None
 */
static inline void pdm_set_fir_coeff(PDM_Type *pdm, uint8_t ch_num, uint32_t ch_fir_coef[PDM_MAX_FIR_COEFFICIENT])
{
    uint32_t i;
    uint32_t *ch_n_fir_coef_0 = (uint32_t *)&(pdm->PDM_CHANNEL_CFG[ch_num].PDM_CH_FIR_COEF_0);

    for(i = 0; i< PDM_MAX_FIR_COEFFICIENT; i++)
    {
        *(ch_n_fir_coef_0) = ch_fir_coef[i];
         ch_n_fir_coef_0 ++;
    }
}

/**
 @fn          uint32_t pdm_get_active_channels(PDM_Type *pdm)
 @brief       return PDM active channels
 @param[in]   pdm : Pointer to the PDM register map
 @return      PDM active channels
 */
static inline uint32_t pdm_get_active_channels(PDM_Type *pdm)
{
    return pdm->PDM_CTL0 & PDM_CHANNEL_ENABLE;
}

/**
 @fn          uint32_t* pdm_get_ch0_1_addr(PDM_Type *pdm)
 @brief       return PDM channel 0 and 1 reg address
 @param[in]   pdm : Pointer to the PDM register map
 @return      return the address
 */
static inline volatile uint32_t* pdm_get_ch0_1_addr(PDM_Type *pdm)
{
    return &(pdm->PDM_CH0_CH1_AUDIO_OUT);
}

/**
 @fn          uint32_t* pdm_get_ch2_3_addr(PDM_Type *pdm)
 @brief       return PDM channel 2 and 3 reg address
 @param[in]   pdm : Pointer to the PDM register map
 @return      return the address
 */
static inline volatile uint32_t* pdm_get_ch2_3_addr(PDM_Type *pdm)
{
    return &(pdm->PDM_CH2_CH3_AUDIO_OUT);
}

/**
 @fn          uint32_t* pdm_get_ch4_5_addr(PDM_Type *pdm)
 @brief       return PDM channel 4 and 5 reg address
 @param[in]   pdm : Pointer to the PDM register map
 @return      return the address
 */
static inline volatile uint32_t* pdm_get_ch4_5_addr(PDM_Type *pdm)
{
    return &(pdm->PDM_CH4_CH5_AUDIO_OUT);
}

/**
 @fn          uint32_t* pdm_get_ch6_7_addr(PDM_Type *pdm)
 @brief       return PDM channel 6 and 7 reg address
 @param[in]   pdm : Pointer to the PDM register map
 @return      return the address
 */
static inline volatile uint32_t* pdm_get_ch6_7_addr(PDM_Type *pdm)
{
    return &(pdm->PDM_CH6_CH7_AUDIO_OUT);
}

/**
 @fn          void pdm_disable_error_irq(PDM_Type *pdm)
 @brief       Disable pdm error irq
 @param[in]   pdm : Pointer to the PDM register map
 @return      None
 */
static inline void pdm_disable_error_irq(PDM_Type *pdm)
{
    pdm->PDM_IRQ_ENABLE &= ~PDM_FIFO_OVERFLOW_IRQ;
}
/**
  @fn          void pdm_error_detect_irq_handler(PDM_Type *pdm);
  @brief       IRQ handler for the error interrupt
  @param[in]   pdm      : Pointer to the PDM register map
  @return      none
*/
void pdm_error_detect_irq_handler(PDM_Type *pdm);

/**
  @fn          void pdm_audio_detect_irq_handler(PDM_Type *pdm, pdm_transfer_t *transfer );
  @brief       IRQ handler for the audio detect interrupt
  @param[in]   pdm      : Pointer to the PDM register map
  @param[in]   transfer     : The transfer structure of the PDM instance
  @return      none
*/
void pdm_audio_detect_irq_handler(PDM_Type *pdm, pdm_transfer_t *transfer );

/**
  @fn          void pdm_warning_irq_handler(PDM_Type *pdm, pdm_transfer_t *transfer);
  @brief       IRQ handler for the PDM warning interrupt.
  @param[in]   pdm      : Pointer to the PDM register map
  @param[in]   transfer : The transfer structure of the PDM instance
  @return      none
*/
void pdm_warning_irq_handler(PDM_Type *pdm, pdm_transfer_t *transfer);

#endif /* PDM_H_ */
