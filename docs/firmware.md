# Building the Firmware From Source

This guide covers installing a development environment and building the OpenMV firmware from source.
For information on OpenMV Camera general usage, please see the [forums](http://openmv.io/forums) or [docs](http://openmv.io/docs).
- [Docker Build](#docker-build)
- [Linux / macOS (arm64) Build](#linux--macos-arm64-build)
  * [Install build dependencies](#install-build-dependencies)
  * [Clone the repository](#clone-the-repository)
    + [Shallow clone](#shallow-clone)
  * [Install the SDK](#install-the-sdk)
  * [Build the firmware](#build-the-firmware)
    + [Build artifacts](#build-artifacts)
    + [Notes on building the firmware in a Virtual Machine](#notes-on-building-the-firmware-in-a-virtual-machine)
- [Windows Build](#windows-build)
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
> Note: `TARGET_NAME` is one of the [supported boards](https://github.com/openmv/openmv/tree/master/boards). 

## Linux / macOS (arm64) Build

Building natively is supported on Linux (x86_64) and macOS (arm64). All required tools, including the ARM toolchain, are installed automatically by `make sdk`.

### Install build dependencies
On Linux, install the following packages or their equivalents based on your distro:
```bash
sudo apt-get update
sudo apt-get install git build-essential
```

On macOS, install the required packages using [Homebrew](https://brew.sh/):
```bash
brew install bash make coreutils
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
git -C lib/micropython/ submodule update --init --depth=1
```

### Install the SDK
Before building, install the OpenMV SDK. This downloads and installs all required tools (ARM toolchain, etc.):
```bash
cd openmv
make sdk
```

### Build the firmware
To build the firmware, run the following commands inside the openmv repository:
```bash
make -j$(nproc) -C lib/micropython/mpy-cross   # Builds MicroPython mpy cross-compiler
make -j$(nproc) TARGET=<TARGET_NAME>            # Builds the OpenMV firmware
```

> Note: `TARGET_NAME` is one of the [supported boards](https://github.com/openmv/openmv/tree/master/boards).

### Build artifacts
When the firmware build is done, the build artifacts should be located in `build/bin`. Note the build artifacts depend on the target board, for example whether a bootloader is generated or not depends on the target board's configuration. However, generally speaking the following files are generated:
```
* bootloader.bin  # Bootloader Binary Image (not directly used)
* bootloader.dfu  # Bootloader DFU Image (not directly used)
* bootloader.elf  # Bootloader ELF Image (used to generate the BIN/DFU Files)
* firmware.bin    # Firmware Binary Image (Used by `Tools->Run Bootloader` in OpenMV IDE)
* firmware.dfu    # Firmware DFU Image (not directly used)
* firmware.elf    # Firmware ELF Image (used to generate the BIN/DFU Files)
* openmv.bin      # Combined Bootloader+Firmware Binary Image (not directly used)
* openmv.dfu      # Combined Bootloader+Firmware DFU Image (Used by `Tools->Run Bootloader` in OpenMV IDE)
* romfs<n>.img    # ROM filesystem image
```

### Notes on building the firmware in a Virtual Machine
* When building the firmware inside of a Linux Virtual Machine, shared folders should be used to make the `build/bin` folder inside the VM available to the host OS. If using VMware, this can be done by going to `Player->Manage -> Virtual Machine Settings -> Options -> Shared Folders` and adding a shared folder named `shared` on the host OS. The shared folder will appear in Linux (for Ubuntu) under `/mnt/hgfs/shared/`. Any files copied to that path appear in the host OS inside of the shared folder. To copy firmware after the build into that folder run `mv build/bin/* /mnt/hgfs/shared/`.

* OpenMV IDE's bootloader will have trouble connecting to your OpenMV Cam from inside of a virtual machine. You can download OpenMV IDE [here](https://openmv.io/pages/download). *To be clear, OpenMV IDE can connect to an OpenMV Cam from inside of a virtual machine, however OpenMV IDE's bootloader needs to connect to the OpenMV Cam using a timing critical handshake that generally fails when you introduce moving USB devices around between the Host OS and a Virtual Machine*.

## Windows Build

There is no native Windows development environment. Instead, use the [Docker Build](#docker-build) or install Ubuntu on a virtual machine:

1. Install [VMware Player](https://my.vmware.com/en/web/vmware/free#desktop_end_user_computing/vmware_workstation_player/15_0) (free) or [VirtualBox](https://www.virtualbox.org/wiki/Downloads) (free).
2. Download [Ubuntu](http://www.ubuntu.com/desktop) and install it in the VM.
3. Follow the [Linux / macOS (arm64) Build](#linux--macos-arm64-build) instructions inside the VM.

## Flashing the firmware
To update the OpenMV camera with the newly built firmware, install the [OpenMV IDE](https://openmv.io/pages/download) and upload the `firmware.bin` file to the OpenMV camera using the builtin bootloader from `Tools->Run Bootloader` in OpenMV IDE. Please make sure to only load firmware meant for your model of the OpenMV camera onto it. The IDE does not check this if the firmware matches the camera model. If the wrong firmware is uploaded accidentally the camera will like just crash without being damaged. The OpenMV camera can be recovered by using the bootloader again to load the correct `firmware.bin`.

Note: If you are installing OpenMV IDE on Linux please make sure to follow the `README.txt` file in the IDE install dir to complete any post-installation steps like setting up `udev` and `dialout` permissions.

## The OpenMV bootloader

OpenMV cameras come preloaded with a bootloader from the factory. The OpenMV IDE communicates with the bootloader to load new firmware images over USB. There shouldn't be a need to upload a `bootloader.*` firmware onto an OpenMV camera unless modifying the bootloader itself. Note that the bootloader can't overwrite itself, if the bootloader is overwritten somehow, DFU can be used to recover the OpenMV camera by load `bootloader.dfu` image. The `bootloader.dfu` DFU image can be loaded using OpenMV IDE using `Tools->Run Bootloader`. When passed a DFU file OpenMV IDE automatically invokes `dfu-util` to program your OpenMV Cam versus using our custom bootloader.

### Note about STM32 DFU bootloader

The latest STM32 devices (e.g. the OpenMV Cam H7 Plus) do not support loading large binary images over DFU currently. However, you are able to still load the `bootloader.dfu` file on these devices which can then be used to load the `firmware.bin` file using OpenMV IDE.
