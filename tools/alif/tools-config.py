#!/usr/bin/env python3
#pylint: disable=invalid-name,superfluous-parens,anomalous-unicode-escape-in-string

import sys, shutil
import json
from json.decoder import JSONDecodeError
import utils.config
from utils.config import *
from utils.gen_fw_cfg import *

# Define Version constant for each separate tool
# 0.05.000 - add cmd line options and multiple key directories 
# 0.06.000 - add new DEV key for SPARK
# 0.07.000 - fixed SE-2761 (keyEnv is not being updated in Azure)
TOOL_VERSION = "0.07.000"

EXIT_WITH_ERROR = 1

CONFIG_FILE = 'utils/global-cfg.db'
FAMILY_DB = 'utils/familiesDB.db'
FEATURES_DB = 'utils/featuresDB.db'
DEVICE_DB = 'utils/devicesDB.db'
JTAG_ADAPTERS = 'utils/jtag-adapters.db'

DEVICE_CFG_FILE = "build/config/device-config.json"
JTAG_ADAPTERS_FILE = "build"
KEY_PATH = 'utils/key/'
CERT_PATH = 'cert/'

# DEV key environments
FUSION_REV_A1 = 'fusion_rev_a1'
FUSION_REV_B0 = 'fusion_rev_b0'
FUSION_REV_B4 = 'fusion_rev_b4'
SPARK_REV_A0  = 'spark_rev_a0'
EAGLE_REV_A0  = 'eagle_rev_a0'
# future: Spark, etc


mram_interface = ['jtag', 'isp']

def read_json_file(file):

    f = open(file, 'r')
    try:
        data = json.load(f)

    except JSONDecodeError as e:
        print("ERROR in JSON file.")
        print(str(e))
        sys.exit(EXIT_WITH_ERROR)

    except ValueError as v:
        print("ERROR in JSON file:")
        print(str(v))
        sys.exit(EXIT_WITH_ERROR)

    except:
        print("ERROR: Unknown error loading JSON file")
        sys.exit(EXIT_WITH_ERROR)

    f.close()
    return data

def save_global_config(cfg):
    with open(CONFIG_FILE, 'w', encoding='utf-8') as f:
        json.dump(cfg, f, ensure_ascii=False, indent=4)

def showAndSelectOptions(list, default):
    option = 'x'
    while option == 'x':
        print('\nAvailable options:\n')
        i = 1
        for item in list:
            if item == 'SEP':
                print('')
            else:
                if item == default:   
                    print(str(i) + ' - ' + item + ' (default)')
                else:
                    print(str(i) + ' - ' + item)    
                i += 1
        while 'SEP' in list:
            list.remove('SEP')

        option = input('\nPlease enter the number of your option: ')
        if option == '':
            return default
        try:
            idx = int(option)
        except ValueError:
            print('Invalid option - Please try again')
            option = 'x'
            continue
        
        if idx < 1 or idx > len(list):
            print('Invalid option - Please try again')
            option = 'x'

    return list[int(idx -1)]

def update_device_config_file(device_part, device_revision):

    if isThisAPP(): # this is logic only for ICV device configuration
        return

    load_global_config()
    # generate the temp file 'build/fw_cfg.json'
    gen_fw_cfg_icv(device_part["family"], device_part["mram_size"], device_part["sram_size"], device_revision, device_part["featureSet"])

    # open the device config file
    with open(DEVICE_CFG_FILE, "r") as device_config_file:
        device_config_json = json.load(device_config_file)

    # incorporate fw_cfg.json into the device config file
    with open(FW_CFG_FILE, "r") as fw_cfg_file:
        fw_json = json.load(fw_cfg_file)
        device_config_json["firewall"] = fw_json
        # add the wounding information as well
        device_config_json["wounding"] = device_part["wounding"]

    # save the updated device config file
    with open(DEVICE_CFG_FILE, "w") as device_config_file:
        json.dump(device_config_json, device_config_file, indent=4)

def printMenu(cfg):
    print('\n')
    print('* * * * * * * * * * * * * * * * * * * * * *')
    print('Current configuration')
    
    print(' - DEVICE Family: ' + devDB[cfg['DEVICE']['Part#']]['family'] + ' - Part#: ' + cfg['DEVICE']['Part#'] + ' - Rev: ' + cfg['DEVICE']['Revision'])
    print(' - MRAM BURNER')
    print('     Interface:  ' + cfg['MRAM-BURNER']['Interface'])
    print('     JTAG Adapter: ' + cfg['MRAM-BURNER']['Jtag-adapter'])
    print('* * * * * * * * * * * * * * * * * * * * * *')

def loadParts(family):
    parts = []
    for key in devDB:
        if devDB[key]['family'] == family:
            parts.append(key)
    return parts

def validateRevision(dev_revisions, current_revision):
    if current_revision not in dev_revisions:
        current_revision = dev_revisions[0]
    return current_revision    

def clean_directory_rot():
    # clean key folder
    path = KEY_PATH
    for file in os.listdir(path):
        if file[0:3].lower() == 'oem':
            continue
        f = path + file
        if os.path.isfile(f):
            os.remove(f)

    # clean certs folder
    path = CERT_PATH
    for file in os.listdir(path):
        if file[0:3].lower() == 'oem':
            continue
        f = path + file
        if os.path.isfile(f):
            os.remove(f)


def copy_content_rot(rot_dir):
    # this only applies to ICV DEV key release... (local, no Azure)
    if isThisPROD():
        return
    
    # copy key env from the selected RoT
    path = KEY_PATH + rot_dir
    for file in os.listdir(path):
        f = path + '/' + file
        if file[0:3].lower() == 'oem':
            continue
        shutil.copy(f, KEY_PATH)

    # copy certs env from the selected RoT
    path = CERT_PATH + rot_dir
    for file in os.listdir(path):
        f = path + '/' + file
        #shutil.copy(f, 'cert/')
        shutil.copy(f, CERT_PATH)

def isThisAPP():
    if os.path.isdir('alif/'):
        return True
    return False

def isThisPROD():
    for file in os.listdir(KEY_PATH):
        f = KEY_PATH + '/' + file
        if os.path.isdir(f):
            return False
    return True

def setKeyEnvironment(cfg):

    # do not apply for APP tools
    if isThisAPP():
        return

    keyEnvCfg = cfg['DEVICE']['keyEnv']
    feature = devDB[cfg['DEVICE']['Part#']]['featureSet']
    revision = cfg['DEVICE']['Revision']
    # check key env rules
    keyEnv = FUSION_REV_B4   # default key env for REV_B4 FUSION
    if feature == 'Fusion' and revision == 'A1':   # backward compatibility
        keyEnv = FUSION_REV_A1
    if feature == 'Fusion' and revision != 'B4':   # Rev B4 has a new RoT
        keyEnv = FUSION_REV_B0        
    if feature == 'Spark' and revision == 'A0':
        keyEnv = SPARK_REV_A0
    if feature == 'Eagle' and revision == 'A0':
        keyEnv = EAGLE_REV_A0
    # future rules...

    # set the new key env and save it in global-cfg.json 
    print("Setting a new Key Environment")
    clean_directory_rot()
    copy_content_rot(keyEnv)
    cfg['DEVICE']['keyEnv'] = keyEnv
    save_global_config(cfg)

def processCmdLineOption(args):
    # read global cfg
    cfg = read_json_file(CONFIG_FILE)
    # validate options    
    if args.part != None:
        try:
            cfg['DEVICE']['Part#'] = args.part.upper()
        except KeyError:
            print('Invalid Part#')
            sys.exit(EXIT_WITH_ERROR)  

    if args.rev != None:
        dev_revisions = featDB[devDB[cfg['DEVICE']['Part#']]['featureSet']]['revisions']
        if args.rev.upper() in dev_revisions:
            cfg['DEVICE']['Revision'] = args.rev.upper()
        else:
            print('Invalid Revision')
            sys.exit(EXIT_WITH_ERROR)  

    save_global_config(cfg)
    update_device_config_file(devDB[cfg['DEVICE']['Part#']], cfg['DEVICE']['Revision'])    
    setKeyEnvironment(cfg)

def main():
    global devDB
    global featDB

    # Deal with Command Line
    parser = argparse.ArgumentParser(description=
                                     'SETOOLS Selection')
    parser.add_argument("-p", "--part", type=str,
                       help="Part#")   
    parser.add_argument("-r", "--rev", type=str,
                       help="Revision")     

    # load data from DBs
    devDB = read_json_file(DEVICE_DB)

    families = []
    famDB = read_json_file(FAMILY_DB)
    for key in famDB:
        families.append(key)

    features = []
    featDB = read_json_file(FEATURES_DB)
    for key in featDB:
        features.append(key)

    dev_revisions = []

    jtag_adapters = []
    f = open(JTAG_ADAPTERS, 'r')
    jtag_adapters = f.read().strip().split('\n')

    args = parser.parse_args()
    if args.part != None or args.rev != None:
        processCmdLineOption(args)
        sys.exit()

    print('SETOOLS OPTIONS CONFIGURATION')
    option = ''
    while option != 'Exit':
        cfg = read_json_file(CONFIG_FILE)
        dev_revisions = featDB[devDB[cfg['DEVICE']['Part#']]['featureSet']]['revisions']
        printMenu(cfg)
        option = showAndSelectOptions(['Part#', 'Revision', 'Interface', 'JTAG Adapter', 'SEP', 'Exit'], 'Exit')
        if option == 'Part#':
            selected_family = showAndSelectOptions(families, devDB[cfg['DEVICE']['Part#']]['family'])
            parts = loadParts(selected_family)
            cfg['DEVICE']['Part#'] = showAndSelectOptions(parts, cfg['DEVICE']['Part#'])
            dev_revisions = featDB[devDB[cfg['DEVICE']['Part#']]['featureSet']]['revisions']
            cfg['DEVICE']['Revision'] = validateRevision(dev_revisions, cfg['DEVICE']['Revision'])  
            update_device_config_file(devDB[cfg['DEVICE']['Part#']], cfg['DEVICE']['Revision'])

        elif option == 'Revision':
            cfg['DEVICE']['Revision'] = showAndSelectOptions(dev_revisions, cfg['DEVICE']['Revision'])
            update_device_config_file(devDB[cfg['DEVICE']['Part#']], cfg['DEVICE']['Revision'])

        elif option == 'Interface':
            cfg['MRAM-BURNER']['Interface'] = showAndSelectOptions(mram_interface, cfg['MRAM-BURNER']['Interface'])

        elif option == 'JTAG Adapter':
            cfg['MRAM-BURNER']['Jtag-adapter'] = showAndSelectOptions(jtag_adapters, cfg['MRAM-BURNER']['Jtag-adapter'])            

        save_global_config(cfg)
        setKeyEnvironment(cfg)

if __name__ == "__main__":
    main()

