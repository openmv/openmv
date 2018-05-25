#! /usr/bin/env python

PLL_M = 6
PLL_N = 360
PLL_Q = 15
PLL_P = 4
HSE_VALUE = 12000000
PLL_VCO = (HSE_VALUE / PLL_M) * PLL_N
print ("PLL_VCO = %d"%PLL_VCO)
print ("SYSCLK  = %d"%(PLL_VCO / PLL_P))
print ("USB/SDIO/RNG =%d"%(PLL_VCO / PLL_Q))
