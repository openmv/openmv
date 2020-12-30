#  OpenMV M7 I2C interface with Garmin Lidar Lite V3 - By: Grant Phillips - Sun Apr 8 2018


#  Returns a basic distance reading from the lidar in cm for the target point and prints to console
#  Uses default lidar settings. For more advanced settings, see the I2C commands in the manual:
#  https://static.garmin.com/pumac/LIDAR_Lite_v3_Operation_Manual_and_Technical_Specifications.pdf

#  I2C Control of LIDAR Lite V3
#  1. Write 0x04 to register 0x00
#  2. Read register 0x01. Repeat until bit 0 (LSB) goes low.
#  3. Read two bytes from 0x8f (high byte 0x0f then low byte 0x10) to obtain 16 bit measurement in cm

#  HARDWARE CONNECTIONS:
#  Connect the lidar SCL line (green) to I2C 2 SCL on openMV (Pin 4)
#  Connect the lidar SDA line (blue) to I2C 2 SDA on openMV (pin 5)
#  680uF filter capacitor in parallel with the lidar
#  10k pullup resistors on the SCL and SDA lines to +5Vdc


import pyb
from pyb import I2C


lidarReady = bytearray([0xff])          #  holds the returned data for ready check
lidarReadyCheck = bytes([1])            #  to compare bit 0 of lidarReady

startBuf = bytearray([0x00,0x04])       #  step 1 address and data
readyBuf = bytearray([0x01])            #  step 2 address for readiness check
distBuf = bytearray([0x8f])             #  step 3 address for distance reading
distance = -1                           #  variable for distance reading

#  I2C setup
Lidar=I2C(2,I2C.MASTER)                 #  initialise I2C 2 bus in master mode


while(True):
    distance = -1                       #  reset to -1 so we know when we get a real reading

    try:                                #  handles errors thrown up if we have an I2C error
    #  Step 1 Write 0x04 to register 0x00
        Lidar.send(startBuf,0x62)       #  this is making it read (laser visible)

    #  Step 2 Read register 0x01 and wait for bit 0 to go low
        while (lidarReady[0] & readyBuf[0]):
            Lidar.send(readyBuf,0x62)
            lidarReady=Lidar.recv(1,0x62)
            pyb.delay(50)               #  This seems to help reduce errors on the I2C bus
        lidarReady=bytearray([0xff])    #  reset the ready check data for next reading

    #  Step 3 Read the distance measurement from 0x8f (0x0f and 0x10)
        Lidar.send(distBuf,0x62)
        dist=Lidar.recv(2,0x62)
        distance=dist[0]
        distance<<=8                    #  move 2 bytes into a 16 bit int
        distance|=dist[1]
        pyb.delay(100)                  #  allow time between readings, can go faster but more errors

    except OSError:                     #  reninitialise i2c bus if error
        Lidar.init(I2C.MASTER)
        print("error, reinitialising")

    if distance > -1:
        print("Distance:", distance, "cm")
