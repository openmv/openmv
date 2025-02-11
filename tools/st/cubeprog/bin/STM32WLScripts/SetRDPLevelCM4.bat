set DIR_CUBE_P=C..\bin


:: Do pin reset to be sure OB have been considered after OB launch with SwBoot0 = 1
"%DIR_CUBE_P%\STM32_Programmer_CLI.exe" -c port=swd  ap=0 mode=UR

:: Enable C2BOOT
::"%DIR_CUBE_P%\STM32_Programmer_CLI.exe" -c port=swd  ap=0 mode=HOTPLUG -w32dbg 0x5800040C 0x00008000

"%DIR_CUBE_P%\STM32_Programmer_CLI.exe" -c port=swd  ap=0 mode=UR -halt -w32dbg 0x58004014 0xC0000000 -w32dbg 0x58004008 0x45670123 -w32dbg 0x58004008 0xCDEF89AB -w32dbg 0x5800400C 0x08192A3B -w32dbg 0x5800400C 0x4C5D6E7F -w32dbg 0x58004020 0x3FFFF1BB -w32dbg 0x58004014 0x00020000

:: Allow Flash and OB writing
"%DIR_CUBE_P%\STM32_Programmer_CLI.exe" -c port=swd  ap=0 mode=UR -halt -w32dbg 0x58004014 0xC0000000 -w32dbg 0x58004008 0x45670123 -w32dbg 0x58004008 0xCDEF89AB -w32dbg 0x5800400C 0x08192A3B -w32dbg 0x5800400C 0x4C5D6E7F -w32dbg 0x58004020 0x3FFFF0AA -w32dbg 0x58004014 0x00020000

:: Write CR (OBL_LAUNCH)
"%DIR_CUBE_P%\STM32_Programmer_CLI.exe" -c port=swd  ap=0 mode=UR -w32dbg 0x58004014 0x08020000

:: Do pin reset to be sure OB have been considered after OB launch with SwBoot0 = 1

