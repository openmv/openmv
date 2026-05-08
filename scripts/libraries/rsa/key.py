# -*- coding: utf-8 -*-
# SPDX-FileCopyrightText: 2011 Sybren A. Stüvel <sybren@stuvel.eu>
#
# SPDX-License-Identifier: Apache-2.0

"""
`rsa.key`
====================================================

RSA key generation code.

Create new keys with the newkeys() function. It will give you a PublicKey and a
PrivateKey object.

Loading and saving keys requires the pyasn1 module. This module is imported as
late as possible, such that other functionality will remain working in absence
of pyasn1.

.. note::

    Storing public and private keys via the `pickle` module is possible.
    However, it is insecure to load a key from an untrusted source.
    The pickle module is not secure against erroneous or maliciously
    constructed data. Never unpickle data received from an untrusted
    or unauthenticated source.

"""


import rsa.prime
import rsa.pem
import rsa.common
import rsa.randnum
import rsa.core

try:
    from typing import Any, Tuple, Dict, Callable

    try:
        from typing import Literal
    except ImportError:
        from typing_extensions import Literal
except ImportError:
    pass


# pylint: disable=invalid-name, useless-object-inheritance, redefined-builtin, no-name-in-module, too-few-public-methods


DEFAULT_EXPONENT = 65537


class AbstractKey(object):
    """Abstract superclass for private and public keys."""

    def __init__(self, n: int, e: int) -> None:
        self.n = n
        self.e = e

    @classmethod
    def _load_pkcs1_pem(cls, keyfile: bytes) -> "AbstractKey":
        """Loads a key in PKCS#1 PEM format, implement in a subclass.

        :param bytes keyfile: contents of a PEM-encoded file that contains
            the public key.
        :return: the loaded key
        :rtype: AbstractKey
        """

    @classmethod
    def _load_pkcs1_der(cls, keyfile: bytes) -> "AbstractKey":
        """Loads a key in PKCS#1 PEM format, implement in a subclass.

        :param bytes keyfile: contents of a DER-encoded file that contains
            the public key.
        :return: the loaded key
        :rtype: AbstractKey
        """

    def _save_pkcs1_pem(self) -> bytes:
        """Saves the key in PKCS#1 PEM format, implement in a subclass.

        :return: the PEM-encoded key.
        :rtype: bytes
        """

    def _save_pkcs1_der(self) -> bytes:
        """Saves the key in PKCS#1 DER format, implement in a subclass.

        :return: the DER-encoded key.
        :rtype: bytes
        """

    @classmethod
    def load_pkcs1(
        cls, keyfile: bytes, format: Literal["PEM", "DER"] = "PEM"
    ) -> "AbstractKey":
        """Loads a key in PKCS#1 DER or PEM format.

        :param bytes keyfile: contents of a DER- or PEM-encoded file that
            contains the key.
        :param str format: the format of the file to load; 'PEM' or 'DER'
        :return: the loaded key
        :rtype: AbstractKey
        """
        raise NotImplementedError(
            "Loading PEM Files not supported by this CircuitPython library."
        )

        # methods = {
        #    'PEM': cls._load_pkcs1_pem,
        #    'DER': cls._load_pkcs1_der,
        # }

        # method = cls._assert_format_exists(format, methods)
        # return method(keyfile)

    @staticmethod
    def _assert_format_exists(
        file_format: str, methods: Dict[str, Callable]
    ) -> Callable[[], bytes]:
        """Checks whether the given file format exists in 'methods'."""

        try:
            return methods[file_format]
        except KeyError as err:
            formats = ", ".join(sorted(methods.keys()))
            raise ValueError(
                "Unsupported format: %r, try one of %s" % (file_format, formats)
            ) from err

    def save_pkcs1(self, format: Literal["PEM", "DER"] = "PEM") -> bytes:
        """Saves the key in PKCS#1 DER or PEM format.

        :param str format: the format to save; 'PEM' or 'DER'
        :return: the DER- or PEM-encoded key.
        :rtype: bytes
        """

        methods = {
            "PEM": self._save_pkcs1_pem,
            "DER": self._save_pkcs1_der,
        }

        method = self._assert_format_exists(format, methods)
        return method()

    def blind(self, message: int, r: int) -> int:
        """Performs blinding on the message using random number 'r'.

        :param int message: the message, as integer, to blind.
        :param int r: the random number to blind with.
        :return: the blinded message.
        :rtype: int

        The blinding is such that message = unblind(decrypt(blind(encrypt(message))).

        See https://en.wikipedia.org/wiki/Blinding_%28cryptography%29
        """

        return (message * rsa.core.fast_pow(r, self.e, self.n)) % self.n

    def unblind(self, blinded: int, r: int) -> int:
        """Performs blinding on the message using random number 'r'.

        :param int blinded: the blinded message, as integer, to unblind.
        :param int r: the random number to unblind with.
        :return: the original message.
        :rtype: int

        The blinding is such that message = unblind(decrypt(blind(encrypt(message))).

        See https://en.wikipedia.org/wiki/Blinding_%28cryptography%29
        """

        return (rsa.common.inverse(r, self.n) * blinded) % self.n


# pylint: disable=abstract-method
class PublicKey(AbstractKey):
    """Represents a public RSA key.

    This key is also known as the 'encryption key'. It contains the 'n' and 'e'
    values.

    Supports attributes as well as dictionary-like access. Attribute access is
    faster, though.

    >>> PublicKey(5, 3)
    PublicKey(5, 3)

    >>> key = PublicKey(5, 3)
    >>> key.n
    5
    >>> key['n']
    5
    >>> key.e
    3
    >>> key['e']
    3

    """

    __slots__ = ("n", "e")

    def __getitem__(self, key: str) -> Any:
        return getattr(self, key)

    def __repr__(self) -> str:
        return "PublicKey(%i, %i)" % (self.n, self.e)

    def __getstate__(self) -> Tuple[int, int]:
        """Returns the key as tuple for pickling."""
        return self.n, self.e

    def __setstate__(self, state: Tuple[int, int]) -> None:
        """Sets the key from tuple."""
        self.n, self.e = state

    def __eq__(self, other: Any) -> bool:
        if other is None:
            return False

        if not isinstance(other, PublicKey):
            return False

        return self.n == other.n and self.e == other.e

    def __ne__(self, other: Any) -> bool:
        return not self == other

    def __hash__(self) -> int:
        return hash((self.n, self.e))

    @classmethod
    def _load_pkcs1_der(cls, keyfile: bytes) -> "PublicKey":
        """Loads a key in PKCS#1 DER format.

        :param bytes keyfile: contents of a DER-encoded file that contains the
            public key.
        :return: a PublicKey object

        First let's construct a DER encoded key:

        >>> import base64
        >>> b64der = 'MAwCBQCNGmYtAgMBAAE='
        >>> der = base64.standard_b64decode(b64der)

        This loads the file:

        >>> PublicKey._load_pkcs1_der(der)
        PublicKey(2367317549, 65537)

        """
        # pylint: disable=import-outside-toplevel
        try:
            from rsa.tools.pyasn1.codec.der import decoder
            from rsa.asn1 import AsnPubKey
        except ImportError as err:
            raise ImportError("This functionality requires the pyasn1 library") from err

        (priv, _) = decoder.decode(keyfile, asn1Spec=AsnPubKey())
        return cls(n=int(priv["modulus"]), e=int(priv["publicExponent"]))

    def _save_pkcs1_der(self) -> bytes:
        """Saves the public key in PKCS#1 DER format.

        :return: the DER-encoded public key.
        :rtype: bytes
        """
        # pylint: disable=import-outside-toplevel
        try:
            from pyasn1.codec.der import encoder
        except ImportError as err:
            raise ImportError("This functionality requires the  library") from err
        try:
            from rsa.asn1 import AsnPubKey
        except ImportError as err:
            raise ImportError(
                "This functionality requres the CPython rsa library, "
                "not available in CircuitPython"
            ) from err

        # Create the ASN object
        asn_key = AsnPubKey()
        asn_key.setComponentByName("modulus", self.n)
        asn_key.setComponentByName("publicExponent", self.e)

        return encoder.encode(asn_key)

    @classmethod
    def _load_pkcs1_pem(cls, keyfile: bytes) -> "PublicKey":
        """Loads a PKCS#1 PEM-encoded public key file.

        The contents of the file before the "-----BEGIN RSA PUBLIC KEY-----" and
        after the "-----END RSA PUBLIC KEY-----" lines is ignored.

        :param bytes keyfile: contents of a PEM-encoded file that contains the
            public key.
        :return: a PublicKey object
        """

        der = rsa.pem.load_pem(keyfile, "RSA PUBLIC KEY")
        return cls._load_pkcs1_der(der)

    def _save_pkcs1_pem(self) -> bytes:
        """Saves a PKCS#1 PEM-encoded public key file.

        :return: contents of a PEM-encoded file that contains the public key.
        :rtype: bytes
        """

        der = self._save_pkcs1_der()
        return rsa.pem.save_pem(der, "RSA PUBLIC KEY")

    @classmethod
    def load_pkcs1_openssl_pem(cls, keyfile: bytes) -> "PublicKey":
        """Loads a PKCS#1.5 PEM-encoded public key file from OpenSSL.

        These files can be recognised in that they start with BEGIN PUBLIC KEY
        rather than BEGIN RSA PUBLIC KEY.

        The contents of the file before the "-----BEGIN PUBLIC KEY-----" and
        after the "-----END PUBLIC KEY-----" lines is ignored.

        :param bytes keyfile: contents of a PEM-encoded file that contains the
            public key, from OpenSSL.
        :type keyfile: bytes
        :return: a PublicKey object
        """

        der = rsa.pem.load_pem(keyfile, "PUBLIC KEY")
        return cls.load_pkcs1_openssl_der(der)

    @classmethod
    def load_pkcs1_openssl_der(cls, keyfile: bytes) -> "PublicKey":
        """Loads a PKCS#1 DER-encoded public key file from OpenSSL.

        :param bytes keyfile: contents of a DER-encoded file that contains the
            public key, from OpenSSL.
        :return: a PublicKey object
        """
        # pylint: disable=import-outside-toplevel
        try:
            from rsa.asn1 import OpenSSLPubKey
            from pyasn1.codec.der import decoder
            from pyasn1.type import univ
        except ImportError as err:
            raise ImportError("This functionality requires the pyasn1 library") from err

        (keyinfo, _) = decoder.decode(keyfile, asn1Spec=OpenSSLPubKey())

        if keyinfo["header"]["oid"] != univ.ObjectIdentifier("1.2.840.113549.1.1.1"):
            raise TypeError("This is not a DER-encoded OpenSSL-compatible public key")

        return cls._load_pkcs1_der(keyinfo["key"][1:])


class PrivateKey(AbstractKey):
    """Represents a private RSA key.

    This key is also known as the 'decryption key'. It contains the 'n', 'e',
    'd', 'p', 'q' and other values.

    Supports attributes as well as dictionary-like access. Attribute access is
    faster, though.

    >>> PrivateKey(3247, 65537, 833, 191, 17)
    PrivateKey(3247, 65537, 833, 191, 17)

    exp1, exp2 and coef will be calculated:

    >>> pk = PrivateKey(3727264081, 65537, 3349121513, 65063, 57287)
    >>> pk.exp1
    55063
    >>> pk.exp2
    10095
    >>> pk.coef
    50797

    """

    __slots__ = ("n", "e", "d", "p", "q", "exp1", "exp2", "coef")

    # pylint: disable=too-many-arguments
    def __init__(self, n: int, e: int, d: int, p: int, q: int) -> None:
        AbstractKey.__init__(self, n, e)
        self.d = d
        self.p = p
        self.q = q

        # Calculate exponents and coefficient.
        self.exp1 = int(d % (p - 1))
        self.exp2 = int(d % (q - 1))
        self.coef = rsa.common.inverse(q, p)

    def __getitem__(self, key: str) -> Any:
        return getattr(self, key)

    def __repr__(self) -> str:
        return "PrivateKey(%i, %i, %i, %i, %i)" % (
            self.n,
            self.e,
            self.d,
            self.p,
            self.q,
        )

    def __getstate__(self) -> Tuple[int, int, int, int, int, int, int, int]:
        """Returns the key as tuple for pickling."""
        return self.n, self.e, self.d, self.p, self.q, self.exp1, self.exp2, self.coef

    def __setstate__(
        self, state: Tuple[int, int, int, int, int, int, int, int]
    ) -> None:
        """Sets the key from tuple."""
        self.n, self.e, self.d, self.p, self.q, self.exp1, self.exp2, self.coef = state

    def __eq__(self, other: Any) -> bool:
        if other is None:
            return False

        if not isinstance(other, PrivateKey):
            return False

        return (
            self.n == other.n
            and self.e == other.e
            and self.d == other.d
            and self.p == other.p
            and self.q == other.q
            and self.exp1 == other.exp1
            and self.exp2 == other.exp2
            and self.coef == other.coef
        )

    def __ne__(self, other: Any) -> bool:
        return not self == other

    def __hash__(self) -> int:
        return hash(
            (self.n, self.e, self.d, self.p, self.q, self.exp1, self.exp2, self.coef)
        )

    def blinded_decrypt(self, encrypted: int) -> int:
        """Decrypts the message using blinding to prevent side-channel attacks.

        :param int encrypted: the encrypted message

        :return: the decrypted message
        :rtype: int
        """

        blind_r = rsa.randnum.randint(self.n - 1)
        blinded = self.blind(encrypted, blind_r)  # blind before decrypting
        decrypted = rsa.core.decrypt_int(blinded, self.d, self.n)

        return self.unblind(decrypted, blind_r)

    def blinded_encrypt(self, message: int) -> int:
        """Encrypts the message using blinding to prevent side-channel attacks.

        :param int message: the message to encrypt
        :return: the encrypted message
        :rtype: int
        """

        blind_r = rsa.randnum.randint(self.n - 1)
        blinded = self.blind(message, blind_r)  # blind before encrypting
        encrypted = rsa.core.encrypt_int(blinded, self.d, self.n)
        return self.unblind(encrypted, blind_r)

    @classmethod
    def _load_pkcs1_der(cls, keyfile: bytes) -> "PrivateKey":
        """Loads a key in PKCS#1 DER format.

        :param bytes keyfile: contents of a DER-encoded file that contains the
            private key.
        :return: a PrivateKey object

        First let's construct a DER encoded key:

        >>> import base64
        >>> b64der = 'MC4CAQACBQDeKYlRAgMBAAECBQDHn4npAgMA/icCAwDfxwIDANcXAgInbwIDAMZt'
        >>> der = base64.standard_b64decode(b64der)

        This loads the file:

        >>> PrivateKey._load_pkcs1_der(der)
        PrivateKey(3727264081, 65537, 3349121513, 65063, 57287)

        """

        try:
            from rsa.tools.pyasn1.codec.der import (  # pylint: disable=import-outside-toplevel
                decoder,
            )
        except ImportError as err:
            raise ImportError("This functionality requires the pyasn1 library") from err

        (priv, _) = decoder.decode(keyfile)

        # ASN.1 contents of DER encoded private key:
        #
        # RSAPrivateKey ::= SEQUENCE {
        #     version           Version,
        #     modulus           INTEGER,  -- n
        #     publicExponent    INTEGER,  -- e
        #     privateExponent   INTEGER,  -- d
        #     prime1            INTEGER,  -- p
        #     prime2            INTEGER,  -- q
        #     exponent1         INTEGER,  -- d mod (p-1)
        #     exponent2         INTEGER,  -- d mod (q-1)
        #     coefficient       INTEGER,  -- (inverse of q) mod p
        #     otherPrimeInfos   OtherPrimeInfos OPTIONAL
        # }

        if priv[0] != 0:
            raise ValueError("Unable to read this file, version %s != 0" % priv[0])

        as_ints = map(int, priv[1:6])
        key = cls(*as_ints)

        exp1, exp2, coef = map(int, priv[6:9])

        if (key.exp1, key.exp2, key.coef) != (exp1, exp2, coef):
            pass

        return key

    def _save_pkcs1_der(self) -> bytes:
        """Saves the private key in PKCS#1 DER format.

        :return: the DER-encoded private key.
        :rtype: bytes
        """
        # pylint: disable=import-outside-toplevel
        try:
            from pyasn1.type import univ, namedtype
            from pyasn1.codec.der import encoder
        except ImportError as err:
            raise ImportError("This functionality requires the pyasn1 library") from err

        class AsnPrivKey(univ.Sequence):
            """Creates PKCS#1 DER Formatted AsnPrivKey"""

            componentType = namedtype.NamedTypes(
                namedtype.NamedType("version", univ.Integer()),
                namedtype.NamedType("modulus", univ.Integer()),
                namedtype.NamedType("publicExponent", univ.Integer()),
                namedtype.NamedType("privateExponent", univ.Integer()),
                namedtype.NamedType("prime1", univ.Integer()),
                namedtype.NamedType("prime2", univ.Integer()),
                namedtype.NamedType("exponent1", univ.Integer()),
                namedtype.NamedType("exponent2", univ.Integer()),
                namedtype.NamedType("coefficient", univ.Integer()),
            )

        # Create the ASN object
        asn_key = AsnPrivKey()
        asn_key.setComponentByName("version", 0)
        asn_key.setComponentByName("modulus", self.n)
        asn_key.setComponentByName("publicExponent", self.e)
        asn_key.setComponentByName("privateExponent", self.d)
        asn_key.setComponentByName("prime1", self.p)
        asn_key.setComponentByName("prime2", self.q)
        asn_key.setComponentByName("exponent1", self.exp1)
        asn_key.setComponentByName("exponent2", self.exp2)
        asn_key.setComponentByName("coefficient", self.coef)

        return encoder.encode(asn_key)

    @classmethod
    def _load_pkcs1_pem(cls, keyfile: bytes) -> "PrivateKey":
        """Loads a PKCS#1 PEM-encoded private key file.

        The contents of the file before the "-----BEGIN RSA PRIVATE KEY-----" and
        after the "-----END RSA PRIVATE KEY-----" lines is ignored.

        :param bytes keyfile: contents of a PEM-encoded file that contains the
            private key.
        :return: a PrivateKey object
        """

        der = rsa.pem.load_pem(keyfile, b"RSA PRIVATE KEY")
        return cls._load_pkcs1_der(der)

    def _save_pkcs1_pem(self) -> bytes:
        """Saves a PKCS#1 PEM-encoded private key file.

        :return: contents of a PEM-encoded file that contains the private key.
        :rtype: bytes
        """

        der = self._save_pkcs1_der()
        return rsa.pem.save_pem(der, b"RSA PRIVATE KEY")


def find_p_q(
    nbits: int,
    getprime_func: Callable[[int], int] = rsa.prime.getprime,
    accurate: bool = True,
) -> Tuple[int, int]:
    """Returns a tuple of two different primes of nbits bits each.

    The resulting p * q has exacty 2 * nbits bits, and the returned p and q
    will not be equal.

    :param nbits: the number of bits in each of p and q.
    :param getprime_func: the getprime function, defaults to
        :py:func:`rsa.prime.getprime`.

        *Introduced in Python-RSA 3.1*

    :param accurate: whether to enable accurate mode or not.
    :return: (p, q), where p > q

    >>> (p, q) = find_p_q(128)
    >>> from rsa.rsa import common
    >>> common.bit_size(p * q)
    256

    When not in accurate mode, the number of bits can be slightly less

    >>> (p, q) = find_p_q(128, accurate=False)
    >>> from rsa.rsa import common
    >>> common.bit_size(p * q) <= 256
    True
    >>> common.bit_size(p * q) > 240
    True

    """

    total_bits = nbits * 2

    # Make sure that p and q aren't too close or the factoring programs can
    # factor n.
    shift = nbits // 16
    pbits = nbits + shift
    qbits = nbits - shift

    # Choose the two initial primes
    p = getprime_func(pbits)
    q = getprime_func(qbits)

    def is_acceptable(p: int, q: int) -> bool:
        """Returns True iff p and q are acceptable:

        - p and q differ
        - (p * q) has the right nr of bits (when accurate=True)
        """

        if p == q:
            return False

        if not accurate:
            return True

        # Make sure we have just the right amount of bits
        found_size = rsa.common.bit_size(p * q)
        return total_bits == found_size

    # Keep choosing other primes until they match our requirements.
    change_p = False
    while not is_acceptable(p, q):
        # Change p on one iteration and q on the other
        if change_p:
            p = getprime_func(pbits)
        else:
            q = getprime_func(qbits)

        change_p = not change_p

    # We want p > q as described on
    # http://www.di-mgt.com.au/rsa_alg.html#crt
    return max(p, q), min(p, q)


def calculate_keys_custom_exponent(p: int, q: int, exponent: int) -> Tuple[int, int]:
    """Calculates an encryption and a decryption key given p, q and an exponent,
    and returns them as a tuple (e, d)

    :param int p: the first large prime
    :param int q: the second large prime
    :param int exponent: the exponent for the key; only change this if you
        know what you're doing, as the exponent influences how difficult
        your private key can be cracked. A very common choice for e is 65537.
    """

    phi_n = (p - 1) * (q - 1)

    try:
        d = rsa.common.inverse(exponent, phi_n)
    except rsa.common.NotRelativePrimeError as ex:
        raise rsa.common.NotRelativePrimeError(
            exponent,
            phi_n,
            ex.d,
            msg="e (%d) and phi_n (%d) are not relatively prime (divider=%i)"
            % (exponent, phi_n, ex.d),
        )

    if (exponent * d) % phi_n != 1:
        raise ValueError(
            "e (%d) and d (%d) are not mult. inv. modulo "
            "phi_n (%d)" % (exponent, d, phi_n)
        )

    return exponent, d


def calculate_keys(p: int, q: int) -> Tuple[int, int]:
    """Calculates an encryption and a decryption key given p and q, and
    returns them as a tuple (e, d)

    :param int p: the first large prime
    :param int q: the second large prime

    :return: tuple (e, d) with the encryption and decryption exponents.
    """

    return calculate_keys_custom_exponent(p, q, DEFAULT_EXPONENT)


def gen_keys(
    nbits: int,
    getprime_func: Callable[[int], int],
    accurate: bool = True,
    exponent: int = DEFAULT_EXPONENT,
) -> Tuple[int, int, int, int]:
    """Generate RSA keys of nbits bits. Returns (p, q, e, d).

    Note: this can take a long time, depending on the key size.

    :param int nbits: the total number of bits in ``p`` and ``q``. Both ``p`` and
        ``q`` will use ``nbits/2`` bits.
    :param Callable getprime_func: either :py:func:`rsa.rsa.prime.getprime` or a function
        with similar signature.
    :param bool accurate: when True, ``n`` will have exactly the number of bits you
        asked for. However, this makes key generation much slower. When False,
        `n`` may have slightly less bits.
    :param int exponent: the exponent for the key; only change this if you know
        what you're doing, as the exponent influences how difficult your
        private key can be cracked. A very common choice for e is 65537.
    """

    # Regenerate p and q values, until calculate_keys doesn't raise a
    # ValueError.
    while True:
        (p, q) = find_p_q(nbits // 2, getprime_func, accurate)
        try:
            (e, d) = calculate_keys_custom_exponent(p, q, exponent=exponent)
            break
        except ValueError:
            pass

    return p, q, e, d


def newkeys(
    nbits: int,
    accurate: bool = True,
    poolsize: int = 1,
    exponent: int = DEFAULT_EXPONENT,
    log_level: str = "INFO",
) -> Tuple["PublicKey", "PrivateKey"]:
    """Generates public and private keys, and returns them as (pub, priv).

    The public key is also known as the 'encryption key', and is a
    :py:class:`rsa.rsa.PublicKey` object. The private key is also known as the
    'decryption key' and is a :py:class:`rsa.rsa.PrivateKey` object.

    :param int nbits: the number of bits required to store ``n = p*q``.
    :param bool accurate: when True, ``n`` will have exactly the number of bits you
        asked for. However, this makes key generation much slower. When False,
        ``n`` may have slightly less bits.
    :param int poolsize: the number of processes to use to generate the prime
        numbers.
    :param int exponent: the exponent for the key; only change this if you know
        what you're doing, as the exponent influences how difficult your
        private key can be cracked. A very common choice for e is 65537.
    :param str log_level: Logger level, setting to DEBUG will log info about when
                        p and q are generating.

    :return: a tuple (:py:class:`rsa.PublicKey`, :py:class:`rsa.PrivateKey`)

    The ``poolsize`` parameter was added in *Python-RSA 3.1* and requires
    Python 2.6 or newer.

    """

    if nbits < 16:
        raise ValueError("Key too small")

    if poolsize < 1:
        raise ValueError("Pool size (%i) should be >= 1" % poolsize)

    getprime_func = rsa.prime.getprime

    # Generate the key components
    (p, q, e, d) = gen_keys(nbits, getprime_func, accurate=accurate, exponent=exponent)

    # Create the key objects
    n = p * q

    return (PublicKey(n, e), PrivateKey(n, e, d, p, q))


__all__ = ["PublicKey", "PrivateKey", "newkeys"]
