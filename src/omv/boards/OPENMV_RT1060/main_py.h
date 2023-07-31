static const char fresh_main_py[] =
"# main.py -- put your code here!\n"
"import machine, time\n"
"led = machine.LED(3)\n"
"while (True):\n"
"   led.on()\n"
"   time.sleep_ms(150)\n"
"   led.off()\n"
"   time.sleep_ms(100)\n"
"   led.on()\n"
"   time.sleep_ms(150)\n"
"   led.off()\n"
"   time.sleep_ms(600)\n"
;
