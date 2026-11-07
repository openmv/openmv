# OpenMV Unix Port

The Unix port runs OpenMV image processing without camera hardware. It uses MicroPython's Unix runtime with the `UNIX` variant and shared OpenMV modules.

The build and CI commands below use a Linux Docker environment.

## Build

The Linux host needs Bash, Git, Make, coreutils and Docker. The Docker image provides the host compiler and Python. The Unix-specific Docker targets do not require the OpenMV SDK.

From a new checkout:

```bash
git clone https://github.com/openmv/openmv.git
cd openmv
make -C docker build-unix
```

The command initialises missing MicroPython, ulab and AprilTag submodules, plus MicroPython's `lib/mbedtls`, `lib/berkeley-db-1.xx` and `lib/micropython-lib` dependencies. The Unix make rules build `mpy-cross` once under `build/UNIX/mpy-cross` and place the interpreter at `build/UNIX/bin/micropython`.

The container runs with the host UID/GID and mounts the repository at its host absolute path, so build output and debug information use host paths. See the [Docker guide](../docker/README.md) for worktree mounts.

`build-unix` is incremental and writes to `build/UNIX`. For a clean build run `make -C docker clean-dev build-unix`. `clean-dev` removes `build` and `lib/micropython/mpy-cross/build`. The C unit-test modules and allocator inspection needed by the full test suite are enabled by default. Set `MICROPY_PY_UNITTEST=0` or `MICROPY_PY_UMALLOC=0` on the build command to disable them. A change to these options or to `DEBUG` rebuilds the port from scratch.

## Run

```bash
build/UNIX/bin/micropython
build/UNIX/bin/micropython myscript.py
```

To run inside the build environment, open a shell and use the same commands:

```bash
make -C docker shell-unix
build/UNIX/bin/micropython
```

## API

The configured OpenMV modules are `image`, `omv`, `gif`, `mjpeg` and `crc`. The build also enables `ulab`, `btree` and `ssl`.

Image processing features are selected in `boards/UNIX/imlib_config.h`. Camera capture (`csi`), the `sensor` module, hardware-specific APIs and the `ml` module are not provided.

Create an image and save it:

```python
import image

img = image.Image(320, 240, image.RGB565)
img.draw_rectangle((10, 10, 50, 50), color=(255, 0, 0))
img.save("example.bmp")
```

Image streams use `image.ImageIO`:

```python
import image

img = image.Image(40, 40, image.RGB565)
img.draw_rectangle((4, 4, 16, 16), color=(255, 0, 0))

stream = image.ImageIO("frames.bin", "w")
stream.write(img)
stream.close()

stream = image.ImageIO("frames.bin", "r")
frame = stream.read(copy_to_fb=False)
stream.close()
assert frame.width() == 40 and frame.height() == 40
```

`copy_to_fb=False` keeps the returned frame separate from the shared framebuffer. Close file streams explicitly.

## Execution Model

The Unix configuration is single-threaded. The `_thread` module is unavailable.

Ctrl-C queues an interrupt that is delivered at the next VM or image-processing poll.

## Memory Limits

The Unix port has separate VM and image-processing memory limits:

| Storage | Limit and use |
| --- | --- |
| MicroPython GC heap | Starts at 2 MiB. Holds Python objects and ordinary `image.Image` pixel storage. |
| UMA pool | Fixed 16 MiB. Holds the dynamically allocated main framebuffer, persistent allocator blocks and temporary image-processing data. Allocator metadata also uses this space. |
| Stream buffer | Fixed 512 KiB, including its header. A preview frame or compressed image must fit within the remaining payload space. |

Allocations fail when their pool cannot satisfy the request, even if the host has free RAM.

Increase the VM heap for larger ordinary images:

```bash
build/UNIX/bin/micropython -X heapsize=32M myscript.py
```

## Tests

Run the full suite after building:

```bash
make -C docker test-unix
```

This runs the Python tests, the six `unittest_*` C modules and allocator leak checks inside Docker. The equivalent command, from the repository root inside the container, is:

```bash
build/UNIX/bin/micropython -m unittest scripts/unittest/unix_tests.py
```

`scripts/unittest/unix_tests.py` runs under the `unittest` and `unittest-discover` packages from micropython-lib, which the Unix build freezes into the interpreter. It runs every file in `scripts/unittest/tests` and every `test_*` function of the C modules in one process. A Python test fails when its `unittest(data_path, temp_path)` function raises or returns `False`, and is skipped when it returns `"skip"`. A test also fails when it leaves non-persistent `umalloc` allocations. The exit status is the number of failed tests. A crash of the interpreter ends the run with the signal status; the last line printed names the test that crashed.

The required C modules are `unittest_collections`, `unittest_fmath`, `unittest_fb`, `unittest_imlib`, `unittest_simd` and `unittest_umalloc`, and the suite needs `umalloc`. A build with `MICROPY_PY_UNITTEST=0` or `MICROPY_PY_UMALLOC=0` cannot run the suite.

Tests that import the unavailable `csi` or `ml` modules are reported as skipped.

To run one test file, execute it in the interpreter and call its `unittest` function:

```bash
build/UNIX/bin/micropython -c 'exec(open("scripts/unittest/tests/crc.py").read()); print(unittest("scripts/unittest/data", "/tmp"))'
```

The firmware workflow builds `UNIX` as an entry of the `build-firmware` matrix and runs the full suite in the Docker firmware environment.

## Debug and Size

Build with debug compiler options:

```bash
make -C docker build-unix DEBUG=1
```

Use a host debugger such as `gdb build/UNIX/bin/micropython`. `make TARGET=UNIX debug` starts the host `gdb` on that binary.

Inside the Docker shell, `make TARGET=UNIX size` runs the host size tool on `build/UNIX/bin/micropython`.

## Source Layout

`boards/UNIX` selects the MicroPython variant and image features. `ports/unix` supplies host compatibility code, framebuffer storage and the OpenMV runtime lifecycle. Shared image and stream APIs remain in `modules` and `lib/imlib`.

The Unix build defines `OMV_OS_UNIX=1` for the Unix-only paths in shared code.
