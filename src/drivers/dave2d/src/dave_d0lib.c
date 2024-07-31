/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_d0lib.c (%version: 7 %)
 *
 * Description:
 *  memory management
 *  %date_modified: Tue Feb 06 16:31:07 2007 %  (%derived_by:  hh74026 %)
 *
 * Changes:
 *   2006-11-21 CSe start
 *   2007-01-31 MGe Redesign
 *   2007-08-30 ASc fix d0_initheapmanager description, changed C++ to C comments 
 *   2007-09-10 ASc added error checking to d0_allocvidmem 
 *   2008-02-12 ASc removed compiler warnings
 *   2008-11-24 MRe removed compiler warnings 
 *-------------------------------------------------------------------------- */
#if (!defined(_NO_ASSERT) && !defined(__CA850__))
      #include "assert.h"
#else
   #define assert(value)
#endif
#include "dave_d0lib.h"

#if ! defined( WITH_MM_FIXED_RANGE_FIXED_BLKCNT ) && ! defined( WITH_MM_FIXED_RANGE ) && ! defined( WITH_MM_DYNAMIC )
  #ifdef WIN32
    #error "No memory manager defined! Please define: WITH_MM_FIXED_RANGE_FIXED_BLKCNT, WITH_MM_FIXED_RANGE or WITH_MM_DYNAMIC."
  #else
    #warning "No memory manager defined! Please define: WITH_MM_FIXED_RANGE_FIXED_BLKCNT, WITH_MM_FIXED_RANGE or WITH_MM_DYNAMIC."
    #define WITH_MM_DYNAMIC
  #endif
#endif

#ifdef WITH_MM_FIXED_RANGE_FIXED_BLKCNT
#include "dave_d0_mm_fixed_range_fixed_blkcnt.h"
#endif

#ifdef WITH_MM_FIXED_RANGE
#include "dave_d0_mm_fixed_range.h"
#endif

#ifdef WITH_MM_DYNAMIC
#include "dave_d0_mm_dynamic.h"
#endif

#ifdef WITH_MM_DYNAMIC
  /******************************************************************************
   * purpose: allocate a piece of memory in the heap
   * params:
   *  size  - size of the memory block that shall be allocated
   * returns: a ptr to new allocated the memory block or NULL if an error occured
   *****************************************************************************/
  void* d0_dyn_allocmem_wrapper( void* ctrlblk, unsigned int size ) {
    (void *) ctrlblk;
    return d0_dyn_allocmem( size );
  }

  /******************************************************************************
   * purpose: free a block of allocated memory
   * params:
   *  ptr - the startadress of the memory block we want to deallocate
   * returns:
   *****************************************************************************/
  void d0_dyn_freemem_wrapper( void* ctrlblk, void *ptr ) {
    (void *) ctrlblk;
    d0_dyn_freemem( ptr );
  }

  /******************************************************************************
   * purpose: returns the size of the given memory block
   * params:
   *  ptr - prt to the start of the memoryblock
   * returns: the size of the given memoryblock
   *****************************************************************************/
  unsigned int d0_dyn_memsize_wrapper( void* ctrlblk, void *ptr ) {
    (void *) ctrlblk;
    return d0_dyn_memsize( ptr );
  }
#endif

#ifdef WITH_MM_FIXED_RANGE
  /******************************************************************************
   * purpose: allocate a piece of memory in the heap
   * params:
   *  size  - size of the memory block that shall be allocated
   * returns: a ptr to new allocated the memory block or NULL if an error occured
   *****************************************************************************/
  void* d0_fixed_range_allocmen_wrapper( void* ctrlblk, unsigned int size ) {
    return d0_fixed_range_heapalloc( ctrlblk, size );
  }

  /******************************************************************************
   * purpose: free a block of allocated memory
   * params:
   *  ptr - the startadress of the memory block we want to deallocate
   * returns:
   *****************************************************************************/
  void d0_fixed_range_freemen_wrapper( void* ctrlblk, void *ptr ) {
    d0_fixed_range_heapfree( ctrlblk, ptr );
  }

  /******************************************************************************
   * purpose: returns the size of the given memory block
   * params:
   *  ptr - prt to the start of the memoryblock
   * returns: the size of the given memoryblock
   *****************************************************************************/
  unsigned int d0_fixed_range_memsize_wrapper( void* ctrlblk, void *ptr ) {
    return d0_fixed_range_heapmsize( ctrlblk, ptr );
  }
#endif

#ifdef WITH_MM_FIXED_RANGE_FIXED_BLKCNT
  /******************************************************************************
   * purpose: allocate a piece of memory in the heap
   * params:
   *  size  - size of the memory block that shall be allocated
   * returns: a ptr to new allocated the memory block or NULL if an error occured
   *****************************************************************************/
  void * d0_fixed_range_fixed_blkcnt_allocmem_wrapper( void* ctrlblk, unsigned int size )
  {
    void *ptr = NULL;

    ptr = d0_fixed_range_fixed_blkcnt_heapalloc( ctrlblk, size);

    assert(ptr);
    if (!ptr)
      return NULL;

    return ptr;
  }

  /******************************************************************************
   * purpose: free a block of allocated memory
   * params:
   *  ptr - the startadress of the memory block we want to deallocate
   * returns:
   *****************************************************************************/
  void d0_fixed_range_fixed_blkcnt_freemem_wrapper( void* ctrlblk, void *ptr )
  {
    d0_fixed_range_fixed_blkcnt_heapfree( ctrlblk, ptr);
  }

  /******************************************************************************
   * purpose: returns the size of the given memory block
   * params:
   *  ptr - prt to the start of the memoryblock
   * returns: the size of the given memoryblock
   *****************************************************************************/
  unsigned int d0_fixed_range_fixed_blkcnt_memsize_wrapper( void* ctrlblk, void *ptr )
  {
   if (!ptr) return 0;
   return d0_fixed_range_fixed_blkcnt_heapmsize( ctrlblk, ptr);
  }
#endif

/******************************************************************************
 * purpose: dummy functions for alloc, free and msize
 *****************************************************************************/
void* d0_dummy_alloc( void* ctrlblk,unsigned int size ) {  (void ) ctrlblk; (void) size; return NULL; }
void d0_dummy_free( void* ctrlblk,void * ptr) { (void ) ctrlblk; (void) ptr; }
unsigned int d0_dummy_memsize( void* ctrlblk,void *ptr){ (void ) ctrlblk; (void ) ptr; return 0;}

/******************************************************************************
 * purpose: initalize a heapmanager
 * params:
 *  memmanagertype  - defines which kind of memorymangment will be used
 *
 *  base            - baseaddress of the addressrange( not used for
 *                    d0_mm_dynamic memorymanager)
 *
 *  size            - size of the memory in bytes( not used for
 *                    d0_mm_dynamic memorymanager)
 *
 *  ctrlblk         - startaddress of memory block used for controlstructure
 *                    this block must have a size of:
 *                    2*blockcnt*sizeof( d0_fixed_range_fixed_blk_memblock )
 *                    +sizeof(d0_fixed_range_fixed_blkcnt_heap)
 *
 *  blockcnt        - amount of allocatable blocks in memoryrange( only used
 *                    for d0_mm_fixed_range_fixed_blkcnt memorymanager)
 *****************************************************************************/
void d0_preparememorymanager( d0_memmanager *mgr, d0_memmanager_type memmanagertype, void *base,
                          unsigned int size, void *ctrlblk, unsigned int blkcnt )
{
  (void)blkcnt; (void*)ctrlblk; (void)size; (void*)base;
  if ( ! mgr ) {
    return;
  }
  switch ( memmanagertype ) {
    case d0_mm_dynamic :
      #ifdef WITH_MM_DYNAMIC
        mgr->allocmem = d0_dyn_allocmem_wrapper;
        mgr->freemem  = d0_dyn_freemem_wrapper;
        mgr->memsize  = d0_dyn_memsize_wrapper;
      #endif
      break;
    case d0_mm_fixed_range :
      #ifdef WITH_MM_FIXED_RANGE
        d0_fixed_range_setheapmem( base, size );
        mgr->allocmem = d0_fixed_range_allocmen_wrapper;
        mgr->freemem  = d0_fixed_range_freemen_wrapper;
        mgr->memsize  = d0_fixed_range_memsize_wrapper;
        mgr->ctrlblk = base;
      #endif
      break;
    case d0_mm_fixed_range_fixed_blkcnt :
      #ifdef WITH_MM_FIXED_RANGE_FIXED_BLKCNT
        d0_fixed_range_fixed_blkcnt_heapinit( mgr->ctrlblk, base, size, blkcnt );
        mgr->allocmem = d0_fixed_range_fixed_blkcnt_allocmem_wrapper;
        mgr->freemem  = d0_fixed_range_fixed_blkcnt_freemem_wrapper;
        mgr->memsize  = d0_fixed_range_fixed_blkcnt_memsize_wrapper;
        mgr->ctrlblk = ctrlblk;
      #endif
      break;
    default:
      mgr->allocmem = d0_dummy_alloc;
      mgr->freemem = d0_dummy_free;
      mgr->memsize = d0_dummy_memsize;
      break;
  }
}

/******************************************************************************
 * videomemory and heapmemory manager
 *****************************************************************************/
static d0_memmanager* videomemory = NULL;
static d0_memmanager heapmemory={NULL,NULL,NULL,NULL};


/******************************************************************************
 * purpose: returns the size of a ctrlblk structure for a
 *          fixed_range_fixed_blkcnt memory manager
 * params:
 *  blkcnt  - blockcount used in the memorymanager
 * returns: returns the size of a ctrlblk structure for a
 *          fixed_range_fixed_blkcnt memory manager
 *****************************************************************************/
unsigned int get_fixed_range_fixed_blkcnt_ctrlblksize( unsigned int blkcnt )
{
  #ifdef WITH_MM_FIXED_RANGE_FIXED_BLKCNT
  return 2*blkcnt*sizeof( d0_fixed_range_fixed_blk_memblock )
         + sizeof(d0_fixed_range_fixed_blkcnt_heap);
  #else
    (void)blkcnt;
    assert(0);
    return 0;
  #endif
}

/******************************************************************************
 * purpose: initialize one memory manager for the heap memory and one for the 
 *          video memory
 * params:
 *  heapaddress   - start address of the heap memory
 *  heapsize      - size of the heap in bytes
 *  heapmgrtype   - type of the memory manager use for the memory
 *  vidmemaddress - start address of the video memory manager
 *  vidmemsize    - size of the video memory
 *  vidmemblkcnt  - amount of allocatable blocks in video memory range
 *  vidmemmgrtype - type of the memory manager use for the videomemory
 *  flags         - possible values (d0_ma_unified, d0_ma_separated)
 * returns: 1 on success otherwise 0
 
 *****************************************************************************/
int d0_initheapmanager(  void* heapaddress, unsigned int heapsize,
                         d0_memmanager_type heapmgrtype, void* vidmemaddress,
                         unsigned int vidmemsize, unsigned int vidmemblkcnt,
                         d0_memmanager_type vidmemmgrtype,
                         d0_memarchitectures flags
                        )
{
  /* check arguments */
  if ( ! heapsize ) {
    return 0;
  }
  /* initialize system heap memory manager */
  if ( heapmgrtype == d0_mm_fixed_range_fixed_blkcnt ) {
    return 0;
  }
  d0_preparememorymanager( &heapmemory, heapmgrtype, heapaddress, heapsize, NULL , 0 );
  /* initialize video memory manager if needed */
  if ( flags & d0_ma_separated ) {
    if ( vidmemsize ) {
      /* allocate space for the videomemory controlstructure */
      videomemory = d0_allocmem( sizeof(d0_memmanager) ) ;
      if ( ! videomemory ) {
        return 0;
      }
      /* allocate extended videomemory control structure if necessary */
      if ( vidmemmgrtype == d0_mm_fixed_range_fixed_blkcnt ) {
        videomemory->ctrlblk = d0_allocmem( get_fixed_range_fixed_blkcnt_ctrlblksize( vidmemblkcnt ) );
        if ( ! videomemory->ctrlblk ) {
          return 0;
        }
      } else {
        videomemory->ctrlblk = vidmemaddress;
      }
      d0_preparememorymanager( videomemory, vidmemmgrtype, vidmemaddress, vidmemsize, videomemory->ctrlblk, vidmemblkcnt );
    } else {
      return 0;
    }
  } else {
    videomemory = &heapmemory;
  }
  return 1;
}
/******************************************************************************
 * purpose: initialize d0_mm_dynamic as default memorymanager.
 * params:
 * returns:  1
 *****************************************************************************/
int d0_initdefaultheapmanager( void )
{
  d0_preparememorymanager( &heapmemory, d0_mm_dynamic, 0, 0, NULL , 0 );
  videomemory = &heapmemory;
  return 1;
}

/******************************************************************************
 * purpose: allocate videomemory
 * params:
 *  size    -   size of the memory block in bytes
 * returns: a ptr to the new allocated memoryblock or NULL if an error occured
 *****************************************************************************/
void * d0_allocvidmem( unsigned int size )
{
  if (!videomemory) return NULL;
  if (!videomemory->allocmem || !videomemory->ctrlblk) return NULL;
  
  return videomemory->allocmem( videomemory->ctrlblk, size);
}

/******************************************************************************
 * purpose: free a block of videomemory
 * params:
 *  ptr     - pointer to the memory block
 *****************************************************************************/
void d0_freevidmem( void *ptr )
{
  videomemory->freemem(videomemory->ctrlblk, ptr);
}
/******************************************************************************
 * purpose: returns the size of a video memory block
 * params:
 *  ptr     - points to the memory block
 * returns: the size of the memory block
 *****************************************************************************/
unsigned int d0_vidmemsize(  void *ptr )
{
  return videomemory->memsize(videomemory->ctrlblk, ptr);
}

/******************************************************************************
 * purpose: allocate heapmemory
 * params:
 *  size    -   size of the memory block in bytes
 * returns: a ptr to the new allocated memoryblock or NULL if an error occured
 *****************************************************************************/
void * d0_allocmem( unsigned int size )
{
  if ( heapmemory.allocmem ) {
    return heapmemory.allocmem( heapmemory.ctrlblk, size);
  } else {
  #ifdef WITH_MM_DYNAMIC
      d0_initdefaultheapmanager();
      return heapmemory.allocmem( heapmemory.ctrlblk, size);
  #else
    return NULL;
  #endif
  }
}
/******************************************************************************
 * purpose: free a block of videomemory
 * params:
 *  ptr     - pointer to the memory block
 *****************************************************************************/
void d0_freemem( void *ptr )
{
  heapmemory.freemem(heapmemory.ctrlblk, ptr);
}
/******************************************************************************
 * purpose: returns the size of a memory block
 * params:
 *  ptr     - points to the memory block
 * returns: the size of the memory block
 *****************************************************************************/
unsigned int d0_memsize( void *ptr )
{
  return heapmemory.memsize(heapmemory.ctrlblk, ptr);
}

