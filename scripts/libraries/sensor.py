# This file is part of the OpenMV project.
#
# Copyright (C) 2026 OpenMV, LLC.
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Backwards compatibility layer for the deprecated sensor module. This module
# implements the sensor module's API on top of the csi module. New code should
# use the csi module directly.

import time
import csi
from csi import *  # noqa

# Framesizes that the csi module does not export. These are passed to the csi
# module as custom resolutions, and get_framesize() returns the one that was
# set, so that comparing against these still works.
QQCIF = (88, 72)
QQSIF = (88, 60)
QQQQVGA = (40, 30)
HQQQQVGA = (30, 20)
HQQQVGA = (60, 40)
HQQVGA = (120, 80)
B64X32 = (64, 32)
B64X64 = (64, 64)
B128X64 = (128, 64)
B128X128 = (128, 128)
B160X160 = (160, 160)
B320X320 = (320, 320)
LCD = (128, 160)
QQVGA2 = (128, 160)

print(
    "WARNING: The sensor module is deprecated and will be removed in a future "
    "release. Please use the csi module instead."
)

# The sensor module operated on the main camera, and raised an error on import
# if it was not detected.
_csi = csi.CSI()

# The last framesize passed to set_framesize(), for get_framesize().
_framesize = None


def __init__():
    # The sensor module raised an error here if the camera was not detected,
    # which now happens on import, when the CSI object is created.
    pass


def reset():
    global _framesize
    _framesize = None
    _csi.reset()


def sleep(enable):
    try:
        _csi.sleep(enable)
    except RuntimeError:
        raise OSError("Sleep Failed")


def shutdown(enable):
    try:
        _csi.shutdown(enable)
    except RuntimeError:
        raise OSError("Shutdown Failed")


def flush():
    _csi.flush()


def snapshot(*args, **kwargs):
    return _csi.snapshot()


def skip_frames(*args, **kwargs):
    # skip_frames()             -> capture for 300 ms.
    # skip_frames(n)            -> capture n frames.
    # skip_frames(time=t)       -> capture for t ms.
    # skip_frames(n, time=t)    -> capture n frames, or t ms, whichever is first.
    frames = args[0] if args else None
    timed = "time" in kwargs
    millis = kwargs.get("time", 300)
    start = time.ticks_ms()

    if frames is None:
        while time.ticks_diff(time.ticks_ms(), start) < millis:
            snapshot()
    else:
        for i in range(frames):
            if timed and time.ticks_diff(time.ticks_ms(), start) >= millis:
                break
            snapshot()


def width():
    return _csi.width()


def height():
    return _csi.height()


def get_fb():
    # The csi module returns the image from snapshot() instead.
    raise OSError("get_fb is deprecated")


def get_id():
    return _csi.cid()


def get_frame_available():
    return _csi.readable()


def alloc_extra_fb(w, h, pixfmt):
    raise OSError("alloc_extra_fb is deprecated")


def dealloc_extra_fb():
    raise OSError("dealloc_extra_fb is deprecated")


def set_pixformat(pixformat):
    _csi.pixformat(pixformat)


def get_pixformat():
    return _csi.pixformat()


def set_framesize(framesize):
    global _framesize
    _csi.framesize(framesize)
    _framesize = framesize


def get_framesize():
    framesize = _csi.framesize()
    # Custom resolutions read back as the csi module's custom framesize, so
    # return the tuple that was set, if the camera is still using it.
    if isinstance(_framesize, tuple) and _framesize == (_csi.width(), _csi.height()):
        return _framesize
    return framesize


def set_framerate(framerate):
    _csi.framerate(framerate)


def get_framerate():
    return _csi.framerate()


def set_windowing(arg, *args):
    # set_windowing((x, y, w, h)) or set_windowing(x, y, w, h) or (w, h).
    _csi.window(arg if not args else (arg,) + args)


def get_windowing():
    return _csi.window()


def set_gainceiling(gainceiling):
    return _csi.gainceiling(gainceiling)


def set_brightness(brightness):
    return _csi.brightness(brightness)


def set_contrast(contrast):
    return _csi.contrast(contrast)


def set_saturation(saturation):
    return _csi.saturation(saturation)


def set_quality(quality):
    return _csi.quality(quality)


def set_colorbar(enable):
    return _csi.colorbar(enable)


def set_auto_gain(enable, gain_db=None, gain_db_ceiling=None):
    _csi.auto_gain(enable, gain_db=gain_db, gain_db_ceiling=gain_db_ceiling)


def get_gain_db():
    return _csi.gain_db()


def set_auto_exposure(enable, exposure_us=-1):
    _csi.auto_exposure(enable, exposure_us=exposure_us)


def get_exposure_us():
    return _csi.exposure_us()


def set_auto_whitebal(enable, rgb_gain_db=None):
    _csi.auto_whitebal(enable, rgb_gain_db=rgb_gain_db)


def get_rgb_gain_db():
    return _csi.rgb_gain_db()


def set_auto_blc(enable, regs=None):
    _csi.auto_blc(enable, regs=regs)


def get_blc_regs():
    return _csi.blc_regs()


def set_hmirror(enable):
    _csi.hmirror(enable)


def get_hmirror():
    return _csi.hmirror()


def set_vflip(enable):
    _csi.vflip(enable)


def get_vflip():
    return _csi.vflip()


def set_transpose(enable):
    _csi.transpose(enable)


def get_transpose():
    return _csi.transpose()


def set_auto_rotation(enable):
    raise OSError("set_auto_rotation is deprecated")


def get_auto_rotation():
    raise OSError("get_auto_rotation is deprecated")


def set_framebuffers(count):
    _csi.framebuffers(count)


def get_framebuffers():
    return _csi.framebuffers()


def disable_delays(enable=None):
    # The csi module only accepts this on construction: csi.CSI(delays=False).
    raise OSError("disable_delays is deprecated")


def disable_full_flush(enable=None):
    # The csi module only accepts this on construction: csi.CSI(fflush=False).
    raise OSError("disable_full_flush is deprecated")


def set_special_effect(sde):
    return _csi.special_effect(sde)


def set_lens_correction(enable, radi, coef):
    return _csi.lens_correction(enable, radi, coef)


def set_vsync_callback(cb):
    _csi.vsync_callback(cb)


def set_frame_callback(cb):
    _csi.frame_callback(cb)


def ioctl(*args):
    # The old module took up to four arguments after the request, and ignored
    # any that the request did not use.
    return _csi.ioctl(*args[:4])


def set_color_palette(palette):
    _csi.color_palette(palette)


def get_color_palette():
    return _csi.color_palette()


def __write_reg(addr, val):
    _csi.__write_reg(addr, val)


def __read_reg(addr):
    return _csi.__read_reg(addr)
