::put to level 1
STM32_Programmer_CLI -c port=swd mode=UR -ob RDP=0xBB
::put to level 0
STM32_Programmer_CLI -c port=swd mode=UR -ob WRP1A_STRT=0x7F WRP1A_END=0x0 WRP1B_STRT=0x7F WRP1B_END=0x0 ESE=0x0 RDP=0xAA DDS=0x0 C1BOOTLOCK=0x0 C2BOOTLOCK=0x0