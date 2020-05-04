# Remote Control - As The Remote Device
#
# This script configures your OpenMV Cam as a co-processor that can be remotely controlled by
# another microcontroller or computer such as an Arduino, ESP8266/ESP32, RaspberryPi, and
# even another OpenMV Cam.
#
# This script is designed to pair with "popular_features_as_the_controller_device.py".

import image, network, math, rpc, sensor, struct, tf

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)

# The RPC library above is installed on your OpenMV Cam and provides mutliple classes for
# allowing your OpenMV Cam to be controlled over CAN, I2C, SPI, UART, USB VCP, or WiFi.

################################################################
# Choose the interface you wish to control your OpenMV Cam over.
################################################################

# Uncomment the below line to setup your OpenMV Cam for control over CAN.
#
# * message_id - CAN message to use for data transport on the can bus (11-bit).
# * bit_rate - CAN bit rate.
# * sampling_point - Tseg1/Tseg2 ratio. Typically 75%. (50.0, 62.5, 75, 87.5, etc.)
#
# NOTE: Master and slave message ids and can bit rates must match. Connect master can high to slave
#       can high and master can low to slave can lo. The can bus must be terminated with 120 ohms.
#
# interface = rpc.rpc_can_slave(message_id=0x7FF, bit_rate=250000, sampling_point=75)

# Uncomment the below line to setup your OpenMV Cam for control over I2C.
#
# * slave_addr - I2C address.
#
# NOTE: Master and slave addresses must match. Connect master scl to slave scl and master sda
#       to slave sda. You must use external pull ups. Finally, both devices must share a ground.
#
# interface = rpc.rpc_i2c_slave(slave_addr=0x12)

# Uncomment the below line to setup your OpenMV Cam for control over SPI.
#
# * cs_pin - Slave Select Pin.
# * clk_polarity - Idle clock level (0 or 1).
# * clk_phase - Sample data on the first (0) or second edge (1) of the clock.
#
# NOTE: Master and slave settings much match. Connect CS, SCLK, MOSI, MISO to CS, SCLK, MOSI, MISO.
#       Finally, both devices must share a common ground.
#
# interface = rpc.rpc_spi_slave(cs_pin="P3", clk_polarity=1, clk_phase=0)

# Uncomment the below line to setup your OpenMV Cam for control over UART.
#
# * baudrate - Serial Baudrate.
#
# NOTE: Master and slave baud rates must match. Connect master tx to slave rx and master rx to
#       slave tx. Finally, both devices must share a common ground.
#
interface = rpc.rpc_uart_slave(baudrate=115200)

# Uncomment the below line to setup your OpenMV Cam for control over a USB VCP.
#
# interface = rpc.rpc_usb_vcp_slave()

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

# Helper methods used by the call backs below.

def draw_detections(img, dects):
    for d in dects:
        c = d.corners()
        l = len(c)
        for i in range(l): img.draw_line(c[(i+0)%l] + c[(i+1)%l], color = (0, 255, 0))
        img.draw_rectangle(d.rect(), color = (255, 0, 0))

# Remote control works via call back methods that the controller
# device calls via the rpc module on this device. Call backs
# are functions which take a bytes() object as their argument
# and return a bytes() object as their result. The rpc module
# takes care of moving the bytes() objects across the link.
# bytes() may be the micropython int max in size.

# When called returns x, y, w, and h of the largest face within view.
#
# data is unused
def face_detection(data):
    sensor.set_pixformat(sensor.GRAYSCALE)
    sensor.set_framesize(sensor.QVGA)
    faces = sensor.snapshot().gamma_corr(contrast=1.5).find_features(image.HaarCascade("frontalface"))
    if not faces: return bytes() # No detections.
    for f in faces: sensor.get_fb().draw_rectangle(f, color = (255, 255, 255))
    out_face = max(faces, key = lambda f: f[2] * f[3])
    return struct.pack("<HHHH", out_face[0], out_face[1], out_face[2], out_face[3])

# When called returns if there's a "person" or "no_person" within view.
#
# data is unused
def person_detection(data):
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    scores = tf.classify("person_detection", sensor.snapshot())[0].output()
    return ['unsure', 'person', 'no_person'][scores.index(max(scores))].encode()

# When called returns the payload string for the largest qrcode
# within the OpenMV Cam's field-of-view.
#
# data is unused
def qrcode_detection(data):
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.VGA)
    sensor.set_windowing((320, 240))
    codes = sensor.snapshot().find_qrcodes()
    if not codes: return bytes() # No detections.
    draw_detections(sensor.get_fb(), codes)
    return max(codes, key = lambda c: c.w() * c.h()).payload().encode()

# When called returns a json list of json qrcode objects for all qrcodes in view.
#
# data is unused
def all_qrcode_detection(data):
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.VGA)
    sensor.set_windowing((320, 240))
    codes = sensor.snapshot().find_qrcodes()
    if not codes: return bytes() # No detections.
    draw_detections(sensor.get_fb(), codes)
    return str(codes).encode()

# When called returns the x/y centroid, id number, and rotation of the largest
# AprilTag within the OpenMV Cam's field-of-view.
#
# data is unused
def apriltag_detection(data):
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QQVGA)
    tags = sensor.snapshot().find_apriltags()
    if not tags: return bytes() # No detections.
    draw_detections(sensor.get_fb(), tags)
    output_tag = max(tags, key = lambda t: t.w() * t.h())
    return struct.pack("<HHHH", output_tag.cx(), output_tag.cy(), output_tag.id(),
                       int(math.degrees(output_tag.rotation())))

# When called returns a json list of json apriltag objects for all apriltags in view.
#
# data is unused
def all_apriltag_detection(data):
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QQVGA)
    tags = sensor.snapshot().find_apriltags()
    if not tags: return bytes() # No detections.
    draw_detections(sensor.get_fb(), tags)
    return str(tags).encode()

# When called returns the payload string for the largest datamatrix
# within the OpenMV Cam's field-of-view.
#
# data is unused
def datamatrix_detection(data):
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.VGA)
    sensor.set_windowing((320, 240))
    codes = sensor.snapshot().find_datamatrices()
    if not codes: return bytes() # No detections.
    draw_detections(sensor.get_fb(), codes)
    return max(codes, key = lambda c: c.w() * c.h()).payload().encode()

# When called returns a json list of json datamatrix objects for all datamatrices in view.
#
# data is unused
def all_datamatrix_detection(data):
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.VGA)
    sensor.set_windowing((320, 240))
    codes = sensor.snapshot().find_datamatrices()
    if not codes: return bytes() # No detections.
    draw_detections(sensor.get_fb(), codes)
    return str(codes).encode()

# When called returns the payload string for the largest barcode
# within the OpenMV Cam's field-of-view.
#
# data is unused
def barcode_detection(data):
    sensor.set_pixformat(sensor.GRAYSCALE)
    sensor.set_framesize(sensor.VGA)
    sensor.set_windowing((sensor.width(), sensor.height()//8))
    codes = sensor.snapshot().find_barcodes()
    if not codes: return bytes() # No detections.
    return max(codes, key = lambda c: c.w() * c.h()).payload().encode()

# When called returns a json list of json barcode objects for all barcodes in view.
#
# data is unused
def all_barcode_detection(data):
    sensor.set_pixformat(sensor.GRAYSCALE)
    sensor.set_framesize(sensor.VGA)
    sensor.set_windowing((sensor.width(), sensor.height()//8))
    codes = sensor.snapshot().find_barcodes()
    if not codes: return bytes() # No detections.
    return str(codes).encode()

# When called returns the x/y centroid of the largest blob
# within the OpenMV Cam's field-of-view.
#
# data is the 6-byte color tracking threshold tuple of L_MIN, L_MAX, A_MIN, A_MAX, B_MIN, B_MAX.
def color_detection(data):
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    thresholds = struct.unpack("<bbbbbb", data)
    blobs = sensor.snapshot().find_blobs([thresholds],
                                         pixels_threshold=500,
                                         area_threshold=500,
                                         merge=True,
                                         margin=20)
    if not blobs: return bytes() # No detections.
    for b in blobs:
        sensor.get_fb().draw_rectangle(b.rect(), color = (255, 0, 0))
        sensor.get_fb().draw_cross(b.cx(), b.cy(), color = (0, 255, 0))
    out_blob = max(blobs, key = lambda b: b.density())
    return struct.pack("<HH", out_blob.cx(), out_blob.cy())

# When called returns a jpeg compressed image from the OpenMV
# Cam in one RPC call.
#
# data is unused
def jpeg_snapshot(data):
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    return sensor.snapshot().compress(quality=90).bytearray()

# Register call backs.

interface.register_callback(face_detection)
interface.register_callback(person_detection)
interface.register_callback(qrcode_detection)
interface.register_callback(all_qrcode_detection)
interface.register_callback(apriltag_detection)
interface.register_callback(all_apriltag_detection)
interface.register_callback(datamatrix_detection)
interface.register_callback(all_datamatrix_detection)
interface.register_callback(barcode_detection)
interface.register_callback(all_barcode_detection)
interface.register_callback(color_detection)
interface.register_callback(jpeg_snapshot)

# Once all call backs have been registered we can start
# processing remote events. interface.loop() does not return.

interface.loop()
