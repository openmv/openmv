import io
import struct
import marshal
from uopenamp import *

_epts = {}
_MP_STREAM_POLL_WR = const(0x04)
_MP_STREAM_POLL_RD = const(0x01)
_MP_STREAM_POLL_HUP = const(0x10)


async def coro():  # noqa
    pass


class EndpointIO(io.IOBase):
    def __init__(self, ept):
        self.ept = ept

    def write(self, buf):
        if buf != b"\r\n":
            self.ept.send(buf, timeout=0)

    def ioctl(self, op, arg):
        if op == _MP_STREAM_POLL and self.ept.is_ready():
            return _MP_STREAM_POLL_WR
        return 0


def vm_out_callback(src_addr, data):
    print("VM ❯", data.decode())


def vm_ns_callback(src_addr, name):
    print(f'New service announcement src address: {src_addr} name: "{name}"')
    if name == "vm":
        # The "vm" channel is announced. Create our side of the channel
        # and send the tasks' mpy. The other core will create the tasks,
        # and their associated endpoints, and announce them.
        _epts["vm"] = Endpoint("vm", vm_out_callback, dest=src_addr)
        for name in _epts:
            if name != "vm":
                mpy = _epts[name]["mpy"]
                _epts["vm"].send(
                    struct.pack(
                        f"II{len(name)}s{len(mpy)}s",
                        len(name),
                        len(mpy),
                        bytes(name, "utf-8"),
                        mpy,
                    ),
                    timeout=1000,
                )
    else:
        # A task's endpoint is announced. Create our side of the endpoint
        # using the task's callback as the output callback.
        _epts[name] = Endpoint(name, callback=_epts[name]["cb"])


def async_remote(callback=None):
    # Register the new service callback to create endpoints, when the
    # remote processor announces them.
    new_service_callback(vm_ns_callback)

    def decorator(func):
        try:
            name = func.__name__
            mpy = marshal.dumps(func.__code__)
            if len(mpy) + len(name) + 8 > 500:
                raise ValueError("The maximum mpy size supported is 500 bytes")
            _epts[name] = {"cb": callback, "mpy": mpy}
        except Exception as e:
            print(str(e))

    if isinstance(callback, type(coro)):
        # The decorator is called without any args.
        # `callback` is actually the function to be decorated.
        func = callback
        callback = None
        return decorator(func)

    return decorator
