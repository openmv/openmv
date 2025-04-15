/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "cmp.h"

/**
  @fn          void cmp_irq_handler(CMP_Type *cmp)
  @brief       Clear the interrupt status
  @param[in]   cmp    Pointer to the CMP register map
  @return      none
*/
void cmp_irq_handler(CMP_Type *cmp)
{
    /* clear the interrupt before re-starting */
    if(cmp->CMP_INTERRUPT_STATUS == 1)
        cmp->CMP_INTERRUPT_STATUS = CMP_INTERRUPT_CLEAR;
}
