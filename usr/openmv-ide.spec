# -*- mode: python -*-

block_cipher = None

a = Analysis(['openmv-ide.py'],
             hiddenimports=['usb', 'numpy'],
             hookspath=None,
             runtime_hooks=None,
             excludes=None,
             cipher=block_cipher)
pyz = PYZ(a.pure,
             cipher=block_cipher)

# append 'exe' to windows binary
if platform.system() == "Linux":
    exe_name ='openmv-ide'
else:
    exe_name ='openmv-ide.exe'

exe_tree = [('openmv-ide.glade', 'openmv-ide.glade', 'DATA')]

exe = EXE(pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          exe_tree,
          name=exe_name,
          debug=False,
          strip=None,
          upx=True,
          console=True )

import shutil

# remove util dir if it exists
util_dir = 'util'
if os.path.exists(util_dir):
    shutil.rmtree(util_dir)

# create util dir and copy scripts
os.mkdir(util_dir)
shutil.copy('pydfu.py', util_dir)
shutil.copy('openmv-cascade.py', util_dir)

data_tree  = Tree('util', prefix='util')
data_tree += Tree('examples', prefix='examples')
data_tree += Tree('../udev', prefix='udev')
data_tree += Tree('../firmware', prefix='firmware',
excludes=['cmsis', 'fatfs', 'micropython', 'omv', 'sthal', '*.lds'])

coll = COLLECT(exe,
               data_tree,
               strip=None,
               upx=True,
               name='openmv')
# cleanup
if os.path.exists(util_dir):
    shutil.rmtree(util_dir)

