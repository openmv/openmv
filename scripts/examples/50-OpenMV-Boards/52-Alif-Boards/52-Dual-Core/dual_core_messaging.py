# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Dual-Core Messaging Example
#
# This example shows how to pass messages from the HE core to the HP core over
# a dedicated OpenAMP endpoint. The HE task counts up and sends each value to
# the HP core, where it is delivered via the registered callback.

import time
import openamp


# This callback runs on the HP core whenever the HE task calls ept.send().
def task_callback(src_addr, data):
    print("HP received:", data.decode())


# Passing a callback to the decorator binds it to this task's endpoint.
@openamp.async_remote(task_callback)
async def task1(ept):
    import asyncio
    count = 0
    while True:
        ept.send(f"count = {count}")
        count += 1
        await asyncio.sleep(1)


# Start the HE core.
rproc = openamp.RemoteProc(0x80320000)
rproc.start()

while True:
    time.sleep(1)
