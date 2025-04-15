/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_circle.h (%version: 3 %)
 *          created Mon Jan 24 15:25:42 2005 by hh04027
 *
 * Description:
 *  %date_modified: Mon Feb 14 14:37:17 2005 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_circle_h_H
#define __1_dave_circle_h_H
/*--------------------------------------------------------------------------- */

d2_s32 d2_rendercircle_intern( d2_devicedata *handle, d2_contextdata *ctx, d2_point x, d2_point y, d2_width r, d2_width w );

extern d2_s32 d2_rendercircle_solid( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w );

extern d2_s32 d2_rendercircle_shadow( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w );

extern d2_s32 d2_rendercircle_outline( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w );

extern d2_s32 d2_rendercircle_solidoutline( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w );

extern d2_s32 d2_rendercircle_solidshadow( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w );

/*--------------------------------------------------------------------------- */
#endif
