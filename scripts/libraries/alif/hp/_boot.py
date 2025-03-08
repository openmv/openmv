import sys
import os
import alif
import vfs

bdev = alif.Flash()
try:
    fat = vfs.VfsFat(bdev)
    vfs.mount(fat, "/flash")
except:
    vfs.VfsFat.mkfs(bdev)
    fat = vfs.VfsFat(bdev)
    vfs.mount(fat, "/flash")

sys.path.append("/flash")
sys.path.append("/flash/lib")
os.chdir("/flash")

del sys, os, alif, bdev
