#ifndef __PY_IMAGE_H__
#define __PY_IMAGE_H__
mp_obj_t py_image_load_image(mp_obj_t path_obj);
mp_obj_t py_image_load_cascade(mp_obj_t path_obj);
mp_obj_t py_image(int width, int height, int bpp, void *pixels);
void *py_image_cobj(mp_obj_t image);
#endif /* __PY_IMAGE_H__ */

