/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_triangle.c (%version: 18 %)
 *          created Mon Jan 24 16:13:27 2005 by hh04027
 *
 * Description:
 *  %date_modified: Mon Feb 19 16:12:50 2007 %  (%derived_by:  hh74036 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_viewport.h"
#include "dave_render.h"
#include "dave_pattern.h"
#include "dave_triangle.h"
#include "dave_edge.h"
#include "dave_line.h"
#include "dave_texture.h"
#include "dave_polyline.h"


/*--------------------------------------------------------------------------
 *
 * */
static d2_s32 d2_rendertri_intern( d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags ); /* MISRA */


/*--------------------------------------------------------------------------
 *
 * */
static d2_s32 d2_rendertri_intern( d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags )
{
   d2_bbox bbox;
   d2_u32 bR1, bR2, bR3;
   d2_u32 dxb1, dxb2;
   d2_u32 control, delay;
   d2_limdata edge[3];

   /* find deltas */
   d2_s32 dx1 = x2 - x1;
   d2_s32 dx2 = x3 - x2;
   d2_s32 dx3 = x1 - x3;
   d2_s32 dy1 = y2 - y1;
   d2_s32 dy2 = y3 - y2;
   d2_s32 dy3 = y1 - y3;

   /* build masks from signbits */
   d2_u32 dxm = ((dx1 < 0) ? 1u : 0) | ((dx2 < 0) ? 2u : 0) | ((dx3 < 0) ? 4u : 0);
   d2_u32 dym = ((dy1 < 0) ? 1u : 0) | ((dy2 < 0) ? 2u : 0) | ((dy3 < 0) ? 4u : 0);
   d2_u32 idxm = dxm ^ 7u;
   d2_u32 idym = dym ^ 7u;

   /* catch degenerate triangle (zero area) */
   if(0 != (idxm & idym))
   {
      if(
         (0 == ( (dx1 | dy1) & ~7 )) ||    /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $*/
         (0 == ( (dx2 | dy2) & ~7 )) ||    /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $*/
         (0 == ( (dx3 | dy3) & ~7 ))       /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $*/
         )
      {
         return 0;
      }
   }

   /* classify edges (right edge flags) - basically inverted dys
    * but using dxs instead of dys for horizontal edges */
   bR1 = (0 == dy1) ? (idxm & 1u) : (idym & 1u);
   bR2 = (0 == dy2) ? (idxm & 2u) : (idym & 2u);
   bR3 = (0 == dy3) ? (idxm & 4u) : (idym & 4u);

   /* find x minimum */
   switch(dxm & (idxm >> 1))
   {
      case 0: bbox.xmin = (d2_point) D2_FLOOR4( x1 ); break;
      case 1: bbox.xmin = (d2_point) D2_FLOOR4( x2 ); break;
      case 2: bbox.xmin = (d2_point) D2_FLOOR4( x3 ); break;
      default:
         break;
   }

   /* find x maximum */
   switch(idxm & (dxm >> 1))
   {
      case 0: bbox.xmax = (d2_point) D2_CEIL4( x1 ); break;
      case 1: bbox.xmax = (d2_point) D2_CEIL4( x2 ); break;
      case 2: bbox.xmax = (d2_point) D2_CEIL4( x3 ); break;
      default:
         break;
   }

   /* find y minimum */
   switch (dym & (idym >> 1))
   {
      case 0: bbox.ymin = (d2_point) D2_FLOOR4( y1 ); break;
      case 1: bbox.ymin = (d2_point) D2_FLOOR4( y2 ); break;
      case 2: bbox.ymin = (d2_point) D2_FLOOR4( y3 ); break;
      default:
         break;
   }

   /* find y maximum */
   switch(idym & (dym >> 1))
   {
      case 0: bbox.ymax = (d2_point) D2_CEIL4( y1 ); break;
      case 1: bbox.ymax = (d2_point) D2_CEIL4( y2 ); break;
      case 2: bbox.ymax = (d2_point) D2_CEIL4( y3 ); break;
      default:
         break;
   }

   /* clipping */
   if(0 == d2_clipbbox_intern( handle, &bbox ))
   {
      return 0;
   }

   /* set register values (material parameters) */
   d2_setupmaterial_intern( handle, ctx );

   /* make relative */
   x1 = (d2_point)(x1 - bbox.xmin);
   y1 = (d2_point)(y1 - bbox.ymin);
   x2 = (d2_point)(x2 - bbox.xmin);
   y2 = (d2_point)(y2 - bbox.ymin);
   x3 = (d2_point)(x3 - bbox.xmin);
   y3 = (d2_point)(y3 - bbox.ymin);

   /* calc edge interpolation parameters */
   control = 0u;

   if(0 != (flags & d2_edge0_shared))
   {
      d2_triedge_setup_intern( &edge[0], x1, y1, dx1, dy1, bR1 );
      control |= D2C_LIM1THRESHOLD;
      edge[0].start += D2_FIX16(1) / 2;
   }
   else
   {
      if(0 != (ctx->features & d2_feat_blur))
      {
         d2_triedge_setupblur_intern( &edge[0], x1, y1, dx1, dy1, ctx );
      }
      else
      {
         d2_triedge_setup_intern( &edge[0], x1, y1, dx1, dy1, bR1 );
         edge[0].start += D2_FIX16(1);
      }
   }

   if(0 != (flags & d2_edge1_shared))
   {
      d2_triedge_setup_intern( &edge[1], x2, y2, dx2, dy2, bR2 );
      control |= D2C_LIM2THRESHOLD;
      edge[1].start += D2_FIX16(1) / 2;
   }
   else
   {
      if(0 != (ctx->features & d2_feat_blur))
      {
         d2_triedge_setupblur_intern( &edge[1], x2, y2, dx2, dy2, ctx );
      }
      else
      {
         d2_triedge_setup_intern( &edge[1], x2, y2, dx2, dy2, bR2 );
         edge[1].start += D2_FIX16(1);
      }
   }

   if(0 != (flags & d2_edge2_shared))
   {
      d2_triedge_setup_intern( &edge[2], x3, y3, dx3, dy3, bR3 );
      control |= D2C_LIM3THRESHOLD;
      edge[2].start += D2_FIX16(1) / 2;
   }
   else
   {
      if(0 != (ctx->features & d2_feat_blur))
      {
         d2_triedge_setupblur_intern( &edge[2], x3, y3, dx3, dy3, ctx );
      }
      else
      {
         d2_triedge_setup_intern( &edge[2], x3, y3, dx3, dy3, bR3 );
         edge[2].start += D2_FIX16(1);
      }
   }

   /* handle nonantialiased */
   if(0 == (ctx->features & d2_feat_aa))
   {
      control |= D2C_LIM1THRESHOLD | D2C_LIM2THRESHOLD | D2C_LIM3THRESHOLD;
   }

   /* optimize for span abort / span store */
   dxb1 = (dxm & dym);
   delay = 0u;

   if(0 != (bR1 ^ (bR2 >> 1) ^ (bR3 >> 2)))
   {
      /* two edges left */
      dxb2 = (idym | dxm );

      if( (0u != dxb1) && (7u != dxb2))
      {
         d2_s32 pymid, pymax;

         pymax = bbox.ymax - bbox.ymin;

         /* left edges have different directions
          * find mid y scanline (ymid - ymin) */
         switch( (dym & (idym >> 1)) ^ (idym & (dym >> 1)) )
         {
            case 1: pymid = y3; break;
            case 2: pymid = y2; break;
            case 3: pymid = y1; break;
            default:
               pymid = y1; break; /* MISRA */
         }

         /* find larger half (note: could improve heuristics to consider whole area) */
         if(pymid > (pymax / 2))
         {
            dxb1 = 0; /* bottom-up */
            delay = (d2_u32)D2_INT4(pymax - pymid) + 1u;    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         }
         else
         {
            delay = (d2_u32)D2_INT4(pymid) + 1u;            /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         }
      }
   }

   /* setup rasterization flags */
   control |= D2C_LIM1ENABLE | D2C_LIM2ENABLE | D2C_LIM3ENABLE | D2C_SPANABORT;

   /* disable spanstore for thin geometry (not supported in hw anyway) */
   if( (bbox.xmax - bbox.xmin) >= D2_FIX4(4) )              /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   {
      control |= D2C_SPANSTORE;
   }

   /* check for alpha gradients */
   if(0 != (ctx->alphamode & (d2_am_gradient1 | d2_am_gradient2)))
   {
      control = d2_initgradients_intern( handle, ctx, &bbox, 3, control );

      if(0 == (control & D2C_SPANSTORE))
      {
         dxb1 = 1u;
      }
   }

   /* bottom-up rendering / left edges are x-decreasing */
   if(0 == dxb1)
   {
      d2_s32 h = D2_INT4( bbox.ymax - bbox.ymin );    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

      d2_invertlimiter_intern(&edge[0], h);
      d2_invertlimiter_intern(&edge[1], h);
      d2_invertlimiter_intern(&edge[2], h);
      d2_setuppattern( handle, ctx, &bbox, 1 );
      d2_setuptexture( handle, ctx, &bbox, 1 );
   }
   else
   {
      d2_setuppattern( handle, ctx, &bbox, 0 );
      d2_setuptexture( handle, ctx, &bbox, 0 );
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

   D2_DLISTWRITEU( D2_CONTROL, control );

   /* start rendering */
   if(0 == dxb1)
   {
      d2_startrender_bottom_intern( handle, &bbox, delay + 1u );
   }
   else
   {
      d2_startrender_intern( handle, &bbox, delay + 1u );
   }

   return 1;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_rendertri_solid( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags )
{
   (void)d2_rendertri_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1, y1, x2, y2, x3, y3, flags );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_rendertri_shadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags )
{
   d2_point sx,sy;

   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   (void)d2_rendertri_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(x1 + sx), (d2_point)(y1 + sy), (d2_point)(x2 + sx), (d2_point)(y2 + sy), (d2_point)(x3 + sx), (d2_point)(y3 + sy), flags );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_rendertri_solidshadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags )
{
   d2_point sx,sy;

   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   /* render shadow */
   (void)d2_rendertri_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(x1 + sx), (d2_point)(y1 + sy), (d2_point)(x2 + sx), (d2_point)(y2 + sy), (d2_point)(x3 + sx), (d2_point)(y3 + sy), flags );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_rendertri_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1, y1, x2, y2, x3, y3, flags );
   d2_rendertobase_intern( handle );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 * shared edges do not contribute to vertex tangents. to implement this
 * a set of line/open polyline/closed polyline configurations is used
 * depending on the shared edge configuration
 * */
d2_s32 d2_rendertri_outline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags )
{
   d2_width wo = (d2_width)(D2_DEV(handle)->outlinewidth * 2);
   d2_contextdata *ctx = D2_DEV(handle)->ctxoutline;
   d2_point verts[3*2];

   /* isolate sharing flags */
   flags &= (d2_edge0_shared | d2_edge1_shared | d2_edge2_shared);

   /* skip invisible cases */
   if(wo < D2_EPSILON)
   {
      return D2_OK;
   }

   /* special case all sharing combinations */
   switch(flags)
   {
      /* no shared edge - use a closed polyline */
      case 0:
         verts[0] = x1;  verts[1] = y1;
         verts[2] = x2;  verts[3] = y2;
         verts[4] = x3;  verts[5] = y3;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 3, wo, d2_le_closed, 0,0 );
         break;

         /* single shared edge - use an open polyline */
      case d2_edge0_shared:
         verts[0] = x1;  verts[1] = y1;
         verts[2] = x3;  verts[3] = y3;
         verts[4] = x2;  verts[5] = y2;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 3, wo, d2_le_exclude_both, 0,0 );
         break;

      case d2_edge1_shared:
         verts[0] = x2;  verts[1] = y2;
         verts[2] = x1;  verts[3] = y1;
         verts[4] = x3;  verts[5] = y3;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 3, wo, d2_le_exclude_both, 0,0 );
         break;

      case d2_edge2_shared:
         verts[0] = x1;  verts[1] = y1;
         verts[2] = x2;  verts[3] = y2;
         verts[4] = x3;  verts[5] = y3;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 3, wo, d2_le_exclude_both, 0,0 );
         break;

         /* two shared edges - use a single line */
      case d2_edge0_shared | d2_edge1_shared:
         (void)d2_renderline_outline_I( handle, x1, y1, x3, y3, wo, d2_le_exclude_both );
         break;

      case d2_edge0_shared | d2_edge2_shared:
         (void)d2_renderline_outline_I( handle, x2, y2, x3, y3, wo, d2_le_exclude_both );
         break;

      case d2_edge1_shared | d2_edge2_shared:
         (void)d2_renderline_outline_I( handle, x1, y1, x2, y2, wo, d2_le_exclude_both );
         break;

         /* more than two shared edges - no outline at all */

      default:
         break;
   }

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_rendertri_solidoutline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags )
{
   /* render outline */
   (void)d2_rendertri_outline( D2_DEV(handle), x1, y1, x2, y2, x3, y3, flags );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_rendertri_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1, y1, x2, y2, x3, y3, flags );

   d2_rendertobase_intern( handle );

   return D2_OK;
}
