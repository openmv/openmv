import math

def manhattan(a, b):
    s = 0
    for i in range(0, min(len(a), len(b))):
        s += abs(a[i] - b[i])
    return s

def euclidean(a, b):
    s = 0
    for i in range(0, min(len(a), len(b))):
        s += (a[i] - b[i])**2
    return math.sqrt(s)
