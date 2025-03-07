#!/usr/bin/env python3
import os
import sys

EXIT_WITH_ERROR = 1

def validateNumber(number, alignment):
    #validate a number (to be used as an Address, Size or Pattern)
    # 1. check number is valid (positive integer <= 32bits)
    # 2. if address is in decimal, check for negative values
    # 3. check for 16-byte alignment if requested
    if number[:2].lower() == '0x':
        base = 16
    else:
        base = 10

    try:
        val = int(number,base)
        if val < 0:
            raise Exception
                
    except:
        print('Error in parameter ' + number + ' - Please check')
        sys.exit(EXIT_WITH_ERROR)

    if alignment == True:
        if val%16:
            print('Error: parameter ' + number + ' must be multiple of 16')
            print('Please correct and retry')
            sys.exit(EXIT_WITH_ERROR)

    if len(hex(val)) > 10:
        print("Error: parameter " + number + " can't be larger than a 32-bit value")
        print('Please correct and retry')
        sys.exit(EXIT_WITH_ERROR)       
    
    return val

def validateItems(items, pad):
    # we get a list of dictionaries
    for item in items:
        for key in item:
            if key == 'file':
                # validate file
                # get file size to check existance
                try:
                    # mram_burner runs in bin/ folder so all references would
                    # have '../' to land into tools root directory and describe the path from there...
                    # GUI will solve all these limitations.
                    fPath = item[key][3:]
                    fSize = os.path.getsize(fPath)
                    if (fSize % 16) != 0:
                        print("File size = %d" % fSize)
                        print('!!!WARNING: the SIZE of ' + fPath + ' is NOT multiple of 16 bytes as required by MRAM')
                        if pad:
                            padlen = (16 - (fSize % 16))
                            print('Since the -p option was invoked, the binary will be padded with ' + str(padlen) + ' bytes')
                            with open(fPath, 'ab') as f:
                                f.write(('\0' * padlen).encode('utf8'))
                            
                        else:
                            print('Please correct the image or use the -p option to pad the binary')
                            os._exit(1)

                except:
                    print(sys.exc_info()[0])
                    print("Error: there is a problem with the image " + item[key])
                    print('Please check it exists and retry')
                    sys.exit(EXIT_WITH_ERROR)

            elif key == 'address':
                # validate address is valid
                validateNumber(item[key], False)

            elif key == 'addressX16':
                # validate address is valid and 16-byte aligned
                validateNumber(item[key], True)
             
            elif key == 'sizeX16':
                #validate size
                validateNumber(item[key], True)

            elif key == 'pattern':
                # validate pattern
                validateNumber(item[key], False)
 

def convertToHex(item):
    # item could be a file (returned as it is) or a number
    # if decimal number, it will be converted to hex and returned
    # if hexadecimal number, it will be returned as it is
    if item[:2].lower() != '0x':
        try:
            return hex(int(item, 10))    

        except:
            return item

    return item


def validateArgList(action, argList, pad=False):
    items = []
    if action == 'Burning: ':
        # we should have a pair of parameters (file and MRAM_Address)
        pairList = argList.split(' ')
        if len(pairList)%2:
            print('Error in parameters: check the help using -h option')
            sys.exit(EXIT_WITH_ERROR)

        tmp = {}
        for i in range(0, len(pairList), 2):
            tmp["file"] = pairList[i]
            tmp["address"] = pairList[i+1]
            items.append(tmp.copy())

    elif action == 'Erasing: ':
        # we should have a pair of parameters (file and MRAM_Address)
        params = argList.split(' ')
        if len(params)<3 or len(params)>4:
            print('Error in parameters: check the help using -h option')
            sys.exit(EXIT_WITH_ERROR)

        tmp = {}
        tmp["addressX16"] = params[1]
        tmp["sizeX16"] = params[2]
        if len(params)==4:
            tmp["pattern"] = params[3]

        items.append(tmp)

    else:
        # we can't validate arguments as we don't recognize the action
        print('Unknown option in argument validation: ' + action)
        sys.exit(EXIT_WITH_ERROR)
    
    # validate the list of items to validate
    validateItems(items, pad)
    
    newArgList = ''
    for item in argList.split(' '):
        newArgList += convertToHex(item) + ' '
    
    return newArgList
    
