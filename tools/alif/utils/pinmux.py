""" pinmuxing """
import json
import ctypes
import struct

# pylint: disable=too-few-public-methods
# pylint: disable=too-many-instance-attributes
# pylint: disable=invalid-name
# pylint: disable=attribute-defined-outside-init

c_uint32 = ctypes.c_uint32
c_uint8 = ctypes.c_uint8

class pad_configuration_bits(ctypes.LittleEndianStructure):
    """ fw_rgn_mpe """
    _fields_ = [
                ("port",              c_uint32, 8),
                ("pin",               c_uint32, 8),
                ("mux_mode",          c_uint32, 7),
                ("pad_control_valid", c_uint32, 1),
                ("pad_control",       c_uint32, 8)
               ]

    def init_from_cfg(self, cfg):
        """ Initialize from JSON configuration """
        self.port = cfg['port']
        self.pin = cfg['pin']
        self.mux_mode = cfg['mux_mode']
        self.pad_control_valid = 1
        self.pad_control = int(cfg['pad_config'], 0)

class pad_configuration_bytes(ctypes.LittleEndianStructure):
    """ fw_rgn_mpe """
    _fields_ = [
                ("b1", c_uint8),
                ("b2", c_uint8),
                ("b3", c_uint8),
                ("b4", c_uint8)
               ]

class pad_configuration(ctypes.Union):
    _fields_ = [
                ("bits", pad_configuration_bits), 
                ("bytes", pad_configuration_bytes)
               ]

    def init_from_cfg(self, cfg):
        """ Initialize from JSON configuration """
        self.bits.port = cfg['port']
        self.bits.pin = cfg['pin']
        self.bits.mux_mode = cfg['mux_mode']
        self.bits.pad_control_valid = 1
        self.bits.pad_control = int(cfg['pad_config'], 0)

def json_to_bin(jsn):
    """ Convert a JSON object into binary """
    data = []
    pads = jsn
    for pad in pads:
        pad_cfg = pad_configuration()
        pad_cfg.init_from_cfg(pad)
        data.append(pad_cfg.bytes.b1)
        data.append(pad_cfg.bytes.b2)
        data.append(pad_cfg.bytes.b3)
        data.append(pad_cfg.bytes.b4)

    return data

def main():
    """ main """
    json_to_bin()
    return 0

if __name__ == "__main__":
    main()
