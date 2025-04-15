//--------------------------------------------------------------------------
// Project: D/AVE
// File:    dave_d0_mm_fixed_range_fixed_blkcnt.h (%version: 1 %)
//
// Description:
//  memory management with controlstructure in a separate addressspace
//  %date_modified: Wed Jan 31 13:56:57 2007 %  (%derived_by:  hh74036 %)
//
// Changes:
//   2006-11-21 CSe start
//   2006-01-31 MGe redesign

#ifndef __DAVE_D0_MM_FIXED_RANGE_FIXED_BLK_H_
#define __DAVE_D0_MM_FIXED_RANGE_FIXED_BLK_H_
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

typedef struct _d0_fixed_range_fixed_blk_memblock {
  void*        addr;
  unsigned int size;
} d0_fixed_range_fixed_blk_memblock;

typedef struct _d0_fixed_range_fixed_blk_heap {
  d0_fixed_range_fixed_blk_memblock *freeblocks;
  d0_fixed_range_fixed_blk_memblock *usedblocks;
  unsigned int maxidx;  // size of both arrays -1
} d0_fixed_range_fixed_blkcnt_heap;


//---------------------------------------------------------------------------

extern void d0_fixed_range_fixed_blkcnt_heapinit( void* ctrlblk, void* heapaddr, unsigned int heapsize, unsigned int maxblocks);
extern void * d0_fixed_range_fixed_blkcnt_heapalloc(void* ctrlblk, unsigned int size);
extern unsigned int d0_fixed_range_fixed_blkcnt_heapfree(void* ctrlblk, void *ptr);
extern unsigned int d0_fixed_range_fixed_blkcnt_heapmsize(void* ctrlblk, void *ptr);

//---------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
