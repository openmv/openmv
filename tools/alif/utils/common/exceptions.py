# Copyright (c) 2001-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause OR Arm’s non-OSI source license
#


class RsaKeyLoadingError(Exception):
    """Raised when the function cannot load an Rsa key from a PEM file"""
    pass


class ConfigParsingError(Exception):
    """Raised when trying to load a misconfigured CFG file"""
    pass


class CertCreationError(Exception):
    """Raised when certificate creation fails"""
    pass
