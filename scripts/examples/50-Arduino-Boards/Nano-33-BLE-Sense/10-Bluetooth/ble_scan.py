# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
import time
from ubluepy import Scanner, constants


def bytes_to_str(bytes):
    string = ""
    for b in bytes:
        string += chr(b)
    return string


def get_device_names(scan_entries):
    dev_names = []
    print(len(scan_entries))
    for e in scan_entries:
        scan = e.getScanData()
        for s in scan:
            print(s)
            if s[0] == constants.ad_types.AD_TYPE_COMPLETE_LOCAL_NAME:
                dev_names.append((e, bytes_to_str(s[2])))
    return dev_names


def find_device_by_name(name):
    s = Scanner()
    scan_res = s.scan(1000)
    device_names = get_device_names(scan_res)
    for dev in device_names:
        if name == dev[1]:
            return dev[0]


while True:
    res = find_device_by_name("micr")
    if res:
        print("address:", res.addr())
        print("address type:", res.addr_type())
        print("rssi:", res.rssi())
    time.sleep_ms(500)
