/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_perfcount.c (%version: 5 %)
 *          created Wed Jan 12 16:11:17 2005 by hh04027
 *
 * Description:
 *  %date_modified: Mon Mar 12 15:29:23 2007 %  (%derived_by:  hh74026 %)
 *
 * Changes:
 *  2007-03-14 CSe  added DEVICEBUSY error when DLR is active
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs 
 *  2008-07-16 ASt  added new performance counter events for ndoc
 *  2012-09-25 BSp  MISRA cleanup
 */

/*--------------------------------------------------------------------------
 *
 * Title: Profiling
 * Performance measurement counter functions
 *
 * Check D2FB_PERFCOUNT bit of <d2_getrevisionhw> to see if performance counters
 * are available.
 *
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"

/*--------------------------------------------------------------------------
 * Group: Performance counting
 * */


/*--------------------------------------------------------------------------
 * function: d2_setperfcountevent
 * Set the event to be counted by performance counter.
 *
 * Does not work while D/AVE is active! When calling this while D/AVE is active
 * executing a display list, rendering errors or even hangups can occur.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   counter - 0 or 1: which of both available performance counters to use
 *   event - which events the selected performance counter has to count
 *
 * events:
 *  d2_pc_disable        - disable performance counter
 *  d2_pc_davecycles     - DAVE active cycles
 *  d2_pc_fbreads        - framebuffer read access
 *  d2_pc_fbwrites       - framebuffer write access
 *  d2_pc_texreads       - texture read access
 *  d2_pc_invpixels      - invisible pixels (enumerated but selected with alpha 0%)
 *  d2_pc_invpixels_miss - invisible pixels while internal fifo is empty (lost cycles)
 *  d2_pc_dlrcycles      - displaylist reader active cycles
 *  d2_pc_fbreadhits     - framebuffer read hits
 *  d2_pc_fbreadmisses   - framebuffer read misses
 *  d2_pc_fbwritehits    - framebuffer write hits
 *  d2_pc_fbwritemisses  - framebuffer write misses
 *  d2_pc_texreadhits    - texture read hits
 *  d2_pc_texreadmisses  - texture read misses
 *  d2_pc_clkcycles      - every clock cycle (for use as timer)
 *  d2_pc_dlrburstreads  - displaylist reader burst reads
 *  d2_pc_dlrwordsread   - displaylist reader words read
 *  d2_pc_rlerewinds     - texture rle decoder rewinds
 *  d2_pc_texburstreads  - texture cache burst reads
 *  d2_pc_texwordsread   - texture cache words read
 *  d2_pc_fbburstreads   - framebuffer cache burst reads
 *  d2_pc_fbwordsread    - framebuffer cache words read
 *  d2_pc_fbburstwrites  - framebuffer cache burst writes
 *  d2_pc_fbwordswritten - framebuffer cache words written
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_setperfcountevent( d2_device *handle, d2_u32 counter, d2_u32 event )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE );    /* PRQA S 3112 3453 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( counter < 2, D2_VALUETOOBIG ); /* PRQA S 3112 3453 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 != ((d2_u32)d2hw_get( D2_DEV(handle)->hwid, D2_STATUS ) & D2C_DLISTACTIVE))
   {
      D2_RETERR( handle, D2_DEVICEBUSY );
   }

   /* fbcache performance measurement is different for prefetching framebuffer cache */
   if(0 != (D2_DEV(handle)->hwrevision & D2FB_FBPREFETCH))
   {
      switch (event)
      {
         case d2_pc_fbwritehits:
            event = d2_pc_fbwrites;
            break;

         case d2_pc_fbwritemisses:
            event = d2_pc_noevent;
            break;

         case d2_pc_fbrwconflicts:
            event = d2_pc_fbwritehits;
            break;

         case d2_pc_fbrwconflictcycles:
            event = d2_pc_fbwritemisses;
            break;

         default:
            break;
      }
   }

   D2_DEV(handle)->perftriggermask &= ~(0xffffu << (counter << 4));
   D2_DEV(handle)->perftriggermask |=  event << (counter << 4);

   d2hw_set( D2_DEV(handle)->hwid, D2_PERFTRIGGER, (d2_s32)D2_DEV(handle)->perftriggermask );

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setperfcountvalue
 * Set the current performance counter value.
 *
 * Does not work while D/AVE is active! When calling this while D/AVE is active
 * executing a display list, rendering errors or even hangups can occur.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   counter - 0 or 1: which of both available performance counters to use
 *   value - value to be set: 0 for reset
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_setperfcountvalue( d2_device *handle, d2_u32 counter, d2_slong value )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE );     /* PRQA S 3112 3453 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( counter < 2, D2_VALUETOOBIG );  /* PRQA S 3112 3453 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 != ((d2_u32)d2hw_get( D2_DEV(handle)->hwid, D2_STATUS ) & D2C_DLISTACTIVE))
   {
      D2_RETERR( handle, D2_DEVICEBUSY );
   }

   if(0 == counter)
   {
      d2hw_set( D2_DEV(handle)->hwid, D2_PERFCOUNT1 , value );
   }
   else
   {
      d2hw_set( D2_DEV(handle)->hwid, D2_PERFCOUNT2 , value );
   }

   D2_RETOK(handle);
}


/*--------------------------------------------------------------------------
 * function: d2_getperfcountvalue
 * Get the current performance counter value.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   counter - 0 or 1: which of both available performance counters to use
 *
 * returns:
 *   current counter register value
 * */
d2_slong d2_getperfcountvalue( d2_device *handle, d2_u32 counter )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE );    /* PRQA S 3112 3453 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( counter < 2, D2_VALUETOOBIG ); /* PRQA S 3112 3453 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 == counter)
   {
      return d2hw_get( D2_DEV(handle)->hwid, D2_PERFCOUNT1 );
   }
   else
   {
      return d2hw_get( D2_DEV(handle)->hwid, D2_PERFCOUNT2 );
   }
}
