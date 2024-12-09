/******************************************************************************/
/*                                                                            */
/*  Copyright (C) 2017, FLIR Systems                                          */
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

#include "timeoutLogic.h"

#include "py/mphal.h"

int clock_gettime(int _ignore, struct timespec *spec)
{
    uint32_t time_ms = mp_hal_ticks_ms();
    spec->tv_sec  = time_ms / 1000L;
    spec->tv_nsec = time_ms % 1000L * 1000000;
    return 0;
}

double difftime(long current, long reference)
{
    return ((double)current - (double)reference);
}

double diff_timespec(struct timespec *current, struct timespec *reference)
{
    double elapsed_sec = difftime(current->tv_sec, reference->tv_sec);
    elapsed_sec += (((double)current->tv_nsec) - ((double)reference->tv_nsec))/1000000000;
    return elapsed_sec;
}
