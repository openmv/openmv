# Control local tone mapping on the PixArt PS5520 sensor.
# Lock exposure, gain, and white balance as needed for repeatable measurements.
# Other processing and clipping can still affect the resulting pixel values.
import csi

cam = csi.CSI()
cam.reset()
cam.pixformat(csi.RGB565)
cam.framesize(csi.QVGA)
cam.ioctl(csi.IOCTL_PS5520_SET_LTM_ENABLE, False)
print("LTM enabled:", cam.ioctl(csi.IOCTL_PS5520_GET_LTM_ENABLE))

while True:
    img = cam.snapshot()
