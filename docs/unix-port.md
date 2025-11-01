# OpenMV Unix Port

The OpenMV Unix port brings OpenMV's powerful image processing library to desktop and server environments. This port is designed for development, testing, prototyping, and integration into desktop applications.

## Overview

The Unix port provides access to OpenMV's complete image processing library (imlib) without requiring physical hardware. It runs on Linux, macOS, and Windows (via WSL), making it ideal for:

- Algorithm development and testing
- Desktop image processing applications
- Education and learning
- Prototyping before deploying to hardware
- Batch processing of images

## Building

### Prerequisites

- GCC or Clang compiler
- Make
- Git
- Python 3

### Build Steps

```bash
# Clone the repository (without --recursive to avoid unnecessary submodules)
git clone https://github.com/openmv/openmv.git
cd openmv

# Build the Unix port (automatically initializes required submodules and builds mpy-cross)
make unix
```

The build process creates a `micropython` binary at `lib/micropython/ports/unix/build-openmv/micropython`.

## Running

```bash
cd lib/micropython/ports/unix
./build-openmv/micropython
```

You can also run scripts directly:

```bash
./build-openmv/micropython myscript.py
```

Or use the REPL interactively:

```bash
./build-openmv/micropython
>>> import image
>>> img = image.Image(320, 240, image.RGB565)
```

## Features

### Available Modules

The Unix port includes:

- **image** - Full image processing library
- **gif** - GIF recording and playback
- **mjpeg** - MJPEG recording and playback
- **imageio** - Image file I/O operations

### Supported Image Operations

All OpenMV image processing operations are supported:

- Image creation, loading, and saving (BMP, PNG, JPEG, GIF)
- Drawing operations (lines, rectangles, circles, text)
- Color space conversions (RGB, grayscale, binary, etc.)
- Image filters (Gaussian, median, bilateral, etc.)
- Morphological operations (erode, dilate, open, close)
- Feature detection:
  - Blobs
  - Lines and line segments
  - Circles and rectangles
  - QR codes and barcodes
  - AprilTags
  - Data matrices
  - Keypoints (ORB, FAST, etc.)
- Template matching
- Image statistics and histograms
- Color tracking

## Limitations

The Unix port is software-only and does not include hardware-specific functionality:

### Unavailable Modules

- **sensor** - Camera/sensor control (no physical sensor)
- **display** - Display control (no physical display)
- **imu** - IMU/accelerometer (no physical IMU)
- **fir** - FIR/thermal camera (no physical hardware)
- **audio** - Audio processing (no audio hardware)
- **tv** - TV output (no TV hardware)
- **ml** - Machine learning (TensorFlow Lite for Microcontrollers)

### Memory Allocation

The Unix port uses a hybrid memory allocation strategy:

- **Framebuffer**: Static 1.5MB buffers (1MB main + 512KB streaming) using official framebuffer.c
- **Temporary buffers** (fb_alloc): GC-based allocation for image processing operations
- **Python heap**: MicroPython's garbage collector manages Python objects

### Performance

Performance characteristics may differ from embedded targets:

- CPU-only processing (no hardware acceleration)
- Different optimization profiles
- Memory access patterns differ from embedded systems

## Use Cases

### Algorithm Development

Develop and test image processing algorithms on your desktop before deploying to hardware:

```python
import image

# Load a test image
img = image.Image("test.jpg")

# Apply processing
img.gaussian(1)
blobs = img.find_blobs([(30, 100, 15, 127, 15, 127)])

# Save result
img.save("result.jpg")
```

### Algorithm Validation

Test and validate image processing algorithms with known inputs:

```bash
#!/bin/bash
# test_vision_algorithm.sh

./build-openmv/micropython test_algorithm.py
if [ $? -eq 0 ]; then
    echo "Algorithm validation passed"
else
    echo "Algorithm validation failed"
    exit 1
fi
```

### Desktop Applications

Build desktop tools that leverage OpenMV's image processing:

- Image batch processing tools
- Computer vision prototyping
- Educational applications
- Image analysis utilities

### Education

Learn computer vision concepts without requiring hardware:

```python
# Students can experiment with image processing
import image

img = image.Image(320, 240, image.RGB565)
img.draw_string(10, 10, "Hello OpenMV!", color=(255, 255, 255))
img.draw_rectangle(50, 50, 100, 100, color=(255, 0, 0))
img.save("my_first_image.bmp")
```

## Architecture

### Port Structure

The Unix port is organized as follows:

```
openmv/
├── ports/unix/              # Unix port implementation
│   ├── omv_portconfig.mk    # Build configuration
│   ├── omv_mpconfigport.h   # MicroPython config
│   ├── fb_alloc.c           # GC-based temporary buffer allocator
│   ├── py_unix_stubs.c      # Framebuffer memory and hardware stubs
│   └── unix_compat/         # Compatibility wrappers
│       ├── omv_common.h     # Unix-specific overrides
│       ├── fmath.h          # Fast math wrappers
│       ├── arm_math.h       # ARM math stubs
│       └── ...
├── boards/UNIX/             # Unix board configuration
│   ├── omv_boardconfig.mk   # Board settings
│   ├── imlib_config.h       # Image library config
│   └── mpconfigvariant.h    # MicroPython variant
└── modules/                 # OpenMV Python modules
    ├── py_image.c           # Image module (shared)
    ├── py_imageio.c         # Image I/O (shared)
    └── ...
```

### Integration with MicroPython

The Unix port builds as a MicroPython Unix port variant:

- Uses MicroPython's Unix port infrastructure
- Registers as the "openmv" variant
- Adds OpenMV modules via USER_C_MODULES
- Provides Unix-specific compatibility layer

### Memory Management

The Unix port uses a two-tier memory architecture:

- **Framebuffer memory**: Static 1.5MB buffers (1MB main + 512KB streaming) using official framebuffer.c from imlib
- **fb_alloc system**: GC-based allocation for temporary image processing buffers
- **Python objects**: MicroPython's standard heap with automatic garbage collection

## Troubleshooting

### Build Errors

**Error: "cmsis missing"**
```bash
# Update submodules
git submodule update --init --recursive
```

**Error: "mpy-cross not found"**
```bash
# Build mpy-cross first
make -C lib/micropython/mpy-cross
```

### Runtime Issues

**Module import fails**
- Ensure you're running from the correct directory
- Check that the build completed successfully
- Verify module names (lowercase: `import image`, not `import Image`)

**Out of memory**
```python
# Force garbage collection
import gc
gc.collect()
```

## Contributing

Contributions to the Unix port are welcome! Please follow [OpenMV's contribution guidelines](../README.md#contributing-to-the-project).

## License

The OpenMV Unix port is licensed under the MIT license, consistent with the rest of the OpenMV project. See the LICENSE file for details.

Note: Some image library components use GPL licenses (AGAST, LSD, ZBAR). These can be disabled by defining `OMV_NO_GPL` in `imlib_config.h`.
