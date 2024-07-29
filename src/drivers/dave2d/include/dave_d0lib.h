/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_d0lib.h (%version: 5 %)
 *
 * Description:
 *  memory management
 *  %date_modified: Mon Feb 05 18:57:37 2007 %  (%derived_by:  hh74036 %)
 *
 * Changes:
 *  MGe 013107 redesign
 *  ASc 083007 fix d0_initheapmanager description, changed C++ to C comments
 *  CSe 080422 export d0_preparememorymanager */

/******************************************************************************
 *  Title: Heap management
 *  Collection of heap managers, which may be used by a d1 driver implementation on
 *  a certain platform. Eases d1 platform porting e.g. on non-UMA platforms or on
 *  platforms which don't have a stdlib with malloc/free/msize
 *
 *  There are three available types of memory managers. These managers can be
 *  used to manage blocks of memory e.g. video memory, heap memory etc.
 *
 *  Types of memory managers:
 *
 *    d0_mm_dynamic  - Wrappers for stdlib malloc, free and msize of the system.
 *
 *    d0_mm_fixed_range - Memory manager which uses one block of memory for
 *                        memory allocation and control structures (list
 *                        of allocated memory blocks).
 *                        This manager is normally used as manager for a heap
 *                        on systems that do not provide dynamic memory
 *                        allocation.
 *                        This memory manager needs 42 bytes of fixed control information.
 *                        For each block of allocated memory additional 8 bytes of
 *                        control information will be stored.
 *
 *    d0_mm_fixed_range_fixed_blkcnt - Memory manager which uses two blocks of
 *                                     memory: one for allocation and one for
 *                                     control structures (lists for free and
 *                                     used blocks).
 *                                     This manager is normally used to manage
 *                                     memory that isn't directly mapped to the
 *                                     system memory. Because of this, it needs
 *                                     a second block of memory in order to
 *                                     store control information. You have to
 *                                     predefine a fixed number of memory
 *                                     blocks that can be used. Typically used
 *                                     on systems with separate video memory.
 *                                     This memory manager needs 28 bytes of fixed control information
 *                                     and 16 bytes for each block of allocatable memory.
 *
 * Topic: Usage
 * How to use the D/AVE driver memory management interface.
 *
 *  If you want to make use of the d0 memory management functions, your d1 platform
 *  code for <d1_allocmem>, <d1_freemem>, <d1_memsize>, <d1_allocvidmem> and <d1_freevidmem>
 *  should wrap the respective d0_* functions.
 *  You then need to initialize the heap managers somewhere in your application
 *  before you do any d1 or d2 driver calls.
 *  The simplest way is to call <d0_initdefaultheapmanager>.
 *  This function registers a wrapper for the stdlib malloc, free and msize
 *  functions. These functions will be used as managers for the
 *  video and heap memory.
 *  In order to register different managers for the heap and video memory, you
 *  have to use the function <d0_initheapmanager>. This function allows to choose
 *  different memory managers for video and heap management.
 *
 * Topic: 
 *****************************************************************************/

#ifndef __1_dave_d0lib_h_H
#define __1_dave_d0lib_h_H
#ifdef __cplusplus
extern "C" {
#endif
/*--------------------------------------------------------------------------- */

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

typedef enum
{
  d0_ma_unified   =0x0,    /* Video memory is in the same address space as the heapmanager */
  d0_ma_separated  = 0x1   /* Video memory is separated from system memory and can be accessed by transfer or mapping functions only. */
} d0_memarchitectures;



typedef enum {
  d0_mm_fixed_range,
  d0_mm_fixed_range_fixed_blkcnt,
  d0_mm_dynamic
  } d0_memmanager_type;

/*****************************************************************************
 * typedef : d0_fp_allocmem
 *  Function pointer to a memory allocation function.
 *
 * parameters :
 *  ctrlblk - pointer to a control block of the current memory manager
 *  size    - size of the requested memory block
 *
 * returns :
 *  a pointer to the allocated memory block or NULL
 ****************************************************************************/
typedef void*         (*d0_fp_allocmem)( void*, unsigned int );

/*****************************************************************************
 * typedef : d0_fp_freemem
 *  Function pointer to a memory freeing function.
 *
 * parameters :
 *  ctrlblk - pointer to a control block of the current memory manager
 *  ptr    - pointer to the memory block that shall be freed
 ****************************************************************************/
typedef void          (*d0_fp_freemem) ( void*, void*  );

/*****************************************************************************
 * typedef : d0_fp_memsize
 *  Function pointer to the memory size function.
 *
 * parameters :
 *  ctrlblk - pointer to a control block of the current memory manager
 *  ptr    - pointer to the memory block, whose size is requested
 *
 * returns :
 *  the size of the memory block
 ****************************************************************************/
typedef unsigned int  (*d0_fp_memsize) ( void*, void* );

/*****************************************************************************
 * type : struct d0_memmanager
 *  Instance of a memory manager
 *
 *  fields :
 *    allocmem  - function pointer to the memory allocation function <d0_fp_allocmem>
 *    freemem - function pointer to the memory free function <d0_fp_freemem>
 *    memsize - function pointer to the memory size function <d0_fp_memsize>
 *    ctrlblk - points to the control data of the memory manager
 ****************************************************************************/
typedef struct {
  d0_fp_allocmem allocmem;
  d0_fp_freemem  freemem;
  d0_fp_memsize  memsize;
  void* ctrlblk;
} d0_memmanager;



/******************************************************************************
 * function: d0_preparememorymanager
 *  Initalize a heapmanager.
 *
 * parameters:
 *  mgr             - pointer to a d0_memmanager structure to be initialized
 *
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
extern void d0_preparememorymanager( d0_memmanager *mgr, d0_memmanager_type memmanagertype, void *base,
                                     unsigned int size, void *ctrlblk, unsigned int blkcnt );


/******************************************************************************
 * function: d0_initheapmanager
 *  Initialize the memory managers for the local heap memory and one for the
 *  video memory.
 *
 * parameters:
 *  heapaddress   - start address of the local heap memory
 *  heapsize      - size of the local heap in bytes
 *  heapmgrtype   - type of the memory manager use for the local heap memory (see above)
 *  vidmemaddress - start address of the video memory
 *  vidmemsize    - size of the video memory
 *  vidmemblkcnt  - amount of allocatable blocks in video memory range
 *  vidmemmgrtype - type of the memory manager use for the video memory (see above)
 *  flags         - possible values (d0_ma_unified, d0_ma_separated)
 *
 * note:
 *  If 'd0_ma_unified' is passed as flag, the video memory manager parameters
 *  are ignored. The heap memory manager will also be used as video memory manager.
 *
 * returns:
 *    1 on success otherwise 0
 *****************************************************************************/
extern int d0_initheapmanager(  void* heapaddress, unsigned int heapsize,
                         d0_memmanager_type heapmgrtype, void* vidmemaddress,
                         unsigned int vidmemsize, unsigned int vidmemblkcnt,
                         d0_memmanager_type vidmemmgrtype,
                         d0_memarchitectures flags
                        );


/******************************************************************************
 * function: d0_initdefaultheapmanager
 *  Initialize 'd0_mm_dynamic' as default memory manager.
 *
 * returns:
 *  This function returns 1
 *****************************************************************************/
extern int d0_initdefaultheapmanager( void ) ;

/******************************************************************************
 * function: d0_allocvidmem
 *  Allocate a block of video memory.
 *  This is a wrapper for the memory allocation function of the video memory.
 *
 * parameters:
 *  size    -   size of the memory block in bytes
 *
 * returns:
 *  a pointer to the newly allocated memory block or NULL if an error occured
 *****************************************************************************/
extern void* d0_allocvidmem( unsigned int size );

/******************************************************************************
 * function: d0_freevidmem
 *  Free a block of video memory.
 *  This is a wrapper for the memory freeing function of the video memory.
 *
 * parameters:
 *  ptr     - pointer to the memory block
 *****************************************************************************/
extern void  d0_freevidmem( void *ptr );

/******************************************************************************
 * function: d0_vidmemsize
 *  Returns the size of a video memory block.
 *  This is a wrapper for the memory size function of the video memory.
 *
 * parameters:
 *  ptr     - points to the memory block
 *
 * returns:
 *  the size of the memory block
 *****************************************************************************/
extern unsigned int d0_vidmemsize( void *ptr );

/******************************************************************************
 * function: d0_allocmem
 *  Allocate local heap memory.
 *  This is a wrapper for the memory allocation function of the local heap memory.
 *
 * parameters:
 *  size    -   size of the memory block in bytes
 *
 * returns:
 *  a pointer to the newly allocated memoryblock or NULL if an error occured
 *****************************************************************************/
extern void* d0_allocmem( unsigned int size );

/******************************************************************************
 * function: d0_freemem
 *  Free a block of local heap memory.
 *  This is a wrapper for the memory freeing function of the local heap memory.
 *
 * parameters:
 *  ptr     - pointer to the memory block
 *****************************************************************************/
extern void  d0_freemem( void *ptr );

/******************************************************************************
 * function: d0_memsize
 *  Returns the size of a local memory block.
 *  This is a wrapper for the memory size function of the local heap memory.
 *
 * parameters:
 *  ptr     - pointer to the memory block
 *
 * returns:
 *  the size of the memory block
 *****************************************************************************/
extern unsigned int  d0_memsize( void *ptr );


/*--------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif
