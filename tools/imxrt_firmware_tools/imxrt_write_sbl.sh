#!/bin/bash
export "sdphost_connect=-u 0x1FC9,0x0135"
export "set_sdphost_baud_rate=115200"
export "blhost_connect=-u 0x15A2,0x0073"
export "erase_all=1"
export "write_fcb=1"
export "sdphost=./sdphost/linux/amd64/sdphost"
export "blhost=./blhost/linux/amd64/blhost"

export "flashloader=unsigned_MIMXRT1060_flashloader.bin"
export "sbl_image=evkbmimxrt1060_flashloader_nopadding.bin"
export "sbl_size=$(stat -c %s $sbl_image)"

echo "### Load FlashLoader ###"
"$sdphost" $sdphost_connect -- write-file 0x20001C00 "$flashloader"
if [ $? -ge 2 ]; then
    exit 2
fi

echo "### Start FlashLoader ###"
"$sdphost" $sdphost_connect -- jump-address 0x20001C00
if [ $? -ge 2 ]; then
    exit 2
fi

echo "### Waiting FlashLoader to be initialized ###"
sleep 3
"$blhost" $blhost_connect -t 5000 -- get-property 1 0
if [ $? -ge 2 ]; then
    exit 2
fi

echo "### Configure FlexSPI NOR memory using options on address 0x2000 ###"
"$blhost" $blhost_connect -- fill-memory 0x2000 4 0xC0000008 word #133MHz
if [ $? -ge 2 ]; then
    exit 2
fi
"$blhost" $blhost_connect -- configure-memory 9 0x2000
if [ $? -ge 2 ]; then
    exit 2
fi

echo "### Erase memory before writing image ###"
if [ "$erase_all" = "1" ]; then
    "$blhost" $blhost_connect -t 100000 -- flash-erase-all 9
else
    "$blhost" $blhost_connect -t 100000 -- flash-erase-region 0x60000000 $sbl_size 9
fi
if [ $? -ge 2 ]; then
    exit 2
fi

if [ "$write_fcb" = "1" ]; then
# FCB is embedded in image
echo "### Create Flash Configuration Block (FCB) using option on address 0x2000 ###"
"$blhost" $blhost_connect -- fill-memory 0x2000 4 0xF000000F word
if [ $? -ge 2 ]; then
    exit 2
fi
"$blhost" $blhost_connect -- configure-memory 9 0x2000
if [ $? -ge 2 ]; then
    exit 2
fi
fi

echo "### Write image ###"
"$blhost" $blhost_connect -- write-memory 0x60001000 "$sbl_image"

if [ $? -ge 2 ]; then
    exit 2
fi

echo "### Reset ###"
"$blhost" $blhost_connect -- reset
