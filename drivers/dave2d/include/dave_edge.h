/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_edge.h (%version: 14 %)
 *          created Mon Jan 31 18:40:01 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Jan 04 13:51:46 2007 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2010-09-28 MRe  use 64bit arithmetic for blurring calculation
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_edge_h_H
#define __1_dave_edge_h_H
/*-------------------------------------------------------------------------- */

/*--------------------------------------------------------------------------
 * line length may not be zero.
 * */
D2_EXTERN void d2_lineedge_setup3blur_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, d2_u32 flags, const d2_contextdata *ctx, d2_s32 *length );


/*--------------------------------------------------------------------------
 * line length may not be zero.
 * */
D2_EXTERN void d2_lineedge_setup3sqrt_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, d2_u32 flags, d2_s32 *length );


/*--------------------------------------------------------------------------
 * note use for thin lines (w<2) only (due to l1 norm approximation)
 * line length may not be zero.
 * */
D2_EXTERN void d2_lineedge_setup3_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, d2_u32 flags, d2_s32 *length );


/*--------------------------------------------------------------------------
 * edge deltas may not both be zero. centering of start value (+0.5) is
 * omitted here. this was necessary to allow highler level to change center
 * for non shared edges.
 * */
D2_EXTERN void d2_triedge_setup_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, d2_u32 rightedge );

/*--------------------------------------------------------------------------
 * note should use sqrtsetup when bluring long distance
 * edge deltas may not both be zero.
 * */
D2_EXTERN void d2_triedge_setupblur_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, const d2_contextdata *ctx );


/*--------------------------------------------------------------------------
 * warning: will overflow earlier than aa version. might blow 11bit border
 * edge deltas may not both be zero.
 * */
D2_EXTERN void d2_triedge_setupnoaa_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, d2_s32 rightedge );


/*--------------------------------------------------------------------------
 *
 * */
D2_EXTERN void d2_triedge_setupsqrt_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, d2_s32 rightedge );


/*--------------------------------------------------------------------------
 * used to setup bottom-up rendering
 * */
D2_EXTERN void d2_invertlimiter_intern(d2_limdata *lim, d2_s32 ystep);


/*-------------------------------------------------------------------------- */
#endif
