"""
OpenMV Protocol V2 Exceptions

This module defines all the custom exceptions used in the OpenMV Protocol V2
implementation for proper error handling and debugging.
"""

import traceback

class OMVPException(Exception):
    """Base exception for OpenMV protocol errors"""
    def __init__(self, message):
        super().__init__(message)
        self.traceback = traceback.format_exc()

class OMVPTimeoutException(OMVPException):
    """Raised when a protocol operation times out"""
    def __init__(self, message):
        super().__init__(message)

class OMVPChecksumException(OMVPException):
    """Raised when CRC validation fails"""
    def __init__(self, message):
        super().__init__(message)

class OMVPSequenceException(OMVPException):
    """Raised when sequence number validation fails"""
    def __init__(self, message):
        super().__init__(message)