/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_polyline.h (%version: 3 %)
 *          created Wed Mar 15 13:47:56 2006 by hh04027
 *
 * Description:
 *  %date_modified: Fri Jun 30 11:43:18 2006 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_polyline_h_H
#define __1_dave_polyline_h_H
/*--------------------------------------------------------------------------- */

D2_EXTERN d2_s32 d2_renderpolyline_intern( d2_devicedata *handle, d2_contextdata *ctx, const d2_point *data, const d2_u32 *sflags, d2_u32 count, d2_width w, d2_u32 flags, d2_point soffx, d2_point soffy);

D2_EXTERN d2_s32 d2_renderpolyline_solid( d2_device *handle, const d2_point *data, d2_u32 count, d2_width w, d2_u32 flags);
D2_EXTERN d2_s32 d2_renderpolyline_shadow( d2_device *handle, const d2_point *data, d2_u32 count, d2_width w, d2_u32 flags);
D2_EXTERN d2_s32 d2_renderpolyline_outline( d2_device *handle, const d2_point *data, d2_u32 count, d2_width w, d2_u32 flags);
D2_EXTERN d2_s32 d2_renderpolyline_solidshadow( d2_device *handle, const d2_point *data, d2_u32 count, d2_width w, d2_u32 flags);
D2_EXTERN d2_s32 d2_renderpolyline_solidoutline( d2_device *handle, const d2_point *data, d2_u32 count, d2_width w, d2_u32 flags);

D2_EXTERN d2_s32 d2_renderpolyline2_solid( d2_device *handle, const d2_point *data, d2_u32 count, const d2_width *w, d2_u32 flags);
D2_EXTERN d2_s32 d2_renderpolyline2_shadow( d2_device *handle, const d2_point *data, d2_u32 count, const d2_width *w, d2_u32 flags);
D2_EXTERN d2_s32 d2_renderpolyline2_outline( d2_device *handle, const d2_point *data, d2_u32 count, const d2_width *w, d2_u32 flags);
D2_EXTERN d2_s32 d2_renderpolyline2_solidshadow( d2_device *handle, const d2_point *data, d2_u32 count, const d2_width *w, d2_u32 flags);
D2_EXTERN d2_s32 d2_renderpolyline2_solidoutline( d2_device *handle, const d2_point *data, d2_u32 count, const d2_width *w, d2_u32 flags);

/*--------------------------------------------------------------------------- */
#endif
