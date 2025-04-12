/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef HWSEM_H_
#define HWSEM_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

typedef struct {                                     /*!< (@ HWSEM Structure                                                       */
    volatile uint32_t  HWSEM_REQ_REG;                /*!< (@ 0x00000000) Request register                                           */
    volatile uint32_t  HWSEM_REL_REG;                /*!< (@ 0x00000004) Release register                                           */
    volatile uint32_t  HWSEM_RST_REG;                /*!< (@ 0x00000008) Reset register                                             */
} HWSEM_Type;

/**
  \fn          static inline uint32_t hwsem_request(HWSEM_Type *hwsem, uint32_t master_id)
  \brief       Request ownership of a hwsem instance.
  \param[in]   hwsem     Pointer to the HWSEM register map
  \param[in]   master_id The master id to be used to request the ownership
  \return      Current master id. Will be same as the parameter master_id if the request was successful.
*/
static inline uint32_t hwsem_request(HWSEM_Type *hwsem, uint32_t master_id)
{
    /* Write the master_id to the request register */
    hwsem->HWSEM_REQ_REG = master_id;

    /* Read back and return the value */
    return hwsem->HWSEM_REQ_REG;
}

/**
  \fn          static inline uint32_t hwsem_getcount(HWSEM_Type *hwsem)
  \brief       Get the count for a hwsem instance.
  \param[in]   hwsem     Pointer to the HWSEM register map
  \return      HWSEM count
*/
static inline uint32_t hwsem_getcount(HWSEM_Type *hwsem)
{
    return hwsem->HWSEM_REL_REG;
}

/**
  \fn          static inline void hwsem_release(HWSEM_Type *hwsem, uint32_t master_id)
  \brief       Release ownership of a hwsem instance.
  \param[in]   hwsem     Pointer to the HWSEM register map
  \param[in]   master_id The master id to be used to release ownership
  \return      none
*/
static inline void hwsem_release(HWSEM_Type *hwsem, uint32_t master_id)
{
    hwsem->HWSEM_REL_REG = master_id;
}

/**
  \fn          static inline void hwsem_reset(HWSEM_Type *hwsem)
  \brief       Reset a hwsem instance.
  \param[in]   hwsem     Pointer to the HWSEM register map
  \return      none
*/
static inline void hwsem_reset(HWSEM_Type *hwsem)
{
    hwsem->HWSEM_RST_REG = 0x1U;
}

#ifdef __cplusplus
}
#endif
#endif /* HWSEM_H_ */
