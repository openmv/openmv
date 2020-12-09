# Image Transfer - As The Remote Device
#
# This script is meant to talk to the "image_transfer_jpg_as_the_controller_device.py" on your computer.
#
# This script shows off how to transfer the frame buffer to your computer as a jpeg image.

import image, network, omv, rpc, sensor, struct

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)

# Turn off the frame buffer connection to the IDE from the OpenMV Cam side.
#
# This needs to be done when manually compressing jpeg images at higher quality
# so that the OpenMV Cam does not try to stream them to the IDE using a fall back
# mechanism if the JPEG image is too large to fit in the IDE JPEG frame buffer on the OpenMV Cam.

omv.disable_fb(True)

# The RPC library above is installed on your OpenMV Cam and provides mutliple classes for
# allowing your OpenMV Cam to be controlled over USB or WIFI.

################################################################
# Choose the interface you wish to control your OpenMV Cam over.
################################################################

# Uncomment the below line to setup your OpenMV Cam for control over a USB VCP.
#
interface = rpc.rpc_usb_vcp_slave()

# Uncomment the below line to setup your OpenMV Cam for control over WiFi.
#
# * ssid - WiFi network to connect to.
# * ssid_key - WiFi network password.
# * ssid_security - WiFi security.
# * port - Port to route traffic to.
# * mode - Regular or access-point mode.
# * static_ip - If not None then a tuple of the (IP Address, Subnet Mask, Gateway, DNS Address)
#
# interface = rpc.rpc_wifi_slave(ssid="",
#                                ssid_key="",
#                                ssid_security=network.WINC.WPA_PSK,
#                                port=0x1DBA,
#                                mode=network.WINC.MODE_STA,
#                                static_ip=None)

################################################################
# Call Backs
################################################################

# When called sets the pixformat and framesize, takes a snapshot
# and then returns the frame buffer jpg size to store the image in.
#
# data is a pixformat string and framesize string.
def jpeg_image_snapshot(data):
    pixformat, framesize = bytes(data).decode().split(",")
    sensor.set_pixformat(eval(pixformat))
    sensor.set_framesize(eval(framesize))
    img = sensor.snapshot().compress(quality=90)
    return struct.pack("<I", img.size())

def jpeg_image_read_cb():
    interface.put_bytes(sensor.get_fb().bytearray(), 5000) # timeout

# Read data from the frame buffer given a offset and size.
# If data is empty then a transfer is scheduled after the RPC call finishes.
#
# data is a 4 byte size and 4 byte offset.
def jpeg_image_read(data):
    if not len(data):
        interface.schedule_callback(jpeg_image_read_cb)
        return bytes()
    else:
        offset, size = struct.unpack("<II", data)
        return memoryview(sensor.get_fb().bytearray())[offset:offset+size]

# Register call backs.

interface.register_callback(jpeg_image_snapshot)
interface.register_callback(jpeg_image_read)

# Once all call backs have been registered we can start
# processing remote events. interface.loop() does not return.

interface.loop()
