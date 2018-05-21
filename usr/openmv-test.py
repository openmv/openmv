#!/usr/bin/env python2
import sys, os
import openmv
import argparse
from time import sleep
from random import randint

def main():
    # CMD args parser
    parser = argparse.ArgumentParser(description='openmv stress test')
    parser.add_argument("-j", "--disable_fb", action = "store_true",  help = "Disable FB JPEG compression")
    parser.add_argument("-p", "--port",   action = "store", help = "OpenMV serial port")
    parser.add_argument("-t", "--time",   action = "store", default = 100, help = "Max time before stopping the script")
    parser.add_argument("-s", "--script", action = "store", default="examples/01-Basics/helloworld.py", help = "OpenMV script file")

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
            openmv.init(portname, baudrate=921600, timeout=0.050)
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
    openmv.set_timeout(0.500)

    # Enable/Disable framebuffer compression.
    print(">>>Enable FB JPEG compression %s" %(str(not args.disable_fb)))
    openmv.enable_fb(not args.disable_fb)

    # Interrupt running script.
    openmv.stop_script()
    max_timeout = int(args.time)
    for i in xrange(1000):
        openmv.exec_script(script)
        sleep(randint(0, max_timeout)/1000)
        openmv.stop_script()

if __name__ == '__main__':
    main()
