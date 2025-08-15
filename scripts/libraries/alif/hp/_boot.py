import sys
import os
import alif
import vfs

main_py = """import time
from machine import LED

led = LED("LED_BLUE")

while True:
    led.on()
    time.sleep_ms(150)
    led.off()
    time.sleep_ms(100)
    led.on()
    time.sleep_ms(150)
    led.off()
    time.sleep_ms(600)
"""

readme_txt = """Thank you for supporting the OpenMV project!

To download the IDE, please visit:
https://openmv.io/pages/download

For tutorials and documentation, please visit:
http://docs.openmv.io/

For technical support and projects, please visit the forums:
http://forums.openmv.io/

Please use github to report bugs and issues:
https://github.com/openmv/openmv
"""


def create_file(path, data=None):
    with open(path, "w") as f:
        if data is not None:
            f.write(data)


bdev = alif.Flash()
try:
    fat = vfs.VfsFat(bdev)
    vfs.mount(fat, "/flash")
except:
    vfs.VfsFat.mkfs(bdev)
    fat = vfs.VfsFat(bdev)
    vfs.mount(fat, "/flash")
    create_file("/flash/main.py", main_py)
    create_file("/flash/README.txt", readme_txt)

os.chdir("/flash")
sys.path.append("/flash")
sys.path.append("/flash/lib")

try:
    os.stat(".openmv_disk")
except Exception:
    create_file(".openmv_disk")

del sys, os, alif, bdev
