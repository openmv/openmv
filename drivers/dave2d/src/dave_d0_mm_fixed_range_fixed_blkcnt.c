//--------------------------------------------------------------------------
// Project: D/AVE
// File:    dave_d0_mm_fixed_range_fixed_blkcnt.c (%version: 2 %)
//
// Description:
//  memory management with controlstructure in a separate addressspace
//  %date_modified: Wed Feb 21 17:16:29 2007 %  (%derived_by:  hh74036 %)
//
// Changes:
//   2006-11-21 CSe: start
//   2007-01-31 MGe: redesign
//   2007-05-24 MGe: bugfix:added initialization of size field in blocklists
//   2009-09-23 SSt: bugfix in 3-block merge, added comments

#include "dave_d0_mm_fixed_range_fixed_blkcnt.h"
#if (!defined(_NO_ASSERT) && !defined(__CA850__))
      #include "assert.h"
#else
   #define assert(value)
#endif

//------------------------------------------------------------------------------------------------------------
//* Initialize the fixed block count heap.                                                                   *
//*                                                                                                          *
//* A fixed block count heap can manage up to maxblocks individual memory regions (mallloc).                 *
//*                                                                                                          *                
//* Memory information (allocations etc.) is stored inside a control block.                                  *
//* The control block stores the following information:                                                      *
//* - pointer to a free_list                                                                                 *
//* - pointer to a used_list                                                                                 *
//* - maximum number of blocks -1                                                                            *
//* - the free list (maxblocks entries of pointer and size)                                                  *
//* - the used list (maxblocks entries of pointer and size)                                                  *
//*                                                                                                          *
//* When calling this function ctrlblk must be big enough to store all this information.                     *                                                                                                         *
//*                                                                                                          *
//* Parameters:                                                                                              *
//*  ctrlblk :   pointer to the heap contol block.                                                           *
//*  heapaddr :  base address of the heap.                                                                   *
//*  heapsize :  size of the heap.                                                                           *
//*  maxblocks : maximum number of blocks.                                                                   *
//*                                                                                                          *
void d0_fixed_range_fixed_blkcnt_heapinit( void* ctrlblk, void* heapaddr, unsigned int heapsize, unsigned int maxblocks)
{
  unsigned int i;
  unsigned int alignedaddr;
  /* create heap structure */
  d0_fixed_range_fixed_blkcnt_heap* heap = (d0_fixed_range_fixed_blkcnt_heap*)ctrlblk;
  heap->freeblocks  = (d0_fixed_range_fixed_blk_memblock *)((char*)ctrlblk + sizeof( d0_fixed_range_fixed_blkcnt_heap ));
  heap->usedblocks  = (d0_fixed_range_fixed_blk_memblock *)((char*)heap->freeblocks + maxblocks*sizeof( d0_fixed_range_fixed_blk_memblock ));
  heap->maxidx      = maxblocks; 
  
  for (i=0;i<maxblocks;++i)
  {
    heap->freeblocks[i].addr = NULL;
    heap->freeblocks[i].size = 0;
    heap->usedblocks[i].addr = NULL;
    heap->usedblocks[i].size = 0;
  }
  /*align start of heap to 16 32Bit Words*/      
  alignedaddr = ( (unsigned int)heapaddr + 63 ) & ~63;
  heapsize -= alignedaddr - (unsigned int)heapaddr;
  
  heap->freeblocks[0].addr = (void*)alignedaddr;
  heap->freeblocks[0].size = heapsize;
  heap->maxidx = maxblocks-1;
}


//------------------------------------------------------------------------------------------------------------
//* allocate memory                                                                                          *
//*                                                                                                          *
//* Tries to allocate size bytes of heap (belonging to ctrlblk)                                              *
//*                                                                                                          *
//* Parameters:                                                                                              *
//*  ctrlblk :  pointer to the heap contol block.                                                            *
//*  size:      requested size in bytes                                                                      *
//*                                                                                                          *
//* Return Value:                                                                                            *
//*  NULL in case of an error, a pointer to the reserved memory otherwise.                                   *
//*                                                                                                          *
void * d0_fixed_range_fixed_blkcnt_heapalloc(void* ctrlblk, unsigned int size)
{
  char *mem = NULL;    
  d0_fixed_range_fixed_blkcnt_heap* heap = (d0_fixed_range_fixed_blkcnt_heap*)ctrlblk;
  // find room in used list
  d0_fixed_range_fixed_blk_memblock *used_blk  = heap->usedblocks;
  d0_fixed_range_fixed_blk_memblock *last_used = used_blk + heap->maxidx;
  /* align newly allocated memoryblocks to a size of 16 32Bit Words */  
  size = ( size + 63 ) & ~63;
  while ((used_blk <= last_used) && (used_blk->addr))
    ++used_blk;

  if (used_blk <= last_used)
  {
    // find free block
    d0_fixed_range_fixed_blk_memblock *free_blk  = heap->freeblocks;
    d0_fixed_range_fixed_blk_memblock *last_free = free_blk + heap->maxidx;

    // find suitable block in free list
    while ((!mem) && (free_blk <= last_free))
    {
      if (free_blk->size >= size)
      {
        mem = free_blk->addr;
        free_blk->addr = (char*)(free_blk->addr)+size;
        free_blk->size -= size;
        used_blk->addr = mem;
        used_blk->size = size;
      }
      else
        ++free_blk;
    }
  }
  if (!mem)
    assert(mem);

  return (void*)mem;
}

//------------------------------------------------------------------------------------------------------------
//* free memory                                                                                              *
//*                                                                                                          *
//* frees previously reserved memory                                                                         *
//*                                                                                                          *
//* Parameters:                                                                                              *
//*  ctrlblk :  pointer to the heap contol block.                                                            *
//*  ptr:       pointer to the memory block                                                                  *
//*                                                                                                          *
//* Return Value:                                                                                            *
//*  0 in case of an error, the size of the freed memory block.                                              *
//*                                                                                                          *
unsigned int d0_fixed_range_fixed_blkcnt_heapfree(void *ctrlblk, void *ptr )
{
  d0_fixed_range_fixed_blkcnt_heap* heap = (d0_fixed_range_fixed_blkcnt_heap*)ctrlblk;
  // find ptr in used list
  d0_fixed_range_fixed_blk_memblock *used_blk  = heap->usedblocks;
  d0_fixed_range_fixed_blk_memblock *last_used = used_blk + heap->maxidx;
  while ((used_blk <= last_used) && (used_blk->addr != ptr))
    ++used_blk;
  
  assert(used_blk->addr == ptr);

  if (used_blk->addr == ptr)
  {
    // find free blocks, which can be merged with current block
    d0_fixed_range_fixed_blk_memblock *free_blk   = heap->freeblocks;
    d0_fixed_range_fixed_blk_memblock *last_free  = free_blk + heap->maxidx;
    d0_fixed_range_fixed_blk_memblock *pred_free  = NULL;  // direct predecessor
    d0_fixed_range_fixed_blk_memblock *succ_free  = NULL;  // direct successor
    d0_fixed_range_fixed_blk_memblock *empty_free = NULL;  // unused entry in free list

    // find empty block/possibility to merge in free list
    while ( (!(pred_free && succ_free)) && (free_blk <= last_free) )
    {
      if (free_blk->addr)
      {
        if ( (!pred_free) && (((char*)(free_blk->addr) + free_blk->size) == ptr) )
          pred_free = free_blk;

        if ( (!succ_free) && ((((char*)ptr) + used_blk->size) == free_blk->addr) )
          succ_free = free_blk;
      }
      else
        if (!empty_free)
          empty_free = free_blk;  // save first empty block
      ++free_blk;
    }

    // now performe merge (in front of successor if possible, because that is the direction,
    // from where we allocate)
    if (succ_free)
    {
      succ_free->addr  = ptr;
      succ_free->size += used_blk->size;
      if (pred_free)
      {
        // both blocks (before and after) are free -> merge all three blocks
        // this leads to an empty enty in the free list
        succ_free->addr  = pred_free->addr;
        succ_free->size += pred_free->size;

        pred_free->addr  = NULL; //  mark free entry as empty
        pred_free->size  = 0;    // any empty entry must have a size of 0
      }
    }
    else
    {
      if (pred_free)
        pred_free->size += used_blk->size;
      else
      {
        // no merge possible -> use an empty free entry
	      assert(empty_free);
	      if (empty_free)
	      {
	        empty_free->addr = ptr;
	        empty_free->size = used_blk->size;
	      }
      }
    }

    // free used list entry
    used_blk->addr = NULL;
    return used_blk->size;
  }
  return 0;
}

//------------------------------------------------------------------------------------------------------------
//* query block size memory                                                                                  *
//*                                                                                                          *
//* retrieve the size of a memory block previously reserved memory                                           *
//*                                                                                                          *
//* Parameters:                                                                                              *
//*  ctrlblk :  pointer to the heap contol block.                                                            *
//*  ptr:       pointer to the memory block                                                                  *
//*                                                                                                          *
//* Return Value:                                                                                            *
//*  0 in case of an error, the size of the memory block.                                                    *
//*                                                                                                          *
unsigned int d0_fixed_range_fixed_blkcnt_heapmsize(void *ctrlblk, void *ptr)
{
  d0_fixed_range_fixed_blkcnt_heap* heap = (d0_fixed_range_fixed_blkcnt_heap*)ctrlblk;
	// find ptr in used list
  d0_fixed_range_fixed_blk_memblock *used_blk  = heap->usedblocks;
  d0_fixed_range_fixed_blk_memblock *last_used = used_blk + heap->maxidx;
  while ((used_blk <= last_used) && (used_blk->addr != ptr))
    ++used_blk;
  
  // return 0 if pointer was not found, blocksize otherwise
  if (used_blk->addr != ptr) return 0;

	return used_blk->size;
}
