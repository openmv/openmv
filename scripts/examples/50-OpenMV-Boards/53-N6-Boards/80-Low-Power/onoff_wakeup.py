# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# ONOFF Wakeup Example
#
# The ONOFF pin is an internal wakeup-capable Pin on your OpenMV Cam.
# It is connected to a hardware wakeup line so toggling it brings
# the board out of deep sleep without any software involvement.
#
# This example enters deep sleep with no RTC alarm. The board will
# stay asleep until the ONOFF pin is asserted (e.g. via the external
# power button), at which point it resets and re-runs the script.

import machine

# Enter deep sleep. Toggle ONOFF to wake the board back up.
machine.deepsleep()
