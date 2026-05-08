# -*- coding: utf-8 -*-
# SPDX-FileCopyrightText: 2011 Sybren A. Stüvel <sybren@stuvel.eu>
#
# SPDX-License-Identifier: Apache-2.0

"""
`rsa.common`
====================================================

Common functionality shared by several modules.
"""

# pylint: disable=invalid-name

try:
    from typing import Optional, Tuple, Sequence
except ImportError:
    pass


def bit_length(int_type: int) -> int:
    """Return the number of bits necessary to represent an integer in binary,
    excluding the sign and leading zeros

    :param int int_type: The integer to check
    """

    length = 0
    while int_type:
        int_type >>= 1
        length += 1
    return length


class NotRelativePrimeError(ValueError):
    """Raises if provided a and b not relatively prime."""

    def __init__(self, a: int, b: int, d: int, msg: Optional[str] = None):
        super().__init__(
            msg or "%d and %d are not relatively prime, divider=%i" % (a, b, d)
        )
        self.a = a
        self.b = b
        self.d = d


def bit_size(num: int) -> int:
    """
    Number of bits needed to represent a integer excluding any prefix
    0 bits.

    Usage::

        >>> bit_size(1023)
        10
        >>> bit_size(1024)
        11
        >>> bit_size(1025)
        11

    :param int num:
        Integer value. If num is 0, returns 0. Only the absolute value of the
        number is considered. Therefore, signed integers will be abs(num)
        before the number's bit length is determined.
    :return:
        Returns the number of bits in the integer.
    """

    try:
        return bit_length(num)
    except AttributeError as err:
        raise TypeError(
            "bit_size(num) only supports integers, not %r" % type(num)
        ) from err


def byte_size(number: int) -> int:
    """
    Returns the number of bytes required to hold a specific long number.

    The number of bytes is rounded up.

    Usage::

        >>> byte_size(1 << 1023)
        128
        >>> byte_size((1 << 1024) - 1)
        128
        >>> byte_size(1 << 1024)
        129

    :param int number: An unsigned integer
    :return: The number of bytes required to hold a specific long number.
    """
    if number == 0:
        return 1
    return ceil_div(bit_size(number), 8)


def ceil_div(num: int, div: int) -> int:
    """
    Returns the ceiling function of a division between ``num`` and ``div``.

    Usage:

        >>> ceil_div(100, 7)
        15
        >>> ceil_div(100, 10)
        10
        >>> ceil_div(1, 4)
        1

    :param int num: Division's numerator, a number
    :param int div: Division's divisor, a number
    :return: Rounded up result of the division between the parameters.
    """
    quanta, mod = divmod(num, div)
    if mod:
        quanta += 1
    return quanta


def extended_gcd(a: int, b: int) -> Tuple[int, int, int]:
    """Returns a tuple (r, i, j) such that r = gcd(a, b) = ia + jb"""
    # r = gcd(a,b) i = multiplicitive inverse of a mod b
    #      or      j = multiplicitive inverse of b mod a
    # Neg return values for i or j are made positive mod b or a respectively
    # Iterateive Version is faster and uses much less stack space
    x = 0
    y = 1
    lx = 1
    ly = 0
    oa = a  # Remember original a/b to remove
    ob = b  # negative values from return results
    while b != 0:
        q = a // b
        (a, b) = (b, a % b)
        (x, lx) = ((lx - (q * x)), x)
        (y, ly) = ((ly - (q * y)), y)
    if lx < 0:
        lx += ob  # If neg wrap modulo orignal b
    if ly < 0:
        ly += oa  # If neg wrap modulo orignal a
    return a, lx, ly  # Return only positive values


def inverse(x: int, n: int) -> int:
    """Returns the inverse of x % n under multiplication, a.k.a x^-1 (mod n)

    >>> inverse(7, 4)
    3
    >>> (inverse(143, 4) * 143) % 4
    1
    """

    (divider, inv, _) = extended_gcd(x, n)

    if divider != 1:
        raise NotRelativePrimeError(x, n, divider)

    return inv


def crt(a_values: Sequence[int], modulo_values: Sequence[int]) -> int:
    """Chinese Remainder Theorem.

    Calculates x such that x = a[i] (mod m[i]) for each i.

    :param Sequence[int] a_values: the a-values of the above equation
    :param Sequence[int] modulo_values: the m-values of the above equation
    :return: x such that x = a[i] (mod m[i]) for each i


    >>> crt([2, 3], [3, 5])
    8

    >>> crt([2, 3, 2], [3, 5, 7])
    23

    >>> crt([2, 3, 0], [7, 11, 15])
    135
    """

    m = 1
    x = 0

    for modulo in modulo_values:
        m *= modulo

    for (m_i, a_i) in zip(modulo_values, a_values):
        M_i = m // m_i
        inv = inverse(M_i, m_i)

        x = (x + a_i * M_i * inv) % m

    return x
