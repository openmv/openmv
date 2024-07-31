/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_polyline.c (%version: 12 %)
 *          created Wed Mar 15 13:48:36 2006 by hh04027
 *
 * Description:
 *  %date_modified: Thu Mar 08 13:22:00 2007 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2007-03-14 NSc  removed Compiler Warning (uninitialized variables)
 *  2008-01-09 ASc  fixed bugs to draw correct line joins
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2011-01-20 SSt  made lines and polylines thread safe (eliminated globals)
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_render.h"
#include "dave_line.h"
#include "dave_polyline.h"


/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE void d2_fixlength(d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_s32 *dx, d2_s32 *dy); /* MISRA */

static d2_s32 d2_renderpolyline2_intern( d2_devicedata *handle, d2_contextdata *ctx, const d2_point *data, const d2_u32 *sflags, d2_u32 count, const d2_width *w, d2_u32 flags, d2_point soffx, d2_point soffy, d2_width olwidth ); /* MISRA */


/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE void d2_fixlength(d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_s32 *dx, d2_s32 *dy)
{
   d2_s32 dxi, dyi, l;

   dxi = x2 - x1;
   dyi = y2 - y1;

   l = d2_sqrt( (d2_u32) ((dxi * dxi) + (dyi * dyi)) );

   /* normalize to length 16 (for join tangent calculation) */
   if(0 != l)
   {
      *dx = (dxi * D2_FIX4(16)) / l;
      *dy = (dyi * D2_FIX4(16)) / l;
   }
   else
   {
      /* degenerate case. length 0 */
      *dx = dxi;
      *dy = dyi;
   }
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderpolyline_intern( d2_devicedata *handle, d2_contextdata *ctx, const d2_point *data, const d2_u32 *sflags, d2_u32 count, d2_width w, d2_u32 flags, d2_point soffx, d2_point soffy )
{
   d2_u32 i;
   d2_point px1, py1;
   d2_point px2, py2;
   const d2_point *pstart = data;
   d2_u32 startflag;
   d2_s32 dxm1=0, dym1=0;   /* normalized deltas at index-1 */
   d2_s32 dxp1, dyp1;    /* normalized deltas at index+1 */
   d2_s32 joins, dots;
   d2_s32 connectors[4]; /* (note) connectors might be used uninitialized, depending on flags */

   connectors[0] = connectors[1] = connectors[2] = connectors[3] = 0;

   /* check if linejoins have to be renderd */
   joins = (w > D2_FIX4(1)) ? 1 : 0;

   if(count < 3)
   {
      joins = 0;

      if(count < 2)
      {
         return 1;
      }
   }

   /* check joinstyle */
   if(0 != joins)
   {
      switch( ctx->linejoin )
      {
         case d2_lj_none:
            joins = 0;
            dots = 0;
            break;

         case d2_lj_round:
            dots = 1;
            joins = 0;
            break;

         case d2_lj_bevel:
         case d2_lj_miter:
            dots = 0;
            break;

         default:
            dots = 0;
            break;
      }
   }
   else
   {
      dots = 0;
   }

   /* set correct loop conditions for closed / open polylines */
   if(0 != (flags & d2_le_closed))
   {
      /* startpoint is last point */
      px1 = (d2_point)(data[(count * 2) - 2] + soffx);
      py1 = (d2_point)(data[(count * 2) - 1] + soffy);

      /* no linecap at startpoint */
      startflag = 0;

      /* linejoin at startpoint, need to init deltas */
      if(0 != joins)
      {
         d2_fixlength((d2_point)(data[(count * 2) - 4] + soffx), (d2_point)(data[(count * 2) - 3] + soffy), px1, py1, &dxm1, &dym1);
      }
   }
   else
   {
      /* startpoint first point */
      px1 = data[0];
      py1 = data[1];
      data += 2;

      px1 = (d2_point)(px1 + soffx);
      py1 = (d2_point)(py1 + soffy);

      count--;

      /* linecap at startpoint (only when no exclude flag is set) */
      startflag = ~flags & d2_le_exclude_start;

      /* linecap at both ends - single segment polyline (only when no exclude flag is set) */
      if( (1 == count) && (0 != (~flags & d2_le_exclude_end)) )
      {
         startflag |= d2_le_exclude_end;
      }

      dxm1 = 0;
      dym1 = 0;
   }

   /* future delta unknown */
   dxp1 = 0;
   dyp1 = 0;

   /* iterate over polyline */
   for(i=0; i<count; i++)
   {
      d2_u32 sflag;
      d2_s32 dx, dy;
      d2_s32 bSkip;

      /* next point */
      px2 = data[0];
      py2 = data[1];
      data += 2;

      px2 = (d2_point)(px2 + soffx);
      py2 = (d2_point)(py2 + soffy);

      /* ignore repeated points */
      if( (px1 == px2) && (py1 == py2) )
      {
         bSkip = 1;
         /*continue;*/
      }
      else
      {
         bSkip = 0;
      }

      if(0 == bSkip)
      {

         /* endpoint classification (inner/outer endpoint) */
         sflag = d2_le_exclude_start | d2_le_exclude_end;
         sflag &= ~startflag;

         if(0 != joins)
         {
            /* check if length is already known */
            if(0 != (dxp1 | dyp1))                      /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $*/
            {
               dx = dxp1;
               dy = dyp1;
            }
            else
            {
               /* line length */
               d2_fixlength(px1, py1, px2, py2, &dx, &dy);
            }

            /* check for line joins */
            if(d2_le_exclude_start == (sflag & d2_le_exclude_start))
            {
               /* join at start of line */
               sflag |= d2_lei_ext_first_edge;

               /* bisecting line direction *2 (to avoid loosing bits) */
               connectors[0] =  (dym1 + dy);
               connectors[1] = -(dxm1 + dx);
            }

            if(d2_le_exclude_end == (sflag & d2_le_exclude_end))
            {
               /* join at end of line */

               if(i == (count - 1))                    /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
               {
                  /* wrap back to start */
                  d2_fixlength(px2, py2, (d2_point)(pstart[0] + soffx), (d2_point)(pstart[1] + soffy), &dxp1, &dyp1);
               }
               else
               {
                  /* find future delta */
                  d2_fixlength(px2, py2, (d2_point)(data[0] + soffx), (d2_point)(data[1] + soffx), &dxp1, &dyp1);
               }

               sflag |= d2_lei_ext_last_edge;

               /* bisecting line direction *2 (to avoid loosing bits) */
               connectors[2] =  (dy + dyp1);
               connectors[3] = -(dx + dxp1);
            }
         }
         else
         {
            /* delta not required. */
            dx = dxp1;
            dy = dyp1;
         }

         {
            d2_s32 bSkipSeg;

            if(NULL != sflags)
            {
               bSkipSeg = (*sflags == d2_sf_skip) ? 1 : 0;
               sflags++;
            }
            else
            {
               bSkipSeg = 0;
            }

            /* check segment flag for current segment */
            /*if( (0 == sflags) || (*sflags++ != d2_sf_skip) )*/
            if(0 == bSkipSeg)
            {
               d2_limdata edge_buffer[2];
               d2_bbox    edge_bbox;

               /* render segment */
               sflag |= d2_lei_buffer_first_edge | d2_lei_buffer_last_edge;

               if(0 != dots)
               {
                  (void)d2_renderline_intern_split( handle, ctx, px1, py1, px2, py2, w, sflag, edge_buffer, &edge_bbox, connectors );

                  if( (i>0) || (0 != (flags & d2_le_closed)) )
                  {
                     (void)d2_renderlinedot_intern( handle, ctx, px1, py1, w, 1, edge_buffer, &edge_bbox );
                  }
               }
               else
               {
                  if(0 != joins)
                  {
                     /* limit corners */
                     sflag |= d2_lei_miter_edge;

                     /* check for right-turn on first endpoint */
                     if( ((dx * dym1) - (dxm1 * dy) ) > 0)
                     {
                        sflag |= d2_lei_miter1_flip;
                     }

                     if( ((dx * dyp1) - (dxp1 * dy) ) > 0)
                     {
                        sflag |= d2_lei_miter2_flip;
                     }
                  }

                  (void)d2_renderline_intern_split( handle, ctx, px1, py1, px2, py2, w, sflag, edge_buffer, &edge_bbox, connectors );
               }
            }
         }

         /* next segment */
         px1 = px2;
         py1 = py2;
         dxm1 = dx;
         dym1 = dy;

         /* check for line caps */
         if( (0 == (flags & (d2_le_closed | d2_le_exclude_end))) && (i == (count - 2)))  /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
         {
            startflag = d2_le_exclude_end;
         }
         else
         {
            startflag = 0;
         }

      } /* 0 == bSkip */

   } /* for i..count */

   return 1;
}

/*--------------------------------------------------------------------------
 *
 * */
static d2_s32 d2_renderpolyline2_intern( d2_devicedata *handle, d2_contextdata *ctx, const d2_point *data, const d2_u32 *sflags, d2_u32 count, const d2_width *w, d2_u32 flags, d2_point soffx, d2_point soffy, d2_width olwidth )
{
   d2_u32 i;
   d2_point px1, py1;
   d2_point px2, py2;
   d2_width w1, w2;

   /* set correct loop conditions for closed / open polylines */
   if(0 != (flags & d2_le_closed))
   {
      /* startpoint is last point */
      px1 = data[(count * 2) - 2];
      py1 = data[(count * 2) - 1];
      w1  = w[count - 1];
   }
   else
   {
      /* startpoint first point */
      px1 = data[0];
      py1 = data[1];
      data += 2;
      w1  = *w;
      w++;
      count--;
   }

   px1 = (d2_point)(px1 + soffx);
   py1 = (d2_point)(py1 + soffy);
   w1  = (d2_width)(w1 + olwidth);

   /* iterate over polyline */
   for (i=0; i<count; i++)
   {
      d2_s32 bSkip;

      /* next point */
      px2 = data[0];
      py2 = data[1];
      data += 2;
      w2  = *w;
      w++;
      px2 = (d2_point)(px2 + soffx);
      py2 = (d2_point)(py2 + soffy);
      w2  = (d2_width)(w2 + olwidth);

      /* ignore repeated points */
      if( (px1 == px2) && (py1 == py2) )
      {
         bSkip = 1; /*continue;*/
      }
      else
      {
         bSkip = 0;
      }

      if(0 == bSkip)
      {
         d2_s32 bSkipSeg;

         /* check segment flag for current segment */
         if(NULL != sflags)
         {
            bSkipSeg = (*sflags == d2_sf_skip) ? 1 : 0;
            sflags++;
         }
         else
         {
            bSkipSeg = 0;
         }

         /*if( (0 == sflags) || (*sflags++ != d2_sf_skip) )*/
         if(0 == bSkipSeg)
         {
            d2_limdata edge_buffer[2];
            d2_bbox    edge_bbox;

            /* render segment */
            d2_u32 temp_flag = d2_le_exclude_start | d2_le_exclude_end;    /* default is not to draw start and end caps */

            if( (0 == i) && (0 == (flags & d2_le_exclude_start)) && (0 == (flags & d2_le_closed)) )
            {
               temp_flag &= ~d2_le_exclude_start;  /* draw start cap */
            }

            if( (i == (count - 1)) && (0 == (flags & d2_le_exclude_end)) && (0 == (flags & d2_le_closed)) )
            {
               temp_flag &= ~d2_le_exclude_end;    /* draw end cap */
            }

            (void)d2_renderline2_intern( handle, ctx, px1, py1, px2, py2, w1, w2, temp_flag, edge_buffer, &edge_bbox);

            if( (i > 0) || (0 != (flags & d2_le_closed)) )
            {
               (void)d2_renderlinedot_intern( handle, ctx, px1, py1, w1, 0, edge_buffer, &edge_bbox );  /* render a dot at line startpos */
            }
         }

         /* next segment */
         px1 = px2;
         py1 = py2;
         w1  = w2;
      }
   }

   return 1;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderpolyline_solid( d2_device *handle, const d2_point *data, d2_u32 count, d2_width w, d2_u32 flags )
{
   (void)d2_renderpolyline_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, data, NULL, count, w, flags, 0, 0);

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderpolyline_shadow( d2_device *handle, const d2_point *data, d2_u32 count, d2_width w, d2_u32 flags )
{
   d2_point sx, sy;

   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   (void)d2_renderpolyline_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, data, NULL, count, w, flags, sx, sy);

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderpolyline_outline( d2_device *handle, const d2_point *data, d2_u32 count, d2_width w, d2_u32 flags )
{
   (void)d2_renderpolyline_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, data, NULL, count, (d2_width)(w + (D2_DEV(handle)->outlinewidth * 2)), flags, 0, 0 );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderpolyline_solidshadow( d2_device *handle, const d2_point *data, d2_u32 count, d2_width w, d2_u32 flags )
{
   d2_point sx, sy;

   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   /* render shadow */
   (void)d2_renderpolyline_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, data, NULL, count, w, flags, sx, sy );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_renderpolyline_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, data, NULL, count, w, flags, 0, 0 );
   d2_rendertobase_intern( handle );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderpolyline_solidoutline( d2_device *handle, const d2_point *data, d2_u32 count, d2_width w, d2_u32 flags )
{
   /* render outline */
   (void)d2_renderpolyline_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, data, NULL, count, (d2_width)(w + (D2_DEV(handle)->outlinewidth * 2)), flags, 0, 0 );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_renderpolyline_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, data, NULL, count, w, flags, 0, 0 );
   d2_rendertobase_intern( handle );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderpolyline2_solid( d2_device *handle, const d2_point *data, d2_u32 count, const d2_width *w, d2_u32 flags )
{
   (void)d2_renderpolyline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, data, NULL, count, w, flags, 0, 0, 0 );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderpolyline2_shadow( d2_device *handle, const d2_point *data, d2_u32 count, const d2_width *w, d2_u32 flags )
{
   d2_point sx,sy;

   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   (void)d2_renderpolyline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, data, NULL, count, w, flags, sx, sy, 0 );

   return D2_OK;
}


/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderpolyline2_outline( d2_device *handle, const d2_point *data, d2_u32 count, const d2_width *w, d2_u32 flags )
{
   (void)d2_renderpolyline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, data, NULL, count, w, flags, 0,0, (d2_width)(D2_DEV(handle)->outlinewidth * 2) );

   return D2_OK;
}


/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderpolyline2_solidshadow( d2_device *handle, const d2_point *data, d2_u32 count, const d2_width *w, d2_u32 flags )
{
   d2_point sx, sy;

   sx = D2_DEV(handle)->soffx;
   sy = D2_DEV(handle)->soffy;

   /* render shadow */
   (void)d2_renderpolyline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, data, NULL, count, w, flags, sx, sy, 0 );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_renderpolyline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, data, NULL, count, w, flags, 0, 0, 0 );
   d2_rendertobase_intern( handle );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_renderpolyline2_solidoutline( d2_device *handle, const d2_point *data, d2_u32 count, const d2_width *w, d2_u32 flags )
{
   /* render outline */
   (void)d2_renderpolyline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, data, NULL, count, w, flags, 0, 0, (d2_width)(D2_DEV(handle)->outlinewidth * 2) );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_renderpolyline2_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, data, NULL, count, w, flags, 0, 0, 0 );
   d2_rendertobase_intern( handle );

   return D2_OK;
}
