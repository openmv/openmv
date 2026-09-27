#!/bin/sh
# Sign the cashew firmware as an FSBL image and write it to the NUCLEO-N657X0-Q's external
# flash, so it starts on its own at power-up.
#   ./flash_cashew.sh [path/to/cashew_n6.bin]
# Default image: ../Release/cashew_n6.bin (STM32CubeIDE Release build), else ../build/cashew_n6.bin (make).
# Needs STM32CubeProgrammer 2.18 or newer. Set BOOT1 = 1 (JP2 position 2) and power-cycle first.
set -e
CUBEPROG="${CUBEPROG:-$HOME/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin}"
LOADER="$CUBEPROG/ExternalLoader/MX25UM51245G_STM32N6570-NUCLEO.stldr"
cd "$(dirname "$0")"
BIN="$1"
if [ -z "$BIN" ]; then
    if [ -f ../Release/cashew_n6.bin ]; then BIN=../Release/cashew_n6.bin; else BIN=../build/cashew_n6.bin; fi
fi
[ -f "$BIN" ] || { echo "no image: $BIN (build the project first)"; exit 1; }
OUT="${BIN%.bin}-trusted.bin"

"$CUBEPROG/STM32_SigningTool_CLI" -bin "$BIN" -nk -of 0x80000000 -t fsbl -hv 2.3 -align -s -o "$OUT"
"$CUBEPROG/STM32_Programmer_CLI" -c port=SWD mode=HOTPLUG ap=1 -el "$LOADER" -w "$OUT" 0x70000000

echo "Done. Now set BOOT1 = 0 (JP2 position 1), keep BOOT0 = 0, and power-cycle the board."
echo "Open the ST-LINK virtual COM port at 921600 baud 8N1 to see the output."
