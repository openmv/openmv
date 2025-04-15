/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_line.c (%version: 40 %)
 *          created Mon Jan 31 16:49:01 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Mar 08 14:52:36 2007 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2007-03-08 NSc  fixed incorrect blurring of lines
 *  2008-01-02 ASc  fixed drawing lines with width=0
 *  2008-01-14 ASc  fixed bug in d2_renderline_intern_split
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2011-01-20 SSt  made lines and polylines thread safe (eliminated globals)
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_viewport.h"
#include "dave_render.h"
#include "dave_pattern.h"
#include "dave_line.h"
#include "dave_circle.h"
#include "dave_edge.h"
#include "dave_curve.h"
#include "dave_texture.h"

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline_intern( d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags, d2_limdata *edge_buffer, d2_bbox *edge_bbox, const d2_s32 *connectors)
{
   d2_bbox bbox, bbox2;
   d2_limdata edge[4];
   d2_width wh, er;
   d2_s32 w16;
   d2_u32 control;
   d2_u32 tmask;
   d2_s32 flip, h, swt, miter, len;
   d2_u32 cap;

   /* find deltas */
   d2_s32 dx = x2 - x1;
   d2_s32 dy = y2 - y1;

   if(0 == (dx | dy))           /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $ */
   {
      return 0;
   }

   cap = ctx->linecap;

   /* find bounding box */
   wh = (d2_width)((w + 15) >> 1);          /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   if(d2_lc_square == cap)
   {
      /* increase bbox for square ends */
      if( (flags & d2_le_exclude_both) != d2_le_exclude_both )
      {
         /* scale halfwidth by sqrt(2) */
         wh = (d2_width)((wh * 23) >> 4);   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      }
   }

   if(0 != (flags & d2_lei_miter_edge))
   {
      if(d2_lj_bevel == ctx->linejoin)
      {
         /* scale halfwidth by 1.25 */
         wh = (d2_width)((wh * 20) >> 4);   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         miter = (w >> 1) << (16 - 4); /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      }
      else
      {
         miter = (ctx->miterlimit + (w >> 1)) << (16 - 4); /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         wh = (d2_width)( ((ctx->miterlimit + (w >> 1)) * 20) >> 4 ); /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      }
   }
   else
   {
      miter = 0;
   }

   if(dx > 0)
   {
      bbox.xmin = (d2_point) D2_FLOOR4( x1 - wh );
      bbox.xmax = (d2_point) D2_CEIL4( x2 + wh );
   }
   else
   {
      bbox.xmin = (d2_point) D2_FLOOR4( x2 - wh );
      bbox.xmax = (d2_point) D2_CEIL4( x1 + wh );
   }

   if(dy > 0)
   {
      bbox.ymin = (d2_point) D2_FLOOR4( y1 - wh );
      bbox.ymax = (d2_point) D2_CEIL4( y2 + wh );
   }
   else
   {
      bbox.ymin = (d2_point) D2_FLOOR4( y2 - wh );
      bbox.ymax = (d2_point) D2_CEIL4( y1 + wh );
   }

   /* clipping */
   if(0 == d2_clipbbox_intern( handle, &bbox ))
   {
      return 0;
   }

   /* make relative (dont need relative endpoint) */
   x1 = (d2_point)(x1 - bbox.xmin);
   y1 = (d2_point)(y1 - bbox.ymin);

   /* don't use round endpoints on very small lines */
   if(w < D2_FIX4(3))
   {
      if(d2_lc_round == cap)
      {
         cap = d2_lc_square;
      }
   }

   /* don't consider endpoint style if both are omitted */
   if( (flags & d2_le_exclude_both) == d2_le_exclude_both )
   {
      cap = d2_lc_butt;
   }

   /* calc edge interpolation parameters */
   if( (ctx->features & (d2_feat_blur | d2_feat_aa)) == (d2_feat_blur | d2_feat_aa) )
   {
      /* using special setup for blurred lines */
      d2_lineedge_setup3blur_intern( edge, x1, y1, dx, dy, flags, ctx, &len );
      w16 = (w * (d2_s32)ctx->invblur) >> 4;         /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      edge[0].start += w16 >> 1;             /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

      /* apply linecap */
      if(d2_lc_square == cap)
      {
         if(0 == (flags & d2_le_exclude_start))
         {
            edge[1].start += w16 >> 1;       /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         }

         if(0 == (flags & d2_le_exclude_end))
         {
            edge[2].start += w16 >> 1;       /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         }
      }

      w16 += (d2_s32)(ctx->invblur >> 1) - 65536;
   }
   else
   {
      /* using sqrt setup for wide lines (note: reduce threshold to 1?) */
      if(w < D2_FIX4(2))
      {
         d2_lineedge_setup3_intern( edge, x1, y1, dx, dy, flags, &len );
      }
      else
      {
         d2_lineedge_setup3sqrt_intern( edge, x1, y1, dx, dy, flags, &len );
      }

      w16 = w << (16 - 4);                  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      edge[0].start += (w16 >> 1);          /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

      /* apply linecap */
      if(d2_lc_square == cap)
      {
         if(0 == (flags & d2_le_exclude_start))
         {
            edge[1].start += (w16 - D2_FIX16(1)) >> 1;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         }

         if(0 == (flags & d2_le_exclude_end))
         {
            edge[2].start += (w16 - D2_FIX16(1)) >> 1;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         }
      }
   }

   /* set register values (material parameters) */
   d2_setupmaterial_intern( handle, ctx );
   tmask = ctx->thresholdmask;

   /* determine rendering direction */
   if( ((dy > 0) && (dx < 0)) || ((dy < 0) && (dx > 0)) )
   {
      flip = 1;
   }
   else
   {
      flip = 0;
   }

   control = 
      D2C_LIM1ENABLE  | 
      D2C_LIM2ENABLE  | 
      D2C_LIM5ENABLE  |
      D2C_BAND1ENABLE | 
      (tmask & (D2C_LIM1THRESHOLD | D2C_LIM2THRESHOLD | D2C_LIM5THRESHOLD)) |
      D2C_SPANABORT   | 
      D2C_SPANSTORE   ;

   /* check for alpha gradients */
   if(0 != (ctx->alphamode & (d2_am_gradient1 | d2_am_gradient2)))
   {
      d2_u32 gidx;

      if(d2_lc_round != cap)
      {
         gidx = 2;   /* put gradients on limiters 3&4 for nonround lines */
      }
      else
      {
         gidx = 5;   /* put gradients on limiter 6 for rounded lines */
      }

      if(0 != (flags & d2_lei_miter_edge))
      {
         gidx = 5;   /* keep limiters 3&4 free for miter limiters */
      }

      control = d2_initgradients_intern( handle, ctx, &bbox, gidx, control );

      if(0 == (control & D2C_SPANSTORE))
      {
         flip = 0;
      }
   }

   /* pattern autoalign */
   if(0 != (ctx->patmode & d2_pm_autoalign))
   {
      d2_gradientdata *grad;

      grad = & ctx->patulim[0];
      ctx->internaldirty |= d2_dirty_upatlim;

      grad->mode = d2_grad_linear | d2_grad_aapattern;
      grad->x1 = x1;
      grad->y1 = y1;

      if(0 != (ctx->patmode & d2_pm_orthogonal))
      {
         grad->xadd = (edge[1].yadd << 4) / ctx->patscale;   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         grad->yadd = (-edge[1].xadd << 4) / ctx->patscale;  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      }
      else
      {
         grad->xadd = (edge[1].xadd << 4) / ctx->patscale;   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         grad->yadd = (edge[1].yadd << 4) / ctx->patscale;   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      }
      grad->xadd2 = ctx->patoffset << (16 - 4);              /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

      /* pattern auto advance */
      if(0 != (ctx->patmode & d2_pm_advance))
      {
         ctx->patoffset += (len << 4) / ctx->patscale;       /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      }
   }

   /* output texture and pattern setup */
   d2_setuppattern( handle, ctx, &bbox, flip );
   d2_setuptexture( handle, ctx, &bbox, flip );

   /* include first round end */
   if(d2_lc_round == cap)
   {
      d2_point x,y;

      /* endpoint radius */
      er = (d2_width)((w + D2_FIX4(1)) >> 1);           /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

      if(0 != (flags & d2_le_exclude_start))
      {
         /* endpoint (no second part) */
         if(0 != flip)
         {
            x = (d2_point)(x2 - bbox.xmin);
            y = (d2_point)(bbox.ymax - y2);
         }
         else
         {
            x = (d2_point)(x2 - bbox.xmin);
            y = (d2_point)(y2 - bbox.ymin);
         }

         /* no need to consider round ends anymore */
         cap = d2_lc_butt;
         swt = 1;
      }
      else
      {
         /* startpoint */
         if(0 != flip)
         {
            x = x1;
            y = (d2_point)((bbox.ymax - bbox.ymin) - y1);
         }
         else
         {
            x = x1;
            y = y1;
         }

         if(0 != (flags & d2_le_exclude_end))
         {
            /* no second part */
            cap = d2_lc_butt;
         }
         else
         {
            /* shared edge at endpoint (endpoint rendered as second part) */
            control |= D2C_LIM5THRESHOLD;
         }
         swt = 0;
      }

      /* check endpoint clipping to avoid 'far off' endpoints */
      if( ((x + (er*2)) > 0) &&
          ((y + (er*2)) > 0) &&
          ((x - (er*2)) < (bbox.xmax - bbox.xmin)) &&
          ((y - (er*2)) < (bbox.ymax - bbox.ymin))
          )
      {
         /* round endpoint limiter setup */
         d2_circlesetup_intern( handle, ctx, 2, x, y, er, 0, 0, 0);

         /* include l3/l4 */
         control |= D2C_LIM3ENABLE | D2C_QUAD2ENABLE | D2C_UNIONAB | (tmask & D2C_LIM3THRESHOLD);
         /* update cache for circular rendering */
      }
   }
   else
   {
      swt = 0;
      /* just to shutup compiler */
      er = 0;
   }

   /* disable antialiasing for excluded endpoints */
   if(0 != (flags & d2_le_exclude_start))
   {
      if(0 == swt)
      {
         control |= D2C_LIM2THRESHOLD;
      }
      else
      {
         control |= D2C_LIM5THRESHOLD;
      }

      if(0 != (flags & d2_lei_ext_first_edge))
      {
         /* use external edge */
         d2_triedge_setupnoaa_intern(&edge[1], x1, y1, connectors[0], connectors[1], 0 );
         wh = (d2_width)(wh*2); /* expand span delay (experimental) */   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      }
      else
      {
         /* keep original edge */
         edge[1].start += D2_FIX16(1) >> 1;   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         /* rightedge flags */
         if(dy > 0)
         {
            edge[1].start += D2_EPSILON;
         }
      }
   }

   if(0 != (flags & d2_le_exclude_end))
   {
      if(0 != swt)
      {
         control |= D2C_LIM2THRESHOLD;
      }
      else
      {
         control |= D2C_LIM5THRESHOLD;
      }

      if(0 != (flags & d2_lei_ext_last_edge))
      {
         /* use external edge */
         d2_triedge_setupnoaa_intern(&edge[2], (d2_point)(x2 - bbox.xmin), (d2_point)(y2 - bbox.ymin), -connectors[2], -connectors[3], 1 );
         wh = (d2_width)(wh*2); /* expand span delay (experimental) */   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      }
      else
      {
         /* keep original edge */
         edge[2].start += D2_FIX16(1) >> 1;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         /* rightedge flags */
         if(dy <= 0)
         {
            edge[2].start += D2_EPSILON;
         }
      }
   }

   /* store edges if requested */
   if(0 != (flags & (d2_lei_buffer_first_edge | d2_lei_buffer_last_edge)))
   {
      if(0 != (flags & d2_lei_buffer_first_edge))
      {
         edge_buffer[0] = edge[1];
      }

      if(0 != (flags & d2_lei_buffer_last_edge))
      {
         edge_buffer[1] = edge[2];
      }

      *edge_bbox = bbox;
   }

   /* optimize for span abort / span store */
   if(0 != flip)
   {
      h = D2_INT4( bbox.ymax - bbox.ymin );   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      d2_invertlimiter_intern(&edge[0], h);
      d2_invertlimiter_intern(&edge[1], h);
      d2_invertlimiter_intern(&edge[2], h);
   }
   else
   {
      /* just to shutup compiler */
      h = 0;
   }

   /* add miter edges if requested */
   if(0 != (flags & d2_lei_miter_edge))
   {
      /* miter outward shift to avoid line cutting into itself at 180� */
      miter += D2_FIX16(1);

      if( (0 != (flags & d2_lei_ext_first_edge)) && (0 != (connectors[0] | connectors[1])) ) /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $ */
      {
         /* add first edge miter limit (L3) */
         control |= D2C_LIM3ENABLE | (tmask & D2C_LIM3THRESHOLD);

         /* could be extracted from edge[1], but edge1 is not normalized */
         d2_triedge_setupsqrt_intern(&edge[3], x1, y1, -connectors[1], connectors[0], 0 );

         if(0 != flip)
         {
            d2_invertlimiter_intern(&edge[3], h);
         }

         if(0 == (flags & d2_lei_miter1_flip))
         {
            D2_DLISTWRITES( D2_L3START, edge[3].start + miter );
            D2_DLISTWRITES( D2_L3XADD , edge[3].xadd );
            D2_DLISTWRITES( D2_L3YADD , edge[3].yadd );
         }
         else
         {
            D2_DLISTWRITES( D2_L3START, -edge[3].start + miter );
            D2_DLISTWRITES( D2_L3XADD , -edge[3].xadd );
            D2_DLISTWRITES( D2_L3YADD , -edge[3].yadd );
         }
      }

      if( (0 != (flags & d2_lei_ext_last_edge)) && (0 != (connectors[2] | connectors[3])) ) /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $ */
      {
         /* add last edge miter limit (L4) */
         control |= D2C_LIM4ENABLE | (tmask & D2C_LIM4THRESHOLD);

         /* could be extracted from edge[2], but edge2 is not normalized */
         d2_triedge_setupsqrt_intern(&edge[3], (d2_point)(x2 - bbox.xmin), (d2_point)(y2 - bbox.ymin), -connectors[3], connectors[2], 0 );

         if(0 != flip)
         {
            d2_invertlimiter_intern(&edge[3], h);
         }

         if(0 != (flags & d2_lei_miter2_flip))
         {
            D2_DLISTWRITES( D2_L4START, edge[3].start + miter );
            D2_DLISTWRITES( D2_L4XADD , edge[3].xadd );
            D2_DLISTWRITES( D2_L4YADD , edge[3].yadd );
         }
         else
         {
            D2_DLISTWRITES( D2_L4START, -edge[3].start + miter );
            D2_DLISTWRITES( D2_L4XADD , -edge[3].xadd );
            D2_DLISTWRITES( D2_L4YADD , -edge[3].yadd );
         }
      }
   }

   /* set register values (geometric parameters) */
   D2_DLISTWRITES( D2_L1START, edge[0].start );
   D2_DLISTWRITES( D2_L1XADD , edge[0].xadd );
   D2_DLISTWRITES( D2_L1YADD , edge[0].yadd );

   if(0 != swt)
   {
      /* endpoint limiter must be in first group */
      D2_DLISTWRITES( D2_L5START, edge[1].start );
      D2_DLISTWRITES( D2_L5XADD , edge[1].xadd );
      D2_DLISTWRITES( D2_L5YADD , edge[1].yadd );
      D2_DLISTWRITES( D2_L2START, edge[2].start );
      D2_DLISTWRITES( D2_L2XADD , edge[2].xadd );
      D2_DLISTWRITES( D2_L2YADD , edge[2].yadd );
   }
   else
   {
      /* startpoint limiter must be in first group */
      D2_DLISTWRITES( D2_L2START, edge[1].start );
      D2_DLISTWRITES( D2_L2XADD , edge[1].xadd );
      D2_DLISTWRITES( D2_L2YADD , edge[1].yadd );
      D2_DLISTWRITES( D2_L5START, edge[2].start );
      D2_DLISTWRITES( D2_L5XADD , edge[2].xadd );
      D2_DLISTWRITES( D2_L5YADD , edge[2].yadd );
   }

   D2_DLISTWRITES( D2_L1BAND , w16 );

   D2_DLISTWRITEU( D2_CONTROL, control );

   /* span delay height is approximated by linewidth */
   w = (d2_width)(D2_INT4((wh + ((w + 15) >> 1)) + 15));   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   if(0 != flip)
   {
      d2_startrender_bottom_intern( handle, &bbox, (d2_u32)w );
   }
   else
   {
      d2_startrender_intern( handle, &bbox, (d2_u32)w );
   }

   /* include second round end */
   if(d2_lc_round == cap)
   {
      /* new bbox */
      bbox2.xmin = (d2_point) D2_FLOOR4( x2 - er );
      bbox2.ymin = (d2_point) D2_FLOOR4( y2 - er );
      bbox2.xmax = (d2_point) D2_CEIL4( x2 + er );
      bbox2.ymax = (d2_point) D2_CEIL4( y2 + er );

      /* clip endpoint bbox */
      if(0 == d2_clipbbox_intern( handle, &bbox2 ))
      {
         return 1;
      }

      /* make relative */
      x2 = (d2_point)(x2 - bbox2.xmin);
      y2 = (d2_point)(y2 - bbox2.ymin);

      /* circle limiters */
      d2_circlesetup_intern(handle, ctx, 0, x2, y2, er, 0, 0, 0);

      /* new control word */
      control = 
         D2C_LIM1ENABLE              | 
         D2C_QUAD1ENABLE             | 
         D2C_LIM3ENABLE              | 
         (tmask & D2C_LIM1THRESHOLD) | 
         D2C_LIM3THRESHOLD           | 
         D2C_SPANABORT               ;

      /* restore UV and alpha limiters to fit new bbox */
      d2_setuppattern( handle, ctx, &bbox2, 0 );
      d2_setuptexture( handle, ctx, &bbox2, 0 );

      if(0 != (ctx->alphamode & (d2_am_gradient1 | d2_am_gradient2)))
      {
         control = d2_initgradients_intern( handle, ctx, &bbox2, 3, control );
      }

      /* restore seperation limiter to fit new bbox */
      if(0 != flip)
      {
         dx = D2_INT4(bbox2.xmin - bbox.xmin);       /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         dy = D2_INT4(bbox2.ymin - bbox.ymin) - h;   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         edge[2].yadd = -edge[2].yadd;
      }
      else
      {
         dx = D2_INT4(bbox2.xmin - bbox.xmin);       /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         dy = D2_INT4(bbox2.ymin - bbox.ymin);       /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      }

      /* D2_DLISTWRITES( D2_L3START, -edge[2].start - (dx * edge[2].xadd) - (dy * edge[2].yadd) + D2_FIX16(1) + D2_EPSILON ); */
      D2_DLISTWRITES( D2_L3START, 
                      (
                         (
                            (-edge[2].start - (dx * edge[2].xadd))
                            - (dy * edge[2].yadd)
                          )
                         + D2_FIX16(1)              /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
                       )
                      + D2_EPSILON
                      );
      D2_DLISTWRITES( D2_L3XADD , -edge[2].xadd );
      D2_DLISTWRITES( D2_L3YADD , -edge[2].yadd );

      D2_DLISTWRITEU( D2_CONTROL, control );

      d2_startrender_intern( handle, &bbox2, (d2_u32) (w >> 4) );  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   }

   return 1;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline2_intern( d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags, d2_limdata *edge_buffer, d2_bbox *edge_bbox )
{
   d2_bbox bbox, bbox2;
   d2_limdata edge[4];
   d2_s32 r, l, dlx, dly, nx, ny, h;
   d2_s32 w1x, w1y, w2x, w2y, xs, ys, xe, ye, xp, yp;
   d2_s32 ax, ay, bx, by, cx, cy, dx, dy;
   d2_s32 t1, t2, flip, cap2;
   d2_u32 delay;
   d2_u32 cap;
   d2_u32 control;
   d2_u32 tmask;

   /* check for parallel lines
    *if (w1 == w2) return d2_renderline_intern( handle, ctx, x1,y1, x2,y2, w1, d2_include_both ); */

   /* find deltas */
   dlx = x2 - x1;
   dly = y2 - y1;
   if(0 == (dlx | dly))    /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $*/
   {
      return 0;
   }

   cap = ctx->linecap;

   l = (65536 * 256) / d2_sqrt( (d2_u32) ( (dlx * dlx) + (dly * dly) ) );

   nx = (-dly * l) >> 8;   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
   ny = ( dlx * l) >> 8;   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   /*
     {
     // accurate calculation
     l = d2_sqrt( dlx*dlx + dly*dly );
     nx = (int) (((d2_int64)-dly * 65536) / l);
     ny = (int) (((d2_int64) dlx * 65536) / l);
     }
   */

   /* find outer endpoints */
   w1x = ((nx * w1) >> 1) >> 16;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   w1y = ((ny * w1) >> 1) >> 16;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   w2x = ((nx * w2) >> 1) >> 16;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   w2y = ((ny * w2) >> 1) >> 16;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   ax = x1 + w1x;
   ay = y1 + w1y;
   bx = x2 + w2x;
   by = y2 + w2y;
   cx = x2 - w2x;
   cy = y2 - w2y;
   dx = x1 - w1x;
   dy = y1 - w1y;

   /* don't render round endpoints on small lines */
   if(d2_lc_round == cap)
   {
      if(w1 < D2_FIX4(3))        /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      {
         flags |= d2_le_exclude_start;
      }

      if(w2 < D2_FIX4(3))        /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      {
         flags |= d2_le_exclude_end;
      }
   }

   /* simplify caps if no endpoints */
   if(d2_le_exclude_both == (flags & d2_le_exclude_both))
   {
      cap = d2_lc_butt;
   }

   /* expand bbox to contain caps */
   if(d2_lc_square == cap)
   {
      if(0 == (flags & d2_le_exclude_start))
      {
         ax -= w1y;  ay -= -w1x;
         dx -= w1y;  dy -= -w1x;
      }

      if(0 == (flags & d2_le_exclude_end))
      {
         bx += w2y;  by += -w2x;
         cx += w2y;  cy += -w2x;
      }
   }

   /* find bounding box */
   t1 = (ax < bx) ? ax : bx;
   t2 = (cx < dx) ? cx : dx;
   bbox.xmin = (d2_point) D2_FLOOR4( (t1 < t2) ? t1 : t2 );
   t1 = (ax > bx) ? ax : bx;
   t2 = (cx > dx) ? cx : dx;
   bbox.xmax = (d2_point) D2_CEIL4( (t1 > t2) ? t1 : t2 );
   t1 = (ay < by) ? ay : by;
   t2 = (cy < dy) ? cy : dy;
   bbox.ymin = (d2_point) D2_FLOOR4( (t1 < t2) ? t1 : t2 );
   t1 = (ay > by) ? ay : by;
   t2 = (cy > dy) ? cy : dy;
   bbox.ymax = (d2_point) D2_CEIL4( (t1 > t2) ?  t1 : t2 );

   /* clipping */
   if(0 == d2_clipbbox_intern( handle, &bbox ))
   {
      return 0;
   }

   /* store original endpoints */
   xs = x1; ys = y1;
   xe = x2; ye = y2;

   /* make relative */
   x1 = (d2_point)(x1 - bbox.xmin);
   y1 = (d2_point)(y1 - bbox.ymin);
   x2 = (d2_point)(x2 - bbox.xmin);
   y2 = (d2_point)(y2 - bbox.ymin);

   /* calc edge interpolation parameters */
   edge[0].start = (( (y1 * nx) - (x1 * ny) ) >> 4) + D2_FIX16(1);   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   edge[0].xadd  = ny;
   edge[0].yadd  = -nx;
   edge[1].start = (( (x2 * ny) - (y2 * nx) ) >> 4) + (D2_FIX16(1) >> 1);  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   edge[1].xadd  = -ny;
   edge[1].yadd  = nx;

   /* side parameters */
   if((ctx->features & (d2_feat_blur | d2_feat_aa)) == (d2_feat_blur | d2_feat_aa))
   {
      d2_triedge_setupblur_intern( &edge[2], (d2_point)(ax - bbox.xmin), (d2_point)(ay - bbox.ymin), (ax - bx), (ay - by), ctx );
      d2_triedge_setupblur_intern( &edge[3], (d2_point)(dx - bbox.xmin), (d2_point)(dy - bbox.ymin), (cx - dx), (cy - dy), ctx );
   }
   else
   {
      d2_triedge_setup_intern( &edge[2], (d2_point)(ax - bbox.xmin), (d2_point)(ay - bbox.ymin), (ax - bx), (ay - by), 0 );
      d2_triedge_setup_intern( &edge[3], (d2_point)(dx - bbox.xmin), (d2_point)(dy - bbox.ymin), (cx - dx), (cy - dy), 0 );
      edge[2].start += (D2_FIX16(1) >> 1);  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      edge[3].start += (D2_FIX16(1) >> 1);  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   }

   /* apply linecap */
   if(d2_lc_square == cap)
   {
      if(0 == (flags & d2_le_exclude_start))
      {
         edge[0].start += ((w1 << (16 - 4)) - D2_FIX16(1)) >> 1;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      }

      if(0 == (flags & d2_le_exclude_end))
      {
         edge[1].start += ((w2 << (16 - 4)) - D2_FIX16(1)) >> 1;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      }
   }

   /* set register values (material parameters) */
   d2_setupmaterial_intern( handle, ctx );

   /* determine sharing flags */
   tmask = ctx->thresholdmask;

   if(d2_lc_round == cap)
   {
      if(0 == (flags & d2_le_exclude_start))
      {
         tmask |= D2C_LIM1THRESHOLD;
      }

      if(0 == (flags & d2_le_exclude_end))
      {
         tmask |= D2C_LIM2THRESHOLD;
      }
   }

   /* pattern autoalign */
   if(0 != (ctx->patmode & d2_pm_autoalign))
   {
      d2_gradientdata *grad;

      grad = & ctx->patulim[0];
      ctx->internaldirty |= d2_dirty_upatlim;

      grad->mode = d2_grad_linear | d2_grad_aapattern;
      grad->x1 = x1;
      grad->y1 = y1;

      if(0 != (ctx->patmode & d2_pm_orthogonal))
      {
         grad->xadd = (edge[0].yadd << 4) / ctx->patscale;   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         grad->yadd = (-edge[0].xadd << 4) / ctx->patscale;  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      }
      else
      {
         grad->xadd = (edge[0].xadd << 4) / ctx->patscale;   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         grad->yadd = (edge[0].yadd << 4) / ctx->patscale;   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      }

      grad->xadd2 = ctx->patoffset << (16 - 4);              /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

      /* pattern auto advance */
      if(0 != (ctx->patmode & d2_pm_advance))
      {
         ctx->patoffset += (l << 4) / ctx->patscale;         /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      }
   }

   /* determine rendering direction */
   if((dy - cy) > 0)
   {
      dlx = dx - cx;
      dly = dy - cy;
   }
   else
   {
      dlx = ax - bx;
      dly = ay - by;
   }

   if( ((dly > 0) && (dlx < 0)) || ((dly < 0) && (dlx > 0)) )
   {
      flip = 1;

      d2_setuppattern( handle, ctx, &bbox, 1 );
      d2_setuptexture( handle, ctx, &bbox, 1 );

      /* find offset from endpoint to bbox border */
      if(ys > ye)
      {
         delay = (d2_u32) (bbox.ymax - ys);
      }
      else
      {
         delay = (d2_u32) (bbox.ymax - ye);
      }
   }
   else
   {
      flip = 0;

      d2_setuppattern( handle, ctx, &bbox, 0 );
      d2_setuptexture( handle, ctx, &bbox, 0 );

      /* find offset from endpoint to bbox border */
      if(ys > ye)
      {
         delay = (d2_u32) (ye - bbox.ymin);
      }
      else
      {
         delay = (d2_u32) (ys - bbox.ymin);
      }
   }

   delay += (d2_u32) ( ((w1 > w2) ? w1 : w2) >> 4 );        /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   control = 
      D2C_LIM1ENABLE | 
      D2C_LIM2ENABLE | 
      D2C_LIM3ENABLE | 
      D2C_LIM4ENABLE |
      (tmask & (D2C_LIM1THRESHOLD|D2C_LIM2THRESHOLD|D2C_LIM3THRESHOLD|D2C_LIM4THRESHOLD)) |
      D2C_SPANABORT  | 
      D2C_SPANSTORE  ;

   /* check for alpha gradients */
   if(0 != (ctx->alphamode & (d2_am_gradient1 | d2_am_gradient2)))
   {
      control = d2_initgradients_intern( handle, ctx, &bbox, 4, control );

      if(0 == (control & D2C_SPANSTORE))
      {
         flip = 0;
      }
   }

   /* find most important endpoint */
   if(0 != (flags & d2_le_exclude_start))
   {
      /* render only endpoint */
      r = w2 >> 1;                             /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      xp = xe;
      yp = ye;
      l = 1;
      cap2 = 0;
   }
   else if (0 != (flags & d2_le_exclude_end))
   {
      /* render only startpoint */
      r = w1 >> 1;                             /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      xp = xs;
      yp = ys;
      l = 0;
      cap2 = 0;
   }
   else
   {
      /* need to render both points */
      cap2 = 1;
      if(w1 > w2)
      {
         /* startpoint  first */
         r = w1 >> 1;                          /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         xp = xs;
         yp = ys;
         l = 0;
      }
      else
      {
         /* endpoint first */
         r = w2 >> 1;                          /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         xp = xe;
         yp = ye;
         l = 1;
      }
   }

   /* store outer edge if needed by cap later */
   if(0 != cap2)
   {
      edge_buffer[0] = edge[(d2_u32)l ^ 1u];
      *edge_bbox = bbox;
   }

   /* optimize for span abort / span store */
   if(0 != flip)
   {
      h = D2_INT4( bbox.ymax - bbox.ymin );   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      d2_invertlimiter_intern(&edge[0], h);
      d2_invertlimiter_intern(&edge[1], h);
      d2_invertlimiter_intern(&edge[2], h);
      d2_invertlimiter_intern(&edge[3], h);
   }
   else
   {
      h = 0;
   }

   D2_DLISTWRITEU( D2_CONTROL, control );

   /* late blur adjustments */
   if( ((ctx->features & (d2_feat_blur | d2_feat_aa)) == (d2_feat_blur | d2_feat_aa)) && (cap != d2_lc_round) )
   {
      if(0 == (tmask & D2C_LIM1THRESHOLD))
      {
         edge[0].start = ((edge[0].start*16) / ctx->blurring);
         edge[0].xadd  = ((edge[0].xadd*16)  / ctx->blurring);
         edge[0].yadd  = ((edge[0].yadd*16)  / ctx->blurring);
      }
      if(0 == (tmask & D2C_LIM2THRESHOLD))
      {
         edge[1].start = ((edge[1].start*16) / ctx->blurring);
         edge[1].xadd  = ((edge[1].xadd*16)  / ctx->blurring);
         edge[1].yadd  = ((edge[1].yadd*16)  / ctx->blurring);
      }
   }

   /* set register values (geometric parameters) */
   D2_DLISTWRITES( D2_L1START, edge[0].start );
   D2_DLISTWRITES( D2_L1XADD , edge[0].xadd );
   D2_DLISTWRITES( D2_L1YADD , edge[0].yadd );
   D2_DLISTWRITES( D2_L2START, edge[1].start );
   D2_DLISTWRITES( D2_L2XADD , edge[1].xadd );
   D2_DLISTWRITES( D2_L2YADD , edge[1].yadd );
   D2_DLISTWRITES( D2_L3START, edge[2].start );
   D2_DLISTWRITES( D2_L3XADD , edge[2].xadd );
   D2_DLISTWRITES( D2_L3YADD , edge[2].yadd );
   D2_DLISTWRITES( D2_L4START, edge[3].start );
   D2_DLISTWRITES( D2_L4XADD , edge[3].xadd );
   D2_DLISTWRITES( D2_L4YADD , edge[3].yadd );

   /* render line part */
   if(0 != flip)
   {
      d2_startrender_bottom_intern( handle, &bbox, delay );
   }
   else
   {
      d2_startrender_intern( handle, &bbox, delay );
   }

   if(d2_lc_round != cap)
   {
      return 1;
   }

   /* find reduced bbox for endpoint */
   bbox2.xmin = (d2_point) D2_FLOOR4( xp - r );
   bbox2.ymin = (d2_point) D2_FLOOR4( yp - r );
   bbox2.xmax = (d2_point) D2_CEIL4( xp + r );
   bbox2.ymax = (d2_point) D2_CEIL4( yp + r );

   /* endpoint clipping */
   if(0 != d2_clipbbox_intern( handle, &bbox2 ))
   {
      /* make relative */
      xp = xp - bbox2.xmin;
      yp = yp - bbox2.ymin;

      /* calc circle endpoint parameters */
      d2_circlesetup_intern(handle, ctx, 0, (d2_point) xp, (d2_point) yp, (d2_width) r, 0, 0, 0);

      delay = D2_CEIL4( r ) >> 4;                   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

      /* adapt line parameters to new bounding box */
      if(0 != flip)
      {
         nx = D2_INT4(bbox2.xmin - bbox.xmin);      /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         ny = D2_INT4(bbox2.ymin - bbox.ymin) - h;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         edge[l].yadd = -edge[l].yadd;
         edge[2].yadd = -edge[2].yadd;
         edge[3].yadd = -edge[3].yadd;
      }
      else
      {
         nx = D2_INT4(bbox2.xmin - bbox.xmin);      /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         ny = D2_INT4(bbox2.ymin - bbox.ymin);      /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      }

      d2_setuppattern( handle, ctx, &bbox2, 0 );
      d2_setuptexture( handle, ctx, &bbox2, 0 );

      D2_DLISTWRITES( D2_L3START, edge[2].start + (edge[2].xadd * nx) + (edge[2].yadd * ny) );
      D2_DLISTWRITES( D2_L3XADD , edge[2].xadd );
      D2_DLISTWRITES( D2_L3YADD , edge[2].yadd );
      D2_DLISTWRITES( D2_L4START, edge[3].start + (edge[3].xadd * nx) + (edge[3].yadd * ny) );
      D2_DLISTWRITES( D2_L4XADD , edge[3].xadd );
      D2_DLISTWRITES( D2_L4YADD , edge[3].yadd );
      D2_DLISTWRITES( D2_L5START, 
                      (
                         (
                            (
                               (
                                  D2_FIX16(1) - edge[l].start
                                )
                               - (edge[l].xadd * nx)
                             )
                            - (edge[l].yadd * ny)
                          )
                         + D2_EPSILON 
                       )
                      );
      D2_DLISTWRITES( D2_L5XADD , -edge[l].xadd );
      D2_DLISTWRITES( D2_L5YADD , -edge[l].yadd );

      control = 
         D2C_LIM1ENABLE   | 
         D2C_QUAD1ENABLE  | 
         D2C_SPANABORT    | 
         (ctx->thresholdmask & (D2C_LIM1THRESHOLD | D2C_LIM3THRESHOLD | D2C_LIM4THRESHOLD)) |
         D2C_LIM3ENABLE   | 
         D2C_LIM4ENABLE   | 
         D2C_LIM5ENABLE   | 
         D2C_LIM5THRESHOLD;

      control = d2_initgradients_intern( handle, ctx, &bbox2, 5, control );

      D2_DLISTWRITEU( D2_CONTROL, control );

      /* start first endpoint rendering */
      d2_startrender_intern( handle, &bbox2, delay );
   }

   if(0 != cap2)
   {
      /* second point required too */
      if(1 == l)
      {
         /* startpoint second */
         r = w1;
         xp = xs;
         yp = ys;
      }
      else
      {
         /* endpoint second */
         r = w2;
         xp = xe;
         yp = ye;
      }

      (void)d2_renderlinedot_intern( handle, ctx, (d2_point) xp, (d2_point) yp, (d2_width) r, 1, edge_buffer, edge_bbox );
   }

   return 1;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderlinedot_intern( d2_devicedata *handle, const d2_contextdata *ctx, d2_point x, d2_point y, d2_width w, d2_u32 edges, const d2_limdata *edge_buffer, const d2_bbox *edge_bbox )
{
   d2_bbox bbox;
   d2_u32 delay, grad;
   d2_u32 control;
   d2_u32 tmask;
   d2_width er;

   /* endpoint radius */
   er = (d2_width)((w + D2_FIX4(1)) >> 1);           /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   /* circle bounding box */
   bbox.xmin = (d2_point) D2_FLOOR4( x - er );
   bbox.ymin = (d2_point) D2_FLOOR4( y - er );
   bbox.xmax = (d2_point) D2_CEIL4( x + er );
   bbox.ymax = (d2_point) D2_CEIL4( y + er );

   /* clipping */
   if(0 == d2_clipbbox_intern( handle, &bbox ))
   {
      return 0;
   }

   /* make relative */
   x = (d2_point)(x - bbox.xmin);
   y = (d2_point)(y - bbox.ymin);

   /* ASSUME material is still set from line rendering */
   tmask = ctx->thresholdmask;

   /* base circle setup */
   d2_circlesetup_intern(handle, ctx, 0, x, y, er, 0, 0, 0);

   control = 
      D2C_LIM1ENABLE             | 
      D2C_QUAD1ENABLE            | 
      D2C_SPANABORT              | 
      (D2C_SPANSTORE*0)          | 
      (tmask & D2C_LIM1THRESHOLD);

   control = d2_initgradients_intern( handle, ctx, &bbox, 2, control );

   delay = (d2_u32) (D2_CEIL4( er ) >> 4);         /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   grad = 2;

   /* additional edges */
   if(0 != edges)
   {
      d2_s32 xr, yr;

      /* find relative change of position */
      xr = D2_INT4( edge_bbox->xmin - bbox.xmin );  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      yr = D2_INT4( edge_bbox->ymin - bbox.ymin );  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

      if(0 != (edges & 1))
      {
         D2_DLISTWRITES( D2_L3START, 
                         (
                            (
                               (
                                  (
                                     D2_FIX16(1) - edge_buffer[0].start
                                   )
                                  + (xr * edge_buffer[0].xadd)
                                )
                               + (yr * edge_buffer[0].yadd)
                             )
                            + D2_EPSILON
                          )
                         );
         D2_DLISTWRITES( D2_L3XADD , -edge_buffer[0].xadd );
         D2_DLISTWRITES( D2_L3YADD , -edge_buffer[0].yadd );

         control |= D2C_LIM3ENABLE | D2C_LIM3THRESHOLD;

         grad++;
      }

      if(0 != (edges & 2))
      {
         D2_DLISTWRITES( D2_L4START, 
                         (
                            (
                               (
                                  (
                                     D2_FIX16(1) - edge_buffer[1].start
                                   )
                                  + (xr * edge_buffer[1].xadd)
                                )
                               + (yr * edge_buffer[1].yadd)
                             )
                            + D2_EPSILON
                          )
                         );
         D2_DLISTWRITES( D2_L4XADD , -edge_buffer[1].xadd );
         D2_DLISTWRITES( D2_L4YADD , -edge_buffer[1].yadd );

         control |= D2C_LIM4ENABLE | D2C_LIM4THRESHOLD;

         if(3 == grad)
         {
            grad++;
         }
      }
   }

   /* add gradients */
   control = d2_initgradients_intern( handle, ctx, &bbox, grad, control );

   /* start rendering */
   D2_DLISTWRITEU( D2_CONTROL, control );

   d2_setuppattern( handle, ctx, &bbox, 0 );
   d2_setuptexture( handle, ctx, &bbox, 0 );

   d2_startrender_intern( handle, &bbox, delay );

   return 1;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline_intern_split( d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags, d2_limdata *edge_buffer, d2_bbox *edge_bbox, const d2_s32 *connectors )
{
   if(0 != (ctx->features & d2_feat_blur))
   {
      d2_limdata tmp_edge_buffer[2];
      d2_bbox    tmp_edge_bbox;

      (void)d2_renderline2_intern(handle, ctx, x1, y1, x2, y2, w, w, flags, tmp_edge_buffer, &tmp_edge_bbox);
   }
   else
   {
      if ( (w >= D2_FIX4(3)) && (d2_lc_round == ctx->linecap) && ((flags & d2_le_exclude_both) != d2_le_exclude_both) )
      {
         /* FIX to avoid overflowing hw register in quad mode we split geometry. sad thing! */
         if(0 != d2_renderline_intern( handle, ctx, x1,y1, x2,y2, w, flags | d2_le_exclude_both | d2_lei_buffer_first_edge | d2_lei_buffer_last_edge, edge_buffer, edge_bbox, connectors) )
         {
            if(0 == (flags & d2_le_exclude_start))
            {
               (void)d2_renderlinedot_intern( handle, ctx, x1, y1, w, 1, edge_buffer, edge_bbox );
            }

            if(0 == (flags & d2_le_exclude_end))
            {
               (void)d2_renderlinedot_intern( handle, ctx, x2, y2, w, 2, edge_buffer, edge_bbox );
            }
         }
      } 
      else
      {
         (void)d2_renderline_intern( handle, ctx, x1,y1, x2,y2, w, flags, edge_buffer, edge_bbox, connectors );
      }
   }
   return 1;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline_solid( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags )
{
   d2_limdata edge_buffer[2];
   d2_bbox    edge_bbox;

   (void)d2_renderline_intern_split( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1, y1, x2, y2, w, flags, edge_buffer, &edge_bbox, NULL );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline_outline_I( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags )
{
   d2_limdata edge_buffer[2];
   d2_bbox    edge_bbox;

   (void)d2_renderline_intern_split( D2_DEV(handle), D2_DEV(handle)->ctxoutline, x1, y1, x2, y2, w, flags, edge_buffer, &edge_bbox, NULL );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline_outline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags )
{
   d2_limdata edge_buffer[2];
   d2_bbox    edge_bbox;

   (void)d2_renderline_intern_split( D2_DEV(handle), D2_DEV(handle)->ctxoutline, x1, y1, x2, y2, (d2_width)(w + (D2_DEV(handle)->outlinewidth * 2)), flags, edge_buffer, &edge_bbox, NULL );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline_shadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags )
{
   d2_point   sx,sy;
   d2_limdata edge_buffer[2];
   d2_bbox    edge_bbox;

   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   (void)d2_renderline_intern_split( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(x1 + sx), (d2_point)(y1 + sy), (d2_point)(x2 + sx), (d2_point)(y2 + sy), w, flags, edge_buffer, &edge_bbox, NULL );

   return D2_OK;
}


/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline_solidshadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags )
{
   d2_point   sx,sy;
   d2_limdata edge_buffer[2];
   d2_bbox    edge_bbox;

   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   /* render shadow */
   (void)d2_renderline_intern_split( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(x1 + sx), (d2_point)(y1 + sy), (d2_point)(x2 + sx), (d2_point)(y2 + sy), w, flags, edge_buffer, &edge_bbox, NULL );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_renderline_intern_split( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1,y1, x2,y2, w, flags, edge_buffer, &edge_bbox, NULL );
   d2_rendertobase_intern( handle );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline_solidoutline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags )
{
   d2_limdata edge_buffer[2];
   d2_bbox    edge_bbox;

   /* render outline */
   (void)d2_renderline_intern_split( D2_DEV(handle), D2_DEV(handle)->ctxoutline, x1, y1, x2, y2, (d2_width)(w + (D2_DEV(handle)->outlinewidth * 2)), flags, edge_buffer, &edge_bbox, NULL );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_renderline_intern_split( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1, y1, x2, y2, w, flags, edge_buffer, &edge_bbox, NULL);
   d2_rendertobase_intern( handle );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline2_solid( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags )
{
   d2_limdata edge_buffer[2];
   d2_bbox    edge_bbox;

   (void)d2_renderline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1, y1, x2, y2, w1, w2, flags, edge_buffer, &edge_bbox );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline2_outline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags )
{
   d2_width   ow;
   d2_limdata edge_buffer[2];
   d2_bbox    edge_bbox;

   ow = (d2_width)(D2_DEV(handle)->outlinewidth * 2);
   (void)d2_renderline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, x1, y1, x2, y2, (d2_width)(w1 + ow), (d2_width)(w2 + ow), flags, edge_buffer, &edge_bbox );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline2_shadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags )
{
   d2_point   sx,sy;
   d2_limdata edge_buffer[2];
   d2_bbox    edge_bbox;

   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   (void)d2_renderline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(x1 + sx), (d2_point)(y1 + sy), (d2_point)(x2 + sx), (d2_point)(y2 + sy), w1, w2, flags, edge_buffer, &edge_bbox );

   return D2_OK;
}


/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline2_solidshadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags )
{
   d2_point   sx,sy;
   d2_limdata edge_buffer[2];
   d2_bbox    edge_bbox;

   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   /* render shadow */
   (void)d2_renderline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(x1 + sx), (d2_point)(y1 + sy), (d2_point)(x2 + sx), (d2_point)(y2 + sy), w1, w2, flags, edge_buffer, &edge_bbox );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_renderline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1, y1, x2, y2, w1, w2, flags, edge_buffer, &edge_bbox );
   d2_rendertobase_intern( handle );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderline2_solidoutline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags )
{
   d2_width   ow;
   d2_limdata edge_buffer[2];
   d2_bbox    edge_bbox;

   ow = (d2_width)(D2_DEV(handle)->outlinewidth * 2);

   /* render outline */
   (void)d2_renderline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, x1, y1, x2, y2, (d2_width)(w1 + ow), (d2_width)(w2 + ow), flags, edge_buffer, &edge_bbox );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_renderline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1, y1, x2, y2, w1, w2, flags, edge_buffer, &edge_bbox );
   d2_rendertobase_intern( handle );

   return D2_OK;
}
