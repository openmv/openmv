/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_gradient.h (%version: 6 %)
 *          created Mon Jan 24 13:14:23 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Jul 13 18:25:49 2006 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_gradient_h_H
#define __1_dave_gradient_h_H
/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_gradient_modes;

#define d2_grad_none        0u
#define d2_grad_linear      1u
#define d2_grad_circular    2u
#define d2_grad_threshold   4u
#define d2_grad_rightedge   8u
#define d2_grad_offset     16u
#define d2_grad_aapattern  32u
#define d2_grad_concave    64u

/*---------------------------------------------------------------------------
 * Gradient structure */

typedef struct _d2_gradientdata
{
   d2_u32   mode;
   d2_s32   xadd;
   d2_s32   yadd;
   d2_s32   xadd2;
   d2_s32   yadd2;
   d2_point x1,y1;
} d2_gradientdata;

/*--------------------------------------------------------------------------- */

D2_EXTERN void d2_initgradient_intern( d2_gradientdata *grad );

/*--------------------------------------------------------------------------- */
#endif
