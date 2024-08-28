#!/usr/bin/env python3
"""
    recovery.py

    Support
        - SEROM Recovery mode
        - Allows updating of the MRAM through SEROM
        - This is always ISP mode.
    __author__ = ""
    __copyright__ = "ALIF Seminconductor"
    __version__ = "0.1.0"
    __status__ = "Dev"

    TODO:
    - baud rate should be set to the SEROM default
"""
# pylint: disable=unused-argument, invalid-name, bare-except
import sys
sys.path.append("./isp")
from isp_protocol import *
from isp_core import *
from isp_util import *
import utils.config
from utils.config import *
from utils.user_validations import validateArgList
import device_probe
import time

# Probe error codes
PROBE_OK        = 0
PROBE_SEROM     = 1
PROBE_SERAM     = 2
PROBE_NO_MESSAGE= 3

EXIT_WITH_ERROR = 1

BRING_UP_MODE   = 1

def write_mram(isp, fileName, destAddress, verbose_display):
    """
        write_mram - use ISP method to write MRAM
    """
    try:
        f = open(fileName, 'rb')
    except IOError as e:
        print('[ERROR] {0}'.format(e))
        sys.exit(EXIT_WITH_ERROR)
    with f:
        fileSize = file_get_size(f)
#        print("FILESIZE = %d" %fileSize)

        # SEROM Recovery uses an Offset rather than an address
        MRAM_BASE_ADDRESS = utils.config.MRAM_BASE_ADDRESS
        offset = 0
        data_size = 16
        writeAddress = int(destAddress,base=16) - MRAM_BASE_ADDRESS

#        print("Actual offset = ", hex(writeAddress))
        number_of_blocks = fileSize // data_size
        left_over_blocks = fileSize % data_size

        start_time = time.time()
        while number_of_blocks != 0:
            f.seek(offset)
            mram_line = f.read(data_size)

            if verbose_display is False:
                progress_bar(fileName,offset+data_size,fileSize)
            message = isp_mram_write(isp,writeAddress,mram_line)

            number_of_blocks = number_of_blocks - 1
            offset = offset + data_size
            writeAddress = writeAddress + data_size # increment by 16
            if isp.CTRLCHandler.Handler_exit():
                print("[INFO] CTRL-C")
                break

        end_time = time.time()
        print("\r")

        if verbose_display is False:
            print("[INFO] recovery time {:10.2f} seconds".format(end_time-start_time))

    f.close()

def checkTargetWithSelection(targetDescription, targetRevision, selectedDescription, selectedRevision):
    partIsDifferent = False
    if targetDescription != selectedDescription:
        print('Connected target is not the default Part#')
        partIsDifferent = True

    if targetRevision != selectedRevision:
        print('Connected target is not the default Revision')
        partIsDifferent = True

    if partIsDifferent:
        try:
            answer = input('Do you want to set this part as default? (y/n): ')
        except EOFError as e:
            print('\nUser aborted the process')
            sys.exit()

        if answer.lower() == 'y' or answer.lower() == 'yes':
            save_global_config(targetDescription, targetRevision)

def recovery_action(isp):
    """
        Recover MRAM via SEROM
    """
    # probe the device before update
    device = device_probe.device_get_attributes(isp)
    # check SERAM is the bootloader stage
    print('Bootloader stage: ' + device_probe.STAGE_TEXT[device.stage])
    if device.stage == device_probe.STAGE_SERAM:
        print('[ERROR] Device not in Recovery mode, use systemUpdatePackage Tool')
        return

    if device.revision == 'B2':  #Device Part# is not retrieved by SEROM in B2
        # load parameters based on Part# selected in tools-config
        load_global_config()
        print('Device Revision: ' + device.revision)
    
    else:
        if ord(device.part_number[:1]) == 0:  # blank Part#
            if BRING_UP_MODE:
                print('Bring Up mode - Blank part detected!')
                if device.revision in ['B0','B2', 'B4']:  # Ensemble 
                    device.part_number = 'AE722F80F55D5LS'
                elif device.revision in ['B1', 'A0']:     # Balletto  B1 ?
                    device.part_number = 'AB1C1F4M51920PH'
                else:
                    print('[ERROR] Revision is not reconginzed', device.revision)
                    sys.exit(EXIT_WITH_ERROR)        

            else:
                print('[ERROR] There is no Part# in the device!')  
                sys.exit(EXIT_WITH_ERROR)

        # selected device from global-cfg.db
        selectedDescription = utils.config.DEVICE_PART_NUMBER
        selectedRevision = utils.config.DEVICE_REVISION

        # print detected device
        print('Detected Part#: ' + device.part_number)
        print('Detected Revision: ' + device.revision)
        partDescription = getPartDescription(device.part_number)
        # load configuration from detected device
        load_device_config(partDescription, device.revision)
        # check the default Part#/Rev in tools-config and offer to switch
        checkTargetWithSelection(partDescription, device.revision, selectedDescription, selectedRevision)


    # update params from either selected part (B2) or detected part (B3/B4, etc)
    DEVICE_PART_NUMBER = utils.config.DEVICE_PART_NUMBER
    DEVICE_REVISION = utils.config.DEVICE_REVISION
    DEVICE_PACKAGE = utils.config.DEVICE_PACKAGE
    DEVICE_REV_PACKAGE_EXT = utils.config.DEVICE_REV_PACKAGE_EXT
    DEVICE_OFFSET  = utils.config.DEVICE_OFFSET    
    MRAM_BASE_ADDRESS = utils.config.MRAM_BASE_ADDRESS
    ALIF_BASE_ADDRESS = utils.config.ALIF_BASE_ADDRESS
    MRAM_SIZE = utils.config.MRAM_SIZE
    HASHES_DB = utils.config.HASHES_DB

    if device.revision == 'B2':
        if device.revision != DEVICE_REVISION:
            print("[ERROR] Target device revision (%s) mismatch with configured device (%s)"
                % (device.revision,DEVICE_REVISION))
            sys.exit(EXIT_WITH_ERROR)

    env_ext = ''
    if device.env.lower() in HASHES_DB:
        if HASHES_DB[device.env] == 'DEV':
           env_ext = '-dev' 

    # for devices in CM LCS, we use the DEV package
    if device.env == '00000000000000000000000000000000':
        print('Device is not provisioned!')
        env_ext = '-dev'

    # Start the recovery process
    print('[INFO] System TOC Recovery with parameters:')
    print('- Device Part# ' + DEVICE_PART_NUMBER + ' - Rev: ' + DEVICE_REVISION)
    print('- MRAM Base Address: ' + hex(ALIF_BASE_ADDRESS))

    argList = ''
    action = 'Burning: '

    rev_ext = DEVICE_REV_PACKAGE_EXT[DEVICE_REVISION]
    alif_image = 'alif/' + DEVICE_PACKAGE + '-' + rev_ext + env_ext + '.bin'
    alif_offset = 'alif/' + DEVICE_OFFSET + '-' + rev_ext + env_ext + '.bin'

    # check images exist...
    if not os.path.exists(alif_image):
        print('Image ' + alif_image + ' does not exist!')
        sys.exit(EXIT_WITH_ERROR)

    if not os.path.exists(alif_offset):
        print('Image ' + alif_offset + ' does not exist!')
        sys.exit(EXIT_WITH_ERROR)

    argList = '../' + alif_image + ' ' + hex(ALIF_BASE_ADDRESS) + \
              ' ../' + alif_offset + ' ' + hex(MRAM_BASE_ADDRESS + MRAM_SIZE - 16)      

    # validate all parameters
    #argList = validateArgList(action, argList.strip())
    if sys.platform == "linux" or sys.platform == "darwin":
        argList = argList.replace('../','')
    else:
        argList = argList.replace('/','\\')

    items = argList.split(' ')

    for e in range(1, len(items),2):
        addr = items[e]
        fileName = items[e-1]
        fileName = fileName.replace('..\\','')

        write_mram(isp, fileName, addr, False)

    print("[INFO] Target reset")
    isp_reset(isp)      # Reset the target
