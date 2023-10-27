# This file is part of the OpenMV project.
#
# Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# ST7701 LCD controller driver.

import time


class ST7701:
    DSI_CMD2_BKX_SEL = const(0xFF)
    DSI_CMD2_BK0_SEL = const(0x10)
    DSI_CMD2_BK1_SEL = const(0x11)
    DSI_CMD2_BKX_SEL_NONE = const(0x00)

    DSI_CMD2_BK0_PVGAMCTRL = const(0xB0)
    DSI_CMD2_BK0_NVGAMCTRL = const(0xB1)
    DSI_CMD2_BK0_LNESET = const(0xC0)
    DSI_CMD2_BK0_PORCTRL = const(0xC1)
    DSI_CMD2_BK0_INVSEL = const(0xC2)

    DSI_CMD2_BK1_SECTRL = const(0xE0)
    DSI_CMD2_BK1_NRCTRL = const(0xE1)
    DSI_CMD2_BK1_SRPCTRL = const(0xE2)
    DSI_CMD2_BK1_CCCTRL = const(0xE3)
    DSI_CMD2_BK1_SKCTRL = const(0xE4)

    DSI_CMD2_BK1_VRHS = const(0xB0)
    DSI_CMD2_BK1_VCOM = const(0xB1)
    DSI_CMD2_BK1_VGHSS = const(0xB2)
    DSI_CMD2_BK1_TESTCMD = const(0xB3)
    DSI_CMD2_BK1_VGLS = const(0xB5)
    DSI_CMD2_BK1_PWCTLR1 = const(0xB7)
    DSI_CMD2_BK1_PWCTLR2 = const(0xB8)
    DSI_CMD2_BK1_DGMLUTR = const(0xB9)
    DSI_CMD2_BK1_SPD1 = const(0xC1)
    DSI_CMD2_BK1_SPD2 = const(0xC2)
    DSI_CMD2_BK1_MIPISET1 = const(0xD0)

    DCS_SOFT_RESET = const(0x01)
    DCS_EXIT_SLEEP_MODE = const(0x11)
    DCS_SET_DISPLAY_ON = const(0x29)

    def __init__(self):
        pass

    def init(self, dc):
        self.dc = dc

        # Soft-reset
        dc.dsi_write(DCS_SOFT_RESET)
        time.sleep_ms(200)

        # Exit sleep mode
        dc.dsi_write(DCS_EXIT_SLEEP_MODE)
        time.sleep_ms(800)

        # Select bank 0
        dc.dsi_write(DSI_CMD2_BKX_SEL, b"\x77\x01\x00\x00\x10")

        # Display Control setting
        dc.dsi_write(DSI_CMD2_BK0_LNESET, b"\x63\x00")
        dc.dsi_write(DSI_CMD2_BK0_PORCTRL, b"\x11\x02")
        dc.dsi_write(DSI_CMD2_BK0_INVSEL, b"\x01\x08")
        # dc.dsi_write(0xCC, b"\x18")
        # Gamma cluster settings
        dc.dsi_write(
            0xB0, b"\x40\xc9\x91\x0d\x12\x07\x02\x09\x09\x1f\x04\x50\x0f\xe4\x29\xdf"
        )
        dc.dsi_write(
            0xB1, b"\x40\xcb\xd0\x11\x92\x07\x00\x08\x07\x1c\x06\x53\x12\x63\xeb\xdf"
        )

        # Select bank 1
        dc.dsi_write(DSI_CMD2_BKX_SEL, b"\x77\x01\x00\x00\x11")

        # Power control settings
        dc.dsi_write(DSI_CMD2_BK1_VRHS, 0x65)
        dc.dsi_write(DSI_CMD2_BK1_VCOM, 0x34)
        dc.dsi_write(DSI_CMD2_BK1_VGHSS, 0x87)
        dc.dsi_write(DSI_CMD2_BK1_TESTCMD, 0x80)
        dc.dsi_write(DSI_CMD2_BK1_VGLS, 0x49)
        dc.dsi_write(DSI_CMD2_BK1_PWCTLR1, 0x85)
        dc.dsi_write(DSI_CMD2_BK1_PWCTLR2, 0x20)
        dc.dsi_write(DSI_CMD2_BK1_DGMLUTR, 0x10)
        dc.dsi_write(DSI_CMD2_BK1_SPD1, 0x78)
        dc.dsi_write(DSI_CMD2_BK1_SPD2, 0x78)
        dc.dsi_write(DSI_CMD2_BK1_MIPISET1, 0x88)
        time.sleep_ms(100)

        # GIP settings
        dc.dsi_write(DSI_CMD2_BK1_SECTRL, b"\x00\x00\x02")
        dc.dsi_write(
            DSI_CMD2_BK1_NRCTRL, b"\x08\x00\x0A\x00\x07\x00\x09\x00\x00\x33\x33"
        )
        dc.dsi_write(
            DSI_CMD2_BK1_SRPCTRL,
            b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        )
        dc.dsi_write(DSI_CMD2_BK1_CCCTRL, b"\x00\x00\x33\x33")
        dc.dsi_write(DSI_CMD2_BK1_SKCTRL, b"\x44\x44")
        dc.dsi_write(
            0xE5, b"\x0E\x60\xA0\xa0\x10\x60\xA0\xA0\x0A\x60\xA0\xA0\x0C\x60\xA0\xA0"
        )
        dc.dsi_write(0xE6, b"\x00\x00\x33\x33")
        dc.dsi_write(0xE7, b"\x44\x44")
        dc.dsi_write(
            0xE8, b"\x0D\x60\xA0\xA0\x0F\x60\xA0\xA0\x09\x60\xA0\xA0\x0B\x60\xA0\xA0"
        )

        dc.dsi_write(0xEB, b"\x02\x01\xE4\xE4\x44\x00\x40")
        dc.dsi_write(0xEC, b"\x02\x01")
        dc.dsi_write(
            0xED, b"\xAB\x89\x76\x54\x01\xFF\xFF\xFF\xFF\xFF\xFF\x10\x45\x67\x98\xBA"
        )

        dc.dsi_write(DSI_CMD2_BKX_SEL, b"\x77\x01\x00\x00\x00")
        time.sleep_ms(10)
        dc.dsi_write(DCS_SET_DISPLAY_ON, dcs=True)
        time.sleep_ms(200)

    def read_id(self):
        return self.dc.dsi_read(0x04, 3)
