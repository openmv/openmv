# Overclocking Example
#
# This example shows how to overclock your OMV2 cam to 216MHz. The camera will
# stay overclocked until the next hard reset, if you need to keep this frequency
# call the set_frequency function from your main script.
#
# WARNING: Overclocking to 216MHz should be safe, however Use at your own risk!

import cpufreq

# Print current CPU frequency
print(cpufreq.get_frequency())

# Set frequency valid values are (120, 144, 168, 192, 216)
cpufreq.set_frequency(cpufreq.CPUFREQ_216MHZ)

# Print current CPU frequency
print(cpufreq.get_frequency())
