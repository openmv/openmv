/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_render.c (%version: 46 %)
 *          created Wed Jan 12 15:24:41 2005 by hh04027
 *
 * Description:
 *  %date_modified: Fri Jan 05 14:55:54 2007 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2006-04-12 CSe  fixed comments for correct documentation
 *  2007-08-31 ASc  removed tabs, changed C++ to C comments, updated
 *                  description for d2_renderpolygon 
 *  2007-12-07 MGe  Enabled usage of renderquad
 *  2008-01-07 ASc  added nullpointer check to several functions
 *  2008-01-12 ASc  fixed bug in d2_rendertristrip
 *  2008-06-12 MRe  added CLUT256 and color keying
 *  2009-02-17 MRe  added d2_wf_concave as flag for d2_renderwedge
 *  2010-09-09 MRe  added renderquad functions
 *  2011-09-05 MRe  fix pipeline wait if cr2 change 
 *  2012-09-25 BSp  MISRA cleanup
 *  2017-07-27 HFu  clearly commented and renamed d2_insertwait...dlist_intern functions 
 */

/*--------------------------------------------------------------------------
 *
 * Title: Rendering Functions
 * There is a rendering function for each supported geometric shape.
 *
 * Rendering functions are the only functions that cause entries into the
 * <Render Buffers>. Material and mode changes translate into hardware register
 * access only when something is actually rendered.
 *
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_render.h"
#include "dave_box.h"
#include "dave_line.h"
#include "dave_triangle.h"
#include "dave_quad.h"
#include "dave_circle.h"
#include "dave_wedge.h"
#include "dave_texture.h"
#include "dave_polyline.h"
#include "dave_triarray.h"

/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE void d2_checkwaitpipeline_intern( d2_devicedata *handle, d2_u32 cr2 );


/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE void d2_checkwaitpipeline_intern( d2_devicedata *handle, d2_u32 cr2 )
{
   d2_u32 cr2_blend_wait_mask = D2C_WRITEALPHA2 | D2C_WRITEALPHA1 | D2C_BDFA | D2C_BDIA | D2C_BSFA | D2C_BSIA | D2C_USE_ACB | D2C_WRITEFORMAT3 | D2C_WRITEFORMAT2 | D2C_WRITEFORMAT1;
   if( ((D2_DEV(handle)->cr2) & cr2_blend_wait_mask) != (cr2 & cr2_blend_wait_mask) )
   {
      (void)d2_insertwaitpipedlist_intern(handle);
   }
   D2_DEV(handle)->cr2 = cr2;
}

/*--------------------------------------------------------------------------
 *
 * */
void d2_rendertrifan_intern( d2_device *handle, d2_fp_triangle rtri, const d2_point *vert, const d2_u32 *flags, d2_u32 count)
{
   d2_u32 i;
   d2_u32 share;
   d2_point px1, py1, px2, py2, px3, py3;

   px1 = vert[0];  py1 = vert[1];  /* center point */
   px2 = vert[2];  py2 = vert[3];  /* first support point */

   vert += 4;

   for(i=0; i<count; i++)
   {
      /* final vertex */
      px3 = vert[0];  py3 = vert[1];
      vert += 2;

      /* compute shared edges */
      if(0 == i)
      {
         share = d2_edge2_shared;
      }
      else
      {
         share = d2_edge0_shared | d2_edge2_shared;
      }

      if(i == (count - 1))
      {
         share &= ~d2_edge2_shared;
      }

      if(NULL != flags)
      {
         share |= *flags;
         flags++;
      }

      rtri( handle, px1, py1, px2, py2, px3, py3, share );

      /* shift vertices */
      px2 = px3;
      py2 = py3;
   }
}

/*--------------------------------------------------------------------------
 *
 * */
void d2_setupmaterial_intern( d2_devicedata *handle, d2_contextdata *ctx )
{
   d2_s32 i;
   d2_s32 clut_entries;
   d2_u32 cr2_value;

   /* check if material setup can be skipped */
   if ((handle->srccontext == ctx) && ((ctx->internaldirty & d2_dirty_material) == 0))
   {
      /* last material setup was from same context and context settings have not changed */
      return;
   }

   /* update dirty flags */
   handle->srccontext = ctx;
   ctx->internaldirty &=  (d2_u8)~((d2_u8)d2_dirty_material); /* PRQA S 4130 */ /* $Misra: #MISRA_BUG_UNSIGNED_BITWISE_NOT $*/

   if (ctx->alphablendflags == d2_blendf_blendcolor2) 
   {
      ctx->cr2mask &= ~(D2C_WRITEALPHA2 | D2C_WRITEALPHA1); /* 00 = use alpha from color2 */
      D2_DLISTWRITEU( D2_COLOR2, ((d2_u32)ctx->basealpha[1] << 24) );
   }

   /* see Mantis issue #1416: when the RLE unit enable bit is reset from 1 to 0 with no request
      from the texture cache for a non-RLE texture following, a race condition may occor.
      So we make sure we only disable the RLE unit when really non-RLE texturing is following */
   cr2_value = ctx->cr2mask | D2C_RLE_ENABLE;

   /* setup material */
   switch (ctx->fillmode)
   {
      case d2_fm_color:
         D2_DLISTWRITEU( D2_COLOR1, (ctx->basecolor[0] | ctx->constalpha) );

         d2_checkwaitpipeline_intern( handle, cr2_value );

         D2_DLISTWRITEU( D2_CONTROL2, cr2_value );
         break;

      case d2_fm_twocolor:
         D2_DLISTWRITEU( D2_COLOR1, (ctx->basecolor[0] | ctx->constalpha) );

         if(ctx->alphablendflags == d2_blendf_blendcolor2)
         {
            D2_DLISTWRITEU( D2_COLOR2, (ctx->basecolor[1] | ((d2_u32)ctx->basealpha[1] << 24)) );
         }
         else
         {
            D2_DLISTWRITEU( D2_COLOR2, (ctx->basecolor[1] | ctx->constalpha) );
         }

         cr2_value |= D2C_BC2;
         d2_checkwaitpipeline_intern( handle, cr2_value );

         D2_DLISTWRITEU( D2_CONTROL2, cr2_value );
         break;

      case d2_fm_pattern:
         if(0 != (ctx->internaldirty & d2_dirty_premalpha_p))
         {
            d2_calcpatternalpha_intern( ctx );
         }

         D2_DLISTWRITEU(  D2_COLOR1, (ctx->premalpha[0] | ctx->basecolor[0]) );
         D2_DLISTWRITEU(  D2_COLOR2, (ctx->premalpha[1] | ctx->basecolor[1]) );
         D2_DLISTWRITES(  D2_PATTERN, ctx->pattern );

         cr2_value |= D2C_PATTERNENABLE | ctx->patmodemask;
         d2_checkwaitpipeline_intern( handle, cr2_value );

         D2_DLISTWRITEU( D2_CONTROL2, cr2_value );
         break;

      case d2_fm_texture:
         if(0 != (ctx->internaldirty & d2_dirty_premalpha_t))
         {
            d2_calctexturealpha_intern( ctx );
         }

         D2_DLISTWRITEU(  D2_COLOR1, ctx->texcolreg[0] );

         if(ctx->alphablendflags == d2_blendf_blendcolor2) 
         {
            D2_DLISTWRITEU(  D2_COLOR2, ( (ctx->texcolreg[1] & 0xffffffu) | (((d2_u32)ctx->basealpha[1]) << 24) ) );
         }
         else
         {
            D2_DLISTWRITEU(  D2_COLOR2, ctx->texcolreg[1] );
         }

         /* in case of RLE wait for pipeline complete before setting new texture origin */
         if( (0 != (ctx->rlemask & D2C_RLE_ENABLE)) || (0 != D2_DEV(handle)->lasttextwasrle) )
         {
            (void)d2_insertwaitfulldlist_intern( D2_DEV(handle) );
         }
         D2_DEV(handle)->lasttextwasrle = (d2_s8)((ctx->rlemask & D2C_RLE_ENABLE) != 0);
         
         D2_DLISTWRITEU(  D2_TEXORIGIN, ctx->texbase );
         D2_DLISTWRITES(  D2_TEXPITCH, ctx->texpitch );
         D2_DLISTWRITEU(  D2_TEXMASK, ctx->texwrapmask );
         D2_DLISTWRITEU(  D2_TEXCLUT_OFFSET, ctx->texclutoffset);


         if(0 != (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT))
         {
            if(0 != (D2_DEV(handle)->hwrevision & D2FB_TEXCLUT256))
            {
               clut_entries = MAX_CLUT256_ENTRIES;
            }
            else 
            {
               clut_entries = MAX_CLUT16_ENTRIES;
            }

            if(NULL != ctx->texclut_cached)
            {
               if(handle->srcclut != ctx->texclut_cached)
               {
                  ctx->texclutupload = 1;
               }
            }
            else if( (NULL != ctx->texclut) && (handle->srcclut != ctx->texclut) )
            {
               ctx->texclutupload = 1;
            }
            else
            {
               /* empty else block to satisfy MISRA rule 14.10/2004 */
            }

            if( (0 != ctx->texclutupload) && (NULL != ctx->texclut_cached) )
            {
               D2_DLISTWRITEU( D2_TEXCLUT_ADDR, 0 );

               for(i = 0; i < clut_entries; i++)
               {
                  D2_DLISTWRITEU( D2_TEXCLUT_DATA, ctx->texclut_cached[i] );
               }

               ctx->texclutupload = 0;
               handle->srcclut = ctx->texclut_cached;
            }
            else if( (0 != ctx->texclutupload) && (NULL != ctx->texclut) )
            {
               D2_DLISTWRITEU( D2_TEXCLUT_ADDR, 0 );

               for(i = 0; i < clut_entries; i++)
               {
                  D2_DLISTWRITEU( D2_TEXCLUT_DATA, ctx->texclut[i] );
               }

               ctx->texclutupload = 0;
               handle->srcclut = ctx->texclut;
            }
            else
            {
               /* empty else block to satisfy MISRA rule 14.10/2004 */
            }
         }

         /* color keying: */
         if(0 != (D2_DEV(handle)->hwrevision & D2FB_COLORKEY) )
         {
            if (ctx->colkeymask == D2C_COLKEY_ENABLE)
            {
               D2_DLISTWRITEU( D2_COLKEY, ctx->colorkey );
            }
         }

         cr2_value = D2C_TEXTUREENABLE |
                   ctx->cr2mask      |
                   ctx->texmodemask  | 
                   ctx->rlemask      | 
                   ctx->clutmask     | 
                   ctx->colkeymask;

         d2_checkwaitpipeline_intern( handle, cr2_value );

         D2_DLISTWRITEU( D2_CONTROL2, cr2_value );
         break;

      default:
         break;
   }
}

/*--------------------------------------------------------------------------
 *
 * */
void d2_startrender_intern( d2_devicedata *handle, const d2_bbox *bbox, d2_u32 delay )
{
   d2_s32 w,h;

   /* find size */
   w = D2_INT4( bbox->xmax - bbox->xmin ) + 1;    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   h = D2_INT4( bbox->ymax - bbox->ymin ) + 1;    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   /* set registers */
   D2_DLISTWRITEU(  D2_PITCH, ( ((d2_u16)handle->pitch) + (((d2_u16)delay) * 65536u) ) ); /*note : and can be avoided if pitch forced to be >0 */
   D2_DLISTWRITES(  D2_SIZE,  (h * 65536) + w );

   /* start rendering */
   switch(handle->bpp)
   {
      case 1:
         D2_DLISTWRITEU(  D2_ORIGIN, ( ((d2_s8*)handle->framebuffer) + D2_INT4(bbox->xmin) + (D2_INT4(bbox->ymin) * handle->pitch) ) );  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         break;

      case 2:
         D2_DLISTWRITEU(  D2_ORIGIN, ( ((d2_s16*)handle->framebuffer) + D2_INT4(bbox->xmin) + (D2_INT4(bbox->ymin) * handle->pitch) ) );   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         break;

      case 4:
         D2_DLISTWRITEU(  D2_ORIGIN, ( ((d2_s32*)handle->framebuffer) + D2_INT4(bbox->xmin) + (D2_INT4(bbox->ymin) * handle->pitch) ) );   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         break;

      default:
         break;
   }
}

/*--------------------------------------------------------------------------
 *
 * */
void d2_startrender_bottom_intern( d2_devicedata *handle, const d2_bbox *bbox, d2_u32 delay )
{
   d2_s32 w,h;

   /* find size */
   w = D2_INT4( bbox->xmax - bbox->xmin ) + 1;   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   h = D2_INT4( bbox->ymax - bbox->ymin ) + 1;   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   /* set registers */
   D2_DLISTWRITES(  D2_PITCH, ( ((d2_s32)((d2_u16)(-handle->pitch))) ) + (((d2_s16)delay) * 65536) );
   D2_DLISTWRITES(  D2_SIZE,  (h * 65536) + w );

   /* start rendering */
   switch (handle->bpp)
   {
      case 1:
         D2_DLISTWRITEU(  D2_ORIGIN, ( ((d2_s8*)handle->framebuffer) + D2_INT4(bbox->xmin) + (D2_INT4(bbox->ymax) * handle->pitch) ) );  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         break;

      case 2:
         D2_DLISTWRITEU(  D2_ORIGIN, ( ((d2_s16*)handle->framebuffer) + D2_INT4(bbox->xmin) + (D2_INT4(bbox->ymax) * handle->pitch) ) );   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         break;

      case 4:
         D2_DLISTWRITEU(  D2_ORIGIN, ( ((d2_s32*)handle->framebuffer) + D2_INT4(bbox->xmin) + (D2_INT4(bbox->ymax) * handle->pitch) ) );   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         break;

      default:
         break;
   }
}

/*--------------------------------------------------------------------------
 * Group: Direct Rendering */

/*--------------------------------------------------------------------------
 * function: d2_clear
 * Render fill the entire framebuffer with a single color.
 *
 * The current cliprect is used to determine framebuffer dimensions.
 * Clearing is the only function that bypasses the current solid context
 * and renders plain color - regardless of blendmodes, fillmodes and other
 * attributes.
 *
 * Because clearing does bypass the context material settings it is
 * the only function that includes both alpha and color in the 32bit 'color'
 * argument. The most significant byte is used to fill the framebuffer alpha
 * channel if one is present.
 *
 * To clear using all context attributes simply use <d2_renderbox>.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   color - fill color and alpha
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_clear( d2_device *handle, d2_color color )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   (void)d2_clearbox_solid( handle, color );

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_renderbox
 * Render a rectangle.
 *
 * Nothing is rendered if width or height are 0. Subpixel positions and
 * fractional sizes are supported.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   x1,y1 - top left corner (fixedpoint)
 *   w - width of rectangle in pixels (fixedpoint)
 *   h - height of rectangle in pixels (fixedpoint)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_renderbox( d2_device *handle, d2_point x1, d2_point y1, d2_width w, d2_width h )
{
   d2_s32 err = D2_ILLEGALMODE;

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   switch(D2_DEV(handle)->rendermode)
   {
      case d2_rm_solid :          err = d2_renderbox_solid(handle, x1, y1, w, h);
         break;
      case d2_rm_outline :        err = d2_renderbox_outline(handle, x1, y1, w, h);
         break;
      case d2_rm_solid_outlined : err = d2_renderbox_solidoutline(handle, x1, y1, w, h);
         break;
      case d2_rm_shadow :         err = d2_renderbox_shadow(handle, x1, y1, w, h);
         break;
      case d2_rm_solid_shadow :   err = d2_renderbox_solidshadow(handle, x1, y1, w, h);
         break;
      case d2_rm_postprocess:     err = d2_renderbox_solid(handle, x1, y1, w, h);
         break;

      default:
         break;
   }
   D2_RETERR(handle, err);
}

/*--------------------------------------------------------------------------
 * function: d2_renderline
 * Render a wide line.
 *
 * Subpixel positions and fractional widths are supported.
 *
 * To modify the lineend styles use <d2_setlinecap>, but note that only included
 * endpoints will get a cap. So if d2_lf_exclude_end is specified and linecap
 * is set to round - the endpoint will not get rounded.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   x1,y1 - startpoint (fixedpoint)
 *   x2,y2 - endpoint (fixedpoint)
 *   w - width of line in pixels (fixedpoint)
 *   flags - additional lineend flags
 *
 * lineend flags:
 *   d2_le_exclude_start - startpoint is not part of the line
 *   d2_le_exclude_end   - endpoint is not part of the line
 *   d2_le_exclude_none  - start and endpoint are part of the line
 *   d2_le_exclude_both  - start and endpoint are not part of the line
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_renderline2>, <d2_setlinecap>
 * */
d2_s32 d2_renderline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags )
{
   d2_s32 err = D2_ILLEGALMODE;

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   switch(D2_DEV(handle)->rendermode)
   {
      case d2_rm_solid :          err = d2_renderline_solid(handle, x1, y1, x2,y2, w, flags);
         break;
      case d2_rm_outline :        err = d2_renderline_outline(handle, x1, y1, x2,y2, w, flags);
         break;
      case d2_rm_solid_outlined : err = d2_renderline_solidoutline(handle, x1, y1, x2,y2, w, flags);
         break;
      case d2_rm_shadow :         err = d2_renderline_shadow(handle, x1, y1, x2,y2, w, flags);
         break;
      case d2_rm_solid_shadow :   err = d2_renderline_solidshadow(handle, x1, y1, x2,y2, w, flags);
         break;
      case d2_rm_postprocess:     err = d2_renderline_solid(handle, x1, y1, x2,y2, w, flags);
         break;

      default:
         break;
   }
   D2_RETERR(handle, err);
}

/*--------------------------------------------------------------------------
 * function: d2_renderline2
 * Render a wide line with 2 different widths.
 *
 * Line can have two different width at start and endpoint, forming a trapezoid. If both widths are
 * equal it is faster to use <d2_renderline> instead.
 * Subpixel positions and fractional widths are supported.
 *
 * To modify the lineend styles use <d2_setlinecap>, but note that only included
 * endpoints will get a cap. So if d2_lf_exclude_end is specified and linecap
 * is set to round - the endpoint will not get rounded.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   x1,y1 - startpoint (fixedpoint)
 *   x2,y2 - endpoint (fixedpoint)
 *   w1 - width of line in pixels at startpoint (fixedpoint)
 *   w2 - width of line in pixels at endpoint (fixedpoint)
 *   flags - additional lineend flags
 *
 * lineend flags:
 *   d2_le_exclude_start - startpoint is not part of the line
 *   d2_le_exclude_end   - endpoint is not part of the line
 *   d2_le_exclude_none  - start and endpoint are part of the line
 *   d2_le_exclude_both  - start and endpoint are not part of the line
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_renderline>, <d2_setlinecap>
 * */
d2_s32 d2_renderline2(  d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags )
{
   d2_s32 err = D2_ILLEGALMODE;

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   switch(D2_DEV(handle)->rendermode)
   {
      case d2_rm_solid :          err = d2_renderline2_solid(handle, x1, y1, x2,y2, w1,w2, flags);
         break;
      case d2_rm_outline :        err = d2_renderline2_outline(handle, x1, y1, x2,y2, w1,w2, flags);
         break;
      case d2_rm_solid_outlined : err = d2_renderline2_solidoutline(handle, x1, y1, x2,y2, w1,w2, flags);
         break;
      case d2_rm_shadow :         err = d2_renderline2_shadow(handle, x1, y1, x2,y2, w1,w2, flags);
         break;
      case d2_rm_solid_shadow :   err = d2_renderline2_solidshadow(handle, x1, y1, x2,y2, w1,w2, flags);
         break;
      case d2_rm_postprocess:     err = d2_renderline2_solid(handle, x1, y1, x2,y2, w1,w2, flags);
         break;

      default:
         break;
   }
   D2_RETERR(handle, err);
}

/*--------------------------------------------------------------------------
 * function: d2_rendertri
 * Render a triangle.
 *
 * Triangles must be specified with clockwise orientation. Subpixel positions are supported.
 * In order to get correct antialiasing and outlines shared edges must be specified using
 * edge flags.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   x1,y1 - first point (fixedpoint)
 *   x2,y2 - second point (fixedpoint)
 *   x3,y3 - third point (fixedpoint)
 *   flags - triangle edge flags
 *
 * triangle edge flags:
 * d2_edge0_shared - edge from (x1,y1) - (x2,y2) is shared
 * d2_edge1_shared - edge from (x2,y2) - (x3,y3) is shared
 * d2_edge2_shared - edge from (x3,y3) - (x1,y1) is shared
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_rendertri( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags )
{
   d2_s32 err = D2_ILLEGALMODE;

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   switch(D2_DEV(handle)->rendermode)
   {
      case d2_rm_solid :          err = d2_rendertri_solid(handle, x1, y1, x2,y2, x3,y3, flags);
         break;
      case d2_rm_outline :        err = d2_rendertri_outline(handle, x1, y1, x2,y2, x3,y3, flags);
         break;
      case d2_rm_solid_outlined : err = d2_rendertri_solidoutline(handle, x1, y1, x2,y2, x3,y3, flags);
         break;
      case d2_rm_shadow :         err = d2_rendertri_shadow(handle, x1, y1, x2,y2, x3,y3, flags);
         break;
      case d2_rm_solid_shadow :   err = d2_rendertri_solidshadow(handle, x1, y1, x2,y2, x3,y3, flags);
         break;
      case d2_rm_postprocess:     err = d2_rendertri_solid(handle, x1, y1, x2,y2, x3,y3, flags);
         break;

      default:
         break;
   }
   D2_RETERR(handle, err);
}

/*--------------------------------------------------------------------------
 * function: d2_renderquad
 * Render a quadrangle.
 *
 * A quadrangle is a convex four-sided polygon (quadrilateral is the precise mathematical term).
 * Quadrangles must be specified with clockwise orientation. Subpixel positions are supported.
 * In order to get correct antialiasing and outlines shared edges must be specified using
 * edge flags.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   x1,y1 - first point (fixedpoint)
 *   x2,y2 - second point (fixedpoint)
 *   x3,y3 - third point (fixedpoint)
 *   x4,y4 - fourth point (fixedpoint)
 *   flags - quadrangle edge flags
 *
 * triangle edge flags:
 * d2_edge0_shared - edge from (x1,y1) - (x2,y2) is shared
 * d2_edge1_shared - edge from (x2,y2) - (x3,y3) is shared
 * d2_edge2_shared - edge from (x3,y3) - (x4,y4) is shared
 * d2_edge3_shared - edge from (x4,y4) - (x1,y1) is shared
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_renderquad( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags )
{
   d2_s32 err = D2_ILLEGALMODE;

   D2_VALIDATE( handle, D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   switch (D2_DEV(handle)->rendermode)
   {
      case d2_rm_solid :          err = d2_renderquad_solid(handle, x1, y1, x2,y2, x3,y3, x4,y4, flags);
         break;
      case d2_rm_outline :        err = d2_renderquad_outline(handle, x1, y1, x2,y2, x3,y3, x4,y4, flags);
         break;
      case d2_rm_solid_outlined : err = d2_renderquad_solidoutline(handle, x1, y1, x2,y2, x3,y3, x4,y4, flags);
         break;
      case d2_rm_shadow :         err = d2_renderquad_shadow(handle, x1, y1, x2,y2, x3,y3, x4,y4, flags);
         break;
      case d2_rm_solid_shadow :   err = d2_renderquad_solidshadow(handle, x1, y1, x2,y2, x3,y3, x4,y4, flags);
         break;
      case d2_rm_postprocess:     err = d2_renderquad_solid(handle, x1, y1, x2,y2, x3,y3, x4,y4, flags);
         break;

      default:
         break;
   }
   D2_RETERR(handle, err);
}

/*--------------------------------------------------------------------------
 * function: d2_rendercircle
 * Render a circle or circle ring.
 *
 * Circles are directly rasterized and not reduced to linear parts by the hardware.
 * Subpixel positions, radii and widths are supported.
 * Nonantialiased circle rings with a fractional width below 1 pixel will
 * have drop outs (missing pixels).
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   x,y - center (fixedpoint)
 *   r - radius (fixedpoint)
 *   w - width or 0 for a solid circle (fixedpoint)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_rendercircle( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w )
{
   d2_s32 err = D2_ILLEGALMODE;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( r > 0, D2_VALUENEGATIVE );    /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( w >= 0, D2_VALUENEGATIVE );   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   switch (D2_DEV(handle)->rendermode)
   {
      case d2_rm_solid :          err = d2_rendercircle_solid(handle, x, y, r, w );
         break;
      case d2_rm_outline :        err = d2_rendercircle_outline(handle, x, y, r, w );
         break;
      case d2_rm_solid_outlined : err = d2_rendercircle_solidoutline(handle, x, y, r, w );
         break;
      case d2_rm_shadow :         err = d2_rendercircle_shadow(handle, x, y, r, w );
         break;
      case d2_rm_solid_shadow :   err = d2_rendercircle_solidshadow(handle, x, y, r, w );
         break;
      case d2_rm_postprocess:     err = d2_rendercircle_solid(handle, x, y, r, w );
         break;

      default:
         break;
   }
   D2_RETERR(handle, err);
}

/*--------------------------------------------------------------------------
 * function: d2_renderwedge
 * Render a circle arc or circle ring arc.
 *
 * Same as <d2_rendercircle> but clipped by two additional linear boundaries, resulting in an
 * arc.
 * As default the two half planes form an intersected area for the clipping region. 
 * For angles > 180 deg the flag d2_wf_concave must be used.
 * When the flag d2_wf_concave is set the two half planes form a united, concave area.
 * In order to get correct antialiasing and outlines shared edges must be specified using
 * edge flags (only linear edges can be shared).
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   x,y - center (fixedpoint)
 *   r - radius (fixedpoint)
 *   w - width or 0 for a solid circle (fixedpoint)
 *  nx1, ny1 - normal vector of first edge (16:16 fixedpoint)
 *  nx2, ny2 - normal vector of second edge (16:16 fixedpoint)
 *  flags - edge sharing and concave flags
 *
 * circle wedge flags:
 * d2_edge0_shared - first edge is shared
 * d2_edge1_shared - second edge is shared
 * d2_wf_concave - defines that the clipping region forms a united area for angles > 180 deg
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 *
 * note:
 *   d2_renderwedge internally calls <d2_setclipgradient> for drawing wedges and will
 *   overwrite the clip gradient settings!
 *
 * remarks:
 *   The figure below shows four examples of different wedges and the corresponding normal vectors (n1, n2).
 *   In order to get correct antialiasing all vectors must be normalized to a length of one.
 *   (see wedge.png)
 *
 * example:
 * (start code)
 * ...
 * d2_setcolor(handle, 0, 0x0000ff);
 * d2_renderwedge(handle, 100<<4, 100<<4, 50<<4, 0,    1<<16, 0<<16,   -46340, -46340,   0);
 * d2_renderwedge(handle, 200<<4, 100<<4, 50<<4, 0,    1<<16, 0<<16,    0<<16, -1<<16,   0);
 * d2_renderwedge(handle, 300<<4, 100<<4, 50<<4, 0,    1<<16, 0<<16,    46340, -46340,   0);
 * d2_renderwedge(handle, 400<<4, 100<<4, 50<<4, 0,    1<<16, 0<<16,    1<<16,  0<<16,   0);
 * ...
 * (end code)
 *
 * remarks:
 *   The figure below shows the same examples but with d2_wf_concave set.
 *   (see wedgeconcave.png)
 *
 * example:
 * (start code)
 * ...
 * d2_setcolor(handle, 0, 0x0000ff);
 * d2_renderwedge(handle, 100<<4, 100<<4, 50<<4, 0,    1<<16, 0<<16,   -46340, -46340,   d2_wf_concave);
 * d2_renderwedge(handle, 200<<4, 100<<4, 50<<4, 0,    1<<16, 0<<16,    0<<16, -1<<16,   d2_wf_concave);
 * d2_renderwedge(handle, 300<<4, 100<<4, 50<<4, 0,    1<<16, 0<<16,    46340, -46340,   d2_wf_concave);
 * d2_renderwedge(handle, 400<<4, 100<<4, 50<<4, 0,    1<<16, 0<<16,    1<<16,  0<<16,   d2_wf_concave);
 * ...
 * (end code)
 * */
d2_s32 d2_renderwedge( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags )
{
   d2_s32 err = D2_ILLEGALMODE;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( r > 0, D2_VALUENEGATIVE );   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( w >= 0, D2_VALUENEGATIVE );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   switch (D2_DEV(handle)->rendermode)
   {
      case d2_rm_solid :          err = d2_renderwedge_solid(handle, x, y, r, w, nx1,ny1, nx2,ny2, flags);
         break;
      case d2_rm_outline :        err = d2_renderwedge_outline(handle, x, y, r, w, nx1,ny1, nx2,ny2, flags);
         break;
      case d2_rm_solid_outlined : err = d2_renderwedge_solidoutline(handle, x, y, r, w, nx1,ny1, nx2,ny2, flags);
         break;
      case d2_rm_shadow :         err = d2_renderwedge_shadow(handle, x, y, r, w, nx1,ny1, nx2,ny2, flags);
         break;
      case d2_rm_solid_shadow :   err = d2_renderwedge_solidshadow(handle, x, y, r, w, nx1,ny1, nx2,ny2, flags);
         break;
      case d2_rm_postprocess:     err = d2_renderwedge_solid(handle, x, y, r, w, nx1,ny1, nx2,ny2, flags);
         break;

      default:
         break;
   }
   D2_RETERR(handle, err);
}

/*--------------------------------------------------------------------------
 * Group: Buffer Rendering
 * */

/* function: d2_renderpolyline
 * Render a polyline
 *
 * Line segments inside the polyline are connected as defined by <d2_setlinejoin>
 * and endpoints of the polyline are using the current linecap style set by <d2_setlinecap>.
 * If the line is closed (d2_le_closed) it has no endpoints and therefore no linecaps.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   data - pointer to an array of 2*'count' d2_point values ('count' x,y pairs)
 *   count - number of vertices
 *   w - width of polyline
 *   flags -  additional lineend flags
 *
 * lineend flags:
 *   d2_le_exclude_start - first point is not part of the polyline
 *   d2_le_exclude_end - last point is not part of the polyline
 *   d2_le_closed - polyline has no start or endpoint, last vertex is connected back to first
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_setlinecap>, <d2_setlinejoin>
 * */
d2_s32 d2_renderpolyline( d2_device *handle, const d2_point *data, d2_u32 count, d2_width w, d2_u32 flags)
{
   d2_s32 err = D2_ILLEGALMODE;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );     /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( count > 0, D2_VALUENEGATIVE );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( w >= 0, D2_VALUENEGATIVE );     /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( data, D2_NULLPOINTER );         /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   switch (D2_DEV(handle)->rendermode)
   {
      case d2_rm_solid :          err = d2_renderpolyline_solid(handle, data, count, w, flags);
         break;
      case d2_rm_outline :        err = d2_renderpolyline_outline(handle, data, count, w, flags);
         break;
      case d2_rm_solid_outlined : err = d2_renderpolyline_solidoutline(handle, data, count, w, flags);
         break;
      case d2_rm_shadow :         err = d2_renderpolyline_shadow(handle, data, count, w, flags);
         break;
      case d2_rm_solid_shadow :   err = d2_renderpolyline_solidshadow(handle, data, count, w, flags);
         break;
      case d2_rm_postprocess:     err = d2_renderpolyline_solid(handle, data, count, w, flags);
         break;

      default:
         break;
   }
   D2_RETERR(handle, err);
}


/* function: d2_renderpolyline2
 * Render a polyline with multiple width
 *
 * Similar to <d2_renderpolyline> but a width can be supplied for each vertex.
 * Line segments inside the polyline are connected by round line joins and endpoints of
 * the polyline are using the current linecap style set by <d2_setlinecap>.
 * If the line is closed (d2_le_closed) it has no endpoints and therefore no linecaps.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   data - pointer to an array of 2*'count' d2_point values ('count' x,y pairs)
 *   count - number of vertices
 *   w - pointer width to an array of 'count' d2_width values (width for each vertex)
 *   flags -  additional lineend flags
 *
 * lineend flags:
 *   d2_le_exclude_start - first point is not part of the polyline
 *   d2_le_exclude_end - last point is not part of the polyline
 *   d2_le_closed - polyline has no start or endpoint, last vertex is connected back to first
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *   <d2_setlinecap>, <d2_setlinejoin>, <d2_renderpolyline>
 * */
d2_s32 d2_renderpolyline2( d2_device *handle, const d2_point *data, d2_u32 count, const d2_width *w, d2_u32 flags)
{
   d2_s32 err = D2_ILLEGALMODE;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );     /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( count > 0, D2_VALUENEGATIVE );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( w, D2_NULLPOINTER );            /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( data, D2_NULLPOINTER );         /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   switch (D2_DEV(handle)->rendermode)
   {
      case d2_rm_solid :          err = d2_renderpolyline2_solid(handle, data, count, w, flags);
         break;
      case d2_rm_outline :        err = d2_renderpolyline2_outline(handle, data, count, w, flags);
         break;
      case d2_rm_solid_outlined : err = d2_renderpolyline2_solidoutline(handle, data, count, w, flags);
         break;
      case d2_rm_shadow :         err = d2_renderpolyline2_shadow(handle, data, count, w, flags);
         break;
      case d2_rm_solid_shadow :   err = d2_renderpolyline2_solidshadow(handle, data, count, w, flags);
         break;
      case d2_rm_postprocess:     err = d2_renderpolyline2_solid(handle, data, count, w, flags);
         break;

      default:
         break;
   }
   D2_RETERR(handle, err);
}


/* function: d2_rendertrilist
 * Render a polygon from a triangle list.
 *
 * Each triangle must be defined in clockwise order. In order to get correct antialiasing
 * and outlines, shared edges must be specified using edge flags (see: <d2_rendertri> for flag
 * definition).
 * There is one flagbyte per triangle. If no sharing is required (e.g. no antialiasing)
 * you can pass NULL for the 'flags' pointer.
 *
 * (see trilist.gif)
 * For each triangle 3 vertices are specified in the data array.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   data - pointer to an array of 6*count d2_point values (3*count x,y pairs)
 *  flags - pointer to an array to count bytes containing edgesharing flags or NULL.
 *           If NULL is passed it is assumed that no edges are shared.
 *   count - number of triangles
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *  <d2_rendertri>, <d2_rendertrifan>, <d2_rendertristrip>
 * */
d2_s32 d2_rendertrilist( d2_device *handle, const d2_point *data, const d2_u32 *flags, d2_u32 count)
{
   d2_u32 share;
   d2_u32 i;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );     /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( count > 0, D2_VALUENEGATIVE );  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( data, D2_NULLPOINTER );         /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* simple loop */
   share = 0;

   for(i=0; i<count; i++)
   {
      d2_point px1, py1, px2, py2, px3, py3;

      px1 = data[0];  py1 = data[1];
      px2 = data[2];  py2 = data[3];
      px3 = data[4];  py3 = data[5];

      data += 6;

      if(NULL != flags)
      {
         share = *flags;
         flags++;
      }

      (void)d2_rendertri( handle, px1, py1, px2, py2, px3, py3, share );
   }

   return 0;
}

/* function: d2_rendertristrip
 * Render a polygon from a triangle strip.
 *
 * When rendering connected structures using a set of triangles a triangle strip can be
 * used instead of a triangle list (<d2_rendertrilist>). The advantage is that for every
 * triangle except the first one only one additional vertex has to be specified.
 * The other two vertices are reused from the previous triangle. See diagram for explanation:
 *
 * (see tristrip.gif)
 * Triangles rendered from the order given above are :
 * - 1,2,3
 * - 2,4,3
 * - 3,4,5
 * - 4,6,5
 *
 * Note that every second triangle is flipped automatically in order to keep them all
 * in clockwise orientation.
 * Internal edges are automatically flagged as shared but you can still specify a flag
 * for each triangle in order to define additional shared edges.
 * There is one flagbyte per triangle. If no additional sharing information is required
 * you can pass NULL for the 'flags' pointer.
 *
 * The first triangle must be defined in clockwise order (others will then be clockwise as well).
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   data - pointer to an array of 2*count+4 d2_point values (count+2 x,y pairs)
 *   flags - pointer to an array to count bytes containing edgesharing flags or NULL
 *   count - number of triangles
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *  <d2_rendertri>, <d2_rendertrifan>, <d2_rendertrilist>
 * */
d2_s32 d2_rendertristrip( d2_device *handle, const d2_point *data, const d2_u32 *flags, d2_u32 count)
{
   d2_u32 i;
   d2_s32 flip;
   d2_u32 share;
   d2_point px1, py1, px2, py2, px3, py3;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );      /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( count > 0, D2_VALUENEGATIVE );   /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( data, D2_NULLPOINTER );          /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* first points */
   px1 = data[0];  py1 = data[1];
   px2 = data[2];  py2 = data[3];

   data += 4;

   flip = 0;

   for(i=0; i<count; i++)
   {
      /* third point */
      px3 = data[0];  py3 = data[1];

      data += 2;

      if(0 == flip)
      {
         /* compute shared edges */
         if(0 == i)
         {
            share = d2_edge1_shared;
         }
         else
         {
            share = d2_edge0_shared | d2_edge1_shared;
         }

         if(i == (count - 1))
         {
            share &= ~d2_edge1_shared;
         }

         if(NULL != flags)
         {
            share |= *flags;
            flags++;
         }

         /* render flipped */
         (void)d2_rendertri( handle, px1, py1, px2, py2, px3, py3, share );
         flip = 1;
      }
      else
      {
         /* compute shared edges */
         share = d2_edge1_shared | d2_edge2_shared;

         if(i == (count - 1))
         {
            share &= ~d2_edge1_shared;
         }

         if(NULL != flags)
         {
            share |= *flags;
            flags++;
         }

         /* render normal */
         (void)d2_rendertri( handle, px1, py1, px3, py3, px2, py2, share );
         flip = 0;
      }

      /* shift vertices */
      px1 = px2;
      py1 = py2;
      px2 = px3;
      py2 = py3;
   }

   return 0;
}

/* function: d2_rendertrifan
 * Render a polygon from a triangle fan
 *
 * When rendering connected structures where all triangles share a common vertex a triangle fan
 * can be used instead of a triangle list (<d2_rendertrilist>). The advantage is that for every
 * triangle except the first one only one additional vertex has to be specified.
 * The other two vertices are reused from the previous triangle and the common base vertex.
 * See diagram for explanation:
 *
 * (see trifan.gif)
 * Triangles rendered from the order given above are :
 * - 1,2,3
 * - 1,3,4
 * - 1,4,5
 *
 * Internal edges are automatically flagged as shared but you can still specify a flag
 * for each triangle in order to define additional shared edges.
 * There is one flagbyte per triangle. If no additional sharing information is required
 * you can pass NULL for the 'flags' pointer.
 *
 * The first triangle must be defined in clockwise order (others will then be clockwise as well).
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   data - pointer to an array of 2*count+4 d2_point values (count+2 x,y pairs)
 *   flags - pointer to an array to count bytes containing edgesharing flags or NULL
 *   count - number of triangles
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *  <d2_rendertri>, <d2_rendertrilist>, <d2_rendertristrip>
 * */
d2_s32 d2_rendertrifan( d2_device *handle, const d2_point *data, const d2_u32 *flags, d2_u32 count)
{
   d2_u32 rmode;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );    /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( count > 0, D2_VALUENEGATIVE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( data, D2_NULLPOINTER );        /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   rmode = D2_DEV(handle)->rendermode;

   /* render shadow part */
   if(0 != (rmode & d2_rm_shadow))
   {
      d2_rendertrifan_intern( handle, &d2_rendertri_shadow, data, flags, count);
   }

   /* render solid   part */
   if(0 != (rmode & d2_rm_solid))
   {
      if(0 != (rmode & (d2_rm_shadow | d2_rm_outline)))
      {
         d2_rendertolayer_intern( handle );
      }

      d2_rendertrifan_intern( handle, &d2_rendertri_solid, data, flags, count);

      if(0 != (rmode & (d2_rm_shadow | d2_rm_outline)))
      {
         d2_rendertobase_intern( handle );
      }
   }

   /* render outline */
   if(0 != (rmode & d2_rm_outline))
   {
      (void)d2_renderpolyline_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, data, NULL, count + 2, D2_DEV(handle)->outlinewidth, d2_le_closed, 0,0 );
   }

   return 0;
}

/* function: d2_renderpolygon
 * Render a convex polygon
 *
 * All vertices have to be in clockwise order. If seperation into monoton
 * subregions is required, internal edges will be flagged as 'shared'
 * automatically.
 * Outer edges are always nonshared for now.
 *
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   data - pointer to an array of d2_point values (count x,y pairs)
 *   count - number of points
 *   flags - reserved (should be NULL)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_renderpolygon( d2_device *handle, const d2_point *data, d2_u32 count, d2_u32 flags)
{
   (void)flags; /* PRQA S 3112 */ /* $Misra: #COMPILER_WARNING $*/

   /* quick hack. will work with convex polys only */
   if(count > 2)
   {
      return d2_rendertrifan( handle, data, NULL, (count - 2));
   }
   else
   {
      return D2_VALUENEGATIVE;
   }
}
