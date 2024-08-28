# Copyright (c) 2001-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause OR Arm’s non-OSI source license
#

import os
import abc
import re
import struct
from utils.common import global_defines
from utils.common import flags_global_defines
from utils.common import cryptolayer
from utils.common.exceptions import CertCreationError

# *******************************************************************
#  Modification to handle local directories vs Azure Storage
#  when looking for Key1/key2 certs in gen-toc.py
# *******************************************************************
def getKeyCertificatePath():
    return "cert/" # local files

# *******************************************************************
#  Modification to handle DEV vs PROD (Azure) Environments
#  when looking for the alif package name (suffix) in gen-toc.py
#  (to differentiate the name of packages for PROD vs DEV in APP Tools)
# *******************************************************************
def getEnvironmentSuffix():
    return '-dev' # DEV environment suffix

class ArmCertificateHeader(abc.ABC):
    @property
    @abc.abstractmethod
    def magic_number(self):
        pass

    @property
    @abc.abstractmethod
    def cert_version_number(self):
        pass

    @property
    @abc.abstractmethod
    def signed_content_size(self):
        pass

    @property
    @abc.abstractmethod
    def flags(self):
        pass

    @abc.abstractmethod
    def serialize_to_bytes(self):
        pass


class KeyArmCertificateHeader(ArmCertificateHeader):
    CERT_TOKEN = 0x53426b63

    def __init__(self, cert_version, hbk_id, logger=None):
        self._cert_version = (cert_version[0] << 16) + cert_version[1]
        self._hbk_id = hbk_id
        self.logger = logger
        self._signed_content_size = (global_defines.HEADER_SIZE_IN_WORDS
                                     + global_defines.SW_VERSION_OBJ_SIZE_IN_WORDS
                                     + global_defines.PUBKEY_SIZE_WORDS
                                     + global_defines.NP_SIZE_IN_WORDS
                                     + global_defines.HASH_ALGORITHM_SHA256_SIZE_IN_WORDS)
        self._flags = hbk_id

    def magic_number(self):
        return self.CERT_TOKEN

    def cert_version_number(self):
        return self._cert_version

    def signed_content_size(self):
        return self._signed_content_size

    def flags(self):
        return self._flags

    def serialize_to_bytes(self):
        data = (struct.pack('<I', self.CERT_TOKEN)
                + struct.pack('<I', self._cert_version)
                + struct.pack('<I', self._signed_content_size)
                + struct.pack('<I', self._flags))
        return data


class ContentArmCertificateHeader(ArmCertificateHeader):
    CERT_TOKEN = 0x53426363

    def __init__(self, cert_version, code_enc_id, load_verify_scheme, crypto_type, num_of_comps, logger=None):
        self._cert_version = (cert_version[0] << 16) + cert_version[1]
        self.logger = logger
        sizeof_sw_records_field = (global_defines.NONCE_SIZE_IN_WORDS
                                   + num_of_comps
                                   * (global_defines.HASH_ALGORITHM_SHA256_SIZE_IN_WORDS
                                      + global_defines.SW_REC_ADDR32_SIGNED_DATA_SIZE_IN_WORDS))
        self._signed_content_size = (global_defines.HEADER_SIZE_IN_WORDS
                                     + global_defines.SW_VERSION_OBJ_SIZE_IN_WORDS
                                     + global_defines.PUBKEY_SIZE_WORDS
                                     + global_defines.NP_SIZE_IN_WORDS
                                     + sizeof_sw_records_field)

        self._flags = (0xf
                       | (code_enc_id << flags_global_defines.CODE_ENCRYPTION_SUPPORT_BIT_POS)
                       | (load_verify_scheme << flags_global_defines.LOAD_VERIFY_SCHEME_BIT_POS)
                       | (crypto_type << flags_global_defines.CRYPTO_TYPE_BIT_POS)
                       | (num_of_comps << flags_global_defines.NUM_OF_SW_COMPS_BIT_POS))

    def magic_number(self):
        return self.CERT_TOKEN

    def cert_version_number(self):
        return self._cert_version

    def signed_content_size(self):
        return self._signed_content_size

    def flags(self):
        return self._flags

    def serialize_to_bytes(self):
        data = (struct.pack('<I', self.CERT_TOKEN)
                + struct.pack('<I', self._cert_version)
                + struct.pack('<I', self._signed_content_size)
                + struct.pack('<I', self._flags))
        return data


class EnablerDebugArmCertificateHeader(ArmCertificateHeader):
    CERT_TOKEN = 0x5364656E

    def __init__(self, cert_version, rma_mode, hkb_id, lcs, logger=None):
        self.logger = logger
        self._cert_version = (cert_version[0] << 16) + cert_version[1]
        self._signed_content_size = (global_defines.HEADER_SIZE_IN_WORDS
                                     + global_defines.PUBKEY_SIZE_WORDS
                                     + global_defines.NP_SIZE_IN_WORDS
                                     + 8  # size of DCU lock & mask values
                                     + global_defines.HASH_ALGORITHM_SHA256_SIZE_IN_WORDS)

        self._flags = (hkb_id << global_defines.HBK_ID_FLAG_BIT_OFFSET
                       | (lcs << global_defines.LCS_ID_FLAG_BIT_OFFSET)
                       | (rma_mode << global_defines.RMA_CERT_FLAG_BIT_OFFSET))

    def magic_number(self):
        return self.CERT_TOKEN

    def cert_version_number(self):
        return self._cert_version

    def signed_content_size(self):
        return self._signed_content_size

    def flags(self):
        return self._flags

    def serialize_to_bytes(self):
        data = (struct.pack('<I', self.CERT_TOKEN)
                + struct.pack('<I', self._cert_version)
                + struct.pack('<I', self._signed_content_size)
                + struct.pack('<I', self._flags))
        return data


class DeveloperDebugArmCertificateHeader(ArmCertificateHeader):
    CERT_TOKEN = 0x53646465

    def __init__(self, cert_version, logger=None):
        self.logger = logger
        self._cert_version = (cert_version[0] << 16) + cert_version[1]
        self._signed_content_size = (global_defines.HEADER_SIZE_IN_WORDS
                                     + global_defines.PUBKEY_SIZE_WORDS
                                     + global_defines.NP_SIZE_IN_WORDS
                                     + 4  # size of DCU mask values
                                     + global_defines.SOC_ID_SIZE_IN_BYTES // global_defines.BYTES_WITHIN_WORD)

        self._flags = 0

    def magic_number(self):
        return self.CERT_TOKEN

    def cert_version_number(self):
        return self._cert_version

    def signed_content_size(self):
        return self._signed_content_size

    def flags(self):
        return self._flags

    def serialize_to_bytes(self):
        data = (struct.pack('<I', self.CERT_TOKEN)
                + struct.pack('<I', self._cert_version)
                + struct.pack('<I', self._signed_content_size)
                + struct.pack('<I', self._flags))
        return data


class ArmCertificateBody(abc.ABC):
    @property
    @abc.abstractmethod
    def signer_rsa_public_key(self):
        pass

    @abc.abstractmethod
    def serialize_to_bytes(self):
        pass


class KeyArmCertificateBody(ArmCertificateBody):

    def __init__(self, sw_version, signer_keypair_filename, signer_keypair_passphrase_filename,
                 next_cert_pubkey_filename, logger=None):
        self.logger = logger
        self._sw_version = sw_version
        self._signer_rsa_public_key = cryptolayer.Common.get_n_and_np_from_keypair(signer_keypair_filename,
                                                                                   signer_keypair_passphrase_filename,
                                                                                   self.logger)
        self._hashed_pubkey_next_cert = cryptolayer.Common.get_hashed_n_and_np_from_public_key(next_cert_pubkey_filename,
                                                                                               self.logger)

    @property
    def sw_version(self):
        return self._sw_version

    @property
    def signer_rsa_public_key(self):
        return self._signer_rsa_public_key

    @property
    def hashed_pubkey_next_cert(self):
        return self._hashed_pubkey_next_cert

    def serialize_to_bytes(self):
        data = (self._signer_rsa_public_key
                + struct.pack('<I', self._sw_version)
                + self._hashed_pubkey_next_cert)
        return data


class ContentArmCertificateBody(ArmCertificateBody):

    def __init__(self, sw_version, signer_keypair_filename, signer_keypair_passphrase_filename,
                 code_enc_id, images_table, load_verify_scheme, aes_enc_key, crypto_type, logger=None):
        self.logger = logger
        self._sw_version = sw_version
        self._images_table_filename = images_table
        self._load_verify_scheme = load_verify_scheme
        self._enc_key_filename = aes_enc_key
        self._crypto_type = crypto_type
        self._signer_rsa_public_key = cryptolayer.Common.get_n_and_np_from_keypair(signer_keypair_filename,
                                                                                   signer_keypair_passphrase_filename,
                                                                                   self.logger)
        if code_enc_id == global_defines.USE_AES_CE_ID_NONE:
            self._key_nonce = bytes(8)
        else:
            self._key_nonce = os.urandom(8)

        self._sw_records = []
        self._none_signed_info = []
        # process images-table
        with open(self._images_table_filename, "r") as images_file:
            image_list = images_file.readlines()
        self._num_images = len(image_list)
        if not 0 < self._num_images < 17:
            raise CertCreationError("number of images images-table is not supported")
        # analyze image-tables lines
        parsed_images = []
        for line in image_list:
            if not re.match(r'^#', line):   # ignore commented lines
                line_elements = line.split()

                image_filename = line_elements[0]
                mem_load_address = int(line_elements[1], 16)
                if mem_load_address == 0:
                    raise CertCreationError("Invalid load address in images-table: 0 is not allowed as an address")
                mem_load_address_byte_list = list(mem_load_address.to_bytes(global_defines.NUM_OF_BYTES_IN_ADDRESS,
                                                                            byteorder="big"))
                flash_store_address = int(line_elements[2], 16)
                image_max_size = int(line_elements[3], 16)
                is_aes_code_enc_used = int(line_elements[4], 16)

                # perform some basic input checks:
                if (self._load_verify_scheme == global_defines.VERIFY_IMAGE_IN_FLASH
                    and mem_load_address != global_defines.MEM_ADDRESS_UNLOAD_FLAG) \
                        or (self._load_verify_scheme != global_defines.VERIFY_IMAGE_IN_FLASH
                            and mem_load_address == global_defines.MEM_ADDRESS_UNLOAD_FLAG):
                    raise CertCreationError("invalid load address defined in images-table: "
                                            "mem_load_address can be 0xffffffff only in load_verify_scheme==1 case")
                #  Suggested by Georgi to comment out as it was stopping us to use 
                #  IMAGE object type with swLoadAndVerify = 2  - SERGIO 11/12/2020
                #
                #if (self._load_verify_scheme == global_defines.VERIFY_IMAGE_IN_MEM
                #    and flash_store_address != global_defines.MEM_ADDRESS_UNLOAD_FLAG) \
                #        or (self._load_verify_scheme != global_defines.VERIFY_IMAGE_IN_MEM
                #            and flash_store_address == global_defines.MEM_ADDRESS_UNLOAD_FLAG):
                #    raise CertCreationError("invalid flash address defines in images-table: "
                #                            "flash_store_address can be 0xffffffff only in load_verify_scheme==2 case")

                parsed_images.append({"image_filename": image_filename,
                                      "mem_load_address": mem_load_address,
                                      "mem_load_address_byte_list": mem_load_address_byte_list,
                                      "flash_store_address": flash_store_address,
                                      "image_max_size": image_max_size,
                                      "is_aes_code_enc_used": is_aes_code_enc_used})
        # image_hash_and_size = []
        for item in parsed_images:
            if item["is_aes_code_enc_used"] == 1:
                # do Aes and Hash
                if code_enc_id != global_defines.USE_AES_CE_ID_NONE:
                    with open(self._enc_key_filename, "r") as aes_key_file:
                        str_key_data = aes_key_file.read()
                    str_key_data_bytes = str_key_data.split(",")
                    # todo check for exactly 128 bit length key
                    if len(str_key_data_bytes) > global_defines.AES_DECRYPT_KEY_SIZE_IN_BYTES:
                        self.logger.warning("key size in aes-enc-key file is too big - truncating parameter")
                    key_data_list = [int(item, 16) for item in str_key_data_bytes]
                    key_data_list = key_data_list[:global_defines.AES_DECRYPT_KEY_SIZE_IN_BYTES]

                    # combine nonce and address into iv (8 bytes nonce + 4 bytes address + 4 bytes of zeros)
                    aes_iv = self._key_nonce + bytes(item["mem_load_address_byte_list"]) + bytes(4)
                    # encrypt image
                    new_image_filename = item["image_filename"][:-4] + global_defines.SW_COMP_FILE_NAME_POSTFIX
                    hash_of_image, actual_image_size = cryptolayer.Common.encrypt_file_with_aes_ctr(item["image_filename"],
                                                                                                    new_image_filename,
                                                                                                    key_data_list,
                                                                                                    aes_iv,
                                                                                                    self._crypto_type,
                                                                                                    self.logger)
                else:
                    raise CertCreationError("invalid aes-ce-id for image encryption flag in image of images-table - "
                                            "must have aes-enc-key for images to be encrypted")
            else:
                # do only hash
                with open(item["image_filename"], "rb") as input_file:
                    plaintext = input_file.read()
                actual_image_size = len(plaintext)
                hash_of_image = cryptolayer.HashCrypto.calculate_sha256_hash(plaintext)
            # image_hash_and_size.append([hash_of_image, actual_image_size])
            self._sw_records.append([hash_of_image, item["mem_load_address"], item["image_max_size"], item["is_aes_code_enc_used"]])
            self._none_signed_info.append([item["flash_store_address"], actual_image_size])

    @property
    def num_images(self):
        return self._num_images

    @property
    def sw_version(self):
        return self._sw_version

    @property
    def signer_rsa_public_key(self):
        return self._signer_rsa_public_key

    @property
    def key_nonce(self):
        return self._key_nonce

    def serialize_to_bytes(self):
        data = (self._signer_rsa_public_key
                + struct.pack('<I', self._sw_version)
                + self._key_nonce
                + b''.join([struct.pack('<I', item) if type(item) is not bytes else item for sublist in self._sw_records for item in sublist])
                )
        return data

    @property
    def x509_body_extension_data(self):
        data = (struct.pack('<I', self._sw_version)
                + self._key_nonce
                + b''.join(
                    [struct.pack('<I', item) if type(item) is not bytes else item for sublist in self._sw_records for
                     item in sublist])
                )
        return data

    @property
    def none_signed_info_serialized(self):
        return b''.join([struct.pack('<I', item) for sublist in self._none_signed_info for item in sublist])


class EnablerDebugArmCertificateBody(ArmCertificateBody):

    def __init__(self, signer_keypair_filename, signer_keypair_passphrase_filename,
                 debug_mask_values, debug_lock_values, next_cert_pubkey_filename, logger=None):
        self.logger = logger
        self._signer_rsa_public_key = cryptolayer.Common.get_n_and_np_from_keypair(signer_keypair_filename,
                                                                                   signer_keypair_passphrase_filename,
                                                                                   self.logger)
        self._debug_mask_values = debug_mask_values
        self._debug_lock_values = debug_lock_values
        self._hashed_pubkey_next_cert = cryptolayer.Common.get_hashed_n_and_np_from_public_key(next_cert_pubkey_filename,
                                                                                               self.logger)

    @property
    def signer_rsa_public_key(self):
        return self._signer_rsa_public_key

    @property
    def hashed_pubkey_next_cert(self):
        return self._hashed_pubkey_next_cert

    def serialize_to_bytes(self):
        data = (self._signer_rsa_public_key
                + b''.join([struct.pack('<I', i) for i in self._debug_mask_values])
                + b''.join([struct.pack('<I', i) for i in self._debug_lock_values])
                + self._hashed_pubkey_next_cert)
        return data


class DeveloperDebugArmCertificateBody(ArmCertificateBody):

    def __init__(self, signer_keypair_filename, signer_keypair_passphrase_filename,
                 debug_mask_values, soc_id_filename, logger=None):
        self.logger = logger
        self._soc_id_filename = soc_id_filename
        self._signer_rsa_public_key = cryptolayer.Common.get_n_and_np_from_keypair(signer_keypair_filename,
                                                                                   signer_keypair_passphrase_filename,
                                                                                   self.logger)
        self._debug_mask_values = debug_mask_values
        with open(self._soc_id_filename, "rb") as soc_id_holder_file:
            self._soc_id = soc_id_holder_file.read()
            if len(self._soc_id) != global_defines.SOC_ID_SIZE_IN_BYTES:
                raise ValueError("Invalid SoC_ID size in input file " + self._soc_id_filename)

    @property
    def signer_rsa_public_key(self):
        return self._signer_rsa_public_key

    @property
    def soc_id(self):
        return self._soc_id

    def serialize_to_bytes(self):
        data = (self._signer_rsa_public_key
                + b''.join([struct.pack('<I', i) for i in self._debug_mask_values])
                + self._soc_id)
        return data


class ArmCertificateSignature:

    def __init__(self, cert_data_to_sign, signer_keypair_filename, signer_keypair_passphrase_filename, logger=None):
        self.logger = logger
        self._signature = cryptolayer.Common.rsa_sign(cert_data_to_sign,
                                                      signer_keypair_filename,
                                                      signer_keypair_passphrase_filename,
                                                      self.logger)
        self._signature = self._signature[::-1]  # for some reason the original script reverses the signature

    def serialize_to_bytes(self):
        return self._signature


class ArmCertificate(abc.ABC):
    @property
    @abc.abstractmethod
    def header(self):
        pass

    @property
    @abc.abstractmethod
    def body(self):
        pass

    @property
    @abc.abstractmethod
    def signature(self):
        pass

    @property
    @abc.abstractmethod
    def certificate_data(self):
        pass


class KeyArmCertificate(ArmCertificate):

    def __init__(self, certificate_config, cert_version, logger=None):
        self.logger = logger
        self._cert_config = certificate_config
        self._cert_version = cert_version
        self._cert_header = KeyArmCertificateHeader(self._cert_version,
                                                    self._cert_config.hbk_id,
                                                    self.logger)
        self._cert_body = KeyArmCertificateBody(self._cert_config.nvcounter_val,
                                                self._cert_config.cert_keypair,
                                                self._cert_config.cert_keypair_pwd,
                                                self._cert_config.next_cert_pubkey,
                                                self.logger)
        self._certificate_data = self._cert_header.serialize_to_bytes() + self._cert_body.serialize_to_bytes()
        self._cert_signature = ArmCertificateSignature(self._certificate_data,
                                                       self._cert_config.cert_keypair,
                                                       self._cert_config.cert_keypair_pwd,
                                                       self.logger)
        self._certificate_data += self._cert_signature.serialize_to_bytes()

    @property
    def header(self):
        return self._cert_header

    @property
    def body(self):
        return self._cert_body

    @property
    def signature(self):
        return self._cert_signature

    @property
    def certificate_data(self):
        return self._certificate_data


class ContentArmCertificate(ArmCertificate):

    def __init__(self, certificate_config, cert_version, logger=None):
        self.logger = logger
        self._cert_config = certificate_config
        self._cert_version = cert_version
        self._num_of_comps = None
        self._cert_body = ContentArmCertificateBody(self._cert_config.nvcounter_val,
                                                    self._cert_config.cert_keypair,
                                                    self._cert_config.cert_keypair_pwd,
                                                    self._cert_config.aes_ce_id,
                                                    self._cert_config.images_table,
                                                    self._cert_config.load_verify_scheme,
                                                    self._cert_config.aes_enc_key,
                                                    self._cert_config.crypto_type,
                                                    self.logger)
        self._num_of_comps = self._cert_body.num_images
        self._cert_header = ContentArmCertificateHeader(self._cert_version,
                                                        self._cert_config.aes_ce_id,
                                                        self._cert_config.load_verify_scheme,
                                                        self._cert_config.crypto_type,
                                                        self._num_of_comps,
                                                        self.logger)

        self._certificate_data = self._cert_header.serialize_to_bytes() + self._cert_body.serialize_to_bytes()
        self._cert_signature = ArmCertificateSignature(self._certificate_data,
                                                       self._cert_config.cert_keypair,
                                                       self._cert_config.cert_keypair_pwd,
                                                       self.logger)
        self._certificate_data += self._cert_signature.serialize_to_bytes()
        self._certificate_data += self._cert_body.none_signed_info_serialized

    @property
    def header(self):
        return self._cert_header

    @property
    def body(self):
        return self._cert_body

    @property
    def signature(self):
        return self._cert_signature

    @property
    def certificate_data(self):
        return self._certificate_data


class EnablerDebugArmCertificate(ArmCertificate):

    def __init__(self, certificate_config, cert_version, logger=None):
        self.logger = logger
        self._cert_config = certificate_config
        self._cert_version = cert_version
        self._cert_header = EnablerDebugArmCertificateHeader(cert_version,
                                                             self._cert_config.rma_mode,
                                                             self._cert_config.hbk_id,
                                                             self._cert_config.lcs,
                                                             self.logger)
        self._cert_body = EnablerDebugArmCertificateBody(self._cert_config.cert_keypair,
                                                         self._cert_config.cert_keypair_pwd,
                                                         self._cert_config.debug_masks,
                                                         self._cert_config.debug_locks,
                                                         self._cert_config.next_cert_pubkey,
                                                         self.logger)
        self._certificate_data = self._cert_header.serialize_to_bytes() + self._cert_body.serialize_to_bytes()
        self._cert_signature = ArmCertificateSignature(self._certificate_data,
                                                       self._cert_config.cert_keypair,
                                                       self._cert_config.cert_keypair_pwd,
                                                       self.logger)
        self._certificate_data += self._cert_signature.serialize_to_bytes()

    @property
    def header(self):
        return self._cert_header

    @property
    def body(self):
        return self._cert_body

    @property
    def signature(self):
        return self._cert_signature

    @property
    def certificate_data(self):
        return self._certificate_data


class DeveloperDebugArmCertificate(ArmCertificate):

    def __init__(self, certificate_config, cert_version, logger=None):
        self.logger = logger
        self._cert_config = certificate_config
        self._cert_version = cert_version
        self._cert_header = DeveloperDebugArmCertificateHeader(cert_version, self.logger)
        self._cert_body = DeveloperDebugArmCertificateBody(self._cert_config.cert_keypair,
                                                           self._cert_config.cert_keypair_pwd,
                                                           self._cert_config.debug_masks,
                                                           self._cert_config.soc_id,
                                                           self.logger)
        self._certificate_data = self._cert_header.serialize_to_bytes() + self._cert_body.serialize_to_bytes()
        self._cert_signature = ArmCertificateSignature(self._certificate_data,
                                                       self._cert_config.cert_keypair,
                                                       self._cert_config.cert_keypair_pwd,
                                                       self.logger)
        self._certificate_data += self._cert_signature.serialize_to_bytes()

    @property
    def header(self):
        return self._cert_header

    @property
    def body(self):
        return self._cert_body

    @property
    def signature(self):
        return self._cert_signature

    @property
    def certificate_data(self):
        return self._certificate_data
