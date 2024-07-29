/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_hardware.c (%version: 18 %)
 *          created Wed Aug 24 12:57:58 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Mar 08 15:05:38 2007 %  (%derived_by:  hh74026 %)
 *
 * Changes:
 *  2005-12-22 CSe  - d2hw_start():  added copy of dlist to vidmem
 *                  - d2hw_finish(): added wait for dlist irq
 *  2006-02-28 CSe  - d2hw_start():  added flush of dlist blocks
 *  2006-04-12 CSe  - d2hw_start():  removed flush of complete data cache
 *  2006-05-10 CSe  - d2hw_start():  added map of dlist for systems with
 *                                   statically mapped vidmem
 *  2006-10-30 CSe  - d2hw_finish(): changed waiting for dlist irq
 *  2006-11-07 CSe  - d2hw_start():  changes for new 'd2_df_low_localmem' mode
 *  2007-03-08 CSe  - d2hw_start():  moved copying and flushing of dlist to
 *                                   d2_preparedlist_read_intern()
 *  2007-08-29 ASc  - changed g_d1refcount and g_d1handle to static variables
 *                    removed tabs, changed C++ to C comments
 *  2010-11-25 MRe  - fixed d2hw_acquire for multiple calls in case of 
 *                    multithreading
 *  2011-02-07 SSt  - moved instance management for multithreading to d1 driver
 *  2012-09-25 BSp  - MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_render.h"

/*--------------------------------------------------------------------------
 * */
d1_device * d2hw_acquire( d2_device *handle, d2_u32 flags )
{
   d1_device *d1handle;

   d1handle = d1_opendevice( (d2_slong) flags );

   if(NULL != d1handle)
   {
      D2_DEV(handle)->hwrevision = (d2_u32) d1_getregister( d1handle, D1_DAVE2D, D2_HWREVISION );

      if(0 != (D2_DEV(handle)->hwrevision & D2FB_HILIMITERPRECISION))
      {
         D2_DEV(handle)->hilimiterprecision_supported = 1;
      }

      if(0 == (D2_DEV(handle)->hwrevision & D2FB_DLR))
      {
         D2_DEV(handle)->flags |= d2_df_no_dlist;
      }

      /* attention: don't change cache settings while HW is running -> added caution note in doc of d2_opendevice() */
      d1_setregister( d1handle, D1_DAVE2D, D2_CACHECTL, (d2_slong) D2_DEV(handle)->cachectlmask );

      D2_DEV(handle)->hwmemarchitecture = (d2_u32) d1_queryarchitecture(d1handle);
   }

   return d1handle;
}

/*--------------------------------------------------------------------------
 * */
d2_s32 d2hw_release( d1_device * hwid )
{
   if(1 == d1_closedevice( hwid ))
   {
      return D2_OK;
   }
   else
   {
      return D2_INVALIDDEVICE;
   }
}

/*--------------------------------------------------------------------------
 * */
void d2hw_set(d1_device * hwid, d2_u32 index, d2_s32 value)
{
   d1_setregister( hwid, D1_DAVE2D, (d2_s32)index, value );
}

/*--------------------------------------------------------------------------
 * */
d2_s32 d2hw_get(d1_device * hwid, d2_u32 index)
{
   return d1_getregister( hwid, D1_DAVE2D, (d2_s32)index );
}

/*--------------------------------------------------------------------------
 * */
void d2hw_wait(d1_device * hwid)
{
   /* wait for enum ready state */
   while(0 != ((d2_u32)d1_getregister( hwid, D1_DAVE2D, D2_STATUS ) & D2C_BUSY_ENUM) )
   {}
}


/*---------------------------------------------------------------------------
 * return only after all rendering has finished
 * */
void d2hw_finish(const d2_device *handle)
{
   d1_device * hwId = (d1_device *) D2_DEV(handle)->hwid;

   if(0 != (D2_DEV(handle)->flags & d2_df_no_dlist))
   {
      /* wait for ready state */
      while(0 != ((d2_u32)d1_getregister( hwId, D1_DAVE2D, D2_STATUS ) & ( D2C_BUSY_ENUM | D2C_BUSY_WRITE )))
      {}
   }
   else
   {
      /* wait for dlist finish signal */
      while(0 != ((d2_u32)d1_getregister( hwId, D1_DAVE2D, D2_STATUS ) & D2C_DLISTACTIVE))
      {
         if(0 == (D2_DEV(handle)->flags & d2_df_no_irq))
         {
            /* don't wait forever in case another driver instance is scheduled between
             * the register check and the wait for the irq and takes the event */
            (void)d1_queryirq(hwId, d1_irq_dlist, 200);
         }
      }
   }
}

/*---------------------------------------------------------------------------
 * start rendering primitives
 * */
void d2hw_start( d2_device *handle, const d2_dlist *dlist, d2_s32 startnoblk )
{
   d2_devicedata  *dev        = D2_DEV(handle);
   d1_device      *id         = D2_DEV(handle)->hwid;
   d2_dlist_block *blk        = dlist->firstblock;
   d2_s32         *dlist_list = (d2_s32 *) dlist->dlist_addresses;

   if(0 != (dev->flags & d2_df_no_dlist))
   {
      /* emulated display list processing */

      /* note that no video memory has been allocated to store dlists */

      if(0 != startnoblk)
      {
         /* we come from d2_executedlist */
         (void)d2_executedlist_intern(handle, (d2_dlist_entry *)*dlist_list);
      }
      else
      {
         while(NULL != blk)
         {
            d2_dlist_entry *pos = blk->block;

            if(3 == d2_executedlist_intern(handle, pos))
            {
               break;
            }

            blk = blk->next;
         }
      }

      return;
   }

   if(0 != dlist_list[0])
   {
      if(0 != dev->dlist_indirect_supported)
      {
         d2hw_set( id, D2_DLISTSTART , (d2_s32)&dlist_list[0] );
      }
      else
      {
         d2hw_set( id, D2_DLISTSTART , dlist_list[0] );
      }
   }

}
