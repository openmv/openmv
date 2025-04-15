/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_blit.c (%version: 15 %)
 *          created Wed Jan 19 13:09:06 2005 by hh04027
 *
 * Description:
 *  %date_modified: Tue Feb 13 17:14:11 2007 %  (%derived_by:  hh74036 %)
 *
 * Changes:
 *  2006-03-08 CSe  added argb1555 format
 *  2007-09-18 ASc  fix: removed duplicate alpha calculation in d2_blitcopy
 *  2007-09-20 ASc  changed comments from C++ to C, added wrapping for blits
 *  2008-05-21 CSe  corrected documentation for d2_setblitsrc 
 *  2008-06-13 MRe  RLE unit, CLUT256, subbyte formats, color keying
 *  2008-07-17 MRe  added RGBA formats
 *  2008-11-26 MRe  quick fix for texture width up to 2047 
 *  2009-03-06 MRe  fix for texture width up to 2048 
 *  2011-03-11 MRe  improved/removed context backup for blit
 *  2011-06-16 MRe  added Alpha4, Alpha2, Alpha1 texture formats
 *  2012-09-05 MRe  added check for Alpha4, Alpha2, Alpha1 texture formats
 *  2012-09-25 BSp  MISRA cleanup
 */

/*--------------------------------------------------------------------------
 *
 * Title: Blit Functions
 * BLock Image Transfer operations
 *
 * Blit's are special <Rendering Functions> to copy one rectangle part of
 * the video memory into another part of the video memory.
 * The operation could be performed using texturemapping and boxrendering as
 * well, but the blit interface avoids setting and restoring all necessary
 * context states.
 *
 * Texel-to-pixel mapping:
 * Conceptually textures are seen the same way as a raster display: 
 * Each texel is defined at the exact center of a grid cell.
 * The driver maps the left border of the left-most texel to the left border 
 * of the left-most destination pixel and the right border of the right-most 
 * texel to the right border of the right-most destination pixel.
 *
 * Example: 
 * Find below an example of resizing a source texture with filtering enabled.
 * - A source image (3x1) is stretched to a destination rectangle (5x1).
 * - A source image (5x1) is downsized to a destination rectangle (3x1).
 * (see blit_mapping.png)
 *
 * Please not that the texel-to-pixel mapping implementation has changed 
 * for the magnification case since D2 Driver version 3.10.
 *
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_texture.h"
#include "dave_registermap.h"
#include "dave_box.h"

/*--------------------------------------------------------------------------
 *
 * */

#define D2_CHECKERR_BLITWIDTH(x) D2_CHECKERR( (x)<=(1024*2) , D2_INVALIDWIDTH )  /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
#define D2_CHECKERR_BLITHEIGHT(y) D2_CHECKERR( (y)<=1024 , D2_INVALIDHEIGHT )    /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
#define D2_CHECKERR_BLITPITCH(p) D2_CHECKERR( (p)<65536 , D2_VALUETOOBIG )       /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
#define D2_CHECKERR_BLITSRCPOSX(x) D2_CHECKERR( (p)<65536 , D2_VALUETOOBIG )     /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/


/*--------------------------------------------------------------------------
 * Group: BLIT Attributes Writes */


/*--------------------------------------------------------------------------
 * function: d2_setblitsrc
 * Specify the source for blit operation.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   ptr - address of the top left texel (coordinate 0,0)
 *   pitch - number of pixels (*not bytes*) per scanline
 *   width - width of bitmap in pixels (equal or less than pitch)
 *   height - height of bitmap in pixels
 *   format - pixel encoding type (bitmap format)
 *
 * bitmap formats:
 *
 *   d2_mode_alpha8   - monochrome 8bit per pixel
 *   d2_mode_alpha4    - monochrome 4bit per pixel
 *   d2_mode_alpha2    - monochrome 2bit per pixel
 *   d2_mode_alpha1    - monochrome 1bit per pixel
 *   d2_mode_rgb565   - colored 16bit per pixel (alpha is blue)
 *   d2_mode_argb8888 - colored 32bit per pixel
 *   d2_mode_rgba8888 - colored 32bit per pixel
 *   d2_mode_rgb888   -  (same as d2_mode_argb8888, in case of RLE textures 24bit pixel are decoded to 32bit)
 *   d2_mode_argb4444 - colored 16bit per pixel
 *   d2_mode_rgba4444 - colored 16bit per pixel
 *   d2_mode_rgb444   -  (same as d2_mode_argb4444)
 *   d2_mode_argb1555 - colored 16bit per pixel
 *   d2_mode_rgba5551 - colored 16bit per pixel
 *   d2_mode_rgb555   -  (same as d2_mode_argb1555)
 *   d2_mode_ai44     - colored, palletized 8bit per pixel, (4 bit alpha, 4 bit indexed RGB: see <d2_settexclut>)
 *   d2_mode_i8        - colored, palletized 8bit per pixel (palette is used if d2_mode_clut is also set)
 *   d2_mode_i4        - colored, palletized 4bit per pixel (palette is used if d2_mode_clut is also set)
 *   d2_mode_i2        - colored, palletized 2bit per pixel (palette is used if d2_mode_clut is also set)
 *   d2_mode_i1        - colored, palletized 1bit per pixel (palette is used if d2_mode_clut is also set)
 *
 * additional flags (can be combined with all above formats) :
 *
 *   d2_mode_rle      -  Enables the RLE unit (available if the feature bit D2FB_RLEUNIT is set (see <d2_getrevisionhw>)).
 *   d2_mode_clut     -  Enables the color look up table (for d2_mode_i8 through d2_mode_i1) (see <d2_settexclut>).
 *
 * Modes d2_mode_rgb888, d2_mode_rgb444 and d2_mode_rgb555 can be used as well. Alpha information has to
 * be ignored by not selecting the flag d2_bf_usealpha when calling <d2_blitcopy>.
 *
 * If the CLUT is not enabled with an indexed color format (d2_mode_i8 to d2_mode_i1), the index is written directly to the framebuffer.
 * The index can be combined with the offset, see <d2_settexclut_offset>.
 *
 * In case of d2_mode_alpha4 to d2_mode_alpha1 bits are MSB aligned and replicated to the lower bits of the 8 bit format of the internal A, R, G and B channels.
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * note:
 * width can be max 2048 pixel, height can be max 1024 pixel. 
 *
 *   Please notice that a cache flush using 'd1_cacheblockflush' might be necessary if memory contents were changed before!
 *   To avoid problems you can use the d1 driver memory management functions 'd1_copytovidmem' or 'd1_copyfromvidmem',
 *   which implicitly do a cache flush.
 * */
d2_s32 d2_setblitsrc( d2_device *handle, void *ptr, d2_s32 pitch, d2_s32 width, d2_s32 height, d2_u32 format )
{
   d2_contextdata *ctx;
   d2_u32 format_noflags = format & ~d2_mode_rle & ~d2_mode_clut;
   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( ptr, D2_NOVIDEOMEM );       /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR_BLITPITCH( pitch );          /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR_BLITWIDTH( width );          /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR_BLITHEIGHT( height );        /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/


   if ( (0 != (format & d2_mode_rle)) && (0 == ((D2_DEV(handle)->hwrevision) & D2FB_RLEUNIT)) )
   {
      D2_RETERR( handle, D2_ILLEGALMODE );
   }

   if ( (0 != (format & d2_mode_clut)) && (0 == ((D2_DEV(handle)->hwrevision) & D2FB_TEXCLUT)) )
   {
      D2_RETERR( handle, D2_ILLEGALMODE );
   }

   if ( (   (format_noflags == d2_mode_alpha4) 
            || (format_noflags == d2_mode_alpha2) 
            || (format_noflags == d2_mode_alpha1) )
        && ( (0 == (D2_DEV(handle)->hwrevision & D2FB_SWDAVE)) && ((D2_DEV(handle)->hwrevision & 0xff) < 0x0a) )   )
   {
      D2_RETERR( handle, D2_ILLEGALMODE );
   }


   ctx = D2_DEV(handle)->ctxselected;

   ctx->blit_src    = ptr;
   ctx->blit_pitch  = pitch;
   ctx->blit_width  = width;
   ctx->blit_height = height;
   ctx->blit_format = format;

   return D2_OK;
}

/*--------------------------------------------------------------------------
 * Group: BLIT Rendering Functions */

/*--------------------------------------------------------------------------
 * function: d2_blitcopy
 * Copy rectangle part of the source into destination.
 *
 * The source is set prior to the blit using <d2_setblitsrc> and the destination
 * will be part of the framebuffer (see: <d2_framebuffer>) just as with any normal
 * rendering function.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   srcwidth, srcheight - size of source rectangle in pixels (integer)
 *   srcx,srcy  - position in source bitmap (integer)
 *   dstwidth, dstheight - size of destination rectangle in pixels (fixedpoint)
 *   dstx,dsty - position in destination bitmap (fixedpoint)
 *   flags - any combination of blit flag bits (see below)
 *
 * blit flag bits:
 *
 *   d2_bf_filteru  - apply linear filter on U axis (x-direction)
 *   d2_bf_filterv  - apply linear filter in V axis (y-direction)
 *   d2_bf_filter - apply bilinear filter (both axis)
 *   d2_bf_wrapu - wrap bitmap on U axis (x-direction)
 *   d2_bf_wrapv - wrap bitmap on V axis (y-direction)
 *   d2_bf_wrap - wrap bitmap on U and V axis (x/y-direction)
 *   d2_bf_mirroru - mirror bitmap in U axis (x-direction)
 *   d2_bf_mirrorv - mirror bitmap in V axis (y-direction)
 *   d2_bf_colorize - bitmap colors are multiplied by color register index 0 (see: <d2_setcolor>)
 *   d2_bf_colorize2 - bitmap colors are interpolated between color register index 0 and 1 (see: <d2_setcolor>)
 *   d2_bf_usealpha - alpha value from bitmap is used
 *   d2_bf_invertalpha - alpha value from bitmap is inverted before use (requires usealpha)
 *   d2_bf_no_blitctxbackup   - for this blit don't backup context data for better performance; previous texture modes get lost and must be set again
 *
 *
 * (start code)
 *
 *     blitsrc.src:+-------------------------------------+ 
 *                 |           blitsrc.width             | 
 *                 |<----------blitsrc.pitch------------>| 
 *                 |                                     | 
 *                 |    srcx/y:+-----------------+  ^   
 *                 |           |                 |  |   
 *                 |           |<---srcwidth---->|  |   
 *                 |           |                 |  | srcheight  
 *                 |           |                 |  |   
 *                 |           +-------\\--------+  v  
 *                 |                    \\ 
 *                                       \\ 
 *         Display:  +--------------------\\----------------------+ 
 *                   |                     \\                     | 
 *                   |         dstx/y:+-----\\-------------+  ^   
 *                   |                |                    |  |   
 *                   |                |<------dstwidth---->|  |   
 *                   |                |                    |  | dstheight  
 *                   |                |                    |  |   
 *                   |                |                    |  |   
 *                   |                +--------------------+  v   
 *
 * (end)
 *
 * <d2_setblitsrc> sets parameters for the texture buffer. 
 * <d2_blitcopy> sets parameters for an area of the texture buffer and 
 * an area of the framebuffer where the texture area will be mapped to.
 * Both areas don't need to be the same size.
 *
 * note:
 * Wrapping will work with source bitmap dimensions that are integer powers of two only (2,4,8,16,32,..)!
 *
 * texture pitch can be >= 2048 if (srcheight-1) * pitch < 2048*1024 and if srcheight is multiple of dstheight
 *
 * d2_bf_filterv cannot be used if the pitch of the texture is >= 2048
 * 
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_blitcopy( d2_device *handle, d2_s32 srcwidth, d2_s32 srcheight, d2_blitpos srcx, d2_blitpos srcy, d2_width dstwidth, d2_width dstheight, d2_point dstx, d2_point dsty, d2_u32 flags )
{
   d2_s32 sw, sh;
   d2_s32 dxu, dyv, u0, v0;
   d2_s32 texWidth, texHeight;
   d2_contextdata *ctx;
   d2_color texmodecl1, texmodecl2;
   void *src = 0;
   d2_u32 format_noflags;
   d2_u32 isRLE = 0;
   d2_contextdata *blitcontext;
#ifdef D2_USEBLITBACKUPCONTEXT
   d2_contextdata *backup_context;
   d2_s32 i;
#else
   /* backup data for context content used by blit */
   d2_contextdata_backup *backup_context = D2_DEV(handle)->blitcontext_b;
   /* if d2_bf_no_blitctxbackup minimal backup on stack */
   d2_u32 fillmode_b = 0;
#endif

   /* current active context (reading) */
   ctx = D2_DEV(handle)->ctxselected;

   isRLE = (ctx->blit_format & d2_mode_rle);

   D2_VALIDATE( srcwidth>0, D2_INVALIDWIDTH );                                                   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATE( ((d2_u32)dstwidth>0) && ((d2_u32)dstwidth<=D2_FIX4(1024*2)), D2_INVALIDWIDTH );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/  /* PRQA S 3355 */ /* $Misra: #MISRA_BUG_LOGIC_COMP_TRUE_3355 $*/
   D2_VALIDATE( srcheight>0, D2_INVALIDHEIGHT );                                                 /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATE( (dstheight>0) && (dstheight<=D2_FIX4(1024)), D2_INVALIDHEIGHT );                 /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );                                                     /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATEP( ctx->blit_src, D2_NULLPOINTER );                                                /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* no v filtering if pitch too large (11bit) */
   if(ctx->blit_pitch >= (1024 * 2))
   {
      flags &= ~ d2_tm_filterv;
   }

   /* check if pitch > 11bits */
   /* pitch goes to fractional part of v limiter -> will not be used if srcheight is multiple of dstheight */
   D2_VALIDATE( (ctx->blit_pitch < (1024*2)) || ((ctx->blit_pitch >= (1024*2)) && (D2_FIX4(srcheight) >= dstheight)), D2_INVALIDHEIGHT); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/ /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

   /* check max value for ymask (21bit) */
   D2_VALIDATE( ((srcheight - 1) * ctx->blit_pitch) < (2048 * 1024), D2_INVALIDHEIGHT); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/


   /* check: wrapping will work with texture dimensions that are integer powers of two only */
   D2_VALIDATE( !( (0 != (d2_tm_wrapu & flags)) && (0 != ( (d2_u32)ctx->blit_width  & ((d2_u32)ctx->blit_width  - 1))) ), D2_INVALIDWIDTH);  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATE( !( (0 != (d2_tm_wrapv & flags)) && (0 != ( (d2_u32)ctx->blit_height & ((d2_u32)ctx->blit_height - 1))) ), D2_INVALIDHEIGHT); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* compute real texture width and height */
   texWidth  = srcwidth;
   texHeight = srcheight;

   if( (texWidth + srcx)  > ctx->blit_width  ) 
   {
      texWidth  = (ctx->blit_width - srcx);
   }

   if( (texHeight + srcy) > ctx->blit_height )
   {
      texHeight = (ctx->blit_height - srcy);
   }

   /* check: wrapping will only work with rectangle dimensions that are integer powers of two only */
   D2_VALIDATE( !( (0 != (d2_tm_wrapu & flags)) && (0 != ((d2_u32)texWidth  & ((d2_u32)texWidth  - 1))) ), D2_INVALIDWIDTH);   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATE( !( (0 != (d2_tm_wrapv & flags)) && (0 != ((d2_u32)texHeight & ((d2_u32)texHeight - 1))) ), D2_INVALIDHEIGHT);  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

#ifdef D2_USEBLITBACKUPCONTEXT
   /* switch to blit context [hard switch] */
   blitcontext = D2_DEV(handle)->blitcontext;
   backup_context = D2_DEV(handle)->ctxselected;
   D2_DEV(handle)->ctxselected = blitcontext;

   /* setup blit context */
   blitcontext->features       = ctx->features;
   blitcontext->blurring       = ctx->blurring;
   blitcontext->alphamode      = ctx->alphamode;
   blitcontext->constalpha     = ctx->constalpha;
   blitcontext->basealpha[0]   = ctx->basealpha[0];
   blitcontext->basealpha[1]   = ctx->basealpha[1];
   blitcontext->alphablendmask = ctx->alphablendmask;
   blitcontext->blendmask      = ctx->blendmask;
   blitcontext->tbstylemask    = ctx->tbstylemask;
   blitcontext->rlemask        = ctx->rlemask;
   blitcontext->clutmask       = ctx->clutmask;
   blitcontext->colkeymask     = ctx->colkeymask;
   blitcontext->cr2mask        = ctx->cr2mask;
   blitcontext->invblur        = ctx->invblur;
   blitcontext->internaldirty  = 0;
   blitcontext->texclut        = ctx->texclut;
   blitcontext->texclut_cached = ctx->texclut_cached;
   blitcontext->texclutupload  = ctx->texclutupload;
   blitcontext->texclutoffset  = ctx->texclutoffset;
   blitcontext->colorkey       = ctx->colorkey;

   /* copy gradient data if necessary */
   blitcontext->gradients = ctx->gradients;
   if(0 != blitcontext->gradients)
   {
      for(i=0; i<4; i++)
      {
         blitcontext->gradient[i] = ctx->gradient[i];
      }
   }
#else
   blitcontext = D2_DEV(handle)->ctxselected;

   /* backup to stack */
   if(backup_context && (0 == (d2_bf_no_blitctxbackup & flags)))
   {
      backup_context->fillmode          = ctx->fillmode;       
      backup_context->internaldirty     = ctx->internaldirty;
      backup_context->features          = ctx->features;
      backup_context->texcenterx        = ctx->texcenterx;
      backup_context->texcentery        = ctx->texcentery;
      backup_context->tbstylemask       = ctx->tbstylemask;
      backup_context->texbpp            = ctx->texbpp;
      backup_context->texwrapmask       = ctx->texwrapmask;
      backup_context->texmodemask       = ctx->texmodemask;
      backup_context->texmode           = ctx->texmode;
      backup_context->texpitch          = ctx->texpitch;
      backup_context->texwidth          = ctx->texwidth;
      backup_context->texheight         = ctx->texheight;
      backup_context->texbase           = ctx->texbase;
      backup_context->texsubppb         = ctx->texsubppb;
      backup_context->rlebpp            = ctx->rlebpp;
      backup_context->rlemask           = ctx->rlemask;
      backup_context->clutmask          = ctx->clutmask;
      backup_context->cr2mask           = ctx->cr2mask;
      backup_context->texmodecl[0]      = ctx->texmodecl[0];
      backup_context->texmodecl[1]      = ctx->texmodecl[1];
      backup_context->texlim[0].x1      = ctx->texlim[0].x1; 
      backup_context->texlim[0].y1      = ctx->texlim[0].y1;
      backup_context->texlim[0].xadd2   = ctx->texlim[0].xadd2;
      backup_context->texlim[0].yadd2   = ctx->texlim[0].yadd2;
      backup_context->texlim[0].xadd    = ctx->texlim[0].xadd;
      backup_context->texlim[0].yadd    = ctx->texlim[0].yadd;
      backup_context->texlim[1].xadd    = ctx->texlim[1].xadd;
      backup_context->texlim[1].yadd    = ctx->texlim[1].yadd;
   }
   else
   {
      fillmode_b = ctx->fillmode; 
   }
#endif

   (void)d2_setfillmode( handle, d2_fm_texture );
   (void)d2_settexelcenter ( handle, 0, 0 );

   /* find texture origin */
   if(0 != isRLE)
   {
      src = ctx->blit_src; /* rle textures can only be decoded from start */
    
      /* offset will be done using u0, v0 (see below) */
   }
   else
   {
      format_noflags = ctx->blit_format & ~d2_mode_rle & ~d2_mode_clut;
      switch (format_noflags)
      {
         case d2_mode_ai44 :
            if(0 == ((D2_DEV(handle)->hwrevision) & D2FB_TEXCLUT))
            {
               D2_RETERR( handle, D2_ILLEGALMODE );
            }
            blitcontext->tbstylemask = D2C_READFORMAT1 | D2C_READFORMAT3;
            blitcontext->texbpp = 1;
            /* run into next case */
         case d2_mode_alpha8 :
         case d2_mode_alpha4 :
         case d2_mode_alpha2 :
         case d2_mode_alpha1 :
         case d2_mode_i8 :
         case d2_mode_i4 :
         case d2_mode_i2 :
         case d2_mode_i1 :
            src = ((d2_s8 *)ctx->blit_src) + srcx + (srcy * ctx->blit_pitch);
            break;

         case d2_mode_rgb565 :
         case d2_mode_rgb444 :
         case d2_mode_rgb555 :
         case d2_mode_argb4444 :
         case d2_mode_argb1555 :
         case d2_mode_rgba4444 :
         case d2_mode_rgba5551 :
            src = ((d2_s16 *)ctx->blit_src) + srcx + (srcy * ctx->blit_pitch);
            break;

         case d2_mode_argb8888 :
         case d2_mode_rgba8888 :
         case d2_mode_rgb888 :
            src = ((d2_s32 *)ctx->blit_src) + srcx + (srcy * ctx->blit_pitch);
            break;

         default:
            break;
      }

   }

   /* setup texture */
   (void)d2_settexturemode( handle, flags & (d2_bf_filter | d2_bf_wrap) );
   if(0 != isRLE)
   {
      (void)d2_settexture( handle, src, ctx->blit_pitch, ctx->blit_width, ctx->blit_height, ctx->blit_format );
   }
   else
   {
      (void)d2_settexture( handle, src, ctx->blit_pitch, texWidth, texHeight, ctx->blit_format );
   }

   /* setup alpha channel operation */
   if(0 != (flags & d2_bf_usealpha))
   {
      if(0 != (flags & d2_bf_invertalpha))
      {
         /* invert alpha multiply */
         texmodecl1 = 0xff000000u; /* multiply by ctx->constalpha happens in d2_calctexturealpha_intern; */
         texmodecl2 = 0x00000000u;
      }
      else 
      {
         /* alpha multiply */
         texmodecl1 = 0x00000000u;
         texmodecl2 = 0xff000000u; /* multiply by ctx->constalpha happens in d2_calctexturealpha_intern; */
      }
   }
   else
   {
      /* alpha replace */
      texmodecl1 = 0xff000000u; /* multiply by ctx->constalpha happens in d2_calctexturealpha_intern; */
      texmodecl2 = 0xff000000u; /* multiply by ctx->constalpha happens in d2_calctexturealpha_intern; */
   }

   /* setup color channel operation */
   if(0 != (flags & d2_bf_colorize))
   {
      if(0 != (flags & d2_bf_colorize2))
      {
         /* color blend */
         texmodecl1 |= ctx->basecolor[0];
         texmodecl2 |= ctx->basecolor[1];
      }
      else
      {
         /* color multiply */
         texmodecl2 |= ctx->basecolor[0];
      }
   }
   else
   {
      if(0 != (flags & d2_bf_colorize2))
      {
         /* color blend */
         texmodecl1 |= ctx->basecolor[0];
         texmodecl2 |= ctx->basecolor[1];
      }
      else
      {
         /* color copy */
         texmodecl2 |= 0x00ffffff;
      }
   }

   /* write to context */
   blitcontext->texmodecl[0] = texmodecl1;
   blitcontext->texmodecl[1] = texmodecl2;
   blitcontext->internaldirty |= d2_dirty_premalpha_t | d2_dirty_material;

   /* calc mapping parameters */
   dxu = 65536;
   dyv = 65536;
   u0  = 0;
   v0  = 0;

   sw = D2_FIX4( srcwidth );      /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   sh = D2_FIX4( srcheight );     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

   if( (d2_u16)dstwidth > D2_FIX4(1) )
   {
      dxu = (d2_s32) ( ((d2_u32)D2_FIX16(sw)) / ((d2_u16)dstwidth) );     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/   
      u0  = dxu / 2;  /* offset by half a pixel step, as we sample in the center of our pixels */
   }
   if(dstheight > D2_FIX4(1))
   {
      dyv = (d2_s32) ( ((d2_u32)D2_FIX16(sh)) / ((d2_u16)dstheight) );    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      v0  = dyv / 2;  /* offset by half a pixel step, as we sample in the center of our pixels */
   }

   /* handle possible mirroring */
   if(0 != (flags & d2_bf_mirroru))
   {
	   dxu = -dxu;
	   u0 = D2_FIX16(srcwidth)-u0;     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   }
   if(0 != (flags & d2_bf_mirrorv))
   {
	   dyv = -dyv;
	   v0 = D2_FIX16(srcheight)-v0;    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   }

   /* filtering: offset by -1/2 texel compared to nearest neighbour, to position the 2x2 kernel correctly */
   if(0 != (flags & d2_bf_filteru))
   {  
      u0 -= D2_FIX16(1)/2;     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   }
   if(0 != (flags & d2_bf_filterv))
   {  
      v0 -= D2_FIX16(1)/2;     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   }

   /* apply possible shift in source */
   if(0 != isRLE)
   {
      u0 += D2_FIX16((d2_s32)srcx);    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      v0 += D2_FIX16((d2_s32)srcy);    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   }

   /* setup mapping parameters */
   (void)d2_settexturemapping( handle, dstx, dsty, u0, v0, dxu, 0, 0, dyv );

   /* do blit */
   (void)d2_renderbox_intern( (d2_devicedata*)handle, blitcontext, dstx, dsty, dstwidth, dstheight );

   /* restore state */
#ifdef D2_USEBLITBACKUPCONTEXT
   D2_DEV(handle)->ctxselected = backup_context;

   blitcontext->texclut_cached = NULL;
#else
   /* restore from stack */
   if(backup_context && (0 == (d2_bf_no_blitctxbackup & flags)))
   {
      ctx->fillmode        = backup_context->fillmode;       
      ctx->internaldirty   = backup_context->internaldirty;  
      ctx->features        = backup_context->features;       
      ctx->texcenterx      = backup_context->texcenterx;     
      ctx->texcentery      = backup_context->texcentery;     
      ctx->tbstylemask     = backup_context->tbstylemask;    
      ctx->texbpp          = backup_context->texbpp;         
      ctx->texwrapmask     = backup_context->texwrapmask;    
      ctx->texmodemask     = backup_context->texmodemask;    
      ctx->texmode         = backup_context->texmode;        
      ctx->texpitch        = backup_context->texpitch;       
      ctx->texwidth        = backup_context->texwidth;       
      ctx->texheight       = backup_context->texheight;      
      ctx->texbase         = backup_context->texbase;        
      ctx->texsubppb       = backup_context->texsubppb;      
      ctx->rlebpp          = backup_context->rlebpp;         
      ctx->rlemask         = backup_context->rlemask;        
      ctx->clutmask        = backup_context->clutmask;       
      ctx->cr2mask         = backup_context->cr2mask;        
      ctx->texmodecl[0]    = backup_context->texmodecl[0];   
      ctx->texmodecl[1]    = backup_context->texmodecl[1];   
      ctx->texlim[0].x1    = backup_context->texlim[0].x1;   
      ctx->texlim[0].y1    = backup_context->texlim[0].y1;   
      ctx->texlim[0].xadd2 = backup_context->texlim[0].xadd2;
      ctx->texlim[0].yadd2 = backup_context->texlim[0].yadd2;
      ctx->texlim[0].xadd  = backup_context->texlim[0].xadd; 
      ctx->texlim[0].yadd  = backup_context->texlim[0].yadd; 
      ctx->texlim[1].xadd  = backup_context->texlim[1].xadd; 
      ctx->texlim[1].yadd  = backup_context->texlim[1].yadd; 
   }
   else
   {
      (void)d2_setfillmode( handle, fillmode_b); 
   }

#endif

   return D2_OK;
}
