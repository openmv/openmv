/******************************************************************************/
/*                                                                            */
/*  Copyright (C) 2018, FLIR Systems                                          */
/*  All rights reserved.                                                      */
/*                                                                            */
/*  This document is controlled to FLIR Technology Level 2. The information   */
/*  contained in this document pertains to a dual use product controlled for  */
/*  export by the Export Administration Regulations (EAR). Diversion contrary */
/*  to US law is prohibited. US Department of Commerce authorization is not   */
/*  required prior to export or transfer to foreign persons or parties unless */
/*  otherwise prohibited.                                                     */
/*                                                                            */
/******************************************************************************/

#ifndef _TIMEOUT_LOGIC_H
#define _TIMEOUT_LOGIC_H
#include <sys/timespec.h>

#define CLOCK_MONOTONIC 0
int clock_gettime(int _ignore, struct timespec *spec);
double difftime(long current, long reference);

double diff_timespec(struct timespec *current, struct timespec *reference);

#endif //_TIMEOUT_LOGIC_H
