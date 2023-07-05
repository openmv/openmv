from time import sleep_ms
from machine import Pin, I2C

from apds9960 import uAPDS9960 as APDS9960

bus = I2C(1, sda=Pin(13), scl=Pin(14))
apds = APDS9960(bus)

apds.setProximityIntLowThreshold(50)

print("Proximity Sensor Test")
print("=====================")
apds.enableProximitySensor()

while True:
    sleep_ms(250)
    val = apds.readProximity()
    print("proximity={}".format(val))
