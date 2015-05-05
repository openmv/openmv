import pyb, time

led = pyb.LED(3)

while (vcp_is_connected()==False):
   led.on()
   time.sleep(150)
   led.off()
   time.sleep(100)
   led.on()
   time.sleep(150)
   led.off()
   time.sleep(600)
