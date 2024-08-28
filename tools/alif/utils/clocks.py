""" clocks """
import json
import struct


# pylint: disable=too-few-public-methods
# pylint: disable=too-many-instance-attributes
# pylint: disable=invalid-name
# pylint: disable=attribute-defined-outside-init

def processChangeSets(changeSets):
    print('Processing ChangeSets')
    data = bytearray()   
    for set in changeSets:
        print(set)
        data += bytes(int(set['address'], 16).to_bytes(4, 'little'))
        data += bytes(int(set['mask'], 16).to_bytes(4, 'little'))
        data += bytes(int(set['value'], 16).to_bytes(4, 'little'))

    #print(data)
    #print(type(data))
    #print(len(data))
    return data

def json_to_bin(jsn):
    """ Convert a JSON object into binary """
    data = bytearray()
    for sec in jsn:
        if sec == 'change_sets':
            data = processChangeSets(jsn[sec])


    return data

def main():
    """ main """
    json_to_bin()
    return 0

if __name__ == "__main__":
    main()
