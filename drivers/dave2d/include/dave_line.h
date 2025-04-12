/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_line.h (%version: 8 %)
 *          created Mon Jan 31 16:49:01 2005 by hh04027
 *
 * Description:
 *  %date_modified: Fri Mar 17 11:23:54 2006 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2011-01-20 SSt  made lines and polylines thread safe (eliminated globals)
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_line_h_H
#define __1_dave_line_h_H
/*--------------------------------------------------------------------------- */

D2_EXTERN d2_s32 d2_renderline_intern( d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags, d2_limdata *edge_buffer, d2_bbox *edge_bbox, const d2_s32 *connectors );

D2_EXTERN d2_s32 d2_renderline2_intern( d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags, d2_limdata *edge_buffer, d2_bbox *edge_bbox );

D2_EXTERN d2_s32 d2_renderlinedot_intern( d2_devicedata *handle, const d2_contextdata *ctx, d2_point x, d2_point y, d2_width w, d2_u32 edges, const d2_limdata *edge_buffer, const d2_bbox *edge_bbox );

D2_EXTERN d2_s32 d2_renderline_intern_split( d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags, d2_limdata *edge_buffer, d2_bbox *edge_bbox, const d2_s32 *connectors );

/*--------------------------------------------------------------------------- */

D2_EXTERN d2_s32 d2_renderline_solid( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags );

D2_EXTERN d2_s32 d2_renderline_shadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags );

D2_EXTERN d2_s32 d2_renderline_solidshadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags );

D2_EXTERN d2_s32 d2_renderline_outline_I( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags );

D2_EXTERN d2_s32 d2_renderline_outline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags );

D2_EXTERN d2_s32 d2_renderline_solidoutline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags );

/*--------------------------------------------------------------------------- */

D2_EXTERN d2_s32 d2_renderline2_solid( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags );

D2_EXTERN d2_s32 d2_renderline2_shadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags );

D2_EXTERN d2_s32 d2_renderline2_solidshadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags );

D2_EXTERN d2_s32 d2_renderline2_outline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags );

D2_EXTERN d2_s32 d2_renderline2_solidoutline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags );

/*--------------------------------------------------------------------------- */
#endif
