# Copyright (c) 2001-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause OR Arm’s non-OSI source license
#

BITS_WITHIN_WORD = 32

# Definition of number of bits in a byte.
CC_BITS_IN_BYTE = 8
# Definition of number of bits in a 32-bits word.
CC_BITS_IN_32BIT_WORD = 32
# The size of the PKA engine word.
CC_PKA_WORD_SIZE_IN_BITS = 64
# The maximal supported size of modulus in bits.
C_SRP_MAX_MODULUS_SIZE_IN_BITS = 3072
# The maximal supported size of modulus in RSA in bits.
CC_RSA_MAX_VALID_KEY_SIZE_VALUE_IN_BITS = 4096
# The maximal supported size of key-generation in RSA in bits.
CC_RSA_MAX_KEY_GENERATION_HW_SIZE_BITS = 3072
# The maximal supported size of modulus in RSA in words.
CC_RSA_MAX_VALID_KEY_SIZE_VALUE_IN_WORDS = CC_RSA_MAX_VALID_KEY_SIZE_VALUE_IN_BITS / CC_BITS_IN_32BIT_WORD

# The size of the RSA public modulus key of the Secure Boot or Secure Debug certificate in bits.
SB_CERT_RSA_KEY_SIZE_IN_BITS = 3072
# The size of the RSA public modulus key of the Secure Boot or Secure Debug certificate in bytes.
SB_CERT_RSA_KEY_SIZE_IN_BYTES = int(SB_CERT_RSA_KEY_SIZE_IN_BITS / CC_BITS_IN_BYTE)
# The size of the RSA public modulus key of the Secure Boot or Secure Debug certificate in words.
SB_CERT_RSA_KEY_SIZE_IN_WORDS = SB_CERT_RSA_KEY_SIZE_IN_BITS / CC_BITS_IN_32BIT_WORD

# The maximal count of extra bits in PKA operations.
PKA_EXTRA_BITS = 8
# The number of memory registers in PKA operations.
PKA_MAX_COUNT_OF_PHYS_MEM_REGS = 32

# Global defines from common_rsa_keypair
RSA_PRIVATE_KEY_SIZE = SB_CERT_RSA_KEY_SIZE_IN_BITS
NP_SIZE_IN_BYTES = 20
NEW_PKA_WORD_SIZE_BITS = CC_PKA_WORD_SIZE_IN_BITS
NEW_PAK_ADDITIONAL_BITS = 8
SNP = SB_CERT_RSA_KEY_SIZE_IN_BITS + NEW_PKA_WORD_SIZE_BITS + NEW_PAK_ADDITIONAL_BITS - 1

HASH_SHA256_DIGEST_SIZE_IN_BYTES = 32

# Definitions for sw version legal values -
#########################################
SW_REVOCATION_MAX_NUM_OF_BITS_HBK0 = 64
SW_REVOCATION_MAX_NUM_OF_BITS_HBK1 = 96
SW_REVOCATION_MAX_NUM_OF_BITS_HBK2 = 160

RSA_SALT_LEN = 32

BYTES_WITHIN_WORD = 4
PUBKEY_SIZE_BYTES = 384 #256 aligned with SB_CERT_RSA_KEY_SIZE_IN_BITS defined in cc_pka_hw_plat_defs.h
PUBKEY_SIZE_WORDS = (PUBKEY_SIZE_BYTES//BYTES_WITHIN_WORD)
NP_SIZE_IN_WORDS = (NP_SIZE_IN_BYTES//BYTES_WITHIN_WORD)
SHA_256_HASH_SIZE_IN_BYTES = 32
RSA_SIGNATURE_SIZE_BYTES = 384 #256 aligned with SB_CERT_RSA_KEY_SIZE_IN_BITS defined in cc_pka_hw_plat_defs.h

CC_MNG_CHIP_MANUFACTURE_LCS = 0x0
CC_MNG_DEVICE_MANUFACTURE_LCS = 0x1
CC_MNG_SECURE_LCS = 0x5
CC_MNG_RMA_LCS = 0x7

# E value, in a string mode. for the HASh calculation
E_VALUE_REVERSED = "00010001"
# Memory unload flag
MEM_ADDRESS_UNLOAD_FLAG = 0xFFFFFFFF

RSA_SIGNATURE_SIZE_IN_BYTES = 384
RSA_SIGNATURE_SIZE_IN_DOUBLE_BYTES = 768
RSA_SIGNATURE_SIZE_IN_WORDS = 96
# HASH size in SHA256 in bytes
HASH_ALGORITHM_SHA256_SIZE_IN_WORDS = (SHA_256_HASH_SIZE_IN_BYTES//BYTES_WITHIN_WORD)

# H size
RSA_H_SIZE_IN_BYTES = RSA_SIGNATURE_SIZE_IN_BYTES
RSA_H_SIZE_IN_WORDS = (RSA_SIGNATURE_SIZE_IN_BYTES//BYTES_WITHIN_WORD)

# Size of SW versions
SW_VERSION_OBJ_SIZE_IN_WORDS = 1

# header size in bytes
HEADER_SIZE_IN_WORDS = 4
HEADER_SIZE_IN_BYTES = (HEADER_SIZE_IN_WORDS * BYTES_WITHIN_WORD)
#size number of bytes in address
NUM_OF_BYTES_IN_ADDRESS = 4

SOC_ID_SIZE_IN_BYTES = 32

#enabler certificate flag offset
HBK_ID_FLAG_BIT_OFFSET = 0
LCS_ID_FLAG_BIT_OFFSET = 4
RMA_CERT_FLAG_BIT_OFFSET = 8


# HASH output representation
HASH_BINARY_REPRESENTATION = 1
HASH_HEX_REPRESENTATION = 2


# certificate output file prefix
Cert_FileName = "Cert"
# certificate output file suffix
Cert_FileExtBin = ".bin"
Cert_FileExtTxt = ".txt"

# definitions for code encryption
AES_IV_SIZE_IN_BYTES = 16
AES_DECRYPT_KEY_SIZE_IN_BYTES = 16
SW_COMP_FILE_NAME_POSTFIX = "_enc.bin"

NONCE_SIZE_IN_WORDS = 2
MAX_NUM_OF_IMAGES = 16

SW_REC_ADDR32_SIGNED_DATA_SIZE_IN_WORDS = 3

USE_AES_CE_ID_NONE = 0
USE_AES_CE_ID_KCEICV = 1
USE_AES_CE_ID_KCE = 2

LOAD_AND_VERIFY_IMAGE = 0
VERIFY_IMAGE_IN_FLASH = 1
VERIFY_IMAGE_IN_MEM = 2
LOADING_ONLY_IMAGE = 3
