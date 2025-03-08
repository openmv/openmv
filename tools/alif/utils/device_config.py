#!/usr/bin/env python3

import sys
import os
import struct
from utils.config import read_global_config
import utils.firewall
from utils.firewall import firewall_json_to_bin
import utils.pinmux
from printInfo import *

EXIT_WITH_ERROR = 1

DEVICE_CONFIG_PIN_MUX           = 0x01
DEVICE_CONFIG_PIN_PROTECTION    = 0x02
DEVICE_CONFIG_EVENT_ROUTING     = 0x03
DEVICE_CONFIG_INTERRUPT_MAP     = 0x04
DEVICE_CONFIG_FIREWALL          = 0x05
DEVICE_CONFIG_WOUNDING          = 0x06
DEVICE_CONFIG_CLOCKS            = 0x07
DEVICE_CONFIG_REGISTER_SETTINGS = 0x08
DEVICE_CONFIG_MISCELLANEOUS     = 0x09
DEVICE_CONFIG_METADATA          = 0x0A
DEVICE_CONFIG_END_OF_LIST       = 0xFE

HEADER_SIZE_MASK                = 0x00FFFFFF

VALID_SECTIONS = [
    "metadata",
    "firewall",
    "clocks",
    "pinmux",
    "wounding", 
    "register_settings", 
    "miscellaneous"
]

misc_settings_headers = {
  'LFXO_CAP_CTRL'           : '0x00000001', # <16 bits setting type> <16 bits setting size in words>
  'LFXO_GM_CTRL'            : '0x00010001',
  'HFXO_CAP_CTRL'           : '0x00020001',
  'HFXO_PFET_GM_CTRL'       : '0x00030001',
  'HFXO_NFET_GM_CTRL'       : '0x00040001',
  'SE_BOOT_INFO'            : '0x00050001'
#  'ISP_MAINTENANCE_SUPPORT' : '0x00060001',
#  'FW_RUNTIME_CFG'          : '0x00070001',
#  'PINMUX_RUNTIME_CFG'      : '0x00080001',
#  'CLOCK_RUNTIME_CFG'       : '0x00090001'
}

metadata_settings_headers = {
  'HFXO_PRESENT'       : '0x00000001', # <16 bits setting type> <16 bits setting size in words>
  'HFXO_FREQUENCY_CFG' : '0x00010001',
  'LFXO_PRESENT'       : '0x00020001',
  'LFXO_FREQUENCY'     : '0x00030001'
}

# HFXO frequency config data format:
#    3  bits  0 (unsused)
#    1  bit   'divider enable' flag
#    1  bit   0 (unused)
#    7  bits  PLL multiplier integer (N) part
#    20 bits  PLL multiplier fractional (f) part
#
hfxo_frequency_cfg = {
  38400000 : '0x17D00000',
  32000000 : '0x04B00000',
  25000000 : '0x06000000',
  24000000 : '0x06400000',
}

hfxo_frequency_cfg_spark = {
  38400000 : '0x11900000',
}

lfxo_frequency = {
  32768 : 32768,
  32000 : 32000
}


def validateSections(sections, file):
    for sec in sections:
        if sec not in VALID_SECTIONS:
            # Don't terminate the script, just print than an unsupported section was found
            print('[WARN] Unsupported section "' + sec + '" found in file ' + file)  
            #sys.exit(EXIT_WITH_ERROR)

def createBinary(file):

    try:
        f = open(file, 'wb')
    except:
        print(sys.exc_info()[0])
        print('[ERROR] creating Device Configuration binary')
        sys.exit(EXIT_WITH_ERROR)

    return f

def closeBinary(f):
    f.close()

def writeBinary(f, data):
    f.write(data)

def processMetadata(configuration):

    data = bytearray()
    for sec in configuration:
        if sec != "external_clock_sources":
            continue

        for setting in configuration[sec]:
            if "id" not in setting:
                continue
                
            if setting["id"] == "OSC_HFXO":
                if "enabled" in setting:
                    enabled = 0x1 if setting["enabled"] else 0x0

                    header = metadata_settings_headers.get('HFXO_PRESENT')
                    data += bytes(int(header, 16).to_bytes(4, 'little'))
                    data += bytes(enabled.to_bytes(4, 'little'))

                if "frequency" in setting:
                    if utils.config.DEVICE_FEATURE_SET == 'Spark':
                        frequency_cfg = hfxo_frequency_cfg_spark.get(setting['frequency'])
                    else:
                        frequency_cfg = hfxo_frequency_cfg.get(setting['frequency'])
                    if frequency_cfg is None:
                        print("[ERROR] Unsupported HFXO frequency:", setting['frequency'])
                        sys.exit(EXIT_WITH_ERROR)

                    header = metadata_settings_headers.get('HFXO_FREQUENCY_CFG')
                    data += bytes(int(header, 16).to_bytes(4, 'little'))
                    printInfo("HFXO frequency config:", frequency_cfg)
                    data += bytes(int(frequency_cfg, 16).to_bytes(4, 'little'))

            if setting["id"] == "OSC_LFXO":
                if "enabled" in setting:
                    enabled = 0x1 if setting["enabled"] else 0x0

                    header = metadata_settings_headers.get('LFXO_PRESENT')
                    data += bytes(int(header, 16).to_bytes(4, 'little'))
                    data += bytes(enabled.to_bytes(4, 'little'))

                if "frequency" in setting:
                    frequency = lfxo_frequency.get(setting['frequency'])
                    if frequency is None:
                        print("[ERROR] Unsupported LFXO frequency:", setting['frequency'])
                        sys.exit(EXIT_WITH_ERROR)

                    header = metadata_settings_headers.get('LFXO_FREQUENCY')
                    data += bytes(int(header, 16).to_bytes(4, 'little'))
                    printInfo("LFXO frequency:", frequency)
                    # LFXO frequency table is decimal
                    data += bytes(frequency.to_bytes(4, 'little'))

    printInfo("XTAL data:", data)
    return data

def processChangeSets(changeSets):
    printInfo('Processing ChangeSets')
    data = bytearray()   
    for changeSet in changeSets:
        data += bytes(int(changeSet['address'], 16).to_bytes(4, 'little'))
        data += bytes(int(changeSet['mask'], 16).to_bytes(4, 'little'))
        data += bytes(int(changeSet['value'], 16).to_bytes(4, 'little'))

    return data

def processRegisterSettings(configuration):
    printInfo("Process Register_Settings")
    data = bytearray()
    for sec in configuration:
        if sec == 'change_sets':
            data = processChangeSets(configuration[sec])
    printInfo("register_settings: ", data)
    return data

def processClocks(configuration):
    printInfo("Process Clocks")
    data = bytearray()
    for sec in configuration:
        if sec == 'change_sets':
            data = processChangeSets(configuration[sec])
        # TODO: process other clock settings
    printInfo("clocks: ", data)
    return data

def processPinMux(configuration):
    printInfo("Process PinMux")
    data = utils.pinmux.json_to_bin(configuration)
    return bytearray(data)

def processFirewall(configuration, is_icv):
    printInfo("Process Firewall")
    utils.firewall.OUTPUT_FILE = 'build/images/fw_temp.bin'
    firewall_json_to_bin(configuration, is_icv)

def processWounding(configuration):
    printInfo("Process Wounding")

    printInfo("Wounding Data: ", configuration)

    num = int(configuration, 0)  
    data = num.to_bytes(4,byteorder='little')

    return bytearray(data)

def processMiscellaneous(configuration):
    printInfo("Process Miscellaneous")

    data = bytearray()   
    for sec in configuration:
        if ('id' not in sec) or ('value' not in sec):
            continue
        if not sec['id'] in misc_settings_headers:
            printInfo("[WARN] SE not supported: ", sec['id'])
            continue
        header = misc_settings_headers.get(sec['id'])
        data += bytes(int(header, 16).to_bytes(4, 'little'))
        data += bytes(int(sec['value']).to_bytes(4, 'little'))

    return data

def gen_device_config(file, is_icv):

    cfg = read_global_config('build/config/' + file)
    validateSections(cfg, file)   #request from SE-1938
    binFile = file[:-5] + '.bin'
    f = createBinary('build/images/' + binFile)
    for sec in cfg:
        printInfo("Create area for: " + sec)

        if sec == "metadata":
            obj = processMetadata(cfg[sec])
            header = (DEVICE_CONFIG_METADATA<<24) | (len(obj) & HEADER_SIZE_MASK)
            writeBinary(f, struct.pack("I", header))
            writeBinary(f, obj)

        if sec == "wounding":
            obj = processWounding(cfg[sec])
            header = (DEVICE_CONFIG_WOUNDING<<24) | (len(obj) & HEADER_SIZE_MASK)
            writeBinary(f, struct.pack("I", header))
            writeBinary(f, obj)

        if sec == "pinmux":
            obj = processPinMux(cfg[sec])
            header = (DEVICE_CONFIG_PIN_MUX<<24) | (len(obj) & HEADER_SIZE_MASK)
            writeBinary(f, struct.pack("I", header))
            writeBinary(f, obj)

        if sec == "clocks":
            obj = processClocks(cfg[sec])
            header = (DEVICE_CONFIG_CLOCKS<<24) | (len(obj) & HEADER_SIZE_MASK)
            writeBinary(f, struct.pack("I", header))
            writeBinary(f, obj)

        if sec == "register_settings":
            obj = processClocks(cfg[sec])
            header = (DEVICE_CONFIG_REGISTER_SETTINGS<<24) | (len(obj) & HEADER_SIZE_MASK)
            writeBinary(f, struct.pack("I", header))
            writeBinary(f, obj)

        if sec == "firewall":
            processFirewall(cfg[sec], is_icv)

            # checking the Alif TOC Package size
            try:
                fsize = os.path.getsize(utils.firewall.OUTPUT_FILE)
                printInfo("Firewall binary size: " + str(fsize) + " bytes")
            except:
                print(sys.exc_info()[0])
                print("[ERROR] Veryfing Firewall binary size")
                sys.exit(EXIT_WITH_ERROR)

            header = (DEVICE_CONFIG_FIREWALL<<24) | (fsize & HEADER_SIZE_MASK)
            writeBinary(f, struct.pack("I", header))

            # reading the file to include it in Device Config. binary
            with open(utils.firewall.OUTPUT_FILE, 'rb') as bin_file:
                writeBinary(f, bin_file.read())

        if sec == "miscellaneous":
            obj = processMiscellaneous(cfg[sec])
            header = (DEVICE_CONFIG_MISCELLANEOUS<<24) | (len(obj) & HEADER_SIZE_MASK)
            writeBinary(f, struct.pack("I", header))
            writeBinary(f, obj)

    tail = DEVICE_CONFIG_END_OF_LIST<<24
    writeBinary(f, struct.pack("I", tail))
    closeBinary(f)
