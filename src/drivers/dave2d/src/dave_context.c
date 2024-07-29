/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_context.c (%version: 42 %)
 *          created Tue Jan 11 16:25:20 2005 by hh04027
 *
 * Description:
 *  %date_modified: Tue Dec 12 13:43:03 2006 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2007-12-05 ASc  bugfix: return correct value in d2_getlinejoin,
 *                  fix description for d2_getpatternsize and d2_setpatternsize
 *  2007-12-06 ASc  bugfix: d2_getcontext did not return outline context
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs 
 *  2008-09-25 MRe  added d2_setcircleextend
 *  2010-02-18 MRe  added true alpha blending to d2_setalphablendmode
 *  2010-03-01 MRe  added d2_getalphablendmodesrc, d2_getalphablendmodedst,
 *  2010-03-18 MRe  added d2_setalphablendmodeex, d2_getalphablendmodeflags
 *  2011-09-22 MRe  fixed default for ctx->texmodemask
 *  2012-09-25 BSp  MISRA cleanup
 */

/*--------------------------------------------------------------------------
 *
 * Title: Context Functions
 * Modify material settings
 *
 * Rendering attributes (like color, pattern, blendmodes) are stored inside a *context*.
 * A context is a collection of material attributes and must not be mistaken with a device handle.
 * A device handle (see: <Device management>) is the root of all process dependant data and is passed 
 * as first parameter to every function. 
 * This construct allows the driver functions to be reentrant in case of a multiple drawing engine units.
 * 
 * A device handle contains information about displaylists, framebuffers,etc... 
 * Everything that the drawing engine will require. It also includes pointers to contexts.
 *
 * Each device has at all times pointers to three relevant context's :
 *
 *  selected context - This one is changed (written to) by the set commands shown below (see: <d2_selectcontext>)
 *  solid context - Used as a source (read from) when rendering interior regions (see: <d2_solidcontext>)
 *  outline context - Used as a source (read from) when rendering outlines or shadows (see: <d2_outlinecontext>)
 *
 * All three (selected, solid and outline) can point to the same context.
 * Every driver function that starts with *get* or *set* will work on the currently selected context (with
 * the obvious exception of geterror and getversion).
 *
 * There is always a *default context* that can not be freed by the application and is used for
 * everything per default.
 *
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_memory.h"
#include "dave_texture.h"


/*--------------------------------------------------------------------------
 * Group: Context Management */

/*--------------------------------------------------------------------------
 * function: d2_newcontext
 * Create new context (used to store render settings).
 *
 * Note that every context is bound to the device it is created for and can not be
 * used in another process or selected into any other device.
 * Every new context is reset to a default state, it does not reflect the currently
 * selected context.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   context pointer or NULL in case of an error
 * */
d2_context * d2_newcontext( d2_device *handle )
{
   d2_contextdata *ctx = (d2_contextdata *) d2_getmem_p( sizeof(d2_contextdata) );

   D2_VALIDATEP( handle, D2_INVALIDDEVICE );  /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   if (!ctx)
   {
      (void)D2_SETERR(handle, D2_NOMEMORY);
      return NULL;
   }

   /* initialize context */
   ctx->device = D2_DEV(handle);
   ctx->basecolor[0]    = 0xffffff;
   ctx->basecolor[1]    = 0xffffff;
   ctx->basealpha[0]    = 0xff;
   ctx->basealpha[1]    = 0xff;
   ctx->constalpha      = 0xff000000u;
   ctx->patternalpha[0] = 0xff;
   ctx->patternalpha[1] = 0xff;
   ctx->premalpha[0]    = 0xff000000u;
   ctx->premalpha[1]    = 0xff000000u;
   ctx->features        = d2_feat_aa;
   ctx->thresholdmask   = 0;
   ctx->internaldirty   = 0;
   ctx->blurring        = D2_FIX4( 1 );
   ctx->invblur         = D2_FIX16( 1u );
   ctx->blendmask       = D2C_BSF | D2C_BDF | D2C_BDI;
   if(0 == (D2_DEV(handle)->hwrevision & D2FB_ALPHACHANNELBLENDING))
   {
      ctx->alphablendmask = D2C_WRITEALPHA1;
   }
   else
   {
      ctx->alphablendmask  = D2C_USE_ACB | D2C_BDIA;
      ctx->alphablendmask |= D2C_WRITEALPHA1;
   }
   ctx->texmodemask        = D2C_TEXTUREFILTERX | D2C_TEXTUREFILTERY | D2C_TEXTURECLAMPX | D2C_TEXTURECLAMPY;
   ctx->texbpp             = 4;
   ctx->tbstylemask        = D2C_READFORMAT2;
   ctx->rlemask            = 0;
   ctx->clutmask           = 0;
   ctx->colkeymask         = 0;
   ctx->cr2mask            = D2_DEV(handle)->fbstylemask | ctx->alphablendmask | ctx->blendmask | ctx->tbstylemask | ctx->rlemask | ctx->clutmask | ctx->colkeymask;
   ctx->gradients          = 0;
   ctx->alphamode          = d2_am_constant;
   ctx->fillmode           = d2_fm_color;
   ctx->alphablendflags    = d2_blendf_default;
   ctx->pattern            = 0xAA;
   ctx->orgpattern         = 0xAA;
   ctx->patmode            = d2_pm_filter;
   ctx->patmodemask        = D2C_TEXTUREFILTERX;
   ctx->patoffset          = 0;
   ctx->patscale           = D2_FIX4(1);
   ctx->invpatscale        = D2_FIX16( 1 );
   ctx->patlen             = 4;
   ctx->linecap            = d2_lc_square;
   ctx->linejoin           = d2_lj_miter;
   ctx->miterlimit         = D2_FIX4(4);
   ctx->circleextendoffset = 0;
   ctx->roundends          = 0;
   ctx->texcenterx         = 0;
   ctx->texcentery         = 0;
   ctx->texclut            = NULL;
   ctx->texclut_cached     = NULL;
   ctx->texclutupload      = 1;
   ctx->texclutoffset      = 0;
   ctx->colorkey           = 0;

   d2_initgradient_intern( &ctx->gradient[0] );
   d2_initgradient_intern( &ctx->gradient[1] );
   d2_initgradient_intern( &ctx->gradient[2] );
   d2_initgradient_intern( &ctx->gradient[3] );

   d2_initgradient_intern( &ctx->patulim[0] );

   d2_initgradient_intern( &ctx->texlim[0] );
   d2_initgradient_intern( &ctx->texlim[1] );

   /* setup initial texture blendmode */
   ctx->texmode = d2_tm_filter;
   ctx->texwrapmask = 0xffffffffu;
   ctx->texop_p1[0] = 0x00u;
   ctx->texop_p1[1] = 0x00u;
   ctx->texop_p1[2] = 0x00u;
   ctx->texop_p1[3] = 0x00u;
   ctx->texop_p2[0] = 0xffu;
   ctx->texop_p2[1] = 0xffu;
   ctx->texop_p2[2] = 0xffu;
   ctx->texop_p2[3] = 0xffu;
   ctx->texamode = d2_to_one;
   ctx->texrmode = d2_to_copy;
   ctx->texgmode = d2_to_copy;
   ctx->texbmode = d2_to_copy;
   d2_setuptextureblend_intern( handle, ctx );

   /* attach to chain */
   ctx->next = D2_DEV(handle)->ctxchain;
   D2_DEV(handle)->ctxchain = ctx;

   (void)D2_SETOK(handle);
   return ctx;
}

/*--------------------------------------------------------------------------
 * function: d2_freecontext
 * Release Context.
 *
 * Free the memory associated with a context. If the context is still selected
 * or an active source for rendering it is deselected (by selecting the default
 * context instead) before destruction.
 *
 * All contexts assigned to a device are freed automatically when the device
 * is closed.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   ctx - context pointer
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_freecontext( d2_device *handle, d2_context *ctx )
{
   d2_contextdata **prev;

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( ctx, D2_INVALIDCONTEXT );   /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   /* forbid removal of default context */
   if (D2_CTX(ctx) == D2_DEV(handle)->ctxdef) 
   {
      D2_RETERR( handle, D2_DEFCONTEXT );
   }

   /* find in chain */
   prev = & D2_DEV(handle)->ctxchain;
   while ((prev) && (*prev != ctx))
   {
      prev = (d2_contextdata **) *prev;
   }
   if (!prev)
   {
      D2_RETERR( handle, D2_INVALIDCONTEXT );
   }
   /* remove from chain */
   *prev = D2_CTX(ctx)->next;

   /* react to removal if context was active */
   if (D2_DEV(handle)->ctxselected == D2_CTX(ctx)) 
   {
      D2_DEV(handle)->ctxselected = D2_DEV(handle)->ctxdef;
   }
   if (D2_DEV(handle)->ctxoutline == D2_CTX(ctx))
   {
      D2_DEV(handle)->ctxoutline = D2_DEV(handle)->ctxdef;
   }
   if (D2_DEV(handle)->ctxsolid == D2_CTX(ctx))
   {
      D2_DEV(handle)->ctxsolid = D2_DEV(handle)->ctxdef;
   }

   if (  D2_CTX(ctx)->texclut_cached != NULL)
   {
      d1_freemem(D2_CTX(ctx)->texclut_cached);       /*  checkit */
      D2_CTX(ctx)->texclut_cached = NULL;
   }

   /* free context memory */
   d2_freemem_p( ctx );
   D2_RETOK(handle);
}


/*--------------------------------------------------------------------------
 * function: d2_getcontext
 * Get a pointer to an currently active context.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   mode - getcontext mode determines what context should be returned
 *
 * getcontext modes:
 *    d2_context_default - return the default context
 *    d2_context_selected - return the currently selected context (see: <d2_selectcontext>)
 *    d2_context_solid - return current solid context (see: <d2_solidcontext>)
 *    d2_context_outline - return current outline context (see: <d2_outlinecontext>)
 *
 * returns:
 *   context pointer or NULL in case of an error
 * */
d2_context * d2_getcontext( d2_device *handle, d2_s32 mode )   /* PRQA S 3673 */ /* $Misra: #NOT_CONST_IN_DEBUG_BUILD $*/
{
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );                   /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERRP( (((d2_u32)mode) < 4U), D2_INVALIDENUM );      /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   switch (mode)
   {
      case d2_context_default:
         return (d2_context*) D2_DEV(handle)->ctxdef;

      case d2_context_selected:
         return (d2_context*) D2_DEV(handle)->ctxselected;

      case d2_context_solid:
         return (d2_context*) D2_DEV(handle)->ctxsolid;

      case d2_context_outline:
         return (d2_context*) D2_DEV(handle)->ctxoutline;

      default:
         break;
   }

   return NULL;
}

/*--------------------------------------------------------------------------
 * debug inline code to verify that a given block is a valid context
 * for the current device */

#ifdef _DEBUG
#define VERIFY_CONTEXT(c)                                               \
   chain = D2_DEV(handle)->ctxchain;                                    \
   while (chain && (chain != D2_CTX(c)))                                \
   { chain = chain->next; }                                             \
   if ((!chain) || (D2_CTX(c)->device != D2_DEV(handle)))               \
   { D2_RETERR( handle, D2_INVALIDCONTEXT ); }(void)0 /* PRQA S 3412 */ /* $Misra: #COMPILER_WARNING $*/
#else
#define VERIFY_CONTEXT(c)                                               \
   chain = NULL;                                                        \
   (void) chain;                                                        \
   if (D2_CTX(c)->device != D2_DEV(handle))                             \
   { D2_RETERR( handle, D2_INVALIDCONTEXT ); }(void)0 /* PRQA S 3412 */ /* $Misra: #COMPILER_WARNING $*/
#endif

/*--------------------------------------------------------------------------
 * function: d2_selectcontext
 * Make a rendering context active (all following property set operations will use it)
 *
 * Selecting a context has no direct effect on how geometry is rendered (reading is done from
 * solid and outline contexts) and is a very fast operation.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   ctx - context pointer
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_solidcontext>, <d2_outlinecontext>, <d2_getcontext>
 * */
d2_s32 d2_selectcontext( d2_device *handle, d2_context *ctx )
{
   d2_contextdata *chain;  /* required by 'VERIFY_CONTEXT' macro below */

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( ctx, D2_INVALIDCONTEXT );   /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   VERIFY_CONTEXT( ctx );                   /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   D2_DEV(handle)->ctxselected = D2_CTX(ctx);
   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_solidcontext
 * Define the solid rendering context.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   ctx - context pointer
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_selectcontext>, <d2_outlinecontext>, <d2_getcontext>
 * */
d2_s32 d2_solidcontext( d2_device *handle, d2_context *ctx )
{
   d2_contextdata *chain;  /* required by 'VERIFY_CONTEXT' macro below */

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( ctx, D2_INVALIDCONTEXT );   /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   VERIFY_CONTEXT( ctx );                   /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   D2_DEV(handle)->ctxsolid = D2_CTX(ctx);
   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_outlinecontext
 * Define the outline rendering context.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   ctx - context pointer
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_solidcontext>, <d2_selectcontext>, <d2_getcontext>
 * */
d2_s32 d2_outlinecontext( d2_device *handle, d2_context *ctx )
{
   d2_contextdata *chain;  /* required by 'VERIFY_CONTEXT' macro below */

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( ctx, D2_INVALIDCONTEXT );   /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   VERIFY_CONTEXT( ctx );                   /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   D2_DEV(handle)->ctxoutline = D2_CTX(ctx);
   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * Group: Context Attribute Writes */


/*--------------------------------------------------------------------------
 * function: d2_setfillmode
 * Select fillmode (solid,patter,texture,..)
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   mode - fillmode
 *
 * fill modes:
 *
 *   d2_fm_color - single color (default)
 *   d2_fm_twocolor - blending between color1 and color2 instead of color1 and background
 *   d2_fm_pattern - fill with pattern
 *   d2_fm_texture - fill with texture
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setfillmode( d2_device *handle, d2_u32 mode )
{
   d2_contextdata *ctx;
   D2_VALIDATE( handle, D2_INVALIDDEVICE );               /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( (((d2_u32)mode) < 256), D2_INVALIDENUM ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;

   ctx->fillmode       = (d2_u8) mode;
   ctx->internaldirty |= (d2_u8) d2_dirty_material;

   switch (mode)
   {
      case d2_fm_color:
         ctx->features &= (d2_u8) (~((d2_u8)d2_feat_ulim | (d2_u8)d2_feat_vlim));
         break;

      case d2_fm_twocolor:
         ctx->features &= (d2_u8) (~((d2_u8)d2_feat_ulim | (d2_u8)d2_feat_vlim));
         break;

      case d2_fm_pattern:
         ctx->features |= (d2_u8) ( d2_feat_ulim);
         ctx->features &= (d2_u8) (~(d2_u8)d2_feat_vlim);
         break;

      case d2_fm_texture:
         ctx->features |= (d2_u8) (d2_feat_ulim | d2_feat_vlim);
         break;

      default:
         break;
   }

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setcolor
 * Set color registers.
 *
 * Set one of the two color registers, color1 and color2. Second register (index 1) is used for pattern
 * rendering and during d2_fm_twocolor fillmode only.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   index - color register index (0 or 1)
 *   color - 24bit rgb color value
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setcolor( d2_device *handle, d2_s32 index, d2_color color )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE );                 /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( (((d2_u32)index) < 2U), D2_INVALIDINDEX );  /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   D2_DEV(handle)->ctxselected->basecolor[ index ] = color & 0x00ffffff;
   D2_DEV(handle)->ctxselected->internaldirty |= d2_dirty_material;
   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setalpha
 * set constant alpha value
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   alpha - alpha value (0 is totally transparent, 255 is fully opaque)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setalpha( d2_device *handle, d2_alpha alpha )
{
   d2_contextdata *ctx;
   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */  /*$Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->internaldirty |= d2_dirty_material;

   if (ctx->basealpha[0] != alpha)
   {
      ctx->internaldirty |= (d2_dirty_premalpha_p | d2_dirty_premalpha_t);
   }

   ctx->basealpha[0] = alpha;

   if(0 != (ctx->alphamode & d2_am_constant))
   {
      ctx->constalpha = ((d2_color)alpha) << 24;
   }

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setalphaex
 * set constant alpha value
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   index - alpha register index (0 or 1)
 *   alpha - alpha value (0 is totally transparent, 255 is fully opaque)
 *
 * if index is 0 this function does the same as <d2_setalpha>.
 * if index is 1 alpha is set for color register 2 (see: <d2_setalphablendmodeex>).
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setalphaex( d2_device *handle, d2_s32 index, d2_alpha alpha )
{
   d2_contextdata *ctx;
   D2_VALIDATE( handle, D2_INVALIDDEVICE );                    /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERRP( (((d2_u32)index) < 2U), D2_INVALIDINDEX );    /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   ctx = D2_DEV(handle)->ctxselected;
   ctx->internaldirty |= d2_dirty_material;

   if(0 == index)
   {
      return d2_setalpha(handle, alpha);
   }
   else
   {
      ctx->basealpha[index] = alpha;
   }

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setalphamode
 * Choose alpha source.
 *
 * Defines how alpha (transparency) is defined. As a constant value, a
 * gradient or a mixture to both. Gradients are defined using <d2_setalphagradient>.
 *
 * Note that per pixel alpha (e.g. texture alpha channel) information is not disabled by
 * setting the alphamode to opaque. To disable texture alpha look at <d2_settextureoperation>.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   mode - alpha mode, several values can be or'ed together
 *
 * alpha modes:
 *  d2_am_opaque - no transparency (equal to constant alpha of 0xff)
 *  d2_am_constant - constant transparency   (see: <d2_setalpha>)
 *  d2_am_gradient1 - alpha gradient1 is active
 *  d2_am_gradient2 - alpha gradient2 is active
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setalphamode( d2_device *handle, d2_u32 mode )
{
   d2_contextdata *ctx;
   D2_VALIDATE( handle, D2_INVALIDDEVICE );               /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( (((d2_u32)mode) < 256), D2_INVALIDENUM ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->internaldirty |= d2_dirty_material;

   if(ctx->alphamode != mode)
   {
      ctx->internaldirty |= (d2_dirty_premalpha_p | d2_dirty_premalpha_t);
   }

   ctx->alphamode = (d2_u8) mode;

   if(0 != (mode & d2_am_constant))
   {
      ctx->constalpha = ((d2_color)ctx->basealpha[0]) << 24;
   }
   else 
   {
      ctx->constalpha = 0xff000000u;
   }

   D2_RETOK(handle);
}


/*--------------------------------------------------------------------------
 * function: d2_setalphagradient
 * Define an alpha gradient.
 *
 * Instead or in addition to a constant alpha (transparency) value the hardware
 * can also apply one or more alpha gradients to the rendered geometry.
 * Gradients have to be enabled using <d2_setalphamode> in order to become visible.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   index - alpha gradient index (0 or 1)
 *   x,y - startpoint of gradient (point of alpha 0) (fixedpoint)
 *   dx,dy - direction and length of gradient (distance to point of alpha 255)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * */
d2_s32 d2_setalphagradient( d2_device *handle, d2_s32 index, d2_point x, d2_point y, d2_point dx, d2_point dy )
{
   d2_gradientdata *grad;
   d2_s32 xa,ya;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );                /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( (((d2_u32)index) < 2U), D2_INVALIDINDEX ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
/* D2_CHECKERR( ((dx > D2_EPSILON) || (dx < -D2_EPSILON)), D2_VALUETOOSMALL );
 * D2_CHECKERR( ((dy > D2_EPSILON) || (dy < -D2_EPSILON)), D2_VALUETOOSMALL ); */
   index+=2;

   /* calc limiter values */
   if(0 != (dx | dy)) /* PRQA S 3344, 4130 */ /*$Misra: #PERF_LOGICOP $*/
   {
      /* using 64bit division to avoid fixedpoint overflow */
#ifdef _NO_LL_
      d2_int64 tmp, tmp2;
#endif
      d2_s32 idx = dx;
      d2_s32 idy = dy;
      d2_s32 l = (d2_s32)(((idx * idx) + (idy * idy)));
#ifndef _NO_LL_
      xa = (d2_s32)((idx * (d2_int64) 0x100000) / l);
      ya = (d2_s32)((idy * (d2_int64) 0x100000) / l);
#else
      d2_mul3232to64(idx, 0x100000, &tmp);
      d2_div6432(&tmp, l, &tmp2);
      xa  = d2_cast64to32(&tmp2);
      
      d2_mul3232to64(idy, 0x100000, &tmp);
      d2_div6432(&tmp, l, &tmp2);
      ya  = d2_cast64to32(&tmp2);
#endif
   } 
   else 
   {
      xa = 0;
      ya = 0;
   }

   /* get access */
   grad = & D2_DEV(handle)->ctxselected->gradient[index];
   /* store parameters */
   grad->mode  = d2_grad_linear;
   grad->x1    = x;
   grad->y1    = y;
   grad->xadd  = xa;
   grad->yadd  = ya;
   grad->xadd2 = 0;
   grad->yadd2 = 0;
   /* mark as active */
   D2_DEV(handle)->ctxselected->gradients |= (d2_u8)BIT(index);

   D2_RETOK(handle);
}


/*--------------------------------------------------------------------------
 * function: d2_setblendmode
 * Choose blendmode for RGB.
 *
 * Blendmode defines how the RGB channels of new pixels (source) are combined with already existing data
 * in the framebuffer (destination). Blend modes for alpha channel are set by <d2_setalphablendmode>.
 *
 * The most common blending (and also the default) is a direct linear blend :
 * > d2_bm_alpha , d2_bm_one_minus_alpha
 *
 * Additive blending is also pretty common :
 * > d2_bm_alpha , d2_bm_one
 *
 * Note that antialiasing is not possible without blending (because partly covered
 * pixels are represented by transparency). Therefore antialiasing is impossible
 * with blendmodes that ignore alpha entirely.
 *
 * If d2_mode_alpha8 is used, alpha is treated as color.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   srcfactor - source blend factor (see available blend factors)
 *   dstfactor - destination blend factor (see available blend factors)
 *
 * blend factors:
 *     d2_bm_zero             - constant 0
 *     d2_bm_one              - constant 1
 *     d2_bm_alpha            - current alpha
 *     d2_bm_one_minus_alpha  - inverted alpha
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setblendmode( d2_device *handle, d2_u32 srcfactor, d2_u32 dstfactor )
{
   d2_contextdata *ctx;
   d2_u32 blendMask;

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->internaldirty |= d2_dirty_material;
   blendMask = 0;

   /* create blendunit bits */
   switch (srcfactor)
   {
      case d2_bm_one:
         break;

      case d2_bm_zero:
         blendMask |= D2C_BSI;
         break;

      case d2_bm_alpha:
         blendMask |= D2C_BSF;
         break;

      case d2_bm_one_minus_alpha:
         blendMask |= (D2C_BSF | D2C_BSI);
         break;

      default:
         D2_RETERR( handle, D2_INVALIDENUM );
   }

   switch (dstfactor)
   {
      case d2_bm_one:
         break;

      case d2_bm_zero:
         blendMask |= D2C_BDI;
         break;

      case d2_bm_alpha:
         blendMask |= D2C_BDF;
         break;

      case d2_bm_one_minus_alpha:
         blendMask |= (D2C_BDF | D2C_BDI);
         break;

      default:
         D2_RETERR( handle, D2_INVALIDENUM );
   }

   /* store */
   ctx->blendmask = blendMask;
   ctx->cr2mask = D2_DEV(handle)->fbstylemask | ctx->blendmask | ctx->tbstylemask | ctx->alphablendmask | ctx->rlemask | ctx->clutmask | ctx->colkeymask;

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setalphablendmode
 * Choose blendmode for alpha channel.
 *
 * Blendmode defines how new alpha values (source) are combined with already existing data
 * in the framebuffer (destination).
 *
 * - The most common blending for alpha channel (and also the default) is write src alpha :
 *
 * > d2_bm_one, d2_bm_zero
 *
 * - Another possible mode is alpha blending:
 *
 * > d2_bm_one, d2_bm_one_minus_alpha
 *
 * - It is also possible to keep dst alpha :
 *
 * > d2_bm_zero, d2_bm_one
 *
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   srcfactor - source blend factor (see available blend factors)
 *   dstfactor - destination blend factor (see available blend factors)
 *
 * blend factors:
 *     d2_bm_zero - constant 0
 *     d2_bm_one  - constant 1
 *     d2_bm_alpha            - current alpha
 *     d2_bm_one_minus_alpha  - inverted alpha
 *
 * note:
 *  If the framebuffer format is set to d2_mode_alpha8 (<d2_framebuffer>) then alpha blend mode should be set to write src alpha. 
 *  For d2_mode_alpha8 the blend modes set by <d2_setblendmode> do apply.
 *
 * note:
 *   the blend flags of the selected context is set to d2_blendf_default (see: <d2_setalphablendmodeex>)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setalphablendmode( d2_device *handle, d2_u32 srcfactor, d2_u32 dstfactor )
{
   d2_contextdata *ctx;
   d2_u32 alphaBlendMask;

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->internaldirty |= d2_dirty_material;

   ctx->alphablendflags = d2_blendf_default;

   if(0 == (D2_DEV(handle)->hwrevision & D2FB_ALPHACHANNELBLENDING))
   {
      /* create write-alpha bits */
      alphaBlendMask = D2C_WRITEALPHA1;

      if((srcfactor == d2_bm_one) && (dstfactor == d2_bm_zero))
      {
         alphaBlendMask &= ~D2C_WRITEALPHA2;
      }
      else if((srcfactor == d2_bm_zero) && (dstfactor == d2_bm_one))
      {
         alphaBlendMask |= D2C_WRITEALPHA2;
      }
      else
      {
         D2_RETERR( handle, D2_INVALIDENUM );
      }
   } 
   else 
   {
      /* create alpha-blendunit bits */
      alphaBlendMask  = D2C_USE_ACB;
      alphaBlendMask |= D2C_WRITEALPHA1;

      switch (srcfactor)
      {
         case d2_bm_one:
            break;

         case d2_bm_zero:
            alphaBlendMask |= D2C_BSIA;
            break;

         case d2_bm_alpha:
            alphaBlendMask |= D2C_BSFA;
            break;

         case d2_bm_one_minus_alpha:
            alphaBlendMask |= (D2C_BSFA | D2C_BSIA);
            break;

         default:
            D2_RETERR( handle, D2_INVALIDENUM );
      }

      switch (dstfactor)
      {
         case d2_bm_one:
            break;

         case d2_bm_zero:
            alphaBlendMask |= D2C_BDIA;
            break;

         case d2_bm_alpha:
            alphaBlendMask |= D2C_BDFA;
            break;

         case d2_bm_one_minus_alpha:
            alphaBlendMask |= (D2C_BDFA | D2C_BDIA);
            break;

         default:
            D2_RETERR( handle, D2_INVALIDENUM );
      }   
   }

   /* store */
   ctx->alphablendmask = alphaBlendMask;
   ctx->cr2mask        = 
      D2_DEV(handle)->fbstylemask | 
      ctx->blendmask              | 
      ctx->tbstylemask            | 
      ctx->alphablendmask         | 
      ctx->rlemask                | 
      ctx->clutmask               | 
      ctx->colkeymask             ;


   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setalphablendmodeex
 * Choose blendmode for alpha channel.
 *
 * Blendmode defines how new alpha values (source) are combined with already existing data
 * in the framebuffer (destination).
 *
 * - The most common blending for alpha channel (and also the default) is write src alpha :
 *
 * > d2_bm_one, d2_bm_zero
 *
 * - Another possible mode is alpha blending:
 *
 * > d2_bm_one, d2_bm_one_minus_alpha
 *
 * - It is also possible to keep dst alpha :
 *
 * > d2_bm_zero, d2_bm_one
 *
 *
 * parameters:
 *   handle     - device pointer (see: <d2_opendevice>)
 *   srcfactor  - source blend factor (see available blend factors)
 *   dstfactor  - destination blend factor (see available blend factors)
 *   blendflags - blend flags (see available blend flags)
 *
 * blend factors:
 *     d2_bm_zero - constant 0
 *     d2_bm_one  - constant 1
 *     d2_bm_alpha            - current alpha
 *     d2_bm_one_minus_alpha  - inverted alpha
 *
 * blend flags:
 *     d2_blendf_blenddst    - use framebuffer alpha as dst alpha (is d2_blendf_default)
 *     d2_blendf_blendcolor2 - use color2 alpha as dst alpha as input for the blending formula (see: <d2_setalphaex>)
 *
 * note:
 *  If the framebuffer format is set to d2_mode_alpha8 (<d2_framebuffer>) the alpha blend mode should be set to write src alpha:
 *  For d2_mode_alpha8 the blend modes set by <d2_setblendmode> do apply.
 *
 *  On older D/AVE 2D hardware revisions without alpha channel blending (see: <d2_getrevisionhw>, D2FB_ALPHACHANNELBLENDING)
 *  only the following two blend combinations can be used for the alpha channel:
 *  - (1/0): 1 * src + 0 * dst -> write source alpha
 *  - (0/1): 0 * src + 1 * dst -> write framebuffer alpha or color2 alpha (depending on blendflags)
 *
 * These modes also require that the same source blend factor is configured for RGB and A.
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setalphablendmodeex( d2_device *handle, d2_u32 srcfactor, d2_u32 dstfactor, d2_u32 blendflags )
{
   d2_contextdata *ctx;
   d2_s32 ret;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );                      /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( (((d2_u32)blendflags) < 256), D2_INVALIDENUM );  /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;

   if(0 == (D2_DEV(handle)->hwrevision & D2FB_ALPHACHANNELBLENDING))
   {
      if ((srcfactor == d2_bm_one) && (dstfactor == d2_bm_zero))
      {
         blendflags = d2_blendf_default;
      }      
   }

   ret = d2_setalphablendmode(handle, srcfactor, dstfactor);
   if (ret == D2_OK) 
   {
      ctx->alphablendflags = (d2_u8) blendflags;
   }
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_setantialiasing
 * Globally disable or enable antialiasing.
 *
 * When antialiasing is enabled all geometry is blended with the background
 * according to its coverage value.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   enable - boolean value (set 1 to enable antialiasing)
 *
 * If antaliasing is disabled blurring will also be disabled (see: <d2_setblur>).
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setantialiasing( d2_device *handle, d2_s32 enable )
{
   d2_contextdata *ctx;

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;

   if(0 != enable)
   {
      ctx->features      |= (d2_u8) d2_feat_aa;
      ctx->thresholdmask  = 0;
   } 
   else
   {
      ctx->features     &= (d2_u8) ~(d2_u8)d2_feat_aa;
      ctx->thresholdmask = D2C_LIM1THRESHOLD | D2C_LIM2THRESHOLD | D2C_LIM3THRESHOLD |
         D2C_LIM4THRESHOLD | D2C_LIM5THRESHOLD | D2C_LIM6THRESHOLD ;
      ctx->invblur       = D2_FIX16(1u);
      ctx->features     &= (d2_u8) ~(d2_u8)d2_feat_blur;
      ctx->blurring      = D2_FIX4(1);
   }

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setblur
 * Set global blurring factor.
 *
 * The blurring factor is a fixed point (4 bit fraction) number of pixels
 * during which the edge opacity changes from 0 (no coverage) to 1 (fully covered).
 * The intended use is a finetuning of the antialiasing. The default is a value of 1.0 
 * equals a single pixel wide region around each primitive for antialiasing.
 *
 * A lower value is not useful (and not supported). A higher value will produce 
 * results similar to an additional lowpass filter (hence the name).
 * Extending the AA region above 2 or 3 pixels is not recommended and should be 
 * used for special effects only. 
 *
 * Note that blurring will not work when antialiasing is disabled (see: <d2_setantialiasing>).
 * If blurring factor is > 1 then antialiasing will be enabled (see: <d2_setantialiasing>).
 *
 * In order to disable blurring set a value of 1 (fixed point)
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   blur - fixed point
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setblur( d2_device *handle, d2_width blur )
{
   d2_contextdata *ctx;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );    /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( blur != 0, D2_VALUETOOSMALL ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   if (blur <= D2_FIX4(1))
   {
      ctx->invblur   = D2_FIX16(1u);
      ctx->features &= (d2_u8) ~(d2_u8)d2_feat_blur;
      blur = D2_FIX4(1);
   } 
   else 
   {
/*       if ( !(ctx->features & d2_feat_aa))
 *       {
 *         // antialiasing must have been set 
 *         D2_RETERR(handle, D2_INVALIDCONTEXT);
 *       } */
      ctx->invblur   = (65536u * 16u) / ((d2_u32)blur);
      ctx->features |= (d2_u8) d2_feat_blur;
      ctx->features |= (d2_u8) d2_feat_aa;
      ctx->thresholdmask = 0;
   }
   ctx->blurring = blur;

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setlinecap
 * Specify lineend style.
 *
 * Linecaps are applied when rendering lines using <d2_renderline> or <d2_renderline2>
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   mode - linecap mode
 *
 * linecap modes:
 *   d2_lc_butt - lines end directly at endpoints
 *   d2_lc_round - lines end with halfcircles
 *   d2_lc_square - line ends are extended by the half linewidth
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setlinecap( d2_device *handle, d2_u32 mode )
{
   d2_contextdata *ctx;
   D2_VALIDATE( handle, D2_INVALIDDEVICE );            /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( (mode < d2_lc_max), D2_INVALIDENUM );  /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->linecap = (d2_u8) mode;

   if(mode == d2_lc_round) 
   {
      ctx->roundends = 3; 
   }
   else 
   {
      ctx->roundends = 0;
   }

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setlinejoin
 * Specify polyline connection style.
 *
 * Linejoins are applied when rendering polylines using <d2_renderpolyline>
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   mode - linejoin mode
 *
 * linejoin modes:
 *   d2_lj_none - no connection is applied
 *   d2_lj_miter - lines meet in a sharp angle (see also <d2_setmiterlimit>)
 *   d2_lj_round - line are connected by circle segments
 *   d2_lj_bevel - lines meet in a flat angle
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setlinejoin( d2_device *handle, d2_u32 mode )
{
   d2_contextdata *ctx;
   D2_VALIDATE( handle, D2_INVALIDDEVICE );            /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( (mode < d2_lj_max), D2_INVALIDENUM );  /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->linejoin = (d2_u8) mode;

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setmiterlimit
 * Clipping distance for miter polyline connections.
 *
 * When using sharp linejoin's (d2_lj_miter) with <d2_renderpolyline> the sharp
 * edge has to be clipped at some point. Otherwise it would extend to infinity
 * when the two segments meet at 180�.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   miter - maximum pixel distance the join can extend beyond bevel edge
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setmiterlimit( d2_device *handle, d2_width miter )
{
   d2_contextdata *ctx;
   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->miterlimit = miter;

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setpattern
 * Specify pattern bitmask.
 *
 * When patterns are used to fill a primitive an index into the bitmask is
 * generated for each pixel (see: <d2_setpatternparam> for mapping details). The
 * pattern wraps around after N bits (N can be set using <d2_setpatternsize>).
 *
 * Depending on the bit color1 or color2 and patternalpha1 or patternalpha2
 * are used. Fractional indices are interpolated between those two values.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   pattern - N bit pattern mask
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * example:
 *   The following figure shows a rendered line filled with an 8 bit pattern. Color1 is blue
 *   and color2 is green, the bitstring is 0101 1001. For the upper line filtering is off, the
 *   lower line is rendered with filtering enabled (flag: d2_pm_filter).
 *   (see pattern.png)
 * */
d2_s32 d2_setpattern( d2_device *handle, d2_pattern pattern )
{
   d2_contextdata *ctx;
   d2_s32 i,patLen, maxPatLen;
   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->internaldirty |= d2_dirty_material;
   ctx->orgpattern = (d2_s32) pattern;

   maxPatLen = ctx->device->maxpatlen;
   patLen = ctx->patlen;
   /* mask invalid bits in pattern
    * pattern &= ((2 << (patlen-1))-1);  (warning: would break 'eye' demo) */

   /* extend it to fill entire hardware word */
   for (i=0; i<(maxPatLen / patLen); i++) 
   {
      pattern |= (pattern << patLen);
   }
   /* store */
   ctx->pattern = (d2_s32) pattern;

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setpatternalpha
 * Specify pattern transparency.
 *
 * Pattern alpha is mixed with global alpha (gradient or constant). Pattern alpha
 * index 0 is used where 0bits occur in the pattern bitmask.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   index - register index (0 or 1 as patterns are always twocolor)
 *   alpha - alphavalue (0 is totally transparent, 255 is fully opaque)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_setcolor>, <d2_setpattern>
 * */
d2_s32 d2_setpatternalpha( d2_device *handle, d2_s32 index, d2_alpha alpha )
{
   d2_contextdata *ctx;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );                /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( (((d2_u32)index) < 2U), D2_INVALIDINDEX ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->patternalpha[ index ] = alpha;
   ctx->internaldirty |= (d2_dirty_premalpha_p | d2_dirty_premalpha_t) | d2_dirty_material;

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_setpatternparam
 * Define mapping of pattern to geometry.
 *
 * Pattern mapping is defined by specifying a line segment (startpoint and
 * vector to endpoint) along which the pattern is mapped. The startpoint
 * maps to bit0 of the pattern bitmask (see: <set_pattern>).
 * With the bitmask extending along the direction vector.
 *
 * Note that the pattern is repeated infinitely along the defined line.
 *
 * Free pattern directions apply to line based geometry only when pattern
 * mode (see: <d2_setpatternmode>) is not set to auto align.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   x, y - pattern startpoint (fixedpoint)
 *   dx, dy - pattern direction and size (fixedpoint)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_setpatternsize>, <d2_setpattern>, <d2_setpatternmode>
 * */
d2_s32 d2_setpatternparam( d2_device *handle, d2_point x, d2_point y, d2_width dx, d2_width dy )
{
   d2_contextdata *ctx;
   d2_gradientdata *grad;
   d2_s32 xa,ya;

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->internaldirty |= d2_dirty_upatlim;

   grad = & ctx->patulim[0];
   /* calc limiter values */
   if(0 != (dx | dy))  /* PRQA S 3344, 4130 */ /* $Misra: #PERF_LOGICOP $*/
   {
      /* using 64bit division to avoid fixedpoint overflow */
      d2_s32 idx = dx;
      d2_s32 idy = dy;
#ifdef _NO_LL_
      d2_int64 tmp, tmp2;
#endif
      d2_s32 l = ((idx*idx) + (idy*idy));
      /* scale to pattern bitlength */
#ifndef _NO_LL_
      xa = (d2_s32) ((idx * (d2_int64) (ctx->patlen * (1 << 16) * (1 << 4)) ) / l);
      ya = (d2_s32) ((idy * (d2_int64) (ctx->patlen * (1 << 16) * (1 << 4)) ) / l);
#else
      d2_mul3232to64(idx, ctx->patlen * (1 << 16) * (1 << 4), &tmp);
      d2_div6432(&tmp, l, &tmp2);
      xa  = d2_cast64to32(&tmp2);
      
      d2_mul3232to64(idy, ctx->patlen * (1 << 16) * (1 << 4), &tmp);
      d2_div6432(&tmp, l, &tmp2);
      ya  = d2_cast64to32(&tmp2);
#endif
      
   } 
   else 
   {
      xa = 0;
      ya = 0;
   }

   /* store gradient */
   grad->mode = d2_grad_linear;
   grad->x1 = x;
   grad->y1 = y;
   grad->xadd = xa;
   grad->yadd = ya;

   D2_RETOK( handle );
}

/*--------------------------------------------------------------------------
 * function: d2_setlinepattern
 * Specify parameters for aligned patterns.
 *
 * When rendering line based geometry it is very often required to align the
 * pattern perpendicular to the line direction. For single elements it can
 * be done using <d2_setpatternparam> but this is not efficient and sometimes
 * (e.g. with polylines) not even possible.
 *
 * When enabled by setting the patternmode to contain *d2_pm_autoalign* this
 * function can be used to describe the pattern relative to any line geometry
 * by scale and offset.
 *
 * The offset can be incremented automatically after rendering an element
 * if d2_pm_advance is set using <d2_setpatternmode>. Calling <d2_setlinepattern>
 * resets the internally managed offset to the specified value.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   scale - number of screenpixels one pattern bit is mapped onto
 *   offset - pattern offset in pixels (fixedpoint format equal to d2_point)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_setpatternmode>, <d2_setpatternparam>
 * */
d2_s32 d2_setlinepattern( d2_device *handle, d2_width scale, d2_s32 offset )
{
   d2_contextdata *ctx;
   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->patoffset = offset;
   ctx->patscale = scale;

   /* precal reciproc */
   ctx->invpatscale = (65536*16) / scale;

   D2_RETOK( handle );
}


/*--------------------------------------------------------------------------
 * function: d2_setpatternmode
 * Define pattern addressing details.
 *
 * Filtering (interpolation) of pattern data can be disabled by passing 0
 * as patternmode.
 *
 * Patterns can be aligned to run perpendicular to rendered lines automatically
 * when d2_pm_autoalign mode is enabled (works only when rendering line based
 * geometry). In this case calling <d2_setpatternparam> is unnecessary but
 * additional data has to be passed using <d2_linepattern>.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *  mode - any combination of pattern mode bits (see below)
 *
 * pattern mode bits:
 * d2_pm_filter - use linear interpolation between colors (default)
 * d2_pm_autoalign - map pattern to line direction (see: <d2_setlinepattern>)
 * d2_pm_advance - increase offset automatically (see: <d2_setlinepattern>)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_setpatternmode( d2_device *handle, d2_u32 mode )
{
   d2_contextdata *ctx;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );               /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( (((d2_u32)mode) < 256), D2_INVALIDENUM ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   /* store */
   ctx = D2_DEV(handle)->ctxselected;

   ctx->internaldirty |= d2_dirty_material;
   ctx->patmode        = (d2_u8) mode;

   /* precalc modefield */
   if(0 != (mode & d2_pm_filter))
   {
      ctx->patmodemask = D2C_TEXTUREFILTERX;
   }
   else
   {
      ctx->patmodemask = 0;
   }

   D2_RETOK( handle );
}


/*--------------------------------------------------------------------------
 * function: d2_setpatternsize
 * Define pattern size.
 *
 * Patterns are defined as a bitvector. This function can be used to
 * define the number of valid bits. The size may not exceed the maximum
 * size supported by the hardware (currently 8).
 * The default size is set to 4 in every new context for compatibility reasons.
 *
 * Size changes affect only the following calls to <d2_setpatternparam>
 * and <d2_setpattern>
 *
 * Note that for correct wraparound the size must divide the hardware
 * maximum without remainder.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   size - number of valid bits used for pattern mask
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * please note:
 *   when using small pattern size (<8) all leading unused bits in the pattern mask (see: <d2_setpattern>) must be zero!
 * */
d2_s32 d2_setpatternsize( d2_device *handle, d2_s32 size )
{
   d2_contextdata *ctx;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );                                                  /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( size > 0, D2_VALUETOOSMALL );                                                /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( size <= (D2_DEV(handle)->ctxselected->device->maxpatlen), D2_INVALIDWIDTH ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   /* store */
   ctx = D2_DEV(handle)->ctxselected;
   ctx->patlen = size;

   D2_RETOK( handle );
}

/*--------------------------------------------------------------------------
 * function: d2_setclipgradient
 * Define an alpha gradient for clipping.
 *
 * Instead or in addition to a constant alpha (transparency) value the hardware
 * can also apply one or more alpha gradients to the rendered geometry.
 *
 * <d2_setalphagradient> offers an interface suitable to specify smooth alpha gradients,
 * but does not provide enough accuracy for steep gradients, which effectively clip
 * objects along a straight edge, e.g. let the alpha value drop from 255 to 0 within
 * the distance of a single pixel.
 * For this reason d2_setclipgradient offers the necessary accuracy by using 16.16 
 * fixed point instead of <d2_point> (12.4 fixed point) for the definition of the gradient.
 * Also the interpretation of nx,ny is different compared to dx,dy of <d2_setalphagradient> 
 * as follows:
 * nx,ny specify a normal vector of the clipping edge, pointing towards the non-clipped side.
 * When this vector is normalized to a length of 1.0 (i.e. 0x00010000), the gradient alpha value
 * changes from 0 to 255 within the distance of a single pixel. When it is longer, the gradient
 * is even steeper respectively smoother, when it is shorter. The gradient alpha value is always
 * clamped to 0 respectively 255 when it leaves this range.
 *
 * note:
 *   Additionally index 2 or 3 can also be used in some cases. These indices are 
 *   shared with the alpha gradients (and will overwrite these settings) and have to be 
 *   enabled using <d2_setalphamode> in order to become visible. 
 *   Please also note that the total number of available gradients (alpha or clip) is 
 *   restricted by the D/AVE 2D hardware and varies on the primitive types and render modes!
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   index - alpha gradient index (0 or 1, additionally 2 or 3)
 *   x,y - startpoint of gradient (point of alpha 0) (fixedpoint)
 *   nx,ny - normal vector of the clipping edge, pointing towards the non-clipped side (16.16 fixedpoint)
 *   flags - reserved
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * see also:
 *  <d2_setalphagradient>
 * */
d2_s32 d2_setclipgradient( d2_device *handle, d2_s32 index, d2_point x, d2_point y, d2_s32 nx, d2_s32 ny, d2_u32 flags )
{
   d2_gradientdata *grad;

   if(NULL != handle)
   {
      /*D2_VALIDATE( handle, D2_INVALIDDEVICE );*/             /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
      D2_CHECKERR( (((d2_u32)index) < 4U), D2_INVALIDINDEX );  /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

      /* get access */
      grad = & D2_DEV(handle)->ctxselected->gradient[index];
      if(0 == (nx | ny))                                       /* PRQA S 3344, 4130 */ /* $Misra: #PERF_LOGICOP $*/
      {
         /* disable clip plane */
         D2_DEV(handle)->ctxselected->gradients &= (d2_u8) ~ BIT(index);
      }
      else
      {
         /* store parameters */
         grad->mode = d2_grad_linear | flags;
         grad->x1 = x;
         grad->y1 = y;
         grad->xadd = nx;
         grad->yadd = ny;
         grad->xadd2 = 0;
         grad->yadd2 = 0;
         /* mark as active */
         D2_DEV(handle)->ctxselected->gradients |= (d2_u8) BIT(index);
      }

      D2_RETOK(handle);
   }
   else
   {
      return D2_INVALIDDEVICE;
   }
}

/*--------------------------------------------------------------------------
 * function: d2_setcircleextend
 * Increase bbox of circles.
 *
 * Due to limited precision for circle parameters large circles with enabled 
 * blurring can become inaccurate.
 * To avoid clipping at the bbox of the circle the bbox can be extended by offset.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   offset - number by which bbox of circles will be extended (fixedpoint)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * */
d2_s32 d2_setcircleextend( d2_device *handle, d2_width offset )
{
   d2_contextdata *ctx;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );       /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( offset >= 0, D2_VALUETOOSMALL );  /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   /* store */
   ctx = D2_DEV(handle)->ctxselected;
   ctx->circleextendoffset = offset;

   D2_RETOK( handle );
}

/*--------------------------------------------------------------------------
 *
 * */
void d2_calcpatternalpha_intern( d2_contextdata *ctx )
{
   d2_u32 patAlpha;

   patAlpha = (d2_u32) (ctx->constalpha >> 8);
   ctx->premalpha[0] = ((patAlpha * ctx->patternalpha[0]) + 0xffffffu) & 0xff000000u;
   ctx->premalpha[1] = ((patAlpha * ctx->patternalpha[1]) + 0xffffffu) & 0xff000000u;

   ctx->internaldirty &= (d2_u8) ~(d2_u8)d2_dirty_premalpha_p;
}

/*--------------------------------------------------------------------------
 * Group: Context Attribute Queries */


/*--------------------------------------------------------------------------
 * function: d2_getfillmode
 * Query fillmode from selected context.
 *
 * see: <d2_setfillmode> for a list of fill modes
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   Select fillmode (d2_fm_color,d2_fm_pattern,..). undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *  <d2_setfillmode>
 * */
d2_u8 d2_getfillmode( d2_device *handle )
{
   d2_u8 ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->fillmode;

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getcolor
 * Query a color from selected context.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   index - color register index (0 or 1)
 *
 * returns:
 *   content of specified color register. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_setcolor>
 * */
d2_color d2_getcolor( d2_device *handle, d2_s32 index )
{
   d2_color ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );                    /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERRP( (((d2_u32)index) < 2U), D2_INVALIDINDEX );     /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->basecolor[ index ];

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getalpha
 * Query constant alpha from selected context.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   content of constant alpha register. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_setalpha>
 * */
d2_alpha d2_getalpha( d2_device *handle )
{
   d2_alpha ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->basealpha[0];

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getalphaex
 * Query constant alpha from selected context.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   index - alpha register index (0 or 1)
 *
 * returns:
 *   content of constant alpha register. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_setalphaex>
 * */
d2_alpha d2_getalphaex( d2_device *handle, d2_s32 index )
{
   d2_alpha ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );                    /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERRP( (((d2_u32)index) < 2U), D2_INVALIDINDEX );     /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->basealpha[index];

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getalphamode
 * Query alpha source from selected context.
 *
 * see: <d2_setalphamode> for a list of alpha mode bits
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   alpha source bitmask. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *  <d2_setalphamode>
 * */
d2_u8 d2_getalphamode( d2_device *handle )
{
   d2_u8 ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->alphamode;

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getblendmodesrc
 * Query source blend factor from selected context.
 *
 * see: <d2_setblendmode> for a list of blendmodes
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   source blend factor. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_getblendmodedst>, <d2_setblendmode>
 * */
d2_u32 d2_getblendmodesrc( d2_device *handle )
{
   d2_u32 ret;
   d2_u32 blendMask;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   /* retrieve blendunit bits */
   blendMask = D2_DEV(handle)->ctxselected->blendmask & (D2C_BSF | D2C_BSI);

   switch(blendMask)
   {
      case 0:
         ret = d2_bm_one;
         break;

      case D2C_BSI:
         ret = d2_bm_zero;
         break;

      case D2C_BSF:
         ret = d2_bm_alpha;
         break;

      case (D2C_BSF | D2C_BSI):
         ret = d2_bm_one_minus_alpha;
         break;

      default:
         D2_RETERRU( handle, D2_INVALIDENUM );
   }

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getblendmodedst
 * Query destination blend factor from selected context.
 *
 * see: <d2_setblendmode> for a list of blendmodes
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   destination blend factor. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_getblendmodesrc>, <d2_setblendmode>
 * */
d2_u32 d2_getblendmodedst( d2_device *handle )
{
   d2_u32 ret;
   d2_u32 blendMask;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   /* retrieve blendunit bits */
   blendMask = D2_DEV(handle)->ctxselected->blendmask & (D2C_BDF | D2C_BDI);
   switch(blendMask)
   {
      case 0:
         ret = d2_bm_one;
         break;

      case D2C_BDI:
         ret = d2_bm_zero;
         break;

      case D2C_BDF:
         ret = d2_bm_alpha;
         break;

      case (D2C_BDF | D2C_BDI):
         ret = d2_bm_one_minus_alpha;
         break;

      default:
         D2_RETERRU( handle, D2_INVALIDENUM );
   }

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getalphablendmodesrc
 * Query source blend factor for alpha channel blending from selected context.
 *
 * see: <d2_setalphablendmode> for a list of blendmodes
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   alpha source blend factor. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_getalphablendmodedst>, <d2_setalphablendmode>
 * */
d2_u32 d2_getalphablendmodesrc( d2_device *handle )
{
   d2_u32 ret;
   d2_u32 blendMask;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 == (D2_DEV(handle)->hwrevision & D2FB_ALPHACHANNELBLENDING))
   {
      /* create write-alpha bits */
      blendMask = D2_DEV(handle)->ctxselected->alphablendmask & (D2C_WRITEALPHA2);
      if(0 != blendMask)
      {
         ret = d2_bm_zero;
      }
      else
      {
         ret = d2_bm_one;
      }
   } 
   else 
   {
      /* retrieve blendunit bits */
      blendMask = D2_DEV(handle)->ctxselected->alphablendmask & (D2C_BSFA | D2C_BSIA);
      switch(blendMask)
      {
         case 0:
            ret = d2_bm_one;
            break;

         case D2C_BSIA:
            ret = d2_bm_zero;
            break;

         case D2C_BSFA:
            ret = d2_bm_alpha;
            break;

         case (D2C_BSFA | D2C_BSIA):
            ret = d2_bm_one_minus_alpha;
            break;

         default:
            D2_RETERRU( handle, D2_INVALIDENUM ); 
      }
   }

   (void)D2_SETOK(handle);
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getalphablendmodedst
 * Query destination blend factor for alpha channel blending from selected context.
 *
 * see: <d2_setalphablendmode> for a list of blendmodes
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   alpha destination blend factor. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_getblendmodesrc>, <d2_setalphablendmode>
 * */
d2_u32 d2_getalphablendmodedst( d2_device *handle )
{
   d2_u32 ret;
   d2_u32 blendMask;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 == (D2_DEV(handle)->hwrevision & D2FB_ALPHACHANNELBLENDING))
   {
      /* create write-alpha bits */
      blendMask = D2_DEV(handle)->ctxselected->alphablendmask & (D2C_WRITEALPHA2);
      if(0 == blendMask)
      {
         ret = d2_bm_zero;
      }
      else
      {
         ret = d2_bm_one;
      }
   } 
   else 
   {
      /* retrieve blendunit bits */
      blendMask = D2_DEV(handle)->ctxselected->alphablendmask & (D2C_BDFA | D2C_BDIA);
      switch(blendMask)
      {
         case 0:
            ret = d2_bm_one;
            break;

         case D2C_BDIA:
            ret = d2_bm_zero;
            break;

         case D2C_BDFA:
            ret = d2_bm_alpha;
            break;

         case (D2C_BDFA | D2C_BDIA):
            ret = d2_bm_one_minus_alpha;
            break;

         default:
            D2_RETERRU( handle, D2_INVALIDENUM );
      }
   }

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getalphablendmodeflags
 * Query alpha blend flags from selected context.
 *
 * see: <d2_setalphablendmodeex> for a list of blend flags
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   alpha blend flags. 
 *   Undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *  <d2_setalphablendmodeex>
 * */
d2_u8 d2_getalphablendmodeflags( d2_device *handle )
{
   d2_u8 ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->alphablendflags;

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getantialiasing
 * Query antialiasing setting from selected context.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   Boolean (0 or 1) antialiasing setting. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *  <d2_setantialiasing>
 * */
d2_s32 d2_getantialiasing( d2_device *handle )
{
   d2_s32 ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */  /*$Misra: #DEBUG_MACRO $*/

   if(0 != (D2_DEV(handle)->ctxselected->features & d2_feat_aa))
   {
      ret = 1;
   }
   else
   {
      ret = 0;
   }

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getblur
 * Query blurring factor from selected context.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   global blurring factor (fixedpoint). undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_setblur>
 * */
d2_width d2_getblur( d2_device *handle )
{
   d2_width ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->blurring;

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getlinecap
 * Query lineend style from selected context.
 *
 * see: <d2_setlinecap> for a list of all line cap modes
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   Linecap mode. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *  <d2_setlinecap>
 * */
d2_u8 d2_getlinecap( d2_device *handle )
{
   d2_u8 ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->linecap;

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getlinejoin
 * Query polyline connection style from selected context.
 *
 * see: <d2_setlinejoin> for a list of all line join modes
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   Linejoin mode. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *  <d2_setlinejoin>
 * */
d2_u8 d2_getlinejoin( d2_device *handle )
{
   d2_u8 ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->linejoin;

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getpattern
 * Query pattern bitmask from selected context.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   Pattern bitmask. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *  <d2_setpattern>
 * */
d2_pattern d2_getpattern( d2_device *handle )
{
   d2_pattern ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ret = (d2_pattern) D2_DEV(handle)->ctxselected->orgpattern;

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_getpatternmode
 * Query pattern addressing details.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   Pattern address mode. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *  <d2_setpatternmode>
 * */
d2_u32 d2_getpatternmode( d2_device *handle )
{
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   (void)D2_SETOK( handle );
   return D2_DEV(handle)->ctxselected->patmode;
}

/*--------------------------------------------------------------------------
 * function: d2_getpatternsize
 * Query pattern bitmask size.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   Pattern bitmask size. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *  <d2_setpatternsize>
 * */
d2_s32 d2_getpatternsize( d2_device *handle )
{
   D2_VALIDATEP( handle, D2_INVALIDDEVICE ); /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   (void)D2_SETOK( handle );
   return D2_DEV(handle)->ctxselected->patlen;
}

/*--------------------------------------------------------------------------
 * function: d2_getpatternalpha
 * Query pattern transparency from selected context.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   index - register index (0 or 1 as patterns are always twocolor)
 *
 * returns:
 *   Pattern alpha. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *  <d2_setpatternalpha>
 * */
d2_alpha d2_getpatternalpha( d2_device *handle, d2_s32 index )
{
   d2_alpha ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );                    /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERRP( (((d2_u32)index) < 2U), D2_INVALIDINDEX );     /* PRQA S 3112, 4130 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->patternalpha[ index ];

   (void)D2_SETOK( handle );
   return ret;
}
