#ifndef __SENSOR_H__
#define __SENSOR_H__
#include <stdint.h>
#include "imlib.h"
struct sensor_id {
    uint8_t MIDH;
    uint8_t MIDL;
    uint8_t PID;
    uint8_t VER;
};

enum sensor_pixformat { 
    PIXFORMAT_RGB565=1,  /* 2BPP/RGB565*/
    PIXFORMAT_YUV422,    /* 2BPP/YUV422*/
    PIXFORMAT_GRAYSCALE, /* 1BPP/GRAYSCALE*/
};

enum sensor_framesize { 
    FRAMESIZE_QQCIF=1,  /* 88x72     */
    FRAMESIZE_QQVGA,    /* 160x120   */
    FRAMESIZE_QCIF,     /* 176x144   */
    FRAMESIZE_QVGA,     /* 320x240   */
    FRAMESIZE_CIF,      /* 352x288   */
    FRAMESIZE_VGA,      /* 640x480   */
    FRAMESIZE_SXGA,     /* 1280x1024 */
};

enum sensor_framerate { 
    FRAMERATE_2FPS =0x9F,
    FRAMERATE_8FPS =0x87,
    FRAMERATE_15FPS=0x83,
    FRAMERATE_30FPS=0x81,
    FRAMERATE_60FPS=0x80,
};

enum sensor_command { 
    CMD_RESET_SENSOR=1,
    CMD_SET_PIXFORMAT,
    CMD_SET_FRAMERATE,
    CMD_SET_FRAMESIZE,
    CMD_SET_BRIGHTNESS,
    CMD_WRITE_REGISTER,
    CMD_READ_REGISTER,
    CMD_SNAPSHOT,
    CMD_COLOR_TRACK, 
    CMD_MOTION_DETECTION,
    CMD_FACE_DETECTION,
};

enum sensor_result { 
    CMD_ACK  =0x01,
    CMD_NACK =0x02,
};

struct sensor_dev {
    struct sensor_id id;
    enum sensor_pixformat pixformat;
    enum sensor_framesize framesize;
    enum sensor_framerate framerate;
    struct frame_buffer frame_buffer;
    /* Sensor function pointers */
    int  (*reset)          ();
    int  (*set_pixformat)  (enum sensor_pixformat pixformat);
    int  (*set_framesize)  (enum sensor_framesize framesize);
    int  (*set_framerate)  (enum sensor_framerate framerate);
    int  (*set_brightness) (uint8_t level);
    int  (*set_exposure)   (uint16_t exposure);
};

/**
 * Initialize the sensor.
 * This function will initialize SCCB and XCLK, and will attempt to detect 
 * the connected sensor. If a sensor supported sensor is detected, its driver will be used.
 * 
 * @param sensor A pointer to the sensor device handle.
 * @return On success, 0 is returned. If the sensor is not supported, or not detected, -1 is returned.
 */
int sensor_init(struct sensor_dev *sensor);
/**
 * Reset the sensor to its default state.
 *
 * @param sensor A pointer to the sensor device handle.
 * @return On success, 0 is returned. If the sensor is not supported, or not detected, -1 is returned.
 */
int sensor_reset(struct sensor_dev *sensor);
/**
 * Read a sensor register.
 *
 * @param sensor A pointer to the sensor device handle.
 * @param reg    Register address.
 * @return On success, the regsiter value is returned. Otherwise, -1 is returned.
 */
int sensor_read_reg(struct sensor_dev *sensor, uint8_t reg);
/**
 * Write a sensor register.
 *
 * @param sensor A pointer to the sensor device handle.
 * @param reg Register address.
 * @param val Register value.
 * @return On success, 0 is returned. Otherwise, -1 is returned.
 */
int sensor_write_reg(struct sensor_dev *sensor, uint8_t reg, uint8_t val);
/**
 * Capture a Snapshot.
 *
 * @param sensor A pointer to the sensor device handle.
 * @return  On success, 0 is returned. If the format is not supported by this sensor, -1 is returned.
 */
int sensor_snapshot(struct sensor_dev *sensor);
/**
 * Set the sensor pixel format.
 *
 * @see   sensor_pixelformat. 
 * @param sensor A pointer to the sensor device handle.
 * @param pixformat The new pixel format.
 * @return  On success, 0 is returned. If the operation not supported by the sensor, -1 is returned.
 */
int sensor_set_pixformat(struct sensor_dev *sensor, enum sensor_pixformat pixformat);
/**
 * Set the sensor frame size.
 *
 * @see   sensor_framesize. 
 * @param sensor A pointer to the sensor device handle.
 * @param framesize The new frame size.
 * @return  On success, 0 is returned. If the operation not supported by the sensor, -1 is returned.
 */
int sensor_set_framesize(struct sensor_dev *sensor, enum sensor_framesize framesize);
/**
 * Set the sensor frame rate.
 *
 * @see   sensor_framerate. 
 * @param sensor A pointer to the sensor device handle.
 * @param pixformat The new frame rate.
 * @return  On success, 0 is returned. If the operation not supported by the sensor, -1 is returned.
 */
int sensor_set_framerate(struct sensor_dev *sensor, enum sensor_framerate framerate);
/**
 * Set the sensor brightness level.
 *
 * @param sensor A pointer to the sensor device handle.
 * @param level The new brightness level allowed values from -3 to +3.
 * @return  On success, 0 is returned. If the operation not supported by the sensor, -1 is returned.
 */
int sensor_set_brightness(struct sensor_dev *sensor, uint8_t level);
/**
 * Set the sensor exposure level. This function has no
 * effect when AEC (Automatic Exposure Control) is enabled.
 *
 * @param sensor A pointer to the sensor device handle.
 * @param exposure The new exposure level.
 * @return  On success, 0 is returned. If the operation not supported by the sensor, -1 is returned.
 */
int sensor_set_exposure(struct sensor_dev *sensor, uint16_t exposure);
#endif /* __SENSOR_H__ */
