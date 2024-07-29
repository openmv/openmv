/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_quad.c (%version: 9 %)
 *          created Fri Feb 11 14:06:48 2005 by hh04027
 *
 * Description:
 *  %date_modified: Fri Dec 07 13:40:33 2007 %  (%derived_by:  hh74036 %)
 *
 * Changes:
 *  2007-12-07 MGe  several bugfixes in d2_renderquad_intern, removed quadconfig
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2010-09-09 MRe  added renderquad functions for solidoutlined etc. 
 *  2010-09-27 MRe  fixed quad render function
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_viewport.h"
#include "dave_render.h"
#include "dave_pattern.h"
#include "dave_quad.h"
#include "dave_edge.h"
#include "dave_line.h"
#include "dave_texture.h"
#include "dave_polyline.h"

/*--------------------------------------------------------------------------*/
static d2_s32 d2_renderquad_intern( d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags );

/*--------------------------------------------------------------------------
 *
 * */
static d2_s32 d2_renderquad_intern( d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags )
{
   d2_bbox bbox;
   d2_u32 bR1, bR2, bR3, bR4;
   d2_s32 flip, delay;
   d2_u32 control;
   d2_s32 ssd;
   d2_limdata edge[4];

   /* find deltas */
   d2_s32 dx1 = x2 - x1;
   d2_s32 dx2 = x3 - x2;
   d2_s32 dx3 = x4 - x3;
   d2_s32 dx4 = x1 - x4;
   d2_s32 dy1 = y2 - y1;
   d2_s32 dy2 = y3 - y2;
   d2_s32 dy3 = y4 - y3;
   d2_s32 dy4 = y1 - y4;

   /* build masks from signbits */
   d2_u32 dxm = ((dx1 < 0) ? 1u : 0u) | ((dx2 < 0) ? 2u : 0u) | ((dx3 < 0) ? 4u : 0u) | ((dx4 < 0) ? 8u : 0u);
   d2_u32 dym = ((dy1 < 0) ? 1u : 0u) | ((dy2 < 0) ? 2u : 0u) | ((dy3 < 0) ? 4u : 0u) | ((dy4 < 0) ? 8u : 0u);
   d2_u32 idxm = dxm ^ 15u;
   d2_u32 idym = dym ^ 15u;

   /* catch degenerate quad (collapsed edge) and render a triangle instead */
   if(0 != (idxm & idym))
   {
      if(0 == (dx1 | dy1))   /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $*/
      {
         return d2_rendertri( handle, x2, y2, x3, y3, x4, y4, flags >> 1 );
      }
      if(0 == (dx2 | dy2))   /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $*/
      {
         return d2_rendertri( handle, x1, y1, x3, y3, x4, y4, ((flags & 12u ) >> 1) | (flags & 1u) );
      }
      if(0 == (dx3 | dy3))   /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $*/
      {
         return d2_rendertri( handle, x1, y1, x2, y2, x4, y4, ((flags & 8u) >> 1) |  (flags & 3u) );
      }
      if(0 == (dx4 | dy4))   /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $*/
      {
         return d2_rendertri( handle, x1, y1, x2, y2, x3, y3, flags & 7u );
      }
   }

   /* classify edges (right edge flags) - basically inverted dys
    * but using dxs instead of dys for horizontal edges */
   bR1 = (dy1 == 0) ? (idxm & 1u) : (idym & 1u);
   bR2 = (dy2 == 0) ? (idxm & 2u) : (idym & 2u);
   bR3 = (dy3 == 0) ? (idxm & 4u) : (idym & 4u);
   bR4 = (dy4 == 0) ? (idxm & 8u) : (idym & 8u);


   /* get xmin, xmax and the endposition of spanstore(ssd) */
   if(x1 < x2)
   {
      bbox.xmin = x1;
      bbox.xmax = x2;
      ssd = y1;
   }
   else
   {
      bbox.xmin = x2;
      bbox.xmax = x1;
      ssd = y2;
   }

   if(x3 < x4)
   {
      if(bbox.xmin > x3)
      {
         bbox.xmin = x3;
         ssd= y3;
      }
      if(bbox.xmax < x4)
      {
         bbox.xmax = x4;
      }
   }
   else
   {
      if(bbox.xmin > x4)
      {
         bbox.xmin = x4;
         ssd = y4;
      }

      if(bbox.xmax < x3)
      {
         bbox.xmax = x3;
      }
   }

   /* get ymin , ymax */
   if(y1 < y2)
   {
      bbox.ymin = y1;
      bbox.ymax = y2;
   }
   else
   {
      bbox.ymin = y2;
      bbox.ymax = y1;
   }

   if(y3 < y4)
   {
      bbox.ymin = ( bbox.ymin < y3 ) ? bbox.ymin : y3 ;
      bbox.ymax = ( bbox.ymax > y4 ) ? bbox.ymax : y4 ;
   }
   else
   {
      bbox.ymin = ( bbox.ymin < y4 ) ? bbox.ymin : y4 ;
      bbox.ymax = ( bbox.ymax > y3 ) ? bbox.ymax : y3 ;
   }

   bbox.xmin = (d2_point) D2_FLOOR4(bbox.xmin);
   bbox.ymin = (d2_point) D2_FLOOR4(bbox.ymin);

   bbox.xmax = (d2_point) D2_CEIL4(bbox.xmax);
   bbox.ymax = (d2_point) D2_CEIL4(bbox.ymax);

   /* calculate spanstoredelay */ 
   delay = (ssd - bbox.ymin) >> 4;    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

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
   x4 = (d2_point)(x4 - bbox.xmin);
   y4 = (d2_point)(y4 - bbox.ymin);

   /* calc edge interpolation parameters */
   control = 0;

   if(0 == (ctx->features & d2_feat_aa))
   {
      flags |= d2_all_shared;
   }

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

   if(0 != (flags & d2_edge3_shared))
   {
      d2_triedge_setup_intern( &edge[3], x4, y4, dx4, dy4, bR4 );
      control |= D2C_LIM4THRESHOLD;
      edge[3].start += D2_FIX16(1) / 2;
   }
   else
   {
      if(0 != (ctx->features & d2_feat_blur))
      {
         d2_triedge_setupblur_intern( &edge[3], x4, y4, dx4, dy4, ctx );
      }
      else 
      {
         d2_triedge_setup_intern( &edge[3], x4, y4, dx4, dy4, bR4 );
         edge[3].start += D2_FIX16(1);
      }
   }

   /* optimize for span abort / span store */
   if ( ( bbox.ymax - ssd ) < ( ssd - bbox.ymin ) )
   {
      flip = 1;
      delay = (bbox.ymax - ssd  ) >> 4;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   }
   else
   {
      flip = 0;
   }


   control |= D2C_LIM1ENABLE | D2C_LIM2ENABLE | D2C_LIM3ENABLE | D2C_LIM4ENABLE | D2C_SPANABORT | D2C_SPANSTORE; 

   /* check for alpha gradients */
   if(0 != (ctx->alphamode & (d2_am_gradient1 | d2_am_gradient2)))
   {
      control = d2_initgradients_intern( handle, ctx, &bbox, 4, control );

      if(0 == (control & D2C_SPANSTORE))
      {
         flip = 0;
      }
   }

   /* bottom-up rendering / left edges are x-decreasing */
   if(0 != flip)
   {
      d2_s32 h = D2_INT4( bbox.ymax - bbox.ymin );   /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
      d2_invertlimiter_intern(&edge[0], h);
      d2_invertlimiter_intern(&edge[1], h);
      d2_invertlimiter_intern(&edge[2], h);
      d2_invertlimiter_intern(&edge[3], h);
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
   D2_DLISTWRITES( D2_L4START, edge[3].start );
   D2_DLISTWRITES( D2_L4XADD , edge[3].xadd );
   D2_DLISTWRITES( D2_L4YADD , edge[3].yadd );
   D2_DLISTWRITEU( D2_CONTROL,  control );

   /* start rendering */
   if(0 != flip)
   {
      d2_startrender_bottom_intern( handle, &bbox, (d2_u32)delay + 1 );
   }
   else
   {
      d2_startrender_intern( handle, &bbox, (d2_u32)delay + 1 );
   }

   return 1;
}


/*--------------------------------------------------------------------------
 * shared edges do not contribute to vertex tangents. to implement this
 * a set of line/open polyline/closed polyline configurations is used
 * depending on the shared edge configuration
 * */
d2_s32 d2_renderquad_outline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags)
{
   d2_width wo = (d2_width)(D2_DEV(handle)->outlinewidth * 2);
   d2_contextdata *ctx = D2_DEV(handle)->ctxoutline;
   d2_point verts[4*2];

   /* isolate sharing flags */
   flags = flags & (d2_edge0_shared | d2_edge1_shared | d2_edge2_shared | d2_edge3_shared);

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
         verts[6] = x4;  verts[7] = y4;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 4, wo, d2_le_closed, 0, 0 );
         break;

         /* single shared edge - use an open polyline */
      case d2_edge0_shared:
         verts[0] = x1;  verts[1] = y1;
         verts[2] = x4;  verts[3] = y4;
         verts[4] = x3;  verts[5] = y3;
         verts[6] = x2;  verts[7] = y2;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 4, wo, d2_le_exclude_both, 0, 0 );
         break;

      case d2_edge1_shared:
         verts[0] = x2;  verts[1] = y2;
         verts[2] = x1;  verts[3] = y1;
         verts[4] = x4;  verts[5] = y4;
         verts[6] = x3;  verts[7] = y3;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 4, wo, d2_le_exclude_both, 0, 0 );
         break;

      case d2_edge2_shared:
         verts[0] = x4;  verts[1] = y4;
         verts[2] = x1;  verts[3] = y1;
         verts[4] = x2;  verts[5] = y2;
         verts[6] = x3;  verts[7] = y3;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 4, wo, d2_le_exclude_both, 0,0 );
         break;

      case d2_edge3_shared:
         verts[0] = x1;  verts[1] = y1;
         verts[2] = x2;  verts[3] = y2;
         verts[4] = x3;  verts[5] = y3;
         verts[6] = x4;  verts[7] = y4;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 4, wo, d2_le_exclude_both, 0, 0 );
         break;

         /* two adjacent shared edges - use two lines */
      case d2_edge0_shared | d2_edge1_shared:
         verts[0] = x1;  verts[1] = y1;
         verts[2] = x4;  verts[3] = y4;
         verts[4] = x3;  verts[5] = y3;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 3, wo, d2_le_exclude_both, 0,0 );
         break;

      case d2_edge1_shared | d2_edge2_shared:
         verts[0] = x4;  verts[1] = y4;
         verts[2] = x1;  verts[3] = y1;
         verts[4] = x2;  verts[5] = y2;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 3, wo, d2_le_exclude_both, 0, 0 );
         break;

      case d2_edge2_shared | d2_edge3_shared:
         verts[0] = x1;  verts[1] = y1;
         verts[2] = x2;  verts[3] = y2;
         verts[4] = x3;  verts[5] = y3;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 3, wo, d2_le_exclude_both, 0, 0 );
         break;

      case d2_edge3_shared | d2_edge0_shared:
         verts[0] = x2;  verts[1] = y2;
         verts[2] = x3;  verts[3] = y3;
         verts[4] = x4;  verts[5] = y4;
         (void)d2_renderpolyline_intern( D2_DEV(handle), ctx, verts, NULL, 3, wo, d2_le_exclude_both, 0, 0);
         break; 

         /* two opposite shared edges - use a single lines */
      case d2_edge0_shared | d2_edge2_shared:
         (void)d2_renderline_outline_I( handle, x1, y1, x4, y4, wo, d2_le_exclude_both );
         (void)d2_renderline_outline_I( handle, x3, y3, x2, y2, wo, d2_le_exclude_both );
         break;

      case d2_edge1_shared | d2_edge3_shared:
         (void)d2_renderline_outline_I( handle, x2, y2, x1, y1, wo, d2_le_exclude_both );
         (void)d2_renderline_outline_I( handle, x4, y4, x3, y3, wo, d2_le_exclude_both );
         break;

         /* three shared edges - use a single lines */
      case d2_edge0_shared | d2_edge1_shared | d2_edge2_shared:
         (void)d2_renderline_outline_I( handle, x1, y1, x4, y4, wo, d2_le_exclude_both );
         break;

      case d2_edge0_shared | d2_edge1_shared | d2_edge3_shared:
         (void)d2_renderline_outline_I( handle, x4, y4, x3, y3, wo, d2_le_exclude_both );
         break;

      case d2_edge0_shared | d2_edge2_shared | d2_edge3_shared:
         (void)d2_renderline_outline_I( handle, x3, y3, x2, y2, wo, d2_le_exclude_both );
         break;

      case d2_edge1_shared | d2_edge2_shared | d2_edge3_shared:
         (void)d2_renderline_outline_I( handle, x1, y1, x2, y2, wo, d2_le_exclude_both );
         break;

         /* more than three shared edges - no outline at all */

      default:
         break;
   }

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderquad_solid( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags )
{
   (void)d2_renderquad_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1,y1, x2,y2, x3,y3, x4,y4, flags );
      
   /* HACK: replaced with rendering two triangles */
/*
  d2_rendertri_solid( handle, x1,y1, x2,y2, x3,y3, flags | d2_edge2_shared );
  d2_rendertri_solid( handle, x3,y3, x4,y4, x1,y1, (flags >> 2) | d2_edge2_shared );
*/
 
   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderquad_solidoutline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags )
{

   /* render outline */
   (void)d2_renderquad_outline( handle, x1,y1, x2,y2, x3,y3, x4,y4, flags );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_renderquad_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1,y1, x2,y2, x3,y3, x4,y4, flags );

   d2_rendertobase_intern( handle );
   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderquad_shadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags )
{
   d2_point sx,sy;

   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   /* render shadow */
   (void)d2_renderquad_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(x1+sx),(d2_point)(y1+sy), (d2_point)(x2+sx),(d2_point)(y2+sy), (d2_point)(x3+sx), (d2_point)(y3+sy), (d2_point)(x4+sx), (d2_point)(y4+sy), flags );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderquad_solidshadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags )
{
   d2_point sx,sy;

   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   /* render shadow */
   (void)d2_renderquad_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, (d2_point)(x1+sx), (d2_point)(y1+sy), (d2_point)(x2+sx), (d2_point)(y2+sy), (d2_point)(x3+sx), (d2_point)(y3+sy), (d2_point)(x4+sx), (d2_point)(y4+sy), flags );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_renderquad_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x1, y1, x2, y2, x3, y3, x4, y4, flags );

   d2_rendertobase_intern( handle );

   return D2_OK;
}
