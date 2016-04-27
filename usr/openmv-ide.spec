# -*- mode: python -*-
import platform

block_cipher = None
sysname = platform.system()

rt_hooks = None
if sysname == "Linux" or sysname == "Darwin":
    rt_hooks=['gdk_rthook.py']

a = Analysis(['openmv-ide.py'],
             hiddenimports=['numpy'],
             hookspath=None,
             runtime_hooks=rt_hooks,
             excludes=None,
             cipher=block_cipher)
pyz = PYZ(a.pure,
             cipher=block_cipher)

# append 'exe' to windows binary
if sysname in ["Linux", "Darwin"]:
    exe_name ='openmv-ide'
else:
    exe_name ='openmv-ide.exe'

exe_tree = [('logo.png', 'logo.png', 'DATA'),
            ('pinout.png', 'pinout.png', 'DATA'),
            ('openmv-ide.glade', 'openmv-ide.glade', 'DATA'),
            ('loaders.cache', 'loaders.cache', 'DATA')]

gdk_loaders = []
if sysname == "Linux" or sysname == "Darwin":
    pixbuf_dir = '/usr/lib/x86_64-linux-gnu/gdk-pixbuf-2.0/2.10.0/loaders'
    for pixbuf_type in os.listdir(pixbuf_dir):
        if pixbuf_type.endswith('.so'):
            gdk_loaders.append((pixbuf_type, os.path.join(pixbuf_dir, pixbuf_type), 'BINARY'))

exe = EXE(pyz,
          a.scripts,
          a.binaries + gdk_loaders,
          a.zipfiles,
          a.datas,
          exe_tree,
          name=exe_name,
          debug=False,
          strip=None,
          upx=True,
          console=False )

import sys, shutil

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
data_tree += Tree('../firmware', prefix='firmware')

# bundle gtksourceview style/lang files
if sysname == "Linux":
    data_tree += Tree('/usr/share/gtksourceview-2.0/', prefix='share/gtksourceview-2.0')
elif sysname == "Darwin":
    data_tree += Tree('/usr/local/share/gtksourceview-2.0/', prefix='share/gtksourceview-2.0')
elif sysname == "Windows":
    data_tree += Tree('C:/Python27/Lib/site-packages/gtk-2.0/runtime/share/gtksourceview-2.0', prefix='share/gtksourceview-2.0')

if sysname in ["Linux", "Windows"]:
    col = COLLECT(exe,
                  data_tree,
                  upx=True,
                  strip=None,
                  name=sys.argv[2])
else:
# create an app bundle for OSX
    app = BUNDLE(exe,
                 data_tree,
                 icon=None,
                 name=sys.argv[2],
                 bundle_identifier=None)

# cleanup
if os.path.exists(util_dir):
    shutil.rmtree(util_dir)
