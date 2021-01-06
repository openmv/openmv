# Python (and MicroPython) APDS-9960 Library

Python library for the APDS-9960 gesture sensor developed while I was looking to get the APDS-9960 to work with a _Raspberry Pi_ to build a user interface feeling like in _Minority Report_.

This library is a port of the [APDS-9960 Raspberry Pi Library](https://bitbucket.org/justin_woodman/apds-9960-raspberry-pi-library) of [Justin Woodman](https://justinwoodman.wordpress.com/2014/11/15/using-the-apds-9960-rgb-proximity-and-gesture-sensor-with-the-raspberry-pi-2/). Sadly his library is coded in C++ and seems not to be maintained any more.

This library has been tested with [SparkFun RGB and Gesture Sensor - APDS-9960](https://www.sparkfun.com/products/12787) but should work with any other APDS-9960 based I²C device, too.

## Installation

This library is available from [PyPi](https://pypi.org/project/apds9960/). You may want to setup a `virtualenv` before installing the library using:

```
pip install apds9960
```


## Features of the APDS-9960
- operational voltage: 3.3V
- ambient light & RGB color sensing
- proximity sensing
- gesture detection
- operating range: 10 - 20cm
- I²C interface (hard wired I²C address: 0x39)

## Documentation & Examples
- [RPi](RPi.md) - connect and configure the APDS-9960 on Raspberry Pi
- Example scripts:
  - simple ambient light level demo: [rpi](rpi/test_ambient.py), [micropython](micropython/test_ambient.py)
  - simple gesture detection demo: [rpi](rpi/test_gesture.py), [micropython](micropython/test_gesture.py)
  - simple proximity level demo: [rpi](rpi/test_prox.py), [micropython](micropython/test_prox.py)
