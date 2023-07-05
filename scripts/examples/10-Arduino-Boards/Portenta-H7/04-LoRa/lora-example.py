from lora import *

lora = Lora(band=BAND_EU868, poll_ms=60000, debug=False)

print("Firmware:", lora.get_fw_version())
print("Device EUI:", lora.get_device_eui())
print("Data Rate:", lora.get_datarate())
print("Join Status:", lora.get_join_status())

# Example keys for connecting to the backend
appEui = "1234567890123456"
appKey = "12345678901234567890123456789012"

try:
    lora.join_OTAA(appEui, appKey)
    # Or ABP:
    # lora.join_ABP(devAddr, nwkSKey, appSKey, timeout=5000)
# You can catch individual errors like timeout, rx etc...
except LoraErrorTimeout as e:
    print("Something went wrong; are you indoor? Move near a window and retry")
    print("ErrorTimeout:", e)
except LoraErrorParam as e:
    print("ErrorParam:", e)

print("Connected.")
lora.set_port(3)

try:
    if lora.send_data("HeLoRA world!", True):
        print("Message confirmed.")
    else:
        print("Message wasn't confirmed")

except LoraErrorTimeout as e:
    print("ErrorTimeout:", e)

# Read downlink messages
while True:
    if lora.available():
        data = lora.receive_data()
        if data:
            print("Port: " + data["port"])
            print("Data: " + data["data"])
    lora.poll()
    sleep_ms(1000)
