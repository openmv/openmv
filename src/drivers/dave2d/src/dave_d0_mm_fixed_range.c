//--------------------------------------------------------------------------
// Project: D/AVE
// File:    dave_d0_mm_fixed_range.c (%version: 3 %)
//
// Description:
//  memory managment functions for dave.
//  first fit in a fixed range.
//  %date_modified: Tue Aug 28 10:39:19 2007 %  (%derived_by:  hh74036 %)
//
// Changes:
//   2006-11-21 CSe start
//   2007-01-24 MGe usedd as base for a local heapmanger without block lists
//   2007-08-28 MGe fixed comments, fixed a minor bug in setheapmem, maintains
//              testcode
#include "dave_d0_mm_fixed_range.h"
#if (!defined(_NO_ASSERT) && !defined(__CA850__))
      #include "assert.h"
#else
   #define assert(value)
#endif


/******************************************************************************
 * memory allocater :
 *  - strategy: first fit
 *  - alignment : 4 bytes
 *  - memory status is stored in the memory blocks
 *     B  : address of memory block
 *     B0 : ptr to the last block if the ptr is null it means that this block is free
 *     B1 : size of the usable memory block
 *     B2-Bn : usable Memory Block
 *
 *  - If a block is freed it will be merged with next and previous free blocks
 *
 *  - each functions works on the structure d0_heap that will be passed as first
 *    parameter
 *
 *  - the function d0_setheapmem must be used to initialize a heap
 *
 *  - a free blk is marked with lastblk = NULL
 *
 *****************************************************************************/


typedef struct {
  void *base;
  void *end;  /* last valid address in heap*/
} d0_heap;

typedef struct _memblk {
  struct _memblk *lastblk;
  unsigned int size;
} memblk;

/* the minimalsize of a datablock in bytes*/
#define MINIMUM_BLOCKSIZE 4

/******************************************************************************
 * sets the memory range that shall be used as heap
 * param:
 *  base - pointer to the memory block that shall be used as heapmemory
 *  size - size of the memory block in bytes
 *****************************************************************************/
void d0_fixed_range_setheapmem( void *base, unsigned int size )
{
  /* create heap struct as header of memory alloc */
  d0_heap* heap  = (d0_heap*)base;
  /* test alignment and minimal size*/
  assert( ! (((unsigned int)base | size) & 3) );
  assert( size > ( sizeof(memblk) + sizeof(d0_heap)));
  heap->base = heap + 1;
  heap->end  = (char*)(heap->base) + size - 1 ;
  ((memblk*)(heap->base))->lastblk = NULL;
  ((memblk*)(heap->base))->size = size - sizeof(memblk);
}

/******************************************************************************
 * purpose: allocates a memory block
 * param:
 *  ctrlblk - startaddress of the heap
 *  size    - size of the memory block that shall be allocated
 * returns:
 *  ptr to the allocated memory block
 *****************************************************************************/
void * d0_fixed_range_heapalloc(void *ctrlblk, unsigned int size)
{
  /* create heap struct as header of memory alloc */
  d0_heap* heap  = (d0_heap*)ctrlblk;

  memblk *runptr        = heap->base;
  memblk *blk_before    = (memblk*)1;
  memblk *newfragment   = NULL;

  assert( size );
  /*align size to 4 bytes*/
  size = ( size + 3 ) & ~3;
  /* iterate heap range until a fitting block is found or end of heap is reached*/
  do {
    /* see if block is free and fits */
    if ( ( ! runptr->lastblk )  && ( runptr->size >= ( size + sizeof(memblk) ) ) )
    {
      /* write information for next block, if we produce a new fragment, which has as least MINIMUM_BLOCKSIZE Bytes usable size*/
      if ( ( runptr->size - size - sizeof(memblk) ) >  MINIMUM_BLOCKSIZE )
      {
        /* calc address of the resulting fragment  */
        newfragment = (memblk*)((char*)runptr + size + sizeof(memblk));
        /* calc size for the resulting fragment  */
        newfragment->size = runptr->size - size - sizeof(memblk);
        newfragment->lastblk = NULL;

        /* correct last block ptr of old next ptr */
        if ( (void*)( (char*)newfragment + newfragment->size + sizeof(memblk)  ) <= heap->end )
        {
          ((memblk*)( (char*)newfragment + newfragment->size + sizeof(memblk) ))->lastblk = newfragment;
        }

      } else {
        /* enlarge block, so that we don't have unusable small blks  */
        size = runptr->size;
      }
      /* write informat for found block*/
      runptr->size = size;
      runptr->lastblk = blk_before;

      return (void*)((unsigned int)runptr + sizeof(memblk));
    }
    /* proceed to next element*/
    blk_before = runptr;

    runptr = (memblk*)((unsigned int)runptr + runptr->size + sizeof(memblk));
  } while( (void*)runptr  <= (heap->end) );
  return NULL;
}

/******************************************************************************
 * purpose: frees a allocated memory block
 * params:
 *  ctrlblk - address of the heap
 *  ptr     - address of the memory block that shall be freed
 * returns: 1 on success
 *****************************************************************************/
unsigned int d0_fixed_range_heapfree(void *ctrlblk, void *ptr )
{
  /* create heap struct as header of memory alloc */
  d0_heap* heap  = (d0_heap*)ctrlblk;

  // fetch runptr, last_blk and next_blk
  memblk *runptr     = ((memblk*)ptr) - 1;
  memblk *last_blk   = runptr->lastblk;
  memblk *next_blk   = (memblk*)((unsigned int)runptr + runptr->size + sizeof( memblk ));

   // set runptr to free
  runptr->lastblk = NULL;
  // now merge with last blk if inside of the heap
  if ( (void*)last_blk >= heap->base )
  {
    if ( ! last_blk->lastblk ) // last blk is a free block, so we have to merge
    {
      last_blk->size += runptr->size + sizeof( memblk );
      /* overwrite old infos from blk */
      runptr->size = 0;
      runptr->lastblk = 0;
      /* adjust lastblk from next blk merge now with next blk if possible */
      runptr = last_blk;
    }
  }

  // see if we have to merge with the next_blk
  if ( ( ! next_blk->lastblk ) && ( (void*)next_blk  <= (heap->end) ) )
  {
    runptr->size += next_blk->size + sizeof( memblk );
    next_blk = (memblk*)((unsigned int)next_blk + next_blk->size + sizeof( memblk ));
  }

  // check if next block is still inside of the heap,
  // so we have to adjust the lastblk pointer from the next_blk
  if ( (void*)next_blk  <= (heap->end) )
  {
    next_blk->lastblk = runptr;
  }

  return 1;
}
/******************************************************************************
 * purpose: returns the size of the current memoryblock
 * params :
 *  unused - not used
 *  ptr   - ptr to the current memoryblock
 * returns: size of the currentmemory block in bytes
 *****************************************************************************/
unsigned int d0_fixed_range_heapmsize(void *unused, void *ptr)
{
  (void*)unused;
  return ((memblk*)ptr-1)->size;
}


#ifdef _TEST_HEAPMANAGER
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************
 * purpose: test a heap with the memoryallocater
 * params:
 *  base  - startaddress for the heap
 *  size  - size of the heap
 * returns: 0 on failure
 *          1 on success
 *****************************************************************************/
int checkHeap( void* ctrlblk, unsigned int size ) {
  /* create heap struct as header of memory alloc */
  d0_heap* heap  = (d0_heap*)ctrlblk;

  int counted_size = 0;
  memblk* runptr=heap->base;
  memblk* last=0;
  int blkindex=1;
  while ( (void*)runptr <= heap->end ) {
    counted_size += runptr->size + sizeof(memblk);
    if ( ! ( runptr->size ) ) {
      printf("illegal block size 0\n");
    }

    // check that backward reference is consistent
    if ( last && runptr->lastblk) {
      if ( !(runptr->lastblk == last) ) {
        printf("link is broken\n");
      }
    }
    // next blk
    last = runptr;
    runptr = (memblk*)( (char*)runptr + sizeof(memblk) + runptr->size );
    ++blkindex;
    // check size
    if ( ( counted_size ) != ((int)runptr - (int)(heap->base) ) )
    {
      printf("error\n");
      //assert(( counted_size ) == ((int)runptr - (int)(heap->base) ));
    }
  }
  // check overall size
  if ( size != counted_size )
  {
    printf(" size should be: 0x%x, is: 0x%x\n",size, counted_size );
    assert( size == counted_size );
  }
}

/******************************************************************************
 * purpose: test a heap with the memoryallocater
 * params:
 *  base  - startaddress for the heap
 *  size  - size of the heap
 * returns: 0 on failure
 *          1 on success
 *****************************************************************************/
int testheap ( void* base, unsigned int size )
{
  int i, j, testcnt_overall = 50000;
  void* ptr[100];
  int blk;
  int sizeofblk;
  if ( base ) {
    d0_fixed_range_setheapmem( base, size );
    // init ptr array
    for ( i=0; i < 100; i++ ) ptr[i]=NULL;
    // do random mallocs and random frees
    for ( i = 0 ;  i < testcnt_overall ; i++ )
    {
      // random mallocs
      for ( j = 0 ; j < 100; j++ )
      {
        if ( ptr[j] ) continue;
        if ( ! ( rand() % 2 ) )
        {
          do {
            //sizeofblk = ((rand() % (size+10))+3)&~3;
            sizeofblk = rand();//((rand() % (size+10))+3)&~3;
          } while ( sizeofblk == 0);
          ptr[j] = d0_fixed_range_heapalloc( base, sizeofblk );
          if ( ptr[j] ) 
            checkHeap( base, size );
        }
      }
      // random frees
      for ( j = 0 ; j < 100; j++ )
      {
        if ( ! ptr[j] ) continue;
        if ( ! ( rand() % 4 ) )
        {
          if ( d0_fixed_range_heapfree( base, ptr[j] ) )
          {
            ptr[j] = NULL;
            checkHeap( base, size );
          }
         }
      }
    }
  } else {
    return 1;
  }
  for ( j = 0 ; j < 100; j++ )
  {
      if ( ! ptr[j] ) continue;
        if ( d0_fixed_range_heapfree( base, ptr[j] ) ) ptr[j] = NULL;
  }
  // check if the size of the heap is still the same
  unsigned int mbsize = ((memblk*)(((d0_heap*)base)->base))->size;
  if ( mbsize != (size-sizeof(memblk)) ) 
  {
    printf( "size(0x%x) of memoryblock does not equals the initial size (0x%x)\n",
        mbsize, size );
    return 0;
  }
  //return ( ((memblk*)base)->size == (size-sizeof(memblk)) ) ? 1 : 0;
  return 1;

}
/******************************************************************************
 * purpose: test the memoryallocater
 * params:
 *  param1
 * returns: 0 on failure
 *          1 on success
 *****************************************************************************/
int testheapmanager( )
{
  int i = 100;
  int base, size;
  void* blk;
  // test several heaps
  for ( i=0 ; i < 100; i++ ) {
    size = rand() & ~3;
    //size=64;
    blk = malloc( size );
    if ( blk && (size >= 8)) {
      if ( testheap(  blk, size ) ) {
        printf("test %d %d success\n", blk, size);
      } else {
        printf("test %d %d fails\n", blk, size);
      };
    } else {
      //printf("warning: can't get memory block from malloc\n");
    }
    free(blk);
  }
}

int main() {
  testheapmanager();
}
#endif

