# Installing the Python OpenMV-IDE

## Install libusb

### For Linux 
```$ sudo apt-get install libusb-1.0-0 python-vte```

### For Everything else
Download and install: http://www.libusb.org/

## Install PyUSB

```$ sudo pip install --pre pyusb```

Use the --pre flag if you are getting the following error. It tells pip to accept pre-release (alpha, beta) versions.

```  Could not find a version that satisfies the requirement pyusb (from versions: 1.0.0a2, 1.0.0a2, 1.0.0a3, 1.0.0a3, 1.0.0b1)```

## Install pySerial

```$ sudo pip install pySerial```

