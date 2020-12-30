# MAVLink AprilTags Landing Target Script.
#
# This script sends out AprilTag detections using the MAVLink protocol to
# an ArduPilot/PixHawk controller for precision landing using your OpenMV Cam.
#
# P4 = TXD

import image, math, pyb, sensor, struct, time

# Parameters #################################################################

uart_baudrate = 115200

MAV_system_id = 1
MAV_component_id = 0x54
MAX_DISTANCE_SENSOR_enable = True

lens_mm = 2.8 # Standard Lens.
lens_to_camera_mm = 22 # Standard Lens.
sensor_w_mm = 3.984 # For OV7725 sensor - see datasheet.
sensor_h_mm = 2.952 # For OV7725 sensor - see datasheet.

# Only tags with a tag ID in the dictionary below will be accepted by this
# code. You may add as many tag IDs to the below dictionary as you want...

# For each tag ID you need to provide then length of the black tag border
# in mm. Any side of the tag black border square will work.

valid_tag_ids = {
                  0 : 165, # 8.5" x 11" tag black border size in mm
                  1 : 165, # 8.5" x 11" tag black border size in mm
                  2 : 165, # 8.5" x 11" tag black border size in mm
                }

##############################################################################

# Camera Setup

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)

x_res = 160 # QQVGA
y_res = 120 # QQVGA
f_x = (lens_mm / sensor_w_mm) * x_res
f_y = (lens_mm / sensor_h_mm) * y_res
c_x = x_res / 2
c_y = y_res / 2
h_fov = 2 * math.atan((sensor_w_mm / 2) / lens_mm)
v_fov = 2 * math.atan((sensor_h_mm / 2) / lens_mm)

def z_to_mm(z_translation, tag_size): # z_translation is in decimeters...
    return (((z_translation * 100) * tag_size) / 165) - lens_to_camera_mm

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

MAV_DISTANCE_SENSOR_message_id = 132
MAV_DISTANCE_SENSOR_min_distance = 1 # in cm
MAV_DISTANCE_SENSOR_max_distance = 10000 # in cm
MAV_DISTANCE_SENSOR_type = 0 # MAV_DISTANCE_SENSOR_LASER
MAV_DISTANCE_SENSOR_id = 0 # unused
MAV_DISTANCE_SENSOR_orientation = 25 # MAV_SENSOR_ROTATION_PITCH_270
MAV_DISTANCE_SENSOR_covariance = 0 # unused
MAV_DISTANCE_SENSOR_extra_crc = 85

# http://mavlink.org/messages/common#DISTANCE_SENSOR
# https://github.com/mavlink/c_library_v1/blob/master/common/mavlink_msg_distance_sensor.h
def send_distance_sensor_packet(tag, tag_size):
    global packet_sequence
    temp = struct.pack("<lhhhbbbb",
                       0,
                       MAV_DISTANCE_SENSOR_min_distance,
                       MAV_DISTANCE_SENSOR_max_distance,
                       min(max(int(z_to_mm(tag.z_translation(), tag_size) / 10), MAV_DISTANCE_SENSOR_min_distance), MAV_DISTANCE_SENSOR_max_distance),
                       MAV_DISTANCE_SENSOR_type,
                       MAV_DISTANCE_SENSOR_id,
                       MAV_DISTANCE_SENSOR_orientation,
                       MAV_DISTANCE_SENSOR_covariance)
    temp = struct.pack("<bbbbb14s",
                       14,
                       packet_sequence & 0xFF,
                       MAV_system_id,
                       MAV_component_id,
                       MAV_DISTANCE_SENSOR_message_id,
                       temp)
    temp = struct.pack("<b19sh",
                       0xFE,
                       temp,
                       checksum(temp, MAV_DISTANCE_SENSOR_extra_crc))
    packet_sequence += 1
    uart.write(temp)

MAV_LANDING_TARGET_message_id = 149
MAV_LANDING_TARGET_min_distance = 1/100 # in meters
MAV_LANDING_TARGET_max_distance = 10000/100 # in meters
MAV_LANDING_TARGET_frame = 8 # MAV_FRAME_BODY_NED
MAV_LANDING_TARGET_extra_crc = 200

# http://mavlink.org/messages/common#LANDING_TARGET
# https://github.com/mavlink/c_library_v1/blob/master/common/mavlink_msg_landing_target.h
def send_landing_target_packet(tag, w, h, tag_size):
    global packet_sequence
    temp = struct.pack("<qfffffbb",
                       0,
                       ((tag.cx() / w) - 0.5) * h_fov,
                       ((tag.cy() / h) - 0.5) * v_fov,
                       min(max(z_to_mm(tag.z_translation(), tag_size) / 1000, MAV_LANDING_TARGET_min_distance), MAV_LANDING_TARGET_max_distance),
                       0.0,
                       0.0,
                       0,
                       MAV_LANDING_TARGET_frame)
    temp = struct.pack("<bbbbb30s",
                       30,
                       packet_sequence & 0xFF,
                       MAV_system_id,
                       MAV_component_id,
                       MAV_LANDING_TARGET_message_id,
                       temp)
    temp = struct.pack("<b35sh",
                       0xFE,
                       temp,
                       checksum(temp, MAV_LANDING_TARGET_extra_crc))
    packet_sequence += 1
    uart.write(temp)

# Main Loop

clock = time.clock()
while(True):
    clock.tick()
    img = sensor.snapshot()
    tags = sorted(img.find_apriltags(fx=f_x, fy=f_y, cx=c_x, cy=c_y), key = lambda x: x.w() * x.h(), reverse = True)

    if tags and (tags[0].id() in valid_tag_ids):
        if MAX_DISTANCE_SENSOR_enable: send_distance_sensor_packet(tags[0], valid_tag_ids[tags[0].id()])
        send_landing_target_packet(tags[0], img.width(), img.height(), valid_tag_ids[tags[0].id()])
        img.draw_rectangle(tags[0].rect())
        img.draw_cross(tags[0].cx(), tags[0].cy())
        print("Distance %f mm - FPS %f" % (z_to_mm(tags[0].z_translation(), valid_tag_ids[tags[0].id()]), clock.fps()))
    else:
        print("FPS %f" % clock.fps())
