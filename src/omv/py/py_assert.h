#ifndef __PY_ASSERT_H__
#define __PY_ASSERT_H__
#define PY_ASSERT_TRUE(cond)                            \
    do {                                                \
        if ((cond) ==0){                                \
            nlr_jump(mp_obj_new_exception_msg(          \
                        &mp_type_OSError,               \
                        "Operation not supported"));    \
        }                                               \
    } while(0)

#define PY_ASSERT_TYPE(obj, type)                       \
    do {                                                \
        __typeof__ (obj) _a = (obj);                    \
        __typeof__ (type) _b = (type);                  \
        if (!MP_OBJ_IS_TYPE(_a, _b)) {                  \
            nlr_jump(mp_obj_new_exception_msg_varg(     \
                        &mp_type_TypeError,             \
                        "can't convert %s to %s",       \
                        mp_obj_get_type_str(_a),        \
                        _b->name));                     \
        }                                               \
    } while(0)
/* IS_TYPE doesn't work for str objs */
#define PY_ASSERT_STR(obj)                              \
    do {                                                \
        __typeof__ (obj) _a = (obj);                    \
        if (!MP_OBJ_IS_STR(_a)) {                       \
            nlr_jump(mp_obj_new_exception_msg_varg(     \
                        &mp_type_TypeError,             \
                        "can't convert %s to %s",       \
                        mp_obj_get_type_str(_a),        \
                        str_type.name));                \
        }                                               \
    } while(0)

#endif /* __PY_ASSERT_H__ */
