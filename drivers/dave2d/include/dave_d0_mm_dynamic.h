//--------------------------------------------------------------------------
// Project: D/AVE
// File:    dave_d0_mm_dynamic.h (%version: 1 %)
//
// Description:
//  dynamic memory managment functions for dave.
//  %date_modified: Wed Jan 31 13:56:27 2007 %  (%derived_by:  hh74036 %)
//
// Changes:
//   2007-01-30 MGe start used old structure of d0_libs as base
//

#ifndef __DAVE_D0_MM_DYNAMIC_H_
#define __DAVE_D0_MM_DYNAMIC_H_

extern void * d0_dyn_allocmem( unsigned int size );
extern void d0_dyn_freemem( void *ptr );
extern unsigned int d0_dyn_memsize( void *ptr );

#endif

