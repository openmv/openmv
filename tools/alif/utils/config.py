import sys
import json
from json.decoder import JSONDecodeError

# Define Version constant for each separate tool
# 0.06.000 added package and offset params
TOOL_VERSION = "0.07.000"

# DB files
CONFIG_FILE = 'utils/global-cfg.db'
DEVICE_DB_FILE   = 'utils/devicesDB.db'
FEATURES_DB_FILE = 'utils/featuresDB.db'
HASHES_DB_FILE = 'utils/hashesDB.db'


EXIT_WITH_ERROR = 1

# Device Part# and Revision
DEVICE_PART_NUMBER = ''
DEVICE_REVISION = ''
DEVICE_FAMILY = ''
DEVICE_FEATURE_SET = ''
DEVICE_PACKAGE = ''
DEVICE_REV_PACKAGE_EXT = ''
DEVICE_REV_BAUD_RATE = ''
DEVICE_OFFSET = ''

# Hashes
HASHES_DB = []

# MRAM Access Interface
MRAM_BURN_INTERFACE = ''

# JTAG Adapter
JTAG_ADAPTER = ''

# memory defines for Alif/OEM MRAM Addresses
MRAM_BASE_ADDRESS   = 0x00
ALIF_BASE_ADDRESS   = 0x00
APP_BASE_ADDRESS    = 0x00
# memory defines for Alif/OEM MRAM Sizes
MRAM_SIZE           = 0x00
ALIF_MRAM_SIZE      = 0x00
APP_MRAM_SIZE       = 0x00

# Hardcoded values
APP_OFFSET = 0x00

# parameters by architecture
SERAM_BANKS     = 0

# ALIF TOC POINTER SIZE (pointers to)
ALIF_TOC_POINTER_SIZE = 16

def read_global_config(file):

#    print("Opening file %s" % file);
    f = open(file, 'r')
    try:
        cfg = json.load(f)

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
    return cfg

def save_global_config(devDescription, devRevision):
    with open(CONFIG_FILE, 'r', encoding='utf-8') as f:
        cfg = json.load(f)

    with open(CONFIG_FILE, 'w', encoding='utf-8') as f:  
        cfg['DEVICE']['Part#'] = devDescription
        cfg['DEVICE']['Revision'] = devRevision
        json.dump(cfg, f, ensure_ascii=False, indent=4)


def checkAttribute(cfg, attribute):
    try:
        test = cfg[attribute]

    except KeyError as e:
        print('Parameter ' + str(e) + ' is not configured')
        sys.exit(EXIT_WITH_ERROR)

    except:
        print('General error checking attribute ' + attribute)
        sys.exit(EXIT_WITH_ERROR)

def getPartDescription(devPartNumber):
    # read devices DB
    db = read_global_config(DEVICE_DB_FILE)  
    partDescription = ''
    for desc in db:
        if devPartNumber in desc:
            partDescription = desc
            break

    if partDescription == '':
        print("We couldn't find the " + devPartNumber + " in the available parts")
        sys.exit(EXIT_WITH_ERROR)

    return partDescription

def load_device_config(devDescription, devRevision):
    global DEVICE_PART_NUMBER
    global DEVICE_REVISION
    global DEVICE_FAMILY
    global DEVICE_FEATURE_SET
    global DEVICE_PACKAGE
    global DEVICE_REV_PACKAGE_EXT
    global DEVICE_REV_BAUD_RATE
    global DEVICE_OFFSET
    global MRAM_BASE_ADDRESS
    global ALIF_BASE_ADDRESS
    global APP_BASE_ADDRESS
    global MRAM_SIZE
    global ALIF_MRAM_SIZE
    global APP_MRAM_SIZE
    global SERAM_BANKS

    # assign Part#/Rev according detection
    DEVICE_PART_NUMBER = devDescription
    DEVICE_REVISION = devRevision

    # read devices DB
    db = read_global_config(DEVICE_DB_FILE)

    # read architectures DB
    features = read_global_config(FEATURES_DB_FILE)

    DEVICE_FAMILY        = db[devDescription]['family']
    DEVICE_FEATURE_SET   = db[devDescription]['featureSet']
    DEVICE_PACKAGE       = db[devDescription]['package']

    MRAM_BASE_ADDRESS      = int(features[db[devDescription]['featureSet']]['mram_base'],16) 
    REVISIONS              = features[db[devDescription]['featureSet']]['revisions']
    DEVICE_REV_PACKAGE_EXT = features[db[devDescription]['featureSet']]['rev_package_ext']
    DEVICE_REV_BAUD_RATE   = features[db[devDescription]['featureSet']]['rev_baud_rate']
    DEVICE_OFFSET        = db[devDescription]['offset']

    # validate configuration
    if devRevision not in REVISIONS:
        print('Revision is invalid!')
        sys.exit(EXIT_WITH_ERROR)


    # set MRAM Size (same for all revisions)
    MRAM_SIZE = int(features[db[devDescription]['featureSet']]['mram_total'], 16)
    SERAM_BANKS = features[db[devDescription]['featureSet']]['seram_banks']

    # relative addresses (offsets)
    alif_offset = int(db[devDescription]['alif_offset'], 16)
    app_size    = int(db[devDescription]['app_size'], 16) - APP_OFFSET

    # calculate 
    ALIF_BASE_ADDRESS = MRAM_BASE_ADDRESS + alif_offset
    APP_BASE_ADDRESS  = MRAM_BASE_ADDRESS + APP_OFFSET
    ALIF_MRAM_SIZE = MRAM_SIZE - alif_offset - ALIF_TOC_POINTER_SIZE
    APP_MRAM_SIZE = app_size


def load_global_config():
    global DEVICE_PART_NUMBER
    global DEVICE_REVISION
    global MRAM_BURN_INTERFACE
    global JTAG_ADAPTER
    global HASHES_DB
 
    cfg = read_global_config(CONFIG_FILE)
 
    # validate configuration parameters
    # check parameter is configured...
    checkAttribute(cfg['DEVICE'], 'Revision')

    # read hashes DB
    hashes = read_global_config(HASHES_DB_FILE)

    DEVICE_PART_NUMBER   = cfg['DEVICE']['Part#']
    DEVICE_REVISION = cfg['DEVICE']['Revision']  
    
    # load rest of config params
    load_device_config(DEVICE_PART_NUMBER, DEVICE_REVISION)

    # retrieve DEV Environments 
    HASHES_DB            = hashes 

    # set MRAM BURNER Access Interface
    MRAM_BURN_INTERFACE = cfg['MRAM-BURNER']['Interface']

    # set JTAG Adapter
    JTAG_ADAPTER = cfg['MRAM-BURNER']['Jtag-adapter']

