'''

Part of the micro-linalg project to provide a small
matrix / linear algebra package for Micropython (Python3)

The MIT License (MIT)

Copyright (c) 2015 Jamie Lawson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
'''
import math
import umatrix


def zeros(m, n, dtype=umatrix.ddtype):
    return umatrix.matrix([[0 for i in range(n)] for j in range(m)], dtype=dtype)


def ones(m, n, dtype=umatrix.ddtype):
    return zeros(m, n, dtype) + 1


def eye(m, dtype=umatrix.ddtype):
    Z = zeros(m, m, dtype=dtype)
    for i in range(m):
        Z[i, i] = 1
    return Z

def det_inv(x):
    ''' Return (det(x) and inv(x))

        Operates on a copy of x
        Using elementary row operations convert X to an upper matrix
        the product of the diagonal = det(X)
        Continue to convert X to the identity matrix
        All the operation carried out on the original identity matrix
        makes it the inverse of X
    '''
    if not x.is_square:
        raise ValueError('Matrix must be square')
    else:
        # divide each row element by [0] to give a one in the first position
        # (may have to find a row to switch with if first element is 0)
        x = x.copy()
        inverse = eye(len(x), dtype=float)
        sign = 1
        factors = []
        p = 0
        while p < len(x):
            d = x[p, p]
            if abs(d) < umatrix.flt_eps:
                # pivot == 0 need to swap a row
                # check if swap row also has a zero at the same position
                np = 1
                while (p + np) < len(x) and abs(x[p + np, p]) < umatrix.flt_eps:
                    np += 1
                if (p + np) == len(x):
                    # singular
                    return [0, []]
                # swap rows
                z = x[p + np]
                x[p + np, :] = x[p]
                x[p, :] = z
                # do identity
                z = inverse[p + np]
                inverse[p + np, :] = inverse[p]
                inverse[p, :] = z
                # change sign of det
                sign = -sign
                continue
            factors.append(d)
            # change target row
            for n in range(p, len(x)):
                x[p, n] = x[p, n] / d
            # need to do the entire row for the inverse
            for n in range(len(x)):
                inverse[p, n] = inverse[p, n] / d
            # eliminate position in the following rows
            for i in range(p + 1, len(x)):
                # multiplier is that column entry
                t = x[i, p]
                for j in range(p, len(x)):
                    x[i, j] = x[i, j] - (t * x[p, j])
                for j in range(len(x)):
                    inverse[i, j] = inverse[i, j] - (t * inverse[p, j])
            p = p + 1
        s = sign
        for i in factors:
            s = s * i  # determinant
        # travel through the rows eliminating upper diagonal non-zero values
        for i in range(len(x) - 1):
            # final row should already be all zeros
            # except for the final position
            for p in range(i + 1, len(x)):
                # multiplier is that column entry
                t = x[i, p]
                for j in range(i + 1, len(x)):
                    x[i, j] = x[i, j] - (t * x[p, j])
                for j in range(len(x)):
                    inverse[i, j] = inverse[i, j] - (t * inverse[p, j])
        return (s, inverse)


def pinv(X):
    ''' Calculates the pseudo inverse Adagger = (A'A)^-1.A' '''
    Xt = X.transpose()
    d, Z = det_inv(dot(Xt, X))
    return dot(Z, Xt)


def dot(X, Y):
    ''' Dot product '''
    if X.size(2) == Y.size(1):
        Z = []
        for k in range(X.size(1)):
            for j in range(Y.size(2)):
                Z.append(sum([X[k, i] * Y[i, j] for i in range(Y.size(1))]))
        return umatrix.matrix(Z, cstride=1, rstride=Y.size(2))
    else:
        raise ValueError('shapes not aligned')


def cross(X, Y, axis=1):
    ''' Cross product
        axis=1 Numpy default
        axis=0 MATLAB, Octave, SciLab default
    '''
    if axis == 0:
        X = X.T
        Y = Y.T
    if (X.n in (2, 3)) and (Y.n in (2, 3)):
        if X.m == Y.m:
            Z = []
            for k in range(min(X.m, Y.m)):
                z = X[k, 0] * Y[k, 1] - X[k, 1] * Y[k, 0]
                if (X.n == 3) and (Y.n == 3):
                    Z.append([X[k, 1] * Y[k, 2] - X[k, 2] * Y[k, 1],
                              X[k, 2] * Y[k, 0] - X[k, 0] * Y[k, 2], z])
                else:
                    Z.append([z])
            if axis == 0:
                return umatrix.matrix(Z).T
            else:
                return umatrix.matrix(Z)
        else:
            raise ValueError('shape mismatch')
    else:
        raise ValueError('incompatible dimensions for cross product'
                         ' (must be 2 or 3)')

def eps(x = 0):
    # ref. numpy.spacing(), Octave/MATLAB eps() function
    if x:
        return 2**(math.floor(math.log(abs(x))/math.log(2)))*umatrix.flt_eps
    else:
        return umatrix.flt_eps
