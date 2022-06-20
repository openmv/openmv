import time

@micropython.asm_thumb
def asm():
    movw(r0, 42)

@micropython.viper
def viper(a, b):
    return a + b

@micropython.native
def native(a, b):
    return a + b


print(asm())
print(viper(1, 2))
print(native(1, 2))

