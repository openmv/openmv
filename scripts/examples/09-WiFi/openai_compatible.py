# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# OpenAI compatible API example.
#
# This example shows how to snapshot an image and send it to an
# OpenAI compatible chat completions API for scene description.

import binascii
import csi
import network
import requests
import time

SSID = ""
PASSWORD = ""

OPENAI_API_BASE = "https://api.openai.com/v1"
OPENAI_API_KEY = ""
MODEL = "gpt-5.5"

# Init wlan module and connect to network
wlan = network.WLAN(network.STA_IF)
print('Trying to connect to "{:s}"...'.format(SSID))
wlan.active(True)
wlan.connect(SSID, PASSWORD)

while not wlan.isconnected():
    print("Waiting for WiFi... status:", wlan.status())
    time.sleep_ms(500)

print("WiFi Connected ", wlan.ifconfig())

# Init camera and wait for auto exposure/white balance to settle.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)


def openai_compatible(text, image_url, model=MODEL):
    url = OPENAI_API_BASE + "/chat/completions"
    headers = {"Authorization": "Bearer " + OPENAI_API_KEY}
    data = {
        "model": model,
        "max_tokens": 128,
        "messages": [
            {
                "role": "user",
                "content": [
                    {"type": "image_url", "image_url": {"url": image_url}},
                    {"type": "text", "text": text},
                ],
            }
        ],
    }

    response = requests.post(url, headers=headers, json=data, timeout=60)
    print("HTTP status:", response.status_code, response.reason)
    return response.json()["choices"][0]["message"]["content"]


jpeg = csi0.snapshot().to_jpeg()
print("JPEG size:", jpeg.size())

encoded = binascii.b2a_base64(jpeg.bytearray())
if encoded[-1:] == b"\n":
    encoded = encoded[:-1]
image_url = "data:image/jpeg;base64," + encoded.decode()

result = openai_compatible(
    "Answer in one sentence: what scene is depicted in the image?",
    image_url,
)
print("Response:", result)
