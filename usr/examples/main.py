import led, time
while (vcp_is_connected()==False):
   led.on(led.BLUE)
   time.sleep(150)
   led.off(led.BLUE)
   time.sleep(100)
   led.on(led.BLUE)
   time.sleep(150)
   led.off(led.BLUE)
   time.sleep(600)
