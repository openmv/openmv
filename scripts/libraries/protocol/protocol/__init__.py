# This work is licensed under the MIT license.
# Copyright (c) 2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This is an extension to the protocol module. Freeze this module
# in the board's manifest and CBORChannel will be importable from
# protocol directly.

import time

import cbor2
from uprotocol import *  # noqa: F401


# SenML-compatible CBOR integer keys.
_N = 0        # name
_U = 1        # unit
_V = 2        # numeric value
_VS = 3       # string value
_T = 6        # time
_UT = 7       # update time
_VD = 8       # data value

# 2D data extension keys.
_W = -20      # width
_H = -21      # height
_MIN = -23    # data min
_MAX = -24    # data max

# Widget keys.
_W_TYPE = -30  # widget type
_W_MIN = -31   # slider min
_W_MAX = -32   # slider max
_W_STEP = -33  # slider step
_W_OPTS = -34  # select options

_ITEMSIZE = {"b": 1, "B": 1, "h": 2, "H": 2}


class CBORChannel:
    """A protocol channel backend that serializes named fields to CBOR.

    Supports display widgets (label, text, depth, waveform) and
    interactive controls (toggle, pushbutton, slider, spinbox, select,
    radio, lineedit) with on_read/on_write callbacks. Slider and spinbox
    values can be set as (min, max, value) tuples; waveform values are
    set to interleaved sample bytes.

    Usage::

        from protocol import CBORChannel
        import protocol

        def on_read(ch):
            ch["temp"] = read_temp()

        def on_write(ch, name, value):
            if name == "mirror":
                set_mirror(value)

        ch = CBORChannel(on_read=on_read, on_write=on_write)
        ch.add("temp", type="label", unit="Cel")
        ch.add("mirror", type="toggle")
        protocol.register(name="sensors", backend=ch)
    """

    def __init__(self, on_read=None, on_write=None):
        self._fields = []
        self._index = {}
        self._buf = b""
        self._on_read = on_read
        self._on_write = on_write
        self._dirty = False
        self._t_us = 0
        self._t_prev = time.ticks_us()

    def add(self, name, type, value=None, unit=None,
            min=None, max=None, step=None, options=None,
            width=None, height=None, sample_rate=None,
            series=1, typecode=None):
        """Add a named field to the channel.

        Args:
            name: Display name (must be unique within this channel).
            type: Widget type - "label", "text", "toggle", "pushbutton",
                  "slider", "spinbox", "select", "radio", "lineedit",
                  "depth", or "waveform".
            value: Initial value. Defaults depend on type.
            unit: Unit string for label/slider/spinbox/waveform (e.g.
                "Cel", "%RH").
            min: Minimum value (slider/spinbox range or depth range).
            max: Maximum value (slider/spinbox range or depth range).
            step: Step size (slider/spinbox).
            options: List of option strings (select/radio), or of series
                names (waveform).
            width: Pixel width (depth).
            height: Pixel height (depth).
            sample_rate: Samples per second (waveform).
            series: Interleaved series count (waveform, e.g. 3 for XYZ).
            typecode: array typecode of the waveform samples (e.g. "f",
                  "h"); "H" (uint16) when omitted.
        """
        field = {_N: name, _W_TYPE: type}
        if type == "label":
            field[_V] = value if value is not None else 0
            if unit:
                field[_U] = unit
        elif type in ("text", "lineedit"):
            field[_V] = value if value is not None else ""
        elif type in ("toggle", "pushbutton"):
            field[_V] = value if value is not None else False
        elif type in ("slider", "spinbox"):
            field[_V] = value if value is not None else (min if min is not None else 0)
            field[_W_MIN] = min if min is not None else 0
            field[_W_MAX] = max if max is not None else 100
            field[_W_STEP] = step if step is not None else 1
            if unit:
                field[_U] = unit
        elif type in ("select", "radio"):
            field[_V] = value if value is not None else ""
            field[_W_OPTS] = options if options is not None else []
        elif type == "depth":
            field[_VD] = b""
            field[_W] = width
            field[_H] = height
            field[_MIN] = min if min is not None else 0
            field[_MAX] = max if max is not None else 1000
        elif type == "waveform":
            field[_VD] = b""
            field[_W] = 0
            field[_H] = series
            field[_MIN] = min if min is not None else 0
            field[_MAX] = max if max is not None else 65535
            if unit:
                field[_U] = unit
            if sample_rate:
                field[_UT] = 1.0 / sample_rate
            if typecode is not None:
                field[_VS] = typecode
            if options is not None:
                field[_W_OPTS] = options
        self._index[name] = len(self._fields)
        self._fields.append(field)
        self._serialize()

    def __getitem__(self, name):
        f = self._fields[self._index[name]]
        return f.get(_VD, f.get(_V))

    def __setitem__(self, name, value):
        f = self._fields[self._index[name]]
        if f.get(_W_TYPE) == "waveform":
            f[_VD] = value
            itemsize = _ITEMSIZE.get(f.get(_VS, "H"), 4)
            f[_W] = len(value) // (itemsize * f[_H])
            now = time.ticks_us()
            self._t_us += time.ticks_diff(now, self._t_prev)
            self._t_prev = now
            f[_T] = self._t_us
        elif _VD in f:
            f[_VD] = value
        elif isinstance(value, tuple):
            if f[_W_TYPE] in ("slider", "spinbox"):
                f[_W_MIN] = value[0]
                f[_W_MAX] = value[1]
                f[_V] = value[2]
            else:
                f[_V] = list(value)
        else:
            f[_V] = value
        self._serialize()

    def _serialize(self):
        self._buf = cbor2.dumps(self._fields)
        self._dirty = True

    # -- Protocol backend interface --

    def poll(self):
        if self._on_read is not None:
            return len(self._fields) > 0
        return self._dirty

    def size(self):
        if self._on_read:
            self._on_read(self)
        self._dirty = False
        return len(self._buf)

    def read(self, offset, size):
        end = min(offset + size, len(self._buf))
        return bytes(self._buf[offset:end])

    def write(self, offset, data):
        updates = cbor2.loads(bytes(data))
        for rec in updates:
            name = rec.get(_N)
            value = rec.get(_V)
            if name is None or value is None:
                continue
            idx = self._index.get(name)
            if idx is None:
                continue
            self._fields[idx][_V] = value
            if self._on_write:
                self._on_write(self, name, value)
        return len(data)
