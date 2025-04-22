# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Atmel WINC1500 Firmware dump.

import network

wlan = network.WINC(mode=network.WINC.MODE_FIRMWARE)

# For ATWINC1500-MR210PB only.
wlan.fw_dump("winc_19_7_6.bin")
