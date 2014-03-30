#include <libmp.h>
#include "sensor.h"
#include "sccb.h"
#include "py_sensor.h"
#include "py_image.h"

static mp_obj_t py_sensor_reset() {
    sensor_reset();
    sensor_set_pixformat(PIXFORMAT_RGB565);
    sensor_set_framesize(FRAMESIZE_QQVGA);
    sensor_set_framerate(FRAMERATE_30FPS);
    sensor_set_gainceiling(GAINCEILING_8X);
    sensor_set_brightness(-2);
    return mp_const_none;
}

static mp_obj_t py_sensor_snapshot() {
    mp_obj_t image = py_image(0, 0, 0, 0);
    sensor_snapshot((struct image*) py_image_cobj(image));
    return image;
}

static mp_obj_t py_sensor_set_pixformat(mp_obj_t pixformat) {
    if (sensor_set_pixformat(mp_obj_get_int(pixformat)) != 0) {
        return mp_const_false;
    }
    return mp_const_true;
}

static mp_obj_t py_sensor_set_framerate(mp_obj_t framerate) {
    if (sensor_set_framerate(mp_obj_get_int(framerate)) != 0) {
        return mp_const_false;
    }
    return mp_const_true;
}

static mp_obj_t py_sensor_set_framesize(mp_obj_t framesize) {
    if (sensor_set_framesize(mp_obj_get_int(framesize)) != 0) {
        return mp_const_false;
    }
    return mp_const_true;
}

static mp_obj_t py_sensor_set_gainceiling(mp_obj_t gainceiling) {
    if (sensor_set_gainceiling(mp_obj_get_int(gainceiling)) != 0) {
        return mp_const_false;
    }
    return mp_const_true;
}

static mp_obj_t py_sensor_set_brightness(mp_obj_t brightness) {
    if (sensor_set_brightness(mp_obj_get_int(brightness)) != 0) {
        return mp_const_false;
    }
    return mp_const_true;
}

static mp_obj_t py_sensor_write_reg(mp_obj_t addr, mp_obj_t val) {
    SCCB_Write(mp_obj_get_int(addr), mp_obj_get_int(val));
    return mp_const_none;
}

static mp_obj_t py_sensor_read_reg(mp_obj_t addr) {
    return mp_obj_new_int(SCCB_Read(mp_obj_get_int(addr)));
}

static void py_sensor_print(void (*print)(void *env, const char *fmt, ...),
                            void *env, mp_obj_t self_in, mp_print_kind_t kind) {
//        printf(env, "<Sensor MID:0x%.2X%.2X PID:0x%.2X VER:0x%.2X>",
  //          sensor.id.MIDH, sensor.id.MIDL, sensor.id.PID, sensor.id.VER);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_sensor_reset_obj,           py_sensor_reset);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_sensor_snapshot_obj,        py_sensor_snapshot);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_sensor_set_pixformat_obj,   py_sensor_set_pixformat);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_sensor_set_framerate_obj,   py_sensor_set_framerate);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_sensor_set_framesize_obj,   py_sensor_set_framesize);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_sensor_set_gainceiling_obj, py_sensor_set_gainceiling);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_sensor_set_brightness_obj,  py_sensor_set_brightness);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_sensor_write_reg_obj,       py_sensor_write_reg);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_sensor_read_reg_obj,        py_sensor_read_reg);

STATIC const mp_map_elem_t module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),    MP_OBJ_NEW_QSTR(MP_QSTR_sensor) },
    /* Pixel format */
    { MP_OBJ_NEW_QSTR(MP_QSTR_RGB565),      MP_OBJ_NEW_SMALL_INT(PIXFORMAT_RGB565)},   /* 2BPP/RGB565*/
    { MP_OBJ_NEW_QSTR(MP_QSTR_YUV422),      MP_OBJ_NEW_SMALL_INT(PIXFORMAT_YUV422)},   /* 2BPP/YUV422*/
    { MP_OBJ_NEW_QSTR(MP_QSTR_GRAYSCALE),   MP_OBJ_NEW_SMALL_INT(PIXFORMAT_GRAYSCALE)},/* 1BPP/GRAYSCALE*/
//  { MP_OBJ_NEW_QSTR(MP_QSTR_JPEG),        MP_OBJ_NEW_SMALL_INT(PIXFORMAT_JPEG)},     /* JPEG/COMPRESSED*/
    /* Frame size */
    { MP_OBJ_NEW_QSTR(MP_QSTR_QQCIF),       MP_OBJ_NEW_SMALL_INT(FRAMESIZE_QQCIF)},    /* 88x72     */
    { MP_OBJ_NEW_QSTR(MP_QSTR_QQVGA),       MP_OBJ_NEW_SMALL_INT(FRAMESIZE_QQVGA)},    /* 160x120   */
    { MP_OBJ_NEW_QSTR(MP_QSTR_QCIF),        MP_OBJ_NEW_SMALL_INT(FRAMESIZE_QCIF)},     /* 176x144   */
    { MP_OBJ_NEW_QSTR(MP_QSTR_QVGA),        MP_OBJ_NEW_SMALL_INT(FRAMESIZE_QVGA)},     /* 320x240   */
    { MP_OBJ_NEW_QSTR(MP_QSTR_CIF),         MP_OBJ_NEW_SMALL_INT(FRAMESIZE_CIF)},      /* 352x288   */
    { MP_OBJ_NEW_QSTR(MP_QSTR_VGA),         MP_OBJ_NEW_SMALL_INT(FRAMESIZE_VGA)},      /* 640x480   */
    { MP_OBJ_NEW_QSTR(MP_QSTR_SXGA),        MP_OBJ_NEW_SMALL_INT(FRAMESIZE_SXGA)},     /* 1280x1024 */

    /* Sensor functions */
    { MP_OBJ_NEW_QSTR(MP_QSTR_reset),           (mp_obj_t)&py_sensor_reset_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_snapshot),        (mp_obj_t)&py_sensor_snapshot_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_pixformat),   (mp_obj_t)&py_sensor_set_pixformat_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_framerate),   (mp_obj_t)&py_sensor_set_framerate_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_framesize),   (mp_obj_t)&py_sensor_set_framesize_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_gainceiling), (mp_obj_t)&py_sensor_set_gainceiling_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_brightness),  (mp_obj_t)&py_sensor_set_brightness_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR___write_reg),     (mp_obj_t)&py_sensor_write_reg_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR___read_reg),      (mp_obj_t)&py_sensor_read_reg_obj },
};

STATIC const mp_map_t module_globals = {
    .all_keys_are_qstrs = 1,
    .table_is_fixed_array = 1,
    .used  = sizeof(module_globals_table) / sizeof(mp_map_elem_t),
    .alloc = sizeof(module_globals_table) / sizeof(mp_map_elem_t),
    .table = (mp_map_elem_t*)module_globals_table,
};

static const mp_obj_module_t py_sensor_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_sensor,
    .globals = (mp_map_t*)&module_globals,
};

const mp_obj_module_t *py_sensor_init()
{
    /* Init sensor */
    if (sensor_init() != 0) {
        return NULL;
    }

    /* Reset sensor and registers */
    sensor_reset();

    /* Use some default settings */
    sensor_set_pixformat(PIXFORMAT_RGB565);
    sensor_set_framesize(FRAMESIZE_QQVGA);
    sensor_set_framerate(FRAMERATE_30FPS);
    sensor_set_gainceiling(GAINCEILING_8X);
    sensor_set_brightness(-2);

    return &py_sensor_module;
}
