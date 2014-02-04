#include <libmp.h>
#include "sensor.h"
#include "py_sensor.h"
#include "py_imlib.h"

/* sensor handle */
struct sensor_dev sensor; 

struct sym_entry {
    const char *sym;
    int val;
} static constants[] = {
    {NULL, 0}
};


mp_obj_t py_sensor_sinit() {
    sensor_init(&sensor);
    /* hack */
    sensor_reset(&sensor);
    sensor_set_pixformat(&sensor, PIXFORMAT_RGB565);
    sensor_set_framesize(&sensor, FRAMESIZE_QQVGA);
    sensor_set_framerate(&sensor, FRAMERATE_30FPS);
    sensor_set_gainceiling(&sensor, GAINCEILING_16X);
    sensor_set_brightness(&sensor, 3);
    return mp_const_none;
}

mp_obj_t py_sensor_reset() {
    sensor_reset(&sensor);
    return mp_const_none;
}

mp_obj_t py_sensor_snapshot() {
    sensor_snapshot(&sensor);
    struct frame_buffer *fb = &sensor.frame_buffer;
    mp_obj_t image = py_image(fb->width, fb->height, fb->bpp, fb->pixels);
    return image;
}

void py_sensor_print(void (*print)(void *env, const char *fmt, ...), void *env, mp_obj_t self_in, mp_print_kind_t kind) {
    print(env, "<Sensor MID:0x%.2X%.2X PID:0x%.2X VER:0x%.2X>", 
          sensor.id.MIDH, sensor.id.MIDL, sensor.id.PID, sensor.id.VER);
}

mp_obj_t py_sensor_init()
{
    /* Init sensor */
    /* hack */
    sensor_init(&sensor);
    sensor_reset(&sensor);
    sensor_set_pixformat(&sensor, PIXFORMAT_RGB565);
    sensor_set_framesize(&sensor, FRAMESIZE_QQVGA);
    sensor_set_framerate(&sensor, FRAMERATE_30FPS);
    sensor_set_gainceiling(&sensor, GAINCEILING_16X);
    sensor_set_brightness(&sensor, 3);

    /* Create module */
    mp_obj_t m = mp_obj_new_module(qstr_from_str("sensor"));
    rt_store_attr(m, qstr_from_str("init"), rt_make_function_n(0, py_sensor_sinit));
    rt_store_attr(m, qstr_from_str("reset"), rt_make_function_n(0, py_sensor_reset));
    rt_store_attr(m, qstr_from_str("snapshot"), rt_make_function_n(0, py_sensor_snapshot));

    /* Store module attributes */
    for (struct sym_entry *p = constants; p->sym != NULL; p++) {
        rt_store_attr(m, QSTR_FROM_STR_STATIC(p->sym), MP_OBJ_NEW_SMALL_INT((machine_int_t)p->val));
    }

    return m;
}
