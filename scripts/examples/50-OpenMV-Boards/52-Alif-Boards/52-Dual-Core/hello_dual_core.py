# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Hello Dual-Core Example
#
# The Alif AE3 has two cores: the High-Performance (HP) core that runs the main
# script, and the High-Efficiency (HE) core for low-power background work. This
# example starts a task on the HE core. Its print() output is forwarded over
# the default OpenAMP endpoint to the HP core's stdout.

import time
import openamp


# This async function runs on the HE core.
@openamp.async_remote
async def task1(ept):
    import asyncio
    while True:
        print("Hello from the HE core!")
        await asyncio.sleep(1)


# Boot the HE core and start the registered tasks.
rproc = openamp.RemoteProc(0x80320000)
rproc.start()

# This loop runs on the HP core.
while True:
    print("Hello from the HP core!")
    time.sleep(1)
