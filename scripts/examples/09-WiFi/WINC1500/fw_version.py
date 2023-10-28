# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Atmel WINC1500 firmware version.

import network

wlan = network.WINC()
print("\nFirmware version:", wlan.fw_version())
