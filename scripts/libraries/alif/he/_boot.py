import os
import openamp
import asyncio
from asyncio import CancelledError
from machine import Pin
import micropython
import sys
import alif
import vfs


# MicroPython doesn't have this exception
class InvalidStateError(Exception):
    pass


class DoneException(Exception):
    pass


tasks = {}
_epts = {}


def vm_out_callback(src_addr, data):
    try:
        name = "task%d" % (len(tasks))
        g = micropython.fun_from_mpy(data, {})
        ept = openamp.Endpoint(name)
        tasks[name] = asyncio.create_task(g(ept))
    except Exception as e:
        print(str(e))


async def main():
    led = Pin("LED_GREEN", Pin.OUT)
    while True:
        if tasks:
            task_except = None
            try:
                await asyncio.gather(*tasks.values(), return_exceptions=False)
            except Exception as e:
                task_except = e

            for name in list(tasks):
                task = tasks[name]
                try:
                    if task.done():
                        tasks.pop(name)
                        if not isinstance(task_except, DoneException):
                            print(f'Task "{name}" raised: {task_except}')
                        break  # Break after the first task is removed.
                except (CancelledError, InvalidStateError):
                    pass
        led(not led())
        await asyncio.sleep(0.5)
        print(f"running tasks:{len(tasks)}")


bdev = None
try:
    bdev = vfs.VfsRom(alif.Flash(id="romfs"))
    vfs.mount(bdev, "/rom")
    sys.path.append("/rom")
    os.chdir("/rom")
except:
    pass

del sys, alif, vfs, bdev

try:
    # openamp.new_service_callback(vm_ns_callback)
    # Create the RPMsg endpoints to communicate with HP core.
    _epts["vm"] = openamp.Endpoint("vm", callback=vm_out_callback)
    os.dupterm(openamp.EndpointIO(_epts["vm"]), 0)
    asyncio.run(main())
except Exception as e:
    print(e)
