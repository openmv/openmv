# Board Configuration Customization

You can customize the firmware for your OpenMV Cam by editing the board configuration files
under `boards`.

## Image Library Configuration

All boards have an `imlib_config.h` which enables or disables different machine vision algorithms
onboard your OpenMV Cam. If you need to save firmware space quickly then the best way to do so is
to disable different unused algorithms onboard via commenting out things in the `imlib_config.h`.

## Built-in Python Modules

The `manifest.py` file specifies which python modules are built-into the firmware and available as
frozen modules which can be imported.

## Low-level Board Configuration

`omv_boardconfig.h`, `omv_boardconifg.mk`, `omv_bootconfig.h`, and `omv_pins.h` control low-level
board controls like RAM/FLASH memory layouts, pin controls, and other resources allocations. Do
not edit these files unless you know what you are doing.

## ROMFS Configuration

You can edit the default files built-into the ROMFS partition by the build system by editing the
`romfs.json` file. The ROMFS parition is a memory mapped file system in which large data structures
like models can be executed in-place from.

The json layout in the `romfs.json` file specifies the different ROMFS paritions to be created
by the build system and the size of these partitions. In general, you should only edit the entry
array of files built-into the ROMFS image.

Each entry in `entries` accepts a `type`, `path`, and optional `alignment` field. `type` should be:

`<None>`: The file is included without any transformation. 

`tflite`: Use this format for compiling `tflite` models into the ROMFS. Note that the `alignment`
for `tfilte` models should be `16` on cameras without NPUs, `16` for for cameras with the ARM
Ethos NPU, and `32` for STEdgeAI powered boards. For ARM Ethos NPU boards, the entry must contain
the `optimize` argument for the NPU to control optimization, which may either be `Size`
or `Performance`. For STEdgeAI Powered boards, the entry must contain an argment to select the
`profile` to use when compiling models, which may be `default`.

`haar`: Use this format for compile `xml` haar cascades into the ROMFS. Accepts a required `stages`
argument which specifies the maximum number of stages of the cascade to run. Use `0` to run all
stages. Cascades do not require any memory alignment.

## ULAB Library Configuration

You can change how [ULAB](https://micropython-ulab.readthedocs.io/en/latest/ulab-intro.html) is
setup for the firmware by edting the `ulab_config.h` file.
