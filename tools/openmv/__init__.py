# SPDX-License-Identifier: MIT
#
# Copyright (c) 2025 OpenMV, LLC.
#
# OpenMV Protocol Package
#
# This package provides a Python implementation of the OpenMV Protocol
# for communicating with OpenMV cameras.
#
# Main classes:
#     OMVCamera: High-level camera interface with channel operations
#     
# Main exceptions:
#     OMVPException: Base exception for protocol errors
#     OMVPTimeoutException: Timeout during protocol operations
#     OMVPChecksumException: CRC validation failures
#     OMVPSequenceException: Sequence number validation failures

from .camera import OMVCamera
from .exceptions import (
    OMVPException,
    OMVPTimeoutException, 
    OMVPChecksumException,
    OMVPSequenceException
)

__version__ = "2.0.0"

__all__ = [
    'OMVCamera',
    'OMVPException',
    'OMVPTimeoutException',
    'OMVPChecksumException', 
    'OMVPSequenceException'
]
