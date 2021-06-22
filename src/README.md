# Firmware Guide

This guide covers how to install the OpenMV Cam firmware development environment on Windows, Mac, and Linux.

**If you are looking for information on how to use the OpenMV Cam from the python level interface please see our [forums](http://openmv.io/forums) or [docs](http://openmv.io/docs). This README details how to setup the development environment to compile your OpenMV Cam's firmware.**

# Windows Installation

There is no Windows development environment. It is very difficult to install the toolchain on Windows. Instead you can install Ubuntu on a virtual machine running on your windows machine:

1. You can get VMware Player (free) [here](https://my.vmware.com/en/web/vmware/free#desktop_end_user_computing/vmware_workstation_player/15_0) to run Ubuntu. Or, you can get VirtualBox (free) [here](https://www.virtualbox.org/wiki/Downloads). VMware Player is recommended.
2. Download Ubuntu [here](http://www.ubuntu.com/desktop). Then use whatever virtual machine software you installed to install the operating system. VMware Player makes this easy with a automated install option where it will install everything for you without you having to do anything other than enter your name and password initially.
3. Install any updates, etc. for your operating system. Also, if you're using VMware Player make sure to install VMware Tools so you can drag and drop files between your Windows desktop and Ubuntu desktop along with being able to setup shared folders.

# Mac Installation

There is no Mac development environment. It is very difficult to install the toolchain on Mac. Instead you can install Ubuntu on a virtual machine running on your Mac machine:

1. You can get VMware Fusion (paid) [here](https://www.vmware.com/products/fusion/) to run Ubuntu. Or, you can get VirtualBox (free) [here](https://www.virtualbox.org/wiki/Downloads). VMware Fusion is recommended.
2. Download Ubuntu [here](http://www.ubuntu.com/desktop). Then use whatever virtual machine software you installed to install the operating system. VMware Fusion makes this easy with a automated install option where it will install everything for you without you having to do anything other than enter your name and password initially.
3. Install any updates, etc. for your operating system. Also, if you're using VMware Fusion make sure to install VMware Tools so you can drag and drop files between your Mac desktop and Ubuntu desktop along with being able to setup shared folders.

# Linux Installation

Open a terminal and run the following commands:

    sudo apt-get remove gcc-arm-none-eabi
    sudo apt-get autoremove
    sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
    sudo apt-get update
    sudo apt-get install gcc-arm-embedded
    sudo apt-get install libc6-i386
    sudo apt-get install git

This should install all the required libraries on your computer.

Next, you need to install OpenMV IDE on your computer to flash firmware and run python scripts. Please install OpenMV IDE on your host system. OpenMV IDE's bootloader will have trouble connecting to your OpenMV Cam from inside of a virtual machine. You can download OpenMV IDE [here](https://openmv.io/pages/download). *To be clear, OpenMV IDE can connect to an OpenMV Cam from inside of a virtual machine, however OpenMV IDE's bootloader needs to connect to the OpenMV Cam using a timing critical handshake that generally fails when you introduce moving USB devices around between the Host OS and a Virtual Machine*.

* If you are installing OpenMV IDE on Linux please make sure to follow the `README.txt` file in the IDE install dir to complete any post-installation steps like setting up `udev` and `dialout` permissions.

Finally, you need to install the OpenMV GitHub Repo. In Linux, `cd` in a terminal to a place where you want to put the repo (we recommend making a folder called `repositories` in your home directory and storing repositories in that) and then:

    git clone --recursive https://github.com/openmv/openmv.git

However, if you are interested in submitting code fixes back to us you will need to fork our repo first and clone your fork so that you can send pull requests. You need to fork these two repos:

    https://github.com/openmv/openmv.git
    https://github.com/openmv/micropython.git

Then:

    git clone --recursive https://github.com/<username>/openmv.git
    cd openmv
    git remote add remote https://github.com/openmv/openmv.git
    cd src/micropython
    git remote set-url origin https://github.com/<username>/micropython.git
    git remote add remote https://github.com/openmv/micropython.git

This will setup the `openmv` and `micropython` repos so `origin` points to your forks and so you can pull updates from the official repos with `remote`. Now when you want to create a new feature branch to send a Pull Request to OpenMV you just need to do:

    git checkout -b <your_name>/<some_branch_name>
    <commit changes>
    git push origin -u <your_name>/<some_branch_name>

Finally, after pushing your changes you can then use Github to automatically generate a Pull Request to the official OpenMV Github repo to get your changes upstreamed into the official OpenMV Cam Firmware.

### Committing Etiquette

If you would like to send a Pull Request to get your changes integrated into the official source tree please try to keep one commit to one Pull Request. Additionally, please create example scripts (in `../scripts/examples`) for any new features you are committing.

# Work Flow

We recommend you use [VS Code](https://code.visualstudio.com/) to edit the firmware. VS Code includes a code explorer, excellent code editor, and a built-in terminal. You should install VS Code on your Linux system.

Once you've installed VS Code please launch it and then go to `File->Open Folder...` and select your cloned `openmv` repository. Next, to compile firmware open a terminal inside VS Code and do:

    cd src/micropython/mpy-cross
    make

This will build the MicroPython Cross Compilier which we use to include frozen bytecode modules into the OpenMV Cam firmware. This only needs to be done once (or if mpy-cross changes). Next:

    cd ../../
    make

To build the latest OpenMV Cam firmware. You make target building the firmware for particular OpenMV Cam models by passing `TARGET=OPENMV...` to make:

    make TARGET=OPENMV2 # To build the OpenMV Cam M4 Firmware
    make TARGET=OPENMV3 # To build the OpenMV Cam M7 Firmware
    make TARGET=OPENMV4 # To build the OpenMV Cam H7 Firmware (default)
    make TARGET=OPENMV4P # To build the OpenMV Cam H7 Plus Firmware
    make TARGET=OPENMVPT # To build the OpenMV Pure Thermal Firmware
    make TARGET=PORTENTA # To build the Arduino H7 Portenta Firmware
    make TARGET=NANO33 # To build the Arduino Nano 33 BLE Firmware

After building the firmware the output will appear under `src/build/bin` where you will find the following files:

* bootloader.bin - Bootloader Binary Image (not directly used)
* bootloader.dfu - Bootloader DFU Image (not directly used)
* bootloader.elf - Bootloader ELF Image (used to generate the BIN/DFU Files)
* firmware.bin - Firmware Binary Image (Used by `Tools->Run Bootloader` in OpenMV IDE)
* firmware.dfu - Firmware DFU Image (not directly used)
* firmware.elf - Firmware ELF Image (used to generate the BIN/DFU Files)
* openmv.bin - Combined Bootloader+Firmware Binary Image (not directly used)
* openmv.dfu - Combined Bootloader+Firmware DFU Image (Used by `Tools->Run Bootloader` in OpenMV IDE)
* uvc.bin - Alternative UVC Binary Image (not directly used)
* uvc.dfu - Alternative UVC DFU Image (not directly used)
* uvc.elf - Alternative UVC ELF Image (used to generate the BIN/DFU Files)

Next, if you have Linux running inside of a Virtual Machine you will want to use shared folders to make the `src/build/bin` folder inside of your Virtual Machine visible to your host OS. With VMware you can do this by going to `Player->Manage->Virtual Machine Settings->Options->Shared Folders` and adding a shared folder named `shared` on the host OS. The shared folder will appear in Linux (using Ubuntu) under `/mnt/hgfs/shared/`. Any files you copy to that path will appear in the host OS inside of the shared folder you created. To copy firmware after you compile into that folder just add `&& mv build/bin/* /mnt/hgfs/shared/` to your `make` command to compile firmware.

After doing this OpenMV IDE will then be able to read the newly compiled binaries under the shared folder path you created.

To update your OpenMV Cam's firmware you want to load the `firmware.bin` file onto your OpenMV Cam using the `Tools->Run Bootloader` command in OpenMV IDE. Please make sure to only load firmware meant for your model of the OpenMV Cam onto it. The IDE does not check this. If you accidentally load the wrong firmware your OpenMV Cam will just crash without being damaged when trying to execute the main firmware image and you can then just use the bootloader onboard again to load the correct `firmware.bin`.

### Docker Build

To build the firmware using docker, follow the following steps:

```
git clone https://github.com/openmv/openmv.git
cd openmv
make TARGET=<TARGET NAME>
```

After building you should see the target build output under `build/<TARGET_NAME>`, for example

```
ls build/OPENMV4 
bootloader.bin  bootloader.dfu  bootloader.elf  firmware.bin  firmware.dfu  firmware.elf  openmv.bin  uvc.bin  uvc.dfu  uvc.elf
```

### About Building

To build the firmware faster we recommend passing `-j4` or higher depending on how many cores your computer has with make. E.g. `make -j4 TARGET=...`. Additionally, if you have issues with files not being recompiled correctly when building do `make clean` before building the firmware. We recommend using `make clean` regularly whenever you pull new changes or edit MicroPython QSTR files.

### About The Binaries

Your OpenMV Cam has a bootloader that comes installed on it from the factory. OpenMV IDE communicates to this bootloader to load the `firmware.bin` file onto your OpenMV Cam. You should never directly need to ever load any `bootloader.*` file onto your OpenMV Cam unless you are trying to modify the bootloader. Note that you cannot use the bootloader to program itself. If however, you manage to break the default bootloader you can use DFU to reset your OpenMV Cam by loading the `openmv.dfu` file onto your OpenMV Cam.

If you want to modify your OpenMV Cam's bootloader you can do so by loading the `bootloader.dfu` file using OpenMV IDE onto your OpenMV Cam using `Tools->Run Bootloader`. When passed a DFU file OpenMV IDE automatically invokes `dfu-util` to program your OpenMV Cam versus using our custom bootloader.

The `uvc.bin` file is an alternative to the normal `firmware.bin` file which you can load onto your OpenMV Cam to turn it into a UVC camera. That said, OpenMV does not currently maintain the UVC firwmare.

#### Note about STM32 DFU Support

The latest STM32 devices (e.g. the OpenMV Cam H7 Plus, Arduino H7 Portenta and later) do not support loading large binary images over DFU currently. However, you are able to still load the `bootloader.dfu` file on these devices which can then be used to load the `firmware.bin` file using OpenMV IDE.
