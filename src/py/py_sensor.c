#include <libmp.h>
#include "sensor.h"
#include "sccb.h"
#include "py_sensor.h"
#include "py_image.h"

struct sym_entry {
    const char *sym;
    int val;
};

static struct sym_entry pixformat_constants[] = {
    {"RGB565",      PIXFORMAT_RGB565},    /* 2BPP/RGB565*/
    {"YUV422",      PIXFORMAT_YUV422},    /* 2BPP/YUV422*/
    {"GRAYSCALE",   PIXFORMAT_GRAYSCALE}, /* 1BPP/GRAYSCALE*/
    {NULL, 0}
};

static struct sym_entry framesize_constants[] = {
    {"QQCIF",   FRAMESIZE_QQCIF},    /* 88x72     */
    {"QQVGA",   FRAMESIZE_QQVGA},    /* 160x120   */
    {"QCIF",    FRAMESIZE_QCIF},     /* 176x144   */
    {"QVGA",    FRAMESIZE_QVGA},     /* 320x240   */
    {"CIF",     FRAMESIZE_CIF},      /* 352x288   */
    {"VGA",     FRAMESIZE_VGA},      /* 640x480   */
    {"SXGA",    FRAMESIZE_SXGA},     /* 1280x1024 */
    {NULL, 0}
};

static struct sym_entry framerate_constants[] = {
    {"FPS2",    FRAMERATE_2FPS },
    {"FPS8",    FRAMERATE_8FPS },
    {"FPS15",   FRAMERATE_15FPS},
    {"FPS30",   FRAMERATE_30FPS},
    {"FPS60",   FRAMERATE_60FPS},
    {NULL, 0}
};

static struct sym_entry gainceiling_constants[] = {
    {"X2",      GAINCEILING_2X},
    {"X4",      GAINCEILING_4X},
    {"X8",      GAINCEILING_8X},
    {"X16",     GAINCEILING_16X},
    {"X32",     GAINCEILING_32X},
    {"X64",     GAINCEILING_64X},
    {"X128",    GAINCEILING_128X},
    {NULL, 0}
};

mp_obj_t py_sensor_sinit() {
    sensor_init();
    /* hack */
    sensor_reset();
    sensor_set_pixformat(PIXFORMAT_RGB565);
    sensor_set_framesize(FRAMESIZE_QQVGA);
    sensor_set_framerate(FRAMERATE_30FPS);
    sensor_set_gainceiling(GAINCEILING_8X);
    sensor_set_brightness(-2);
    return mp_const_none;
}

mp_obj_t py_sensor_reset() {
    sensor_reset();
    return mp_const_none;
}

mp_obj_t py_sensor_snapshot() {
    mp_obj_t image = py_image(0, 0, 0, 0);
    sensor_snapshot((struct image*) py_image_cobj(image));
    return image;
}

mp_obj_t py_sensor_set_pixformat(mp_obj_t pixformat) {
    if (sensor_set_pixformat(mp_obj_get_int(pixformat)) != 0) {
        return mp_const_false;
    }
    return mp_const_true;
}

mp_obj_t py_sensor_set_framerate(mp_obj_t framerate) {
    if (sensor_set_framerate(mp_obj_get_int(framerate)) != 0) {
        return mp_const_false;
    }
    return mp_const_true;
}

mp_obj_t py_sensor_set_framesize(mp_obj_t framesize) {
    if (sensor_set_framesize(mp_obj_get_int(framesize)) != 0) {
        return mp_const_false;
    }
    return mp_const_true;
}

mp_obj_t py_sensor_set_gainceiling(mp_obj_t gainceiling) {
    if (sensor_set_gainceiling(mp_obj_get_int(gainceiling)) != 0) {
        return mp_const_false;
    }
    return mp_const_true;
}

mp_obj_t py_sensor_set_brightness(mp_obj_t brightness) {
    if (sensor_set_brightness(mp_obj_get_int(brightness)) != 0) {
        return mp_const_false;
    }
    return mp_const_true;
}

mp_obj_t py_sensor_write_reg(mp_obj_t addr, mp_obj_t val) {
    SCCB_Write(mp_obj_get_int(addr), mp_obj_get_int(val));
    return mp_const_none;
}

mp_obj_t py_sensor_read_reg(mp_obj_t addr) {
    return mp_obj_new_int(SCCB_Read(mp_obj_get_int(addr)));
}

void py_sensor_print(void (*print)(void *env, const char *fmt, ...),
                            void *env, mp_obj_t self_in, mp_print_kind_t kind) {
    //print(env, "<Sensor MID:0x%.2X%.2X PID:0x%.2X VER:0x%.2X>",
          //sensor.id.MIDH, sensor.id.MIDL, sensor.id.PID, sensor.id.VER);
}

static void rt_store_constants(mp_obj_t m, struct sym_entry *constants)
{
    for (struct sym_entry *p = constants; p->sym != NULL; p++) {
        rt_store_attr(m, QSTR_FROM_STR_STATIC(p->sym), MP_OBJ_NEW_SMALL_INT((machine_int_t)p->val));
    }
}

mp_obj_t py_sensor_init()
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

    /* Create module */
    mp_obj_t m = mp_obj_new_module(qstr_from_str("sensor"));

    /* Store module functions */
    rt_store_attr(m, qstr_from_str("init"), rt_make_function_n(0, py_sensor_sinit));
    rt_store_attr(m, qstr_from_str("reset"), rt_make_function_n(0, py_sensor_reset));
    rt_store_attr(m, qstr_from_str("snapshot"), rt_make_function_n(0, py_sensor_snapshot));
    rt_store_attr(m, qstr_from_str("set_pixformat"), rt_make_function_n(1, py_sensor_set_pixformat));
    rt_store_attr(m, qstr_from_str("set_framerate"), rt_make_function_n(1, py_sensor_set_framerate));
    rt_store_attr(m, qstr_from_str("set_framesize"), rt_make_function_n(1, py_sensor_set_framesize));
    rt_store_attr(m, qstr_from_str("set_gainceiling"), rt_make_function_n(1, py_sensor_set_gainceiling));
    rt_store_attr(m, qstr_from_str("set_brightness"), rt_make_function_n(1, py_sensor_set_brightness));
    rt_store_attr(m, qstr_from_str("__write_reg"), rt_make_function_n(2, py_sensor_write_reg));
    rt_store_attr(m, qstr_from_str("__read_reg"), rt_make_function_n(1, py_sensor_read_reg));

    /* Store module constants */
    rt_store_constants(m, pixformat_constants);
    rt_store_constants(m, framesize_constants);
    rt_store_constants(m, framerate_constants);
    rt_store_constants(m, gainceiling_constants);
    return m;
}
