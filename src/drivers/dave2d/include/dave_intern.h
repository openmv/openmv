/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_intern.h (%version: 29 %)
 *          created Tue Jan 11 12:57:48 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Jan 25 18:12:54 2007 %  (%derived_by:  hh74026 %)
 *
 * Changes:
 *  2005-12-22 CSe  removed transfermode field, added flags field in _d2_devicedata
 *  2006-02-16 CSe  added performance counter support
 *  2006-10-30 CSe  removed waitdlist flag
 *  2006-11-07 CSe  removed waitdlist flag
 *  2006-11-07 CSe  added dlist_buffer to device struct for 'd2_df_low_localmem' mode
 *  2007-01-26 CSe  made 'low localmem' mode configurable
 *  2007-05-23 MGe  added delayed_errorcode to context
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2008-11-24 AJ   Added support for IAR compiler
 *  2011-09-05 MRe  added cr2 register to device structure
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_intern_h_H
#define __1_dave_intern_h_H
/*---------------------------------------------------------------------------
 * define inline modifier if available */
#ifdef _MSC_VER
#define D2_INLINE __forceinline
#else
#ifdef __CA850__
#define D2_INLINE
#elif defined(__ICCV850__)
#define D2_INLINE _Pragma("inline")
#else
#define D2_INLINE inline
#endif
#endif

/*---------------------------------------------------------------------------
 * define null to become independent of stdlib */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

/*---------------------------------------------------------------------------
 * internally used types */

#ifdef _MSC_VER
typedef __int64 d2_int64;
#elif (defined(__CA850__) || defined(__ICC850__))
#define _NO_LL_
typedef struct {
   d2_u32 low32;
   d2_s32 high32;
} d2_int64;
#else
typedef long long d2_int64;
#endif

/*---------------------------------------------------------------------------
 * use separate context for blit 
 see also d2_df_no_blitctxbackup and d2_bf_no_blitctxbackup*/

/*
#define D2_USEBLITBACKUPCONTEXT 
*/


/*---------------------------------------------------------------------------
 * force register cache usage
 see also d2_df_no_registercaching */

#define D2_USEREGCACHE 

/*---------------------------------------------------------------------------
 * set initial size of list of dlist start address */

#define DLIST_ADRESSES_NUMBER 16


/*---------------------------------------------------------------------------
 * Register Cache */

#include "dave_registermap.h"

typedef struct _d2_cacheddata
{
   d2_s32    data[  D2_QUANTITY ];
   d2_s8     valid[ D2_QUANTITY ];
} d2_cacheddata;

#ifdef D2_USEREGCACHE
D2_EXTERN const d2_u8 d2_cacheableregs[D2_QUANTITY];
#endif

typedef void (*d2_fp_scratchfull)( void *handle );

/*--------------------------------------------------------------------------- */
#include "dave_base.h"
#include "dave_dlist.h"
#include "dave_rbuffer.h"
#include "dave_hardware.h"
#include "dave_context.h"
#include "dave_math.h"

/*---------------------------------------------------------------------------
 * use higher precision of qlimiters */

#define LIMITER_HIPRECISION 6 


/*---------------------------------------------------------------------------
 * These helper macros are used to stringify a given macro */
#define D2_STR(s)           # s
#define D2_XSTR(s)          D2_STR(s)  /* PRQA S 3453 */ /* $Misra: #MISRA_BUG_MACRO_HASH $*/


/*---------------------------------------------------------------------------
 * Define the D2_VERSION and D2_VERSION_STRING macros */

/* Build up the D2_VERSION macro */
#define D2_VERSION ((D2_VERSION_BRANCH << 24) | (D2_VERSION_MAJOR << 16) | D2_VERSION_MINOR )

/* Create the D2_VERSION_STRING macro */
#define D2_VERSION_STRING  D2_VERSION_BRANCH_STRING " V" D2_XSTR(D2_VERSION_MAJOR) "." D2_XSTR(D2_VERSION_MINOR)

/*---------------------------------------------------------------------------
 * Internal device structure */

typedef struct _d2_devicedata
{
   struct _d2_devicedata *next;        /* to chain devices [must be index 0] */

   d2_dlist *readlist;                 /* read (executing) dlist */
   d2_dlist *writelist;                /* write (under construction) dlist */
   d2_rbuffer *selectedbuffer;         /* currently selected renderbuffer */
   d2_rbuffer *renderbuffer[2];        /* default renderbuffers */

   d1_device      *hwid;               /* lowlevel access handle */
   d2_contextdata *ctxchain;           /* context chain (list of all contexts for this device) */
   d2_contextdata *ctxdef;             /* default context (cannot be removed by application) */
   d2_contextdata *ctxselected;        /* selected context (used as target of context write/read ops) */
   d2_contextdata *ctxsolid;           /* solid context (used as source for rendering solid geometry) */
   d2_contextdata *ctxoutline;         /* outline context (used as source for rendering outlines) */
#ifdef D2_USEBLITBACKUPCONTEXT
   d2_contextdata *blitcontext;        /* internal context used only for blits */
#else
   d2_contextdata_backup *blitcontext_b; /* internal context backup used only for blits */
#endif
   d2_contextdata *srccontext;         /* last context that acted as source for display list (for dirtybit checks) */
   d2_color       *srcclut;            /* last clut that was loaded  */

   void *   framebuffer;               /* rendering baseaddress */
   d2_s32   pitch, bpp;                /* framebuffer pitch and bytes per pixel */
   d2_s32   fbformat;                  /* framebuffer format */
   d2_u32   fbwidth,fbheight;          /* framebuffer size */
   d2_u32   fbstylemask;               /* dave ctrl1 bitmask for framebuffer format */
   d2_u32   cr2;                       /* control register 2 */

   d2_u32   cachectlmask;              /* dave cache ctrl register bitmask */
   d2_u32   perftriggermask;           /* dave performance counter trigger source register bitmask */

   d2_u32   rendermode;                /* see 'enum d2_rendermodes' */
   d2_u32   flags;                     /* see 'enum d2_deviceflags' */
   d2_width outlinewidth;
   d2_point soffx, soffy;
   d2_point clipxmin, clipymin;
   d2_point clipxmax, clipymax;

   d2_lowlocalmem_mode *lowlocalmem_mode;       /* only used in 'low localmem' mode */

   d2_u32 hwrevision;
   d2_u32 hwmemarchitecture;

   d2_u32 dlistblocksize;              /* specify number of entries to grow when dlist is full */
#ifdef D2_USEREGCACHE
   d2_cacheddata cache;                 /* register cache for this device */
#endif
   /* display list assembly buffer */
   d2_dlist_scratch_entry *dlscratch_base;
   d2_dlist_scratch_entry *dlscratch_pos;
   d2_s32                  dlscratch_cnt;
   d2_fp_scratchfull       dlscratch_hook;

   d2_s8   lasttextwasrle;               /* last texture format was RLE */
   d2_s8   maxpatlen;                    /* maximum size of pattern bitvector supported by the hardware */
   d2_s8   errorcode;                    /* last actions detaild result code */
   d2_s8   delayed_errorcode;            /* errorcondition that exists until the renderbuffer gets executed. This errorcode is used for errors that occure while the scratchbuffer is written to the displaylist */
   d2_s8   hilimiterprecision_supported; /* is set to 1 when the hilimiterprecision feature is supported by the hardware */

   d2_s8   dlist_indirect_supported;     /* is set to 1 when lists of dlist addresses are supported by d1 driver */
   d2_s32* dlist_list_single[2];         /* for d2_executedlist we need an extra dlist list with just one entry   */

} d2_devicedata;

/*--------------------------------------------------------------------------- */

typedef struct _d2_bbox
{
   d2_point xmin, ymin;
   d2_point xmax, ymax;
} d2_bbox;

typedef struct _d2_limdata
{
   d2_s32 start;
   d2_s32 xadd;
   d2_s32 yadd;
} d2_limdata;

/*---------------------------------------------------------------------------
 * Internal flags */

typedef d2_u32 d2_lineendflags_intern;

#define d2_lei_base                256u

#define d2_lei_ext_first_edge     (d2_lei_base *  2)
#define d2_lei_ext_last_edge      (d2_lei_base *  4)
#define d2_lei_buffer_first_edge  (d2_lei_base *  8)
#define d2_lei_buffer_last_edge   (d2_lei_base * 16)
#define d2_lei_miter_edge         (d2_lei_base * 32)
#define d2_lei_miter1_flip        (d2_lei_base * 64)
#define d2_lei_miter2_flip        (d2_lei_base *128)

/*---------------------------------------------------------------------------
 * Internal macros */

#define D2_DEV(d)           ((d2_devicedata *) (d))           /* PRQA S 3453 */ /* $Misra: #MACRO_TYPECAST_OVERKILL $*/
#define D2_CTX(c)           ((d2_contextdata *) (c))          /* PRQA S 3453 */ /* $Misra: #MACRO_TYPECAST_OVERKILL $*/
#define D2_FNC(r)           ((d2_renderfunctions *) (r))      /* PRQA S 3453 */ /* $Misra: #MACRO_TYPECAST_OVERKILL $*/
#define D2_DRB(r)           ((d2_rbuffer *) (r))              /* PRQA S 3453 */ /* $Misra: #MACRO_TYPECAST_OVERKILL $*/

static D2_INLINE d2_s32 d2_seterr(d2_device *handle, d2_s32 err); /* to satisfy MISRA rule 3450 */
static D2_INLINE d2_s32 d2_seterr(d2_device *handle, d2_s32 err) /* PRQA S 3219, 3480 */ /* $Misra: #UTIL_INLINE_FUNC $*/
{
   D2_DEV(handle)->errorcode = (d2_s8)(err);

   return err;
}

#define D2_SETERR(d,e)      d2_seterr(d, e)                   /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/
#define D2_SETOK(d)         D2_SETERR(d, D2_OK)               /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/
#define D2_RETERR(d,e)      return D2_SETERR(d,e)             /* PRQA S 3453 */ /* $Misra: #MISRA_BUG_MACRO_RETURN_STATEMENT $*/ /* PRQA S 3412 */ /* $Misra: #MACRO_RETURN $*/
#define D2_RETERRU(d,e)     return (d2_u32) (D2_SETERR(d,e))  /* PRQA S 3412 */ /* $Misra: #MACRO_RETURN $*/
#define D2_RETOK(d)         return D2_SETOK(d)                /* PRQA S 3412 */ /* $Misra: #MACRO_RETURN $*/

#ifdef _DEBUG
#define D2_VALIDATE(x,e)    if (0 == (x)) \
   {                                      \
      return (e);                         \
   }(void)0                                                   /* PRQA S 3412 */ /* $Misra: #COMPILER_WARNING $*/
#define D2_CHECKERR(x,e)    if (! (x))    \
   {                                      \
      D2_RETERR(handle, (e));             \
   }(void)0                                                   /* PRQA S 3412 */ /* $Misra: #COMPILER_WARNING $*/
#define D2_VALIDATEP(x,e)   if (0 == (x)) \
   {                                      \
      return 0;                           \
   }(void)0                                                   /* PRQA S 3412 */ /* $Misra: #COMPILER_WARNING $*/
#define D2_CHECKERRP(x,e)   if (! (x)) \
   {                                   \
      (void)D2_SETERR(handle, (e));    \
      return 0;                        \
   } (void)0                                                  /* PRQA S 3412 */ /* $Misra: #COMPILER_WARNING $*/
#define D2_CHECKRANGE( v, min, max ) if( ((min) > (v)) || ( (v) > (max)) ) \
   {                                                                       \
      (void)D2_SETERR( handle, D2_INVALIDINDEX);                           \
      return D2_INVALIDINDEX;                                              \
   }(void)0                                                   /* PRQA S 3412 */ /* $Misra: #COMPILER_WARNING $*/
#else
#define D2_VALIDATE(x,e)(void)0              /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
#define D2_CHECKERR(x,e)(void)0              /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
#define D2_VALIDATEP(x,e)(void)0             /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
#define D2_CHECKERRP(x,e)(void)0             /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
#define D2_CHECKRANGE( v, min, max )(void)0  /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
#endif

D2_EXTERN void d2_cast32to64(d2_s32 par, d2_int64 *res);
D2_EXTERN d2_s32  d2_cast64to32(const d2_int64 *par);
D2_EXTERN void d2_add64(const d2_int64 *a, const d2_int64 *b, d2_int64 *res);
D2_EXTERN void d2_sub64(const d2_int64 *a, const d2_int64 *b, d2_int64 *res);
D2_EXTERN void d2_mul3232to64(d2_s32 a, d2_s32 b, d2_int64 *res);
D2_EXTERN void d2_mul3264(d2_s32 a, d2_int64 *b, d2_int64 *res);
D2_EXTERN void d2_div6432(const d2_int64 *dividend, d2_s32 divisor, d2_int64 *res);
D2_EXTERN void d2_shiftleft64(const d2_int64 *var, d2_s32 index, d2_int64 *res);
D2_EXTERN void d2_shiftright64(const d2_int64 *var, d2_s32 index, d2_int64 *res);

#ifdef _NO_LL_
#define D2_CAST32TO64(PAR, RES)    d2_cast32to64(PAR, RES)     /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/
#define D2_CAST64TO32(PAR)         d2_cast64to32(PAR)          /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/
#define D2_ADD64(A, B, RES)        d2_add64(A, B, RES)         /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/
#define D2_SUB64(A, B, RES)        d2_sub64(A, B, RES)         /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/
#define D2_MUL3232TO64(A, B, RES)  d2_mul3232to64(A, B, RES)   /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/
#define D2_MUL3264(A, B, RES)      d2_mul3264(A, B, RES)       /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/
#define D2_DIV6432(DVD, DVS, RES)  d2_div6432(DVD, DVS, RES)   /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/
#define D2_SHIFTLEFT64(V, I, RES)  d2_shiftleft64(V, I, RES)   /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/
#define D2_SHIFTRIGHT64(V, I, RES) d2_shiftright64(V, I, RES)  /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/
#else
#define D2_CAST32TO64(PAR, RES)    *(RES) = (PAR)              /* PRQA S 3453 */ /* $Misra: #MACRO_TYPE_SAFE_FXN_REPL $*/
#define D2_CAST64TO32(PAR)         ((d2_s32)*(PAR))            /* PRQA S 3453 */ /* $Misra: #MACRO_TYPE_SAFE_FXN_REPL $*/
#define D2_ADD64(A, B, RES)        *(RES) = (*(A)) + (*(B))    /* PRQA S 3453 */ /* $Misra: #MACRO_TYPE_SAFE_FXN_REPL $*/
#define D2_SUB64(A, B, RES)        *(RES) = (*(A)) - (*(B))    /* PRQA S 3453 */ /* $Misra: #MACRO_TYPE_SAFE_FXN_REPL $*/
#define D2_MUL3232TO64(A, B, RES)  *(RES) = ((A)) * (*(B))     /* PRQA S 3453 */ /* $Misra: #MACRO_TYPE_SAFE_FXN_REPL $*/
#define D2_MUL3264(A, B, RES)      *(RES) = ((A)) * (*(B))     /* PRQA S 3453 */ /* $Misra: #MACRO_TYPE_SAFE_FXN_REPL $*/
#define D2_DIV6432(DVD, DVS, RES)  *(RES) = (*(DVD)) / ((DVS)) /* PRQA S 3453 */ /* $Misra: #MACRO_TYPE_SAFE_FXN_REPL $*/
#define D2_SHIFTLEFT64(V, I, RES)  *(RES) = ((*(V)) << (I))    /* PRQA S 3453 */ /* $Misra: #MACRO_TYPE_SAFE_FXN_REPL $*/
#define D2_SHIFTRIGHT64(V, I, RES) *(RES) = ((*(V)) >> (I))    /* PRQA S 3453 */ /* $Misra: #MACRO_TYPE_SAFE_FXN_REPL $*/
#endif


/*---------------------------------------------------------------------------
 * Display List entry macro (moved from dlist.h because of include dependencies) */

static D2_INLINE void d2_add2dlist_intern( d2_devicedata *handle, d2_u32 regIdx, d2_s32 value )  /* PRQA S 3450, 3480, 3219 */ /* $Misra: #UTIL_INLINE_FUNC $*/
{
   d2_dlist_scratch_entry *entry = handle->dlscratch_pos;

   entry->reg   = regIdx;
   entry->value = value;

   handle->dlscratch_pos++;
   handle->dlscratch_cnt--;
   if(handle->dlscratch_cnt <= 0)
   {
      handle->dlscratch_hook( (d2_device*) handle );
   }
}

/*--------------------------------------------------------------------------- */
#endif
