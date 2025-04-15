/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_render.h (%version: 14 %)
 *          created Wed Jan 19 13:18:44 2005 by hh04027
 *
 * Description:
 *  %date_modified: Fri Jun 30 11:46:09 2006 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_render_h_H
#define __1_dave_render_h_H
/*--------------------------------------------------------------------------- */

typedef d2_s32 (*d2_fp_triangle)( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags );


/*--------------------------------------------------------------------------- */

D2_EXTERN void d2_startrender_intern( d2_devicedata *handle, const d2_bbox *bbox, d2_u32 delay );

D2_EXTERN void d2_startrender_bottom_intern( d2_devicedata *handle, const d2_bbox *bbox, d2_u32 delay );

D2_EXTERN void d2_setupmaterial_intern( d2_devicedata *handle, d2_contextdata *ctx );

/*--------------------------------------------------------------------------- */

D2_EXTERN d2_u32 d2_initgradients_intern( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox, d2_u32 limindex, d2_u32 control );


/*--------------------------------------------------------------------------- */
#endif
