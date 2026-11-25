def unittest(data_path, temp_path):
    import csi

    if not hasattr(csi, "IOCTL_PS5520_SET_LTM_ENABLE"):
        return True

    cam = csi.CSI()
    if cam.cid() != 0x5520:
        return True

    def strengths():
        cam.__write_reg(0xEF, 0x06)
        try:
            return cam.__read_reg(0x9A), cam.__read_reg(0x99)
        finally:
            cam.__write_reg(0xEF, 0x01)

    def disabled():
        return not cam.ioctl(csi.IOCTL_PS5520_GET_LTM_ENABLE) and strengths() == (0, 0)

    try:
        cam.reset()
        cam.pixformat(csi.RGB565)
        cam.framesize(csi.QVGA)
        if not cam.ioctl(csi.IOCTL_PS5520_GET_LTM_ENABLE):
            return False
        cam.ioctl(csi.IOCTL_PS5520_SET_LTM_ENABLE, False)
        if not disabled():
            return False
        cam.auto_gain(False, gain_db=12)
        cam.auto_exposure(False, exposure_us=10000)
        if not disabled():
            return False
        cam.framesize(csi.VGA)
        if not disabled():
            return False
        cam.pixformat(csi.BAYER)
        if not disabled():
            return False
        cam.pixformat(csi.RGB565)
        if not disabled():
            return False
        cam.ioctl(csi.IOCTL_PS5520_SET_LTM_ENABLE, True)
        if not cam.ioctl(csi.IOCTL_PS5520_GET_LTM_ENABLE):
            return False
        ltm, backlight = strengths()
        if ltm == 0 or backlight == 0:
            return False
        cam.ioctl(csi.IOCTL_PS5520_SET_LTM_ENABLE, False)
        cam.reset()
        return cam.ioctl(csi.IOCTL_PS5520_GET_LTM_ENABLE)
    finally:
        cam.reset()
