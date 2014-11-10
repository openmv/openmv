To update the firmware using the IDE follow these steps:

* Plug in the OpenMV Cam to a USB port.
* Use the IDE built-in dfu tool to update the firmware.

To update the firmware manually follow these steps:

* Connect BOOT0 to VCC on the debugging header.
* Plug in the OpenMV Cam to a USB port.
* Type make flash, wait until it's done.
* Remove the jumper and reset the board.
