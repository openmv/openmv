# -*- coding: utf-8 -*-
# SPDX-FileCopyrightText: 2011 Sybren A. Stüvel <sybren@stuvel.eu>
#
# SPDX-License-Identifier: Apache-2.0

"""
RSA module
====================================================

Module for calculating large primes, and RSA encryption, decryption, signing
and verification. Includes generating public and private keys.

**WARNING:** This implementation does not use compression of the cleartext input to
prevent repetitions, or other common security improvements. Use with care.

"""
