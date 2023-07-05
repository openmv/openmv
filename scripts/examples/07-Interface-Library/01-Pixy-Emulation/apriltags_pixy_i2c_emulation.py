# AprilTags Pixy I2C Emulation Script
#
# This script allows your OpenMV Cam to transmit AprilTag detection data like
# a Pixy (CMUcam5) tracking colors in I2C mode. This script allows you to
# easily replace a Pixy (CMUcam5) color tracking sensor with an OpenMV Cam
# AprilTag tracking sensor. Note that this only runs on the OpenMV Cam M7.
#
# P4 = SCL
# P5 = SDA
#
# P7 = Servo 1
# P8 = Servo 2
#
# Note: The tag family is TAG36H11. Additionally, in order to for the
#       signature value of a tag detection to be compatible with pixy
#       interface libraries all tag ids have 8 added to them in order
#       to move them in the color code signature range. Finally, tags
#       are all reported as color code blocks...

import math
import pyb
import sensor
import struct
import time

# Pixy Parameters ############################################################
max_blocks = 1000
max_blocks_per_id = 1000

i2c_address = 0x54

# Pan Servo
s0_lower_limit = 1000  # Servo pulse width lower limit in microseconds.
s0_upper_limit = 2000  # Servo pulse width upper limit in microseconds.

# Tilt Servo
s1_lower_limit = 1000  # Servo pulse width lower limit in microseconds.
s1_upper_limit = 2000  # Servo pulse width upper limit in microseconds.

analog_out_enable = False  # P6 -> Analog Out (0v - 3.3v).
analog_out_mode = 0  # 0 == x position of largest tag - 1 == y position of largest tag

# Camera Setup
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time=2000)

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


def to_object_block_format(tag):
    angle = int((tag.rotation() * 180) // math.pi)
    temp = struct.pack(
        "<hhhhhh", tag.id() + 8, tag.cx(), tag.cy(), tag.w(), tag.h(), angle
    )
    return struct.pack("<hh12s", 0xAA56, checksum(temp), temp)


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
clock = time.clock()
while True:
    clock.tick()
    img = sensor.snapshot()
    tags = img.find_apriltags()  # default TAG36H11 family

    # Transmit Tags #
    if tags and (max_blocks > 0) and (max_blocks_per_id > 0):  # new frame
        dat_buf = struct.pack("<h", 0xAA55)
        id_map = {}
        first_b = False

        for tag in sorted(tags, key=lambda x: x.area(), reverse=True)[0:max_blocks]:
            if not tag.id() in id_map:
                id_map[tag.id()] = 1
            else:
                id_map[tag.id()] += 1

            if id_map[tag.id()] <= max_blocks_per_id:
                dat_buf += to_object_block_format(tag)
                img.draw_rectangle(tag.rect())
                img.draw_cross(tag.cx(), tag.cy())

            if dac and not first_b:
                x_scale = 255 / (img.width() - 1)
                y_scale = 255 / (img.height() - 1)
                dac.write(
                    round(
                        (tag.y() * y_scale) if analog_out_mode else (tag.x() * x_scale)
                    )
                )
                first_b = True

        dat_buf += struct.pack("<h", 0x0000)
        write(dat_buf)  # write all data in one packet...

    else:  # nothing found
        write(struct.pack("<h", 0x0000))

        if dac:
            dac.write(0)

    # Parse Commands #
    for i in range(available()):
        parse_byte(read_byte())

    num_tags = min(len(tags), max_blocks)
    print("%d tags(s) found - FPS %f" % (num_tags, clock.fps()))
