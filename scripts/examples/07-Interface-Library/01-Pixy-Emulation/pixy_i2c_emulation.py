# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Pixy I2C Emulation Script
#
# This script allows your OpenMV Cam to emulate the Pixy (CMUcam5) in I2C mode.
# Note that you need to setup the lab color thresholds below for your application.
#
# P4 = SCL
# P5 = SDA
#
# P7 = Servo 1
# P8 = Servo 2
#
# Pixy Parameters ############################################################

import math
import pyb
import sensor
import struct
import time

color_code_mode = 1  # 0 == Disabled, 1 == Enabled, 2 == Color Codes Only, 3 == Mixed

max_blocks = 1000
max_blocks_per_signature = 1000
min_block_area = 20

i2c_address = 0x54

# Pan Servo
s0_lower_limit = 1000  # Servo pulse width lower limit in microseconds.
s0_upper_limit = 2000  # Servo pulse width upper limit in microseconds.

# Tilt Servo
s1_lower_limit = 1000  # Servo pulse width lower limit in microseconds.
s1_upper_limit = 2000  # Servo pulse width upper limit in microseconds.

analog_out_enable = False  # P6 -> Analog Out (0v - 3.3v).
analog_out_mode = 0  # 0 == x position of largest blob - 1 == y position of largest blob

# Parameter 0 - L Min.
# Parameter 1 - L Max.
# Parameter 2 - A Min.
# Parameter 3 - A Max.
# Parameter 4 - B Min.
# Parameter 5 - B Max.
# Parameter 6 - Is Color Code Threshold? (True/False).
# Parameter 7 - Enable Threshold? (True/False).
lab_color_thresholds = [
    (0, 100, 40, 127, -128, 127, True, True),  # Generic Red Threshold
    (0, 100, -128, -10, -128, 127, True, True),  # Generic Green Threshold
    (0, 0, 0, 0, 0, 0, False, False),
    (0, 0, 0, 0, 0, 0, False, False),
    (0, 0, 0, 0, 0, 0, False, False),
    (0, 0, 0, 0, 0, 0, False, False),
    (0, 0, 0, 0, 0, 0, False, False),
]

fb_pixels_threshold = 500  # minimum number of pixels that must be in a blob
fb_merge_margin = 5  # how close pixel wise blobs can be before merging

##############################################################################

e_lab_color_thresholds = []  # enabled thresholds
e_lab_color_code = []  # enabled color code
e_lab_color_signatures = []  # original enabled threshold indexes
for i in range(len(lab_color_thresholds)):
    if lab_color_thresholds[i][7]:
        e_lab_color_thresholds.append(lab_color_thresholds[i][0:6])
        e_lab_color_code.append(lab_color_thresholds[i][6])
        e_lab_color_signatures.append(i + 1)

# Camera Setup
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)

# LED Setup
red_led = pyb.LED(1)
green_led = pyb.LED(2)
blue_led = pyb.LED(3)

red_led.off()
green_led.off()
blue_led.off()

# DAC Setup
dac = pyb.DAC("P6") if analog_out_enable else None

if dac:
    dac.write(0)

# Servo Setup
min_s0_limit = min(s0_lower_limit, s0_upper_limit)
max_s0_limit = max(s0_lower_limit, s0_upper_limit)
min_s1_limit = min(s1_lower_limit, s1_upper_limit)
max_s1_limit = max(s1_lower_limit, s1_upper_limit)

s0_pan = pyb.Servo(1)  # P7
s1_tilt = pyb.Servo(2)  # P8

s0_pan.pulse_width(int((max_s0_limit - min_s0_limit) // 2))  # center
s1_tilt.pulse_width(int((max_s1_limit - min_s1_limit) // 2))  # center

s0_pan_conversion_factor = (max_s0_limit - min_s0_limit) / 1000
s1_tilt_conversion_factor = (max_s1_limit - min_s1_limit) / 1000


def s0_pan_position(value):
    s0_pan.pulse_width(
        round(s0_lower_limit + (max(min(value, 1000), 0) * s0_pan_conversion_factor))
    )


def s1_tilt_position(value):
    s1_tilt.pulse_width(
        round(s1_lower_limit + (max(min(value, 1000), 0) * s1_tilt_conversion_factor))
    )


# Link Setup
bus = pyb.I2C(2, pyb.I2C.SLAVE, addr=i2c_address)


def write(data):
    # Prepare the data to transmit first so we can do it quickly.
    out_data = []
    for i in range(0, len(data), 2):
        out_data.append(data[i : i + 2])
    # Disable interrupts so we can send all packets without gaps.
    state = pyb.disable_irq()
    for i in range(len(out_data)):
        max_exceptions = 10
        loop = True
        while loop:
            try:
                bus.send(out_data[i], timeout=1)
                loop = False
            except OSError as error:
                if max_exceptions <= 0:
                    pyb.enable_irq(state)
                    return
                max_exceptions -= 1
    pyb.enable_irq(state)


def available():
    return 0  # Not implemented as there is no way for the us to be ready to receive the data.


def read_byte():
    return 0  # Not implemented as there is no way for the us to be ready to receive the data.


def checksum(data):
    checksum = 0
    for i in range(0, len(data), 2):
        checksum += ((data[i + 1] & 0xFF) << 8) | ((data[i + 0] & 0xFF) << 0)
    return checksum & 0xFFFF


def get_normal_signature(code):
    for i in range(len(e_lab_color_signatures)):
        if code & (1 << i):
            return e_lab_color_signatures[i]
    return 0


def to_normal_object_block_format(blob):
    temp = struct.pack(
        "<hhhhh",
        get_normal_signature(blob.code()),
        blob.cx(),
        blob.cy(),
        blob.w(),
        blob.h(),
    )
    return struct.pack("<hh10s", 0xAA55, checksum(temp), temp)


def get_color_code_signature(code):
    color_code_list = []
    for i in range(len(e_lab_color_signatures)):
        if code & (1 << i):
            color_code_list.append(e_lab_color_signatures[i])
    octal = 0
    color_code_list_len = len(color_code_list) - 1
    for i in range(color_code_list_len + 1):
        octal += color_code_list[i] << (3 * (color_code_list_len - i))
    return octal


def to_color_code_object_block_format(blob):
    angle = int((blob.rotation() * 180) // math.pi)
    temp = struct.pack(
        "<hhhhhh",
        get_color_code_signature(blob.code()),
        blob.cx(),
        blob.cy(),
        blob.w(),
        blob.h(),
        angle,
    )
    return struct.pack("<hh12s", 0xAA56, checksum(temp), temp)


def get_signature(blob, bits):
    return (
        get_normal_signature(blob.code())
        if (bits == 1)
        else get_color_code_signature(blob.code())
    )


def to_object_block_format(blob, bits):
    return (
        to_normal_object_block_format(blob)
        if (bits == 1)
        else to_color_code_object_block_format(blob)
    )


# FSM Code
fsm_state = 0
last_byte = 0

FSM_STATE_NONE = 0
FSM_STATE_ZERO = 1
FSM_STATE_SERVO_CONTROL_0 = 2
FSM_STATE_SERVO_CONTROL_1 = 3
FSM_STATE_SERVO_CONTROL_2 = 4
FSM_STATE_SERVO_CONTROL_3 = 5
FSM_STATE_CAMERA_CONTROL = 6
FSM_STATE_LED_CONTROL_0 = 7
FSM_STATE_LED_CONTROL_1 = 8
FSM_STATE_LED_CONTROL_2 = 9


def parse_byte(byte):
    global fsm_state
    global last_byte

    if fsm_state == FSM_STATE_NONE:
        if byte == 0x00:
            fsm_state = FSM_STATE_ZERO
        else:
            fsm_state = FSM_STATE_NONE

    elif fsm_state == FSM_STATE_ZERO:
        if byte == 0xFF:
            fsm_state = FSM_STATE_SERVO_CONTROL_0
        elif byte == 0xFE:
            fsm_state = FSM_STATE_CAMERA_CONTROL
        elif byte == 0xFD:
            fsm_state = FSM_STATE_LED_CONTROL_0
        else:
            fsm_state = FSM_STATE_NONE

    elif fsm_state == FSM_STATE_SERVO_CONTROL_0:
        fsm_state = FSM_STATE_SERVO_CONTROL_1

    elif fsm_state == FSM_STATE_SERVO_CONTROL_1:
        fsm_state = FSM_STATE_SERVO_CONTROL_2
        s0_pan_position(((byte & 0xFF) << 8) | ((last_byte & 0xFF) << 0))

    elif fsm_state == FSM_STATE_SERVO_CONTROL_2:
        fsm_state = FSM_STATE_SERVO_CONTROL_3

    elif fsm_state == FSM_STATE_SERVO_CONTROL_3:
        fsm_state = FSM_STATE_NONE
        s1_tilt_position(((byte & 0xFF) << 8) | ((last_byte & 0xFF) << 0))

    elif fsm_state == FSM_STATE_CAMERA_CONTROL:
        fsm_state = FSM_STATE_NONE
        # Ignore...

    elif fsm_state == FSM_STATE_LED_CONTROL_0:
        fsm_state = FSM_STATE_LED_CONTROL_1
        if byte & 0x80:
            red_led.on()
        else:
            red_led.off()

    elif fsm_state == FSM_STATE_LED_CONTROL_1:
        fsm_state = FSM_STATE_LED_CONTROL_2
        if byte & 0x80:
            green_led.on()
        else:
            green_led.off()

    elif fsm_state == FSM_STATE_LED_CONTROL_2:
        fsm_state = FSM_STATE_NONE
        if byte & 0x80:
            blue_led.on()
        else:
            blue_led.off()

    last_byte = byte


# Main Loop
pri_color_code_mode = color_code_mode % 4


def bits_set(code):
    count = 0
    for i in range(7):
        count += 1 if (code & (1 << i)) else 0
    return count


def color_code(code):
    for i in range(len(e_lab_color_code)):
        if code & (1 << i):
            return e_lab_color_code[i]
    return False


def fb_merge_cb(blob0, blob1):
    if not pri_color_code_mode:
        return blob0.code() == blob1.code()
    else:
        return (
            True
            if (blob0.code() == blob1.code())
            else (color_code(blob0.code()) and color_code(blob1.code()))
        )


def blob_filter(blob):
    if pri_color_code_mode == 0:
        return True
    elif pri_color_code_mode == 1:  # color codes with two or more colors or regular
        return (bits_set(blob.code()) > 1) or (not color_code(blob.code()))
    elif pri_color_code_mode == 2:  # only color codes with two or more colors
        return bits_set(blob.code()) > 1
    elif pri_color_code_mode == 3:
        return True


clock = time.clock()
while True:
    clock.tick()
    img = sensor.snapshot()
    blobs = list(
        filter(
            blob_filter,
            img.find_blobs(
                e_lab_color_thresholds,
                area_threshold=min_block_area,
                pixels_threshold=fb_pixels_threshold,
                merge=True,
                margin=fb_merge_margin,
                merge_cb=fb_merge_cb,
            ),
        )
    )

    # Transmit Blobs
    if blobs and (max_blocks > 0) and (max_blocks_per_signature > 0):  # new frame
        dat_buf = struct.pack("<h", 0xAA55)
        sig_map = {}
        first_b = False

        for blob in sorted(blobs, key=lambda x: x.area(), reverse=True)[0:max_blocks]:
            bits = bits_set(blob.code())
            sign = get_signature(blob, bits)

            if not sign in sig_map:
                sig_map[sign] = 1
            else:
                sig_map[sign] += 1

            if sig_map[sign] <= max_blocks_per_signature:
                dat_buf += to_object_block_format(blob, bits)
                img.draw_rectangle(blob.rect())
                img.draw_cross(blob.cx(), blob.cy())

            if dac and not first_b:
                x_scale = 255 / (img.width() - 1)
                y_scale = 255 / (img.height() - 1)
                dac.write(
                    round(
                        (blob.y() * y_scale)
                        if analog_out_mode
                        else (blob.x() * x_scale)
                    )
                )
                first_b = True

        dat_buf += struct.pack("<h", 0x0000)
        write(dat_buf)  # write all data in one packet...

    else:  # nothing found
        write(struct.pack("<h", 0x0000))

        if dac:
            dac.write(0)

    # Parse Commands
    for i in range(available()):
        parse_byte(read_byte())

    num_blobs = min(len(blobs), max_blocks)
    print("%d blob(s) found - FPS %f" % (num_blobs, clock.fps()))
