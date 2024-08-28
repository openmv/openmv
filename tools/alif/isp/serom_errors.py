#!/usr/bin/python3

"""
   ISP

   - SEROM Error codes

   __author__ onyettr
"""
# pylint: disable=unused-argument, invalid-name
SEROM_STATUS_SUCCESS                          = 0x0

# Crypto errors
SEROM_BSV_INIT_FAIL                           = 0x1
SEROM_BSV_LCS_GET_AND_INIT_FAIL               = 0x2
SEROM_BSV_LCS_GET_FAIL                        = 0x2
SEROM_BSV_SEC_MODE_SET_FAIL                   = 0x3
SEROM_BSV_PRIV_MODE_SET_FAIL                  = 0x4
SEROM_BSV_CORE_CLK_GATING_ENABLE_FAIL         = 0x5

# MRAM errors
SEROM_MRAM_INITIALIZATION_FAILURE             = 0x6
SEROM_MRAM_INITIALIZATION_TIMEOUT             = 0x7
SEROM_MRAM_WRITE_FAILURE                      = 0x8

# ATOC errors
SEROM_ATOC_EXT_HDR_OFFSET_ZERO                = 0x9
SEROM_ATOC_EXT_HDR_OFFSET_TOO_LARGE           = 0xA
SEROM_ATOC_OBJECT_OFFSET_ZERO                 = 0xB
SEROM_ATOC_OBJECT_OFFSET_MISALIGNED           = 0xC
SEROM_ATOC_OBJECT_OFFSET_TOO_LARGE            = 0xD
SEROM_ATOC_OBJECT_OFFSET_TOO_SMALL            = 0xE
SEROM_ATOC_EXT_HDR_OFFSET_MISALIGNED          = 0xF
SEROM_ATOC_HEADER_OFFSET_INVALID              = 0x10
SEROM_ATOC_HEADER_CRC32_ERROR                 = 0x11
SEROM_ATOC_HEADER_STRING_INVALID              = 0x12
SEROM_ATOC_NUM_TOC_ENTRIES_INVALID            = 0x13

# Certificate errors
SEROM_CONTENT_CERTIFICATE_NULL                = 0x14
SEROM_CERTIFICATE_NULL                        = 0x15
SEROM_CERTIFICATE_CHAIN_INVALID               = 0x16
SEROM_INVALID_OEM_ROT                         = 0x17
SEROM_CERTIFICATE_ERROR_BASE                  = 0x18
SEROM_CERTIFICATE_1_ERROR                     = 0x19
SEROM_CERTIFICATE_2_ERROR                     = 0x1A
SEROM_CERTIFICATE_3_ERROR                     = 0x1B

# BOOT errors
SEROM_BOOT_CODE_LOAD_ADDR_INVALID             = 0x1C
SEROM_BOOT_VERIFY_IN_MEMORY_CASE_INVALID      = 0x1D
SEROM_BOOT_ZERO_IMAGE_LENGTH_INVALID          = 0x1E
SEROM_BOOT_ENCRYPTED_IMAGE_INVALID            = 0x1F
SEROM_BOOT_VERIFY_IN_FLASH_CASE_INVALID       = 0x20
SEROM_BOOT_IMAGE_LENGTH_TOO_LARGE             = 0x21
SEROM_BOOT_RAW_IMAGE_LOADING_NOT_ALLOWED      = 0x22
SEROM_BOOT_SERAM_JUMP_RETURN_ERROR            = 0x23
SEROM_BOOT_FAILED                             = 0x24
SEROM_BOOT_JUMP_ADDRESS_NOT_VALID             = 0x25

# Bank selection errors
SEROM_BOTH_BANKS_INVALID                      = 0x26

SEROM_ATOC_EXT_HDR_OFFSET_TOO_SMALL           = 0x27
SEROM_BOOT_END_OF_MAIN_ERROR                  = 0x28
SEROM_INVALID_NULL_PTR                        = 0x29
SEROM_INVALID_TOC_OFFSET                      = 0x30
