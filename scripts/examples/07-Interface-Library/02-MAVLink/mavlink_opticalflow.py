# MAVLink OpticalFlow Script.
#
# This script sends out OpticalFlow detections using the MAVLink protocol to
# an ArduPilot/PixHawk controller for position control using your OpenMV Cam.
#
# P4 = TXD

import sensor
import struct
import time
import machine

UART_BAUDRATE = 115200
MAV_system_id = 1
MAV_component_id = 0x54
packet_sequence = 0

# Below 0.1 or so (YMMV) and the results are just noise.
MAV_OPTICAL_FLOW_confidence_threshold = (0.1)

# LED control
led = machine.LED("LED_BLUE")
led_state = 0


def update_led():
    global led_state
    led_state = led_state + 1
    if led_state == 10:
        led.on()
    elif led_state >= 20:
        led.off()
        led_state = 0


# Link Setup
uart = machine.UART(3, UART_BAUDRATE, timeout_char=1000)


# https://github.com/mavlink/c_library_v1/blob/master/checksum.h
def checksum(data, extra):
    output = 0xFFFF
    for i in range(len(data)):
        tmp = data[i] ^ (output & 0xFF)
        tmp = (tmp ^ (tmp << 4)) & 0xFF
        output = ((output >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4)) & 0xFFFF
    tmp = extra ^ (output & 0xFF)
    tmp = (tmp ^ (tmp << 4)) & 0xFF
    output = ((output >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4)) & 0xFFFF
    return output


MAV_OPTICAL_FLOW_message_id = 100
MAV_OPTICAL_FLOW_id = 0  # unused
MAV_OPTICAL_FLOW_extra_crc = 175


# http://mavlink.org/messages/common#OPTICAL_FLOW
# https://github.com/mavlink/c_library_v1/blob/master/common/mavlink_msg_optical_flow.h
def send_optical_flow_packet(x, y, c):
    global packet_sequence
    temp = struct.pack(
        "<qfffhhbb", 0, 0, 0, 0, int(x), int(y), MAV_OPTICAL_FLOW_id, int(c * 255)
    )
    temp = struct.pack(
        "<bbbbb26s",
        26,
        packet_sequence & 0xFF,
        MAV_system_id,
        MAV_component_id,
        MAV_OPTICAL_FLOW_message_id,
        temp,
    )
    temp = struct.pack("<b31sh", 0xFE, temp, checksum(temp, MAV_OPTICAL_FLOW_extra_crc))
    packet_sequence += 1
    uart.write(temp)
    update_led()


sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.B64X32)  # Set frame size to 64x32... (or 64x64)...
sensor.skip_frames(time=2000)  # Wait for settings take effect.
clock = time.clock()  # Create a clock object to track the FPS.

# Take from the main frame buffer's RAM to allocate a second frame buffer.
# There's a lot more RAM in the frame buffer than in the MicroPython heap.
# However, after doing this you have a lot less RAM for some algorithms...
# So, be aware that it's a lot easier to get out of RAM issues now.
extra_fb = sensor.alloc_extra_fb(sensor.width(), sensor.height(), sensor.RGB565)
extra_fb.replace(sensor.snapshot())

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot()  # Take a picture and return the image.

    displacement = extra_fb.find_displacement(img)
    extra_fb.replace(img)

    # Offset results are noisy without filtering so we drop some accuracy.
    sub_pixel_x = int(-displacement.x_translation() * 35)
    sub_pixel_y = int(displacement.y_translation() * 53)

    send_optical_flow_packet(sub_pixel_x, sub_pixel_y, displacement.response())

    print(
        "{0:+f}x {1:+f}y {2} {3} FPS".format(
            sub_pixel_x, sub_pixel_y, displacement.response(), clock.fps()
        )
    )
