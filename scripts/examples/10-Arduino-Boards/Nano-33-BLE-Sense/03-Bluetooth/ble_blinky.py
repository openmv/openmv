# Use nRF Connect from App store, connect to the Nano and write 1/0 to control the LED.

import time
from board import LED
from ubluepy import Service, Characteristic, UUID, Peripheral, constants


def event_handler(id, handle, data):
    global periph
    global service
    if id == constants.EVT_GAP_CONNECTED:
        pass
    elif id == constants.EVT_GAP_DISCONNECTED:
        # restart advertisment
        periph.advertise(device_name="Nano Blinky", services=[service])
    elif id == constants.EVT_GATTS_WRITE:
        LED(1).on() if int(data[0]) else LED(1).off()


# start off with LED(1) off
LED(1).off()

notif_enabled = False
uuid_service = UUID("0x1523")
uuid_led = UUID("0x1525")

service = Service(uuid_service)
char_led = Characteristic(uuid_led, props=Characteristic.PROP_WRITE)
service.addCharacteristic(char_led)

periph = Peripheral()
periph.addService(service)
periph.setConnectionHandler(event_handler)
periph.advertise(device_name="Nano Blinky", services=[service])

while True:
    time.sleep_ms(500)
