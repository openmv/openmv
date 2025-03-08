#!/usr/bin/env python3
import os
import sys
import shutil
import subprocess
import utils.config
from utils.config import *

CONF_FILE = 'bin/ARMDS_dbg.cfg'
Manufacturer = ""
Platform = ""
Connection = ""

def getJlinkSN():
    """
	JLINK get the serial number
    """
    cmd = "pylink emulator -l usb"
# DEBUG     
#    print("CMD=",cmd)
#    which_pylink = shutil.which("pylink")
#    print("SHUTIL says ", which_pylink)

    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,universal_newlines=True,shell=True)
    output, errors = p.communicate()

    if errors > '':
        print('Errors')
        print(errors)

    idx = output.find('Serial Number:')

    if idx == -1:
        return -1
        
    sn = output[idx+15:].splitlines()
    return sn[0]

def discover_Manufacturer(extensionDB):
    """
	ARM ULINK discovery
    """
    global Manufacturer
    
    print("\nDiscovering Manufacturer...")
    
    cmd = 'armdbg --cdb-root-ignore-default --cdb-root "' + extensionDB + '" --cdb-entry "?"'
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True,shell=True)
    output, errors = p.communicate()
    options = errors.splitlines()
    
    selection = 0
    while selection == 0:
        print('\nAvailable options:')
        i = 0
        for line in options:
            if i == 0:
                i += 1
                continue
            print(str(i) + ". " + line)
            i += 1

        selection = int(input("\nPlease select an option: "))
        if int(selection) > len(options)-1:
            print('Please select a valid option!')
            selection = 0

    Manufacturer = options[selection]
    print('Manufacturer: ' + Manufacturer)


def discover_Platform(extensionDB):
    global Manufacturer
    global Platform
    
    print("\nDiscovering Platform for " + Manufacturer + "...")
    
    cmd = 'armdbg --cdb-root-ignore-default --cdb-root "' + extensionDB + '" --cdb-entry "' + Manufacturer + '::?"'
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True,shell=True)
    output, errors = p.communicate()
    options = errors.splitlines()
 
    selection = 0
    while selection == 0:
        print('\nAvailable options:')
        i = 0
        for line in options:
            if i == 0:
                i += 1
                continue
            print(str(i) + ". " + line)
            i += 1

        selection = int(input("\nPlease select an option: "))
        if int(selection) > len(options)-1:
            print('Please select a valid option!')
            selection = 0
    
    Platform = options[selection]
    print('Platform: ' + Platform)


def discover_Connection(extensionDB, jtag_adapter):
    global Manufacturer
    global Platform
    global Connection

    print("\nDiscovering Connection for " + Manufacturer + "::" + Platform + "...")
    
    cmd = 'armdbg --cdb-root "' + extensionDB + '" --cdb-entry "' + Manufacturer + '::' + Platform + '::Bare Metal Debug::Bare Metal Debug::Cortex-M0+::' + jtag_adapter + '" --cdb-entry-param "Connection=?" --browse '
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True,shell=True)
    output, errors = p.communicate()
    options = output.splitlines()
        
    selection = 0
    while selection == 0:
        print('\nAvailable options:')
        i = 0
        for line in options:
            if i == 0:
                i += 1
                continue
            print(str(i) + ". " + line.strip())
            i += 1

        selection = int(input("\nPlease select an option: "))
        if int(selection) > len(options)-1:
            print('Please select a valid option!')
            selection = 0
    
    Connection = options[selection].strip()
    print('Connection: ' + Connection)

def discoverParameters():
    print("Please be sure the board is powered-on and ULINK Pro is connected!")
    input("Press any key to continue...")
    extensionDB = '%USERPROFILE%\\Development Studio Workspace\\ExtensionDB'
    if sys.platform == "linux":
        extensionDB = '$HOME/developmentstudio-workspace/ExtensionDB'

    # get JTAG Adatper from Global Configuration
    load_global_config()
    JTAG_ADAPTER = utils.config.JTAG_ADAPTER

    print('Writing MRAM with parameters:')
    print('Device Part# ' + DEVICE_PART_NUMBER + ' - Rev: ' + DEVICE_REVISION)   
    print('- MRAM Base Address: ' + hex(ALIF_BASE_ADDRESS))


    discover_Manufacturer(extensionDB)
    discover_Platform(extensionDB)
    discover_Connection(extensionDB, JTAG_ADAPTER)

def saveConfiguration():
    try:
        f = open(CONF_FILE, "w")
        f.write(Manufacturer + ',' + Platform + ',' + Connection)
        f.close()

    except:
        print("There was an error creating the configuration file!")
        return    

def getValues():

    if not os.path.exists(CONF_FILE):
        print("Configuration file doesn't exist. We will run the discovery process...")
        discoverParameters()
        saveConfiguration()

    try:
        f = open(CONF_FILE, 'r')
        Manufacturer, Platform, Connection = f.readline().split(',')

    except FileNotFoundError:
        print("The configuration file could not be created!")
        return
    except:
        print('There was an error reading the configuration file!')
        return

    return Manufacturer, Platform, Connection

def main():
    if sys.version_info.major == 2:
        print("You need Python 3 for this application!")
        return 0
    
    Manuf, Plat, Conn = getValues()
    print('Configuration values:')
    print(Manuf)
    print(Plat)
    print(Conn)
 
if __name__ == "__main__":
    main()
