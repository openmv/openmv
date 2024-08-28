#!/usr/bin/env python3
#pylint: disable=invalid-name,superfluous-parens,anomalous-unicode-escape-in-string

"""
 __author__ = ""
 __copyright__ =Copyright 2021, ALIF Semiconductor"
"""

verboseMode = False

def verboseModeSet(flag):
    global verboseMode
    verboseMode = flag

def verboseModeGet():
    return verboseMode
    
def printInfo(*args):
    if verboseMode:
        msj = ''
        for item in args:
            if type(item).__name__ == 'str':
                msj += item
            else:
                msj += str(item)
            msj += ' '
        print('[INFO] ' + msj)
    