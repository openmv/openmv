/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_curve.h (%version: 12 %)
 *          created Thu Feb 17 15:09:41 2005 by hh04027
 *
 * Description:
 *  %date_modified: Fri Dec 02 15:16:48 2005 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2008-10-02 MRe  fix of blurred circles
 *  2008-11-24 AJ   modify code to support IAR compiler. (no 64bit support)
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_curve_h_H
#define __1_dave_curve_h_H
/*--------------------------------------------------------------------------- */


/*--------------------------------------------------------------------------
 * r must be positive, for negative circles pass 'invert = 1'
 * */
D2_EXTERN void d2_circlesetup_intern(d2_devicedata *handle, const d2_contextdata *ctx, d2_u32 index, d2_point x, d2_point y, d2_width r, d2_s32 band, d2_s32 invert, d2_s32 hiprec );

/*--------------------------------------------------------------------------- */
#endif
