# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Dual-Core ML Regression Example
#
# This example runs a TensorFlow Lite regression model on the HE core and
# sends the prediction to the HP core once per second. This frees the HP core
# to do other work (e.g. camera processing) while inferences happen in the
# background on the low-power core.

import time
import openamp


def task_callback(src_addr, data):
    print("Prediction:", data.decode())


@openamp.async_remote(task_callback)
async def task1(ept):
    import ml
    import asyncio
    from ulab import numpy as np

    model = ml.Model("/rom/force_int_quant.tflite")
    i = np.array([-3, -1, -2,  5, -2, 10, -1, 9, 0, 2, 0, 9,    # noqa
                   1, 10,  2, -1,  3,  5,  3, 9, 3, 9, 6, 2,    # noqa
                   6,  7,  5, 10,  6, -1,  7, 4, 7, 8, 5, 7],   # noqa
                  dtype=np.int8).reshape(model.input_shape[0])  # noqa

    while True:
        ept.send(str(model.predict([i])[0]))
        await asyncio.sleep(1)


rproc = openamp.RemoteProc(0x80320000)
rproc.start()

while True:
    time.sleep(1)
