/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_texture.c (%version: 21 %)
 *          created Wed Jul 06 14:57:59 2005 by hh04027
 *
 * Description:
 *  %date_modified: Tue Dec 12 14:50:45 2006 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2006-03-08 CSe  added argb1555 format
 *  2007-09-20 ASc  rem. C++ comments, fixed d2_settexturemapping params 
 *  2008-04-30 MRe  added RLE and subbyte formats
 *  2008-06-12 MRe  added CLUT256 and color keying
 *  2011-06-16 MRe  added Alpha4, Alpha2, Alpha1 texture formats
 *  2012-09-05 MRe  added check for Alpha4, Alpha2, Alpha1 texture formats
 *  2012-09-25 BSp  MISRA cleanup
 */

/*--------------------------------------------------------------------------
 *
 * Title: Texture Functions
 * Modify texture mapping settings
 *
 * Texture attributes are part of the context (see: <Context Functions>) but
 * a single context might contain multiple mapping attributes if multiple
 * texture hardware units are present.
 *
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_texture.h"


/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE void d2_textureoperator( const d2_contextdata *ctx, d2_u32 index, d2_u32 mode, d2_alpha *c1, d2_alpha *c2); /* MISRA */
static D2_INLINE void d2_setupuvlimiter_intern( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox ); /* MISRA */
static D2_INLINE void d2_setupuvlimiter_invert_intern( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox ); /* MISRA */


/*--------------------------------------------------------------------------
 * Group: Texture Attribute Writes */


/*--------------------------------------------------------------------------
 * function: d2_settexture
 * Specify the source for texture mapping.
 *
 * The specified texture is used only when <d2_setfillmode> is set to *d2_fm_texture*
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   ptr - address of the top left texel (coordinate 0,0)
 *   pitch - number of texels (*not bytes*) per scanline
 *   width - width of texture in texels (equal or less than pitch)
 *   height - height of texture in texels
 *   format - texel encoding type (texture format)
 *
 * texture formats:
 *
 *   d2_mode_alpha8   - monochrome 8bit per pixel
 *   d2_mode_alpha4    - monochrome 4bit per pixel
 *   d2_mode_alpha2    - monochrome 2bit per pixel
 *   d2_mode_alpha1    - monochrome 1bit per pixel
 *   d2_mode_rgb565   - colored 16bit per pixel (alpha is blue)
 *   d2_mode_argb8888 - colored 32bit per pixel
 *   d2_mode_rgba8888 - colored 32bit per pixel
 *   d2_mode_rgb888   -  (same as d2_mode_argb8888)
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
 *   d2_mode_clut     -  Enables the color look up table (for d2_mode_i8 through d2_mode_i1) (see <d2_settexclut>).
 *
 * Modes d2_mode_rgb888, d2_mode_rgb444 and d2_mode_rgb555 can be used as well.
 * In this case alpha information has to be ignored by setting a <d2_settextureoperation> of d2_to_one for alpha.
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
 *   Please notice that a cache flush using 'd1_cacheblockflush' might be necessary if memory contents were changed before!
 *   To avoid problems you can use the d1 driver memory management functions 'd1_copytovidmem' or 'd1_copyfromvidmem',
 *   which implicitly do a cache flush.
 * */
d2_s32 d2_settexture( d2_device *handle, void *ptr, d2_s32 pitch, d2_s32 width, d2_s32 height, d2_u32 format )
{
   d2_u32 format_noflags = ((format & ~d2_mode_rle) & ~d2_mode_clut);
   d2_contextdata *ctx;
   D2_VALIDATE( handle, D2_INVALIDDEVICE );         /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( ptr, D2_NOVIDEOMEM );               /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( pitch >= 0, D2_VALUENEGATIVE );     /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( width >= 0, D2_VALUENEGATIVE );     /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( height >= 0, D2_VALUENEGATIVE );    /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->internaldirty |= d2_dirty_material;
   ctx->texpitch = pitch;
   ctx->texwidth = width;
   ctx->texheight = height;
   ctx->texbase = ptr;
   ctx->texsubppb = 0;

   if( (0 != (format & d2_mode_rle)) && (0 == (D2_DEV(handle)->hwrevision & D2FB_RLEUNIT)) )
   {
      D2_RETERR( handle, D2_ILLEGALMODE );
   }

   if( (0 != (format & d2_mode_clut)) && (0 == (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT)) )
   {
      D2_RETERR( handle, D2_ILLEGALMODE );
   }

   if( 
      (
         (d2_mode_alpha4 == format_noflags)  ||
         (d2_mode_alpha2 == format_noflags)  ||
         (d2_mode_alpha1 == format_noflags) 
       )
      && (
         (0 == (D2_DEV(handle)->hwrevision & D2FB_SWDAVE)) && 
         ((D2_DEV(handle)->hwrevision & 0xffu) < 0x0au) 
          )
       )
   {
      D2_RETERR( handle, D2_ILLEGALMODE );
   }


   /* calc bpp and mask */
   switch(format_noflags)
   {
      case d2_mode_alpha8:
         ctx->tbstylemask = 0;
         ctx->texbpp = 1;
         ctx->rlebpp = 1;
         break;

      case d2_mode_rgb565:
         ctx->tbstylemask = D2C_READFORMAT1;
         ctx->texbpp = 2;
         ctx->rlebpp = 2;
         break;

      case d2_mode_argb8888:
      case d2_mode_rgb888:
         ctx->tbstylemask = D2C_READFORMAT2;
         ctx->texbpp = 4;
         ctx->rlebpp = 4;

         if(d2_mode_rgb888 == format_noflags)
         {
            ctx->rlebpp = 3;
         }
         break;

      case d2_mode_rgba8888:
         ctx->tbstylemask = D2C_READFORMAT3 | D2C_READFORMAT2;
         ctx->texbpp = 4;
         ctx->rlebpp = 4;
         break;

      case d2_mode_argb4444:
      case d2_mode_rgb444:
         ctx->tbstylemask = D2C_READFORMAT2 | D2C_READFORMAT1;
         ctx->texbpp = 2;
         ctx->rlebpp = 2;
         break;

      case d2_mode_rgba4444:
         ctx->tbstylemask = D2C_READFORMAT3 | D2C_READFORMAT2 | D2C_READFORMAT1;
         ctx->texbpp = 2;
         ctx->rlebpp = 2;
         break;

      case d2_mode_argb1555:
      case d2_mode_rgb555:
         ctx->tbstylemask = D2C_READFORMAT3;
         ctx->texbpp = 2;
         ctx->rlebpp = 2;
         break;

      case d2_mode_rgba5551:
         ctx->tbstylemask = D2C_READFORMAT4;
         ctx->texbpp = 2;
         ctx->rlebpp = 2;
         break;

      case d2_mode_ai44:
         if(0 == (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT))
         {
            D2_RETERR( handle, D2_ILLEGALMODE );
         }
         ctx->tbstylemask = D2C_READFORMAT3 | D2C_READFORMAT1;
         ctx->texbpp = 1;
         ctx->rlebpp = 1;
         break;

      case d2_mode_i8:
         if(0 == (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT))
         {
            D2_RETERR( handle, D2_ILLEGALMODE );
         }
         ctx->tbstylemask = D2C_READFORMAT4 | D2C_READFORMAT1;
         ctx->texbpp = 1;
         ctx->rlebpp = 1;
         break;

      case d2_mode_i4:
         if(0 == (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT))
         {
            D2_RETERR( handle, D2_ILLEGALMODE );
         }
         ctx->tbstylemask = D2C_READFORMAT4 | D2C_READFORMAT2;
         ctx->texbpp = 1;
         ctx->rlebpp = 1;
         ctx->texsubppb = 2;
         break;

      case d2_mode_i2:
         if(0 == (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT))
         {
            D2_RETERR( handle, D2_ILLEGALMODE );
         }
         ctx->tbstylemask = D2C_READFORMAT4 | D2C_READFORMAT2 | D2C_READFORMAT1;
         ctx->texbpp = 1;
         ctx->rlebpp = 1;
         ctx->texsubppb = 4;
         break;

      case d2_mode_i1:
         if(0 == (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT))
         {
            D2_RETERR( handle, D2_ILLEGALMODE );
         }
         ctx->tbstylemask = D2C_READFORMAT4 | D2C_READFORMAT3;
         ctx->texbpp = 1;
         ctx->rlebpp = 1;
         ctx->texsubppb = 8;
         break;

      case d2_mode_alpha4:
         ctx->tbstylemask = D2C_READFORMAT4 | D2C_READFORMAT3 | D2C_READFORMAT1;
         ctx->texbpp = 1;
         ctx->rlebpp = 1;
         ctx->texsubppb = 2;
         break;

      case d2_mode_alpha2:
         ctx->tbstylemask = D2C_READFORMAT4 | D2C_READFORMAT3 | D2C_READFORMAT2;
         ctx->texbpp = 1;
         ctx->rlebpp = 1;
         ctx->texsubppb = 4;
         break;

      case d2_mode_alpha1:
         ctx->tbstylemask = D2C_READFORMAT4 | D2C_READFORMAT3 | D2C_READFORMAT2 | D2C_READFORMAT1;
         ctx->texbpp = 1;
         ctx->rlebpp = 1;
         ctx->texsubppb = 8;
         break;

      default:
         break;
   }

   if(0 != (format & d2_mode_rle))
   {
      ctx->rlemask  = D2C_RLE_ENABLE;
      ctx->rlemask |= ((d2_u32) ctx->rlebpp - 1) * D2C_RLEFORMAT1;
   }
   else
   {
      ctx->rlemask = 0;
   }

   if( (0 != (format & d2_mode_clut)) || (d2_mode_ai44 == format_noflags) )
   {
      ctx->clutmask  |= D2C_CLUT_ENABLE;
   }
   else
   {
      ctx->clutmask &= ~D2C_CLUT_ENABLE;
   }

   ctx->cr2mask = 
      D2_DEV(handle)->fbstylemask | 
      ctx->blendmask              | 
      ctx->tbstylemask            | 
      ctx->alphablendmask         | 
      ctx->rlemask                | 
      ctx->clutmask               | 
      ctx->colkeymask;

   ctx->internaldirty |= d2_dirty_texlim;
   d2_calctexturemask_intern( ctx );

   D2_RETOK( handle );
}

/*--------------------------------------------------------------------------
 * function: d2_settexturemode
 * Define texture addressing details.
 *
 * Texture wrapping will work with texture dimensions that are integer powers
 * of two only (2,4,8,16,32,..). Other sizes will wrap at the next higher power
 * of two.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   mode - any combination of texture mode bits (default is d2_tm_filter, see below)
 *
 * texture mode bits:
 *
 *  d2_tm_wrapu - wrap texture on U axis (x-direction)
 *  d2_tm_wrapv - wrap texture on V axis (y-direction)
 *  d2_tm_filteru - apply linear filter on U axis (x-direction)
 *  d2_tm_filterv - apply linear filter in V axis (y-direction)
 *  d2_tm_filter - apply bilinear filter (both axis)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_settexturemode( d2_device *handle, d2_u32 mode )
{
   d2_contextdata *ctx;
   D2_VALIDATE( handle, D2_INVALIDDEVICE );      /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( (mode < 256), D2_INVALIDENUM );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->internaldirty |= d2_dirty_material;

   ctx->texmodemask = 0;

   if(0 != (mode & d2_tm_filteru))
   {
      ctx->texmodemask |= D2C_TEXTUREFILTERX;
   }

   if(0 != (mode & d2_tm_filterv))
   {
      ctx->texmodemask |= D2C_TEXTUREFILTERY;
   }

   if(0 == (mode & d2_tm_wrapu))
   {
      ctx->texmodemask |= D2C_TEXTURECLAMPX;
   }

   if(0 == (mode & d2_tm_wrapv))
   {
      ctx->texmodemask |= D2C_TEXTURECLAMPY;
   }

   /* store */
   ctx->texmode = (d2_u8) (mode & (d2_tm_wrapu | d2_tm_wrapv | d2_tm_filteru | d2_tm_filterv));
   d2_calctexturemask_intern( ctx );

   D2_RETOK( handle );
}

/*--------------------------------------------------------------------------
 * function: d2_settextureoperation
 * Choose texture operation for each channel.
 *
 * Textures can be 'colorized' by a varity of operations. A textureoperation
 * can be defined for each channel (a,r,g,b) individually.
 * Depending on the chosen operation one or two additional parameters
 * have to be set using <d2_settexopparam>.
 *
 * The default setting is d2_to_one, d2_to_copy, d2_to_copy, d2_to_copy
 * and will therfore ignore any alpha information in the texture.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   amode - texture operation for alpha channel
 *   rmode - texture operation for red channel
 *   gmode - texture operation for green channel
 *   bmode - texture operation for blue channel
 *
 * texture operations:
 *   d2_to_zero - replace channel data with zero (no parameters)
 *   d2_to_one - replace channel data with one (no parameters)
 *   d2_to_replace - replace channel data with a constant (p1)
 *   d2_to_copy - copy channel data unchanged (no parameters)
 *   d2_to_invert - invert channel data (no parameters)
 *   d2_to_multiply - multiply channel data with a constant (p1)
 *   d2_to_invmultiply - multiply inverted data with a constant (p1)
 *   d2_to_blend - use channel data to blend between two constants (p1,p2)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_settexopparam>
 * */
d2_s32 d2_settextureoperation( d2_device *handle, d2_u8 amode, d2_u8 rmode, d2_u8 gmode, d2_u8 bmode )
{
   d2_contextdata *ctx;

   if(NULL != handle)
   {
      /*D2_VALIDATE( handle, D2_INVALIDDEVICE );*/

      D2_CHECKERR( (amode < 8u), D2_INVALIDENUM );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
      D2_CHECKERR( (rmode < 8u), D2_INVALIDENUM );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
      D2_CHECKERR( (gmode < 8u), D2_INVALIDENUM );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
      D2_CHECKERR( (bmode < 8u), D2_INVALIDENUM );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

      ctx = D2_DEV(handle)->ctxselected;
      ctx->internaldirty |= d2_dirty_material;

      ctx->texamode = amode;
      ctx->texrmode = rmode;
      ctx->texgmode = gmode;
      ctx->texbmode = bmode;

      /* update precalculated values */
      d2_setuptextureblend_intern( handle, ctx );

      D2_RETOK( handle );
   }
   else
   {
      return D2_INVALIDDEVICE;
   }
}

/*--------------------------------------------------------------------------
 * function: d2_settexopparam
 * Set texture operation parameter.
 *
 * Several texture operations require additional constants. See <d2_settextureoperation>
 * for a list and description of all operations and their constants.
 * The index parameter selects the color channels for which new parameters are to be set.
 *
 * Both constant have to be in the range of 0 .. 255 unused constants (e.g. p2 in most
 * operations) are ignored.
 * Note that several color channel indices can be or'ed together.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   index - color channel index / indices
 *   p1 - parameter 'p1' (see: <d2_settextureoperation>)
 *   p2 - parameter 'p2' (see: <d2_settextureoperation>)
 *
 * color channel indices:
 *   d2_cc_alpha - alpha channel
 *   d2_cc_red - red channel
 *   d2_cc_green - green channel
 *   d2_cc_blue - blue channel
 *   d2_cc_rgb - all channels except alpha
 *   d2_cc_all - all channels
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_settextureoperation>
 * */
d2_s32 d2_settexopparam( d2_device *handle, d2_u32 index, d2_u32 p1, d2_u32 p2 )
{
   d2_contextdata *ctx;

   /* (note) some compilers (e.g. MSVC) do not issue a warning if an actual function argument exceeds the datatype range
           (i.e. p1/p2 are 8bit unsigneds but passing 500 (test_texture_operations testcase) compiles without warnings) */


#ifndef _DEBUG
   D2_VALIDATE( handle, D2_INVALIDDEVICE );   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
#endif /* _DEBUG */
   
   if(NULL != handle)
   {
      d2_alpha ap1, ap2;
      /* D2_CHECKERR( p1 >= 0, D2_VALUENEGATIVE ); */
      /* D2_CHECKERR( p2 >= 0, D2_VALUENEGATIVE ); */
      D2_CHECKERR( p1 < 256, D2_VALUETOOBIG );       /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
      D2_CHECKERR( p2 < 256, D2_VALUETOOBIG );       /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

      ap1 = (d2_alpha) p1;
      ap2 = (d2_alpha) p2;

      ctx = D2_DEV(handle)->ctxselected;
      ctx->internaldirty |= d2_dirty_material;

      /* store parameters */
      if(0 != (index & d2_cc_alpha))
      {
         ctx->texop_p1[0] = ap1;
         ctx->texop_p2[0] = ap2;
      }

      if(0 != (index & d2_cc_red))
      {
         ctx->texop_p1[1] = ap1;
         ctx->texop_p2[1] = ap2;
      }

      if(0 != (index & d2_cc_green))
      {
         ctx->texop_p1[2] = ap1;
         ctx->texop_p2[2] = ap2;
      }

      if(0 != (index & d2_cc_blue))
      {
         ctx->texop_p1[3] = ap1;
         ctx->texop_p2[3] = ap2;
      }

      /* update precalculated values */
      d2_setuptextureblend_intern( handle, ctx );

      D2_RETOK( handle );
   }
   else
   {
      return D2_INVALIDDEVICE;
   }
}

/*--------------------------------------------------------------------------
 * function: d2_settexturemapping
 * Define texture mapping.
 *
 * This is the most basic function to setup the texture mapping frame of
 * reference. You can directly specify the texture increments in u and v
 * direction for stepping on the x or y axis.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   x , y - position of the (u0,v0) texel on screen (fixedpoint)
 *   u0, v0 - initial texture coordinates (valid at point x,y) (16:16 fixedpoint)
 *   dxu, dxv - texture increment for a step in x direction (16:16 fixedpoint)
 *   dyu, dyv - texture increment for a step in y direction (16:16 fixedpoint)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_settexturemapping( d2_device *handle, d2_point x, d2_point y, d2_s32 u0, d2_s32 v0, d2_s32 dxu, d2_s32 dyu, d2_s32 dxv, d2_s32 dyv )
{
   d2_contextdata *ctx;
   D2_VALIDATE( handle, D2_INVALIDDEVICE );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->internaldirty |= d2_dirty_material;

   ctx->texlim[0].x1    = x;
   ctx->texlim[0].y1    = y;
   ctx->texlim[0].xadd2 = u0;
   ctx->texlim[0].yadd2 = v0;
   ctx->texlim[0].xadd  = dxu;
   ctx->texlim[0].yadd  = dyu; /* dxv in previous version */
   /* limiter2 x1,y1,xadd2,yadd2 omitted (identical to limiter1) */
   ctx->texlim[1].xadd  = dxv; /* dyu in previous version */
   ctx->texlim[1].yadd  = dyv;

   ctx->internaldirty |= d2_dirty_texlim;

   D2_RETOK( handle );
}

/*--------------------------------------------------------------------------
 * function: d2_settexelcenter
 * Set texel center offset.
 *
 * Default texel center is top left corner (0.0 , 0.0)
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   x , y - subpixel position of the texel center (fixedpoint)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_settexelcenter( d2_device *handle, d2_point x, d2_point y )
{
   d2_contextdata *ctx;
   D2_VALIDATE( handle, D2_INVALIDDEVICE );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   ctx = D2_DEV(handle)->ctxselected;
   ctx->internaldirty |= d2_dirty_material;

   ctx->texcenterx = x << (16 - 4);  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   ctx->texcentery = y << (16 - 4);  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

   D2_RETOK( handle );
}

/*--------------------------------------------------------------------------
 * function: d2_settexclut
 * Set texture colour palette pointer.
 *
 * For indexed texture formats (see: <d2_settexture>), a colour look-up table
 * is used. This function registers a pointer to a ARGB CLUT (32 bit values, format 0xAARRGGBB)
 * with the current context. The size of the CLUT must be 16 resp. 256 words (see note).
 * 
 * The pointer needs to be persistent, as the table is not immediately copied to the context.
 * It is read once before the next object using textures is rendered or if <d2_settexclut_part> is used.
 * If the CLUT is changed later on, d2_settexclut has to be called again to trigger a new
 * upload.
 *
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   clut   - persistent pointer to CLUT
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * note:
 *   size of CLUT: -  16 x 32bit if feature bit D2FB_TEXCLUT256 = 0 (see <d2_getrevisionhw>)
 *   size of CLUT: - 256 x 32bit if feature bit D2FB_TEXCLUT256 = 1 (see <d2_getrevisionhw>)
 *
 *   the parameter clut can be NULL. If the CLUT was cached then the allocated memory will be freed (see: <d2_settexclut_part>)
 *
 * see also:
 *   <d2_settexture>, <d2_settexclut_part>
 * */
d2_s32 d2_settexclut( d2_device *handle, d2_color* clut )
{
   d2_s32 i;
   d2_s32 clut_entries;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 == (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT))
   {
      D2_RETERR( handle, D2_ILLEGALMODE );
   }

   D2_DEV(handle)->ctxselected->texclut = clut;

   if(NULL == clut)
   {
      D2_DEV(handle)->ctxselected->texclutupload = 0;
      
      if(NULL != D2_DEV(handle)->ctxselected->texclut_cached)
      {
         d1_freemem(D2_DEV(handle)->ctxselected->texclut_cached);  
         D2_DEV(handle)->ctxselected->texclut_cached = NULL;
      }
   }
   else
   {
      if(0 != (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT256))
      {
         clut_entries = MAX_CLUT256_ENTRIES;
      }
      else 
      {
         clut_entries = MAX_CLUT16_ENTRIES;
      }

      if(NULL != D2_DEV(handle)->ctxselected->texclut_cached)
      {
         for (i=0; i<clut_entries; i++)
         {
            D2_DEV(handle)->ctxselected->texclut_cached[i] = D2_DEV(handle)->ctxselected->texclut[i];
         }
      }

      D2_DEV(handle)->ctxselected->texclutupload = 1;
      D2_DEV(handle)->ctxselected->internaldirty |= d2_dirty_material;
   }

   D2_RETOK( handle );
}
 
/*--------------------------------------------------------------------------
 * function: d2_settexclut_part
 * Set a part of the color lookup table.
 *
 * Only parts of the CLUT can be set or overridden.
 * The CLUT content will be cached in the current context.
 * If a pointer to a CLUT was given by <d2_settexclut> this CLUT contents is first copied to the context.
 * The contents of clut_part is then copied to the cached CLUT starting at start_index with a length of length.
 *
 * The size of the CLUT is 256 resp. 16 (if feature bit D2FB_TEXCLUT256 = 1 resp. = 0 (see <d2_getrevisionhw>))
 *
 * parameters:
 *   handle      - device pointer (see: <d2_opendevice>)
 *   clut_part   - pointer to a segment for the CLUT
 *   start_index - start index of the CLUT where clut_part will be copied to (0..size-1)
 *   length      - number of CLUT entries to be copied (1..size)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 *
 * see also:
 *   <d2_settexclut>
 * */
d2_s32 d2_settexclut_part( d2_device *handle, const d2_color* clut_part, d2_u32 start_index, d2_u32 length )
{
   d2_u32 i;
   d2_u32 max_index;
   d2_color** pclut_cached = &(D2_DEV(handle)->ctxselected->texclut_cached);

   D2_VALIDATEP( handle, D2_INVALIDDEVICE );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 == (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT))
   {
      D2_RETERR( handle, D2_ILLEGALMODE );
   }

   if(0 != (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT256) )
   {
      max_index = 256u;
   }
   else
   {
      max_index = 16u;
   }

   if(NULL == clut_part)
   {
      D2_RETERR( handle, D2_NULLPOINTER);
   }

   D2_CHECKERR( start_index            <  max_index, D2_VALUETOOBIG );       /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( length                 >          0, D2_VALUENEGATIVE );     /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( length                 <= max_index, D2_VALUETOOBIG );       /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( (start_index + length) <= max_index, D2_VALUETOOBIG );       /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   if(NULL == *pclut_cached)
   {
      /* allocate the CLUT cache */
      *pclut_cached = (d2_color*)d1_allocmem(max_index * sizeof(d2_color));   

      if(NULL == *pclut_cached)
      {
         D2_RETERR( handle, D2_NOMEMORY );
      }

      /* initialize the CLUT cache */
      if(NULL != D2_DEV(handle)->ctxselected->texclut)
      {
         for(i=0; i<max_index; i++)
         {
            (*pclut_cached)[i] = D2_DEV(handle)->ctxselected->texclut[i];
         }
      }

      else
      {
         for(i=0; i<max_index; i++)
         {
            (*pclut_cached)[i] = 0;
         }
      }
   }

   /* copy the part to the CLUT cache */
   for(i=0; i<length; i++)
   {
      (*pclut_cached)[i+start_index] = clut_part[i];
   }

   D2_DEV(handle)->ctxselected->texclutupload = 1;
   D2_DEV(handle)->ctxselected->internaldirty |= d2_dirty_material;

   D2_RETOK( handle );
}

/*--------------------------------------------------------------------------
 * function: d2_writetexclut_direct
 * Write a part of the color lookup table directly to the render buffer.
 *
 * Only parts of the CLUT can be set.
 * The contents of clut_part is written directly to the render buffer starting at start_index with a length of length.
 *
 * The size of the CLUT is 256 resp. 16 (if feature bit D2FB_TEXCLUT256 = 1 resp. = 0 (see <d2_getrevisionhw>))
 *
 * parameters:
 *   handle      - device pointer (see: <d2_opendevice>)
 *   clut_part   - pointer to a segment for the CLUT
 *   start_index - start index of the CLUT where clut_part will be written to (0..size-1)
 *   length      - number of CLUT entries to write (1..size)
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * note:
 *   - no data needs to be cached
 *   - clut_part does not need to be persistent
 *   - a context switch does not restore the contents of the CLUT
 *
 * see also:
 *   <d2_settexclut> <d2_settexclut_part>
 * */
d2_s32 d2_writetexclut_direct( d2_device *handle, const d2_color* clut_part, d2_u32 start_index, d2_u32 length )
{
   d2_u32 i;
   d2_u32 max_index;

   D2_VALIDATEP( handle, D2_INVALIDDEVICE );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 == (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT) )
   {
      D2_RETERR( handle, D2_ILLEGALMODE );
   }

   if(0 != (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT256))
   {
      max_index = 256u;
   }
   else
   {
      max_index = 16u;
   }

   (void) max_index; /* suppress warning about unused variable in non-debug mode */
   if(NULL == clut_part)
   {
      D2_RETERR( handle, D2_NULLPOINTER);
   }

   D2_CHECKERR( start_index            <  max_index, D2_VALUETOOBIG );     /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( length                 >          0, D2_VALUENEGATIVE );   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( length                 <= max_index, D2_VALUETOOBIG );     /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( (start_index + length) <= max_index, D2_VALUETOOBIG );     /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   D2_DLISTWRITEU( D2_TEXCLUT_ADDR, start_index );

   for(i=0; i<length; i++)
   {
      D2_DLISTWRITEU( D2_TEXCLUT_DATA, clut_part[i] );
   }

   D2_RETOK( handle );
}

/*--------------------------------------------------------------------------
 * function: d2_settexclut_offset
 * Set index offset for indexed texture formats.
 *
 * For the indexed texture formats d2_mode_i4, d2_mode_i2 and d2_mode_i1 (see: <d2_settexture>)
 * an offset to the color index can be used. 
 *
 * The offset is an 8 bit resp. 4 bit value (if feature bit D2FB_TEXCLUT256 = 1 resp. = 0 (see <d2_getrevisionhw>))
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   offset - offset to index (default = 0); will be or'ed with the index of the texel
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 *
 * see also:
 *  <d2_settexclut>  <d2_settexture>
 * */
d2_s32 d2_settexclut_offset( d2_device *handle, d2_u32 offset )
{
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );              /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 == (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT))
   {
      D2_RETERR( handle, D2_ILLEGALMODE );
   }

   offset &= 0xffu;

   if(0 == (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT256))
   {
      D2_CHECKERR( offset         < 16, D2_VALUETOOBIG ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
      offset &= 0x0fu;
   }

   D2_DEV(handle)->ctxselected->texclutoffset = (d2_u8)offset;
   D2_DEV(handle)->ctxselected->internaldirty |= d2_dirty_material;

   D2_RETOK( handle );
}

/*--------------------------------------------------------------------------
 * function: d2_settexclut_format
 * Set color format of texture CLUT.
 *
 * The Color lookup table has 256 entries of 32 bit color values. Each color value is coded as ARGB8888. 
 * In case of RGB565 the CLUT 256x32bit is divided into a lower and an upper part for 256 16bit color entries. 
 * One 32bit word contains two successive 16bit color entries.
 * The lower 16 bit contain the first color entry and the upper 16 bit contain the next color entry.
 * The upper part of the CLUT can be accessed by the texture unit by setting the texclut_offset to 0x80.
 * When loading the CLUT 32bit words are written to the RAM. In this 16bit mode the driver has to take care to write the correct data.
 *
 * If the feature bit D2FB_TEXCLUT256 = 0 the format applies to 16 entries of 32 bit (see <d2_getrevisionhw>).
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   format - color format
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * color formats:
 *
 *   d2_mode_argb8888 - colored 32bit per pixel (default)
 *   d2_mode_rgb565   - colored 16bit per pixel (alpha is blue)
 *
 * see also:
 *   <d2_settexclut> <d2_settexture>
 * */
d2_s32 d2_settexclut_format( d2_device *handle, d2_u32 format )
{
   d2_contextdata *ctx;

   D2_VALIDATEP( handle, D2_INVALIDDEVICE );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 == (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT))
   {
      D2_RETERR( handle, D2_ILLEGALMODE );
   }
   ctx = D2_DEV(handle)->ctxselected;

   if(d2_mode_rgb565 == format)
   {
      ctx->clutmask  |= D2C_CLUTFORMAT1;
   }
   else
   {
      ctx->clutmask &= ~D2C_CLUTFORMAT1;
   }

   D2_DEV(handle)->ctxselected->internaldirty |= d2_dirty_material;

   D2_RETOK( handle );
}

/*--------------------------------------------------------------------------
 * function: d2_setcolorkey
 * Set the color for color keying.
 *
 * Color keying compares every RGB value of a texture pixel with color_key.
 * If the values are equal then Alpha as well as R, G and B are set to 0 to mark the pixel transparent.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   enable - enables color keying
 *   color_key  - RGB value of color key 
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * note:
 *   colorkeying is available if the feature bit D2FB_COLORKEY is set (see <d2_getrevisionhw>)
 *
 * see also:
 *   <d2_settexture>
 * */
d2_s32 d2_setcolorkey( d2_device *handle, d2_s32 enable, d2_color color_key )
{
   d2_contextdata *ctx;

   D2_VALIDATEP( handle, D2_INVALIDDEVICE );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 == (D2_DEV(handle)->hwrevision & D2FB_COLORKEY))
   {
      D2_RETERR( handle, D2_ILLEGALMODE );
   }

   ctx = D2_DEV(handle)->ctxselected;

   ctx->colorkey = color_key & 0x00ffffffu;

   if(0 != enable)
   {
      ctx->colkeymask  = D2C_COLKEY_ENABLE;
   }
   else
   {
      ctx->colkeymask  = 0;
   }

   D2_DEV(handle)->ctxselected->internaldirty |= d2_dirty_material;

   D2_RETOK( handle );
}

/*--------------------------------------------------------------------------
 * Group: Texture Attribute Queries */


/*--------------------------------------------------------------------------
 * function: d2_gettextureoperationa
 * Query texture operation for alpha channel.
 *
 * See <d2_settextureoperation> for a list of texture operations
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   texture operation. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_gettextureoperationr>, <d2_gettextureoperationg>, <d2_gettextureoperationb>
 * */
d2_u8 d2_gettextureoperationa( d2_device *handle )
{
   d2_u8 ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->texamode;

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_gettextureoperationr
 * Query texture operation for red channel.
 *
 * See <d2_settextureoperation> for a list of texture operations
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   texture operation. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_gettextureoperationa>, <d2_gettextureoperationg>, <d2_gettextureoperationb>
 * */
d2_u8 d2_gettextureoperationr( d2_device *handle )
{
   d2_u8 ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->texrmode;

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_gettextureoperationg
 * Query texture operation for green channel.
 *
 * See <d2_settextureoperation> for a list of texture operations
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   texture operation. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_gettextureoperationa>, <d2_gettextureoperationr>, <d2_gettextureoperationb>
 * */
d2_u8 d2_gettextureoperationg( d2_device *handle )
{
   d2_u8 ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->texgmode;

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_gettextureoperationb
 * Query texture operation for blue channel.
 *
 * See <d2_settextureoperation> for a list of texture operations
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *
 * returns:
 *   texture operation. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_gettextureoperationa>, <d2_gettextureoperationr>, <d2_gettextureoperationg>
 * */
d2_u8 d2_gettextureoperationb( d2_device *handle )
{
   d2_u8 ret;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   ret = D2_DEV(handle)->ctxselected->texbmode;

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_gettexopparam1
 * Query texture operation parameter p1.
 *
 * See <d2_settexopparam> and <d2_settextureoperation> for details
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   index - color channel index
 *
 * color channel indices:
 *   d2_cc_alpha - alpha channel
 *   d2_cc_red - red channel
 *   d2_cc_green - green channel
 *   d2_cc_blue - blue channel
 *
 * returns:
 *   texture operation parameter p1. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_settexopparam>
 * */
d2_alpha d2_gettexopparam1( d2_device *handle, d2_u32 index )
{
   d2_alpha ret = (d2_alpha) D2_OK;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 != (index & d2_cc_alpha))
   {
      ret = D2_DEV(handle)->ctxselected->texop_p1[0];
   }

   if(0 != (index & d2_cc_red))
   {
      ret = D2_DEV(handle)->ctxselected->texop_p1[1];
   }

   if(0 != (index & d2_cc_green))
   {
      ret = D2_DEV(handle)->ctxselected->texop_p1[2];
   }

   if(0 != (index & d2_cc_blue))
   {
      ret = D2_DEV(handle)->ctxselected->texop_p1[3];
   }

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 * function: d2_gettexopparam2
 * Query texture operation parameter p2.
 *
 * See <d2_settexopparam> and <d2_settextureoperation> for details
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   index - color channel index
 *
 * color channel indices:
 *   d2_cc_alpha - alpha channel
 *   d2_cc_red - red channel
 *   d2_cc_green - green channel
 *   d2_cc_blue - blue channel
 *
 * returns:
 *   texture operation parameter p2. undefined in
 *   case of an error (check with <d2_geterror> or <d2_geterrorstring>)
 *
 * see also:
 *   <d2_settexopparam>
 * */
d2_alpha d2_gettexopparam2( d2_device *handle, d2_u32 index )
{
   d2_alpha ret = (d2_alpha)D2_OK;
   D2_VALIDATEP( handle, D2_INVALIDDEVICE );   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 != (index & d2_cc_alpha))
   {
      ret = D2_DEV(handle)->ctxselected->texop_p2[0];
   }

   if(0 != (index & d2_cc_red))
   {
      ret = D2_DEV(handle)->ctxselected->texop_p2[1];
   }

   if(0 != (index & d2_cc_green))
   {
      ret = D2_DEV(handle)->ctxselected->texop_p2[2];
   }

   if(0 != (index & d2_cc_blue))
   {
      ret = D2_DEV(handle)->ctxselected->texop_p2[3];
   }

   (void)D2_SETOK( handle );
   return ret;
}

/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE void d2_textureoperator( const d2_contextdata *ctx, d2_u32 index, d2_u32 mode, d2_alpha *c1, d2_alpha *c2)
{
   switch(mode)
   {
      case d2_to_zero:
         *c1 = 0x00u; *c2 = 0x00u;
         break;

      case d2_to_one:
         *c1 = 0xffu; *c2 = 0xffu;
         break;

      case d2_to_replace:
         *c1 = ctx->texop_p1[index]; *c2 = ctx->texop_p1[index];
         break;

      case d2_to_copy:
         *c1 = 0x00u; *c2 = 0xffu;
         break;

      case d2_to_invert:
         *c1 = 0xffu; *c2 = 0x00u;
         break;

      case d2_to_multiply:
         *c1 = 0x00u; *c2 = ctx->texop_p1[index];
         break;

      case d2_to_invmultiply:
         *c1 = ctx->texop_p1[index]; *c2 = 0x00u;
         break;

      case d2_to_blend:
         *c1 = ctx->texop_p1[index]; *c2 = ctx->texop_p2[index];
         break;
         
      default:
         break;
   }
}

/*--------------------------------------------------------------------------
 * precalculate color register settings for current texture blendmode
 * */
void d2_setuptextureblend_intern( const d2_devicedata *handle, d2_contextdata *ctx )
{
   d2_alpha c1 = 0 , c2 = 0;
   d2_color b1, b2;

   /* unused parameter */
   (void) handle;   /* PRQA S 3112 */ /* $Misra: #COMPILER_WARNING $*/

   /* build alpha channel operands */
   d2_textureoperator( ctx, 0, ctx->texamode, &c1, &c2);
   b1 = (d2_u32)c1 << 24;
   b2 = (d2_u32)c2 << 24;

   /* build red channel operands */
   d2_textureoperator( ctx, 1, ctx->texrmode, &c1, &c2);
   b1 |= (d2_u32)c1 << 16;
   b2 |= (d2_u32)c2 << 16;

   /* build green channel operands */
   d2_textureoperator( ctx, 2, ctx->texgmode, &c1, &c2);
   b1 |= (d2_u32)c1 << 8;
   b2 |= (d2_u32)c2 << 8;

   /* build blue channel operands */
   d2_textureoperator( ctx, 3, ctx->texbmode, &c1, &c2);
   b1 |= (d2_u32)c1;
   b2 |= (d2_u32)c2;

   /* writeback */
   ctx->texmodecl[0] = b1;
   ctx->texmodecl[1] = b2;

   /* mark as dirty */
   ctx->internaldirty |= d2_dirty_premalpha_t;
}

/*--------------------------------------------------------------------------
 * */
void d2_calctexturealpha_intern( d2_contextdata *ctx )
{
   d2_u32 texAlpha;
   d2_color tc1, tc2;

   texAlpha = ctx->constalpha >> 8;
   tc1 = ((texAlpha * (ctx->texmodecl[0] >> 24)) + 0x00ffffffu) & 0xff000000u;
   tc2 = ((texAlpha * (ctx->texmodecl[1] >> 24)) + 0x00ffffffu) & 0xff000000u;

   ctx->texcolreg[0] = (ctx->texmodecl[0] & 0x00ffffffu) | tc1;
   ctx->texcolreg[1] = (ctx->texmodecl[1] & 0x00ffffffu) | tc2;

   ctx->internaldirty &= ~d2_dirty_premalpha_t & 255u;
}

/*--------------------------------------------------------------------------
 * */
void d2_calctexturemask_intern( d2_contextdata *ctx )
{
   d2_u32 xmask, ymask;

   if(0 != (ctx->texmode & d2_tm_wrapu))
   {
      /* wrapping. find next larger power of two - 1 */
      xmask = (d2_u32) d2_pow2mask( (d2_u32) ctx->texwidth - 1 );
   }
   else
   {
      /* clamping */
      xmask = (d2_u32) (ctx->texwidth - 1);
   }

   if(0 != (ctx->texmode & d2_tm_wrapv))
   {
      /* wrapping. find next larger power of two - 1 */
      ymask = (d2_u32) d2_pow2mask( (d2_u32) ctx->texheight - 1 );
      ymask = (d2_u32) d2_pow2mask( (d2_u32)((d2_s32)ymask * ctx->texpitch) );
   }
   else
   {
      /* clamping */
      ymask = (d2_u32) (ctx->texheight - 1);
      ymask = (d2_u32) ((d2_s32)ymask * ctx->texpitch);
   }

   /* store */
   ctx->texwrapmask = (ymask << 11) | (xmask & 0x7ffu);
}

/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE void d2_setupuvlimiter_intern( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox )
{
   d2_s32 x, y, s1, s2, xa, ya;

   x = ctx->texlim[0].x1 - bbox->xmin;
   y = ctx->texlim[0].y1 - bbox->ymin;
   s1 = -( ((x * ctx->texlim[0].xadd) + (y * ctx->texlim[0].yadd)) >> 4);   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   s2 = -( ((x * ctx->texlim[1].xadd) + (y * ctx->texlim[1].yadd)) >> 4);   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   s1 += ctx->texcenterx + ctx->texlim[0].xadd2;
   s2 += ctx->texcentery + ctx->texlim[0].yadd2;

   D2_DLISTWRITES( D2_LUSTART, s1 );
   D2_DLISTWRITES( D2_LUXADD , ctx->texlim[0].xadd );
   D2_DLISTWRITES( D2_LUYADD , ctx->texlim[0].yadd );

   xa = ctx->texlim[1].xadd;
   ya = ctx->texlim[1].yadd;
   D2_DLISTWRITES( D2_LVSTARTI, (s2 >> 16) * ctx->texpitch );     /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   D2_DLISTWRITEU( D2_LVSTARTF, (d2_u16)s2 );

   D2_DLISTWRITES( D2_LVXADDI, (xa >> 16) * ctx->texpitch );      /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   D2_DLISTWRITES( D2_LVYADDI, (ya >> 16) * ctx->texpitch );      /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   D2_DLISTWRITEU( D2_LVYXADDF, ((d2_u16)xa) | (((d2_u32)((d2_u16)ya)) << 16) );

   return;
}

/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE void d2_setupuvlimiter_invert_intern( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox )
{
   d2_s32 x, y, s1 , s2, xa, ya;

   x = ctx->texlim[0].x1 - bbox->xmin;
   y = ctx->texlim[0].y1 - bbox->ymax;
   s1 = -( ((x * ctx->texlim[0].xadd) + (y * ctx->texlim[0].yadd)) >> 4);    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   s2 = -( ((x * ctx->texlim[1].xadd) + (y * ctx->texlim[1].yadd)) >> 4);    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   s1 += ctx->texcenterx + ctx->texlim[0].xadd2;
   s2 += ctx->texcentery + ctx->texlim[0].yadd2;

   D2_DLISTWRITES( D2_LUSTART, s1 );
   D2_DLISTWRITES( D2_LUXADD , ctx->texlim[0].xadd );
   D2_DLISTWRITES( D2_LUYADD , -ctx->texlim[0].yadd );

   xa = ctx->texlim[1].xadd;
   ya = -ctx->texlim[1].yadd;
   D2_DLISTWRITES( D2_LVSTARTI, (s2 >> 16) * ctx->texpitch );             /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   D2_DLISTWRITEU( D2_LVSTARTF, (d2_u16)s2 );

   D2_DLISTWRITES( D2_LVXADDI, (xa >> 16) * ctx->texpitch );              /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   D2_DLISTWRITES( D2_LVYADDI, (ya >> 16) * ctx->texpitch );              /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   D2_DLISTWRITEU( D2_LVYXADDF, ((d2_u16)xa) | (((d2_u32)((d2_u16)ya)) << 16) );  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

   return;
}

/*--------------------------------------------------------------------------
 *
 * */
void d2_setuptexture( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox, d2_s32 flip )
{
   if(d2_fm_texture == ctx->fillmode)
   {
      if(0 != flip)
      {
         d2_setupuvlimiter_invert_intern( handle, ctx, bbox );
      }
      else
      {
         d2_setupuvlimiter_intern( handle, ctx, bbox );
      }
   }
}
