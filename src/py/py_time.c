#include <libmp.h>
#include "systick.h"
#include "py_time.h"
#include "py_clock.h"

mp_obj_t py_time_ticks()
{
    return mp_obj_new_int(systick_current_millis());
}

mp_obj_t py_time_clock()
{
    return py_clock();
}

mp_obj_t py_time_sleep(mp_obj_t ms)
{
    systick_sleep(mp_obj_get_int(ms));
    return mp_const_none;
}

mp_obj_t py_time_init()
{
    /* Create module */
    mp_obj_t m = mp_obj_new_module(qstr_from_str("time"));

    /* Store module functions */
    rt_store_attr(m, qstr_from_str("clock"), rt_make_function_n(0, py_time_clock));
    rt_store_attr(m, qstr_from_str("ticks"), rt_make_function_n(0, py_time_ticks));
    rt_store_attr(m, qstr_from_str("sleep"), rt_make_function_n(1, py_time_sleep));
    return m;
}
