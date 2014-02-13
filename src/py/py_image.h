#ifndef __PY_IMAGE_H__
#define __PY_IMAGE_H__
mp_obj_t py_image(int width, int height, int bpp, void *pixels);
void *py_image_cobj(mp_obj_t image);
#endif /* __PY_IMAGE_H__ */

