# LSM6DSOX IMU MLC (Machine Learning Core) Example.
# Download the raw UCF file, copy to storage and reset.

# NOTE: The pre-trained models (UCF files) for the examples can be found here:
# https://github.com/STMicroelectronics/STMems_Machine_Learning_Core/tree/master/application_examples/lsm6dsox

from machine import Pin
from machine import SPI
from lsm6dsox import LSM6DSOX

INT_MODE = True  # Run in interrupt mode.
INT_FLAG = False  # Set True on interrupt.


def imu_int_handler(pin):
    global INT_FLAG
    INT_FLAG = True


if INT_MODE is True:
    int_pin = Pin("PA1", mode=Pin.IN, pull=Pin.PULL_UP)
    int_pin.irq(handler=imu_int_handler, trigger=Pin.IRQ_RISING)

# Vibration detection example
UCF_FILE = "lsm6dsox_vibration_monitoring.ucf"
UCF_LABELS = {0: "no vibration", 1: "low vibration", 2: "high vibration"}
# NOTE: Selected data rate and scale must match the MLC data rate and scale.
lsm = LSM6DSOX(
    SPI(5),
    cs=Pin("PF6", Pin.OUT_PP, Pin.PULL_UP),
    gyro_odr=26,
    accel_odr=26,
    gyro_scale=2000,
    accel_scale=4,
    ucf=UCF_FILE,
)

# Head gestures example
# UCF_FILE = "lsm6dsox_head_gestures.ucf"
# UCF_LABELS = {0:"Nod", 1:"Shake", 2:"Stationary", 3:"Swing", 4:"Walk"}
# NOTE: Selected data rate and scale must match the MLC data rate and scale.
# lsm = LSM6DSOX(SPI(5), cs_pin=Pin("PF6", Pin.OUT_PP, Pin.PULL_UP),
#        gyro_odr=26, accel_odr=26, gyro_scale=250, accel_scale=2, ucf=UCF_FILE)

print("MLC configured...")

while True:
    if INT_MODE:
        if INT_FLAG:
            INT_FLAG = False
            print(UCF_LABELS[lsm.mlc_output()[0]])
    else:
        buf = lsm.mlc_output()
        if buf is not None:
            print(UCF_LABELS[buf[0]])
