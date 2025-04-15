/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_wedge.c (%version: 1 %)
 *          created Mon Feb 14 14:46:39 2005 by hh04027
 *
 * Description:
 *  %date_modified: Mon Feb 14 14:46:40 2005 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2009-03-31 MRe  improved bbox for convex wedges (less enumeration; fix artefact for very small angles)
 *  2010-09-22 MRe  don't render convex wedges with angle of 0
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_viewport.h"
#include "dave_render.h"
#include "dave_circle.h"
#include "dave_wedge.h"

/*--------------------------------------------------------------------------
 *
 * */
static d2_u32 count_leading_zeroes16(d2_u16 v); /* MISRA */
static d2_s32 is_same_direction(d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2); /* MISRA */
static d2_s32 d2_renderwedge_intern( d2_devicedata *handle, d2_contextdata *ctx, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags ); /* MISRA */


/*--------------------------------------------------------------------------
 *
 * */
static d2_u32 count_leading_zeroes16(d2_u16 v)
{
   d2_u32 c = 0;

   if(0 == v)
   {
      c = 16;
   }
   else
   {
      if(0 == (v & 0xff00))
      {
         c = 8;
      }
      else
      {
         v >>= 8;
      }
        
      if(0 == (v & 0xf0u))
      {
         c += 4u;
      }
      else
      {
         v >>= 4;
      }

      if(0 == (v & 0xcu))
      {
         c += 2u;
      }
      else
      {
         v >>= 2;
      }

      c += (0 != (v & 0x2u)) ? 0u : 1u;
   }

   return c;
}

/*--------------------------------------------------------------------------
 *
 * */
static d2_s32 is_same_direction(d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2)
{
   /* arguments are 16.16 fixedpoint */
   d2_u32 leading_zeroes;
   d2_s32 scalarprod;

   /* check 180 deg */
   if(
      ((nx1 >= 0) && (nx2 >= 0)) || ((nx1 < 0) && (nx2 < 0)) || 
      ((ny1 >= 0) && (ny2 >= 0)) || ((ny1 < 0) && (ny2 < 0))
      )
   {
      return 0;
   }

   /* reduce both vector components to less or equal 1 */
   leading_zeroes = count_leading_zeroes16((nx1 > 0) ? (d2_u16)(nx1 >> 16) : (d2_u16)((-nx1) >> 16));  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ /* PRQA S 3759 */ /* $Misra: #MISRA_BUG_IMPLICIT_CONVERSION_S32_TO_U16 $*/
   nx1 >>= 16u - leading_zeroes;    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   ny1 >>= 16u - leading_zeroes;    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   leading_zeroes = count_leading_zeroes16((ny1 > 0) ? (d2_u16)(ny1 >> 16) : (d2_u16)((-ny1) >> 16));  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ /* PRQA S 3759 */ /* $Misra: #MISRA_BUG_IMPLICIT_CONVERSION_S32_TO_U16 $*/
   nx1 >>= 16u - leading_zeroes;    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   ny1 >>= 16u - leading_zeroes;    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   leading_zeroes = count_leading_zeroes16((nx2 > 0) ? (d2_u16)(nx2 >> 16) : (d2_u16)((-nx2) >> 16));  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ /* PRQA S 3759 */ /* $Misra: #MISRA_BUG_IMPLICIT_CONVERSION_S32_TO_U16 $*/
   nx2 >>= 16u - leading_zeroes;    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   ny2 >>= 16u - leading_zeroes;    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   leading_zeroes = count_leading_zeroes16((ny2 > 0) ? (d2_u16)(ny2 >> 16) : (d2_u16)((-ny2) >> 16));  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ /* PRQA S 3759 */ /* $Misra: #MISRA_BUG_IMPLICIT_CONVERSION_S32_TO_U16 $*/
   nx2 >>= 16u - leading_zeroes;    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   ny2 >>= 16u - leading_zeroes;    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   /* if scalar product of v1 * (v2+90�) is zero then both vectors have same direction */

   nx1 >>= 1; ny1 >>= 1; nx2 >>= 1; ny2 >>= 1;      /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   scalarprod = (nx1 * ny2) + (ny1 * -nx2);
   scalarprod = (scalarprod < 0) ? -scalarprod : scalarprod;

   if(scalarprod < D2_FIX16(1)) /* some epsilon */
   {
      return 1;
   }
   else
   {
      return 0;
   }

}

/*--------------------------------------------------------------------------
 *
 * */
static d2_s32 d2_renderwedge_intern( d2_devicedata *handle, d2_contextdata *ctx, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags )
{
   d2_contextdata *oldctx;
   d2_border xminori, yminori, xmaxori, ymaxori;
   d2_border bxmin, bymin, bxmax, bymax;
   d2_u32 flags1 = 0;
   d2_u32 flags2 = 0;

   if(0 == (flags & d2_wf_concave))
   {
      if(0 != is_same_direction(nx1, ny1, nx2, ny2))
      {
         return 1;
      }
   }

   /*DEBUG*/
   /* dummy code */
   oldctx = handle->ctxselected;
   handle->ctxselected = ctx;
   
   (void)d2_getcliprect(handle, &xminori, &yminori, &xmaxori, &ymaxori);

   if(ny1 >= 0)
   {
      if(nx1 < 0)
         flags1 |= d2_grad_rightedge;
   }
   else
   {
      if(nx1 <= 0)
         flags1 |= d2_grad_rightedge;
   }

   if(ny2 >= 0)
   {
      if(nx2 < 0)
         flags2 |= d2_grad_rightedge;
   }
   else
   {
      if(nx2 <= 0)
         flags2 |= d2_grad_rightedge;
   }

   if(0 != (flags & d2_wf_concave))
   {
      /* For the concave case, we need a one pixel offset if the gradients'
       * gap angle <= 90 degrees. Otherwise, both clipgradients will only
       * partially cover some of the pixels in the wedge, which will lead to
       * a ripped wedge.
       */

      /* calculate dot product but discard one bit precision for the sake of simplicity */
      d2_s32 dot = (((nx1>>1)*(nx2>>1))>>15) + (((ny1>>1)*(ny2>>1))>>15);

      if((dot <= 0) && (0 == (flags & d2_edge0_shared)))
      {
         flags1 |= d2_grad_offset;
      }

      if((dot <= 0) && (0 == (flags & d2_edge1_shared)))
      {
         flags2 |= d2_grad_offset;
      }

      flags1 |= d2_grad_concave;
      flags2 |= d2_grad_concave;
   }

   (void)d2_setclipgradient( handle, 0, x, y, nx1, ny1, 
                             ((0 != (flags & d2_edge0_shared)) ? d2_grad_threshold : 0) |
                             flags1                                                     );

   (void)d2_setclipgradient( handle, 1, x, y, nx2, ny2,
                             ((0 != (flags & d2_edge1_shared)) ? d2_grad_threshold : 0) |
                             flags2                                                     );

   /* remove quadrants from clipbox if possible: */
   if(0 == (flags & d2_wf_concave))
   { 
      (void)d2_getcliprect(handle, &bxmin, &bymin, &bxmax, &bymax);

      if( ((nx1 + nx2) > 0) && 
          (
             ((ny2 >= 0) && (ny1 <= 0)) || 
             ((ny1 >= 0) && (ny2 <= 0))
           )
          ) /* right half */
      {
         d2_border val = (d2_border)(D2_INT4(x - (D2_FIX4(1) >> 1))); /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

         if(bxmin < val)
         {
            bxmin = val;
         }
      }

      if( ((nx1 + nx2) < 0 ) &&
          (
             ((ny2 >= 0) && (ny1 <= 0)) ||
             ((ny1 >= 0) && (ny2 <= 0)) 
           )
          ) /* left half */
      {
         d2_border val = (d2_border)(D2_INT4(x + (D2_FIX4(1) >> 1))); /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

         if(bxmax > val)
         {
            bxmax = val;
         }
      }

      if( ((ny1 + ny2) < 0) &&
          (
             (
                ((nx2 >= 0) && (nx1 <= 0)) ||
                ((nx1 >= 0) && (nx2 <= 0))
              )
           )
          ) /* upper half */
      {
         d2_border val = (d2_border)(D2_INT4(y + (D2_FIX4(1) >> 1))); /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

         if(bymax > val)
         {
            bymax = val;
         }
      }

      if( ((ny1 + ny2) > 0) &&
          (
             ((nx2 >= 0) && (nx1 <= 0)) ||
             ((nx1 >= 0) && (nx2 <= 0))
           )
          ) /* lower half */
      {
         d2_border val = (d2_border)(D2_INT4(y - (D2_FIX4(1) >> 1))); /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

         if(bymin < val)
         {
            bymin = val;
         }
      }
          
      (void)d2_cliprect(handle, bxmin, bymin, bxmax, bymax);
   }

   (void)d2_rendercircle_intern( handle, ctx, x, y, r, w );

   /* restore */
   (void)d2_setclipgradient( handle, 0, x, y, 0, 0, 0 );
   (void)d2_setclipgradient( handle, 1, x, y, 0, 0, 0 );
   (void)d2_cliprect(handle, xminori, yminori, xmaxori, ymaxori);
   handle->ctxselected = oldctx;

   return 1;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderwedge_solid( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags )
{
   (void)d2_renderwedge_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x, y, r, w, nx1,ny1, nx2,ny2, flags );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderwedge_outline( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags )
{
   (void)d2_renderwedge_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, x, y, r, (d2_width)(w + (D2_DEV(handle)->outlinewidth * 2)) , nx1, ny1, nx2, ny2, flags );

/*   d2_renderline_intern_split( D2_DEV(handle), D2_DEV(handle)->ctxoutline, x1,y1, x2,y2, w + D2_DEV(handle)->outlinewidth * 2, flags, edge_buffer, &edge_bbox, NULL ); */

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderwedge_shadow( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags )
{
   d2_point sx, sy;
   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   (void)d2_renderwedge_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(x + sx), (d2_point)(y + sy), r, w, nx1, ny1, nx2, ny2, flags );

   return D2_OK;
}


/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderwedge_solidoutline( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags )
{
   /* render outline */
   (void)d2_renderwedge_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, x, y, r, (d2_width)(w + (D2_DEV(handle)->outlinewidth * 2)), nx1, ny1, nx2, ny2, flags );

   /* render solid */
   d2_rendertolayer_intern( handle );
   (void)d2_renderwedge_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x, y, r, w, nx1, ny1, nx2, ny2, flags );
   d2_rendertobase_intern( handle );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderwedge_solidshadow( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags )
{
   d2_point sx,sy;
   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   /* render shadow */
   (void)d2_renderwedge_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(x + sx), (d2_point)(y + sy), r, w, nx1, ny1, nx2, ny2, flags );

   /* render solid */
   d2_rendertolayer_intern( handle );
   (void)d2_renderwedge_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x, y, r, w, nx1, ny1, nx2, ny2, flags );
   d2_rendertobase_intern( handle );

   return D2_OK;
}
