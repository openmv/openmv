/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_driver.h (%version: 69 %)
 *          created Mon Jan 10 13:38:57 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Apr 26 16:21:32 2007 %  (%derived_by:  hh74036 %)
 *
 * This is the only include file that should be read by driver applications
 * See html documentation
 *
 * Changes:
 *  2007-08-29 ASc  Include headers of level0/1 driver and math pkg,
 *                  removed tabs, changed C++ to C comments
 *  2007-09-20 ASc  Added blit flags for wrapping
 *  2008-04-30 MRe  added RLE and subbyte formats
 *  2008-06-12 MRe  added CLUT256 and color keying
 *  2008-07-14 ASt  added additional performance counter values
 *  2009-01-16 MRe  added defines for feature bits from dave_registermap.h 
 *  2009-03-06 LBe  added enum constant for render buffer execute flags
 *  2009-03-11 MRe  incremented version to 3.2
 *  2010-02-18 MRe  incremented version to 3.3 for Alpha blending feature
 *  2010-03-18 MRe  new enum d2_blendflags
 *  2010-07-08 MRe  new device flag d2_df_no_registercaching
 *  2010-09-13 MRe  incremented version to 3.4 beta (due to merge from branch)
 *  2010-09-15 MRe  removed d0 memory functions, replaced by d1 memory functions 
 *  2010-09-22 MRe  fixed typo of dc_cc_rgb, dc_cc_all
 *  2011-03-11 MRe  improved/removed context backup for blit
 *  2011-03-14 MRe  version 3.4
 *  2011-05-30 MRe  version 3.5 beta
 *  2011-06-16 MRe  added Alpha4, Alpha2, Alpha1 texture formats
 *  2012-09-05 MRe  version 3.7
 *  2012-09-25 BSp  MISRA cleanup
 *  2012-10-19 MRe  version 3.8
 *  2020-02-05 MRe  version 3.18 added d2_inithwburstlengthlimit
*-------------------------------------------------------------------------- */

#ifndef __1_dave_driver_h_H
#define __1_dave_driver_h_H
#ifdef __cplusplus
extern "C" {
#endif
/*--------------------------------------------------------------------------- */

#define D2_VERSION_MAJOR    3
#define D2_VERSION_MINOR    18
#define D2_VERSION_STATE    ""  /*"beta"*/
#define D2_VERSION_BRANCH   0

#if (D2_VERSION_BRANCH == 0)
  #define D2_VERSION_BRANCH_STRING
#elif (D2_VERSION_BRANCH == 1)
  #define D2_VERSION_BRANCH_STRING Branch_1
#endif

/*---------------------------------------------------------------------------
 Title: Basic Types

    Note that several types represent fixedpoint numbers. 
    The C compiler cannot directly create these from constants. 
    Therefore if you want to pass an integer value of 42 to a 
    function that expects an argument of e.g. type <d2_point> you would have to write :
    > function( 42 << 4 );    // conversion from integer to fixedpoint
    see also <D2_FIX4(x)>
*/

#ifndef __1_dave_base_h_H
typedef void d1_device;
#endif

#include <dave_types.h>


/*---------------------------------------------------------------------------*/
#define D2_EXTERN extern


/*---------------------------------------------------------------------------
  Type: d2_device
    void

    Abstract type. The application uses pointers of this type to hold the address of a device structure
    without knowing its internal layout.

    see for example : <d2_opendevice>
*/
typedef void d2_device;

/*---------------------------------------------------------------------------
  Type: d2_context
    void

    Abstract type. The application uses pointers of this type to hold the address of a context structure
    without knowing its internal layout.

    see for example : <d2_newcontext>
*/
typedef void d2_context;

/*---------------------------------------------------------------------------
  Type: d2_renderbuffer
    void

    Abstract type. The application uses pointers of this type to hold the address of a renderbuffer structure
    without knowing its internal layout.

    see for example : <d2_newrenderbuffer>
*/
typedef void d2_renderbuffer;

/*---------------------------------------------------------------------------
  Type: d2_color
    unsigned long

    32bit RGB value. Upper 8bits are ignored but should be set to zero.
    All colors are passed to the driver in this format regardless of the framebuffer format. 

    see for example : <d2_setcolor>
*/
typedef d2_u32 d2_color;

/*---------------------------------------------------------------------------
  Type: d2_alpha
    unsigned char

    Alpha information is passed as 8bit values. 255 representing fully opaque and 0 totally 
    transparent colors.

    see for example : <d2_setalpha>
*/
typedef d2_u8 d2_alpha;

/*---------------------------------------------------------------------------
  Type: d2_width
    short (*fixedpoint*)

    Width is defined as an unsigned 10:4 fixedpoint number (4 bits fraction). 
    So the maximum width is 1023 and the smallest nonzero width is 1/16.
*/
typedef d2_s16 d2_width;
   
/*---------------------------------------------------------------------------
  Type: d2_point
    short (*fixedpoint*)

    Point defines a vertex component (e.g. the x coordinate of an endpoint) pixel position 
    and is specified as a signed 1:11:4 fixedpoint number (1bit sign, 11 bits integer, 4 bits fraction).
    So the integer range is 2047 to -2048 and the smallest positive value is 1/16.

    Points are stored as 16bit quantities because they represent direct screen coordinates
    and therefor do not become larger than 2047 even for HDTV resolutions.

    see for example : <d2_renderline>
*/
typedef d2_s16 d2_point;

/*---------------------------------------------------------------------------
Type: d2_border
    short 

    The border type is used only when setting clip borders. In contrast to points, borders do 
    not contain any fractional information (no subpixel clipping) and are simple 11bit signed
    integers.

    see for example : <d2_cliprect>
*/
typedef d2_s16 d2_border;

/*---------------------------------------------------------------------------
  Type: d2_pattern
    unsigned long

    Patterns are Nbit bitmasks (N is 32 at most so they are passed as longs)

    see for example : <d2_setpattern>
*/
typedef d2_u32 d2_pattern;

/*---------------------------------------------------------------------------
  Type: d2_blitpos
      unsigned short

      Blitpos defines an integer position in the source bitmap of a blit rendering operation. 
      The allowed range is 0 to 1023.
*/
typedef d2_u16 d2_blitpos;


/*---------------------------------------------------------------------------
 * enums */

typedef d2_u32 d2_rendermodes;

#define d2_rm_solid           1u
#define d2_rm_outline         2u
#define d2_rm_solid_outlined  ((d2_u32)(1 + 2))
#define d2_rm_shadow          4u
#define d2_rm_solid_shadow    ((d2_u32)(1 + 4))
#define d2_rm_postprocess     8u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_blendmodes;

#define d2_bm_zero             0u
#define d2_bm_one              1u
#define d2_bm_alpha            2u
#define d2_bm_one_minus_alpha  3u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_alphamodes;

#define d2_am_opaque      0u
#define d2_am_constant    1u
#define d2_am_gradient1   2u
#define d2_am_gradient2   4u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_fillmodes;

#define d2_fm_color     0u
#define d2_fm_twocolor  1u
#define d2_fm_pattern   2u
#define d2_fm_texture   3u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_edgeflags;

#define d2_edge0_shared   1u
#define d2_edge1_shared   2u
#define d2_edge2_shared   4u
#define d2_edge3_shared   8u
#define d2_edge4_shared  16u
#define d2_edge5_shared  32u

#define d2_all_shared    63u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_wedgeflags;

#define d2_wf_concave  (d2_all_shared + 1)

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_linecapflags;

#define d2_lc_butt    0u
#define d2_lc_square  1u
#define d2_lc_round   2u

#define d2_lc_max     3u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_linejoinflags;

#define d2_lj_none    0u
#define d2_lj_miter   1u
#define d2_lj_round   2u
#define d2_lj_bevel   4u

#define d2_lj_max     7u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_lineendflags;

#define d2_le_exclude_none   0u
#define d2_le_exclude_start  1u
#define d2_le_exclude_end    2u
#define d2_le_exclude_both   3u
#define d2_le_closed         4u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_segmentflags;

#define d2_sf_none  0u
#define d2_sf_skip  1u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_modeflags;

#define d2_mode_alpha8      0u
#define d2_mode_rgb565      1u

#define d2_mode_argb8888    2u
#define d2_mode_argb4444    3u
#define d2_mode_argb1555    4u
   
#define d2_mode_ai44        5u

#define d2_mode_rgba8888    6u
#define d2_mode_rgba4444    7u
#define d2_mode_rgba5551    8u
   
#define d2_mode_i8          9u
#define d2_mode_i4          10u
#define d2_mode_i2          11u
#define d2_mode_i1          12u
   
#define d2_mode_alpha4      13u
#define d2_mode_alpha2      14u
#define d2_mode_alpha1      15u
   
#define d2_mode_rgb888      64u /* used driver internally */
#define d2_mode_rgb444      65u /* used driver internally */
#define d2_mode_rgb555      66u /* used driver internally */
   
   /* following additional flags can be ored together with previous modes: */
#define d2_mode_rle         16u     /* RLE decoder is used */
#define d2_mode_clut        32u     /* CLUT 256 is used */

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_getcontextmodes;

#define d2_context_default   0u
#define d2_context_selected  1u
#define d2_context_solid     2u
#define d2_context_outline   3u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_textureoperations;

#define d2_to_zero         0u
#define d2_to_one          1u
#define d2_to_replace      2u
#define d2_to_copy         3u
#define d2_to_invert       4u
#define d2_to_multiply     5u
#define d2_to_invmultiply  6u
#define d2_to_blend        7u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_colorchannels;

#define d2_cc_alpha   1u
#define d2_cc_red     2u
#define d2_cc_green   4u
#define d2_cc_blue    8u
#define d2_cc_rgb    14u
#define d2_cc_all    15u
#define dc_cc_rgb    d2_cc_rgb /* backward compatibility */ 
#define dc_cc_all    d2_cc_all /* backward compatibility */ 

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_texturemodes;

#define d2_tm_wrapu     1u
#define d2_tm_wrapv     2u
#define d2_tm_filteru   4u
#define d2_tm_filterv   8u
#define d2_tm_filter   12u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_patternmodes;

#define d2_pm_filter     1u
#define d2_pm_autoalign  2u
#define d2_pm_advance    4u
#define d2_pm_orthogonal 8u   /* TES unsupported parameter */

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_blitflags;

#define d2_bf_filteru           d2_tm_filteru
#define d2_bf_filterv           d2_tm_filterv
#define d2_bf_filter            d2_tm_filter
#define d2_bf_wrapu             d2_tm_wrapu
#define d2_bf_wrapv             d2_tm_wrapv
#define d2_bf_wrap              (d2_tm_wrapu | d2_tm_wrapv)
#define d2_bf_colorize          (d2_bf_filterv * 2)
#define d2_bf_usealpha          (d2_bf_colorize * 2)
#define d2_bf_colorize2         (d2_bf_usealpha * 2)
#define d2_bf_invertalpha       (d2_bf_colorize2 * 2)
#define d2_bf_no_blitctxbackup  (d2_bf_invertalpha * 2)
#define d2_bf_mirroru           (d2_bf_no_blitctxbackup * 2)
#define d2_bf_mirrorv           (d2_bf_mirroru * 2)

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_deviceflags;

#define d2_df_no_dlist             1u
#define d2_df_no_irq               2u
#define d2_df_no_fbcache           4u
#define d2_df_no_texcache          8u
#define d2_df_no_dwclear          16u
#define d2_df_no_registercaching  32u
#define d2_df_no_blitctxbackup    64u

/*--------------------------------------------------------------------------- */

enum d2_perfcountevents
{
  d2_pc_disable            =  0,
  d2_pc_davecycles         =  1,
  d2_pc_fbreads            =  2,
  d2_pc_fbwrites           =  3,
  d2_pc_texreads           =  4,
  d2_pc_invpixels          =  5,
  d2_pc_invpixels_miss     =  6,
  d2_pc_dlrcycles          =  7,
  d2_pc_fbreadhits         =  8,
  d2_pc_fbreadmisses       =  9,
  d2_pc_fbwritehits        = 10,
  d2_pc_fbwritemisses      = 11,
  d2_pc_texreadhits        = 12,
  d2_pc_texreadmisses      = 13,
  d2_pc_cpudatareads       = 14,
  d2_pc_cpudatawrites      = 15,
  d2_pc_cpuinstrreads      = 16,

  d2_pc_dlrburstreads      = 17,
  d2_pc_dlrwordsread       = 18,

  d2_pc_rlerewinds         = 20,
  d2_pc_texburstreads      = 21,
  d2_pc_texwordsread       = 22,
  d2_pc_fbburstreads       = 23,
  d2_pc_fbwordsread        = 24,
  d2_pc_fbburstwrites      = 25,
  d2_pc_fbwordswritten     = 26,

  d2_pc_fbrwconflicts      = 28,
  d2_pc_fbrwconflictcycles = 29,
  d2_pc_noevent            = 30,
  d2_pc_clkcycles          = 31
};

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_executeflags;

#define d2_ef_default           0u
#define d2_ef_execute_once      0u
#define d2_ef_execute_multiple  1u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_blendflags;

#define d2_blendf_default      0u
#define d2_blendf_blenddst     0u
#define d2_blendf_blendcolor2  1u

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_adddlistflags;

#define d2_al_default  0u  /* default behavior                   */
#define d2_al_copy     0u  /* content will be copied             */
#define d2_al_no_copy  1u  /* jump to the dlist will be added    */

/*--------------------------------------------------------------------------- */

typedef d2_u32 d2_busburstlength;

#define d2_bbl_1   0u  /* single cycle bus access       */
#define d2_bbl_2   1u  /* max bus burst length = 2      */
#define d2_bbl_4   2u  /* max bus burst length = 4      */
#define d2_bbl_8   3u  /* max bus burst length = 8      */
#define d2_bbl_16  4u  /* max bus burst length = 16     */
#define d2_bbl_32  5u  /* max bus burst length = 32     */

/*---------------------------------------------------------------------------
 * basic functions */

D2_EXTERN d2_s32         d2_getversion( void );
D2_EXTERN const d2_char *d2_getversionstring( void );
D2_EXTERN d2_device *    d2_opendevice( d2_u32 flags );
D2_EXTERN d2_s32         d2_closedevice( d2_device *handle );
D2_EXTERN d2_s32         d2_geterror( const d2_device *handle );
D2_EXTERN const d2_char *d2_geterrorstring( const d2_device *handle );
D2_EXTERN const d2_char *d2_translateerror( d2_s32 errorcode );
D2_EXTERN d2_s32         d2_inithw( d2_device *handle, d2_u32 flags );
D2_EXTERN d2_s32         d2_deinithw( d2_device *handle );
D2_EXTERN d2_s32         d2_inithwburstlengthlimit(d2_device *handle, d2_busburstlength burstlengthFBread, d2_busburstlength burstlengthFBwrite, d2_busburstlength burstlengthTX, d2_busburstlength burstlengthDL);
D2_EXTERN d1_device *    d2_level1interface( const d2_device *handle );
D2_EXTERN d2_u32         d2_getrevisionhw( const d2_device *handle );
D2_EXTERN const d2_char* d2_getrevisionstringhw( const d2_device *handle);
D2_EXTERN d2_s32         d2_setdlistblocksize(d2_device *handle, d2_u32 size);
D2_EXTERN d2_u32         d2_getdlistblocksize(const d2_device *handle);
D2_EXTERN d2_u32         d2_getdlistblockcount(d2_device *handle);
D2_EXTERN d2_s32         d2_commandspending(d2_device *handle);
D2_EXTERN d2_s32         d2_lowlocalmemmode(d2_device *handle, d2_u32 dlistblockfactor, d2_u32 dlistblocks);

/*---------------------------------------------------------------------------
 * context management */

D2_EXTERN d2_context * d2_newcontext( d2_device *handle );
D2_EXTERN d2_s32       d2_freecontext( d2_device *handle, d2_context *ctx );
D2_EXTERN d2_s32       d2_selectcontext( d2_device *handle, d2_context *ctx );
D2_EXTERN d2_s32       d2_solidcontext( d2_device *handle, d2_context *ctx );
D2_EXTERN d2_s32       d2_outlinecontext( d2_device *handle, d2_context *ctx );
D2_EXTERN d2_context * d2_getcontext( d2_device *handle, d2_s32 mode );

/*---------------------------------------------------------------------------
 * device */

D2_EXTERN d2_s32 d2_framebuffer( d2_device *handle, void *ptr, d2_s32 pitch, d2_u32 width, d2_u32 height, d2_s32 format );
D2_EXTERN d2_s32 d2_cliprect( d2_device *handle, d2_border xmin, d2_border ymin, d2_border xmax, d2_border ymax );
D2_EXTERN d2_s32 d2_flushframe( d2_device *handle );
D2_EXTERN d2_s32 d2_startframe( d2_device *handle );
D2_EXTERN d2_s32 d2_endframe( d2_device *handle );
D2_EXTERN d2_s32 d2_relocateframe( d2_device *handle, const void *ptr );
D2_EXTERN d2_s32 d2_clear( d2_device *handle, d2_color color );
D2_EXTERN d2_s32 d2_getcliprect( d2_device *handle, d2_border *xmin, d2_border *ymin, d2_border *xmax, d2_border *ymax );
D2_EXTERN d2_s32 d2_getframebuffer( d2_device *handle, void** ptr, d2_s32* pitch, d2_u32* width, d2_u32* height, d2_s32* format);

/*---------------------------------------------------------------------------
 * device global attributes */

D2_EXTERN d2_s32 d2_selectrendermode( d2_device *handle, d2_u32 mode );
D2_EXTERN d2_s32 d2_outlinewidth( d2_device *handle, d2_width width );
D2_EXTERN d2_s32 d2_shadowoffset( d2_device *handle, d2_point x, d2_point y );
D2_EXTERN d2_u32 d2_getrendermode( const d2_device *handle );
D2_EXTERN d2_s32 d2_layermerge( d2_device *handle );

/*---------------------------------------------------------------------------
 * renderbuffer management */

D2_EXTERN d2_renderbuffer * d2_newrenderbuffer( d2_device *handle, d2_u32 initialsize, d2_u32 stepsize );
D2_EXTERN d2_s32            d2_freerenderbuffer( d2_device *handle, d2_renderbuffer *buffer );
D2_EXTERN d2_s32            d2_selectrenderbuffer( d2_device *handle, d2_renderbuffer *buffer );
D2_EXTERN d2_s32            d2_executerenderbuffer( d2_device *handle, d2_renderbuffer *buffer, d2_u32 flags );
D2_EXTERN d2_renderbuffer * d2_getrenderbuffer( d2_device *handle, d2_s32 index );
D2_EXTERN d2_s32            d2_dumprenderbuffer( d2_device *handle, d2_renderbuffer *buffer, void **rdata, d2_s32 *rsize );
D2_EXTERN d2_u32            d2_getrenderbuffersize(d2_device *handle, d2_renderbuffer *rb);
D2_EXTERN d2_s32            d2_freedumpedbuffer( d2_device *handle, void *data );

/*---------------------------------------------------------------------------
 * context attribute writes */

D2_EXTERN d2_s32 d2_setcolor( d2_device *handle, d2_s32 index, d2_color color );
D2_EXTERN d2_s32 d2_setalpha( d2_device *handle, d2_alpha alpha );
D2_EXTERN d2_s32 d2_setalphaex( d2_device *handle, d2_s32 index, d2_alpha alpha );
D2_EXTERN d2_s32 d2_setblur( d2_device *handle, d2_width blur );
D2_EXTERN d2_s32 d2_setblendmode( d2_device *handle, d2_u32 srcfactor, d2_u32 dstfactor );
D2_EXTERN d2_s32 d2_setalphablendmode( d2_device *handle, d2_u32 srcfactor, d2_u32 dstfactor );
D2_EXTERN d2_s32 d2_setalphablendmodeex( d2_device *handle, d2_u32 srcfactor, d2_u32 dstfactor, d2_u32 blendflags );
D2_EXTERN d2_s32 d2_setalphagradient( d2_device *handle, d2_s32 index, d2_point x, d2_point y, d2_point dx, d2_point dy );
D2_EXTERN d2_s32 d2_setclipgradient( d2_device *handle, d2_s32 index, d2_point x, d2_point y, d2_s32 nx, d2_s32 ny, d2_u32 flags );
D2_EXTERN d2_s32 d2_setalphamode( d2_device *handle, d2_u32 mode );
D2_EXTERN d2_s32 d2_setantialiasing( d2_device *handle, d2_s32 enable );
D2_EXTERN d2_s32 d2_setpatternalpha( d2_device *handle, d2_s32 index, d2_alpha alpha );
D2_EXTERN d2_s32 d2_setfillmode( d2_device *handle, d2_u32 mode );
D2_EXTERN d2_s32 d2_setpattern( d2_device *handle, d2_pattern pattern );
D2_EXTERN d2_s32 d2_setpatternparam( d2_device *handle, d2_point x, d2_point y, d2_width dx, d2_width dy );
D2_EXTERN d2_s32 d2_setpatternmode( d2_device *handle, d2_u32 mode );
D2_EXTERN d2_s32 d2_setpatternsize( d2_device *handle, d2_s32 size );
D2_EXTERN d2_s32 d2_setlinecap( d2_device *handle, d2_u32 mode );
D2_EXTERN d2_s32 d2_setlinejoin( d2_device *handle, d2_u32 mode );
D2_EXTERN d2_s32 d2_setlinepattern( d2_device *handle, d2_width scale, d2_s32 offset );
D2_EXTERN d2_s32 d2_setmiterlimit( d2_device *handle, d2_width miter );
D2_EXTERN d2_s32 d2_settexture( d2_device *handle, void *ptr, d2_s32 pitch, d2_s32 width, d2_s32 height, d2_u32 format );
D2_EXTERN d2_s32 d2_settexturemode( d2_device *handle, d2_u32 mode );
D2_EXTERN d2_s32 d2_settextureoperation( d2_device *handle, d2_u8 amode, d2_u8 rmode, d2_u8 gmode, d2_u8 bmode );
D2_EXTERN d2_s32 d2_settexopparam( d2_device *handle, d2_u32 index, d2_u32 p1, d2_u32 p2 );
D2_EXTERN d2_s32 d2_settexturemapping( d2_device *handle, d2_point x, d2_point y, d2_s32 u0, d2_s32 v0, d2_s32 dxu, d2_s32 dyu, d2_s32 dxv, d2_s32 dyv );
D2_EXTERN d2_s32 d2_settexelcenter( d2_device *handle, d2_point x, d2_point y );
D2_EXTERN d2_s32 d2_settexclut( d2_device *handle, d2_color* clut );
D2_EXTERN d2_s32 d2_settexclut_part( d2_device *handle, const d2_color* clut_part, d2_u32 start_index, d2_u32 length );
D2_EXTERN d2_s32 d2_writetexclut_direct( d2_device *handle, const d2_color* clut_part, d2_u32 start_index, d2_u32 length );
D2_EXTERN d2_s32 d2_settexclut_offset( d2_device *handle, d2_u32 offset );
D2_EXTERN d2_s32 d2_settexclut_format(  d2_device *handle, d2_u32 format );
D2_EXTERN d2_s32 d2_setcolorkey( d2_device *handle, d2_s32 enable, d2_color color_key );
D2_EXTERN d2_s32 d2_setcircleextend( d2_device *handle, d2_width offset );

/*---------------------------------------------------------------------------
 * context attritbute reads */

D2_EXTERN d2_color d2_getcolor( d2_device *handle, d2_s32 index );
D2_EXTERN d2_alpha d2_getalpha( d2_device *handle );
D2_EXTERN d2_alpha d2_getalphaex( d2_device *handle, d2_s32 index );
D2_EXTERN d2_width d2_getblur( d2_device *handle );
D2_EXTERN d2_u32 d2_getblendmodesrc( d2_device *handle );
D2_EXTERN d2_u32 d2_getblendmodedst( d2_device *handle );
D2_EXTERN d2_u32 d2_getalphablendmodesrc( d2_device *handle );
D2_EXTERN d2_u32 d2_getalphablendmodedst( d2_device *handle );
D2_EXTERN d2_u8  d2_getalphablendmodeflags( d2_device *handle );
D2_EXTERN d2_u8  d2_getalphamode( d2_device *handle );
D2_EXTERN d2_s32 d2_getantialiasing( d2_device *handle );
D2_EXTERN d2_alpha d2_getpatternalpha( d2_device *handle, d2_s32 index );
D2_EXTERN d2_u8  d2_getfillmode( d2_device *handle );
D2_EXTERN d2_pattern d2_getpattern( d2_device *handle );
D2_EXTERN d2_u32 d2_getpatternmode( d2_device *handle );
D2_EXTERN d2_s32 d2_getpatternsize( d2_device *handle );
D2_EXTERN d2_u8  d2_getlinecap( d2_device *handle );
D2_EXTERN d2_u8  d2_getlinejoin( d2_device *handle );
D2_EXTERN d2_u8  d2_gettextureoperationa( d2_device *handle );
D2_EXTERN d2_u8  d2_gettextureoperationr( d2_device *handle );
D2_EXTERN d2_u8  d2_gettextureoperationg( d2_device *handle );
D2_EXTERN d2_u8  d2_gettextureoperationb( d2_device *handle );
D2_EXTERN d2_alpha d2_gettexopparam1( d2_device *handle, d2_u32 index );
D2_EXTERN d2_alpha d2_gettexopparam2( d2_device *handle, d2_u32 index );

/*---------------------------------------------------------------------------
 * rendering commands */

D2_EXTERN d2_s32 d2_renderbox( d2_device *handle, d2_point x1, d2_point y1, d2_width w, d2_width h );
D2_EXTERN d2_s32 d2_renderline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w, d2_u32 flags );
D2_EXTERN d2_s32 d2_rendertri( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags );
D2_EXTERN d2_s32 d2_renderquad( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags );
D2_EXTERN d2_s32 d2_rendercircle( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w );
D2_EXTERN d2_s32 d2_renderwedge( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags );
D2_EXTERN d2_s32 d2_renderline2(  d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_width w1, d2_width w2, d2_u32 flags );

D2_EXTERN d2_s32 d2_renderpolyline( d2_device *handle, const d2_point *data, d2_u32 count, d2_width w, d2_u32 flags);
D2_EXTERN d2_s32 d2_renderpolyline2( d2_device *handle, const d2_point *data, d2_u32 count, const d2_width *w, d2_u32 flags);
D2_EXTERN d2_s32 d2_rendertrilist( d2_device *handle, const d2_point *data, const d2_u32 *flags, d2_u32 count);
D2_EXTERN d2_s32 d2_rendertrifan( d2_device *handle, const d2_point *data, const d2_u32 *flags, d2_u32 count);
D2_EXTERN d2_s32 d2_rendertristrip( d2_device *handle, const d2_point *data, const d2_u32 *flags, d2_u32 count);
D2_EXTERN d2_s32 d2_renderpolygon( d2_device *handle, const d2_point *data, d2_u32 count, d2_u32 flags);

/*---------------------------------------------------------------------------
 * blit attributes write */

D2_EXTERN d2_s32 d2_setblitsrc( d2_device *handle, void *ptr, d2_s32 pitch, d2_s32 width, d2_s32 height, d2_u32 format );

/*---------------------------------------------------------------------------
 * blit rendering functions */

D2_EXTERN d2_s32 d2_blitcopy( d2_device *handle, d2_s32 srcwidth, d2_s32 srcheight, d2_blitpos srcx, d2_blitpos srcy, d2_width dstwidth, d2_width dstheight, d2_point dstx, d2_point dsty, d2_u32 flags );

/*---------------------------------------------------------------------------
 * performance measurement */
D2_EXTERN d2_s32 d2_setperfcountevent( d2_device *handle, d2_u32 counter, d2_u32 event );
D2_EXTERN d2_s32 d2_setperfcountvalue( d2_device *handle, d2_u32 counter, d2_slong value );
D2_EXTERN d2_slong d2_getperfcountvalue( d2_device *handle, d2_u32 counter );

/*---------------------------------------------------------------------------
 * Utility Functions */

d2_s32 d2_utility_maptriangle    ( d2_device *handle, const d2_f32 *points, const d2_f32 *uvs );
d2_s32 d2_utility_perspectivewarp( d2_device *handle, d2_u16 srcwidth, d2_u16 srcheight, d2_s16 srcx, d2_s16 srcy, d2_s16 dstwidth, d2_s16 dstheight, d2_s16 dstx, d2_s16 dsty, d2_u16 wt );
d2_s32 d2_utility_fbblitcopy     ( d2_device *handle, d2_u16 width, d2_u16 height, d2_blitpos srcx, d2_blitpos srcy, d2_blitpos dstx, d2_blitpos dsty, d2_u32 flags);
void d2_rendercircle_no_hilimiterprecision( d2_device *handle, d2_u32 flag );



/*---------------------------------------------------------------------------
 * Dlist Functions */

d2_s32 d2_executedlist( d2_device *handle, const void *address, d2_u32 flags );
d2_s32 d2_adddlist( d2_device *handle, void *address, d2_s32 size, d2_u32 flags );

/*---------------------------------------------------------------------------
 * assign errorcode IDs */

#define ERR(x,y) x,
enum d2_errorcodes {
#include "dave_errorcodes.h"
D2_ERROR_QUANTITY };
#undef ERR

/*---------------------------------------------------------------------------
 * include header files of level0 and level1 driver */

#include "dave_base.h"
#include "dave_math.h"

/*---------------------------------------------------------------------------
* define feature bits of hardware revision */
#ifndef BIT
#define BIT(x)                         (1u<<(x))
#endif
#define D2FB_SWDAVE                    BIT(16)
#define D2FB_DLR                       BIT(17)
#define D2FB_FBCACHE                   BIT(18)
#define D2FB_TXCACHE                   BIT(19)
#define D2FB_PERFCOUNT                 BIT(20)
#define D2FB_TEXCLUT                   BIT(21)
#define D2FB_FBPREFETCH                BIT(22)
#define D2FB_RLEUNIT                   BIT(23)
#define D2FB_TEXCLUT256                BIT(24)
#define D2FB_COLORKEY                  BIT(25)
#define D2FB_HILIMITERPRECISION        BIT(26)
#define D2FB_ALPHACHANNELBLENDING      BIT(27)

/*--------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif
