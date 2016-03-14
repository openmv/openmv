# Copy this module to storage and import it if you want
# to use it in your own scripts. See example usage below.
from time import sleep
from pyb import Pin, UART

class BLE:
    def reset(self):
        self.rst.low()
        sleep(100)
        self.rst.high()
        sleep(100)
        
    def _write_cmd(self, cmd):
        self.uart.write(cmd)
        if (self.uart.any()):
            print(self.uart.readline())
        		
    def set_discoverable(self, discoverable):
        if (discoverable):
            self._write_cmd('adv high\r\n')
        else:
            self._write_cmd('adv off\r\n')
            
    def scan(self):
        devices = []
        #Scan all
        self._write_cmd('scan high all dup\r\n')
        
        # Read scanning results
        sleep(100)
        for i in range(100):
            if (self.uart.any()):
                devices.append(self.uart.readline())
                sleep(10)
                                
        self._write_cmd('scan off\r\n') #Scan off
        return devices
        
    def __init__(self):
        self.rst = Pin('P7', Pin.OUT_PP, Pin.PULL_UP)
        self.uart = UART(3, 115200, bits=8, parity=None, stop=1, flow=UART.RTS|UART.CTS)

if __name__ == "__main__":
    ble = BLE()
    ble.reset()
    ble.set_discoverable(True)
    devices = ble.scan()
    for d in devices:
        print(d)
