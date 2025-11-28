[![Firmware Build 🔥](https://github.com/openmv/openmv/actions/workflows/firmware.yml/badge.svg)](https://github.com/openmv/openmv/actions/workflows/firmware.yml)
[![GitHub license](https://img.shields.io/github/license/openmv/openmv?label=license%20%E2%9A%96)](https://github.com/openmv/openmv/blob/master/LICENSE)
![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/openmv/openmv?sort=semver)
[![GitHub forks](https://img.shields.io/github/forks/openmv/openmv?color=green)](https://github.com/openmv/openmv/network)
[![GitHub stars](https://img.shields.io/github/stars/openmv/openmv?color=yellow)](https://github.com/openmv/openmv/stargazers)
[![GitHub issues](https://img.shields.io/github/issues/openmv/openmv?color=orange)](https://github.com/openmv/openmv/issues)

<img  width="480" src="https://raw.githubusercontent.com/openmv/openmv-media/master/logos/openmv-logo/logo.png">

# The Open-Source Machine Vision Project
  - [Overview](#overview)
  - [TensorFlow support](#tensorflow-support)
  - [Interface library](#interface-library)
    + [Note on serial port](#note-on-serial-port)
  - [ Building the firmware from source](#building-the-firmware-from-source)
  - [Contributing to the project](#contributing-to-the-project)
    + [Contribution guidelines](#contribution-guidelines)
    
## Overview

The OpenMV project brings machine vision to beginners with a user-friendly, open-source platform. OpenMV cameras are programmable in Python3 and feature advanced AI capabilities, including support for TensorFlow, ST Edge AI, and NPU acceleration through ARM Ethos-U55 and ST Neural-ART. The firmware includes a rich image image processing library, such as image filtering, feature detection, color tracking, QR and barcode decoding, AprilTag recognition, GIF and MJPEG recording and streaming, and much more.

The latest generation of OpenMV cameras includes the N6 and AE3 models. The OpenMV-N6 is powered by the STM32N6 microcontroller, which features a 1GHz/600 GOPS NPU, hardware H.264 encoding, high-speed USB, ISP, GPU, and a high-speed μSD card slot. It also comes equipped with WiFi/BLE, Gigabit Ethernet, and extensive GPIO pins—combining the power of a single-board computer with the flexibility of a microcontroller.

The OpenMV-AE3 is our smallest and most energy-efficient camera yet powered by the Alif Ensemble E3, with dual-core processors and dual Ethos-U55 NPUs. This ultra-low-power device can run YOLO models at 30 FPS while drawing only 0.25 W, and it consumes just 2.5 mW in deep sleep mode. It features a 1MP high-frame-rate color global shutter camera, an IMU, a microphone, an 8×8 distance sensor, and connectivity options such as high-speed USB, WiFi/BLE, and QWIIC—making it ideal for battery-powered AI vision applications.

<p align="center"><img width="320" src="https://github.com/openmv/openmv-media/blob/2cef6b1eb4dff065e99f3cda0892aadabfd841cb/boards/openmv-cam/openmv-ae3/ae3-hero-crop.jpg"></p>

Complementing the firmware is an intuitive, cross-platform IDE based on Qt Creator, specifically designed for machine vision development. The IDE lets users view the camera’s frame buffer in real time, adjust sensor settings, and run scripts directly on the device. It also provides a range of tools for image analysis, including tag generation, threshold definition, keypoint detection, and other processing functions.

The OpenMV project was successfully funded on Kickstarter in 2015 and has evolved significantly since its inception. For more information, visit [https://openmv.io](https://openmv.io).

## Interface library

The OpenMV Cam comes built-in with an RPC (Remote Python/Procedure Call) library which makes it easy to connect the OpenMV Cam to another microcontroller like the Arduino or ESP8266/32. The RPC Interface Library works over:

* Async Serial (UART) - at up **7.5 Mb/s** on the OpenMV Cam H7.
* I2C Bus - at up to **1 Mb/s** on the OpenMV Cam H7.
  * Using 1K pull up resistors.
* SPI Bus - at up to **20 Mb/s** on the OpenMV Cam H7.
  * Up to **80 Mb/s** or **40 Mb/s** is achievable with short enough wires.
* CAN Bus - at up to **1 Mb/s** on the OpenMV Cam H7.

With the RPC Library you can easily get image processing results, stream RAW or JPG image data, or have the OpenMV Cam control another Microcontroller for lower-level hardware control like driving motors.

You can find examples that run on the OpenMV Cam under `File->Examples->Remote Control` in OpenMV IDE and online [here](scripts/examples/34-Remote-Control). Finally, OpenMV provides the following libraries for interfacing your OpenMV Cam to other systems below:

* [Arduino Interface Library for CAN, I2C, SPI, UART Comms](https://github.com/openmv/openmv-arduino-rpc)
  * Works on all Arduino variants.
  * CAN support via the MCP2515 over SPI or via the CAN peripheral on the ESP32.

#### Note on serial port

If you only need to read `print()` output from a script running on an OpenMV camera over USB, then you only need to open the OpenMV camera Virtual COM Port and read lines of text from the serial port. For example (using [pyserial](https://pythonhosted.org/pyserial/index.html)):

```Python
import serial
ser = serial.Serial("COM3", timeout=1, dsrdtr=False)
while True:
    line = ser.readline().strip()
    if line: print(line)
```
The above code works for Windows, Mac, or Linux. You just need to change the above port name to the same name of the USB VCP port the OpenMV Cam shows up as (it will be under `/dev/` on Mac or Linux). Note that if you are opening the USB VCP port using another serial library and/or language make sure to set the DTR line to false - otherwise the OpenMV Cam will suppress printed output.

## Building the firmware from source

The easiest way to patch the firmware and rebuild it, is to fork this repository, enable Actions (from the Actions tab) in the forked repository, and pushing the changes. Our GitHub workflow rebuilds the firmware on pushes to the master branch and/or merging pull requests and generates a development release with attached separate firmware packages per supported board. For more complex changes, and building the OpenMV firmware from source locally, see [Building the Firmware From Source](https://github.com/openmv/openmv/blob/master/docs/firmware.md).

For more information about customizing your OpenMV Cam's configuration see [Board Configuration](docs/boards.md).

### Unix Port for Development

OpenMV also provides a Unix port that allows running the image processing library on desktop systems (Linux, Mac, Windows with WSL) without hardware. This is useful for algorithm development, testing, and prototyping. See [Unix Port Documentation](docs/unix-port.md) for details.

```bash
make unix  # Build Unix port
```

## Contributing to the project

Contributions are most welcome. If you are interested in contributing to the project, start by creating a fork of each of the following repositories:

* https://github.com/openmv/openmv.git
* https://github.com/openmv/micropython.git

Clone the forked openmv repository, and add a remote to the main openmv repository:
```bash
git clone --recursive https://github.com/<username>/openmv.git
git -C openmv remote add upstream https://github.com/openmv/openmv.git
```

Set the `origin` remote of the micropython submodule to the forked micropython repo:
```bash
git -C openmv/src/micropython remote set-url origin https://github.com/<username>/micropython.git
```

Finally add a remote to openmv's micropython fork:
```bash
git -C openmv/src/micropython remote add upstream https://github.com/openmv/micropython.git
```

Now the repositories are ready for pull requests. To send a pull request, create a new feature branch and push it to origin, and use Github to create the pull request from the forked repository to the upstream openmv/micropython repository. For example:
```bash
git checkout -b <some_branch_name>
<commit changes>
git push origin -u <some_branch_name>
```

### Contribution guidelines
Please follow the [best practices](https://developers.google.com/blockly/guides/modify/contribute/write_a_good_pr) when sending pull requests upstream. In general, the pull request should:
* Fix one problem. Don't try to tackle multiple issues at once.
* Split the changes into logical groups using git commits.
* Pull request title should be less than 78 characters, and match this pattern:
  * `<scope>:<1 space><description><.>`
* Commit subject line should be less than 78 characters, and match this pattern:
  * `<scope>:<1 space><description><.>`

Example PR titles or commit subject lines:
```
github: Update workflows.
Libtf: Add support for built-in models.
RPC library: Remove CAN bit timing function.
OPENMV4: Add readme template file.
ports/stm32/main.c: Fix storage label.
```

### Licensing

Most of the code in the repository is licensed under the MIT license, with the following exceptions:

* Some image library code is licensed under the GPL. This includes AGAST, LSD, and ZBAR. GPL code can be completely disabled in a build by defining `OMV_NO_GPL` in the `imlib_config.h` files.
* Third-party libraries and drivers in `lib` and `drivers` are licensed under various permissive licenses. Please consult the LICENSE file in each driver/library subdirectory for more details.
* Some drivers, modules, and libraries in OpenMV are proprietary and available for non-commercial use only. These proprietary components can be disabled during the build process. Official OpenMV hardware and licensed devices may use the proprietary code. For commercial licensing options, contact openmv@openmv.io.
