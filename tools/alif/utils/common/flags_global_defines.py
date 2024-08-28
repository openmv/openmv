####################################################################
#
# Copyright (c) 2001-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause OR Arm’s non-OSI source license
#


# Definition for RSA Np or H usage
# ================================
# This flag is set to 1 for Np usage in case H should be used set it to 0
RSA_ALG_USE_Np = 1

# This definition is for the secondary HASH of public key calculation. The calculation is done according to the 
# calculation of HASH of public key (as saved in the OTP) and is set according to platform type  
OTP_HASH_CALC_ON_N_AND_NP = 0
OTP_HASH_CALC_ON_N = 1
OTP_HASH_ON_E_AND_N = 2 

# This definition should be set according to platform and project 
SECONDARY_KEY_HASH_CALC = OTP_HASH_CALC_ON_N_AND_NP

# Definitions for specific projects, should be set to 0 if no additional data is required
SPECIAL_ADDITIONAL_DATA_USED = 1

# Content certificate flag word definition
# =========================================
CODE_ENCRYPTION_SUPPORT_BIT_POS = 4

LOAD_VERIFY_SCHEME_BIT_POS = 8

CRYPTO_TYPE_BIT_POS = 12

# Bit positions of signature offset and number of s/w components
# ================================================================
NUM_OF_SW_COMPS_BIT_POS = 16

SIGNATURE_OFFSET_BIT_POS = 0


