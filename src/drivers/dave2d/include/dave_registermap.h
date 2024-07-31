/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_registermap.h (%version: 18 %)
 *          created Wed Jan 19 11:12:21 2005 by hh04027
 *
 * Description:
 *  %date_modified: Mon Oct 30 09:37:29 2006 %  (%derived_by:  hh74036 %)
 *
 * Changes:
 *  2006-02-16 CSe  added cache control bits for burstmodes
 *  2006-02-28 CSe  removed cache control bits for burstmodes + big endian
 *  2006-03-08 CSe  extended texture format selection from 2 to 3 bits
 *  2006-04-13 CSe  added bitmasks for hardware feature bits
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2008-05-26 MRe  changed bitnumbering (now starting at 0); new texture formats
 *  2009-01-16 MRe  moved defines for feature bits to dave_driver.h 
 *  2010-02-18 MRe  added Control2 bits for Alpha channel blending
 *  2012-09-25 BSp  MISRA cleanup
 *  2029-02-05 MRe  added Control3 register for max burst lengths
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_registermap_h_H
#define __1_dave_registermap_h_H
/*--------------------------------------------------------------------------- */

#define D2_STATUS     0
#define D2_HWREVISION 1


#define D2_CONTROL    0u      /* controlword1 */
#define D2_CONTROL2   1u      /* controlword2 */
#define D2_CONTROL3   2u      /* controlword3 */

#define D2_L1START    4       /* limiter 1-4 */
#define D2_L2START    5
#define D2_L3START    6
#define D2_L4START    7
#define D2_L5START    8
#define D2_L6START    9
#define D2_L1XADD    10
#define D2_L2XADD    11
#define D2_L3XADD    12
#define D2_L4XADD    13
#define D2_L5XADD    14
#define D2_L6XADD    15
#define D2_L1YADD    16
#define D2_L2YADD    17
#define D2_L3YADD    18
#define D2_L4YADD    19
#define D2_L5YADD    20
#define D2_L6YADD    21

#define D2_L1BAND    22       /* fixedpoint bandfilter1 value */
#define D2_L2BAND    23       /* fixedpoint bandfilter2 value */

#define D2_COLOR1    25       /* constant color1 & constant alpha1 */
#define D2_COLOR2    26       /* constant color2 & constant alpha2 */
#define D2_PATTERN   29       /* pattern (0bit=color1, 1bit=color2) */

#define D2_SIZE      30       /* highword : height in pixels,  lowword : width in pixels */
#define D2_PITCH     31       /* highword : spanstore delay in lines,   lowword : pitch in bytes */
#define D2_ORIGIN    32       /* framebffer address of first enum pixel */

   /* reserved 33 - framebuffer minimum address
    * reserved 34 - framebuffer maximum address */

#define D2_LUSTART      36       /* texture limiter1 */
#define D2_LUXADD       37
#define D2_LUYADD       38
#define D2_LVSTARTI     39       /* texture limiter2 */
#define D2_LVSTARTF     40
#define D2_LVXADDI      41
#define D2_LVYADDI      42
#define D2_LVYXADDF     43       /* highword : fractionpart for vyadd , lowword : fractionpart for vxadd */

#define D2_TEXPITCH     45       /* texels per texture line */
                                 /* bits 31-11 : (height-1) * pitch,   bits 10-0 : (width-1) */
#define D2_TEXMASK      46       /* bits 31-11 : Vcordinate mask,   bits 10-0 : Ucordinate mask */
#define D2_TEXORIGIN    47       /*  address of first texture pixel (writing can trigger texturecache flush) */

#define D2_IRQCTL       48        /* dave interrupt control */

#define D2_CACHECTL     49       /* dave cache control (flush) */
#define D2_DLISTSTART   50       /* display list start (writing triggers dlist execution) */

#define D2_PERFCOUNT1   51       /* performance counter 1 (read/reset) */
#define D2_PERFCOUNT2   52       /* performance counter 2 (read/reset) */
#define D2_PERFTRIGGER  53       /* performance counter source */

#define D2_TEXCLUT      54       /* AI44 write index and data */
#define D2_TEXCLUT_ADDR 55       /* clut write address for CLUT256*/
#define D2_TEXCLUT_DATA 56       /* clut write data for CLUT256*/
#define D2_TEXCLUT_OFFSET 57     /* clut offset for CLUT256*/

#define D2_COLKEY       58       /* color for color keying*/

#define D2_QUANTITY     59       /* end of registermap (must be < 128)*/


/*---------------------------------------------------------------------------
 * control word bits */

#ifndef BIT
#define BIT(x)          (1u<<(x))
#endif

#define D2C_LIM1ENABLE     BIT(0)      /* limiter 1 enable */
#define D2C_LIM2ENABLE     BIT(1)      /* limiter 2 enable */
#define D2C_LIM3ENABLE     BIT(2)      /* limiter 3 enable */
#define D2C_LIM4ENABLE     BIT(3)      /* limiter 4 enable */
#define D2C_LIM5ENABLE     BIT(4)      /* limiter 5 enable */
#define D2C_LIM6ENABLE     BIT(5)      /* limiter 6 enable */

#define D2C_QUAD1ENABLE    BIT(6)      /* quadratic coupling 1 enable */
#define D2C_QUAD2ENABLE    BIT(7)      /* quadratic coupling 2 enable */
#define D2C_QUAD3ENABLE    BIT(8)      /* quadratic coupling 3 enable */

#define D2C_LIM1THRESHOLD  BIT(9)      /* limiter 1 threshold enable */
#define D2C_LIM2THRESHOLD  BIT(10)     /* limiter 2 threshold enable */
#define D2C_LIM3THRESHOLD  BIT(11)     /* limiter 3 threshold enable */
#define D2C_LIM4THRESHOLD  BIT(12)     /* limiter 4 threshold enable */
#define D2C_LIM5THRESHOLD  BIT(13)     /* limiter 5 threshold enable */
#define D2C_LIM6THRESHOLD  BIT(14)     /* limiter 6 threshold enable */

#define D2C_BAND1ENABLE    BIT(15)     /* bandfilter 1 enable */
#define D2C_BAND2ENABLE    BIT(16)     /* bandfilter 2 enable */

#define D2C_UNION12        BIT(17)     /* combine 1/2 as union (+) otherwise intersect (*) */
#define D2C_UNION34        BIT(18)     /* combine 3/4 as union (+) otherwise intersect (*) */
#define D2C_UNION56        BIT(19)     /* combine 5/6 as union (+) otherwise intersect (*) */
#define D2C_UNIONAB        BIT(20)     /* combine (12)/(34) as union */
#define D2C_UNIONCD        BIT(21)     /* combine (12)/(34) as union */

#define D2C_SPANABORT      BIT(22)     /* enable span abort */
#define D2C_SPANSTORE      BIT(23)     /* enable span store (does not work without span abort) */

#define D2C_LIMITERPRECISION  BIT(24)     /* limiter precision is increased by 6 (10.22) */

/* controlword2 bits */

#define D2C_PATTERNENABLE    BIT(0)    /* render using pattern */
#define D2C_TEXTUREENABLE    BIT(1)    /* render using texture */
#define D2C_PATTERNSOURCEL5  BIT(2)    /* pattern source limiter 5 */

#define D2C_BSF            BIT(9)      /* Blend Source Factor (alpha when set, 1 otherwise) */
#define D2C_BDF            BIT(10)     /* Blend Destination Factor (alpha when set, 1 otherwise) */
#define D2C_BSI            BIT(11)     /* Blend Source Invert (a = 1-a when set) */
#define D2C_BDI            BIT(12)     /* Blend Destination Invert (a = 1-a when set) */
#define D2C_BC2            BIT(13)     /* Blend Color2 (Blend using Color2 as Dest when set) */

#define D2C_TEXTURECLAMPX  BIT(14)     /* texture u axis clamping */
#define D2C_TEXTURECLAMPY  BIT(15)     /* texture v axis clamping */

#define D2C_TEXTUREFILTERX BIT(16)     /* Bilinear filtering */
#define D2C_TEXTUREFILTERY BIT(17)     /* Bilinear filtering */

#define D2C_READFORMAT1    BIT(18)     /* Textureformat bit0 */
#define D2C_READFORMAT2    BIT(19)     /* Textureformat bit1 */
#define D2C_READFORMAT3    BIT(4)      /* Textureformat bit2 ( 0 = alpha8, 1 = rgb565, 2 = argb8888, 3 = argb4444, 4 = argb1555 ) */
#define D2C_READFORMAT4    BIT(5)      /* Textureformat bit3 ( more formats )  */

#define D2C_WRITEFORMAT1   BIT(20)     /* Framebuffer format bit0 */
#define D2C_WRITEFORMAT2   BIT(21)     /* Framebuffer format bit1 ( 00 = alpha8, 01 = rgb565, 10 = argb8888, 11=argb4444) */
#define D2C_WRITEFORMAT3   BIT(8)      /* Framebuffer format bit2 ( 110 = rgba8888, 111=rgba4444) */

#define D2C_USE_ACB        BIT(3)     /* use alpha-blending instead of write-alpha for alpha channel (see D2C_WRITEALPHA) */
                                       /* the following bits are used if D2C_USE_ACB is not set: */
#define D2C_WRITEALPHA1    BIT(22)     /* Write framebuffer alpha (see also D2C_BSIA and D2C_BDIA)*/
#define D2C_WRITEALPHA2    BIT(23)     /* ( 11 = framebuffer.a, 00 = color2.a, 01 = coverage, 10 = ?) */
                                       /* the following bits are used if D2C_USE_ACB is set: */
#define D2C_BSIA           BIT(28)     /* Alpha channel blending: Blend Source Invert (a = 1-a when set */
#define D2C_BDIA           BIT(29)     /* Alpha channel blending: Blend Destination Invert (a = 1-a when set */
#define D2C_BSFA           BIT(6)      /* Alpha channel blending: Blend Source Factor (alpha when set, 1 otherwise) */
#define D2C_BDFA           BIT(7)      /* Alpha channel blending: Blend Destination Factor (alpha when set, 1 otherwise) */
/*#define D2C_BC2A           BIT(3)*/      /* Alpha channel blending: Blend Color2 (Blend using Alpha of Color2 as Dest when set) */

#define D2C_RLE_ENABLE     BIT(24)     /* enable RLE decoder */
#define D2C_CLUT_ENABLE    BIT(25)     /* enable looking up of indices */
#define D2C_COLKEY_ENABLE  BIT(26)     /* enable color keying */

#define D2C_CLUTFORMAT1    BIT(27)     /* CLUT entry format bit0 */


#define D2C_RFU_0          BIT(29)      /* reserved for future use */

#define D2C_RLEFORMAT1     BIT(30)     /* RLE format bit0 */
#define D2C_RLEFORMAT2     BIT(31)     /* RLE format bit1 00=1 byte per pixel; 01=2 byte per pixel; 10=3 byte per pixel; 11=4 byte per pixel */


/* Controlword1 bits */
#define D2C_BUSY_ENUM     BIT(0)    /* enumeration unit busy, cant start new primitive */
#define D2C_BUSY_WRITE    BIT(1)    /* framebuffer writeback busy, cant change fb type */
#define D2C_CACHE_DIRTY   BIT(2)    /* framebuffer cache dirty, cant flip frame */
#define D2C_DLISTACTIVE   BIT(3)    /* display list active, cant direct access hwregs */
#define D2C_IRQ_ENUM      BIT(4)    /* IRQ on enumeration finish (bit0 low) */
#define D2C_IRQ_DLIST     BIT(5)    /* IRQ on display list finish (bit3 low) */

/* Interruptbits */
#define D2IRQCTL_ENABLE_FINISH_ENUM  BIT(0) /* Interruptmask enable "Enumeration is finished" */
#define D2IRQCTL_ENABLE_FINISH_DLIST BIT(1) /* Interruptmask enable "Displaylist is finished" */
#define D2IRQCTL_CLR_FINISH_ENUM     BIT(2) /* Clear Interrupt "Enumeration is finished" */
#define D2IRQCTL_CLR_FINISH_DLIST    BIT(3) /* Clear Interrupt "Displaylist is finished" */

/* Cache control bits */
#define D2C_CACHECTL_ENABLE_FB       BIT(0)
#define D2C_CACHECTL_FLUSH_FB        BIT(1)
#define D2C_CACHECTL_ENABLE_TX       BIT(2)
#define D2C_CACHECTL_FLUSH_TX        BIT(3)

/* Performance counter source enum (D2_PERFTRIGGER) */
#define D2PC_NONE     0
#define D2PC_DAVECYCLES     1
#define D2PC_FBREADS        2
#define D2PC_FBWRITES       3
#define D2PC_TXREADS        4
#define D2PC_INVPIXELS      5
#define D2PC_INVPIXELS_MISS 6
#define D2PC_DLRCYCLES      7
#define D2PC_FBREADHITS     8
#define D2PC_FBREADMISSES   9
#define D2PC_FBWRITEHITS   10
#define D2PC_FBWRITEMISSES 11
#define D2PC_TXREADHITS    12
#define D2PC_TXREADMISSES  13
#define D2PC_CPUDATAREADS  14
#define D2PC_CPUDATAWRITES 15
#define D2PC_CPUINSTRREADS 16
#define D2PC_RLEREWINDS    20
#define D2PC_CYCLES        31

/* Hardware revision feature bits */
/*
#define D2FB_SWDAVE     BIT(16)
#define D2FB_DLR        BIT(17)
#define D2FB_FBCACHE    BIT(18)
#define D2FB_TXCACHE    BIT(19)
#define D2FB_PERFCOUNT  BIT(20)
#define D2FB_TEXCLUT    BIT(21)
#define D2FB_FBPREFETCH BIT(22)
#define D2FB_RLEUNIT    BIT(23)
#define D2FB_TEXCLUT256 BIT(24)
#define D2FB_COLORKEY   BIT(25)
#define D2FB_HILIMITERPRECISION   BIT(26)
#define D2FB_ALPHACHANNELBLENDING   BIT(27)
*/

/*--------------------------------------------------------------------------- */
#endif
