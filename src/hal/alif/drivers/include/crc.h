/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef CRC_H_
#define CRC_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* 8bit and 32bit Data input Reg offset */
#define CRC_DATA_IN_8BIT_REG_OFFSET   0x20
#define CRC_DATA_IN_32BIT_REG_OFFSET  0x60

/**
 @brief struct CRC_Type:- Register map for CRC
 */
typedef struct {                                     /*!< (@ 0x48107000) CRC0 Structure                                             */
    volatile uint32_t  CRC_CONTROL;                  /*!< (@ 0x00000000) CRC Calculation Setup Register                             */
    volatile const  uint32_t  RESERVED[3];
    volatile uint32_t  CRC_SEED;                     /*!< (@ 0x00000010) Seed Value Register                                        */
    volatile uint32_t  CRC_POLY_CUSTOM;              /*!< (@ 0x00000014) Custom Polynomial Register                                 */
    volatile const  uint32_t  CRC_OUT;               /*!< (@ 0x00000018) Accumulated CRC Register                                   */
    volatile const  uint32_t  RESERVED1;
    volatile  uint8_t   CRC_DATA_IN_8_0;              /*!< (@ 0x00000020) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED2;
    volatile const  uint16_t  RESERVED3;
    volatile  uint8_t   CRC_DATA_IN_8_1;              /*!< (@ 0x00000024) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED4;
    volatile const  uint16_t  RESERVED5;
    volatile  uint8_t   CRC_DATA_IN_8_2;              /*!< (@ 0x00000028) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED6;
    volatile const  uint16_t  RESERVED7;
    volatile  uint8_t   CRC_DATA_IN_8_3;              /*!< (@ 0x0000002C) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED8;
    volatile const  uint16_t  RESERVED9;
    volatile  uint8_t   CRC_DATA_IN_8_4;              /*!< (@ 0x00000030) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED10;
    volatile const  uint16_t  RESERVED11;
    volatile  uint8_t   CRC_DATA_IN_8_5;              /*!< (@ 0x00000034) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED12;
    volatile const  uint16_t  RESERVED13;
    volatile  uint8_t   CRC_DATA_IN_8_6;              /*!< (@ 0x00000038) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED14;
    volatile const  uint16_t  RESERVED15;
    volatile  uint8_t   CRC_DATA_IN_8_7;              /*!< (@ 0x0000003C) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED16;
    volatile const  uint16_t  RESERVED17;
    volatile  uint8_t   CRC_DATA_IN_8_8;              /*!< (@ 0x00000040) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED18;
    volatile const  uint16_t  RESERVED19;
    volatile  uint8_t   CRC_DATA_IN_8_9;              /*!< (@ 0x00000044) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED20;
    volatile const  uint16_t  RESERVED21;
    volatile  uint8_t   CRC_DATA_IN_8_10;             /*!< (@ 0x00000048) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED22;
    volatile const  uint16_t  RESERVED23;
    volatile  uint8_t   CRC_DATA_IN_8_11;             /*!< (@ 0x0000004C) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED24;
    volatile const  uint16_t  RESERVED25;
    volatile  uint8_t   CRC_DATA_IN_8_12;             /*!< (@ 0x00000050) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED26;
    volatile const  uint16_t  RESERVED27;
    volatile  uint8_t   CRC_DATA_IN_8_13;             /*!< (@ 0x00000054) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED28;
    volatile const  uint16_t  RESERVED29;
    volatile  uint8_t   CRC_DATA_IN_8_14;             /*!< (@ 0x00000058) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED30;
    volatile const  uint16_t  RESERVED31;
    volatile  uint8_t   CRC_DATA_IN_8_15;             /*!< (@ 0x0000005C) 8-bit Values Register n                                    */
    volatile const  uint8_t   RESERVED32;
    volatile const  uint16_t  RESERVED33;
    volatile  uint32_t  CRC_DATA_IN_32_0;             /*!< (@ 0x00000060) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_1;             /*!< (@ 0x00000064) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_2;             /*!< (@ 0x00000068) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_3;             /*!< (@ 0x0000006C) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_4;             /*!< (@ 0x00000070) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_5;             /*!< (@ 0x00000074) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_6;             /*!< (@ 0x00000078) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_7;             /*!< (@ 0x0000007C) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_8;             /*!< (@ 0x00000080) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_9;             /*!< (@ 0x00000084) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_10;            /*!< (@ 0x00000088) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_11;            /*!< (@ 0x0000008C) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_12;            /*!< (@ 0x00000090) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_13;            /*!< (@ 0x00000094) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_14;            /*!< (@ 0x00000098) 32-bit Values Register n                                   */
    volatile  uint32_t  CRC_DATA_IN_32_15;            /*!< (@ 0x0000009C) 32-bit Values Register n                                   */
}CRC_Type;                                            /*!< Size = 160 (0xa0)                                                         */

#define CRC_REFLECT             (1 << 11)            /* To Reflect the CRC value            */
#define CRC_INVERT              (1 << 10)            /* To Invert the CRC value             */
#define CRC_CUSTOM_POLY         (1 << 9)             /* To enable the poly custom           */
#define CRC_BIT_SWAP            (1 << 8)             /* To enable the Bit swap              */
#define CRC_BYTE_SWAP           (1 << 7)             /* To enable the Byte swap             */
#define CRC_8_CCITT             (0 << 3)             /* To select the CRC_8_CCITT           */
#define CRC_16                  (2 << 3)             /* To select the CRC_16                */
#define CRC_16_CCITT            (3 << 3)             /* To select the CRC_16_CCITT          */
#define CRC_32                  (4 << 3)             /* To select the CRC_32                */
#define CRC_32C                 (5 << 3)             /* To select the CRC_32C               */
#define CRC_ALGO_8_BIT_SIZE     (0 << 1)             /* To select the 8 bit algorithm size  */
#define CRC_ALGO_16_BIT_SIZE    (1 << 1)             /* To select the 16 bit algorithm size */
#define CRC_ALGO_32_BIT_SIZE    (2 << 1)             /* To select the 32 bit algorithm size */
#define CRC_INIT_BIT            (1 << 0)             /* To select the init value            */
#define CRC_ALGORITHM_CHECK     (3 << 1)             /* To check for the CRC algorithm      */
#define CRC_STANDARD_POLY       0x04C11DB7           /* Standard polynomial for 32 bit CRC  */

#define CRC_ALGO_SEL            (0xF << 3)           /* To clear algorithm select   */
#define CRC_ALGO_SIZE           (0X3 << 1)           /* To clear the algorithm size */

typedef struct _crc_transfer_t {
    const void *data_in;     /**< Pointer to Input buffer                    */
    uint32_t   len;          /**< Total length of Input buffer               */
    uint32_t   *data_out;    /**< Pointer to Output buffer                   */
    uint32_t   aligned_len;   /**< Aligned length                            */
    uint32_t   unaligned_len; /**< Unaligned length                          */
} crc_transfer_t;

/**
 @fn           crc_enable(CRC_Type *crc )
 @brief        Load CRC Init value in CRC Control register
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_enable(CRC_Type *crc)
{
    crc->CRC_CONTROL |= (CRC_INIT_BIT);
}

/**
 @fn           crc_clear_config(CRC_Type *crc )
 @brief        Clear the CRC configuration.
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_clear_config(CRC_Type *crc)
{
    crc->CRC_CONTROL = 0U;
}

/**
 @fn           crc_clear_algo(CRC_Type *crc )
 @brief        Clear the CRC 8,16 and 32 bit algorithm.
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_clear_algo(CRC_Type *crc)
{
    crc->CRC_CONTROL &= ~(CRC_ALGO_SEL);
}

/**
 @fn           crc_clear_algo_size(CRC_Type *crc )
 @brief        Clear the CRC 8,16 and 32 bit algorithm size.
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_clear_algo_size(CRC_Type *crc)
{
    crc->CRC_CONTROL &= ~(CRC_ALGO_SIZE);
}

/**
 @fn           crc_enable_8bit(CRC_Type *crc )
 @brief        Enable 8 bit CRC algorithm and size.
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_enable_8bit(CRC_Type *crc)
{
    /* To enable 8 bit CRC */
    crc->CRC_CONTROL |= CRC_8_CCITT;

    /* To enable the 8 bit algorithm size */
    crc->CRC_CONTROL |= CRC_ALGO_8_BIT_SIZE;
}

/**
 @fn           crc_enable_16bit(CRC_Type *crc )
 @brief        Enable 16 bit CRC algorithm and size.
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_enable_16bit(CRC_Type *crc)
{
    /* To enable 16 bit CRC */
    crc->CRC_CONTROL |= CRC_16;

    /* To enable 16 bit algorithm size */
    crc->CRC_CONTROL |= CRC_ALGO_16_BIT_SIZE;
}

/**
 @fn           crc_enable_16bit_ccitt(CRC_Type *crc )
 @brief        Enable 16 bit CCITT CRC algorithm and size.
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_enable_16bit_ccitt(CRC_Type *crc)
{
    /* To enable 16 bit CCITT CRC */
    crc->CRC_CONTROL |= CRC_16_CCITT;

    /* To enable 16 bit algorithm size */
    crc->CRC_CONTROL |= CRC_ALGO_16_BIT_SIZE;
}

/**
 @fn           crc_enable_32bit(CRC_Type *crc )
 @brief        Enable 32 bit CRC algorithm and size.
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_enable_32bit(CRC_Type *crc)
{
    /* To enable 32 bit CRC */
    crc->CRC_CONTROL |= CRC_32;

    /* To enable 32 bit algorithm size */
    crc->CRC_CONTROL |= CRC_ALGO_32_BIT_SIZE;
}

/**
 @fn           crc_enable_32bit_custom_poly(CRC_Type *crc )
 @brief        Enable 32 bit CRC customize polynomial algorithm and size.
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_enable_32bit_custom_poly(CRC_Type *crc)
{
    /* To enable 32 bit poly custom CRC */
    crc->CRC_CONTROL |= CRC_32C;

    /* To enable 32 bit algorithm size */
    crc->CRC_CONTROL |= CRC_ALGO_32_BIT_SIZE;
}

/**
 @fn           crc_get_algorithm_size(CRC_Type *crc )
 @brief        Get the CRC algorithm size.
 @param[in]    crc    : Pointer to the CRC register map
 @return       CRC algorithm size
 */
static inline uint32_t crc_get_algorithm_size(CRC_Type *crc)
{
    return ((crc->CRC_CONTROL & (CRC_ALGORITHM_CHECK)) >> 1);
}

/**
 @fn           crc_get_custom_poly(CRC_Type *crc )
 @brief        Get the CRC custom polynomial.
 @param[in]    crc    : Pointer to the CRC register map
 @return       CRC custom polynomial
 */
static inline uint32_t crc_get_custom_poly(CRC_Type *crc)
{
   return crc->CRC_POLY_CUSTOM;
}

/**
 @fn           crc_get_control_val(CRC_Type *crc )
 @brief        Get the CRC control register value.
 @param[in]    crc    : Pointer to the CRC register map
 @return       CRC control register value
 */
static inline uint32_t crc_get_control_val(CRC_Type *crc)
{
   return crc->CRC_CONTROL;
}

/**
 @fn           crc_set_custom_poly(CRC_Type *crc, uint32_t value)
 @brief        Add Polynomial value to the poly_custom register of CRC.
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_set_custom_poly(CRC_Type *crc, uint32_t value)
{
    crc->CRC_POLY_CUSTOM = value;
}

/**
 @fn           crc_custom_poly_enabled(CRC_Type *crc)
 @brief        Check crc custom polynomial bit is enable or disable.
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline bool crc_custom_poly_enabled(CRC_Type *crc)
{
    return (crc->CRC_CONTROL & CRC_CUSTOM_POLY) ? true : false;
}

/**
 @fn           crc_set_seed(CRC_Type *crc, uint32_t seed_value)
 @brief        Add 8 or 16 or 32 bit seed value to the Seed register of CRC.
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_set_seed(CRC_Type *crc, uint32_t seed_value)
{
    crc->CRC_SEED = seed_value;
}

/**
 @fn           crc_enable_byte_swap(CRC_Type *crc)
 @brief        Enable the CRC byte swap
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_enable_byte_swap(CRC_Type *crc)
{
    crc->CRC_CONTROL |= CRC_BYTE_SWAP;
}

/**
 @fn           crc_disable_byte_swap(CRC_Type *crc)
 @brief        Disable the CRC byte swap
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_disable_byte_swap(CRC_Type *crc)
{
    crc->CRC_CONTROL &= ~(CRC_BYTE_SWAP);
}

/**
 @fn           crc_enable_bit_swap(CRC_Type *crc)
 @brief        Enable the CRC bit swap
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_enable_bit_swap(CRC_Type *crc)
{
    crc->CRC_CONTROL |= CRC_BIT_SWAP;
}

/**
 @fn           crc_disable_bit_swap(CRC_Type *crc)
 @brief        Disable the CRC bit swap
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_disable_bit_swap(CRC_Type *crc)
{
    crc->CRC_CONTROL &= ~(CRC_BIT_SWAP);
}

/**
 @fn           crc_enable_custom_poly(CRC_Type *crc)
 @brief        Enable the CRC Customize polynomial bit
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_enable_custom_poly(CRC_Type *crc)
{
    crc->CRC_CONTROL |= CRC_CUSTOM_POLY;
}

/**
 @fn           crc_disable_custom_poly(CRC_Type *crc)
 @brief        Disable the CRC Customize polynomial bit
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_disable_custom_poly(CRC_Type *crc)
{
    crc->CRC_CONTROL &= ~(CRC_CUSTOM_POLY);
}

/**
 @fn           crc_enable_invert(CRC_Type *crc)
 @brief        Enable the CRC Invert bit
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_enable_invert(CRC_Type *crc)
{
    crc->CRC_CONTROL |= CRC_INVERT;
}

/**
 @fn           crc_disable_invert(CRC_Type *crc)
 @brief        Disable the CRC Invert bit
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_disable_invert(CRC_Type *crc)
{
    crc->CRC_CONTROL &= ~(CRC_INVERT);
}

/**
 @fn           crc_enable_reflect(CRC_Type *crc)
 @brief        Enable the CRC reflect bit
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_enable_reflect(CRC_Type *crc)
{
    crc->CRC_CONTROL |= CRC_REFLECT;
}

/**
 @fn           crc_disable_reflect(CRC_Type *crc)
 @brief        Disable the CRC reflect bit
 @param[in]    crc    : Pointer to the CRC register map
 @return       none
 */
static inline void crc_disable_reflect(CRC_Type *crc)
{
    crc->CRC_CONTROL &= ~(CRC_REFLECT);
}

/**
  \fn          void* crc_get_8bit_datain_addr(CRC_Type *crc)
  \brief       Return the 8bit data in Address
  \param[in]   crc   Pointer to CRC register map
  \return      \ref  Return the address
*/
static inline void* crc_get_8bit_datain_addr(CRC_Type *crc)
{
    return ((uint8_t *)crc + CRC_DATA_IN_8BIT_REG_OFFSET);
}

/**
  \fn          void* crc_get_32bit_datain_addr(CRC_Type *crc)
  \brief       Return the 32bit data in Address
  \param[in]   crc   Pointer to CRC register map
  \return      \ref  Return the address
*/
static inline void* crc_get_32bit_datain_addr(CRC_Type *crc)
{
    return ((uint8_t *)crc + CRC_DATA_IN_32BIT_REG_OFFSET);
}

/**
  \fn          uint32_t crc_read_output_value(CRC_Type *crc)
  \brief       Return the crc calculated output value
  \param[in]   crc   Pointer to CRC register map
  \return      \ref  Return the address
*/
static inline uint32_t crc_read_output_value(CRC_Type *crc)
{
    return (crc->CRC_OUT);
}

/**
 @fn           crc_bit_reflect(uint32_t input)
 @brief        Reflect the CRC 32 bit output
 @param[in]    input    : 32 bit CRC output
 @return       result of reflected CRC 32 bit output
 */
uint32_t crc_bit_reflect(uint32_t input);

/**
@fn         uint32_t CRC_calculate_unaligned(uint32_t key, const uint8_t *input,
                                             uint32_t length, uint32_t poly)
@brief      To calculate the CRC result for unaligned input data
@param[in]  key   : Output of aligned data for CRC from the hardware
@param[in]  input : unaligned input data
@param[in]  length: length of unaligned data
@param[in]  poly  : Standard polynimial or the user entered polynomial
                    depending upon the CRC algorithm
@return     Calculated CRC output for unaligned data
*/
uint32_t crc_calculate_unaligned(uint32_t key, const uint8_t *input,
                                 uint32_t length, uint32_t poly);

/**
 @fn           crc_calculate_8bit(CRC_Type *crc, const void *data_in,
                                  uint32_t len, uint32_t *data_out)
 @brief        Calculate the CRC output  for 8 bit CRC algorithm
 @param[in]    crc      : Pointer to the CRC register map
 @param[in]    data_in  : pointer which holds the address of CRC 8 bit input
 @param[in]    len      : Length of the input data
 @param[in]    data_out : 8 bit CRC output
 @return       None
 */
void crc_calculate_8bit(CRC_Type *crc, const void *data_in,
                        uint32_t len, uint32_t *data_out);

/**
 @fn           crc_calculate_16bit(CRC_Type *crc, const void *data_in,
                                   uint32_t len, uint32_t *data_out)
 @brief        Calculate the CRC output  for 16 bit CRC algorithm
 @param[in]    crc      : Pointer to the CRC register map
 @param[in]    data_in  : pointer which holds the address of CRC 16 bit input
 @param[in]    len      : Length of the input data
 @param[in]    data_out : 16 bit CRC output
 @return       None
 */
void crc_calculate_16bit(CRC_Type *crc, const void *data_in,
                         uint32_t len, uint32_t *data_out);

/**
 @fn           crc_calculate_32bit(CRC_Type *crc, const void *data_in,
                                   uint32_t len, uint32_t *data_out)
 @brief        Calculate the CRC output  for 32 bit CRC algorithm
 @param[in]    crc      : Pointer to the CRC register map
 @param[in]    data_in  : pointer which holds the address of CRC 32 bit input
 @param[in]    len      : Length of the input data
 @param[in]    data_out : 32 bit CRC output
 @return       None
 */
void crc_calculate_32bit(CRC_Type *crc, const void *data_in,
                         uint32_t len, uint32_t *data_out);

/**
 @fn           crc_calculate_32bit_unaligned_sw(CRC_Type *crc,
                                                  crc_transfer_t *transfer)
 @brief        Calculate the 32bit CRC output for the unaligned part
 @param[in]    crc      : Pointer to the CRC register map
 @param[in]    transfer : CRC transfer information
 @return       None
 */
void crc_calculate_32bit_unaligned_sw(CRC_Type *crc, crc_transfer_t *transfer);

#ifdef __cplusplus
}
#endif

#endif /* CRC_H_ */

