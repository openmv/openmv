@echo off
rem Sign the cashew firmware as an FSBL image and write it to the NUCLEO-N657X0-Q's external
rem flash, so it starts on its own at power-up.
rem   flash_cashew.bat [path\to\cashew_n6.bin]
rem Default image: ..\Release\cashew_n6.bin (STM32CubeIDE Release build), else ..\build\cashew_n6.bin.
rem Needs STM32CubeProgrammer 2.18 or newer. Set BOOT1 = 1 (JP2 position 2) and power-cycle first.
setlocal
if "%CUBEPROG%"=="" set "CUBEPROG=C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin"
set "LOADER=%CUBEPROG%\ExternalLoader\MX25UM51245G_STM32N6570-NUCLEO.stldr"
cd /d "%~dp0"
set "BIN=%~1"
if "%BIN%"=="" (
  if exist "..\Release\cashew_n6.bin" (set "BIN=..\Release\cashew_n6.bin") else (set "BIN=..\build\cashew_n6.bin")
)
if not exist "%BIN%" (
  echo no image: %BIN% ^(build the project first^)
  exit /b 1
)
set "OUT=%BIN:.bin=-trusted.bin%"

"%CUBEPROG%\STM32_SigningTool_CLI.exe" -bin "%BIN%" -nk -of 0x80000000 -t fsbl -hv 2.3 -align -s -o "%OUT%"
if errorlevel 1 exit /b 1
"%CUBEPROG%\STM32_Programmer_CLI.exe" -c port=SWD mode=HOTPLUG ap=1 -el "%LOADER%" -w "%OUT%" 0x70000000
if errorlevel 1 exit /b 1

echo Done. Now set BOOT1 = 0 (JP2 position 1), keep BOOT0 = 0, and power-cycle the board.
echo Open the ST-LINK virtual COM port at 921600 baud 8N1 to see the output.
