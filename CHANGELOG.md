# Change Log
## [2.6](https://github.com/openmv/openmv/releases/tag/v2.6) (2017-11-04)
* Update to MicroPython 1.9.2
* Support saving bayer (raw) images.
* Add perspective rotation correction code.
* Fix blob density.
* Fix color VGA image save.
* Remove invalid resolutions. 

## [2.5](https://github.com/openmv/openmv/releases/tag/v2.5) (2017-08-10)
* Fix UART timeout when using slow baudrate.
* Enable RTC.
* Remove openmv.inf and update Readme.
* Support recording and viewing raw videos.
* Add linear regression.
* Add find_rectangles and find_circles.
* Improve find_lines merging.
* Fix bug in ORB matching descriptor loaded from file.
* Support new OpenCV Haar format.
* Fix bug in Haar cascades loading.
* Add initial LeNet port.
* Add unit-tests.
* Fix uninitialized FB enabled bug.
* Fix Servo(3).
* Fix MJPEG/GIF BAYER support.

## [2.4.1](https://github.com/openmv/openmv/releases/tag/v2.4.1) (2017-06-04)
* Upstream Kanji fix.
* Upstream MP SCSI fix.
* Fix binary ops names.

## [2.4](https://github.com/openmv/openmv/releases/tag/v2.4) (2017-05-30)
* Implement faster line detection algorithm.
* Support line segments detection.
* Support higher FPS on OpenMV 2 and 3.
* Add data matrix support.
* Add more small resolutions.
* Enable UART1 on OpenMV3/M7
* Enable VSYNC output on IO pin.
* Fix QR Code bug.
* Fix UDP recvfrom bug.
* Minor fixes, typos and docs updates.

## [2.3](https://github.com/openmv/openmv/releases/tag/v2.3) (2017-03-26)
* Support WiFi Access Point mode.
* New BAYER/RAW pixel format.
* Support RGB VGA frames.
* 1D barcode support using (zbar).

## [2.2](https://github.com/openmv/openmv/releases/tag/v2.2) (2017-02-28)
* Add Apriltags support
* Fix OMV3 bootloader LED pins
* Enable CAN
* Enable extra MP modules (json, re, zlib, hashlib, binascii, random)
* Add Pixy emulation.
* QR Code bug fixes

## [2.1](https://github.com/openmv/openmv/releases/tag/v2.1) (2017-01-21)
* New keypoints descriptor (ORB).
* QR decoding via quirc library (https://github.com/dlbeer/quirc)
* Support load to FB directly in copy_to_fb.
* Export lens shading function.
* Add AGAST corner detector.
* Implement set_gain/exposure/whitebalance functions.
* Various optimizations and speedups
* Fix uSD cache issues on M7.
* Fix broken ADC on M4 and M7.
* Fix image compress and compressed.
* Fix ff_wrapper bug.
* Fix OMV3 LEDs pinout

## [2.0](https://github.com/openmv/openmv/releases/tag/v2.0) (2016-11-04)
Firmware:
* WiFi driver fixes.

Image processing:
* Add HoG (not used yet).
* Add lens correction function.
* Add clear image for quick testing.
* Fix template ROI.
* Switch to FAST-12.
* Misc fixes to image library.

## [1.9](https://github.com/openmv/openmv/releases/tag/v1.9) (2016-09-20)
Firmware:
* Initialize RNG when calling randint.

Image processing:
* Fix and update Kmeans code.
* Add ellipse masking function.
* Add face recognition code and example script.
* Add Hough Transform code and example script.
* Add Canny edge code and example script.
* Add Gaussian function for quick testing.

## [1.8](https://github.com/openmv/openmv/releases/tag/v1.8) (2016-08-31)
Firmware:
* Mainly WiFi driver fixes, more stable streaming, timeouts and better error handling.
* Fixed FPS slow down in dark images (max FPS reduction is 30FPS)

## [1.7](https://github.com/openmv/openmv/releases/tag/v1.7) (2016-08-25)
Firmware:
* Update CMSIS, DSP lib and HAL.
* Adaptive JPEG quality based on JPEG frame size.
* Improved self-tests on OV7725.
* New CPU frequency scaling Python module.
* Allow setting MLX refresh rate and ADC resolution.
* Use a dedicated JPEG buffer (improves IDE FPS).

## [1.6](https://github.com/openmv/openmv/releases/tag/v1.6) (2016-07-27)
IDE:
* Add checkbox to disable the framebuffer update

Firmware:
* Set FB JPEG quality/subsampling based on frame size.

Image processing:
* Implement windowing.
* Implement horizontal and vertical binning.
* Implement optical flow with phase correlation.
* Implement copy image to framebuffer for testing.
* Allow ROIs and step in template matching function.
* Implemented diamond search for fast template matching.
* Fix bug in integral_image_sq and lookup.
* Add new smaller resolutions
* Improved/fixed JPEG code

## [1.5](https://github.com/openmv/openmv/releases/tag/v1.5) (2016-06-01)
IDE:
* Fix pinout reference image.
* Fix reset on bootloader (reset cam just before the bootloader runs).
* Add an option to erase flash filesystem sectors in bootloader dialog.
* Show color statistics in a message dialog.

Firmware:
* Update to MicroPython v1.8
* Change MLX ADC resolution to 18 bits.
* Fixed GC collect bug (.bss and .data were not scanned, fixed in MP update).
* Generate a combined (bootloader + app) dfu and binary images
* Rename firmware images:
 - bootloader.xxx (CDC bootloader images)
 - firmware.xx (main application firmware images)
 - openmv.xx (combined bootloader+firmware images)

Image processing:
* Allow image line-by-line pre-processing from Python callbacks.

## [1.4](https://github.com/openmv/openmv/releases/tag/v1.4) (2016-05-02)
IDE:
* Fix text editor undo bug.
* New bootloader dialog.
* Fix and update example scripts.
* Fix preferences dialog.
* Remove refresh button.

Firmware:
* Fixed file wrapper initialization bug.
* New CDC-based bootloader (works on Linux, Windows and OSX)
* Implement new sensor functions (disable AGC and AEC)
* Fix WINC bug overriding sent data.

Image processing:
* Color codes support.
* New color blob detector.

## [1.3](https://github.com/openmv/openmv/releases/tag/v1.3) (2016-04-07)
IDE:
* Implement the IDE copy color function.
* Update examples menu using categories.
* Fix conflict with PyInstaller scripts.

Firmware:
* Add initial WiFi (WINC1500) support.
* Update WINC1500 driver and firmware to 19.4.4
* Support WINC1500 firmware update from uSD fw image.
* Improved MLX (FIR) temperature scaling and drawing.
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
