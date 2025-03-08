# Copyright (c) 2001-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause OR Arm’s non-OSI source license
#

import getpass

import cryptography.exceptions
from cryptography.hazmat.primitives.serialization import load_pem_public_key, load_pem_private_key
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import padding as sym_padding
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import hashes, serialization, cmac
from cryptography.hazmat.primitives.asymmetric import padding, utils
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes, aead

from utils.common import global_defines
from utils.common.exceptions import RsaKeyLoadingError


class RsaCrypto:
    @staticmethod
    def generate_rsa_pem_key(pem_filename, password_filename=None, logger=None):
        if password_filename is not None:
            try:
                with open(password_filename, "r") as pwd_file:
                    passphrase = pwd_file.read()
                    passphrase = passphrase.strip().encode("utf-8")
            except OSError as e:
                if logger is not None:
                    logger.warning("Could not open passphrase file %s" % password_filename)
                raise ValueError("Could not open passphrase file: %s" % e)
        else:
            passphrase = getpass.getpass(prompt="Enter passphrase for creating RSA key <<" + pem_filename + ">>: ").encode("utf-8")
        private_key = rsa.generate_private_key(public_exponent=65537, key_size=3072, backend=default_backend())
        serialized_pem_key = private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.BestAvailableEncryption(passphrase)
        )
        with open(pem_filename, "wb") as output_file:
            output_file.write(serialized_pem_key)
        if logger is not None:
            logger.info("Private key written to: %s" % pem_filename)

        return private_key

    @staticmethod
    def extract_public_rsa_pem_key(private_key, public_pem_filename, logger=None):
        public_key = private_key.public_key()
        serialized_public_key = public_key.public_bytes(encoding=serialization.Encoding.PEM,
                                                        format=serialization.PublicFormat.SubjectPublicKeyInfo)

        with open(public_pem_filename, "wb") as output_file:
            output_file.write(serialized_public_key)
        if logger is not None:
            logger.info("Public key written to: %s" % public_pem_filename)

    @staticmethod
    def load_rsa_pem_key(pem_filename, is_key_private, keypair_passphrase_filename=None, logger=None):
        """
        Loads RSA private/public key encoded in PEM format

        :param pem_filename: input file in PEM format
        :param is_key_private: True for private keys, False for public keys
        :param keypair_passphrase_filename: filename containing passphrase for private key, left out in for public keys
        :param logger: initialized logging.logger instance
        :return: instance of RSAPublicKey/RSAPrivateKey depending on is_key_private

        Raises RsaKeyLoadingError in case of failure.
        """
        # opening PEM file
        try:
            with open(pem_filename) as key_file:
                pem_data = key_file.read()
        except OSError as e:
            if logger is not None:
                logger.warning("Could not open RSA key file %s" % pem_filename)
            raise RsaKeyLoadingError("Could not open RSA key file: %s" % e)
        # loading passphrase for private keys
        if is_key_private is True:
            if len(keypair_passphrase_filename) > 0:
                try:
                    with open(keypair_passphrase_filename, "r") as pwd_file:
                        passphrase = pwd_file.read()
                        passphrase = passphrase.strip().encode("utf-8")
                except OSError as e:
                    if logger is not None:
                        logger.warning("Could not open keypair passphrase file %s" % keypair_passphrase_filename)
                    raise RsaKeyLoadingError("Could not open keypair passphrase file: %s" % e)
            else:
                passphrase = getpass.getpass(prompt="Enter passphrase for RSA private key: ").encode("utf-8")
        # loading public/private key
        try:
            if is_key_private is True:  # private key case
                key = load_pem_private_key(bytes(pem_data, encoding="utf-8"), passphrase, backend=default_backend())
                # check return value
                if isinstance(key, rsa.RSAPrivateKey) is False:
                    if logger is not None:
                        logger.warning("Could not parse key as an RSA Private key from file %s" % pem_filename)
                    raise RsaKeyLoadingError("Could not parse PEM file as an RSAPrivateKey")
            else:  # public key case
                key = load_pem_public_key(bytes(pem_data, encoding="utf-8"), backend=default_backend())
                # check return value
                if isinstance(key, rsa.RSAPublicKey) is False:
                    if logger is not None:
                        logger.warning("Could not parse key as an RSA Public key from file %s" % pem_filename)
                    raise RsaKeyLoadingError("Could not parse PEM file as an RSAPublicKey")
        except ValueError or cryptography.exceptions.UnsupportedAlgorithm as e:
            # look for internal errors
            if logger is not None:
                logger.warning("Could not read RSA key from file %s" % pem_filename)
            raise RsaKeyLoadingError("Could load RSA key: %s" % e)
        else:
            return key

    @staticmethod
    def get_n_from_rsa_pem_private_key(rsa_key):
        """
        Get modulus (N) from RSA Private key

        :param rsa_key: instance of rsa.RSAPrivateKey
        :return: modulus (N) of the given key object
        """
        return rsa_key.public_key().public_numbers().n

    @staticmethod
    def get_n_from_rsa_pem_public_key(rsa_key):
        """
        Get modulus (N) from RSA Public key

        :param rsa_key: instance of rsa.RSAPublicKey
        :return: modulus (N) of the given key object
        """
        return rsa_key.public_numbers().n

    @staticmethod
    def calculate_np_from_n(n_as_modulus, logger=None):
        if not isinstance(n_as_modulus, int):
            if logger is not None:
                logger.warning("input parameter n_as_modulus expected to be integer")
            raise TypeError("input parameter n_as_modulus expected to be integer")
        param_np = (2 ** global_defines.SNP) // n_as_modulus
        return param_np

    @staticmethod
    def concatenate_n_and_np(n_as_modulus, param_np, logger=None):
        if not isinstance(n_as_modulus, int):
            if logger is not None:
                logger.warning("input parameter n_as_modulus expected to be integer")
            raise TypeError("input parameter n_as_modulus expected to be integer")
        if not isinstance(param_np, int):
            if logger is not None:
                logger.warning("input parameter param_np expected to be integer")
            raise TypeError("input parameter param_np expected to be integer")
        n_in_bytes = n_as_modulus.to_bytes(global_defines.SB_CERT_RSA_KEY_SIZE_IN_BYTES, 'big')
        np_in_bytes = param_np.to_bytes(global_defines.NP_SIZE_IN_BYTES, 'big')
        n_and_np_in_bytes = n_in_bytes + np_in_bytes
        return n_and_np_in_bytes

    @staticmethod
    def sign_pkcs_v21(prehashed_data_to_sign, rsa_key, logger=None):
        signature = rsa_key.sign(prehashed_data_to_sign,
                                 padding.PSS(mgf=padding.MGF1(hashes.SHA256()),
                                             salt_length=global_defines.RSA_SALT_LEN
                                             ),
                                 utils.Prehashed(hashes.SHA256()))
        return signature

    @staticmethod
    def verify_signature_pkcs_v21(signature, prehashed_message, public_key, logger=None):
        """
        Verifies signature on prehashed message. Raises InvalidSignature if it does not match.
        """
        public_key.verify(signature,
                          prehashed_message,
                          padding.PSS(mgf=padding.MGF1(hashes.SHA256()),
                                      salt_length=global_defines.RSA_SALT_LEN
                                      ),
                          utils.Prehashed(hashes.SHA256()))

    @staticmethod
    def decrypt_oaep(ciphertext, private_key):
        decrypted_text = private_key.decrypt(
            ciphertext,
            padding.OAEP(
                mgf=padding.MGF1(algorithm=hashes.SHA256()),
                algorithm=hashes.SHA256(),
                label=None
            )
        )
        return decrypted_text

    @staticmethod
    def encrypt_oaep(message, public_key):
        ciphertext = public_key.encrypt(
            message,
            padding.OAEP(
                mgf=padding.MGF1(algorithm=hashes.SHA256()),
                algorithm=hashes.SHA256(),
                label=None
            )
        )
        return ciphertext

    @staticmethod
    def load_rsa_public_key_from_n(n_as_modulus):
        # todo: check for param type is integer
        rsa_params = rsa.RSAPublicNumbers(65537, n_as_modulus)
        return rsa_params.public_key(default_backend())


class HashCrypto:
    @staticmethod
    def calculate_sha256_hash(input_bytes):
        hash_function = hashes.Hash(hashes.SHA256(), backend=default_backend())
        hash_function.update(input_bytes)
        message_digest = hash_function.finalize()
        return message_digest

    @staticmethod
    def calculate_md5_hash(input_bytes):
        hash_function = hashes.Hash(hashes.MD5(), backend=default_backend())
        hash_function.update(input_bytes)
        message_digest = hash_function.finalize()
        return message_digest


class AesCrypto:
    @staticmethod
    def encrypt_aes_ctr(message, aes_key, iv):
        """
        :param message: message as bytes object
        :param aes_key: bytes-like aes key used for encryption
        :param iv: bytes-like initialization vector
        :return: ciphertext
        """
        aes_cipher = Cipher(algorithms.AES(aes_key), modes.CTR(iv), backend=default_backend())
        aes_encryptor = aes_cipher.encryptor()
        ciphertext = aes_encryptor.update(message) + aes_encryptor.finalize()
        return ciphertext

    @staticmethod
    def encrypt_aes_cbc(message, aes_key, aes_iv):
        """
        :param message: message as bytes object
        :param aes_key: bytes-like aes key used for encryption
        :param aes_iv: bytes-like initialization vector
        :return: ciphertext
        """
        aes_cipher = Cipher(algorithms.AES(aes_key), modes.CBC(aes_iv), backend=default_backend())
        aes_encryptor = aes_cipher.encryptor()
        ciphertext = aes_encryptor.update(message) + aes_encryptor.finalize()
        return ciphertext

    @staticmethod
    def decrypt_aes_cbc(ciphertext, aes_key, aes_iv):
        aes_cipher = Cipher(algorithms.AES(aes_key), modes.CBC(aes_iv), backend=default_backend())
        aes_decryptor = aes_cipher.decryptor()
        return aes_decryptor.update(ciphertext) + aes_decryptor.finalize()

    @staticmethod
    def calc_aes_cmac(message, key):
        aes_cmac = cmac.CMAC(algorithms.AES(key), backend=default_backend())
        aes_cmac.update(message)
        return aes_cmac.finalize()

    @staticmethod
    def encrypt_aes_ccm(key, nonce, associated_data, message):
        aes_ccm = aead.AESCCM(key, 16)  # tag length = 16 bytes
        ciphertext = aes_ccm.encrypt(nonce, message, associated_data)
        return ciphertext


class Common:
    @staticmethod
    def get_n_and_np_from_public_key(public_key_filename, logger=None):
        key = RsaCrypto.load_rsa_pem_key(public_key_filename, False, logger=logger)
        param_n = RsaCrypto.get_n_from_rsa_pem_public_key(key)
        param_np = RsaCrypto.calculate_np_from_n(param_n, logger)
        n_and_np_bytes = RsaCrypto.concatenate_n_and_np(param_n, param_np, logger)
        return n_and_np_bytes

    @staticmethod
    def get_hashed_n_and_np_from_public_key(public_key_filename, logger=None):
        n_and_np_bytes = Common.get_n_and_np_from_public_key(public_key_filename, logger)
        digest = HashCrypto.calculate_sha256_hash(n_and_np_bytes)
        return digest

    @staticmethod
    def get_n_and_np_from_keypair(keypair_filename, keypair_passphrase_file, logger=None):
        key = RsaCrypto.load_rsa_pem_key(keypair_filename, True, keypair_passphrase_file, logger=logger)
        param_n = RsaCrypto.get_n_from_rsa_pem_private_key(key)
        param_np = RsaCrypto.calculate_np_from_n(param_n, logger)
        n_and_np_bytes = RsaCrypto.concatenate_n_and_np(param_n, param_np, logger)
        return n_and_np_bytes

    @staticmethod
    def rsa_sign(data_to_sign, keypair_filename, keypair_passphrase_file, logger=None):
        key = RsaCrypto.load_rsa_pem_key(keypair_filename, True, keypair_passphrase_file, logger=logger)
        hashed_data_to_sign = HashCrypto.calculate_sha256_hash(data_to_sign)
        signature = RsaCrypto.sign_pkcs_v21(hashed_data_to_sign, key, logger)
        RsaCrypto.verify_signature_pkcs_v21(signature, hashed_data_to_sign, key.public_key(), logger)
        return signature

    @staticmethod
    def rsa_verify_with_pubkey_params(rsa_pubkey_n, signed_data, signature, logger=None):
        modulus_in_bytes = rsa_pubkey_n[0:global_defines.PUBKEY_SIZE_BYTES]  # chop of np is input contains it
        modulus = int.from_bytes(modulus_in_bytes, "big")
        public_key = RsaCrypto.load_rsa_public_key_from_n(modulus)
        hashed_data_to_sign = HashCrypto.calculate_sha256_hash(signed_data)
        RsaCrypto.verify_signature_pkcs_v21(signature, hashed_data_to_sign, public_key, logger)

    @staticmethod
    def decrypt_data_with_rsa_keypair(keypair_filename, keypair_passphrase_file, ciphertext, logger=None):
        private_key = RsaCrypto.load_rsa_pem_key(keypair_filename, True, keypair_passphrase_file, logger=logger)
        message = RsaCrypto.decrypt_oaep(ciphertext, private_key)
        return message

    @staticmethod
    def encrypt_data_with_rsa_pubkey_params(rsa_pubkey_n_and_np, input_data):
        modulus_in_bytes = rsa_pubkey_n_and_np[0:global_defines.PUBKEY_SIZE_BYTES]
        modulus = int.from_bytes(modulus_in_bytes, "big")
        public_key = RsaCrypto.load_rsa_public_key_from_n(modulus)
        ciphertext = RsaCrypto.encrypt_oaep(input_data, public_key)
        return ciphertext

    @staticmethod
    def encrypt_file_with_aes_ctr(input_filename, output_filename, key_int_array, aes_iv, hash_type, logger=None):
        """
        Encrypts image and calculates SHA256 hash on it
        :param input_filename: filename of image to be encrypted
        :param output_filename: filename of encrypted image
        :param key_int_array: bytes of aes key in a list
        :param aes_iv: initialization vector
        :param hash_type: 0 for hash calculated on plain image, 1 for hash on encrypted image
        :param logger:
        :return: returns the hash digest and the image size in a tuple
        """
        with open(input_filename, "rb") as input_file:
            plaintext = input_file.read()
        image_actual_size = len(plaintext)
        if image_actual_size == 0 or image_actual_size % 4 != 0:
            raise ValueError("invalid images size")
        ciphertext = AesCrypto.encrypt_aes_ctr(plaintext, bytes(key_int_array), aes_iv)
        with open(output_filename, "wb") as output_file:
            output_file.write(ciphertext)
        if hash_type == 0:
            hash_digest = HashCrypto.calculate_sha256_hash(plaintext)
        elif hash_type == 1:
            hash_digest = HashCrypto.calculate_sha256_hash(ciphertext)
        else:
            raise ValueError("Invalid input value for hash_type (crypto_type)")
        return hash_digest, image_actual_size

    @staticmethod
    def derive_aes_key(enc_password_filename, logger=None):
        # get passphrase
        if len(enc_password_filename) > 0:
            try:
                with open(enc_password_filename, "r") as pwd_file:
                    passphrase = pwd_file.read()
                    passphrase = passphrase.strip().encode("utf-8")
            except OSError as e:
                if logger is not None:
                    logger.warning("Could not open passphrase file %s" % enc_password_filename)
                raise ValueError("Could not open passphrase file: %s" % e)
        else:
            passphrase = getpass.getpass(prompt="Enter passphrase for encrypted key: ").encode("utf-8")

        # PBDF1 key and iv derivation with 1 round, no salt, MD5 hash for compatibility reasons:
        # "openssl enc -e -nosalt -md md5 -aes-128-cbc -in <in_file,bin> -out <out_file.bin> -pass file:<pwd_file.txt>"
        # and also with EVP_BytesToKey(cipher, dgst, NULL, (uint8_t *) pwdBuff, pwdBuffLen, 1, keyBuff, ivBuff);
        # in line 129 of common_crypto_sim.c of original scripts
        digest_round_1 = HashCrypto.calculate_md5_hash(passphrase)
        aes_key = digest_round_1
        aes_iv = HashCrypto.calculate_md5_hash(digest_round_1 + passphrase)
        return aes_key, aes_iv

    @staticmethod
    def encrypt_asset_with_aes_cbc(input_filename, output_filename, enc_password_filename, logger=None):
        aes_key, aes_iv = Common.derive_aes_key(enc_password_filename, logger)

        # encrypt asset with the derived key
        try:
            with open(input_filename, "rb") as asset_file:
                unencrypted_data = asset_file.read()
        except OSError as e:
            if logger is not None:
                logger.warning("Could not open asset file %s" % input_filename)
            raise ValueError("Could not open asset file: %s" % e)
        # add pkcs7 padding
        pkcs7_padder = sym_padding.PKCS7(128).padder()
        padded_unencrypted_data = pkcs7_padder.update(unencrypted_data)
        padded_unencrypted_data += pkcs7_padder.finalize()
        # encrypt
        encrypted_data = AesCrypto.encrypt_aes_cbc(padded_unencrypted_data, aes_key, aes_iv)

        # write encrypted key to output file
        with open(output_filename, "wb") as encrypted_outfile:
            encrypted_outfile.write(encrypted_data)

    @staticmethod
    def decrypt_asset_with_aes_cbc(ciphertext, password_filename, logger=None):
        aes_key, aes_iv = Common.derive_aes_key(password_filename, logger)
        decrypted_data_padded = AesCrypto.decrypt_aes_cbc(ciphertext, aes_key, aes_iv)
        # remove padding
        pkcs7_unpadder = sym_padding.PKCS7(128).unpadder()
        decrypted_data = pkcs7_unpadder.update(decrypted_data_padded)
        decrypted_data += pkcs7_unpadder.finalize()
        return decrypted_data
