/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_box.h (%version: 3 %)
 *          created Tue Feb 08 17:11:45 2005 by hh04027
 *
 * Description:
 *  %date_modified: Mon Jan 16 17:10:30 2006 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_box_h_H
#define __1_dave_box_h_H
/*--------------------------------------------------------------------------- */

D2_EXTERN d2_s32 d2_renderbox_intern(  d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_width w, d2_width h );

D2_EXTERN d2_s32 d2_renderbox_inline( d2_devicedata *handle, d2_contextdata *ctx, d2_point x1, d2_point y1, d2_width w, d2_width h );

D2_EXTERN d2_s32 d2_renderbox_solid( d2_device *handle, d2_point x1, d2_point y1, d2_width w, d2_width h );

D2_EXTERN d2_s32 d2_renderbox_shadow( d2_device *handle, d2_point x1, d2_point y1, d2_width w, d2_width h );

D2_EXTERN d2_s32 d2_renderbox_outline( d2_device *handle, d2_point x1, d2_point y1, d2_width w, d2_width h );

D2_EXTERN d2_s32 d2_renderbox_solidoutline( d2_device *handle, d2_point x1, d2_point y1, d2_width w, d2_width h );

D2_EXTERN d2_s32 d2_renderbox_solidshadow( d2_device *handle, d2_point x1, d2_point y1, d2_width w, d2_width h );

D2_EXTERN d2_s32 d2_clearbox_solid( d2_device *handle, d2_color clearcolor );

/*--------------------------------------------------------------------------- */
#endif
