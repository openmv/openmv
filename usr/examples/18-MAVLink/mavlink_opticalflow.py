# MAVLink OpticalFlow Script.
#
# This script sends out OpticalFlow detections using the MAVLink protocol to
# an ArduPilot/PixHawk controller for position control using your OpenMV Cam.
#
# P4 = TXD

import image, math, pyb, sensor, struct, time

# Parameters #################################################################

uart_baudrate = 115200

MAV_system_id = 1
MAV_component_id = 0x54
MAV_OPTICAL_FLOW_confidence_threshold = 0.2

##############################################################################

# Camera Setup

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)

# Link Setup

uart = pyb.UART(3, uart_baudrate, timeout_char = 1000)

# Helper Stuff

packet_sequence = 0

def checksum(data, extra): # https://github.com/mavlink/c_library_v1/blob/master/checksum.h
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
MAV_OPTICAL_FLOW_id = 0 # unused
MAV_OPTICAL_FLOW_extra_crc = 175

# http://mavlink.org/messages/common#OPTICAL_FLOW
# https://github.com/mavlink/c_library_v1/blob/master/common/mavlink_msg_optical_flow.h
def send_optical_flow_packet(x, y, c):
    global packet_sequence
    temp = struct.pack("<qfffhhbb",
                       0,
                       0,
                       0,
                       0,
                       int(x * 10 * 4), # up sample by 4
                       int(y * 10 * 4), # up sample by 4
                       MAV_OPTICAL_FLOW_id,
                       int(c * 255))
    temp = struct.pack("<bbbbb26s",
                       26,
                       packet_sequence & 0xFF,
                       MAV_system_id,
                       MAV_component_id,
                       MAV_OPTICAL_FLOW_message_id,
                       temp)
    temp = struct.pack("<b31sh",
                       0xFE,
                       temp,
                       checksum(temp, MAV_OPTICAL_FLOW_extra_crc))
    packet_sequence += 1
    uart.write(temp)

# Main Loop

clock = time.clock()
old_img = sensor.snapshot().mean_pooled(4, 4) # 160x120 -> 40x30
while(True):
    clock.tick()
    new_img = sensor.snapshot().mean_pooled(4, 4) # 160x120 -> 40x30
    x, y, c = new_img.find_displacement(old_img)
    old_img = new_img

    if (not (math.isnan(x) or math.isnan(y) or math.isnan(c))) and (c > MAV_OPTICAL_FLOW_confidence_threshold):
        send_optical_flow_packet(-x, -y, c)
        print("dx %10f, dy %10f, confidence %10f - FPS %f" % (-x, -y, c, clock.fps()))
    else:
        print("FPS %f" % (clock.fps()))
