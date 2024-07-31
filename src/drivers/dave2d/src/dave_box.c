/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_box.c (%version: 17 %)
 *          created Wed Jan 19 13:09:06 2005 by hh04027
 *
 * Description:
 *  %date_modified: Fri Jan 05 13:49:37 2007 %  (%derived_by:  hh04027 %)
 *
 *  2008-01-10 ASc  removed tabs, changed C++ to C comments, fixed small
 *                  bug when using clearing optimizations
 *  2008-01-10 ASc  fixed bug in d2_clearbox_intern
 *  2010-09-24 MRe  removed inline from function d2_renderbox_inline
 *  2010-09-30 MRe  fix: optimized clear not allowed for unaligned fb start and width
 *  2012-09-25 BSp  MISRA cleanup
 *  2013-03-15 MRe  fix d2_clear: insert dlist waits
 *  2017-07-27 HFu  clearly commented and renamed d2_insertwait...dlist_intern functions 
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_viewport.h"
#include "dave_render.h"
#include "dave_pattern.h"
#include "dave_box.h"
#include "dave_texture.h"
#include "dave_polyline.h"

/*--------------------------------------------------------------------------*/

static void d2_clearbox_intern( d2_devicedata *handle, const d2_contextdata *ctx, d2_color clearcol );

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderbox_inline( d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_width w, d2_width h )
{
   d2_bbox bbox;
   d2_s32 xas,yas;
   d2_u32 control;
   d2_point x2,y2;

   /* reject invisible */
   if (((d2_u16)w < D2_EPSILON) || (h < D2_EPSILON))
   {
      return 0;
   }
   w = (d2_width)(w-D2_FIX4(1));
   h = (d2_width)(h-D2_FIX4(1));

   /* find endpoint */
   x2 = (d2_point)(x1 + w);
   y2 = (d2_point)(y1 + h);

   /* find bounding box */
   bbox.xmin = (d2_s16) D2_FLOOR4( x1 );
   bbox.ymin = (d2_s16) D2_FLOOR4( y1 );
   bbox.xmax = (d2_s16) D2_CEIL4( x2 );
   bbox.ymax = (d2_s16) D2_CEIL4( y2 );

   /* clipping */
   if (! d2_clipbbox_intern( handle, &bbox ) )
   {
      return 0;
   }

   /* set register values (material parameters) */
   d2_setupmaterial_intern( handle, ctx );

   /* check for xaxis/yaxis subpixels (0 if none) */
   if(0 != (ctx->features & d2_feat_aa))
   {
      xas = (d2_s32) D2_FRAC4( (d2_u32)x1 | (d2_u32)x2 );
      yas = (d2_s32) D2_FRAC4( (d2_u32)y1 | (d2_u32)y2 );
   }
   else
   {
      xas = 0;
      yas = 0;
   }

   /* special case blurred boxes */
   if(0 != (ctx->features & d2_feat_blur))
   {
      D2_DLISTWRITES( D2_L1START, (d2_s32)( ((((d2_u32)(bbox.xmin - x1)) * ctx->invblur) >> 4) + ctx->invblur ) );
      D2_DLISTWRITES( D2_L1XADD , (d2_s32) ctx->invblur );
      D2_DLISTWRITES( D2_L1YADD , 0 );

      D2_DLISTWRITES( D2_L2START, (d2_s32)( ((((d2_u32)(x2 - bbox.xmin)) * ctx->invblur) >> 4) + ctx->invblur ) );
      D2_DLISTWRITES( D2_L2XADD , -(d2_s32)ctx->invblur );
      D2_DLISTWRITES( D2_L2YADD , 0 );

      D2_DLISTWRITES( D2_L3START, (d2_s32)( ((((d2_u32)(bbox.ymin - y1)) * ctx->invblur) >> 4) + ctx->invblur ) );
      D2_DLISTWRITES( D2_L3XADD , 0 );
      D2_DLISTWRITES( D2_L3YADD , (d2_s32) ctx->invblur );

      D2_DLISTWRITES( D2_L4START, (d2_s32)( ((((d2_u32)(y2 - bbox.ymin)) * ctx->invblur) >> 4) + ctx->invblur ) );
      D2_DLISTWRITES( D2_L4XADD , 0 );
      D2_DLISTWRITES( D2_L4YADD , -(d2_s32)ctx->invblur );

      control = d2_initgradients_intern( handle, ctx, &bbox, 4, D2C_LIM1ENABLE | D2C_LIM2ENABLE | D2C_LIM3ENABLE | D2C_LIM4ENABLE );
   }
   else
   {
      /* choose optimal limiter count */
      if(0 == xas)
      {
         if(0 == yas)
         {
            /* no subpixel positioning */

            control = d2_initgradients_intern( handle, ctx, &bbox, 0, 0 );
         }
         else
         {
            /* yaxis subpixel only */

            D2_DLISTWRITES( D2_L1START, (d2_s32)( ((((d2_u32)(bbox.ymin - y1)) << (16-4)) + D2_FIX16(1)) ) );
            D2_DLISTWRITES( D2_L1XADD , D2_FIX16(0) );
            D2_DLISTWRITES( D2_L1YADD , D2_FIX16(1) );

            D2_DLISTWRITES( D2_L2START, (d2_s32)( ((((d2_u32)(y2 - bbox.ymin)) << (16-4)) + D2_FIX16(1)) ) );
            D2_DLISTWRITES( D2_L2XADD , D2_FIX16(0) );
            D2_DLISTWRITES( D2_L2YADD , D2_FIX16(-1) );   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

            control = d2_initgradients_intern( handle, ctx, &bbox, 2, D2C_LIM1ENABLE | D2C_LIM2ENABLE );
         }
      }
      else
      {
         if(0 == yas)
         {
            /* xaxis subpixel only */
            D2_DLISTWRITES( D2_L1START, (d2_s32)( ((((d2_u32)(bbox.xmin - x1)) << (16-4)) + D2_FIX16(1)) ) );
            D2_DLISTWRITES( D2_L1XADD , D2_FIX16(1) );   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            D2_DLISTWRITES( D2_L1YADD , D2_FIX16(0) );

            D2_DLISTWRITES( D2_L2START, (d2_s32)( ((((d2_u32)(x2 - bbox.xmin)) << (16-4)) + D2_FIX16(1)) ) );
            D2_DLISTWRITES( D2_L2XADD , D2_FIX16(-1) );  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            D2_DLISTWRITES( D2_L2YADD , D2_FIX16(0) );

            control = d2_initgradients_intern( handle, ctx, &bbox, 2, D2C_LIM1ENABLE | D2C_LIM2ENABLE );
         }
         else
         {
            /* full subpixel positioning */

            D2_DLISTWRITES( D2_L1START, (d2_s32)( (((d2_u32)(bbox.xmin - x1)) << (16-4)) + D2_FIX16(1) ) );
            D2_DLISTWRITES( D2_L1XADD , D2_FIX16(1) );   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            D2_DLISTWRITES( D2_L1YADD , D2_FIX16(0) );

            D2_DLISTWRITES( D2_L2START, (d2_s32)( (((d2_u32)(x2 - bbox.xmin)) << (16-4)) + D2_FIX16(1) ) );
            D2_DLISTWRITES( D2_L2XADD , D2_FIX16(-1) );  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            D2_DLISTWRITES( D2_L2YADD , D2_FIX16(0) );

            D2_DLISTWRITES( D2_L3START, (d2_s32)( (((d2_u32)(bbox.ymin - y1)) << (16-4)) + D2_FIX16(1) ) );
            D2_DLISTWRITES( D2_L3XADD , D2_FIX16(0) );
            D2_DLISTWRITES( D2_L3YADD , D2_FIX16(1) );   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

            D2_DLISTWRITES( D2_L4START, (d2_s32)( (((d2_u32)(y2 - bbox.ymin)) << (16-4)) + D2_FIX16(1) ) );
            D2_DLISTWRITES( D2_L4XADD , D2_FIX16(0) );
            D2_DLISTWRITES( D2_L4YADD , D2_FIX16(-1) );  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ 

            control = d2_initgradients_intern( handle, ctx, &bbox, 4, D2C_LIM1ENABLE | D2C_LIM2ENABLE | D2C_LIM3ENABLE | D2C_LIM4ENABLE );
         }
      }
   }

   /* setup pattern/texture if used */
   d2_setuppattern( handle, ctx, &bbox, 0 );
   d2_setuptexture( handle, ctx, &bbox, 0 );

   /* render */
   D2_DLISTWRITEU( D2_CONTROL, control );
   d2_startrender_intern( handle, &bbox, 0 );

   return 1;
}

/*--------------------------------------------------------------------------
 *
 * */
static void d2_clearbox_intern( d2_devicedata *handle, const d2_contextdata *ctx, d2_color clearcol )
{
   d2_bbox  bbox;
   d2_s32      s;

   /* unused parameter */
   (void) ctx;   /* PRQA S 3112 */ /* $Misra: #COMPILER_WARNING $*/

   /* always clear entier cliprect area */
   bbox.xmin = handle->clipxmin;
   bbox.xmax = handle->clipxmax;
   bbox.ymin = handle->clipymin;
   bbox.ymax = handle->clipymax;

   /* find scaling for size in bpp4 */
   s = (4 / handle->bpp);

   /* check if optimisation is applicable (no pitch) */
   if (((handle->flags & d2_df_no_dwclear) == 0) &&
       (handle->bpp < 4) &&
       (((handle->pitch / s) * s) == handle->pitch)  &&            /* pitch must be divisible by s without remainder */
       (((d2_u32)handle->framebuffer & (~3u)) == (d2_u32)handle->framebuffer) && /* fb start must be divisible by 4 */
       ((handle->fbwidth & (~3u)) == handle->fbwidth) &&   /* width must be divisible by 4 */
       ( D2_INT4(bbox.xmin) == 0 ) &&                          /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
       ( D2_INT4(bbox.xmax) == (d2_s32)(handle->fbwidth-1) ))  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   {
      d2_color dblcolor;
      d2_s32 w,h;

      /* can fill using bpp4. convert color */
      switch (handle->fbformat)
      {
         case d2_mode_alpha8:
            clearcol = (clearcol & 0xff000000u) >> 24;
            dblcolor = (clearcol << 24) | (clearcol << 16) | (clearcol << 8) | (clearcol);
            break;

         case d2_mode_argb4444:
         case d2_mode_rgb444:
            clearcol = (((clearcol & 0xf0000000u) >> 16) |
                        ((clearcol & 0x00f00000u) >> 12) |
                        ((clearcol & 0x0000f000u) >> 8) |
                        ((clearcol & 0x000000f0u) >> 4));
            dblcolor = (clearcol << 16) | (clearcol);
            break;

         case d2_mode_rgba4444:
            clearcol = (((clearcol & 0xf0000000u) >> 28) |
                        ((clearcol & 0x00f00000u) >> 8) |
                        ((clearcol & 0x0000f000u) >> 4) |
                        ((clearcol & 0x000000f0u) >> 0));
            dblcolor = (clearcol << 16) | (clearcol);
            break;

         case d2_mode_rgb565:
            clearcol = (((clearcol & 0x00f80000u) >> 8) |
                        ((clearcol & 0x0000fc00u) >> 5) |
                        ((clearcol & 0x000000f8u) >> 3));
            dblcolor = (clearcol << 16) | (clearcol);
            break;

         case d2_mode_rgba8888:
            dblcolor = (clearcol<<8) | (clearcol>>24);
            break;

         default:
            dblcolor = clearcol;
            break;
      }

      /* find size */
      w = D2_INT4( bbox.xmax - bbox.xmin ) + 1;   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      h = D2_INT4( bbox.ymax - bbox.ymin ) + 1;   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

      /* insert dlist wait */
      (void)d2_insertwaitpipedlist_intern( handle );

      /* start optimized clear */
      D2_DLISTWRITES( D2_COLOR1, (d2_s32) dblcolor );
      D2_DLISTWRITES( D2_CONTROL, 0 );
      D2_DLISTWRITES( D2_CONTROL2, D2C_WRITEFORMAT2 | D2C_BDI | D2C_WRITEALPHA1 | D2C_RLE_ENABLE );  /* D2C_RLE_ENABLE seems irrelevant here but is necessary for a HW issue: Mantis #1416 */
      D2_DLISTWRITES( D2_PITCH, (d2_s32)( ((d2_u32)(handle->pitch / s)) & 0xffffu ) );
      D2_DLISTWRITES( D2_SIZE,  (d2_s32)( ((d2_u32)h << 16) | ((d2_u32)w / (d2_u32)s) ) );

      /* start rendering */
      switch (handle->bpp)
      {
         case 1:
            D2_DLISTWRITES(  D2_ORIGIN, (d2_s32) (((d2_s8*)handle->framebuffer) + D2_INT4(bbox.xmin) + (D2_INT4(bbox.ymin) * handle->pitch)) );  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            break;

         case 2:
            D2_DLISTWRITES(  D2_ORIGIN, (d2_s32) (((d2_s16*)handle->framebuffer) + D2_INT4(bbox.xmin) + (D2_INT4(bbox.ymin) * handle->pitch)) ); /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            break;

         case 4:
            D2_DLISTWRITES(  D2_ORIGIN, (d2_s32) (((d2_s32*)handle->framebuffer) + D2_INT4(bbox.xmin) + (D2_INT4(bbox.ymin) * handle->pitch)) ); /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            break;

         default:
            break;
      }

      /* insert dlist wait */
      (void)d2_insertwaitpipedlist_intern( handle );
   }
   else
   {
      (void)d2_insertwaitpipedlist_intern( handle );
      /* direct clear (special setup for clear material) */
      D2_DLISTWRITES( D2_COLOR1, (d2_s32) clearcol );
      D2_DLISTWRITES( D2_CONTROL, 0 );
      D2_DLISTWRITES( D2_CONTROL2, (d2_s32) (D2_DEV(handle)->fbstylemask | D2C_BDI | D2C_WRITEALPHA1 | D2C_RLE_ENABLE) );  /* D2C_RLE_ENABLE seems irrelevant here but is necessary for a HW issue: Mantis #1416 */
      d2_startrender_intern( handle, &bbox, 0 );
      (void)d2_insertwaitpipedlist_intern( handle );
   }
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderbox_intern(  d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_width w, d2_width h )
{
   return d2_renderbox_inline( D2_DEV(handle), ctx, x1, y1, w, h );
}


/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderbox_solid( d2_device *handle, d2_point x1, d2_point y1, d2_width w, d2_width h )
{
   (void)d2_renderbox_inline( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1, y1, w, h );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderbox_shadow( d2_device *handle, d2_point x1, d2_point y1, d2_width w, d2_width h )
{
   (void)d2_renderbox_inline( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(D2_DEV(handle)->soffx + x1), (d2_point)(D2_DEV(handle)->soffy + y1), w, h );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderbox_outline( d2_device *handle, d2_point x1, d2_point y1, d2_width w, d2_width h )
{
   d2_width wo = D2_DEV(handle)->outlinewidth;
   d2_contextdata *ctx = D2_DEV(handle)->ctxoutline;
   d2_point verts[4 * 2];

   /* render outline */
   if (wo >= D2_EPSILON)
   {
      d2_width woh = wo / 2;

      if( (wo == D2_FIX4(1)) || 
          (
             (ctx->linejoin == d2_lj_miter)       && 
             (ctx->miterlimit > wo )              && 
             (0 == (ctx->features & d2_feat_blur))
           )
          )
      {
         /* render outline with thin boxs (optimal performance) */
         (void)d2_renderbox_inline( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(x1 - wo), (d2_point)(y1 - wo), wo, (d2_width)(h + (2 * wo)) );  /*left */
         (void)d2_renderbox_inline( D2_DEV(handle), D2_DEV(handle)->ctxoutline, x1, (d2_point)(y1 - wo), w, wo );                    /*top */
         (void)d2_renderbox_inline( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(x1 + w), (d2_point)(y1 - wo), wo, (d2_width)(h + (2*wo)) );     /*right */
         (void)d2_renderbox_inline( D2_DEV(handle), D2_DEV(handle)->ctxoutline, x1, (d2_point)(y1 + h), w, wo );                     /*bottom */
      }
      else
      {
         /* render outline as polyline (supports linejoins) */
         verts[0] = (d2_point)((x1     - woh) - 8);    verts[1] = (d2_point)((y1     - woh) - 8);   /* (note) parentheses added for MISRA compliance.. */
         verts[2] = (d2_point)((x1 + w) + (woh - 8));  verts[3] = (d2_point)((y1     - woh) - 8);
         verts[4] = (d2_point)((x1 + w) + (woh - 8));  verts[5] = (d2_point)((y1 + h) + (woh - 8));
         verts[6] = (d2_point)((x1     - woh) - 8);    verts[7] = (d2_point)((y1 + h) + (woh - 8));

         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 4, wo, d2_le_closed, 0,0 );
      }
   }

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderbox_solidoutline( d2_device *handle, d2_point x1, d2_point y1, d2_width w, d2_width h )
{

   /* render outline part */
   (void)d2_renderbox_outline( D2_DEV(handle), x1, y1, w, h );

   /* render solid [base part] extended by one pixel */
   d2_rendertolayer_intern( handle );
   (void)d2_renderbox_inline( D2_DEV(handle), D2_DEV(handle)->ctxsolid, (d2_point)(x1 - 16), (d2_point)(y1 - 16), (d2_width)(w + 32), (d2_width)(h + 32) );
   d2_rendertobase_intern( handle );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderbox_solidshadow( d2_device *handle, d2_point x1, d2_point y1, d2_width w, d2_width h )
{
   /* render shadow */
   (void)d2_renderbox_inline( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(D2_DEV(handle)->soffx + x1), (d2_point)(D2_DEV(handle)->soffy + y1), w, h );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_renderbox_inline( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1, y1, w, h );
   d2_rendertobase_intern( handle );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_clearbox_solid( d2_device *handle, d2_color clearcolor )
{
   d2_clearbox_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, clearcolor );

   return D2_OK;
}
