import sys
import os
import glob
from pathlib import Path
import json
from json.decoder import JSONDecodeError
from utils import cert_sb_content_util
import utils.toc_common
from printInfo import *

EXIT_WITH_ERROR = 1

certPath = Path("cert/")
imagePath = Path("build/images/")


# we need to pad 12 bytes to put the image into a 16-byte aligned address
CERT_CHAIN_SIZE = (840 * 2) + 868 + 12   #  2560 bytes (0xA00)
# we pad out to have 16-byte aligned sizes = 880 total size
CONT_CERT_SIZE  = 868 + 12
# we need to pad 12 bytes to put the image into a 16-byte aligned address
CERT_CHAIN_SIZE = (840 * 2) + 868 + 12

def getImageSize(file):
    """
       find the image size
    """

    try:
        fsize = os.path.getsize(file)

    except FileNotFoundError:
        print("ERROR: The file " + file + " was not found in build/ directory!")
        print("Please check the file and try again")
        sys.exit(EXIT_WITH_ERROR)

    except:
        print(sys.exc_info()[0])
        print("ERROR: getting the image size for file " + file)
        sys.exit(EXIT_WITH_ERROR)

    #print("OS file size: " + str(fsize))

    padlen = 0

    if fsize % 16:
        # If we have to pad out
        newsize = fsize + (16 - (fsize % 16))
        #print("Size should be " + str(newsize))
        padlen = (16 - (fsize % 16))

    return fsize, padlen


def validateHexAddress(address, param, image):
    """
        check address
    """
    if (address[0:2].lower() != '0x'):
        print(param + ' for image ' + image + ' should in in hex!')
        sys.exit(EXIT_WITH_ERROR)

    try:
        val = int(address, 0)

    except ValueError:
        print('Invalid address in ' + param + ' for image ' + image)
        sys.exit(EXIT_WITH_ERROR)




def globalToLocalAddress(globalAddress):
    """
        convert addresses
    """
    # Global to Local Addresses Convertion is based on this table
    #Rgn​ SE Address Region (CM0+)​ RegSize​   Global add Region​    Coverage​
    #  EXAMPLE FOR REV_A0 DEVICES
    #1​	0x6000_0000 - 0x8000_0000​   512MB​	0x0000_0000 - 0x2000_0000​	Boot register, CVM, XNVM, Host Peripherals, Power, Firewall, MHU​
    #2​	0x8000_0000 - 0x8200_0000​	32MB	0x4800_0000 - 0x4A00_0000​  EXPMST0 Shared Peripherals​
    #3​	0x8800_0000 - 0x8900_0000​	16MB​	0x5000_0000 - 0x5100_0000​	EXTSYS0 HP-CM55 TCM​
    #4​	0x9000_0000 - 0x9800_0000​	128MB​	0x6000_0000 - 0x6800_0000​	EXTSYS1 TCMs​
    #5​	0x9800_0000 - 0x9C00_0000​	64MB​	0x7000_0000 - 0x7400_0000​	Low Power Peripherals (VBAT at 0x7000 and LPAON at 0x7100)​
    #6​	0xA000_0000 - 0xA800_0000​	128MB	0x8000_0000 - 0x8800-0000​	OCVM​
    #7​	0xC000_0000 – 0xE000_0000​	512MB​	0xD000_0000 – 0xF000_0000​	OctalSPI0 and OctalSPI1​
    #
    #The upper region addresses are exclusive for the purpose of readability
    #(e.g. instead of 0x7FFF_FFFF the upper address is listed as 0x8000_0000)​

    # Steps for the conversion"
    #   1. Check if provided global address is in one of the above regions
    #   2. if in region, apply the corresponding Delta to convert to a Local address
    #   3. If NOT in region, we exit with error and terminate the program

    #Rgn    Delta
    #1      + 0x6000-0000
    #2      + 0x3800-0000
    #3      + 0x3800-0000
    #4      + 0x3000-0000
    #5      + 0x2800-0000
    #6      + 0x2000-0000
    #7      - 0x1000-0000


    # Global addresses definition for REV_Ax devices
    regions_Ax = [
        ['0x00000000','0x20000000'],
        ['0x48000000','0x4A000000'],
        ['0x50000000','0x51000000'],
        ['0x60000000','0x68000000'],
        ['0x70000000','0x74000000'],
        ['0x80000000','0x88000000'],
        ['0xD0000000','0xF0000000']
    ]

    deltas_Ax = [
        ['+','0x60000000'],
        ['+','0x38000000'],
        ['+','0x38000000'],
        ['+','0x30000000'],
        ['+','0x28000000'],
        ['+','0x20000000'],
        ['-','0x10000000']
    ]

    # Global addresses definition for REV_B0 devices
    regions_B0 = [
        ['0x02000000','0x02400000'],    # SRAM0_BASE
        ['0x08000000','0x08280000'],    # SRAM1_BASE
        ['0x50000000','0x50040000'],    # M55_HP_ITCM_BASE
        ['0x50800000','0x50900000'],    # M55_HP_DTCM_BASE  
        ['0x58000000','0x58040000'],    # M55_HE_ITCM_BASE
        ['0x58800000','0x58840000'],    # M55_HE_DTCM_BASE
        ['0x62000000','0x62100000'],    # MODEM_ITCM_BASE
        ['0x62100000','0x62200000'],    # MODEM_DTCM_BASE
        ['0x63000000','0x63080000'],    # DSP_ITCM_BASE
        ['0x63100000','0x63300000'],    # DSP_DTCM_BASE
        ['0x60000000','0x60040000'],    # GNSS_ITCM_BASE
        ['0x60040000','0x600C0000'],    # GNSS_DTCM_BASE   
        ['0x80000000','0x80600000']     # MRAM_BASE
    ]

    deltas_B0 = [
        ['+','0x60000000'],
        ['+','0x60000000'],
        ['+','0x44000000'], 
        ['+','0x44000000'],
        ['+','0x40000000'],
        ['+','0x40000000'],
        ['+','0x3C000000'],
        ['+','0x3C000000'],
        ['+','0x3C000000'],
        ['+','0x3C000000'],
        ['+','0x3C000000'],
        ['+','0x3C000000'],
        ['+','0x10000000']
    ]

    # Global addresses definition for Spark devices
    regions_Spark = [
        ['0x00000000','0x20000000'],
        ['0x40000000','0x50000000'],
        ['0x50000000','0x501E0000'],
        ['0x58000000','0x5C000000'],
        ['0x60000000','0x64000000'],
        ['0x80000000','0x84000000'],
        ['0xA0000000','0xE0000000']
    ]

    deltas_Spark = [
        ['+','0x60000000'],
        ['+','0x40000000'],
        ['+','0x44000000'],
        ['+','0x40000000'],
        ['+','0x3C000000'],
        ['+','0x10000000'],
        ['+','0x00000000']
    ]

    # Global addresses definition for REV_B0 devices
    regions_Eagle = [
        ['0x02000000','0x02400000'],    # SRAM0_BASE
        ['0x08000000','0x08280000'],    # SRAM1_BASE
        ['0x50000000','0x50040000'],    # M55_HP_ITCM_BASE
        ['0x50800000','0x50900000'],    # M55_HP_DTCM_BASE  
        ['0x58000000','0x58040000'],    # M55_HE_ITCM_BASE
        ['0x58800000','0x58840000'],    # M55_HE_DTCM_BASE
        ['0x62000000','0x62100000'],    # MODEM_ITCM_BASE
        ['0x62100000','0x62200000'],    # MODEM_DTCM_BASE
        ['0x63000000','0x63080000'],    # DSP_ITCM_BASE
        ['0x63100000','0x63300000'],    # DSP_DTCM_BASE
        ['0x60000000','0x60040000'],    # GNSS_ITCM_BASE
        ['0x60040000','0x600C0000'],    # GNSS_DTCM_BASE   
        ['0x80000000','0x80600000']     # MRAM_BASE
    ]

    deltas_Eagle = [
        ['+','0x60000000'],
        ['+','0x60000000'],
        ['+','0x44000000'], 
        ['+','0x44000000'],
        ['+','0x40000000'],
        ['+','0x40000000'],
        ['+','0x3C000000'],
        ['+','0x3C000000'],
        ['+','0x3C000000'],
        ['+','0x3C000000'],
        ['+','0x3C000000'],
        ['+','0x3C000000'],
        ['+','0x10000000']
    ]

    # create a list of valid global regions
    #print(utils.config.DEVICE_REVISION)
    if utils.config.DEVICE_FEATURE_SET == 'Fusion':
        if utils.config.DEVICE_REVISION == 'A1':
            regions = regions_Ax
            deltas = deltas_Ax
        else:
            regions = regions_B0
            deltas = deltas_B0

    elif utils.config.DEVICE_FEATURE_SET == 'Spark':
        regions = regions_Spark
        deltas = deltas_Spark     
    elif utils.config.DEVICE_FEATURE_SET == 'Eagle':
        regions = regions_Eagle
        deltas = deltas_Eagle     
    else:
        print('ERROR: Invalid Device type!')
        sys.exit(EXIT_WITH_ERROR)   


    #print(globalAddress)
    localAddress = int(globalAddress, 16)
    notFound = True
    idx = 0
    for address in regions:
        LOWER = int(address[0], 16)
        UPPER = int(address[1], 16)
        if localAddress >= LOWER and localAddress < UPPER:
            #print('Address inside boundaries - region ' + str(idx))
            notFound = False
            break
        idx += 1

    if notFound:
        print('ERROR: Invalid Global Address. Please check!')
        sys.exit(EXIT_WITH_ERROR)

    delta = int(deltas[idx][1], 16)
    if deltas[idx][0] == '+':
        localAddress += delta
    elif deltas[idx][0] == '-':
        localAddress -= delta
    else:
        print('ERROR in determining the right operation in global address conversion!')
        sys.exit(EXIT_WITH_ERROR)

    #print(hex(localAddress))
    return hex(localAddress)

def initCfgOptions(keys):
    """
        CFG options
    """
    options = {}

    for i in keys:
        options[i] = 'none'

    return options

def dict_raise_on_duplicates(ordered_pairs):
    """
        Reject duplicate keys.
    """
    d = {}
    for k, v in ordered_pairs:
        if k in d:
           raise ValueError("Duplicate key: %r" % (k,))
        else:
           d[k] = v
    return d

def read_json_file(file, attributes, flags, cpus):
    """
        read the json file
    """
    global numImages

    try:
        f = open(file, 'r')

    except FileNotFoundError:
        print('ERROR: file ' + file + ' does not exist!')
        sys.exit(EXIT_WITH_ERROR)

    except:
        print('ERROR opening configuration file')
        sys.exit(EXIT_WITH_ERROR)
    

    try:
        cfg = json.load(f, object_pairs_hook=dict_raise_on_duplicates)

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

    images = []
    for image in cfg:
        #print('IMAGE:')
        #print(image, cfg[image])
        if len(image) == 0:
            print("ERROR: image description can't be empty!","Please correct it in configuration file")
            sys.exit(EXIT_WITH_ERROR)

        # ensure the image descriptor is 8 characters (complete with '\0' otherwise)
        n = 8 - len(image)
        imgDesc = image.ljust(n + len(image), '\0')
        imgOptions = initCfgOptions(attributes)
        userOptions = cfg[image]
        imgOptions['identifier'] = imgDesc[:8]
        #print('Analyze options')
        for opt in userOptions:
            #print(opt, userOptions[opt])
            if imgOptions.get(opt) is None:
                print('ERROR: Invalid key: "' + opt + \
                      '" in configuration file, image: ' + image)
                sys.exit(EXIT_WITH_ERROR)
            else:
                if isinstance(userOptions[opt], list):
                    for s in userOptions[opt]:
                        if s.upper() not in flags:
                            print('ERROR: flag "' + s + '" in image '
                                  + image + ' is not supported')
                            sys.exit(EXIT_WITH_ERROR)

                    imgOptions[opt] = [s.upper() for s in userOptions[opt]]
                else:
                    if opt in ['loadAddress','mramAddress']:
                        validateHexAddress(userOptions[opt], opt, image)
                        if opt == 'loadAddress':
                            if userOptions[opt][0:3] == '0x8':
                                print('ERROR: An MRAM address can not be used in loadAddress attribute')
                                sys.exit(EXIT_WITH_ERROR)

                    if opt == 'cpu_id':
                        if userOptions[opt] not in cpus:
                            print('ERROR: cpu_id "' + userOptions[opt] + '" in image ' + image + ' is not supported')
                            sys.exit(EXIT_WITH_ERROR)

                    imgOptions[opt] = userOptions[opt]

        images.append(imgOptions)

    return images

def compressImage(file):
    """
        compression
    """
    print('Compressing image: ' + file)
    if sys.platform == "linux":
        os.system('chmod +x ./utils/lzf-lnx')
        os.system('./utils/lzf-lnx -cfb 4096 ' + file)
    else:
        os.system('utils\\lzf.exe -cfb 4096 ' + file)

def addContentCertificate(file, binary):
    """
        addContentCertificate
    """
    certFile = "SB" + binary + ".crt"
    certFile = certPath / certFile
    #print("Copying Cont.Cert: " + str(certFile))
    file.write(getBlob(certFile))
    # pad 12 bytes to put the image in a 16-byte aligned address
    file.write(b'\x00' * 12)


def createContentCerts(fwsections, prefix):
    """
        create content certificates
    """
    print('Creating Content Certificates...')
    for sec in fwsections:

        if sec['disabled'] is True:
            continue

        #print(sec)
        #print(imagePath)

        try:
            file = open(imagePath / "images.txt", "w")
        except:
            print(sys.exc_info()[0])
            print("[ERROR] creating template file for Content Certificates")
            sys.exit(EXIT_WITH_ERROR)


        # create the ContentCertificate
        #
        #  swLoadVerifyScheme values in Content Certificates:
        #
        #  SERAMX : 0 - load and verify
        #  XIP (verify in flash) : 1 - (Encryption Not Allowed)
        #  IMAGE  : 2 - verify in memory (LOAD flag set) (Encryption is allowed and optional)
        #

        encrypt = '0'
        # Content Certificate for SERAM needs to have Local Addresses 
        sec['mramAddress'] = globalToLocalAddress(sec['mramAddress'])
        if sec['identifier'].upper()[:5] == 'SERAM':
            configFile = prefix + 'SBContent_lvs_0.cfg'
            mem_load_address = sec['loadAddress']
            # Encryption allowed if load-verify-scheme = 0
            if 'ENCRYPT' in sec['flags']:
                encrypt = '1'

        else:
            if 'LOAD' in sec['flags']:
                # use
                #   load-verify-scheme = 2
                #   aes-ce-id = 2
                configFile = prefix + 'SBContent_lvs_2.cfg'
                mem_load_address = globalToLocalAddress(sec['loadAddress']) 
                # Encryption ONLY allowed if load-verify-scheme = 2 (LOAD flag set)
                if 'ENCRYPT' in sec['flags']:
                    encrypt = '1'
            else:
                # use
                #   load-verify-scheme = 1
                #   aes-ce-id = 0 (no encryption allowed if verify in flash)
                configFile = prefix + 'SBContent_lvs_1.cfg'
                mem_load_address = '0xFFFFFFFF'

        fsize = sec['size']
        if 'COMPRESS' in sec['flags']:
            fsize = sec['uncompressedSize']

        printInfo('Content Certificate1:')
        #print("../build/" + sec['binary'] + " " + str(mem_load_address) + " " + \
        #            str(sec['mramAddress']) + " " + str(fsize) + " " + encrypt)
        file.write("../build/images/" + sec['binary']   + \
                   " " + str(mem_load_address)   + \
                   " " + str(sec['mramAddress']) + \
                   " " + str(fsize)              + \
                   " " + encrypt)
        file.close()

        os.chdir(os.getcwd() + '/utils/')
        cert_sb_content_util.main(["-c", 'cfg/'+configFile, "-l", '../build/logs/SBContent.log'])
        os.chdir(os.getcwd() + '/../')

        # check if file exists (and remove it) before renaming it
        #certFile = "SB" + sec['binary'] + ".crt"
        certFile = "SB" + sec['binary'] + "_" + str(sec['mramAddress']) + ".crt"
        printInfo(certFile)
        if (os.path.exists(certPath / certFile)):
            os.remove(certPath / certFile)

        fileName = prefix + "SBContent.crt"    
        os.rename(certPath / fileName, certPath / certFile)
        # save cert. name in images list
        sec['cert'] = certFile

        # remove ICVSBContent_Cert.txt file (not used)
        textFile = prefix + "SBContent_Cert.txt"
        if (os.path.exists(certPath / textFile)):
            os.remove(certPath / textFile)
    
        # remove images.txt - temporal template file (not used)
        textFile = "images.txt"
        if (os.path.exists(imagePath / textFile)):
            os.remove(imagePath / textFile)


def getObjectType(objType, switcher):
    """
        get object type
    """
    return switcher.get(objType.strip(), 4)

def getCPUID(objCPU, switcher):
    """
        fet CPU ID
    """

    return switcher.get(objCPU, 0xF)


def getObjectFlags(objFlags):
    """
        getObjectFlags
    """
    flagInt = 0
    for f in objFlags:
        if   f == 'COMPRESS':
            flagInt += 0x10
        elif f == 'LOAD':
            flagInt += 0x20
        elif f == 'BOOT':
            flagInt += 0x40
        elif f == 'ENCRYPT':
            flagInt += 0x80
        elif f == 'DEFERRED':
            flagInt += 0x100            

    return flagInt

def getObjectVersion(objVersion):
    """
    """
    if objVersion == 'none':
        return 0

    ver = objVersion.split('.')
    res = 0
    idx = 3
    for n in ver:
        if idx < 1:
            break
        res += (int(n) * (256 ** idx))
        idx -= 1
    return res

def getBlob(file):
    """
        get the Blob
    """
    f = open(file, 'rb')  # TODO: This needs to check for open fail
    blob = f.read()
    f.close()
    return blob

def scanFiles(fileList, action):
    """
        scanFiles
    """
    filesProcesed = 0
    for i in range(len(fileList)):
        files = glob.glob(fileList[i])
        for file in files:
            if action == 'list':
                if i == 0:
                    print('\nThe following files will be deleted:\n')
                print('\t' + file)
            elif action == 'delete':
                os.remove(file)
            filesProcesed += 1

    return filesProcesed

def cleanBuild(fileList):
    """
        cleanBuild
    """

    files = scanFiles(fileList, 'list')

    if files == 0:
        print('\nThere were no files to delete')
        sys.exit()

    print('\nPress Y to confirm, or any other key to exit')
    confirmResponse = input('> ')
    if confirmResponse.upper() == 'Y' or confirmResponse.upper() == 'YES':
        print('Deleting files...')
        scanFiles(fileList, 'delete')
        print('\nThe build workspace was cleaned!', '(' + str(files) + ' files were deleted)')
    else:
        print('The operation was aborted!')

def validateOptions(fwsections):
    # validate options in fwsections - exit if errors
    for sec in fwsections:
        if 'COMPRESS' in sec['flags']:
            # enforce compression is not allowed encryption is used
            if 'ENCRYPT' in sec['flags']:
                print('[ERROR] Compression is not allowed for encrypted images!')
                print('[ERROR] Please correct the configuration: '
                      + str(sec['flags'])
                      + ' in image '
                      + sec['identifier'])
                sys.exit(EXIT_WITH_ERROR)

            # enforce compression is not allowed for XIP use case
            if 'LOAD' not in sec['flags'] and sec['cpu_id'] != 'none':
                print('[ERROR] Compression is not allowed for XIP images!')
                print('[ERROR] Please correct the configuration: '
                      + str(sec['flags'])
                      + ' in image '
                      + sec['identifier'])
                sys.exit(EXIT_WITH_ERROR)

        # if LOAD flag is used, loadAddress should be valid
        if 'LOAD' in sec['flags']:
            if sec['loadAddress'] == 'none':
                print('[ERROR] if LOAD flag is used, a loadAddress must be specified!')
                print('[ERROR] Please correct the configuration in image '
                      + sec['identifier'])
                sys.exit(EXIT_WITH_ERROR)

        # if BOOT flag is used, CPU_ID should be valid
        if 'BOOT' in sec['flags']:
            if sec['cpu_id'] == 'none':
                print('[ERROR] if BOOT flag is used, a cpu_id must be specified!')
                print('[ERROR] Please correct the configuration in image '
                      + sec['identifier'])
                sys.exit(EXIT_WITH_ERROR)
