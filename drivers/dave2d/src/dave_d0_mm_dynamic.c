//--------------------------------------------------------------------------
// Project: D/AVE
// File:    dave_d0_mm_dynamic.c (%version: 2 %)
//
// Description:
//  dynamic memory managment functions for dave.
//  %date_modified: Mon Feb 05 18:59:57 2007 %  (%derived_by:  hh74036 %)
//
// Changes:
//   2007-01-30 MGe start used old structure of d0_libs as base
//
#ifdef WITH_MM_DYNAMIC

#include "dave_d0_mm_dynamic.h"
#include <stdlib.h>

#ifdef NO_MSIZE
  #define MSIZE(PTR) *(((unsigned int*)(PTR))-1)
#else
  #include <malloc.h>
  #define MSIZE(PTR) _msize(PTR)
#endif

static unsigned int g_memsum = 0;
static unsigned int g_numblocks = 0;

//--------------------------------------------------------------------------
// function_:d0_dyn_allocmem
// Allocate memory from local heap.
//
// parameters:
//  size - size in bytes of requested memory
//
// returns:
//  pointer to memory, NULL if allocation failed
//
// see also:
//  <d0_freemem>
//
void * d0_dyn_allocmem( unsigned int size )
{
  unsigned int *ptr;

  g_memsum += size;
  ++g_numblocks;

#ifdef NO_MSIZE  
  ptr = malloc( (size_t)size + sizeof(unsigned int) );
  if (ptr) {
    *ptr = size;
    ++ptr;
  }
#else
  ptr = malloc( (size_t)size );
#endif
 
  return (void*) ptr;
}


//--------------------------------------------------------------------------
// function_: d0_dyn_freemem
// Free memory from local heap.
//
// parameters:
//  ptr - ptr to previously allocated memory
//
// see also:
//  <d0_allocmem>
//
void d0_dyn_freemem( void *ptr )
{
  if (ptr) {
    g_memsum -= MSIZE(ptr);
    --g_numblocks;
#ifdef NO_MSIZE
    free( ((unsigned int*)ptr)-1 );
#else
    free(ptr);
#endif
  }
}

//--------------------------------------------------------------------------
// function_: d0_dyn_memsize
// Return the size of a memory chunk that was allocated using <d0_allocmem>.
//
// parameters:
//  ptr - ptr to allocated memory
//
// returns:
//  size of block, 0 if an error occurred
//
// see also:
//  <d0_allocmem>
//
unsigned int d0_dyn_memsize( void *ptr )
{
  if (!ptr) return 0;
  return MSIZE(ptr);
}

#endif
