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
 * @file     tty_user.c
 * @author   Raj Ranjan
 * @email    raj.ranjan@alifsemi.com
 * @version  V1.0.0
 * @date     24-Aug-2023
 * @brief    TTY User Template
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#if defined(RTE_Compiler_IO_TTY)
#include "retarget_tty.h"
#endif  /* RTE_Compiler_IO_TTY */

/**
  Put a character to the teletypewritter

  \param[in]   ch  Character to output
*/
void _ttywrch(int ch){
    (void)ch;
}