# -*- coding: utf-8 -*-
# SPDX-FileCopyrightText: 2011 Sybren A. Stüvel <sybren@stuvel.eu>
#
# SPDX-License-Identifier: Apache-2.0

"""
`rsa.pkcs1`
====================================================

Functions for PKCS#1 version 1.5 encryption and signing

This module implements certain functionality from PKCS#1 version 1.5. For a
very clear example, read http://www.di-mgt.com.au/rsa_alg.html#pkcs1schemes

At least 8 bytes of random padding is used when encrypting a message. This makes
these methods much more secure than the ones in the ``rsa`` module.

WARNING: this module leaks information when decryption fails. The exceptions
that are raised contain the Python traceback information, which can be used to
deduce where in the process the failure occurred. DO NOT PASS SUCH INFORMATION
to your users.
"""

import os
import hashlib as hashlib
from rsa import common, transform, core

try:
    from typing import Optional, Iterator, Union
    from rsa.key import PublicKey, PrivateKey

    try:
        from typing import Protocol
    except ImportError:
        from typing_extensions import Protocol

    try:
        from typing import Literal
    except ImportError:
        from typing_extensions import Literal

    class _FileLikeObject(Protocol):
        """A file like object that implements the :meth:`read` method"""

        def read(self, blocksize: int) -> Union[bytes, str]:
            """A method that reads a given number of bytes or chracters"""


except ImportError:
    pass


# ASN.1 codes that describe the hash algorithm used.
HASH_ASN1 = {
    "MD5": b"\x30\x20\x30\x0c\x06\x08\x2a\x86\x48\x86\xf7\x0d\x02\x05\x05\x00\x04\x10",
    "SHA-1": b"\x30\x21\x30\x09\x06\x05\x2b\x0e\x03\x02\x1a\x05\x00\x04\x14",
    "SHA-224": b"\x30\x2d\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x04\x05\x00\x04\x1c",
    "SHA-256": b"\x30\x31\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x01\x05\x00\x04\x20",
    "SHA-384": b"\x30\x41\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x02\x05\x00\x04\x30",
    "SHA-512": b"\x30\x51\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x03\x05\x00\x04\x40",
}

hash_methods_to_check = {
    "MD5": "md5",
    "SHA-1": "sha1",
    "SHA-224": "sha224",
    "SHA-256": "sha256",
    "SHA-384": "sha384",
    "SHA-512": "sha512",
}

HASH_METHODS = {
    method: getattr(hashlib, attr)
    for method, attr in hash_methods_to_check.items()
    if hasattr(hashlib, attr)
}


class CryptoError(Exception):
    """Base class for all exceptions in this module."""


class DecryptionError(CryptoError):
    """Raised when decryption fails."""


class VerificationError(CryptoError):
    """Raised when verification fails."""


def _pad_for_encryption(message: bytes, target_length: int) -> bytes:
    r"""Pads the message for encryption, returning the padded message.

    :param bytes message: The message
    :param int target_length: The length of the padded message
    :return: 00 02 RANDOM_DATA 00 MESSAGE
    :rtype: bytes

    >>> block = _pad_for_encryption(b'hello', 16)
    >>> len(block)
    16
    >>> block[0:2]
    b'\x00\x02'
    >>> block[-6:]
    b'\x00hello'

    """

    max_msglength = target_length - 11
    msglength = len(message)

    if msglength > max_msglength:
        raise OverflowError(
            "%i bytes needed for message, but there is only"
            " space for %i" % (msglength, max_msglength)
        )

    # Get random padding
    padding = b""
    padding_length = target_length - msglength - 3

    # We remove 0-bytes, so we'll end up with less padding than we've asked for,
    # so keep adding data until we're at the correct length.
    while len(padding) < padding_length:
        needed_bytes = padding_length - len(padding)

        # Always read at least 8 bytes more than we need, and trim off the rest
        # after removing the 0-bytes. This increases the chance of getting
        # enough bytes, especially when needed_bytes is small
        new_padding = os.urandom(needed_bytes + 5)
        new_padding = new_padding.replace(b"\x00", b"")
        padding = padding + new_padding[:needed_bytes]

    assert len(padding) == padding_length

    return b"".join([b"\x00\x02", padding, b"\x00", message])


def _pad_for_signing(message: bytes, target_length: int) -> bytes:
    r"""Pads the message for signing, returning the padded message.

    The padding is always a repetition of FF bytes.

    :param bytes message: The message to pad
    :param int target_length: The length to pad the message
    :return: 00 01 PADDING 00 MESSAGE
    :rtype: bytes

    >>> block = _pad_for_signing(b'hello', 16)
    >>> len(block)
    16
    >>> block[0:2]
    b'\x00\x01'
    >>> block[-6:]
    b'\x00hello'
    >>> block[2:-6]
    b'\xff\xff\xff\xff\xff\xff\xff\xff'

    """

    max_msglength = target_length - 11
    msglength = len(message)

    if msglength > max_msglength:
        raise OverflowError(
            "%i bytes needed for message, but there is only"
            " space for %i" % (msglength, max_msglength)
        )

    padding_length = target_length - msglength - 3

    return b"".join([b"\x00\x01", padding_length * b"\xff", b"\x00", message])


def encrypt(message: bytes, pub_key: PublicKey) -> bytes:
    """Encrypts the given message using PKCS#1 v1.5

    :param bytes message: the message to encrypt. Must be a byte string no longer than
        ``k-11`` bytes, where ``k`` is the number of bytes needed to encode
        the ``n`` component of the public key.
    :param PublicKey pub_key: the :py:class:`rsaPublicKey` to encrypt with.
    :raise OverflowError: when the message is too large to fit in the padded
        block.

    >>> from rsa.rsa import key, common
    >>> (pub_key, priv_key) = key.newkeys(256)
    >>> message = b'hello'
    >>> crypto = encrypt(message, pub_key)

    The crypto text should be just as long as the public key 'n' component:

    >>> len(crypto) == common.byte_size(pub_key.n)
    True

    """

    keylength = common.byte_size(pub_key.n)
    padded = _pad_for_encryption(message, keylength)
    payload = transform.bytes2int(padded)
    encrypted = core.encrypt_int(payload, pub_key.e, pub_key.n)
    block = transform.int2bytes(encrypted, keylength)

    return block


def decrypt(crypto: bytes, priv_key: PrivateKey) -> bytes:
    """Decrypts the given message using PKCS#1 v1.5

    The decryption is considered 'failed' when the resulting cleartext doesn't
    start with the bytes 00 02, or when the 00 byte between the padding and
    the message cannot be found.

    :param bytes crypto: the crypto text as returned by :py:func:`rsaencrypt`
    :param PrivateKey priv_key: the :py:class:`rsaPrivateKey` to decrypt with.
    :raise DecryptionError: when the decryption fails. No details are given as
        to why the code thinks the decryption fails, as this would leak
        information about the private key.


    >>> import rsa.rsa
    >>> (pub_key, priv_key) = rsanewkeys(256)

    It works with strings:

    >>> crypto = encrypt(b'hello', pub_key)
    >>> decrypt(crypto, priv_key)
    b'hello'

    And with binary data:

    >>> crypto = encrypt(b'\x00\x00\x00\x00\x01', pub_key)
    >>> decrypt(crypto, priv_key)
    b'\x00\x00\x00\x00\x01'

    Altering the encrypted information will *likely* cause a
    :py:class:`rsapkcs1.DecryptionError`. If you want to be *sure*, use
    :py:func:`rsasign`.


    .. warning::

        Never display the stack trace of a
        :py:class:`rsapkcs1.DecryptionError` exception. It shows where in the
        code the exception occurred, and thus leaks information about the key.
        It's only a tiny bit of information, but every bit makes cracking the
        keys easier.

    >>> crypto = encrypt(b'hello', pub_key)
    >>> crypto = crypto[0:5] + b'X' + crypto[6:] # change a byte
    >>> decrypt(crypto, priv_key)
    Traceback (most recent call last):
    ...
    rsapkcs1.DecryptionError: Decryption failed

    """

    blocksize = common.byte_size(priv_key.n)
    encrypted = transform.bytes2int(crypto)
    decrypted = priv_key.blinded_decrypt(encrypted)
    cleartext = transform.int2bytes(decrypted, blocksize)

    # Find the 00 separator between the padding and the message
    try:
        sep_idx = cleartext.index(b"\x00", 2)
    except ValueError as err:
        raise DecryptionError("Decryption failed") from err

    return cleartext[sep_idx + 1 :]


def sign_hash(
    hash_value: Optional[bytes],
    priv_key: PrivateKey,
    hash_method: Literal["MD5", "SHA-1", "SHA-224", "SHA-256", "SHA-384", "SHA-512"],
) -> bytes:
    """Signs a precomputed hash with the private key.

    Hashes the message, then signs the hash with the given key. This is known
    as a "detached signature", because the message itself isn't altered.

    :param bytes hash_value: A precomputed hash to sign (ignores message). Should be
        set to ``None`` if needing to hash and sign message.
    :param PrivateKey priv_key: the :py:class:`rsaPrivateKey` to sign with
    :param hash_method: the hash method used on the message. Use 'MD5', 'SHA-1',
        'SHA-224', SHA-256', 'SHA-384' or 'SHA-512'.
    :return: a message signature block.
    :raise OverflowError: if the private key is too small to contain the
        requested hash.

    """

    # Get the ASN1 code for this hash method
    if hash_method not in HASH_ASN1:
        raise ValueError("Invalid hash method: %s" % hash_method)
    asn1code = HASH_ASN1[hash_method]

    # Encrypt the hash with the private key
    cleartext = asn1code + hash_value
    keylength = common.byte_size(priv_key.n)
    padded = _pad_for_signing(cleartext, keylength)

    payload = transform.bytes2int(padded)
    encrypted = priv_key.blinded_encrypt(payload)
    block = transform.int2bytes(encrypted, keylength)

    return block


def sign(
    message: Union[bytes, _FileLikeObject], priv_key: PrivateKey, hash_method: str
) -> bytes:
    """Signs the message with the private key.

    Hashes the message, then signs the hash with the given key. This is known
    as a "detached signature", because the message itself isn't altered.

    :param message: the message to sign. Can be an 8-bit string or a file-like
        object. If ``message`` has a ``read()`` method, it is assumed to be a
        file-like object.
    :param PrivateKey priv_key: the :py:class:`rsaPrivateKey` to sign
        with
    :param hash_method: the hash method used on the message. Use 'MD5', 'SHA-1',
        'SHA-224', SHA-256', 'SHA-384' or 'SHA-512'.
    :return: a message signature block.
    :raise OverflowError: if the private key is too small to contain the
        requested hash.

    """

    msg_hash = compute_hash(message, hash_method)
    return sign_hash(msg_hash, priv_key, hash_method)


def verify(
    message: Union[bytes, _FileLikeObject], signature: bytes, pub_key: PublicKey
) -> str:
    """Verifies that the signature matches the message.

    The hash method is detected automatically from the signature.

    :param message: the signed message. Can be an 8-bit string or a file-like
        object. If ``message`` has a ``read()`` method, it is assumed to be a
        file-like object.
    :param bytes signature: the signature block, as created with :py:func:`rsa.sign`.
    :param PublicKey pub_key: the :py:class:`rsaPublicKey` of the person
        signing the message.
    :raise VerificationError: when the signature doesn't match the message.
    :return: the name of the used hash.

    """

    keylength = common.byte_size(pub_key.n)
    encrypted = transform.bytes2int(signature)
    decrypted = core.decrypt_int(encrypted, pub_key.e, pub_key.n)
    clearsig = transform.int2bytes(decrypted, keylength)

    # Get the hash method
    method_name = _find_method_hash(clearsig)
    message_hash = compute_hash(message, method_name)

    # Reconstruct the expected padded hash
    cleartext = HASH_ASN1[method_name] + message_hash
    expected = _pad_for_signing(cleartext, keylength)

    # Compare with the signed one
    if expected != clearsig:
        raise VerificationError("Verification failed")

    return method_name


def find_signature_hash(signature: bytes, pub_key: PublicKey) -> str:
    """Returns the hash name detected from the signature.

    If you also want to verify the message, use :py:func:`rsaverify()` instead.
    It also returns the name of the used hash.

    :param bytes signature: the signature block, as created with
        :py:func:`rsasign`.
    :param PublicKey pub_key: the :py:class:`rsaPublicKey`
        of the person signing the message.
    :return: the name of the used hash.
    """

    keylength = common.byte_size(pub_key.n)
    encrypted = transform.bytes2int(signature)
    decrypted = core.decrypt_int(encrypted, pub_key.e, pub_key.n)
    clearsig = transform.int2bytes(decrypted, keylength)

    return _find_method_hash(clearsig)


def yield_fixedblocks(
    infile: _FileLikeObject, blocksize: int
) -> Iterator[Union[bytes, str]]:
    """Generator, yields each block of ``blocksize`` bytes in the input file.

    :param TextIOWrapper infile: file to read and separate in blocks.
    :param int blocksize: block size in bytes.
    :return: a generator that yields the contents of each block
    """

    while True:
        block = infile.read(blocksize)

        read_bytes = len(block)
        if read_bytes == 0:
            break

        yield block

        if read_bytes < blocksize:
            break


def compute_hash(
    message: Union[bytes, str, _FileLikeObject], method_name: str
) -> bytes:
    """Returns the message digest.

    :param message: the signed message. Can be an 8-bit string or a file-like
        object. If ``message`` has a ``read()`` method, it is assumed to be a
        file-like object.
    :param method_name: the hash method, must be a key of
        :py:const:`HASH_METHODS`.
    """

    if method_name not in HASH_METHODS:
        raise ValueError("Invalid or unsupported hash method: %s" % method_name)

    method = HASH_METHODS[method_name]
    hasher = method()

    if hasattr(message, "read") and hasattr(message.read, "__call__"):
        # read as 1K blocks
        for block in yield_fixedblocks(message, 1024):
            hasher.update(block)
    else:
        # hash the message object itself.
        hasher.update(message)

    return hasher.digest()


def _find_method_hash(clearsig: bytes) -> str:
    """Finds the hash method.

    :param bytes clearsig: full padded ASN1 and hash.
    :return: the used hash method.
    :raise VerificationFailed: when the hash method cannot be found
    """

    for (hashname, asn1code) in HASH_ASN1.items():
        if asn1code in clearsig:
            return hashname

    raise VerificationError("Verification failed")


__all__ = [
    "encrypt",
    "decrypt",
    "sign",
    "verify",
    "DecryptionError",
    "VerificationError",
    "CryptoError",
]
