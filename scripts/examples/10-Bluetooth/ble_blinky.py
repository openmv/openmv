# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Bluetooth Blinky Example
#
# Use nRFConnect app from the App store, connect to the board
# "mpy-blinky", and write bool (True/False) to control the LED.

import struct
import asyncio
import aioble
import bluetooth
from machine import LED
from micropython import const

# UUIDs
_LED_CONTROL_UUID = bluetooth.UUID(0x1815)
_LED_CONTROL_CHAR_UUID = bluetooth.UUID(0x2A56)
_ADV_APPEARANCE_GENERIC_TAG = const(512)
_ADV_INTERVAL_MS = 250_000

# LED setup
led = LED("LED_BLUE")

# GATT server
led_service = aioble.Service(_LED_CONTROL_UUID)
led_characteristic = aioble.Characteristic(led_service, _LED_CONTROL_CHAR_UUID, write=True, capture=True)
aioble.register_services(led_service)


async def led_control_task():
    while True:
        conn, data = await led_characteristic.written()
        led.value(struct.unpack("<B", data)[0])


async def peripheral_task():
    while True:
        async with await aioble.advertise(
            _ADV_INTERVAL_MS,
            name="mpy-blinky",
            services=[_LED_CONTROL_UUID],
            appearance=_ADV_APPEARANCE_GENERIC_TAG,
        ) as connection:
            print("Connection from", connection.device)
            await connection.disconnected()


async def main():
    await asyncio.gather(led_control_task(), peripheral_task())


asyncio.run(main())
