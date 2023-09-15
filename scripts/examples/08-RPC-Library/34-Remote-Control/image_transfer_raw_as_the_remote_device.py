# Image Transfer - As The Remote Device
#
# This script is made to pair with another OpenMV Cam running "image_transfer_raw_as_the_controller_device.py"
#
# This script shows off how to transfer the frame buffer from one OpenMV Cam to another.

import rpc
import sensor
import struct

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)

# The RPC library above is installed on your OpenMV Cam and provides multiple classes for
# allowing your OpenMV Cam to be controlled over CAN, I2C, SPI, UART, or LAN/WLAN.

################################################################
# Choose the interface you wish to control your OpenMV Cam over.
################################################################

# Uncomment the below line to setup your OpenMV Cam for control over CAN.
#
# * message_id - CAN message to use for data transport on the can bus (11-bit).
# * bit_rate - CAN bit rate.
# * sample_point - Tseg1/Tseg2 ratio. Typically 75%. (50.0, 62.5, 75.0, 87.5, etc.)
#
# NOTE: Master and slave message ids and can bit rates must match. Connect master can high to slave
#       can high and master can low to slave can lo. The can bus must be terminated with 120 ohms.
#
# interface = rpc.rpc_can_slave(message_id=0x7FF, bit_rate=1000000, sample_point=75.0)

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
interface = rpc.rpc_spi_slave(cs_pin="P3", clk_polarity=1, clk_phase=0)

# Uncomment the below line to setup your OpenMV Cam for control over UART.
#
# * baudrate - Serial Baudrate.
#
# NOTE: Master and slave baud rates must match. Connect master tx to slave rx and master rx to
#       slave tx. Finally, both devices must share a common ground.
#
# interface = rpc.rpc_uart_slave(baudrate=7500000)

################################################################
# Call Backs
################################################################


# When called sets the pixformat and framesize, takes a snapshot
# and then returns the frame buffer shape to store the image in.
#
# data is a 4 byte pixformat and 4 byte framesize.
def raw_image_snapshot(data):
    pixformat, framesize = struct.unpack("<II", data)
    sensor.set_pixformat(pixformat)
    sensor.set_framesize(framesize)
    img = sensor.snapshot()
    return struct.pack(
        "<IIII", sensor.width(), sensor.height(), sensor.get_pixformat(), img.size()
    )


def raw_image_read_cb():
    interface.put_bytes(sensor.get_fb().bytearray(), 5000)  # timeout


# Read data from the frame buffer given a offset and size.
# If data is empty then a transfer is scheduled after the RPC call finishes.
#
# data is a 4 byte size and 4 byte offset.
def raw_image_read(data):
    if not len(data):
        interface.schedule_callback(raw_image_read_cb)
        return bytes()
    else:
        offset, size = struct.unpack("<II", data)
        return memoryview(sensor.get_fb().bytearray())[offset : offset + size]


# Register call backs.

interface.register_callback(raw_image_snapshot)
interface.register_callback(raw_image_read)

# Once all call backs have been registered we can start
# processing remote events. interface.loop() does not return.

interface.loop()
