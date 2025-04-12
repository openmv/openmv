/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_driver.c (%version: 51 %)
 *          created Mon Jan 10 13:38:34 2005 by hh04027
 *
 * Description:
 *  %date_modified: Mon Mar 12 15:29:14 2007 %  (%derived_by:  hh74026 %)
 *
 * Changes:
 *  2005-12-22 CSe  removed transfermode field, added flags field in d2_opendevice()
 *  2006-02-16 CSe  added performance counter support + cache burstmodes
 *  2006-02-28 CSe  removed d2_df_dlist_big_endian which is now handled by hw
 *  2006-04-12 CSe  completed documentation for d2_opendevice
 *  2006-04-13 CSe  extended hwrevision for performance counter feature bit
 *  2006-05-10 CSe  moved initialization of dlists from d2_opendevice to d2_inithw
 *  2006-10-30 CSe  removed waitdlist flag
 *  2006-11-07 CSe  added dlist_buffer to device struct for 'd2_df_low_localmem' mode
 *  2007-03-01 MGe  added check of null values after creation of renderbuffers
 *  2007-03-12 CSe  fixed error in d2_lowlocalmemmode
 *  2007-05-23 MGe  added null checks in d2_inithw; initalize delayed_errorcode in context
 *  2007-08-30 ASc  replaced clib functions, removed tabs, added description for
 *                  d2_getrendermode, changed C++ to C comments, changed g_versionid
 *  2007-09-10 ASc  smaller updates of d2_lowlocalmemmode and d2_inithw
 *  2008-01-15 ASc  add error checking for d2_geterror and d2_geterrorstr
 *  2010-07-08 MRe  fixed support for register caching defined by D2_USEREGCACHE
 *  2010-07-08 MRe  new device flag d2_df_no_registercaching
 *  2011-03-10 MRe  fixed delayed error handling in d2_geterror
 *  2011-03-11 MRe  improved/removed context backup for blit
 *  2012-09-25 BSp  MISRA cleanup
 *  2013-06-25 MRe  changed check of dlistblocks in d2_lowlocalmemmode to >= 1
 *  2020-02-05 MRe  added d2_inithwburstlengthlimit
 *-------------------------------------------------------------------------- */
 
/*--------------------------------------------------------------------------
 *
 * Title: Basic Functions
 * Driver device management and hardware initialization / shutdown.
 *
 *-------------------------------------------------------------------------- */
#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_memory.h"
#include "dave_render.h"
#include "dave_dlist.h"

/*-------------------------------------------------------------------------- */
static d2_devicedata *g_devicechain = NULL;

#ifdef _DEBUG
#define _DEBUG_STR " _DEBUG "
#else
#define _DEBUG_STR ""
#endif 

/*---------------------------------------------------------------------------
 * List of hardware registers that can be cached for redundancy elimination
 * */
#ifdef D2_USEREGCACHE
const d2_u8 d2_cacheableregs[D2_QUANTITY] ={
   0,    /*D2_CONTROL      0 */
   1,    /*D2_CONTROL2     1 */
   0,    /*D2_CONTROL3     2 */
   0,    /*                3 */
   0,    /*D2_L1START      4 */
   0,    /*D2_L2START      5 */
   0,    /*D2_L3START      6 */
   0,    /*D2_L4START      7 */
   0,    /*D2_L5START      8 */
   0,    /*D2_L6START      9 */
   1,    /*D2_L1XADD       10 */
   1,    /*D2_L2XADD       11 */
   1,    /*D2_L3XADD       12 */
   1,    /*D2_L4XADD       13 */
   1,    /*D2_L5XADD       14 */
   1,    /*D2_L6XADD       15 */
   0,    /*D2_L1YADD       16 */
   1,    /*D2_L2YADD       17 */
   0,    /*D2_L3YADD       18 */
   1,    /*D2_L4YADD       19 */
   0,    /*D2_L5YADD       20 */
   1,    /*D2_L6YADD       21 */
   1,    /*D2_L1BAND       22 */
   1,    /*D2_L2BAND       23 */
   0,    /*                24 */
   1,    /*D2_COLOR1       25 */
   1,    /*D2_COLOR2       26 */
   0,    /*                27 */
   0,    /*                28 */
   1,    /*D2_PATTERN      29 */
   1,    /*D2_SIZE         30 */
   1,    /*D2_PITCH        31 */
   0,    /*D2_ORIGIN       32 */
   0,    /*                33 */
   0,    /*                34 */
   0,    /*                35 */
   0,    /*D2_LUSTART      36 */
   1,    /*D2_LUXADD       37 */
   1,    /*D2_LUYADD       38 */
   0,    /*D2_LVSTARTI     39 */
   0,    /*D2_LVSTARTF     40 */
   1,    /*D2_LVXADDI      41 */
   1,    /*D2_LVYADDI      42 */
   1,    /*D2_LVYXADDF     43 */
   0,    /*                44 */
   1,    /*D2_TEXPITCH     45 */
   1,    /*D2_TEXMASK      46 */
   1,    /*D2_TEXORIGIN    47 */
   0,    /*D2_IRQCTL       48 */
   0,    /*D2_CACHECTL     49 */
   0,    /*D2_DLISTSTART   50 */
   0,    /*D2_PERFCOUNT1   51 */
   0,    /*D2_PERFCOUNT2   52 */
   0,    /*D2_PERFTRIGGER  53 */
   0,    /*D2_TEXCLUT      54 */
   0,    /*D2_TEXCLUT_ADDR 55 */
   0,    /*D2_TEXCLUT_DATA 56 */
   1,    /*D2_TEXCLUT_OFFSET 57 */
   1     /*D2_COLKEY       58 */
};
#endif


/*--------------------------------------------------------------------------
 * Group: Static functions
 * */

static d2_contextdata_backup * d2_newbackupcontext_intern( d2_device *handle, d2_u32 flags );
static void d2_strcpy(d2_char *dst, const d2_char *src, d2_u32 dst_size);
static void d2_strcat(d2_char *dst, const d2_char *src, d2_u32 dst_size);
#ifdef NEED_D2_INTTOSTR
static void d2_inttostr(d2_char *a_dst, d2_u32 a_number, d2_u32 a_size);
#endif /* NEED_D2_INTTOSTR */
static void d2_hextostr(d2_char *a_dst, d2_u32 a_number, d2_u32 a_size);

/*--------------------------------------------------------------------------
 * Group: Device management
 * */

/*--------------------------------------------------------------------------
 * function: d2_getversionstring
 * Query versionID string (device independent)
 *
 * returns:
 *  human readable driver version as a string.
 *
 * see also:
 *  <d2_getversion>
 * */
const d2_char * d2_getversionstring( void )
{
   static const d2_char g_versionid[] ="@(#)DAVE driver " D2_VERSION_STRING D2_VERSION_STATE " - " _DEBUG_STR " - " __DATE__ " " __TIME__ ;

   return g_versionid;
}

/*--------------------------------------------------------------------------
 * function: d2_getversion
 * Query versionID (device independent)
 *
 * returns:
 *  driver revision as a single 32bit integer 
 * 
 * bits 31..24 - branch number
 * bits 23..16 - major version number
 * bits 15..0  - minor version number
 *
 * see also:
 *  <d2_getversionstring>
 * */
d2_s32 d2_getversion( void )
{
   return D2_VERSION;
}

/*--------------------------------------------------------------------------
 * d2_newbackupcontext_intern
 * Create a backup area for context data that is used by blit.
 * */
static d2_contextdata_backup * d2_newbackupcontext_intern( d2_device *handle, d2_u32 flags )
{
   d2_contextdata_backup *ctx;

   if(0 != (flags & d2_df_no_blitctxbackup))
   {
      ctx = 0;
   }
   else
   {
      ctx = (d2_contextdata_backup *) d2_getmem_p( sizeof(d2_contextdata_backup) );
   }

   (void)D2_SETOK(handle);
   return ctx;
}

/*--------------------------------------------------------------------------
 * function: d2_opendevice
 * Create a new device handle.
 *
 * A device is the basic software object. It contains references to objects
 * that the drawing engine requires, (e.g.: displaylists). 
 * All 2D drawing functions require such a device pointer as first parameter. 
 *
 * A device on its own is useless, unless bound to a physical hardware (i.e.: Drawing Engine).
 * The binding is done by the function <d2_inithw>. A hardware instance can only be mapped to
 * one device object.
 * It is not possible to share a single device pointer between different processes.
 *
 * Creating a device will never fail (sufficient memory assumed) but binding
 * it to a hardware (see: <d2_inithw>) can fail.
 *
 * parameters:
 *  flags - a bitfield containing flags
 *
 * flags:
 *  d2_df_no_dlist           - don't use a display list (slower single command mode)
 *  d2_df_no_irq             - don't use an interrupt (slower polling used instead)
 *  d2_df_no_fbcache         - disable framebuffer cache (attention: see note below)
 *  d2_df_no_texcache        - disable texture cache (attention: see note below)
 *  d2_df_no_dwclear         - disable double word clearing in d2_clear
 *  d2_df_no_registercaching - don't use register caching
 *  d2_df_no_blitctxbackup   - don't backup context data at blit for better performance; previous texture modes get lost and must be set again
 *
 * note:
 *  Flags 'd2_df_no_fbcache' and 'd2_df_no_texcache' should only be used for debugging purposes.
 *  Especially when multithreading is used, these flags must not be set different for devices which are bound to the same hardware instance through
 *  <d2_inithw>.
 *
 * returns:
 *  device pointer or NULL if not enough memory was available
 * */
d2_device * d2_opendevice( d2_u32 flags )
{
#ifdef D2_USEREGCACHE
   d2_s32 i;
#endif

   d2_devicedata *handle = (d2_devicedata *) d2_getmem_p( sizeof(d2_devicedata) );

   if(NULL != handle)
   {
      /* initialize device context */
      handle->flags             = flags;
      handle->errorcode         = D2_OK;
      handle->delayed_errorcode = D2_OK;
      handle->hwid              = NULL;
      handle->ctxchain          = NULL;
      handle->fbformat          = 2;
      handle->fbwidth           = 0;
      handle->fbheight          = 0;
      handle->fbstylemask       = D2C_WRITEFORMAT2;
      handle->ctxdef            = d2_newcontext( handle );   /* create default context */
      handle->ctxselected       = handle->ctxdef;
      handle->ctxsolid          = handle->ctxdef;
      handle->ctxoutline        = handle->ctxdef;
#ifdef D2_USEBLITBACKUPCONTEXT
      handle->blitcontext       = d2_newcontext( handle ); /* create blit context */
#else
      handle->blitcontext_b     = d2_newbackupcontext_intern( handle, flags ); /* create backup for blit context */
#endif
      handle->srccontext        = NULL;
      handle->srcclut           = NULL;
      handle->lasttextwasrle    = 0;
      /* default rendering parameters */
      handle->framebuffer       = NULL;
      handle->pitch             = 0;
      handle->bpp               = 0;
      handle->rendermode        = d2_rm_solid;
      handle->outlinewidth      = 0;
      handle->soffx             = 0;
      handle->soffy             = 0;
      handle->clipxmin          = 0;
      handle->clipymin          = 0;
      handle->clipxmax          = 0;
      handle->clipymax          = 0;
      /* initialize default display lists */
      handle->renderbuffer[0]   = NULL;
      handle->renderbuffer[1]   = NULL;
      /* preselect list */
      handle->selectedbuffer   = NULL;
      handle->writelist        = NULL;
      handle->readlist         = NULL;
      handle->dlistblocksize   = D2_DLISTBLOCKSIZE;

      /* 0x30 is for enabling burstmode of caches -> can be removed for later versions */
      handle->cachectlmask     = D2C_CACHECTL_ENABLE_FB | D2C_CACHECTL_ENABLE_TX;

      if(0 != (flags & d2_df_no_fbcache))
      {
         handle->cachectlmask &= ~D2C_CACHECTL_ENABLE_FB;
      }

      if(0 != (flags & d2_df_no_texcache))
      {
         handle->cachectlmask &= ~D2C_CACHECTL_ENABLE_TX;
      }

      handle->perftriggermask  = 0;
      handle->lowlocalmem_mode = NULL;

      /* clear register cache */
#ifdef D2_USEREGCACHE
      for(i=0; i<D2_QUANTITY; i++)
      {
         handle->cache.data[i]  = 0;
         handle->cache.valid[i] = 0;
      }
#endif

      /* hw revision */
      handle->hwrevision                   = 0;
      handle->hilimiterprecision_supported = 0;
      handle->maxpatlen                    = 8;

      /* attach to chain */
      handle->next = g_devicechain;
      g_devicechain = handle;
   }

   return handle;
}

/*--------------------------------------------------------------------------
 * function: d2_closedevice
 * Destroy a device handle.
 *
 * All contexts associated with the device are destroyed as well.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_closedevice( d2_device *handle )
{
   d2_s32 result;
   d2_devicedata **prev;
   D2_VALIDATE( handle, D2_INVALIDDEVICE); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* release hardware if assigned */
   if(NULL != D2_DEV(handle)->hwid)
   {
      result = d2_deinithw( handle );
   }
   else
   {
      result = D2_OK;
   }

   /* find in chain */
   prev = &g_devicechain;
   while( (NULL != prev) && (*prev != handle))
   {
      prev = (d2_devicedata **) *prev;
   }

   if(NULL == prev)
   {
      return D2_INVALIDDEVICE;
   }

   /* remove from chain */
   *prev = D2_DEV(handle)->next;

   /* free render contexts */
   D2_DEV(handle)->ctxdef = NULL; /* necessary in order to free default context */

   while(NULL != D2_DEV(handle)->ctxchain)
   {
      (void)d2_freecontext(D2_DEV(handle), D2_DEV(handle)->ctxchain);
   }

   if(0 != D2_DEV(handle)->lowlocalmem_mode)
   {
      d2_freemem_p(D2_DEV(handle)->lowlocalmem_mode);
   }

#ifdef D2_USEBLITBACKUPCONTEXT
   D2_DEV(handle)->blitcontext = NULL; /* was freed by chain */
#else
   if(NULL != D2_DEV(handle)->blitcontext_b)
   {
      d2_freemem_p(D2_DEV(handle)->blitcontext_b);
   }
   D2_DEV(handle)->blitcontext_b = NULL;
#endif

   /* free context memory */
   d2_freemem_p( handle );

   return result;
}

/*--------------------------------------------------------------------------
 * function: d2_geterror
 * Query device error information.
 *
 * See list of all <Errorcodes> for more details.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   integer error code (0 is no error)
 * */
d2_s32 d2_geterror( const d2_device *handle )
{
   d2_s32 errorCode;

   if(NULL != handle)
   {
      /*D2_VALIDATE( handle, D2_INVALIDDEVICE);*/ /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

      errorCode = D2_DEV(handle)->errorcode;

      if(errorCode == D2_OK)
      {
         errorCode = D2_DEV(handle)->delayed_errorcode;
      }
   }
   else
   {
      errorCode = D2_INVALIDDEVICE;
   }

   return errorCode;
}

/*--------------------------------------------------------------------------
 * function: d2_geterrorstring
 * Query detailed device error information.
 *
 * See list of all <Errorcodes> for more details.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   string with human readable error description, or 0 if an error occurs
 * */
const d2_char * d2_geterrorstring( const d2_device *handle )
{
   D2_VALIDATEP( handle, D2_INVALIDDEVICE); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   return d2_translateerror( d2_geterror( handle ) );
}

/*--------------------------------------------------------------------------
 * Function: d2_inithw
 *
 * Initialize hardware for working with specified device.
 * After creation a device is 'bound' to a hardware instance by calling
 * inithw for this device. A single device cannot work with multiple
 * hardware units.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   flags - hardware instance id (use 0 for default)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_inithw( d2_device *handle, d2_u32 flags )
{
#ifdef D2_USEREGCACHE
   d2_s32 i;
#endif
   d2_devicedata *d2_dev = D2_DEV(handle);

   D2_VALIDATE( handle, D2_INVALIDDEVICE);             /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( d2_dev->hwid == NULL, D2_DEVASSIGNED); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   d2_dev->hwid = d2hw_acquire( handle, flags );
   if(NULL == d2_dev->hwid)
   {
      D2_RETERR(handle, D2_HWINUSE);
   }

   /* invalidate register cache */
#ifdef D2_USEREGCACHE
   for(i=0; i<D2_QUANTITY; i++)
   {
      d2_dev->cache.valid[i] = 0;
   }
#endif

   /* setup a ring of two display list blocks as local buffer for low_localmem mode */
   if(0 != d2_dev->lowlocalmem_mode)
   {
      d2_lowlocalmem_mode* llm_mode = d2_dev->lowlocalmem_mode;

      llm_mode->dlist_buffer = d2_alloc_dlistblock_intern(handle, d2_dev->dlistblocksize);
      D2_CHECKERR( llm_mode->dlist_buffer, D2_NOMEMORY ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
      llm_mode->dlist_buffer->next = d2_alloc_dlistblock_intern(handle, d2_dev->dlistblocksize);
      D2_CHECKERR( llm_mode->dlist_buffer->next, D2_NOMEMORY ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
      llm_mode->dlist_buffer->next->next = llm_mode->dlist_buffer;
   }

   /* initialize default display lists (allocates memory -> can't be done before
    * lowlevel initialization!!) */
   d2_dev->renderbuffer[0] = (d2_rbuffer*) d2_newrenderbuffer( handle, d2_dev->dlistblocksize, d2_dev->dlistblocksize );
   d2_dev->renderbuffer[1] = (d2_rbuffer*) d2_newrenderbuffer( handle, d2_dev->dlistblocksize, d2_dev->dlistblocksize );

   if( (NULL == d2_dev->renderbuffer[0]) || (NULL == d2_dev->renderbuffer[1]) )
   {
      (void)D2_SETERR( handle, D2_NOMEMORY);
      return D2_NOMEMORY;
   }

   /* preselect list */
   d2_dev->selectedbuffer = d2_dev->renderbuffer[0];
   d2_dev->writelist      = &d2_dev->renderbuffer[0]->baselist;
   d2_dev->readlist       = &d2_dev->renderbuffer[1]->baselist;
   d2_dev->srccontext     = NULL;

   /* allocate and init list assembly scratch buffer */
   d2_dev->dlscratch_base = (d2_dlist_scratch_entry*) d2_getmem_p( D2_DLISTSCRATCH * sizeof(d2_dlist_scratch_entry) );
   D2_CHECKERR( d2_dev->dlscratch_base, D2_NOMEMORY ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   d2_dev->dlscratch_pos  = d2_dev->dlscratch_base;
   d2_dev->dlscratch_cnt  = D2_DLISTSCRATCH;
   d2_dev->dlscratch_hook = &d2_scratch2dlist_intern;


   d2_dev->dlist_indirect_supported = 0;
   if(0 != d1_devicesupported(d2_dev->hwid, D1_DLISTINDIRECT))
   {
      /* query d1 driver to use lists of dlist start addresses */
      d1_setregister(d2_dev->hwid, D1_DLISTINDIRECT, 0, 0x000000ff);
      if(0x000000ff == d1_getregister(d2_dev->hwid, D1_DLISTINDIRECT, 0))
      {
         d2_dev->dlist_indirect_supported = 1;
      }
   }

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_deinithw
 * Unlink hardware currently bound by specified device.
 * Hardware must be deinitialized before it can be reinitalized for another
 * device.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_deinithw( d2_device *handle )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   /* error if no hw assigned ? */

   if(NULL != D2_DEV(handle)->hwid)
   {
      d2_s32 err;
      d2_rbuffer *rb1,*rb2;

      if(0 != D2_DEV(handle)->lowlocalmem_mode)
      {
         /* free the ring of displaylist blocks */
         D2_DEV(handle)->lowlocalmem_mode->dlist_buffer->next->next = NULL;
         d2_free_dlistblock_intern(handle, D2_DEV(handle)->lowlocalmem_mode->dlist_buffer);
      }

      /* free display lists */
      rb1 = D2_DEV(handle)->renderbuffer[0];
      rb2 = D2_DEV(handle)->renderbuffer[1];
      D2_DEV(handle)->renderbuffer[0] = NULL; /* necessary in oder to free default buffer */
      D2_DEV(handle)->renderbuffer[1] = NULL;
      if(NULL != rb1)
      {
         (void)d2_freerenderbuffer( handle, rb1 );
      }
      if(NULL != rb2)
      {
         (void)d2_freerenderbuffer( handle, rb2 );
      }
      D2_DEV(handle)->selectedbuffer = NULL;
      D2_DEV(handle)->writelist      = NULL;
      D2_DEV(handle)->readlist       = NULL;
      D2_DEV(handle)->srccontext     = NULL;

      /* free scratch buffer */
      d2_freemem_p(D2_DEV(handle)->dlscratch_base);
      D2_DEV(handle)->dlscratch_base = NULL;
      D2_DEV(handle)->dlscratch_pos  = NULL;

      err = d2hw_release( D2_DEV(handle)->hwid );
      D2_DEV(handle)->hwid = NULL;
      D2_RETERR(handle, err);
   }

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
* function: d2_inithwburstlengthlimit
* Set the maximum burst lenght of the master bus interfaces.
* Changing the burst length limit should be done only when Dave completely is in idle mode
* and all bus transfers are finished.
*
* parameters:
*   handle - device pointer (see: <d2_opendevice>)
*   burstlengthFBread - max burst length of framebuffer read interface
*   burstlengthFBwrite - max burst length of framebuffer write interface
*   burstlengthTX - max burst length of texture cache read interface
*   burstlengthDL - max burst length of displaylist read interface
*
* d2_busburstlength:
*   d2_bbl_1  -  single cycle bus access
*   d2_bbl_2  -  max bus burst length = 2
*   d2_bbl_4  -  max bus burst length = 4
*   d2_bbl_8  -  max bus burst length = 8
*   d2_bbl_16 -  max bus burst length = 16
*   d2_bbl_32 -  max bus burst length = 32
*
* returns:
*   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
* */
d2_s32  d2_inithwburstlengthlimit(d2_device *handle, d2_busburstlength burstlengthFBread, d2_busburstlength burstlengthFBwrite, d2_busburstlength burstlengthTX, d2_busburstlength burstlengthDL)
{
    d2hw_set(D2_DEV(handle)->hwid, D2_CONTROL3, burstlengthFBread | (burstlengthFBwrite << 8) | (burstlengthTX << 16) | (burstlengthDL << 24));
    D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_level1interface
 * Get the lowlevel device handle currently used by the d2_device. This is necessary
 * e.g. for allocating framebuffers using the lowlevel (d1_) interface.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   handle for lowlevel device, or NULL if an error occurs
 * */
d1_device * d2_level1interface( const d2_device *handle )
{
   D2_VALIDATEP( handle, D2_INVALIDDEVICE); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   return D2_DEV(handle)->hwid;
}

/*--------------------------------------------------------------------------
 * function: d2_getrevisionhw
 * Query hw revisionID.
 * This information is available after calling <d2_inithw>.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 * HW Revision ID
 *
 * HW Revision ID structure:
 * Bit[7..0]   - revision number
 * Bit[11..8]  - branch number
 * Bit[15..12] - D/AVE Type
 * Bit[20..16] - Features
 *
 * D/AVE Type:
 * 0 - D/AVE2DT-S
 * 1 - D/AVE2DT-L
 *
 * Feature bits:
 * Bit[16] - D2FB_SWDAVE (Software D/AVE)
 * Bit[17] - D2FB_DLR (DisplayListReader available)
 * Bit[18] - D2FB_FBCACHE (Framebuffer Cache available)
 * Bit[19] - D2FB_TXCACHE (Texture Cache available)
 * Bit[20] - D2FB_PERFCOUNT (Two performance counters available)
 * Bit[21] - D2FB_TEXCLUT (Color Lookup Table for ai44 format)
 * Bit[22] - D2FB_FBPREFETCH (frame buffer cache prefetch available)
 * Bit[23] - D2FB_RLEUNIT (RLE unit available)
 * Bit[24] - D2FB_TEXCLUT256 (256 entry CLUT available)
 * Bit[25] - D2FB_COLORKEY (color keying available)
 * Bit[26] - D2FB_HILIMITERPRECISION (limiter high precision mode available)
 * Bit[27] - D2FB_ALPHACHANNELBLENDING (alpha channel blending available)
 *
 * see also:
 *  <d2_getrevisionstringhw>
 * */
d2_u32 d2_getrevisionhw( const d2_device *handle )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   return D2_DEV(handle)->hwrevision;
}

/*--------------------------------------------------------------------------
 * Copy a string.
 * 
 * dst      - Destination string 
 * src      - Null-terminated source string
 * dst_size - Size of destination string in chars
 * */
static void d2_strcpy(d2_char *dst, const d2_char *src, d2_u32 dst_size)
{
   if( (NULL == dst) || (0 == dst_size) )
   {
      return;
   }

   --dst_size;
   while( (0 != *src) && (0 != dst_size) )
   {
      *dst = *src;
      dst++;
      src++;
      --dst_size;
   }
   *dst = 0;
}


/*--------------------------------------------------------------------------
 * Append a string.
 * 
 * dst      - Destination string 
 * src      - Null-terminated source string
 * dst_size - Size of destination string in chars
 * */
static void d2_strcat(d2_char *dst, const d2_char *src, d2_u32 dst_size)
{
   if(NULL == dst)
   {
      return;
   }

   --dst_size;
   while( (0 != *dst) && (0 != dst_size) )
   {
      dst++;
   }

   d2_strcpy(dst, src, dst_size);
}


/*--------------------------------------------------------------------------
 * Convert an unsigned integer to a string.
 * 
 * a_dst    - Destination string 
 * a_number - unsigned integer number
 * a_size   - Size of destination string in chars
 * */
#ifdef NEED_D2_INTTOSTR
static void d2_inttostr(d2_char *a_dst, d2_u32 a_number, d2_u32 a_size)
{
   d2_u32 digits = 0;
   d2_u32 divisor = 1;
   d2_u32 num = a_number;
   a_size--;

   /* find number of digits */
   do
   {
      num /= 10;
      divisor *= 10;
      digits++;
   } while (num > 0);

   /* convert to string */
   while( (a_size > 0) && (digits > 0) )
   {    
      d2_u32 digit;

      divisor /= 10;
      digit = a_number / divisor;
      *a_dst = (d2_char) (digit + '0');
      a_dst++;
      a_number -= digit * divisor;
      a_size--;
      digits--;    
   }

   /* add termination */
   *a_dst = 0;
}
#endif /* NEED_D2_INTTOSTR */

/*--------------------------------------------------------------------------
 * Convert an unsigned integer to a hex string.
 * 
 * a_dst    - Destination string 
 * a_number - unsigned integer number
 * a_size   - Size of destination string in chars
 * */
static void d2_hextostr(d2_char *a_dst, d2_u32 a_number, d2_u32 a_size)
{
   d2_u32 digits = 0;
   d2_u32 divisor = 1;
   d2_u32 num = a_number;
   a_size--;

   /* find number of digits */
   do
   {
      num /= 16;
      divisor *= 16;
      digits++;
   } while (num > 0);

   /* convert to string */
   while( (a_size > 0) && (digits > 0) )
   {    
      d2_u32 digit;

      divisor /= 16;
      digit = a_number / divisor;

      if (digit < 10)
      {
         *a_dst = (d2_char) (digit + '0');
         a_dst++;
      }
      else if (digit < 16)
      {
         *a_dst = (d2_char) ((digit - 10) + 'a');
         a_dst++;
      }
      else
      {
         /* empty else block to satisfy MISRA rule 14.10/2004 */
      }
      /* else if (digit < 16)
         {
           *a_dst = '-';
           a_dst++;
         }
      */ /* (note) block never reached */
      a_number -= digit * divisor;
      a_size--;
      digits--;    
   }

   /* add termination */
   *a_dst = 0;
}

/*--------------------------------------------------------------------------
 * function: d2_getrevisionstringhw
 * Query hw revisionID string.
 * This information is available after calling <d2_inithw>.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *  human readable hw revision string, or 0 if an error occurs
 *  (e.g. "D/AVE 2DT-S, Revision 1.05, Features: DLR FBCACHE").
 *
 * see also:
 *  <d2_getrevisionhw>
 * */
const d2_char * d2_getrevisionstringhw( const d2_device *handle)
{
   static d2_char g_revisionstring[256];
   static const d2_char g_hwrevision[3][16] = { {"D/AVE 2DT-S"},
                                                {"D/AVE 2DT-L"},
                                                {"D/AVE UNKNOWN"}
   };
   static const d2_char g_hwfeatures[12][64] = { {"SWDAVE"},
                                                 {"DLR"},
                                                 {"FBCACHE"},
                                                 {"TXCACHE"},
                                                 {"PERFCOUNT"},
                                                 {"TEXCLUT"},
                                                 {"TEXCLUT256"},
                                                 {"TEXRLE"},
                                                 {"TEXCOLORKEY"},
                                                 {"HILIMITERPRECISION"},
                                                 {"ALPHACHANNELBLENDING"},
                                                 {"???"}
   };
   d2_u32 hwrev;
   d2_u32 nb;
   d2_u32 type;
   d2_char strnb[11];
   const d2_u32 bits[] = { D2FB_SWDAVE, D2FB_DLR, D2FB_FBCACHE, D2FB_TXCACHE, D2FB_PERFCOUNT, D2FB_TEXCLUT, D2FB_TEXCLUT256, D2FB_RLEUNIT, D2FB_COLORKEY, D2FB_HILIMITERPRECISION, D2FB_ALPHACHANNELBLENDING };
   d2_u32 i = 0;

   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   hwrev    = D2_DEV(handle)->hwrevision;
   nb       = hwrev & 4095u;
   type     = (hwrev >> 12) & 15u;

   if(type > 1)
   {
      type = 2;
   }

   d2_strcpy(g_revisionstring, g_hwrevision[type], sizeof(g_revisionstring) );
   d2_strcat(g_revisionstring, ", Revision ", sizeof(g_revisionstring) );
   d2_hextostr(strnb, (nb & 0xf00u) >> 8, sizeof(strnb) );
   d2_strcat(g_revisionstring, strnb, sizeof(g_revisionstring) );
   d2_strcat(g_revisionstring, ".", sizeof(g_revisionstring) );
   d2_hextostr(strnb, (nb & 0x0ffu) >> 0, sizeof(strnb) );
   d2_strcat(g_revisionstring, strnb, sizeof(g_revisionstring) );
   d2_strcat(g_revisionstring, ", Features: ", sizeof(g_revisionstring) );

   for(i=0; i < (sizeof(bits) / sizeof(d2_s32)); ++i)
   {
      if( (hwrev & bits[i]) != 0 )
      {
         d2_strcat(g_revisionstring, g_hwfeatures[i], sizeof(g_revisionstring) );
         d2_strcat(g_revisionstring, " ", sizeof(g_revisionstring) );
      }
   }

   return g_revisionstring;
}


/*--------------------------------------------------------------------------
 * function: d2_lowlocalmemmode
 * Enable and configure the 'low localmem' mode.
 *
 * On systems with low local CPU memory, the display lists can not be completely
 * assembled in the local memory. A special mode is used in this case, which
 * assembles small display list blocks in the local memory and copies them to
 * the video memory, where they are concatenated to larger blocks.
 * In order to use this mode it is necessary to call d2_lowlocalmemmode directly
 * after <d2_opendevice> and before <d2_inithw>.
 *
 * Querying the number of effectively used display list blocks can be done using
 * <d2_getdlistblockcount>, which gives the number of display list blocks used so far
 * (in units of the local display list block size).
 * The application developer must take care that the maximum display list size configured
 * using <d2_lowlocalmemmode> is sufficient.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   dlistblockfactor - size of a dlist block in vidmem is this factor * local size (configurable using <d2_setdlistblocksize>)
 *   dlistblocks - maximum number of dlist blocks in vidmem
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_setdlistblocksize>
 * */
d2_s32 d2_lowlocalmemmode(d2_device *handle, d2_u32 dlistblockfactor, d2_u32 dlistblocks)
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   D2_CHECKERR(dlistblockfactor >= 2, D2_VALUETOOSMALL); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR(dlistblocks >= 1, D2_VALUETOOSMALL);      /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 != (D2_DEV(handle)->flags & d2_df_no_dlist))
   {
      D2_RETERR( handle, D2_NO_DISPLAYLIST);
   }

   if(NULL == D2_DEV(handle)->lowlocalmem_mode)
   {
      D2_DEV(handle)->lowlocalmem_mode = d2_getmem_p(sizeof(d2_lowlocalmem_mode));
      D2_CHECKERR(D2_DEV(handle)->lowlocalmem_mode, D2_NOMEMORY); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   }

   D2_DEV(handle)->lowlocalmem_mode->vidmemblocksizefactor = dlistblockfactor;
   D2_DEV(handle)->lowlocalmem_mode->vidmemblocks = dlistblocks;

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * Group: Rendering Mode
 * */

/*--------------------------------------------------------------------------
 * function: d2_selectrendermode
 * Select a rendering mode.
 * Dave can automatically generate and render geometry outlines and shadows.
 * A different rendering context is used for interior (solid context) and addons
 * (outline context) so that both parts can have entirely different materials.
 *
 * Note that primitives which are issued while rendermode 'postprocess' is active
 * are buffered and put into the commandlist after all normal commands.
 * Combined rendermodes like 'solid_outlined' and 'solid_shadow' internally use
 * postprocessing to sort solid and outline parts.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   mode - rendering mode
 *
 * available rendering modes:
 *   d2_rm_solid - Direct rendering of primitives (default mode)
 *   d2_rm_outline - Only outlines are rendered
 *   d2_rm_solid_outlined - Interior and outlines are rendered
 *   d2_rm_shadow - Only shadows are rendered
 *   d2_rm_solid_shadow - Interior and shadows are rendered
 *  d2_rm_postprocess - Direct rendering of primitives as a postprocess
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_outlinewidth>, <d2_shadowoffset>
 * */
d2_s32 d2_selectrendermode( d2_device *handle, d2_u32 mode )
{
   d2_u32 oldmode;

   if(NULL == handle)
   {
      return D2_INVALIDDEVICE;
   }
   /*D2_VALIDATE( handle, D2_INVALIDDEVICE );*/ /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( ((mode & ( d2_rm_solid | d2_rm_outline | d2_rm_shadow | d2_rm_postprocess)) == mode) && (mode != 0), D2_ILLEGALMODE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   oldmode = D2_DEV(handle)->rendermode;
   if(oldmode == mode)
   {
      D2_RETOK(handle);
   }

   if( (oldmode != mode) && (oldmode != d2_rm_postprocess) && (mode != d2_rm_postprocess) )
   {
      /* merge in layers if they contain data */
      (void)d2_layermerge( handle );
   }

   D2_DEV(handle)->rendermode = mode;
   D2_DEV(handle)->ctxselected->internaldirty |= d2_dirty_material;

   /* change target buffer if postprocess mode */
   if(d2_rm_postprocess == oldmode)
   {
      d2_rendertobase_intern( handle );
   }

   if(d2_rm_postprocess == mode)
   {
      d2_rendertolayer_intern( handle );
   }

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_getrendermode
 * Get the rendering mode.
 * This function can be used to get the rendering mode used for a 
 * specified d2_device.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * available rendering modes:
 *   d2_rm_solid - Direct rendering of primitives (default mode)
 *   d2_rm_outline - Only outlines are rendered
 *   d2_rm_solid_outlined - Interior and outlines are rendered
 *   d2_rm_shadow - Only shadows are rendered
 *   d2_rm_solid_shadow - Interior and shadows are rendered
 *   d2_rm_postprocess - Direct rendering of primitives as a postprocess
 *
 * returns:
 *   integer specifying the rendering mode, or 0 if an error occurs
 *
 * */
d2_u32 d2_getrendermode( const d2_device *handle )
{
   if(NULL == handle)
   {
      return 0;
   }
   else
   {
      return D2_DEV(handle)->rendermode;
   }
}

/*--------------------------------------------------------------------------
 * function: d2_layermerge
 * Join outline and solid parts of currently selected renderbuffer. When
 * using rendermode *d2_rm_postprocess* (see: <d2_selectrendermode>) the
 * postprocess buffer can be flushed using a call to d2_layermerge.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_selectrendermode>
 * */
d2_s32 d2_layermerge( d2_device *handle )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   d2_layer2dlist_intern( handle );
   D2_DEV(handle)->ctxselected->internaldirty |= d2_dirty_material;

   D2_RETOK(handle);
}


/*--------------------------------------------------------------------------
 * function: d2_outlinewidth
 * Define the width of geometry outlines. Used only when using outline or
 * solid_outlined rendering modes.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   width - outline width in pixels (fixedpoint)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_selectrendermode>, <d2_shadowoffset>
 * */
d2_s32 d2_outlinewidth( d2_device *handle, d2_width width )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE );    /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( width >= 0, D2_INVALIDWIDTH ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   D2_DEV(handle)->outlinewidth = width;

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_shadowoffset
 * Define the offset of geometry shadows. Used only when using shadow or
 * solid_shadow rendering modes.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   x - x axis offset in pixels (fixedpoint)
 *   y - y axis offset in pixels (fixedpoint)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_selectrendermode>, <d2_outlinewidth>
 * */
d2_s32 d2_shadowoffset( d2_device *handle, d2_point x, d2_point y )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   D2_DEV(handle)->soffx = x;
   D2_DEV(handle)->soffy = y;

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * Group: Utility Functions
 * */


/*--------------------------------------------------------------------------
 * function: d2_flushframe
 * Wait for current rendering to end.
 *
 * Function will not return until the hardware has finished executing all
 * currently active rendering operations. Note that no rendering operations
 * are started until a renderbuffer is executed. (See <Render Buffers>)
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_flushframe( d2_device *handle )
{
   if(NULL != handle)
   {
      /*D2_VALIDATE( handle, D2_INVALIDDEVICE );*/ /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

      /* wait for current frame rendering to end */
      d2hw_finish( handle );

      D2_RETOK(handle);
   }
   else
   {
      return D2_INVALIDDEVICE;
   }
}

/*--------------------------------------------------------------------------
 * function: d2_setdlistblocksize
 * Set blocksize for default displaylists.
 *
 * Sets the number of displaylist entries per block (aka page).
 * The value given is effectively used for internal calls to <d2_newrenderbuffer>
 * as both 'initialsize' and 'stepsize'.
 * For hints on how to optimize this size see documentation of <d2_newrenderbuffer>.
 *
 * The default blocksize is: 204
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   size   - number of displaylist entries per block (minimum is 3)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * note:
 *   The minimum number of displaylist entries per block is 3. This 
 *   limitation is required to terminate a list correctly, there must be
 *   enough space to insert a dlist jump and a special termination entry.
 *
 * see also:
 *   <d2_getdlistblocksize>, <d2_newrenderbuffer>
 * */
d2_s32 d2_setdlistblocksize(d2_device *handle, d2_u32 size)
{
   if(NULL != handle)
   {
      /*D2_VALIDATE( handle, D2_INVALIDDEVICE );*/ /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

      D2_CHECKERR(size > 2, D2_VALUETOOSMALL); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

      D2_DEV(handle)->dlistblocksize = size;

      D2_RETOK(handle);
   }
   else
   {
      return D2_INVALIDDEVICE;
   }
}

/*--------------------------------------------------------------------------
 * function: d2_getdlistblocksize
 * Get blocksize of default displaylist.
 *
 * Function will return the number of displaylist entries per block
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   number of displaylist entries per block, or 0 if an error occurs
 *
 * see also:
 *   <d2_setdlistblocksize>, <d2_newrenderbuffer>
 * */
d2_u32 d2_getdlistblocksize(const d2_device *handle)
{
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   return D2_DEV(handle)->dlistblocksize;
}

/*--------------------------------------------------------------------------
 * function: d2_getdlistblockcount
 * Get number of blocks of default displaylist (writelist).
 *
 * Function will return the number of used blocks for current displaylist
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   number of used blocks
 *
 * see also:
 *   <d2_setdlistblocksize>, <d2_newrenderbuffer>
 * */
d2_u32 d2_getdlistblockcount(d2_device *handle)
{
   d2_dlist          *dlist;
   d2_dlist_block    *cblock;
   d2_u32             num = 1;

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   dlist = &D2_DEV(handle)->selectedbuffer->baselist;
   cblock = dlist->firstblock;

   /* clear scratch before counting */
   d2_scratch2dlist_intern(handle);

   if(NULL != dlist->vidmem_blocks)
   {
      num += ((d2_u32)(dlist->vidmem_blocks->currentblock - dlist->vidmem_blocks->blocks)) * dlist->vidmem_blocks->num_slices;
      num += dlist->vidmem_blocks->num_slices - dlist->vidmem_blocks->slicesleft;
   }
   else
   {
      while(cblock != dlist->currentblock)
      {
         cblock = cblock->next;
         ++num;
      }
   }

   return num;
}

/*--------------------------------------------------------------------------
 * function: d2_commandspending
 * Check if there are pending commands in the current displaylist.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   boolean value: true when there are commands pending, also 0 if an error occurs
 *
 * see also:
 *   <d2_getdlistblockcount>
 * */
d2_s32 d2_commandspending(d2_device *handle)
{
   d2_dlist *dlist;

   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   dlist = &D2_DEV(handle)->selectedbuffer->baselist;

   /* clear scratch before check */
   d2_scratch2dlist_intern(handle);

   if(dlist->position != dlist->firstblock->block)
   {
      return 1; /* this means commands pending in any case */
   }

   /* in lowlocalmem mode, we might be at the beginning of the first block, but in
    * another vidmem block */
   if( (NULL != dlist->vidmem_blocks) && (dlist->vidmem_blocks->currentaddress == dlist->vidmem_blocks->blocks[0]) )
   {
      return 1;
   }
   else
   {
      return 0;
   }
}
