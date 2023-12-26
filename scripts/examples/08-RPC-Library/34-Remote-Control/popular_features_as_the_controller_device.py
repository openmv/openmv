# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Remote Control - As The Controller Device
#
# This script configures your OpenMV Cam to remotely control another OpenMV Cam using the RPC
# library. This script can be run by any micropython board implementing the pyb module to
# remotely control an OpenMV Cam.
#
# This script is designed to pair with "popular_features_as_the_remote_device.py".

import json
import rpc
import struct

# The RPC library above is installed on your OpenMV Cam and provides multiple classes for
# allowing your OpenMV Cam to control over CAN, I2C, SPI, or UART.

##############################################################
# Choose the interface you wish to control an OpenMV Cam over.
##############################################################

# Uncomment the below line to setup your OpenMV Cam for controlling over CAN.
#
# * message_id - CAN message to use for data transport on the can bus (11-bit).
# * bit_rate - CAN bit rate.
# * sample_point - Tseg1/Tseg2 ratio. Typically 75%. (50.0, 62.5, 75, 87.5, etc.)
#
# NOTE: Master and slave message ids and can bit rates must match. Connect master can high to slave
#       can high and master can low to slave can lo. The can bus must be terminated with 120 ohms.
#
# interface = rpc.rpc_can_master(message_id=0x7FF, bit_rate=250000, sample_point=75)

# Uncomment the below line to setup your OpenMV Cam for controlling over I2C.
#
# * slave_addr - I2C address.
# * rate - I2C Bus Clock Frequency.
#
# NOTE: Master and slave addresses must match. Connect master scl to slave scl and master sda
#       to slave sda. You must use external pull ups. Finally, both devices must share a ground.
#
# interface = rpc.rpc_i2c_master(slave_addr=0x12, rate=100000)

# Uncomment the below line to setup your OpenMV Cam for controlling over SPI.
#
# * cs_pin - Slave Select Pin.
# * freq - SPI Bus Clock Frequency.
# * clk_polarity - Idle clock level (0 or 1).
# * clk_phase - Sample data on the first (0) or second edge (1) of the clock.
#
# NOTE: Master and slave settings much match. Connect CS, SCLK, MOSI, MISO to CS, SCLK, MOSI, MISO.
#       Finally, both devices must share a common ground.
#
# interface = rpc.rpc_spi_master(cs_pin="P3", freq=10000000, clk_polarity=1, clk_phase=0)

# Uncomment the below line to setup your OpenMV Cam for controlling over UART.
#
# * baudrate - Serial Baudrate.
#
# NOTE: Master and slave baud rates must match. Connect master tx to slave rx and master rx to
#       slave tx. Finally, both devices must share a common ground.
#
interface = rpc.rpc_uart_master(baudrate=115200)

##############################################################
# Call Back Handlers
##############################################################


def exe_face_detection():
    result = interface.call("face_detection")
    if result is not None and len(result):
        print(
            "Largest Face Detected [x=%d, y=%d, w=%d, h=%d]"
            % struct.unpack("<HHHH", result)
        )


def exe_qrcode_detection():
    result = interface.call("qrcode_detection")
    if result is not None and len(result):
        print(bytes(result).decode())


def exe_all_qrcode_detection():
    result = interface.call("all_qrcode_detection")
    if result is not None and len(result):
        print("QR Codes Detected:")
        for obj in json.loads(result):
            print(obj)


def exe_apriltag_detection():
    result = interface.call("apriltag_detection")
    if result is not None and len(result):
        print(
            "Largest Tag Detected [cx=%d, cy=%d, id=%d, rot=%d]"
            % struct.unpack("<HHHH", result)
        )


def exe_all_apriltag_detection():
    result = interface.call("all_apriltag_detection")
    if result is not None and len(result):
        print("Tags Detected:")
        for obj in json.loads(result):
            print(obj)


def exe_datamatrix_detection():
    result = interface.call("datamatrix_detection")
    if result is not None and len(result):
        print(bytes(result).decode())


def exe_all_datamatrix_detection():
    result = interface.call("all_datamatrix_detection")
    if result is not None and len(result):
        print("Data Matrices Detected:")
        for obj in json.loads(result):
            print(obj)


def exe_barcode_detection():
    result = interface.call("barcode_detection")
    if result is not None and len(result):
        print(bytes(result).decode())


def exe_all_barcode_detection():
    result = interface.call("all_barcode_detection")
    if result is not None and len(result):
        print("Bar Codes Detected:")
        for obj in json.loads(result):
            print(obj)


def exe_color_detection():
    thresholds = (30, 100, 15, 127, 15, 127)  # generic red thresholds
    # thresholds = (30, 100, -64, -8, -32, 32) # generic green thresholds
    # thresholds = (0, 30, 0, 64, -128, 0) # generic blue thresholds
    result = interface.call("color_detection", struct.pack("<bbbbbb", *thresholds))
    if result is not None and len(result):
        print("Largest Color Detected [cx=%d, cy=%d]" % struct.unpack("<HH", result))


number = 0


def exe_jpeg_snapshot():
    global number
    result = interface.call("jpeg_snapshot")
    if result is not None:
        name = "snapshot-%05d.jpg" % number
        print("Writing jpeg %s..." % name)
        with open(name, "wb") as snap:
            snap.write(result)
            number += 1


# Execute remote functions in a loop. Please choose and uncomment one remote function below.
# Executing multiple at a time may run slowly if the camera needs to change camera modes
# per execution.

while True:
    exe_face_detection()  # Face should be about 2ft away.
    # exe_qrcode_detection() # Place the QRCode about 2ft away.
    # exe_all_qrcode_detection() # Place the QRCode about 2ft away.
    # exe_apriltag_detection()
    # exe_all_apriltag_detection()
    # exe_datamatrix_detection() # Place the Datamatrix about 2ft away.
    # exe_all_datamatrix_detection() # Place the Datamatrix about 2ft away.
    # exe_barcode_detection() # Place the Barcode about 2ft away.
    # exe_all_barcode_detection() # Place the Barcode about 2ft away.
    # exe_color_detection()
    # exe_jpeg_snapshot()
