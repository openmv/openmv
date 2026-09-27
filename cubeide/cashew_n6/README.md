# cashew_n6 — standalone cashew camera firmware (STM32CubeIDE)

Bare-metal firmware for **NUCLEO-N657X0-Q + STEVAL-66GYMAI (ST VD66GY)**. It has the same
kernel detector and the same report lines as the OpenMV build, but no MicroPython and no
OpenMV code: ST HAL, ST's BSD-3 `vd6g` sensor driver, and our own code only.

**Status:** compiles with no warnings (STM32CubeIDE settings and the Makefile) and has been
code-reviewed. It has **not run on hardware yet.**

## Memory: where everything runs

| Memory | Size | Contents |
|---|---|---|
| **ITCM** 0x10000000 | 24 KB of 64 KB | the whole detector (`cashew_core.c`), capture and frame interrupt, detection loop, UART driver, SysTick/DCMIPP/UART interrupts, `libm`, `memset`/`memcpy`, `libgcc` |
| **DTCM** 0x30000000 | 104 KB of 128 KB | the detector's per-row work buffers (93 KB), all other variables, detector tables, and the 16 KB stack (top of DTCM) |
| AXISRAM2 0x34180400 | 88 KB | the boot image: vector table, init code (clocks, HAL, sensor boot), constants, sensor patch tables |
| AXISRAM2 0x341C0000 | 64 KB | the detector's per-candidate buffers (hull, neck test), used only for a few blobs per frame |
| AXISRAM1 0x34000000 | 300 KB | 4 frame buffers of 320 × 240. The camera's DMA cannot write to the TCMs, so each frame is read from here once through the D-cache. |

The linker script `STM32N657X0HXQ_cashew_tcm.ld` assigns code to these memories by object
file. `tcm_init()` in `main.c` copies the ITCM code before `main()` runs.

## Import and build in STM32CubeIDE (1.17 or newer, with STM32N6 support)

1. **File → Import → General → Existing Projects into Workspace** → select the `cashew_n6`
   folder → Finish. Leave "Copy projects into workspace" unticked, or tick it; both work.
2. Choose the **Release** or **Debug** configuration and press **Build** (hammer icon).
   - Both configurations use `-O2`. Debug adds `-g3` and `DEBUG`.
   - The build produces `Release/cashew_n6.elf` and `cashew_n6.bin`, and prints the ITCM/DTCM usage.
3. Command line alternative with the same flags: `make` (optionally `make GCC_PATH=/path/to/arm-gcc/bin`).
   This writes to `build/`.

The project is a secure FSBL project for the STM32N657X0HxQ, like ST's Nucleo templates. The
sensor driver in `Drivers/vd6g` is excluded from the build on purpose: it is compiled through
`Core/Src/vd6g_build.c`, because `vd6g.c` includes its patch tables.

## Run it

### A. From the debugger (development boot, nothing written to flash)

1. Set the jumpers to **BOOT0 = 0, BOOT1 = 1** (JP2 position 2) and power-cycle.
2. In CubeIDE, choose **Run → Debug As → STM32 C/C++ Application** with ST-LINK (ST-LINK GDB server).
   It loads the image into AXISRAM2 and starts it.
   - If it does not stop at `main`, open the debug configuration's **Startup** tab and set
     **"Set program counter"** to the `Reset_Handler` address from `cashew_n6.map`.
3. Open a terminal on the **ST-LINK virtual COM port** at **921600 baud, 8N1**.

### B. Standalone from flash (starts at power-up)

1. Set BOOT1 = 1 (JP2 position 2), keep BOOT0 = 0, and power-cycle.
2. Run `tools/flash_cashew.bat` (Windows) or `tools/flash_cashew.sh`. By default it uses
   `Release/cashew_n6.bin`, else `build/cashew_n6.bin`; you can also pass a path.
   - It signs the image as an FSBL with `STM32_SigningTool_CLI` from STM32CubeProgrammer 2.18 or newer.
   - It then writes it to 0x70000000 with ST's Nucleo external loader.
   - If STM32CubeProgrammer is not in its default folder, set `CUBEPROG` to its `bin` folder first.
3. Set **BOOT1 = 0** (JP2 position 1) and power-cycle.

This firmware **never programs OTP fuses** and leaves the VDDIO2 I/O range at 3.3 V, which is
correct for the Nucleo.

## Serial output

The detection loop starts automatically after boot. It prints:

```
cashew camera: NUCLEO-N657X0-Q + VD66GY, standalone firmware
CPU 600 MHz, build ...
detector: hot buffers 92608 B in DTCM, warm 60296 B in AXISRAM2, max frame 320x240
memory: ITCM code 24312 B used of 64 KB | DTCM data+bss ... + stack 16384 B of 128 KB
VD66GY: model 0x5603 colour, 320x200 bin 2 (sensor rows 400), line 1236 clk (7686 ns), frame 542 lines = 4166 us, min frame 3920 us (255 fps), ...
run: 320x200, report every 240 frames, verbose 1
  K f<frame> t=<us> x=<px> y=<px> L=<mm> W=<mm> A=<mm2> sol=<0..1> rgb=<r>/<g>/<b>
fps <fps> | proc avg <us> max <us> | wait avg <us> | lost <n> err <n> | kernels <n> dup <n> | edge <n> long <n> ...
```

These are the same fields as `cashew.run()` in the OpenMV build:
- One **K** line per new kernel. `t` is the frame-end time in µs.
- One summary line every `report` frames.
- `lost` counts frames the camera delivered while all 4 buffers were busy. It is exact, taken
  from the DCMIPP frame counter.

The first run will give the real timings.

### Commands

Type a command and press Enter.

| Command | What it does |
|---|---|
| `help` | list the commands |
| `run [frames]` / `stop` | start or stop the detection loop (the blue USER button also toggles it) |
| `mean` | frame brightness: mean, min and max, and per colour. Use it to set exposure and gain first. |
| `snap` | process one frame and list every blob with its status (ok / edge / small / long / aspect / concave / big / neck / crowded) |
| `sweep 4166 2400 250 240` | frame-time sweep: the real maximum frame rate for this ROI, binning and vblank (any key stops it) |
| `dump` | send one raw frame; `python tools/grab_frame.py COM5` saves it as `frame_raw.pgm` and `frame_rgb.ppm` |
| `param` | show all settings |
| `status` | camera, capture and memory information |
| `set NAME VALUE` | change a setting (list below) |
| `roi X Y W H` | the detector's region of interest |

`set` names:
- **Detector:** the same names as `cashew.param()` — `threshold invert filter edge_px min_pix max_pix mm_px lmax_mm ar_max ar_min wmax_mm sol_min neck_px gap_mm crowd_min_pix belt_mm_s dir dup_mm dup_y_mm cfa`.
- **Camera:** `fps frame_us expo gain vblank height bin strobe strobe_inv mirror flip colorbar`. `height` and `bin` restart the camera.
- **Output:** `verbose` (0 = summary only, 1 = also kernels, 2 = also rejects) and `report`.

Settings are kept in RAM only. To change the power-up defaults, edit `app_init()` in
`Core/Src/cashew_app.c`, where they match `cashew_bench_n6.py`, and `cam_default_cfg()` in
`camera_vd66gy.c`: 320 × 200, bin 2, 240 fps, 200 µs, 6 dB.

## Pins

| Signal | Pin |
|---|---|
| Kernel output (high for one frame after a frame with a new kernel) | **D2 = PD0** |
| Busy output (high while a frame is being processed; the pulse width is the processing time) | **D3 = PE9** |
| LEDs | green: heartbeat every 120 frames; blue: kernel; red: error |
| Camera | I2C2 PB10/PB11, enable PA0, reset PO5, 2-lane CSI-2 on the 22-pin FFC |
| Log/commands | USART1 PE5/PE6 = ST-LINK virtual COM port |

The kernel output is a marker for a scope or a trigger for your own timing. It is not an
ejector schedule: the ejector delay depends on belt speed and geometry. The K line's `t` and
`x` give you what you need to compute it.

## Differences from the OpenMV build

- No IDE preview. Use `dump` with `tools/grab_frame.py` to see frames.
- The frame size is fixed at 320 wide and at most 240 high (`CAP_MAX_W/H` in `capture.h`).
- No MicroPython scripting; settings go through serial commands.
- 600 MHz CPU clock, same as ST's Nucleo examples.
- No OpenMV code, so OpenMV's non-commercial licence terms do not apply to this firmware.

## What to send back after the first run

1. The boot text up to the `run:` line.
2. The `mean` output, and `sweep 4166 2400 250 240`.
3. A few summary lines with kernels passing on the belt.

If the camera is not found, send the message. Then check the FFC orientation, and that PA0
and PO5 reach the module.

## Files

| Path | What |
|---|---|
| `Core/Src/main.c` | clocks (600 MHz), GPIO, ITCM copy, boot sequence |
| `Core/Src/cashew_app.c` | detection loop, statistics, commands |
| `Core/Src/capture.c` | DCMIPP/CSI-2 raw capture into the 4-buffer ring, frame timestamps |
| `Core/Src/camera_vd66gy.c` | VD66GY: ROI/binning, frame time, exposure, gain, strobe GPIO |
| `Core/Src/uart_log.c` | interrupt-driven UART, small printf, command input |
| `Core/Src/timebase.c` | µs clock from the cycle counter (SysTick fallback) |
| `Core/Src/fault.c` | prints PC and fault registers on a crash |
| `Cashew/cashew_core.*` | the detector (identical to the OpenMV build) |
| `Drivers/` | ST HAL (BSD-3), CMSIS (Apache-2.0), ST vd6g driver (BSD-3), each with its licence |
| `STM32N657X0HXQ_cashew_tcm.ld` | linker script with the ITCM/DTCM placement |
| `tools/` | `flash_cashew.bat/.sh`, `grab_frame.py` |

`Core/Startup/startup_stm32n657xx_fsbl.s` and `Core/Src/system_stm32n6xx_fsbl.c` come from
ST's cmsis-device-n6 (Apache-2.0). One change was made to the startup: interrupts stay off
until `main()`.
