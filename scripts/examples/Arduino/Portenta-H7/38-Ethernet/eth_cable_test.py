# Ethernet Cable Status Example.
#
# This example prints the cable connection status.

import network, time

lan = network.LAN()

# Make sure Eth is not in low-power mode.
lan.config(low_power=False)

# Delay for auto negotiation
time.sleep(3.0)

while (True):
    print("Cable is", "connected." if lan.status() else "disconnected.")
    time.sleep(1.0)

# Put Eth back in low-power mode if needed.
#lan.config(low_power=True)
