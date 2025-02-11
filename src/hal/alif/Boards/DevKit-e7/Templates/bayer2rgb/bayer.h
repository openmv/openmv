#include <stdint.h>

typedef enum {
    DC1394_BAYER_METHOD_NEAREST=0,
    DC1394_BAYER_METHOD_SIMPLE,
    DC1394_BAYER_METHOD_BILINEAR,
    DC1394_BAYER_METHOD_HQLINEAR,
    DC1394_BAYER_METHOD_DOWNSAMPLE,
    DC1394_BAYER_METHOD_EDGESENSE,
    DC1394_BAYER_METHOD_VNG,
    DC1394_BAYER_METHOD_AHD
} dc1394bayer_method_t;

typedef enum {
    DC1394_COLOR_FILTER_RGGB = 512,
    DC1394_COLOR_FILTER_GBRG,
    DC1394_COLOR_FILTER_GRBG,
    DC1394_COLOR_FILTER_BGGR
} dc1394color_filter_t ;
#define DC1394_COLOR_FILTER_MIN        DC1394_COLOR_FILTER_RGGB
#define DC1394_COLOR_FILTER_MAX        DC1394_COLOR_FILTER_BGGR
#define DC1394_COLOR_FILTER_NUM       (DC1394_COLOR_FILTER_MAX - DC1394_COLOR_FILTER_MIN + 1)

/**
 * Error codes returned by most libdc1394 functions.
 *
 * General rule: 0 is success, negative denotes a problem.
 */
typedef enum {
    DC1394_SUCCESS                     =  0,
    DC1394_FAILURE                     = -1,
    DC1394_NOT_A_CAMERA                = -2,
    DC1394_FUNCTION_NOT_SUPPORTED      = -3,
    DC1394_CAMERA_NOT_INITIALIZED      = -4,
    DC1394_MEMORY_ALLOCATION_FAILURE   = -5,
    DC1394_TAGGED_REGISTER_NOT_FOUND   = -6,
    DC1394_NO_ISO_CHANNEL              = -7,
    DC1394_NO_BANDWIDTH                = -8,
    DC1394_IOCTL_FAILURE               = -9,
    DC1394_CAPTURE_IS_NOT_SET          = -10,
    DC1394_CAPTURE_IS_RUNNING          = -11,
    DC1394_RAW1394_FAILURE             = -12,
    DC1394_FORMAT7_ERROR_FLAG_1        = -13,
    DC1394_FORMAT7_ERROR_FLAG_2        = -14,
    DC1394_INVALID_ARGUMENT_VALUE      = -15,
    DC1394_REQ_VALUE_OUTSIDE_RANGE     = -16,
    DC1394_INVALID_FEATURE             = -17,
    DC1394_INVALID_VIDEO_FORMAT        = -18,
    DC1394_INVALID_VIDEO_MODE          = -19,
    DC1394_INVALID_FRAMERATE           = -20,
    DC1394_INVALID_TRIGGER_MODE        = -21,
    DC1394_INVALID_TRIGGER_SOURCE      = -22,
    DC1394_INVALID_ISO_SPEED           = -23,
    DC1394_INVALID_IIDC_VERSION        = -24,
    DC1394_INVALID_COLOR_CODING        = -25,
    DC1394_INVALID_COLOR_FILTER        = -26,
    DC1394_INVALID_CAPTURE_POLICY      = -27,
    DC1394_INVALID_ERROR_CODE          = -28,
    DC1394_INVALID_BAYER_METHOD        = -29,
    DC1394_INVALID_VIDEO1394_DEVICE    = -30,
    DC1394_INVALID_OPERATION_MODE      = -31,
    DC1394_INVALID_TRIGGER_POLARITY    = -32,
    DC1394_INVALID_FEATURE_MODE        = -33,
    DC1394_INVALID_LOG_TYPE            = -34,
    DC1394_INVALID_BYTE_ORDER          = -35,
    DC1394_INVALID_STEREO_METHOD       = -36,
    DC1394_BASLER_NO_MORE_SFF_CHUNKS   = -37,
    DC1394_BASLER_CORRUPTED_SFF_CHUNK  = -38,
    DC1394_BASLER_UNKNOWN_SFF_CHUNK    = -39
} dc1394error_t;
#define DC1394_ERROR_MIN  DC1394_BASLER_UNKNOWN_SFF_CHUNK
#define DC1394_ERROR_MAX  DC1394_SUCCESS
#define DC1394_ERROR_NUM (DC1394_ERROR_MAX-DC1394_ERROR_MIN+1)

typedef enum {
    DC1394_FALSE= 0,
    DC1394_TRUE
} dc1394bool_t;




dc1394error_t
dc1394_bayer_decoding_8bit(const uint8_t * bayer, uint8_t * rgb, uint32_t sx, uint32_t sy, dc1394color_filter_t tile, dc1394bayer_method_t method);

dc1394error_t
dc1394_bayer_decoding_16bit(const uint16_t * bayer, uint16_t * rgb, uint32_t sx, uint32_t sy, dc1394color_filter_t tile, dc1394bayer_method_t method, uint32_t bits);
