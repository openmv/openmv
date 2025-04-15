/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_memory.h (%version: 5 %)
 *          created Tue Jan 11 13:12:00 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Jan 04 10:37:31 2007 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_memory_h_H
#define __1_dave_memory_h_H
/*--------------------------------------------------------------------------- */

#include "dave_driver.h"

D2_EXTERN void *d2_getmem_p( d2_u32 size );
D2_EXTERN void *d2_reallocmem_p( d2_u32 newsize, void *oldadr, d2_s32 keep );
D2_EXTERN void d2_freemem_p( void *adr );

D2_EXTERN void *d2_getmem_d( const d2_device *handle, d2_u32 size );
D2_EXTERN void d2_freemem_d( const d2_device *handle, void *adr );

/*--------------------------------------------------------------------------- */
#endif
