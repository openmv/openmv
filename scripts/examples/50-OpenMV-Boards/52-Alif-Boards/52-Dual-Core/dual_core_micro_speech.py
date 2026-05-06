# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Dual-Core MicroSpeech Example
#
# This example runs the MicroSpeech keyword spotter on the HE core and forwards
# detected keywords to the HP core. The HP core remains free for other work
# (e.g. image processing) while keyword spotting runs continuously in the
# background on the low-power core.
#
# The built-in MicroSpeech model only recognizes the keywords "Yes" and "No".

import time
import openamp


def task_callback(src_addr, data):
    print("Heard:", data.decode())


@openamp.async_remote(task_callback)
async def task1(ept):
    from ml.apps import MicroSpeech
    speech = MicroSpeech(gain_db=24)
    while True:
        label, scores = speech.listen(timeout=0, threshold=0.70)
        if label:
            ept.send(label)


# Start the HE core.
rproc = openamp.RemoteProc(0x80320000)
rproc.start()

while True:
    time.sleep(1)
