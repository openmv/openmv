#!/usr/bin/env python2
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This script stress-tests script execution.

import sys, os
import pyopenmv
import argparse
from time import sleep
from random import randint

def main():
    # CMD args parser
    parser = argparse.ArgumentParser(description='openmv stress test')
    parser.add_argument("-j", "--disable_fb", action = "store_true",  help = "Disable FB JPEG compression")
    parser.add_argument("-p", "--port",   action = "store", help = "OpenMV serial port")
    parser.add_argument("-t", "--time",   action = "store", default = 100, help = "Max time before stopping the script")
    parser.add_argument("-s", "--script", action = "store",\
            default="../scripts/examples/00-HelloWorld/helloworld.py", help = "OpenMV script file")

    # Parse CMD args
    args = parser.parse_args()

    # init openmv
    if (args.port):
        portname = args.port
    elif 'darwin' in sys.platform:
        portname = "/dev/cu.usbmodem14221"
    else:
        portname = "/dev/openmvcam"
    
    print("\n>>>Reading script: %s\n" %(args.script))
    with open(args.script, "r") as f:
        script = f.read()
    print("%s\n" %(script))

    connected = False
    for i in range(10):
        try:
            # Open serial port.
            # Set small timeout when connecting
            pyopenmv.init(portname, baudrate=921600, timeout=0.050)
            connected = True
            break
        except Exception as e:
            connected = False
            sleep(0.100)
    
    if not connected:
        print ( "Failed to connect to OpenMV's serial port.\n"
                "Please install OpenMV's udev rules first:\n"
                "sudo cp openmv/udev/50-openmv.rules /etc/udev/rules.d/\n"
                "sudo udevadm control --reload-rules\n\n")
        sys.exit(1)
    
    # Set higher timeout after connecting.
    pyopenmv.set_timeout(0.500)

    # Enable/Disable framebuffer compression.
    print(">>>Enable FB JPEG compression %s" %(str(not args.disable_fb)))
    pyopenmv.enable_fb(not args.disable_fb)

    # Interrupt running script.
    pyopenmv.stop_script()
    max_timeout = int(args.time)
    for i in range(1000):
        pyopenmv.exec_script(script)
        sleep(randint(0, max_timeout)/1000)
        pyopenmv.stop_script()

if __name__ == '__main__':
    main()
