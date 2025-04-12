/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_context.h (%version: 20 %)
 *          created Tue Jan 11 16:24:57 2005 by hh04027
 *
 * Description:
 *  %date_modified: Tue Dec 12 13:43:08 2006 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2008-04-30 MRe  added RLE and subbyte formats
 *  2008-09-25 MRe  added circleextendoffset
 *  2010-02-26 MRe  removed srcblend and dstblend from context
 *  2011-03-17 MRe  reduced size of context structure
 *  2012-09-25 BSp  MISRA cleanup
*-------------------------------------------------------------------------- */

#ifndef __1_dave_context_h_H
#define __1_dave_context_h_H
/*--------------------------------------------------------------------------- */

#include "dave_gradient.h"

/*--------------------------------------------------------------------------- */

typedef d2_u8 d2_context_featurebits;

#define d2_feat_none   0u
#define d2_feat_blur   1u    /* set if bluring is active */
#define d2_feat_aa     2u    /* set if antialiasing is active */
#define d2_feat_ulim   4u    /* set if ulimiter is required */
#define d2_feat_vlim   8u    /* set if vlimiter is required */

/*---------------------------------------------------------------------------*/

typedef d2_u8 d2_context_dirtybits;
 
#define d2_dirty_material      1u
#define d2_dirty_premalpha_p   2u
#define d2_dirty_premalpha_t   4u
#define d2_dirty_upatlim       8u
#define d2_dirty_texlim       16u

/*---------------------------------------------------------------------------
 * Context structure */

typedef struct _d2_contextdata
{
   struct _d2_contextdata *next;       /* to chain device context's [must be index 0] */
   struct _d2_devicedata *device;

   /* material parameters */
   d2_color         basecolor[2];
   d2_alpha         patternalpha[2];
   d2_alpha         basealpha[2];
   d2_width         blurring;
   d2_u8            alphamode;
   d2_u8            fillmode;
   d2_u8            alphablendflags;
   d2_u8            gradients;
   d2_gradientdata  gradient[4];
   d2_gradientdata  patulim[1];
   d2_gradientdata  texlim[2];
   d2_s32           orgpattern;
   d2_s32           patlen;
   d2_s32           patoffset;
   d2_s32           patscale;
   d2_u8            patmode;
   d2_u8            features;
   d2_u8            linecap;
   d2_u8            linejoin;
   d2_width         miterlimit;
   d2_width         circleextendoffset;
   d2_alpha         texop_p1[4];
   d2_alpha         texop_p2[4];
   d2_u8            texamode, texrmode, texgmode, texbmode;
   d2_u8            texmode;
   d2_u8            texbpp;
   d2_u8            texsubppb;    /* sub-byte formats: pixel per byte 1, 2, 4 or 8 */
   d2_u8            rlebpp;       /* rle encoded bytes per pixel 1, 2, 3 or 4 */
   d2_s32           texpitch;
   d2_s32           texwidth;
   d2_s32           texheight;
   void *           texbase;
   d2_s32           texcenterx;
   d2_s32           texcentery;
   d2_color *       texclut;
   d2_color *       texclut_cached;
   d2_s8            texclutupload;
   d2_u8            texclutoffset;
   /* precalculated values */
   d2_u8           internaldirty;
   d2_s8           roundends;
   d2_u32          invblur;
   d2_s32          pattern;
   d2_s32          invpatscale;

   d2_u32          blendmask;
   d2_u32          alphablendmask;
   d2_u32          cr2mask;
   d2_u32          thresholdmask;
   d2_u32          tbstylemask;
   d2_u32          patmodemask;
   d2_u32          texmodemask;
   d2_u32          texwrapmask;
   d2_u32          rlemask;
   d2_u32          clutmask;
   d2_u32          colkeymask;

   d2_color        colorkey;

   d2_color        constalpha;
   d2_color        premalpha[2];
   d2_color        texmodecl[2];
   d2_color        texcolreg[2];

   void *          blit_src;
   d2_s32          blit_pitch;
   d2_s32          blit_width;
   d2_s32          blit_height;
   d2_u32          blit_format;

} d2_contextdata;


/*---------------------------------------------------------------------------
* Backup data for context content used by blit structure */

typedef struct _d2_contextdata_backup
{
    d2_u8              fillmode;         
    d2_u8              features;
    d2_u8              internaldirty;
    d2_s32             texcenterx;
    d2_s32             texcentery;
    d2_u32             tbstylemask;
    d2_u32             texwrapmask;
    d2_u32             texmodemask;
    d2_s32             texpitch;
    d2_s32             texwidth;
    d2_s32             texheight;
    void *             texbase;
    d2_u8              texmode;
    d2_u8              texbpp;
    d2_u8              texsubppb;
    d2_u8              rlebpp;
    d2_u32             rlemask;
    d2_u32             clutmask;
    d2_u32             cr2mask;
    d2_color           texmodecl[2];
    d2_gradientdata    texlim[2];

} d2_contextdata_backup;

/*--------------------------------------------------------------------------- */

D2_EXTERN void d2_calcpatternalpha_intern( d2_contextdata *ctx );

/*--------------------------------------------------------------------------- */
#endif
