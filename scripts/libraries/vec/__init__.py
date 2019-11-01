def add(a, b):
    return [a[i] + b[i] for i in range(0, min(len(a), len(b)))]

def sub(a, b):
    return [a[i] - b[i] for i in range(0, min(len(a), len(b)))]

def mul(a, n):
    return [a[i] * n for i in range(0, len(a))]

def div(a, n):
    return [a[i] / n for i in range(0, len(a))]

def dot(a, b):
    s = 0
    for i in range(0, min(len(a), len(b))):
        s += a[i] * b[i]
    return s
