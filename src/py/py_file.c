#include <libmp.h>
#include "py_assert.h"
#include "py_file.h"
typedef struct _py_file_obj_t {
    mp_obj_base_t base;
    FIL fp;
} py_file_obj_t;

const char *ffs_strerror(FRESULT res)
{
    static const char *ffs_errors[]={
        "Succeeded",
        "A hard error occurred in the low level disk I/O layer",
        "Assertion failed",
        "The physical drive cannot work",
        "Could not find the file",
        "Could not find the path",
        "The path name format is invalid",
        "Access denied due to prohibited access or directory full",
        "Access denied due to prohibited access",
        "The file/directory object is invalid",
        "The physical drive is write protected",
        "The logical drive number is invalid",
        "The volume has no work area",
        "There is no valid FAT volume",
        "The f_mkfs() aborted due to any parameter error",
        "Could not get a grant to access the volume within defined period",
        "The operation is rejected according to the file sharing policy",
        "LFN working buffer could not be allocated",
        "Number of open files > _FS_SHARE",
        "Given parameter is invalid",
    };

    if (res>sizeof(ffs_errors)/sizeof(ffs_errors[0])) {
        return "unknown error";
    } else {
        return ffs_errors[res];
    }
}

mp_obj_t py_file_close(py_file_obj_t *file)
{
    f_close(&file->fp);
    return mp_const_none;
}

mp_obj_t py_file_read(py_file_obj_t *file, mp_obj_t n_obj)
{
    UINT n_out;
    UINT n =  mp_obj_get_int(n_obj);

    byte *buf = m_new(byte, n);
    f_read(py_file_cobj(&file->fp), buf, n, &n_out);
    return mp_obj_new_str(buf, n_out, false);
}

mp_obj_t py_file_write(py_file_obj_t *file, mp_obj_t buf)
{
    uint len;
    const char *str;
    FRESULT res;

    str = mp_obj_str_get_data(buf, &len);

    res = f_write(&file->fp, str, len, &len);
    if (res != FR_OK) {
        nlr_jump(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    return mp_obj_new_int(len);
}

void py_file_print(void (*print)(void *env, const char *fmt, ...), void *env, mp_obj_t self_in, mp_print_kind_t kind)
{
    print(env, "<file>");
}

static MP_DEFINE_CONST_FUN_OBJ_1(py_file_close_obj, py_file_close);
static MP_DEFINE_CONST_FUN_OBJ_2(py_file_read_obj, py_file_read);
static MP_DEFINE_CONST_FUN_OBJ_2(py_file_write_obj, py_file_write);

static const mp_method_t py_file_methods[] = {
    { "close",  &py_file_close_obj},
    { "read",   &py_file_read_obj},
    { "write",  &py_file_write_obj},
    { NULL, NULL },
};

static const mp_obj_type_t py_file_type = {
    { &mp_type_type },
    .name       = MP_QSTR_File,
    .print      = py_file_print,
    .methods    = py_file_methods,
};

mp_obj_t py_file_open(mp_obj_t path, mp_obj_t mode_str)
{
    BYTE mode=0;
    FRESULT res;
    py_file_obj_t *o;

    switch (mp_obj_str_get_str(mode_str)[0]) {
        case 'r':
            /* Open file for reading, fail if the file is not existing. */
            mode = FA_READ|FA_OPEN_EXISTING;
            break;
        case 'w':
            /* Open file for reading/writing, create the file if not existing. */
            mode = FA_READ|FA_WRITE|FA_OPEN_ALWAYS;
            break;
        case 'a':
            /* Open file for reading/writing, fail if the file is not existing. */
            mode = FA_READ|FA_WRITE|FA_OPEN_EXISTING;
            break;
        default:
            nlr_jump(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid open mode"));
    }

    /* Create new python file obj */
    o = m_new_obj(py_file_obj_t);
    o->base.type = &py_file_type;

    /* Open underlying file handle */
    res = f_open(&o->fp, mp_obj_str_get_str(path), mode);
    if (res != FR_OK) {
        nlr_jump(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }
    return o;
}

void *py_file_cobj(mp_obj_t file)
{
    PY_ASSERT_TYPE(file, &py_file_type);
    return &((py_file_obj_t *)file)->fp;
}

