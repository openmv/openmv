/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_videomodes.h (%version: 8 %)
 *          created Tue Aug 23 10:32:45 2005 by hh04027
 *
 * Description:
 *  %date_modified: Fri Oct 27 11:24:15 2006 %  (%derived_by:  hh74026 %)
 *
 * Changes:
 *  2006-01-04 CSe  - changed defines for video modes (now using struct)
 *  2006-01-13 NSc  - combined ID and descriptor for video modes
 *  2007-10-18 ASc  - move structs to dave_base to allow compilation 
 *                    without display driver
 *  2008-01-14 ASc  - removed tabs, changed C++ to C comments, added
 *                    new videomode for Ravin platform (320x240)
 * */

/*--------------------------------------------------------------------------
 * Video modes for display controller.
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_videomodes_h_H
#define __1_dave_videomodes_h_H
/*--------------------------------------------------------------------------- */

#define D1_VIDEO_OFF                0
#define D1_VIDEO_DEFAULT            1
#define D1_VIDEO_640_400_60         2
#define D1_VIDEO_640_480_60         3
#define D1_VIDEO_800_600_60         4
#define D1_VIDEO_480_234_60         5
#define D1_VIDEO_800_480_60         6
#define D1_VIDEO_320_240_60         7
#define D1_VIDEO_1024_1024_60       8
#define D1_VIDEO_1280_1024_60       9
#define D1_VIDEO_1920_1080_60      10
#define D1_VIDEO_1920_1200_60      11
#define D1_VIDEO_480_480_60        12

/*---------------------------------------------------------------------------
 * each entry fills a d1_videomode struct (see below) */

#define D1_VIDEODESC_OFF               { D1_VIDEO_OFF, 0, 0, 0 }
#define D1_VIDEODESC_640_400_60        { D1_VIDEO_640_400_60, 640, 400, 60 }
#define D1_VIDEODESC_640_480_60        { D1_VIDEO_640_480_60, 640, 480, 60 }
#define D1_VIDEODESC_800_600_60        { D1_VIDEO_800_600_60, 800, 600, 60 }
#define D1_VIDEODESC_480_234_60        { D1_VIDEO_480_234_60, 480, 234, 60 }
#define D1_VIDEODESC_800_480_60        { D1_VIDEO_800_480_60, 800, 480, 60 }
#define D1_VIDEODESC_320_240_60        { D1_VIDEO_320_240_60, 320, 240, 60 }
#define D1_VIDEODESC_1024_1024_60      { D1_VIDEO_1024_1024_60, 1024, 1024, 60 }
#define D1_VIDEODESC_1280_1024_60      { D1_VIDEO_1280_1024_60, 1280, 1024, 60 }
#define D1_VIDEODESC_1920_1080_60      { D1_VIDEO_1920_1080_60, 1920, 1080, 60 }
#define D1_VIDEODESC_1920_1200_60      { D1_VIDEO_1920_1200_60, 1920, 1200, 60 }
#define D1_VIDEODESC_480_480_60        { D1_VIDEO_480_480_60, 480, 480, 60 }


/*--------------------------------------------------------------------------- */
#endif

