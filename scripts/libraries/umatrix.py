'''

Part of the micro-linalg project to provide a small
matrix / linear algebra package for MicroPython (Python3)

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

import sys

stypes = [bool, int]
ddtype = int
estypes = []
flt_eps = 1


class matrix(object):

    def __init__(self, data, cstride=0, rstride=0, dtype=None):
        ''' Builds a matrix representation of 'data'.
            'data' can be a list (columns) of lists (rows)
            [[1,2,3],[4,5,6]] or
            a simple list organized as determined by rstride and cstride:
            [1,2,3,4,5,6] cstride=1, rstride=3.
            Elements will be of highest type included in 'data' or
            'dtype' can be used to force the type.
        '''
        if cstride != 0:
            if cstride == 1:
                self.n = rstride
                self.m = int(len(data) / self.n)
            else:
                self.m = cstride
                self.n = int(len(data) / self.m)
            self.cstride = cstride
            self.rstride = rstride
            self.data = data
        else:
            # else determine shape from list passed in
            self.n = 1
            if type(data) == int:
                self.m = 1
            else:  # it is a list
                self.m = len(data)
                # is data[0] a list
                if (type(data[0]) == list):
                    self.n = len(data[0])
            self.data = [data[i][j]
                         for i in range(self.m) for j in range(self.n)]
            self.cstride = 1
            self.rstride = self.n
        # ensure all elements are of the same type
        if dtype is None:
            self.dtype = stypes[max([stypes.index(type(i)) for i in self.data])]
        else:
            if dtype in stypes:
                self.dtype = dtype
            else:
                raise TypeError('unsupported type', dtype)
        self.data = [self.dtype(i) for i in self.data]

    def __len__(self):
        return self.m

    def __eq__(self, other):
        if self.shape == other.shape:
            res = all([self.data[i] == other.data[i] for i in range(self.size())])
            return res and (self.shape == other.shape)
        else:
            raise ValueError('shapes not equal')

    def __ne__(self, other):
        return not __eq__(other)

    def __iter__(self):
        self.cur = 0
        # determine proper axis
        if self.m == 1:
            self.cnt_lim = self.n
        else:
            self.cnt_lim = self.m
        return self

    def __next__(self):
        '''
        Returns a matrix if m > 1
        else the next numeric element of the vector.
        (Numpy returns vectors if selected via slice)
        '''
        if self.cur >= self.cnt_lim:
            raise StopIteration
        self.cur = self.cur + 1
        if self.m == 1:
            return self.data[self.cur - 1]
        else:
            return self[self.cur - 1]

    def slice_to_offset(self, r0, r1, c0, c1):
        # check values and limit them
        nd = [self.data[i * self.rstride + j * self.cstride]
              for i in range(r0, r1) for j in range(c0, c1)]
        return matrix(nd, cstride=1, rstride=(c1 - c0))

    def slice_indices(self, index, axis=0):
        # handles the unsupported slice.indices() method in uPy.
        # If implemented: 
        #     midx = index.indices(self.m)
        # should work.
        if isinstance(index.start, type(None)):
            s0 = 0
        else:
            s0 = min(int(index.start), self.shape[axis])
        if isinstance(index.stop, type(None)):
            p0 = self.shape[axis]
        else:
            p0 = min(int(index.stop), self.shape[axis])
        return (s0, p0)

    def __getitem__(self, index):
        if type(index) == tuple:
            # int and int
            # int and slice
            # slice and int
            # slice and slice
            if isinstance(index[0], int):
                s0 = index[0]
                p0 = s0 + 1
            else:  # row slice
                s0, p0 = self.slice_indices(index[0], 0)
            if isinstance(index[1], int):
                s1 = index[1]
                p1 = s1 + 1
            else:  # column slice
                s1, p1 = self.slice_indices(index[1], 1)
        elif type(index) == list:
            # list of indices etc
            raise NotImplementedError('Fancy indexing')
        else:
            # type is int? This will default to returning a row
            s0 = index
            p0 = s0 + 1
            s1 = 0
            p1 = self.n
        # resultant matrix
        z = self.slice_to_offset(s0, p0, s1, p1)
        # if it's a single entry then return that entry as int, float etc.
        if (p0 == s0 + 1) and (p1 == s1 + 1):
            return z.data[0]
        else:
            return z

    def __setitem__(self, index, val):
        if type(index) != tuple:
            # need to make it a slice without the slice function
            raise NotImplementedError('Need to use the slice [1,:] format.')
        # int and int => single entry gets changed
        # combinations of int and slice => row and columns take on elements from val
        if isinstance(index[0], int):
            s0 = index[0]
            p0 = s0 + 1
        else:  # slice
            s0, p0 = self.slice_indices(index[0], 0)
        if isinstance(index[1], int):
            s1 = index[1]
            p1 = s1 + 1
        else:  # slice
            s1, p1 = self.slice_indices(index[1], 1)
        if type(val) == matrix:
            val = val.data
        elif type(val) not in [list, tuple]:
            val = [val]
        if not all([type(i) in stypes for i in val]):
            raise ValueError('Non numeric entry')
        else:
            # assign list values wrapping as necessary to fill destination
            k = 0
            for i in range(s0, p0):
                for j in range(s1, p1):
                    self.data[i * self.rstride + j * self.cstride] = (self.dtype(val[k]))
                    k = (k + 1) % len(val)

    # there is also __delitem__

    # def __str__(self):
    def __repr__(self):
        # things that use __str__ will fallback to __repr__
        # find max string field size for formatting
        l = 0
        for i in self.data:
            l = max(l, len(repr(i)))
        s = 'mat(['
        r = 0
        for i in range(self.m):
            c = 0
            s = s + '['
            for j in range(self.n):
                s1 = repr(self.data[r + c])
                s = s + s1 + ' ' * (l - len(s1))
                if (j < (self.n - 1)):
                    s = s + ', '
                c = c + self.cstride
            if (i < (self.m - 1)):
                s = s + '],\n     '
            else:
                s = s + ']'
            r = r + self.rstride
        s = s + '])'
        return s

    # Reflected operations are not yet implemented in MicroPython
    # __rmul__ for example will not be invoked

    def __neg__(self):
        ndat =[self.data[i] * (-1) for i in range(len(self.data))]
        return matrix(ndat, cstride=self.cstride, rstride=self.rstride)

    def __do_op__(self, a, b, op):
        if op == '+':
            return (a + b)
        elif op == '-':
            return (a - b)
        elif op == '*':
            return (a * b)
        elif op == '**':
            return (a ** b)
        elif op == '/':
            try:
                return (a / b)
            except ZeroDivisionError:
                raise ZeroDivisionError('division by zero')
        elif op == '//':
            try:
                return (a // b)
            except ZeroDivisionError:
                raise ZeroDivisionError('division by zero')
        else:
            raise NotImplementedError('Unknown operator ', op)

    def __OP__(self, a, op):
        if type(a) in stypes:
            # matrix - scaler elementwise operation
            ndat = [self.__do_op__(self.data[i], a, op) for i in range(len(self.data))]
            return matrix(ndat, cstride=self.cstride, rstride=self.rstride)
        elif (type(a) == list):
            # matrix - list elementwise operation
            # hack - convert list to matrix and resubmit then it gets handled below
            # if self.n = 1 try transpose otherwise broadcast error to match numpy
            if (self.n == 1) and (len(a) == self.m):
                return self.__OP__(matrix([a]).T, op)
            elif len(a) == self.n:
                return self.__OP__(matrix([a]), op)
            else:
                raise ValueError('could not be broadcast')
        elif (type(a) == matrix):
            if (self.m == a.m) and (self.n == a.n):
                # matrix - matrix elementwise operation
                # use matrix indices to handle views
                ndat = [self.__do_op__(self[i, j], a[i, j], op) for i in range(self.m) for j in range(self.n)]
                return matrix(ndat, cstride=1, rstride=self.n)
            # generalize the following two elif for > 2 dimensions?
            elif (self.m == a.m):
                # m==m n!=n => column-wise row operation
                Y = self.copy()
                for i in range(self.n):
                    # this call _OP_ once for each row and __do_op__ for each element
                    for j in range(self.m):
                        Y[j, i] = self.__do_op__(Y[j, i], a[j, 0], op)
                return Y
            elif (self.n == a.n):
                # m!=m n==n => row-wise col operation
                Y = self.copy()
                for i in range(self.m):
                    # this call _OP_ once for each col and __do_op__ for each element
                    for j in range(self.n):
                        Y[i, j] = self.__do_op__(Y[i, j], a[0, j], op)
                return Y
            else:
                raise ValueError('could not be broadcast')
        raise NotImplementedError('__OP__ matrix + ', type(a))

    def __add__(self, a):
        ''' matrix - scaler elementwise addition'''
        return self.__OP__(a, '+')

    def __radd__(self, a):
        ''' scaler - matrix elementwise addition'''
        ''' commutative '''
        return self.__add__(a)

    def __sub__(self, a):
        ''' matrix - scaler elementwise subtraction '''
        if type(a) in estypes:
            return self.__add__(-a)
        raise NotImplementedError('__sub__ matrix -', type(a))

    def __rsub__(self, a):
        ''' scaler - matrix elementwise subtraction '''
        self = -self
        return self.__add__(a)

    def __mul__(self, a):
        ''' matrix scaler elementwise multiplication '''
        return self.__OP__(a, '*')

    def __rmul__(self, a):
        ''' scaler * matrix elementwise multiplication
            commutative
        '''
        return self.__mul__(a)

    def __truediv__(self, a):
        ''' matrix / scaler elementwise division '''
        return self.__OP__(a, '/')

    def __rtruediv__(self, a):
        ''' scaler / matrix elementwise division '''
        return self.__OP__(a, '/')

    def __floordiv__(self, a):
        ''' matrix // scaler elementwise integer division '''
        return self.__OP__(a, '//')

    def __rfloordiv__(self, a):
        ''' scaler // matrix elementwise integer division '''
        return self.__OP__(a, '//')

    def __pow__(self, a):
        ''' matrix ** scaler elementwise power '''
        return self.__OP__(a, '**')

    def __rpow__(self, a):
        ''' scaler ** matrix elementwise power '''
        return self.__OP__(a, '**')

    def copy(self):
        """ Return a copy of matrix, not just a view """
        return matrix([i for i in self.data],
                      cstride=self.cstride, rstride=self.rstride)

    def size(self, axis=0):
        """ 0 entries
            1 rows
            2 columns
        """
        return [self.m * self.n, self.m, self.n][axis]

    @property
    def shape(self):
        return (self.m, self.n)

    @shape.setter
    def shape(self, nshape):
        """ check for proper length """
        if (nshape[0] * nshape[1]) == self.size():
            self.m, self.n = nshape
            self.cstride = 1
            self.rstride = self.n
        else:
            raise ValueError('total size of new matrix must be unchanged')
        return self

    @property
    def is_square(self):
        return self.m == self.n

    def reshape(self, nshape):
        """ check for proper length """
        X = self.copy()
        X.shape = nshape
        return X

    @property
    def T(self):
        return self.transpose()

    def transpose(self):
        """ Return a view """
        X = matrix(self.data, cstride=self.rstride, rstride=self.cstride)
        if self.cstride == self.rstride:
            # handle column vector
            X.shape = (self.n, self.m)
        return X

    def reciprocal(self, n=1):
        return matrix([n / i for i in self.data], cstride=self.cstride, rstride=self.rstride)

    def apply(self, func, *args, **kwargs):
        """ call a scalar function on each element, returns a new matrix
        passes *args and **kwargs to func unmodified
        note: this is not useful for matrix-matrix operations
        e.g.
            y = x.apply(math.sin)
            y = x.apply(lambda a,b: a>b, 5) # equivalent to y = x > 5
            y = x.apply(operators.gt, 5)    # equivalent to y = x > 5 (not in micropython)
        """
        return matrix([func(i, *args, **kwargs) for i in self.data],
                      cstride=self.cstride, rstride=self.rstride)

def matrix_isclose(x, y, rtol=1E-05, atol=flt_eps):
    ''' Returns a matrix indicating equal elements within tol'''
    for i in range(x.size()):
        try:
            data = [abs(x.data[i] - y.data[i]) <= atol+rtol*abs(y.data[i]) for i in range(len(x.data))]
        except (AttributeError, IndexError):
            data = [False for i in range(len(x.data))]
    return matrix(data, cstride=x.cstride, rstride=x.rstride, dtype=bool)


def matrix_equal(x, y, tol=0):
    ''' Matrix equality test with tolerance same shape'''
    res = False
    if type(y) == matrix:
        if x.shape == y.shape:
            res = all([abs(x.data[i] - y.data[i]) <= tol for i in range(x.size())])
    return res


def matrix_equiv(x, y):
    ''' Returns a boolean indicating if X and Y share the same data and are broadcastable'''
    res = False
    if type(y) == matrix:
        if x.size() == y.size():
            res = all([x.data[i] == y.data[i] for i in range(len(x.data))])
    return res

def fp_eps():
    ''' Determine floating point resolution '''
    e = 1
    while 1 + e > 1:
        e = e / 2
    return 2 * e

flt_eps = fp_eps()
try:
    if sys.implementation.name == 'micropython' and sys.platform == 'linux':
        # force this as there seems to be some interaction with
        # some operations done using the C library with a smaller epsilon (doubles)
        flt_eps = 1.19E-7   # single precision IEEE 2**-23  double 2.22E-16 == 2**-52
except:
    pass
# Determine supported types
try:
    stypes.append(float)
    ddtype = float
except:
    pass
try:
    stypes.append(complex)
except:
    pass
# extended types
estypes = [matrix]
estypes.extend(stypes)
