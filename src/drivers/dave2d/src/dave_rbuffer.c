/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_rbuffer.c (%version: 27 %)
 *          created Tue May 10 09:42:13 2005 by hh04027
 *
 * Description:
 *  %date_modified: Fri Jan 26 13:50:26 2007 %  (%derived_by:  hh74026 %)
 *
 * Changes:
 *  2005-12-22 CSe  d2_executerenderbuffer(): removed wait for finish of dlist
 *  2006-04-12 CSe  fixed comments for correct documentation
 *  2006-07-21 CSe  implemented d2_relocateframe()
 *  2006-10-30 CSe  removed counting of dlist used slots
 *  2006-11-07 CSe  changes for new 'd2_df_low_localmem' mode
 *  2007-12-10 ASc  fix error checking in d2_newrenderbuffer
 *  2007-12-11 ASc  fix d2_relocateframe: flush scratch buffer
 *  2007-12-12 ASc  add buffer argument check in d2_freedumpedbuffer
 *                  add d2_getrenderbuffersize function
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs 
 *  2009-03-06 LBe  added flag in d2_executerenderbuffer to keep dlist for multiple execution
 *  2011-03-10 MRe  fixed delayed error handling in d2_executerenderbuffer
 *  2012-01-26 MRe  fixed flushing of renderbuffer when a new one is selected
 *  2012-07-18 MRe  improved render buffer management
 *  2012-09-25 BSp  MISRA cleanup
 */

/*--------------------------------------------------------------------------
 *
 * Title: Render Buffers
 * Renderbuffers (similar in concept to OpenGL display lists) are the main
 * interface between driver and hardware.
 *
 * Issuing rendering commands to the D/AVE driver does not directly trigger
 * a hardware activity but adds a set of commands to the currently _selected_
 * Renderbuffer (see: <d2_selectrenderbuffer>).
 *
 * This buffer can be executed directly by the hardware. For best performance
 * and maximum parallelism there should always be one renderbuffer executing
 * while another one is being filled (double-buffered render buffers).
 *
 * The application can manage this on its own by using <d2_selectrenderbuffer>
 * and <d2_executerenderbuffer> or make use of the utility functions and internal
 * buffers (see: <d2_startframe> and <d2_endframe>).
 *
 * startframe / endframe:
 * The recommended method using double-buffered render buffers is to use <d2_startframe> and
 * <d2_endframe>. These functions use two internal render buffers in turn.
 * The internal render buffers can be accessed by <d2_getrenderbuffer>.
 * (start code)
 * d2_startframe(handle); // --> start HW rendering of the previous frame (buffer a), switch to buffer b for all following render commands
 *
 * // ... now issue new render commands to buffer b while HW is rendering the previous frame from render buffer a ...
 *
 * d2_endframe(handle); // --> close render buffer b which has just captured all render commands, wait for HW to finish rendering of buffer a
 *
 * // may now propagate the rendered frame (from buffer a) to the display controller
 * (end code)
 *
 * selectrenderbuffer / executerenderbuffer / flushframe:
 * Render buffers can also be managed by the application. Use <d2_newrenderbuffer> to 
 * allocate a new render buffer. To issue render commands a render buffer must be 
 * selected by <d2_selectrenderbuffer>. A render buffer can then be executed by
 * <d2_executerenderbuffer>. Before <d2_executerenderbuffer> can be called again
 * the application has to wait for Dave to be finished by <d2_flushframe>.
 * (start code)
 * d2_selectrenderbuffer(handle, buffer);
 *
 * // issue render commands
 *
 * d2_executerenderbuffer(handle, buffer);
 * d2_flushframe(handle);
 * // propagate the rendered frame to the display controller
 * (end code)
 *
 * Using a single render buffer:
 * 
 * Please note that when using just a single render buffer (no double-buffering) 
 * the allocated buffer can not be freed as long as it is selected because the 
 * buffer will be linked for internal usage. This means that another call of 
 * <d2_selectrenderbuffer> must be issued with a second buffer to de-select the 
 * previous buffer to avoid memory leaks. 
 *
 * (start code)
 * buffer = d2_newrenderbuffer(handle, 25, 25);
 * d2_selectrenderbuffer(handle, buffer);
 * d2_executerenderbuffer(handle, buffer, d2_ef_default);
 * d2_flushframe(handle);
 * ...
 * // select a different buffer before freeing
 * d2_selectrenderbuffer(handle, newbuffer);
 * d2_freerenderbuffer(handle, buffer);
 * (end code)
 *
 * If no second render buffer is available and only a single render buffer 
 * should be used the following two commands have to be called before creating 
 * any render buffer. This will prevent internal buffer linkage!
 * 
 * (start code)
 * // close internal render buffer 0
 * d2_executerenderbuffer(locD2DHandle,d2_getrenderbuffer(locD2DHandle, 0), 0); 
 * // close internal render buffer 1
 * d2_executerenderbuffer(locD2DHandle,d2_getrenderbuffer(locD2DHandle, 1), 0); 
 * ...
 * buffer = d2_newrenderbuffer(handle, 25, 25);
 * d2_selectrenderbuffer(handle, buffer);
 * d2_executerenderbuffer(handle, buffer, d2_ef_default);
 * d2_flushframe(handle);
 * d2_freerenderbuffer(handle, buffer); // free a selected buffer
 * (end code)
 *
 * However, it is recommended to use at least two render buffers, e.g. 
 * <d2_startframe> and <d2_endframe>.
 *
 * executedlist:
 * Instead of <d2_executerenderbuffer> a given Dlist can be executed directly by
 * <d2_executedlist>.
 *
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_memory.h"
#include "dave_rbuffer.h"


/*--------------------------------------------------------------------------*/
static d2_s32 d2_resizerblayer_intern( const d2_device *handle, d2_rb_layer *layer ); /* MISRA */
static void d2_scratchgrowlayer_intern( d2_device *handle );


/*--------------------------------------------------------------------------
 * Group: Renderbuffer Management */


/*--------------------------------------------------------------------------
 * function: d2_newrenderbuffer
 * Create a new renderbuffer.
 *
 * Renderbuffers are filled by the driver but read (executed) directly by the
 * Dave hardware. Buffers grow on demand but only shrink slowly:
 * If a page was not used for 60 calls of <d2_executerenderbuffer>, the page
 * is freed one at a time: The next page will not be freed before 60 further
 * calls have elapsed.
 *
 * A single displaylist entry requires 20bytes of memory, choosing large
 * page sizes is potential waste of memory while too many small pages can
 * degrade the performance of both reading and writing to the buffer.
 *
 * By using <d2_getrenderbuffersize>, it is possible to query the number of used
 * entries for a given application. This can be used to optimize the page size.
 *
 * The number of used pages of the renderbuffer currently used as writebuffer
 * can be queried using <d2_getdlistblockcount>.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   initialsize - number of displaylist entries in the first page (minimum is 3)
 *   stepsize - number of displaylist entries in following pages (minimum is 3)
 *
 * returns:
 *   pointer to internal renderbuffer (or NULL if failed)
 *
 * see also:
 *   <d2_getrenderbuffer>, <d2_setdlistblocksize>, <d2_freerenderbuffer>
 * */
d2_renderbuffer * d2_newrenderbuffer( d2_device *handle, d2_u32 initialsize, d2_u32 stepsize )
{
   d2_rbuffer *rbuffer;
   d2_s32 bOOM = 0;

   /* error checking */
   if(NULL == handle)
   {
      return NULL;
   }

   if( (initialsize <= 2) || (stepsize <= 2) )
   {
      (void)D2_SETERR(handle, D2_VALUETOOSMALL);
      return NULL;
   }

   /* create new display list (/renderbuffer) */
   rbuffer = (d2_rbuffer*) d2_getmem_p( sizeof(d2_rbuffer) );

   if(NULL != rbuffer)
   {
      rbuffer->layer[0].scratch = NULL;

      if(0 != d2_initdlist_intern( handle, &rbuffer->baselist, initialsize ))
      {
         if(0 == D2_DEV(handle)->lowlocalmem_mode)
         {
            /* rblayer only in case of normal (no lowlocalmem mode) */

            if(0 == d2_initrblayer_intern( handle, &rbuffer->layer[0], D2_DLISTSCRATCH ))
            {
               bOOM = 1;
            }
         }
      }
      else
      {
         bOOM = 1;
      }
   }
   else
   {
      bOOM = 1;
   }

   if(0 == bOOM)
   {
      /* fill additional data */
      rbuffer->baselist.stepsize = stepsize;
      rbuffer->baselist.busy = 0;
      rbuffer->closed = 0;

      (void)D2_SETOK( handle );

      /* Succeeded */
      return rbuffer;
   }
   else
   {
      (void)D2_SETERR( handle, D2_NOMEMORY );

      if(NULL != rbuffer)
      {
         d2_freemem_p(rbuffer);
      }

      /* Failed */
      return NULL;
   }
}


/*--------------------------------------------------------------------------
 * function: d2_freerenderbuffer
 * Destroy and free a renderbuffer.
 *
 * You can only free buffers that were created by <d2_newrenderbuffer> and are
 * not currently executed. To make sure execution has finished the application
 * can call <d2_flushframe>.
 *
 * note:
 * Please note that when using just a single render buffer (no double buffering) 
 * the allocated buffer can not be freed as long as it is selected (see above for 
 * more details).
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   buffer - renderbuffer address
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * */
d2_s32 d2_freerenderbuffer( d2_device *handle,  d2_renderbuffer *buffer )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE );  /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( buffer, D2_INVALIDBUFFER );  /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( D2_DRB(buffer)->baselist.device == D2_DEV(handle) , D2_INVALIDDEVICE ); /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* check if list is currently executed */
   if(0 != D2_DRB(buffer)->baselist.busy)
   {
      D2_RETERR( handle, D2_DEVICEBUSY );
   }

   /* do not allow removal of default buffers */
   if(
      (D2_DEV(handle)->renderbuffer[0] == D2_DRB(buffer)) ||
      (D2_DEV(handle)->renderbuffer[1] == D2_DRB(buffer))
      )
   {
      D2_RETERR( handle, D2_DEFBUFFER );
   }

   if(D2_DEV(handle)->selectedbuffer == D2_DRB(buffer))
   {
      D2_DEV(handle)->selectedbuffer = D2_DRB(D2_DEV(handle)->writelist);
   }

   /* destruct all display list buffers */
   d2_deinitdlist_intern(&(D2_DRB(buffer)->baselist));

   if(NULL != D2_DRB(buffer)->layer[0].scratch)
   {
      d2_freemem_p( D2_DRB(buffer)->layer[0].scratch );
   }

   d2_freemem_p( buffer );

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_selectrenderbuffer
 * Choose a renderbuffer for writing.
 * Selecting a buffer that is currently executed will cause rendering failures.
 *
 * The previously selected renderbuffer is flushed.
 * All subsequent <Rendering Functions> write to the specified renderbuffer.
 * The buffer is finalized when <d2_executerenderbuffer> is called.
 * The render buffer is reset for writing a new list after selection.
 *
 * For the usage of render buffers see <Render Buffers>.
 *
 * If buffer is 0 then the internal renderbuffer for writing will be selected, thus 
 * <d2_startframe>/<d2_endframe> can be continued to use 
 * (see also <d2_getrenderbuffer>).
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   buffer - renderbuffer address or 0
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_selectrenderbuffer( d2_device *handle, d2_renderbuffer *buffer )
{
#ifdef D2_USEREGCACHE
   d2_s32 i;
#endif
   d2_dlist *wlist;
   void     *current_dlist_start;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );  /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* flush old render buffer */
   d2_scratch2dlist_intern(handle);

   if(NULL != D2_DEV(handle)->lowlocalmem_mode)
   {
      d2_dlist *dlist = &D2_DEV(handle)->selectedbuffer->baselist;

      if( (buffer != dlist)  && (0 == D2_DEV(handle)->selectedbuffer->closed) )
      {
         /* make sure to copy rest of dlist vidmem and add a jump to next slice */
         d2_paddlist_intern(dlist);
         (void)d2_insertdlistjump_intern( dlist, NULL );
         (void)d2_preparedlist_read_intern(handle, dlist, 0);
      }
   }

   /* use internal writelist */
   if(NULL == buffer)
   {
      buffer = D2_DEV(handle)->writelist;
   }

   /* reset list closed flag when selecting a render buffer */
   ((d2_rbuffer*)buffer)->closed = 0;
   
   /* hard change of writelist */
   D2_DEV(handle)->selectedbuffer = D2_DRB(buffer);
   wlist = & D2_DRB(buffer)->baselist;
   D2_DEV(handle)->srccontext = NULL;

   if(0 != wlist->busy)
   {
      /*note - might add a flush here
       * list needs reset */
      wlist->busy = 0;
      d2_resetdlist_intern( wlist );
   }

   if(NULL != wlist->vidmem_blocks)
   {
      /* low localmem mode: vidmem is already filled */
      current_dlist_start = wlist->vidmem_blocks->blocks[0];
   }
   else if( (0 != (D2_DEV(handle)->hwmemarchitecture & (d1_ma_separated | d1_ma_mapped))) &&  (NULL != wlist->firstblock->vidmem) )
   {
      current_dlist_start = wlist->firstblock->vidmem;
   }
   else
   {
      current_dlist_start = wlist->firstblock->block;
   }

   /* init list of dlist start addresses */
   d2_clear_dlistlist_intern(handle, wlist);
   (void)d2_add_dlistlist_intern(handle, wlist, current_dlist_start);

   /* invalidate register cache at start of new dlist
    * (inter-list optimisation is considered too risky) */
#ifdef D2_USEREGCACHE
   if(0 == (D2_DEV(handle)->flags & d2_df_no_registercaching))
   {
      for(i=0; i<D2_QUANTITY; i++)
      {
         D2_DEV(handle)->cache.valid[i] = 0;
      }
   }
#endif

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_executerenderbuffer
 * Render the content of a renderbuffer.
 *
 * For the usage of render buffers see <Render Buffers>.
 *
 * Before rendering is started, the renderbuffer is prepared for execution. This
 * involves flushing of scratch buffers, possibly merge of layers in certain render modes (see <d2_selectrendermode>)
 * and potentially copying the display list to a dedicated video memory or at least flushing of the 
 * CPU data caches.
 * The buffer passed to d2_executerenderbuffer can be used for execution again by calling d2_executerenderbuffer
 * again with the same buffer as argument. But before new render commands can be written to the buffer
 * it must be selected by <d2_selectrenderbuffer>.
 *
 * Note that before calling d2_executerenderbuffer, it must be made sure that no render buffer is currently
 * executed on the hardware. This can be done by either waiting for the respective interrupt or by calling <d2_flushframe>.
 *
 * Note that even when <d2_flushframe> is issued to wait for the rendering of the buffer to finish
 * the buffer still has to be reselected (using <d2_selectrenderbuffer>) in order
 * to use it again for rendering.
 *
 * Note that calling this function from inside an interrupt service routine is not recommended, as the
 * execution time can not be foreseen. If it is called from inside the D/AVE 'display list finished' ISR anyway,
 * it needs to be made sure that clearing the IRQ happens before the call to d2_executerenderbuffer!
 * Register writes to the D/AVE core are not allowed while a display list is being executed.
 *
 * Note that if d2_ef_execute_once is used a new render buffer must be selected by <d2_selectrenderbuffer>
 * even if it is the same render buffer.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   buffer - renderbuffer address
 *   flags  - (not used)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * */
d2_s32 d2_executerenderbuffer( d2_device *handle, d2_renderbuffer *buffer, d2_u32 flags )
{
   d2_s32 errorCode=0;
   d2_dlist *rlist;
   d2_renderbuffer *selectedBuffer;
   (void) flags;                              /* PRQA S 3112 */ /* $Misra: #COMPILER_WARNING $*/

   D2_VALIDATE( handle, D2_INVALIDDEVICE );   /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( buffer, D2_INVALIDBUFFER );   /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( D2_DRB(buffer)->baselist.device== D2_DEV(handle) , D2_INVALIDDEVICE );  /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* set selected buffer temporarily to buffer */
   selectedBuffer = D2_DEV(handle)->selectedbuffer;
   D2_DEV(handle)->selectedbuffer = buffer;

   /* check if list is currently executed */
   if(0 != D2_DRB(buffer)->baselist.busy)
   {
      D2_RETERR( handle, D2_DEVICEBUSY );
   }
   
   /* new read list */
   rlist = & D2_DRB( buffer )->baselist;

   /* start new list */
   rlist->busy = 1;

   if(0 == D2_DRB(buffer)->closed ) 
   {
      /* reset postprocessing mode if still active */
      if(d2_rm_postprocess == D2_DEV(handle)->rendermode)
      {
         (void)d2_selectrendermode( handle, d2_rm_solid );
      }

      /* merge in layers if they contain data */
      d2_layer2dlist_intern( handle );

      (void)d2_preparedlist_read_intern( handle, rlist, 1 );
   }

   D2_DRB(buffer)->closed = 1;

   /* start new displaylist */
   d2hw_start( handle, rlist, 0 );
   rlist->busy = 0;

   /* restore selected buffer */
   D2_DEV(handle)->selectedbuffer = selectedBuffer;

   errorCode = D2_DEV(handle)->delayed_errorcode;
   D2_DEV(handle)->delayed_errorcode = D2_OK;  /* reset errorcode */
   D2_RETERR( handle, errorCode);
}


/*--------------------------------------------------------------------------
 * function: d2_getrenderbuffer
 * Return internal renderbuffers.
 *
 * Two renderbuffers are allocated automatically for every device. These cannot
 * be freed manually (destruction is automatic) but can be used for rendering.
 *
 * Note that if internal buffers are used the utility functions <d2_startframe> and
 * <d2_endframe> might not work as expected anymore. These functions use both internal
 * buffers for 'double buffering' and rely on using <d2_selectrenderbuffer> exclusively.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   index - renderbuffer index (only 0 and 1 are valid indices)
 *
 * returns:
 *   pointer to internal renderbuffer (or NULL if failed)
 * */
d2_renderbuffer * d2_getrenderbuffer( d2_device *handle, d2_s32 index )
{
   d2_rbuffer *rbuffer;

   /* error checking */
   if(NULL == handle)
   {
      return NULL;
   }

   if( (index < 0) || (index > 1) )
   {
      (void)D2_SETERR(handle, D2_INVALIDINDEX);
      return NULL;
   }

   /* get buffer */
   rbuffer = D2_DEV(handle)->renderbuffer[ index ];

   if(NULL == rbuffer)
   {
      (void)D2_SETERR(handle, D2_INVALIDBUFFER);
      return NULL;
   }

   (void)D2_SETOK(handle);
   return rbuffer;
}


/*--------------------------------------------------------------------------
 * Group: Utility Functions */


/*--------------------------------------------------------------------------
 * function: d2_startframe
 * Mark the begin of a scene.
 *
 * Use this function (together with <d2_endframe>) to let the driver handle
 * render buffer execution and flipping automatically.
 *
 * When startframe is called the render buffer that was filled during the
 * last frame (between previous <d2_startframe>/<d2_endframe>) is executed on the HW.
 * New render commands that are sent after this 'startframe' are put into the other
 * render buffer.
 *
 * For the usage of render buffers see <Render Buffers>.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_startframe( d2_device *handle )
{
   d2_s32 errorCode=0;
   d2_dlist *rlist, *wlist;
   D2_VALIDATE( handle, D2_INVALIDDEVICE );  /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* backup current pointers */
   rlist = D2_DEV(handle)->readlist;
   wlist = D2_DEV(handle)->writelist;

   /* start rendering of last frame's writelist */
   errorCode = d2_executerenderbuffer( handle, wlist, 0 );

   /* prepare old readlist for writing */
   (void)d2_selectrenderbuffer( handle, rlist );

   /* hard-swap read and write buffers */
   D2_DEV(handle)->readlist  = wlist;
   D2_DEV(handle)->writelist = rlist;

   D2_RETERR( handle, errorCode);
}

/*--------------------------------------------------------------------------
 * function: d2_endframe
 * Mark the end of a scene.
 *
 * Use this function (together with <d2_startframe>) to let the driver handle
 * render buffer execution and flipping automatically.
 *
 * Endframe makes sure the previous frame (the one started on the HW by the last
 * call of <d2_startframe>) has been rendered completely. A wait for the HW
 * is done if necessary.
 * The render buffer just filled with render commands (since the last <d2_startframe>)
 * is logically closed.
 *
 * For the usage of render buffers see <Render Buffers>.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_endframe( d2_device *handle )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE );  /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* wait for rendering to end */
   (void)d2_flushframe( handle );

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_relocateframe
 * Change framebuffer for render commands already issued.
 *
 * This function can be used to change the target framebuffer of all d2
 * render commands that have been issued since the last <d2_startframe>.
 *
 * It does only work if framebuffer format and size stay the same and only
 * a single framebuffer has been set used <d2_framebuffer> since the
 * frame has been started.
 * The current framebuffer is _not_ changed, this would require a
 * call to <d2_framebuffer> afterwards.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   ptr - new address of the top left pixel (coordinate 0,0)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * note:
 *   Intention of this function is to support an immediate flush to a hidden
 *   framebuffer when the frame currently assembled was initially prepared
 *   for the currently displayed framebuffer.
 *   Use <d2_layermerge> to take care about outline and solid parts.
 *
 *   This function can only be used when double word clearing is
 *   disabled (see: <d2_opendevice>) and 'low localmem' mode is not enabled
 *   (see: <d2_lowlocalmemmode>)! 
 *
 * note:
 *   If <d2_adddlist> (d2_al_no_copy) commands have been used these added 
 *   dlists are not changed.
 *
 * */
extern d2_s32 d2_relocateframe( d2_device *handle, const void *ptr )
{
   d2_u32          adrmask;
   d2_dlist_block *blk, *lastBlk;
   d2_dlist_entry *lastEntry;
   d2_s8 *         oldPtr;
  
   D2_VALIDATE( handle, D2_INVALIDDEVICE );  /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( ptr, D2_NOVIDEOMEM );        /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/

   d2_scratch2dlist_intern(handle);

   blk     = D2_DEV(handle)->selectedbuffer->baselist.firstblock;
   lastBlk = D2_DEV(handle)->selectedbuffer->baselist.currentblock;
   oldPtr  = (d2_s8*)D2_DEV(handle)->framebuffer;

   if(ptr == oldPtr)
   {
      D2_RETOK(handle);
   }

   while(NULL != blk)
   {
      d2_u32 argument = 0;
      d2_dlist_entry *entry = blk->block;

      lastEntry = (d2_dlist_entry*)((d2_s32*)entry + (blk->quantity*(sizeof(d2_dlist_entry)/sizeof(d2_s32))));  /* step through all entries in the block */

      while(entry < lastEntry)  
      {
         adrmask = entry->address.mask;

         if(0 != (adrmask & 0x80808080u))
         {
            /* contains special indices */
            if(adrmask == 0x80808080u)
            {
               /* completely empty -> skip */
               entry = (d2_dlist_entry*) ((d2_s32*)entry + 1);
            }
            else if(0 != (adrmask & 0x00008000u))
            {
               /* index1 only */
               if(D2_DLISTSTART == (adrmask & 0xffu))
               {
                  /* catch special case of display list jump */
                  entry = lastEntry; /*break;*/ /* skip this block */
               }
               else
               {
                  /* normal rewrite */
                  if(D2_ORIGIN == (adrmask & 0xffu))
                  {
                     entry->value[0] = (d2_s32) (((d2_s8*)entry->value[0] - oldPtr) + (d2_s8*)ptr);
                  }

                  entry = (d2_dlist_entry*) ((d2_s32*)entry + 2);
               }
            }
            else if(0 != (adrmask & 0x00800000u))
            {
               /* index1&2 only */
               if(0xffu == (adrmask & 0xffu))
               {
                  /* end of list word */
                  argument = (adrmask >> 8) & 0xffu;

                  if( (0 != (argument & 2u)) || (0 != (argument & 4u)) ) /* special bit 1 or 2 set? */
                  {
                     /* just a flush. keep on reading */
                     entry = (d2_dlist_entry*) ((d2_s32*)entry + 1);
                     /*continue;*/
                  } 
                  else
                  {   /* special bit 0 must be set */
                     /* terminate */
                     entry = lastEntry; /*break;*/
                  }
               }
               else 
               {
                  if(D2_ORIGIN == (adrmask & 0xffu))
                  {
                     entry->value[0] = (d2_s32) (((d2_s8*)entry->value[0] - oldPtr) + (d2_s8*)ptr);
                  }
                  
                  if((D2_ORIGIN<<8) == (adrmask & 0xff00u))
                  {
                     entry->value[1] = (d2_s32) (((d2_s8*)entry->value[1] - oldPtr) + (d2_s8*)ptr);
                  }
                  
                  entry = (d2_dlist_entry*) ((d2_s32*)entry + 3);
               }
            }
            else if (0 != (adrmask & 0x80000000u))
            {
               /* index1,2,3 only */
               if(D2_ORIGIN == (adrmask & 0xffu))
               {
                  entry->value[0] = (d2_s32) (((d2_s8*)entry->value[0] - oldPtr) + (d2_s8*)ptr);
               }

               if((D2_ORIGIN<<8) == (adrmask & 0xff00u))
               {
                  entry->value[1] = (d2_s32) (((d2_s8*)entry->value[1] - oldPtr) + (d2_s8*)ptr);
               }

               if((D2_ORIGIN<<16) == (adrmask & 0xff0000u))
               {
                  entry->value[2] = (d2_s32) (((d2_s8*)entry->value[2] - oldPtr) + (d2_s8*)ptr);
               }

               entry = (d2_dlist_entry*) ((d2_s32*)entry + 4);
            }
            else
            {
               /* end of list word */
               if(0xffu == (adrmask & 0xffu))
               {
                  /* get eol argument */
                  argument = (adrmask >> 8) & 0xffu;

                  if( (argument == 2u) || (argument == 4u) )
                  {
                     /* just a flush. keep on reading */
                     entry = (d2_dlist_entry*) ((d2_s32*)entry + 1);
                  }
                  else
                  {
                     /* terminate */
                     entry = lastEntry; /*break;*/
                  }
               }
               else
               {
                  /* invalid case, no index used */
                  entry = (d2_dlist_entry*) ((d2_s32*)entry + 1);
               }
            }
         } /* if(0 != (adrmask & 0x80808080u)) */
         else
         {
            /* contains simple indices only */
            if(D2_ORIGIN == (adrmask & 0xffu))
            {
               entry->value[0] = (d2_s32) (((d2_s8*)entry->value[0] - oldPtr) + (d2_s8*)ptr);
            }

            if((D2_ORIGIN<< 8) == (adrmask & 0x0000ff00u))
            {
               entry->value[1] = (d2_s32) (((d2_s8*)entry->value[1] - oldPtr) + (d2_s8*)ptr);
            }

            if((D2_ORIGIN<<16) == (adrmask & 0x00ff0000u))
            {
               entry->value[2] = (d2_s32) (((d2_s8*)entry->value[2] - oldPtr) + (d2_s8*)ptr);
            }

            if((D2_ORIGIN<<24) == (adrmask & 0xff000000u) )
            {
               entry->value[3] = (d2_s32) (((d2_s8*)entry->value[3] - oldPtr) + (d2_s8*)ptr);
            }

            /* next entry */
            entry++;
         }

      } /* while entry < lastEntry */

      if(0 != (argument & 1u))
      {
         blk = NULL; /*break;*/
      }
      else
      {
         blk = (blk == lastBlk) ? NULL : blk->next;
      }

   } /* while (NULL != blk) */

   D2_RETOK(handle);
}


/*--------------------------------------------------------------------------
 * function: d2_dumprenderbuffer
 * Copy the content of a renderbuffer into user memory.
 *
 * The entire renderbuffer is stored as one flat dave displaylist in memory.
 * Writes to the displaylist control register (display list jumps) are removed
 * and replaced by the code following at the new address - this way the dumped
 * list can be relocated in memory.
 *
 * Note that the application has to free the memory allocated for a dumped list
 * using <d2_freedumpedbuffer>.
 *
 * This function can only be used when double word clearing is
 * disabled (see: <d2_opendevice>) and 'low localmem' mode is not enabled (see: <d2_lowlocalmemmode>).
 *
 * note:
 * Use <d2_layermerge> to take care about outline and solid parts.
 *
 * note:
 *  d2_dumprenderbuffer does not work correctly if <d2_adddlist> (d2_al_no_copy)
 *  commands have been used. 
 *  Use <d2_adddlist> (d2_al_copy) instead if you want to get dlists with the
 *  desired content.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   buffer - renderbuffer address
 *   rdata - pointer to memory (filled by function)
 *   rsize - size of memory in bytes (filled by function)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_dumprenderbuffer( d2_device *handle, d2_renderbuffer *buffer, void **rdata, d2_s32 *rsize )
{
   d2_u32           adrmask;
   d2_dlist       * dlist;
   d2_u32           used;
   d2_s32           *writePtr;
   d2_dlist_block * blk, *lastBlk;
   d2_dlist_entry * write, *lastEntry;
 
   D2_VALIDATE( handle, D2_INVALIDDEVICE );  /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( buffer, D2_INVALIDBUFFER );  /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( rdata, D2_NULLPOINTER );     /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( rsize, D2_NULLPOINTER );     /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/

   d2_scratch2dlist_intern(handle);

   /* prepare list and measure size */
   dlist = & D2_DRB(buffer)->baselist;

   /* pad to next full entry */
   d2_paddlist_intern( dlist );

   /* count used entries if list is still in use */
   used = dlist->currentblock->quantity - dlist->blocksize;
   blk = dlist->firstblock;

   while(blk != dlist->currentblock)
   {
      used += blk->quantity;
      blk = blk->next;
   }

   if(0 == used)
   {
      /* empty buffer */
      *rdata = NULL;
      *rsize = 0;
      D2_RETOK(handle);
   }

   /* get memory (inc additional storage for terminator) */
   *rdata   = d2_getmem_p( sizeof(d2_dlist_entry) * (used + 1) );

   /* setup pointers to destination buffer */
   write    = (d2_dlist_entry *) *rdata;
   writePtr = (d2_s32*) *rdata;

   blk      = D2_DEV(handle)->selectedbuffer->baselist.firstblock;
   lastBlk  = D2_DEV(handle)->selectedbuffer->baselist.currentblock;

   while(NULL != blk)
   {
      d2_u32 argument = 0;
      d2_dlist_entry *entry = blk->block;

      if(blk == lastBlk)
      {
         lastEntry = dlist->position;            /* use insert postion of current block */
      }
      else
      {
         lastEntry = entry + blk->quantity;      /* full block: use end position */
      }

      while(entry < lastEntry)  
      {
         adrmask = entry->address.mask;

         if(0 != (adrmask & 0x80808080u))
         {
            /* contains special indices */
            if(0x80808080u == adrmask)
            {
               /* completely empty -> skip */
               *writePtr = (d2_s32) entry->address.mask;
               writePtr++;
               entry = (d2_dlist_entry*) ((d2_s32*)entry + 1);
            }
            else if(0 != (adrmask & 0x00008000u))
            {
               /* index1 only */
               if(D2_DLISTSTART == (adrmask & 0xffu))
               {
                  /* catch special case of display list jump */
                  break; /* skip this block */
               } 
               writePtr[0] = (d2_s32) entry->address.mask;
               writePtr[1] = entry->value[0];
               writePtr += 2;
               entry = (d2_dlist_entry*) ((d2_s32*)entry + 2);
            }
            else if (0 != (adrmask & 0x00800000u))
            {
               /* index1&2 only */
               if(0xffu == (adrmask & 0xffu))
               {
                  /* end of list word */
                  argument = (adrmask >> 8) & 0xffu;

                  if( (0 != (argument & 2u)) || (0 != (argument & 4u)) ) /* special bit 1 or 2 set? */
                  {
                     /* just a flush. keep on reading */
                     *writePtr = (d2_s32) entry->address.mask;
                     writePtr++;
                     entry = (d2_dlist_entry*) ((d2_s32*)entry + 1);
                     /*continue;*/
                  }
                  else
                  {   /* special bit 0 must be set */
                     /* terminate */
                     entry = lastEntry; /*break;*/
                  }
               }
               else
               {
                  writePtr[0] = (d2_s32) entry->address.mask;
                  writePtr[1] = entry->value[0];
                  writePtr[2] = entry->value[1];
                  writePtr += 3;
                  entry = (d2_dlist_entry*) ((d2_s32*)entry + 3);
               }
            }
            else if(0 != (adrmask & 0x80000000u))
            {
               /* index1,2,3 only */
               writePtr[0] = (d2_s32) entry->address.mask;
               writePtr[1] = entry->value[0];
               writePtr[2] = entry->value[1];
               writePtr[3] = entry->value[2];
               writePtr += 4;
               entry = (d2_dlist_entry*) ((d2_s32*)entry + 4);
            }
            else
            {
               /* end of list word */
               if(0xffu == (adrmask & 0xffu))
               {
                  /* get eol argument */
                  argument = (adrmask >> 8) & 0xffu;

                  if( (argument == 2u) || (argument == 4u) )
                  {
                     /* just a flush. keep on reading */
                     *writePtr = (d2_s32) entry->address.mask;
                     writePtr++;
                     entry = (d2_dlist_entry*) ((d2_s32*)entry + 1);
                  }
                  else
                  {
                     /* terminate */
                     entry = lastEntry; /*break;*/
                  }
               }
               else
               {
                  /* invalid case, no index used */
                  entry = (d2_dlist_entry*) ((d2_s32*)entry + 1);
               }
            }
         } /* if(0 != (adrmask & 0x80808080u)) */
         else
         {
            /* contains simple indices only */
            writePtr[0] = (d2_s32) entry->address.mask;
            writePtr[1] = entry->value[0];
            writePtr[2] = entry->value[1];
            writePtr[3] = entry->value[2];
            writePtr[4] = entry->value[3];
            writePtr += 5;

            /* next entry */
            entry++;
         }

      } /* while(entry < lastEntry) */

      if(0 != (argument & 1u))
      {
         blk = NULL; /*break;*/
      }
      else
      {
         blk = (blk == lastBlk) ? NULL : blk->next;
      }

   } /* while (NULL != blk) */

   write = (d2_dlist_entry*) writePtr;

   /* add terminator */
   write->address.mask = 0x8f8f03ffu;
   write->value[0] = (d2_s32) D2_DEV(handle)->framebuffer;
   write->value[1] = (d2_s32) ( (D2_DEV(handle)->fbheight << 16) | D2_DEV(handle)->fbwidth );
   write->value[2] = 0;
   write->value[3] = 0;
   write++;

   /* adjust size */
   *rsize = (d2_s32) (((d2_s8 *) write) - ((d2_s8 *) (*rdata)));

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_getrenderbuffersize
 * Query the number of allocated display list entries.
 * Each display list entry is based on one address word and 4 data
 * words (20 bytes).
 * This function can be used to profile an application and optimize the
 * page sizes set via <d2_setdlistblocksize> or <d2_newrenderbuffer>.
 * It is recommended to call this function only directly before
 * <d2_executerenderbuffer>, when all required commands are inside the
 * render buffer.
 *
 * This function can only be used when 'low localmem' mode is not enabled (see: <d2_lowlocalmemmode>).
 *
 * note:
 *   If <d2_adddlist> (d2_al_no_copy) commands have been used these added 
 *   dlists are not counted.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   rb     - pointer to renderbuffer
 *
 * returns:
 *   the number of allocated display list entries, or 0 if an error occurs
 * */
d2_u32 d2_getrenderbuffersize(d2_device *handle, d2_renderbuffer *rb)
{
   d2_dlist *dlist;
   d2_dlist_block *cblock;
   d2_u32 used;
   d2_rbuffer *buffer = (d2_rbuffer *) rb;

   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATEP( buffer, D2_INVALIDBUFFER ); /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/

   d2_scratch2dlist_intern( handle );

   /* prepare list and measure size */
   dlist = & buffer->baselist;

   /* pad to next full entry */
   d2_paddlist_intern( dlist );

   if(NULL != dlist->vidmem_blocks) 
   {
      used = dlist->count >> 2;
   }
   else
   {
      /* count used entries if list is still in use */
      used = dlist->currentblock->quantity - dlist->blocksize;
      cblock = dlist->firstblock;

      while(cblock != dlist->currentblock)
      {
         used += cblock->quantity;
         cblock = cblock->next;
      }
   }

   return used + 1;  /* add one to consider display list end entry */
}


/*--------------------------------------------------------------------------
 * function: d2_freedumpedbuffer
 * free a chunk of memory returned from d2_dumprenderbuffer.
 *
 * Dumped renderbuffers must be released using this function. Passing a
 * NULL pointer for 'data' is accepted and does nothing.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *  data - pointer to memory (created by <d2_dumprenderbuffer>)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_freedumpedbuffer( d2_device *handle, void *data )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE );  /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( data,   D2_INVALIDBUFFER );  /* PRQA S 3112 */ /* $Misra: #DEBUG_MACRO $*/

#ifndef _DEBUG
   if(NULL != data)
#endif /* _DEBUG */
   {
      d2_freemem_p( data );
   }

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * */
d2_s32 d2_initrblayer_intern( const d2_device *handle, d2_rb_layer *layer, d2_u32 size )
{
   /* unused parameter */
   (void) handle;           /* PRQA S 3112 */ /* $Misra: #COMPILER_WARNING $*/

   /* alloc scratch space */
   layer->scratch = (d2_dlist_scratch_entry*) d2_getmem_p( size * sizeof(d2_dlist_scratch_entry) );

   if(NULL == layer->scratch)
   {
      return 0;
   }

   /* initialize */
   layer->freesize = size;
   layer->fullsize = size;

   return 1;
}

/*--------------------------------------------------------------------------
 * */
static d2_s32 d2_resizerblayer_intern( const d2_device *handle, d2_rb_layer *layer )
{
   void *newadr;
   d2_u32 newsize, inc;

   /* unused parameter */
   (void) handle;           /* PRQA S 3112 */ /* $Misra: #COMPILER_WARNING $*/

   /* simple groth heuristic : double the size each time it reaches its limit */
   newsize = layer->fullsize << 1;
   newadr = d2_reallocmem_p( newsize * sizeof(d2_dlist_scratch_entry), layer->scratch, 1 );

   if(NULL == newadr)
   {
      return 0;
   }

   /* update layerinfo struct */
   inc = newsize - layer->fullsize;
   layer->freesize += inc;
   layer->fullsize  = newsize;
   layer->scratch   = newadr;

   return (d2_s32) inc;
}

/*--------------------------------------------------------------------------
 * */
static void d2_scratchgrowlayer_intern( d2_device *handle )
{
   d2_s32 inc;
   d2_devicedata *d2_dev = D2_DEV(handle);

   inc = d2_resizerblayer_intern( d2_dev, & d2_dev->selectedbuffer->layer[0] );

   if(0 == inc)
   {
      /* increasing the buffer failed. merge the existing data into
       * main display list as a 'best we can do' action. might destroy
       * order but does at least keep all primitives. */

      d2_layer2dlist_intern( handle );
      d2_dev->dlscratch_cnt = (d2_s32) d2_dev->selectedbuffer->layer[0].freesize;
      d2_dev->dlscratch_pos = d2_dev->dlscratch_base;
      return;
   }

   /* make new free space available for further scratch entries */
   d2_dev->dlscratch_cnt += inc;
   d2_dev->dlscratch_pos  = (d2_dev->dlscratch_pos - d2_dev->dlscratch_base) + d2_dev->selectedbuffer->layer[0].scratch;
   d2_dev->dlscratch_base = d2_dev->selectedbuffer->layer[0].scratch;
}

/*--------------------------------------------------------------------------
 * note - switches may not be nested (on a single device) as there is only
 *       one level of backup (no need for a stack).
 *     functions should never leave rendering in 'rendertolayer' mode
 *     when returning into application code
 * */
void d2_rendertolayer_intern( d2_device *handle )
{
   d2_devicedata *d2_dev = D2_DEV(handle);
   d2_rb_layer *rblayer;

   /* avoid layered buffering on lowmem configuration */
   if(0 != d2_dev->lowlocalmem_mode)
   {
      return;
   }

   /* switch target scratch buffer and buffer mode for buffered rendering */
   rblayer = &d2_dev->selectedbuffer->layer[0];

   rblayer->backup_base = d2_dev->dlscratch_base;
   rblayer->backup_pos  = d2_dev->dlscratch_pos;
   rblayer->backup_cnt  = d2_dev->dlscratch_cnt;
   rblayer->backup_hook = d2_dev->dlscratch_hook;

   d2_dev->dlscratch_base = rblayer->scratch;
   d2_dev->dlscratch_pos  = rblayer->scratch + (rblayer->fullsize - rblayer->freesize);
   d2_dev->dlscratch_cnt  = (d2_s32) rblayer->freesize;
   d2_dev->dlscratch_hook = &d2_scratchgrowlayer_intern;
}

void d2_rendertobase_intern( d2_device *handle )
{
   d2_devicedata *d2_dev = D2_DEV(handle);
   d2_rb_layer *rblayer;

   /* avoid layered buffering on lowmem configuration */
   if(0 != d2_dev->lowlocalmem_mode)
   {
      return;
   }

   /* switch target scratch buffer back to normal */
   rblayer = &d2_dev->selectedbuffer->layer[0];
   rblayer->freesize = (d2_u32) d2_dev->dlscratch_cnt;

   d2_dev->dlscratch_base = rblayer->backup_base;
   d2_dev->dlscratch_pos  = rblayer->backup_pos;
   d2_dev->dlscratch_cnt  = rblayer->backup_cnt;
   d2_dev->dlscratch_hook = rblayer->backup_hook;
}

/*--------------------------------------------------------------------------
 * */
void d2_layer2dlist_intern( d2_device *handle )
{
   d2_devicedata *d2_dev = D2_DEV(handle);
   d2_rb_layer *rblayer;
   d2_s32 targetlayer;

   /* avoid layered buffering on lowmem configuration */
   if(0 != d2_dev->lowlocalmem_mode)
   {
      return;
   }

   rblayer = &d2_dev->selectedbuffer->layer[0];

   if(rblayer->freesize == rblayer->fullsize)
   {
      return;
   }

   targetlayer = (d2_dev->dlscratch_hook == &d2_scratchgrowlayer_intern) ? 1 : 0;

   /* flush base scratch first */
   if(0 != targetlayer)
   {
      d2_rendertobase_intern(handle);
   }

   d2_scratch2dlist_intern( handle );
   d2_rendertolayer_intern(handle);

   /* copy scratch buffer to display list
    * use with layer as active scratch buffer */
   d2_scratch2dlist_intern( handle );

   d2_dev->dlscratch_cnt = (d2_s32) rblayer->fullsize;

   if(0 == targetlayer)
   {
      d2_rendertobase_intern(handle);
   }

   /* clear layer */
   rblayer->freesize = rblayer->fullsize;
}
