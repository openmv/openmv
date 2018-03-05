/*
 * Copyright (C) 2010-2018 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* ----------------------------------------------------------------------
 * Project:      CMSIS NN Library
 * Title:        arm_softmax_q15.c
 * Description:  Q15 softmax function
 *
 * $Date:        17. January 2018
 * $Revision:    V.1.0.0
 *
 * Target Processor:  Cortex-M cores
 *
 * -------------------------------------------------------------------- */

#include "arm_math.h"
#include "arm_nnfunctions.h"

/**
 *  @ingroup groupNN
 */

/**
 * @addtogroup Softmax
 * @{
 */

  /**
   * @brief Q15 softmax function
   * @param[in]       vec_in      pointer to input vector
   * @param[in]       dim_vec     input vector dimention
   * @param[out]      p_out       pointer to output vector
   * @return none.
   *
   * @details
   *
   *  Here, instead of typical e based softmax, we use
   *  2-based softmax, i.e.,:
   *
   *  y_i = 2^(x_i) / sum(2^x_j)
   *
   *  The relative output will be different here.
   *  But mathematically, the gradient will be the same
   *  with a log(2) scaling factor.
   *
   */

void arm_softmax_q15(const q15_t * vec_in, const uint16_t dim_vec, q15_t * p_out)
{
    q31_t     sum;
    int16_t   i;
    q31_t     min, max;
    max = -1 * 0x100000;
    min = 0x100000;
    for (i = 0; i < dim_vec; i++)
    {
        if (vec_in[i] > max)
        {
            max = vec_in[i];
        }
        if (vec_in[i] < min)
        {
            min = vec_in[i];
        }
    }

    /* we ignore really small values  
     * anyway, they will be 0 after shrinking
     * to q7_t
     */
    if (max - min > 16)
    {
        min = max - 16;
    }

    sum = 0;

    for (i = 0; i < dim_vec; i++)
    {
        sum += 0x1 << (vec_in[i] - min);
    }

    for (i = 0; i < dim_vec; i++)
    {
        /* we leave 7-bit dynamic range, so that 128 -> 100% confidence */
        p_out[i] = (q15_t) __SSAT(((0x1 << (vec_in[i] - min + 14)) / sum), 16);
    }

}

/**
 * @} end of Softmax group
 */
