# This work is licensed under the MIT license.
# Copyright (c) 2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# MCP Server Example
#
# This example turns the camera into a Model Context Protocol (MCP) server,
# so AI agents (Claude Code, Claude Desktop, ...) can see through the camera,
# run the on-board models and decode codes. Add it to Claude Code with:
#
#   claude mcp add --transport http openmv http://<camera-ip>/mcp
#
# The library provides the built-in tools (capture_frame, run_model,
# find_qrcodes, find_blobs, ...). The application adds its own tools with
# @mcp.tool. Here, a face detector runs on every frame and the latest
# result is exposed as a tool.

import asyncio
import csi
import ml
import mcp
import network
import time
from ml.postprocessing.mediapipe import BlazeFace

SSID = ""  # Network SSID
KEY = ""  # Network key

wlan = network.WLAN(network.STA_IF)
wlan.active(True)
# Disable WiFi power saving, which delays incoming requests while the radio sleeps.
wlan.config(pm=wlan.PM_NONE)
wlan.connect(SSID, KEY)

while not wlan.isconnected():
    print('Trying to connect to "{:s}"...'.format(SSID))
    time.sleep_ms(1000)

print("MCP endpoint: http://{:s}/mcp".format(wlan.ifconfig()[0]))

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)

model = ml.Model("/rom/blazeface_front_128.tflite", postprocess=BlazeFace(threshold=0.4))

img = None
faces = []


@mcp.tool("Get the faces detected in the latest frame, with bounding boxes and scores.",
          min_score=("number", "Minimum detection score.", 0.4))
def get_faces(min_score):
    return {"faces": [{"rect": list(r), "score": round(s, 2)} for r, s, *_ in faces if s >= min_score]}


async def main():
    global img, faces
    server = mcp.Server(csi=csi0, frame=lambda: img)
    asyncio.create_task(server.serve(port=80))

    while True:
        # The server only runs when this loop yields, so yield after each
        # step to keep request latency low.
        img = csi0.snapshot()
        await asyncio.sleep_ms(0)
        faces = model.predict([img])
        await asyncio.sleep_ms(0)


asyncio.run(main())
