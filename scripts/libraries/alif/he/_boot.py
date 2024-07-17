import os
import openamp
import asyncio
from asyncio import CancelledError
import struct
import marshal
from types import FunctionType


# MicroPython doesn't have this exception
class InvalidStateError(Exception):
    pass


class DoneException(Exception):
    pass


tasks = {}
_epts = {}


def vm_out_callback(src_addr, data):
    try:
        nlen, mlen = struct.unpack("II", data[0:8])
        name, mpy = struct.unpack(f"{nlen}s{mlen}s", data[8:])
        name = name.decode()
        g = FunctionType(marshal.loads(mpy), globals())
        ept = openamp.Endpoint(name)
        tasks[name] = asyncio.create_task(g(ept))
    except Exception as e:
        print(str(e))


async def main():
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
        await asyncio.sleep(0.5)
        print(f"running tasks:{len(tasks)}")


try:
    os.chdir("/rom")
except:
    pass

try:
    # openamp.new_service_callback(vm_ns_callback)
    # Create the RPMsg endpoints to communicate with HP core.
    _epts["vm"] = openamp.Endpoint("vm", callback=vm_out_callback)
    os.dupterm(openamp.EndpointIO(_epts["vm"]), 0)
    asyncio.run(main())
except Exception as e:
    print(e)
