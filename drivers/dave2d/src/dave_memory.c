/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_memory.c (%version: 8 %)
 *          created Tue Jan 11 13:12:18 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Jan 04 10:37:27 2007 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2006-02-28 CSe  special allocation for dlist memory
 *  2006-05-10 CSe  changed allocation of dlist memory  for systems
 *                  with statically mapped vidmem
 *  2006-11-07 CSe  allocate memory through new d0_ functions
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_memory.h"
#include "dave_base.h"

/*--------------------------------------------------------------------------
 * get memory from current process heap
 * */
void *d2_getmem_p( d2_u32 size )
{
   return d1_allocmem(size);
}

/*--------------------------------------------------------------------------
 * free memory from current process heap
 * */
void d2_freemem_p( void *adr )
{
   d1_freemem( adr );
}

/*--------------------------------------------------------------------------
 * resize memory from current process heap
 *
 * keep - set to 0 if it is not necessary to keep the original data
 * */
void *d2_reallocmem_p( d2_u32 newsize, void *oldadr, d2_s32 keep )
{
   void *newadr;
   d2_u32 oldsize;

   /* newsize 0 implies just a free */
   if(0 == newsize)
   {
      d2_freemem_p( oldadr );
      return (void*)0;
   }

   /* query old blocksize */
   oldsize = d1_memsize( oldadr );
   if(0 == oldsize)
   {
      return (void*)0;
   }

   /* realloc only if larger */
   if((d2_u32)newsize < oldsize)
   {
      return oldadr;
   }

   newadr = d2_getmem_p( newsize );
   if(NULL == newadr)
   {
      return newadr;
   }

   /* copy content as ints if possible */
   if(0 != keep)
   {
      d2_u32 i;
      d2_s32 *src = (d2_s32*) oldadr;
      d2_s32 *dst = (d2_s32*) newadr;

      for (i=0; i<(oldsize / sizeof(d2_s32)); i++)
      {
         *dst = *src;
         dst++;
         src++;
      }

      if(0 == (oldsize & (sizeof(d2_u32)-1)))
      {
         /* copy not 'int aligned' part */
         d2_s8 *srcc = (d2_s8*) src;
         d2_s8 *dstc = (d2_s8*) dst;
         for (i=0; i<(oldsize & (sizeof(d2_s32)-1)); i++)
         {
            *dstc = *srcc;
            dstc++;
            srcc++;
         }
      }
   }

   /* release old block */
   d2_freemem_p(oldadr);

   return newadr;
}

/*--------------------------------------------------------------------------
 * get memory from display list heap
 * */
void *d2_getmem_d( const d2_device *handle, d2_u32 size )
{
   d1_device *id = d2_level1interface(handle);

   if(0 != (D2_DEV(handle)->hwmemarchitecture & d1_ma_separated))
   {
      return d1_allocvidmem( id, d1_mem_dlist, size );
   }

   if(0 != (D2_DEV(handle)->hwmemarchitecture & d1_ma_mapped))
   {
      return d1_mapfromvidmem(id, d1_allocvidmem( id, d1_mem_dlist, size ));
   }

   return d1_allocmem( size );
}


/*--------------------------------------------------------------------------
 * free memory from display list heap
 * */
void d2_freemem_d( const d2_device *handle, void *adr )
{
   d1_device *id = d2_level1interface(handle);

   if(0 != (D2_DEV(handle)->hwmemarchitecture & d1_ma_separated))
   {
      d1_freevidmem( id, d1_mem_dlist, adr );
   }

   else if(0 != (D2_DEV(handle)->hwmemarchitecture & d1_ma_mapped))
   {
      d1_freevidmem( id, d1_mem_dlist, d1_maptovidmem(id, adr) );
   }
   else
   {
      d1_freemem( adr );
   }
}
