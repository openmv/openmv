# Image Transfer - As The Controller Device
#
# This script is made to pair with another OpenMV Cam running "image_transfer_raw_as_the_remote_device.py"
#
# This script shows off how to transfer the frame buffer from one OpenMV Cam to another.

import image, network, omv, rpc, sensor, struct, time

# The RPC library above is installed on your OpenMV Cam and provides mutliple classes for
# allowing your OpenMV Cam to control over CAN, I2C, SPI, UART, or LAN/WLAN.

##############################################################
# Choose the interface you wish to control an OpenMV Cam over.
##############################################################

# Uncomment the below line to setup your OpenMV Cam for controlling over CAN.
#
# * message_id - CAN message to use for data transport on the can bus (11-bit).
# * bit_rate - CAN bit rate.
# * sample_point - Tseg1/Tseg2 ratio. Typically 75%. (50.0, 62.5, 75.0, 87.5, etc.)
#
# NOTE: Master and slave message ids and can bit rates must match. Connect master can high to slave
#       can high and master can low to slave can lo. The can bus must be terminated with 120 ohms.
#
# interface = rpc.rpc_can_master(message_id=0x7FF, bit_rate=1000000, sample_point=75.0)

# Uncomment the below line to setup your OpenMV Cam for controlling over I2C.
#
# * slave_addr - I2C address.
# * rate - I2C Bus Clock Frequency.
#
# NOTE: Master and slave addresses must match. Connect master scl to slave scl and master sda
#       to slave sda. You must use external pull ups. Finally, both devices must share a ground.
#
# interface = rpc.rpc_i2c_master(slave_addr=0x12, rate=1000000)

# Uncomment the below line to setup your OpenMV Cam for controlling over SPI.
#
# * cs_pin - Slave Select Pin.
# * freq - SPI Bus Clock Frequency
# * clk_polarity - Idle clock level (0 or 1).
# * clk_phase - Sample data on the first (0) or second edge (1) of the clock.
#
# NOTE: Master and slave settings much match. Connect CS, SCLK, MOSI, MISO to CS, SCLK, MOSI, MISO.
#       Finally, both devices must share a common ground.
#
interface = rpc.rpc_spi_master(cs_pin="P3", freq=20000000, clk_polarity=1, clk_phase=0)

# Uncomment the below line to setup your OpenMV Cam for controlling over UART.
#
# * baudrate - Serial Baudrate.
#
# NOTE: Master and slave baud rates must match. Connect master tx to slave rx and master rx to
#       slave tx. Finally, both devices must share a common ground.
#
# interface = rpc.rpc_uart_master(baudrate=7500000)

##############################################################
# Call Back Handlers
##############################################################

def get_frame_buffer_call_back(pixformat, framesize, cutthrough, silent):
    if not silent: print("Getting Remote Frame...")

    result = interface.call("raw_image_snapshot", struct.pack("<II", pixformat, framesize))
    if result is not None:

        w, h, pixformat, size = struct.unpack("<IIII", result)
        img = image.Image(w, h, pixformat, copy_to_fb=True) # Alloc cleared frame buffer.

        if cutthrough:
            # Fast cutthrough data transfer with no error checking.

            # Before starting the cut through data transfer we need to sync both the master and the
            # slave device. On return both devices are in sync.
            result = interface.call("raw_image_read")
            if result is not None:

                # GET BYTES NEEDS TO EXECUTE NEXT IMMEDIATELY WITH LITTLE DELAY NEXT.

                # Read all the image data in one very large transfer.
                interface.get_bytes(img.bytearray(), 5000) # timeout

        else:
            # Slower data transfer with error checking.

            # Transfer 32/8 KB chunks.
            chunk_size = (1 << 15) if omv.board_type() == "H7" else (1 << 13)

            if not silent: print("Reading %d bytes..." % size)
            for i in range(0, size, chunk_size):
                ok = False
                for j in range(3): # Try up to 3 times.
                    result = interface.call("raw_image_read", struct.pack("<II", i, chunk_size))
                    if result is not None:
                        img.bytearray()[i:i+chunk_size] = result # Write the image data.
                        if not silent: print("%.2f%%" % ((i * 100) / size))
                        ok = True
                        break
                    if not silent: print("Retrying... %d/2" % (j + 1))
                if not ok:
                    if not silent: print("Error!")
                    return None

        return img

    else:
        if not silent: print("Failed to get Remote Frame!")

    return None

clock = time.clock()
while(True):
    clock.tick()

    # You may change the pixformat and the framesize of the image transfered from the remote device
    # by modifying the below arguments.
    #
    # When cutthrough is False the image will be transferred through the RPC library with CRC and
    # retry protection on all data moved. For faster data transfer set cutthrough to True so that
    # get_bytes() and put_bytes() are called after an RPC call completes to transfer data
    # more quicly from one image buffer to another. Note: This works because once an RPC call
    # completes successfully both the master and slave devices are synchronized completely.
    #
    img = get_frame_buffer_call_back(sensor.RGB565, sensor.QQVGA, cutthrough=True, silent=True)
    if img is not None:
        pass # You can process the image here.

    print(clock.fps())
