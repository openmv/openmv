# SPI with the Arduino as the master device and the OpenMV Cam as the slave.
#
# Please wire up your OpenMV Cam to your Arduino like this:
#
# OpenMV Cam Master Out Slave In (P0) - Arduino Uno MOSI (11)
# OpenMV Cam Master In Slave Out (P1) - Arduino Uno MISO (12)
# OpenMV Cam Serial Clock        (P2) - Arduino Uno SCK  (13)
# OpenMV Cam Slave Select        (P3) - Arduino Uno SS   (10)
# OpenMV Cam Ground                   - Arduino Ground

import pyb
import ustruct
import time

text = "Hello World!\n"
data = ustruct.pack("<bi%ds" % len(text), 85, len(text), text) # 85 is a sync char.
# Use "ustruct" to build data packets to send.
# "<" puts the data in the struct in little endian order.
# "b" puts a signed char in the data stream.
# "i" puts a signed integer in the data stream.
# "%ds" puts a string in the data stream. E.g. "13s" for "Hello World!\n" (13 chars).
# See https://docs.python.org/3/library/struct.html

# READ ME!!!
#
# Please understand that when your OpenMV Cam is not the SPI master it may miss responding to
# sending data as a SPI slave no matter if you call "spi.send()" in an interrupt callback or in the
# main loop below. Because of this you must absolutely design your communications protocol such
# that if the slave device (the OpenMV Cam) doesn't call "spi.send()" in time that garbage data
# read from the SPI peripheral by the master (the Arduino) is tossed. To accomplish this we use a
# sync character of 85 (binary 01010101) which the Arduino will look for as the first byte read.
# If it doesn't see this then it aborts the SPI transaction and will try again. Second, in order to
# clear out the SPI peripheral state we always send a multiple of four bytes and an extra four zero
# bytes to make sure that the SPI peripheral doesn't hold any stale data which could be 85. Note
# that the OpenMV Cam will miss the "spi.send()" window randomly because it has to service
# interrupts. Interrupts will happen a lot more while connected to your PC.

# The hardware SPI bus for your OpenMV Cam is always SPI bus 2.
# polarity = 0 -> clock is idle low.
# phase = 0 -> sample data on rising clock edge, output data on falling clock edge.
spi = pyb.SPI(2, pyb.SPI.SLAVE, polarity=0, phase=0)

# NSS callback.
def nss_callback(line):
    global spi, data
    try:
        spi.send(data, timeout=1000)
    except OSError as err:
        pass # Don't care about errors - so pass.
        # Note that there are 3 possible errors. A timeout error, a general purpose error, or
        # a busy error. The error codes are 116, 5, 16 respectively for "err.arg[0]".

# Configure NSS/CS in IRQ mode to send data when requested by the master.
pyb.ExtInt(pyb.Pin("P3"), pyb.ExtInt.IRQ_FALLING, pyb.Pin.PULL_UP, nss_callback)

while(True):
    time.sleep_ms(1000)

###################################################################################################
# Arduino Code
###################################################################################################
#
# #include <SPI.h>
# #define SS_PIN 10
# #define BAUD_RATE 19200
# #define CHAR_BUF 128
# 
# void setup() {
#   pinMode(SS_PIN, OUTPUT);
#   Serial.begin(BAUD_RATE);
#   SPI.begin();
#   SPI.setBitOrder(MSBFIRST);
#   SPI.setClockDivider(SPI_CLOCK_DIV16);
#   SPI.setDataMode(SPI_MODE0);
#   delay(1000); // Give the OpenMV Cam time to bootup.
# }
# 
# void loop() {
#   int32_t len = 0;
#   char buff[CHAR_BUF] = {0};
#   digitalWrite(SS_PIN, LOW);
#   delay(1); // Give the OpenMV Cam some time to setup to send data.
# 
#   if(SPI.transfer(1) == 85) { // saw sync char?
#     SPI.transfer(&len, 4); // get length
#     if (len) {
#       SPI.transfer(&buff, min(len, CHAR_BUF));
#       len -= min(len, CHAR_BUF);
#     }
#     while (len--) SPI.transfer(0); // eat any remaining bytes
#   }
# 
#   digitalWrite(SS_PIN, HIGH);
#   Serial.print(buff);
#   delay(1); // Don't loop to quickly.
# }
