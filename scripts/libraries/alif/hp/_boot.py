import sys
import os
import alif
import vfs

bdev = alif.Flash(id="fs")
try:
    fat = vfs.VfsFat(bdev)
    vfs.mount(fat, "/flash")
except:
    vfs.VfsFat.mkfs(bdev)
    fat = vfs.VfsFat(bdev)
    vfs.mount(fat, "/flash")

try:
    bdev = vfs.VfsRom(alif.Flash(id="romfs"))
    vfs.mount(bdev, "/rom")
    sys.path.append("/rom")
except:
    pass

sys.path.append("/flash")
sys.path.append("/flash/lib")
os.chdir("/flash")

del sys, os, alif, bdev
