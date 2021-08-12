## Useful tools, scripts and more.

## arduino-fwuploader

This tool can be used to update the NINA-W102 WiFi module with the latest firmware available.
Note: The board needs to be running the Arduino FirmwareUploader sketch first. Example usage:

```
arduino-fwuploader firmware flash --fqbn arduino:mbed_nano:nanorp2040connect -a /dev/ttyACM0 --retries 2
```

Source: https://github.com/arduino/arduino-fwuploader

## arduino-fwuploader-0.1.10

And older firmware loader that supports uploading a specific firmware version for testing. Example usage:

```
arduino-fwuploader-0.1.10 -firmware <path_to_bin> -model nina -port /dev/ttyACM0
```

Source: https://github.com/arduino/arduino-fwuploader

## dfu-util

This tool can be used to upload firmware to DFU bootloaders. Example usage:

```
dfu-util -w -d <vid>:<pid> -a 0 -s 0x80000000:leave -D <path_to_bin>
```

Source: http://dfu-util.sourceforge.net/

## picotool

This tool can be used to upload RP2040 UF2 firmware. Example usage:

```
picotool load <path_to_uf2>
picotool reboot
```

Or to upload a binary to a specific address:

```
picotool load <path_to_bin> --offset 0x10000000
```

Source: https://github.com/raspberrypi/picotool/

## bossac

This tool can be used to upload nrf firmware. Example usage:

```
bossac -e -w --offset=0x16000 --port=ttyACM0 -i -d -U -R <path_to_bin>
```

Source: https://github.com/shumatech/BOSSA/


## TODO: Add documentation for the rest of the tools and scripts.
