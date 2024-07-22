# Building the Firmware From Source

This guide covers installing a development environment and building the OpenMV firmware from source on Linux.
For information on OpenMV Camera general usage, please see the [forums](http://openmv.io/forums) or [docs](http://openmv.io/docs).
- [Docker Build](#docker-build)
- [Linux Build](#linux-build)
  * [Install build dependencies](#install-build-dependencies)
  * [Install GNU ARM toolchain](#install-gnu-arm-toolchain)
  * [Clone the repository](#clone-the-repository)
    + [Shallow clone](#shallow-clone)
  * [Build the firmware](#build-the-firmware)
    + [Build artifacts](#build-artifacts)
    + [Notes on building the firmware in a Virtual Machine](#notes-on-building-the-firmware-in-a-virtual-machine)
- [Windows Build](#windows-build)
- [Mac Build](#mac-build)
- [Flashing the Firmware](#flashing-the-firmware)
- [The OpenMV bootloader](#the-openmv-bootloader)
    + [Note about STM32 DFU bootloader](#note-about-stm32-dfu-bootloader)

## Docker Build

The easiest way to build the firmware from source is to use the docker image. To build the firmware using docker, follow these steps:

```bash
git clone https://github.com/openmv/openmv.git --depth=50
cd openmv/docker
make TARGET=<TARGET_NAME>
```

When the firmware build is done, the build artifacts should be located in `docker/build/<TARGET_NAME>`.
> Note: `TARGET_NAME` is one of the [supported boards](https://github.com/openmv/openmv/tree/master/src/omv/boards). 

## Linux Build

### Install build dependencies
Install the following packages or their equivalents based on your distro:
```bash
sudo apt-get update
sudo apt-get install git build-essential
```

### Install GNU ARM toolchain
This step can be skipped if your distro package manager provides an ARM toolchain, however this is the gcc toolchain currently in use by the developers. Note the following commands install the toolchain to `/usr/local/arm-none-eabi` and then add it to the PATH variable, for the current terminal session. The toolchain will need to be added to the PATH again if a new terminal session is started. Note the toolchain can be installed in any other location as long as it's added to the PATH.
```
TOOLCHAIN_PATH=${HOME}/cache/gcc
TOOLCHAIN_URL="https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz"
mkdir ${TOOLCHAIN_PATH}
wget --no-check-certificate -O - ${TOOLCHAIN_URL} | tar --strip-components=1 -Jx -C ${TOOLCHAIN_PATH}
export PATH=${TOOLCHAIN_PATH}/bin:${PATH}
```

### Clone the repository
To clone the OpenMV GitHub repository, run the following command:
```bash
git clone --recursive https://github.com/openmv/openmv.git
```

#### Shallow clone
The above command will clone this repository along with all of its submodules recursively, which may take a very long time to finish. Alternatively, you can make a shallow clone of the repository with the following commands:
```bash
git clone --depth=1 https://github.com/openmv/openmv.git
cd openmv
git submodule update --init --depth=1 --no-single-branch
git -C src/micropython/ submodule update --init --depth=1
```

### Build the firmware
To build the firmware, run the following commands inside the openmv repository:
```bash
cd openmv
make -j$(nproc) -C src/micropython/mpy-cross   # Builds Micropython mpy cross-compiler
make -j$(nproc) TARGET=<TRAGET_NAME> -C src    # Builds the OpenMV firmware
```

> Note: `TARGET_NAME` is one of the [supported boards](https://github.com/openmv/openmv/tree/master/src/omv/boards).

### Build artifacts
When the firmware build is done, the build artifacts should be located in `src/build/bin`. Note the build artifacts depend on the target board, for example whether a bootloader or UVC binaries are generated or not depends on the target board's configuration. However, generally speaking the following files are generated:
```
* bootloader.bin  # Bootloader Binary Image (not directly used)
* bootloader.dfu  # Bootloader DFU Image (not directly used)
* bootloader.elf  # Bootloader ELF Image (used to generate the BIN/DFU Files)
* firmware.bin    # Firmware Binary Image (Used by `Tools->Run Bootloader` in OpenMV IDE)
* firmware.dfu    # Firmware DFU Image (not directly used)
* firmware.elf    # Firmware ELF Image (used to generate the BIN/DFU Files)
* openmv.bin      # Combined Bootloader+Firmware Binary Image (not directly used)
* openmv.dfu      # Combined Bootloader+Firmware DFU Image (Used by `Tools->Run Bootloader` in OpenMV IDE)
* uvc.bin         # Alternative UVC Binary Image (not directly used)
* uvc.dfu         # Alternative UVC DFU Image (not directly used)
* uvc.elf         # Alternative UVC ELF Image (used to generate the BIN/DFU Files)
```

### Notes on building the firmware in a Virtual Machine
* When building the firmware inside of a Linux Virtual Machine, shared folders should be used to make the `src/build/bin` folder inside the VM available to the host OS. If using VMware, this can be done by going to `Player->Manage -> Virtual Machine Settings -> Options -> Shared Folders` and adding a shared folder named `shared` on the host OS. The shared folder will appear in Linux (for Ubuntu) under `/mnt/hgfs/shared/`. Any files copied to that path appear in the host OS inside of the shared folder. To copy firmware after the build into that folder run `mv build/bin/* /mnt/hgfs/shared/`.

* OpenMV IDE's bootloader will have trouble connecting to your OpenMV Cam from inside of a virtual machine. You can download OpenMV IDE [here](https://openmv.io/pages/download). *To be clear, OpenMV IDE can connect to an OpenMV Cam from inside of a virtual machine, however OpenMV IDE's bootloader needs to connect to the OpenMV Cam using a timing critical handshake that generally fails when you introduce moving USB devices around between the Host OS and a Virtual Machine*.

## Windows Build

There is no Windows development environment. It is very difficult to install the toolchain on Windows. Instead you can install Ubuntu on a virtual machine running on your windows machine:

1. You can get VMware Player (free) [here](https://my.vmware.com/en/web/vmware/free#desktop_end_user_computing/vmware_workstation_player/15_0) to run Ubuntu. Or, you can get VirtualBox (free) [here](https://www.virtualbox.org/wiki/Downloads). VMware Player is recommended.
2. Download Ubuntu [here](http://www.ubuntu.com/desktop). Then use whatever virtual machine software you installed to install the operating system. VMware Player makes this easy with a automated install option where it will install everything for you without you having to do anything other than enter your name and password initially.
3. Install any updates, etc. for your operating system. Also, if you're using VMware Player make sure to install VMware Tools so you can drag and drop files between your Windows desktop and Ubuntu desktop along with being able to setup shared folders.

## Mac Build

There is no Mac development environment. It is very difficult to install the toolchain on Mac. Instead you can install Ubuntu on a virtual machine running on your Mac machine:

1. You can get VMware Fusion (paid) [here](https://www.vmware.com/products/fusion/) to run Ubuntu. Or, you can get VirtualBox (free) [here](https://www.virtualbox.org/wiki/Downloads). VMware Fusion is recommended.
2. Download Ubuntu [here](http://www.ubuntu.com/desktop). Then use whatever virtual machine software you installed to install the operating system. VMware Fusion makes this easy with a automated install option where it will install everything for you without you having to do anything other than enter your name and password initially.
3. Install any updates, etc. for your operating system. Also, if you're using VMware Fusion make sure to install VMware Tools so you can drag and drop files between your Mac desktop and Ubuntu desktop along with being able to setup shared folders.

## Flashing the firmware
To update the OpenMV camera with the newly built firmware, install the [OpenMV IDE](https://openmv.io/pages/download) and upload the `firmware.bin` file to the OpenMV camera using the builtin bootloader from `Tools->Run Bootloader` in OpenMV IDE. Please make sure to only load firmware meant for your model of the OpenMV camera onto it. The IDE does not check this if the firmware matches the camera model. If the wrong firmware is uploaded accidentally the camera will like just crash without being damaged. The OpenMV camera can be recovered by using the bootloader again to load the correct `firmware.bin`.

Note: If you are installing OpenMV IDE on Linux please make sure to follow the `README.txt` file in the IDE install dir to complete any post-installation steps like setting up `udev` and `dialout` permissions.

## The OpenMV bootloader

OpenMV cameras come preloaded with a bootloader from the factory. The OpenMV IDE communicates with the bootloader to load new firmware images over USB. There shouldn't be a need to upload a `bootloader.*` firmware onto an OpenMV camera unless modifying the bootloader itself. Note that the bootloader can't overwrite itself, if the bootloader is overwritten somehow, DFU can be used to recover the OpenMV camera by load `bootloader.dfu` image. The `bootloader.dfu` DFU image can be loaded using OpenMV IDE using `Tools->Run Bootloader`. When passed a DFU file OpenMV IDE automatically invokes `dfu-util` to program your OpenMV Cam versus using our custom bootloader.

### Note about STM32 DFU bootloader

The latest STM32 devices (e.g. the OpenMV Cam H7 Plus) do not support loading large binary images over DFU currently. However, you are able to still load the `bootloader.dfu` file on these devices which can then be used to load the `firmware.bin` file using OpenMV IDE.
