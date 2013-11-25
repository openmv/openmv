#ifndef __OV9650_H__
#define __OV9650_H__
#include <stdint.h>
#include "imlib.h"
enum ov9650_pixformat { 
    PIXFORMAT_RGB565, /* 2BPP/RGB565*/
    PIXFORMAT_YUV422, /* 2BPP/YUV422*/
};

enum ov9650_framesize { 
    FRAMESIZE_SXGA, /* 1280x1024 */
    FRAMESIZE_VGA,  /* 640x480   */
    FRAMESIZE_CIF,  /* 352x288   */
    FRAMESIZE_QVGA, /* 320x240   */
    FRAMESIZE_QCIF, /* 176x144   */
    FRAMESIZE_QQVGA,/* 160x120   */
    FRAMESIZE_QQCIF /* 88x72     */
};

enum ov9650_framerate { 
    FRAMERATE_2FPS =0x9F,
    FRAMERATE_8FPS =0x87,
    FRAMERATE_15FPS=0x83,
    FRAMERATE_30FPS=0x81,
    FRAMERATE_60FPS=0x80,
};

enum ov9650_command { 
    CMD_SNAPSHOT=1,
    CMD_COLOR_TRACK, 
    CMD_MOTION_DETECTION,
    CMD_SET_PIXFORMAT,
    CMD_SET_FRAMERATE,
    CMD_SET_FRAMESIZE,
    CMD_FACE_DETECTION,
};

struct ov9650_id {
    uint8_t MIDH;
    uint8_t MIDL;
    uint8_t PID;
    uint8_t VER;
};

struct ov9650_handle {
    struct ov9650_id id;
    enum ov9650_pixformat pixformat;
    enum ov9650_framesize framesize;
    enum ov9650_framerate framerate;
    struct frame_buffer frame_buffer;
};

/* Hardware initialization */
int ov9650_init(struct ov9650_handle *ov9650);
/* Reset sensor */
void ov9650_reset(struct ov9650_handle *ov9650);
int ov9650_set_pixformat(struct ov9650_handle *ov9650, enum ov9650_pixformat pixformat);
int ov9650_set_framesize(struct ov9650_handle *ov9650, enum ov9650_framesize framesize);
int ov9650_set_framerate(struct ov9650_handle *ov9650, enum ov9650_framerate framerate);
int ov9650_set_brightness(struct ov9650_handle *ov9650, int level);
int ov9650_set_exposure(struct ov9650_handle *ov9650, uint16_t exposure);
int ov9650_snapshot(struct ov9650_handle *ov9650);
void delay(uint32_t ntime);
#endif /* __OV9650_H__ */
