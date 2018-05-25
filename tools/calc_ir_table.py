#! /usr/bin/env python
import sys

it = iter(l)

for i,j in zip(it, it):
    aij.append(i/2**39+j/2**32),

for i in range(0, len(aij)):
    sys.stdout.write("{0:.10e}f, ".format(aij[i]))
    if (i+1)%8==0:
        print()
