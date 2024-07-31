//--------------------------------------------------------------------------
// Project: D/AVE
// File:    dave_d0_mm_fixed_range.h (%version: 1 %)
//
// Description:
//  Memory management in a fixed range of memory
//  %date_modified: Wed Jan 31 13:56:41 2007 %  (%derived_by:  hh74036 %)
//
// Changes:
//   2006-11-21 MGe start
//

#ifndef __DAVE_D0_MM_FIXED_RANGE_H
#define __DAVE_D0_MM_FIXED_RANGE_H
#ifdef __cplusplus
extern "C" {	
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

//---------------------------------------------------------------------------
extern void * d0_fixed_range_heapalloc(void *ctrlblk, unsigned int size);
extern unsigned int d0_fixed_range_heapfree(void *ctrlblk, void *ptr);
extern unsigned int d0_fixed_range_heapmsize(void *ctrlblk, void *ptr);
extern void d0_fixed_range_setheapmem( void *base, unsigned int size );
//---------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
