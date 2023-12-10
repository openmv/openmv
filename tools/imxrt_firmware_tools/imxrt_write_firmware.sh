#!/bin/bash
export "blhost_connect=-u 0x15A2,0x0073"
export "blhost=./blhost/linux/amd64/blhost"
export "firmware_image=../../src/build/bin/firmware.bin"
export "firmware_size=$(stat -c %s $firmware_image)"
export "firmware_addr=0x60040000"

echo "### Waiting FlashLoader to be initialized ###"
sleep 0.5
"$blhost" $blhost_connect -t 5000 -- get-property 1 0
if [ $? -ge 2 ]; then
    exit 2
fi

echo "### Erase memory before writing image ###"
"$blhost" $blhost_connect -t 60000 -- flash-erase-region $firmware_addr $firmware_size 9
if [ $? -ge 2 ]; then
    exit 2
fi

echo "### Write firmware image ###"
"$blhost" $blhost_connect -- write-memory $firmware_addr $firmware_image
if [ $? -ge 2 ]; then
    exit 2
fi

echo "### Reset ###"
"$blhost" $blhost_connect -- reset
