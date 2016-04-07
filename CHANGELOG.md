# Change Log
## [1.3](https://github.com/openmv/openmv/releases/tag/v1.3) (2016-04-07)
IDE:
* Implement the IDE copy color function.
* Update examples menu using cateogries.
* Fix conflict with PyInstaller scripts.

Firmware:
* Add initial WiFi (WINC1500) support.
* Update WINC1500 driver and firmware to 19.4.4
* Support WINC1500 firmware update from uSD fw image.
* Improved MLX (FIR) temprature scaling and drawing.
* Add WiFi examples (mjpeg streamer, NTP, scan, connect and firmware update)

Image processing:
* Implement AWB/HMirror/VFlip.
* Implement mean, median and mode filters.

## [1.2](https://github.com/openmv/openmv/releases/tag/v1.2) (2016-03-19)
IDE:
* About dialog, license and credits.
* Pin-out image for quick reference.
* Check for updates on startup.
* Support older firmware versions.
* Retry a few times when connecting.
* Enable/Disable framebuffer JPEG compression.

Firmware:
* Support the newer OV7725 sensor.
* Add snapshot timeout to avoid locking the cam.
* Fix PWM/Servos timer, channels and pin-mappings. 
* Add OpenMV boards configuration files in omv/boards.
* Support the new MLX90621 sensor and add proper rainbow scaling.
* Better script handling, and soft reset support.
* JPEG-compress the framebuffer to lower the bandwidth and fake double-buffering.
* YUV to Grayscale conversion on the fly.
* Add sanity checks and more meaningful error messages.
* Allocate FatFS LFN buffer on stack (frees 255 heap bytes).
* Move the FatFS and MSC buffers to main RAM (saves heap and allows DMA access).
* Use DMA for SDIO transfers.
* Remove framebuffer mutex (IDE reads images before snapshots).
* Define pin aliases (P0..P8)/
* Move LCD to built-in module.

Image processing:
* Improved iris detection.
* Edge detection, generic convolution, motion detection and GIF support.
* JPEG compressor optimizations (70ms @QVGA 320x240) faster BinDCT and 2x2 subsampling.
* Proper JPEG headers for Grayscale images.
* Bug fixes in old integral image code and a new integral image using a moving window.
* Set the number of pixels counted in each blob in imlib_count_blobs.
* Simplify the image descriptor APIs, use a generic image.load/save/match_descriptor functions.
* Add HQVGA resolution, and special digital effects support.
* Support higher Grayscale resolutions (up to QVGA) for most algorithms.
* Image processing functions accept paths to images on uSD.

## [1.1](https://github.com/openmv/openmv/releases/tag/v1.1) (2015-08-15)
* Rollback to gtksourceview
* Use MP peripherals
* Add ABI version and check it in the IDE
* Add common cascades to the flash
* Fix changing pixformat bug
* Fix sensor reset
* gc/xalloc race bug
* Fix sensor clock
* Update to MP 1.4.4
* Add udev rules help and check for udev file
* Update USB PID:VID
* Update inf file
* Generate Linux/Windows packages
* Catch and print syntax errors
* Add colorbar mode function
* Optimize the IDE (revert to numpy, use timeout_add etc..)
* Remove obselete #define from mpconfigboard.h
* Write colorbar test
* Fix silkscreen
* Rename Eagle files
* Move misc image functions to image module
* Delay sensor init after USB storage to log errors to file
* Implement get/set pixel
* Fix push/pop scope (re-init mp before running scripts)
* Update examples
* Fix main script FS template in main.c
* Remove global misc functions
* Remove lib folder
* Fix draw_string
* Disable built-in DFU on Windows

## [1.0.3-beta](https://github.com/openmv/openmv/releases/tag/v1.0.3-beta) (2014-11-15)
* Binary packaged using py2exe
* Mixed 32/64 bit Windows installer
* Fix USB issue on Windows 7 64-bit
* Enable color-lookup (was disabled in binaries)

## [1.0.2-beta](https://github.com/openmv/openmv/releases/tag/v1.0.1-beta) (2014-11-11)
* Fixes USB issues on Windows.
* New MSI package for Windows users
* Moved all user data are stored in home directory.

## [1.0.1-beta](https://github.com/openmv/openmv/releases/tag/v1.0.1-beta) (2014-11-2)
* Minor fixes for compatibility with Windows.

## [1.0.0-beta](https://github.com/openmv/openmv/releases/tag/v1.0.0-beta) (2014-10-31)
* First release.
