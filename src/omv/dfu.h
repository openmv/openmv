/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * USB debug support.
 *
 */

#ifndef __DFU_H
#define __DFU_H

// TODO: include this in the Reset_Handler
#define MAGIC_NUMBER 0xDEADBEEF

void reset_to_dfu(void);

#endif
