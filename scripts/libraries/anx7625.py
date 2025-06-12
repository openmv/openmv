from machine import Pin
from machine import I2C
from time import sleep_ms
from micropython import const
import struct

TX_P0_ADDR = const(0x70)
RX_P0_ADDR = const(0x7e)
RX_P1_ADDR = const(0x84)
RX_P2_ADDR = const(0x54)
TCPC_CONFIG_ADDR = const(0x58)

XTAL_FRQ_SEL = const(0x3F)

PIXEL_CLOCK_L = const(0x25)
PIXEL_CLOCK_H = const(0x26)

VERTICAL_ACTIVE_LINES_L = const(0x14)
VERTICAL_ACTIVE_LINES_H = const(0x15)
VERTICAL_FRONT_PORCH = const(0x16)
VERTICAL_SYNC_WIDTH = const(0x17)
VERTICAL_BACK_PORCH = const(0x18)

HORIZONTAL_TOTAL_PIXELS_L = const(0x19)
HORIZONTAL_TOTAL_PIXELS_H = const(0x1A)
HORIZONTAL_ACTIVE_PIXELS_L = const(0x1B)
HORIZONTAL_ACTIVE_PIXELS_H = const(0x1C)
HORIZONTAL_FRONT_PORCH_L = const(0x1D)
HORIZONTAL_FRONT_PORCH_H = const(0x1E)
HORIZONTAL_SYNC_WIDTH_L = const(0x1F)
HORIZONTAL_SYNC_WIDTH_H = const(0x20)
HORIZONTAL_BACK_PORCH_L = const(0x21)
HORIZONTAL_BACK_PORCH_H = const(0x22)

R_DSC_CTRL_0 = const(0x40)
DSC_EN = const(0x01)

OCM_FW_VERSION = const(0x31)
OCM_FW_REVISION = const(0x32)

AP_AV_STATUS = const(0x28)
AP_MIPI_MUTE = const(1<<4)
AP_MIPI_RX_EN = const(1<<5)

SYSTEM_STATUS = const(0x45)
HPD_STATUS = const(1<<7)

MIPI_SWAP = const(0x4A)
MIPI_SWAP_CH3 = const(4)

MIPI_PHY_CONTROL_3 = const(0x03)
MIPI_CLK_RT_MANUAL_PD_EN = const(4)
MIPI_CLK_HS_MANUAL_PD_EN = const(3)

MIPI_LANE_CTRL_0 = const(0x05)
MIPI_VIDEO_STABLE_CNT = const(0x0A)

MIPI_LANE_CTRL_10 = const(0x0F)
MIPI_DIGITAL_ADJ_1 = const(0x1B)

MIPI_PLL_M_NUM_23_16 = const(0x1E)
MIPI_PLL_M_NUM_15_8 = const(0x1F)
MIPI_PLL_M_NUM_7_0 = const(0x20)

MIPI_PLL_N_NUM_23_16 = const(0x21)
MIPI_PLL_N_NUM_15_8 = const(0x22)
MIPI_PLL_N_NUM_7_0 = const(0x23)

MIPI_DIGITAL_PLL_6 = const(0x2A)
MIPI_M_NUM_READY = const(0x10)
MIPI_N_NUM_READY = const(0x08)

MIPI_DIGITAL_PLL_7 = const(0x2B)
MIPI_PLL_RESET_N = const(0x02)
MIPI_PLL_VCO_TUNE_REG_VAL = const(0x30)

MIPI_DIGITAL_PLL_8 = const(0x33)
MIPI_POST_DIV_VAL = const(4)

MIPI_DIGITAL_PLL_16 = const(0x3B)
MIPI_FREF_D_IND = const(1)
REF_CLK_27000kHz = const(1)

MIPI_DIGITAL_PLL_18 = const(0x3D)
MIPI_DPI_SELECT = const(5)
SELECT_DSI = const(1)

INTERFACE_CHANGE_INT = const(0x44)
INTERFACE_PD_MSG_SEND_BUF = const(0xc0)
INTERFACE_PD_MSG_RECV_BUF = const(0xe0)

TCPC_ROLE_CONTROL = const(0x1a)
AUTO_PD_MODE = const(0x2f)
TRY_SRC_EN = const(0x04)
TRY_SNK_EN = const(0x08)

MAX_VOLTAGE_SETTING_REG = const(0x29)
MAX_POWER_SETTING_REG = const(0x2a)
MIN_POWER_SETTING_REG = const(0x2b)

class ANX7625:
    def __init__(self, bus=1, pwr_pin="K2", rst_pin="J3", otg_pin="J6", det_pin="K3", int_pin="K4"):
        self.bus = I2C(bus, freq=400_000)
        self.pwr_pin = Pin(pwr_pin, Pin.OUT_PP, value=0)
        self.rst_pin = Pin(rst_pin, Pin.OUT_PP, value=0)
        self.otg_pin = Pin(otg_pin, Pin.IN, Pin.PULL_UP)
        self.int_pin = Pin(int_pin, Pin.IN, Pin.PULL_UP)
        self.det_pin = Pin(det_pin, Pin.IN, Pin.PULL_UP)

        self.i2c_buf = bytearray(1)
        self.msg_buf = memoryview(bytearray(32))
        self.reg_fix = { 0x70 : 0xD1, 0x7A : 0x60, 0x7E : 0x39, 0x84 : 0x7F }
        self.last_addr = 0xFF
        self.pll_config = { 21363 : (14583808, 1228800, 15) }

        if self.otg_pin(): # VBUS off
            self.otg_pin = Pin(otg_pin, Pin.OUT_PP, value=1)
            sleep_ms(1000) # Wait for VBUS to discharge
        else:
            raise Exception("VBUS is being actively driven.")

        self.int_pin.irq(
            handler=self.int_callback, trigger=Pin.IRQ_FALLING, hard=False
        )

        self.det_pin.irq(
            handler=self.det_callback, trigger=Pin.IRQ_FALLING | Pin.IRQ_RISING, hard=False
        )

    def _reg_workaround(self, addr):
        if addr != self.last_addr:
            reg = self.reg_fix.get(addr, 0)

            for i in range(5):
                try:
                    self.bus.writeto_mem(addr >> 1, reg, bytes([0]))
                    break
                except Exception as e:
                    sleep_ms(10)

            self.last_addr = addr

    def _read_reg(self, addr, reg):
        self._reg_workaround(addr)
        self.bus.readfrom_mem_into(addr >> 1, reg, self.i2c_buf)
        return self.i2c_buf[0]

    def _write_reg(self, addr, reg, val=0x00, mask=0x00, buf=None):
        self._reg_workaround(addr)
        if mask:
            val |= mask & self._read_reg(addr, reg)
        if buf is None:
            self.i2c_buf[0] = val & 0xFF
        self.bus.writeto_mem(addr >> 1, reg, self.i2c_buf if buf is None else buf)

    def _read_status(self):
        val = self._read_reg(RX_P0_ADDR, SYSTEM_STATUS)
        print("VCONN status:", "ON" if val & (1<<2) else "OFF")
        print("VBUS power:", "provider" if val & (1<<3) else "consumer")
        print("Data Role:", "DFP" if val & (1<<5) else "UFP")
        print("DP HPD:", "high\n" if val & (1<<7) else "low\n")
        return val

    def _send_msg(self, msg_type, msg_buf=None):
        msg_len = len(msg_buf)
        struct.pack_into(f"BB{msg_len}s", self.msg_buf, 0, msg_len + 1, msg_type, msg_buf)
        checksum = 0 - sum(self.msg_buf[0:msg_len])
        struct.pack_into("b", self.msg_buf, msg_len + 2, checksum)

        while True:
            val = self._read_reg(RX_P0_ADDR, INTERFACE_PD_MSG_SEND_BUF)
            if val == 0:
                break
            print("Waiting to send message send_buf=", val)
            sleep_ms(10)

        self._write_buf(RX_P0_ADDR, INTERFACE_PD_MSG_SEND_BUF + 1, self.msg_buf[1:msg_len+2])
        self._write_reg(RX_P0_ADDR, INTERFACE_PD_MSG_SEND_BUF, msg_len+1)

    def _recv_msg(self):
        msg_size = self._read_reg(RX_P0_ADDR, INTERFACE_PD_MSG_RECV_BUF)
        print("Message len=", msg_size)
        for i in range(msg_size):
            val = self._read_reg(RX_P0_ADDR, INTERFACE_PD_MSG_RECV_BUF + 1 + i)
            print("0x%x, "%(val), end="")
        print("")

    def power_off(self):
        self.rst_pin(0)
        sleep_ms(1)
        self.pwr_pin(0)
        sleep_ms(1)

    def power_on(self):
        self.pwr_pin(1)
        sleep_ms(10)
        self.rst_pin(1)
        sleep_ms(10)

        # Program XTAL frequency
        self._write_reg(RX_P0_ADDR, XTAL_FRQ_SEL, (4<<5))

        # Restart OCM
        self._write_reg(RX_P0_ADDR, 0x88, 0x40)
        self._write_reg(RX_P0_ADDR, 0x88, 0x00)

        # Wait for OCM to reset
        for i in range(10):
            if self._read_reg(RX_P0_ADDR, 0x05) & 0x80:
                break
            if i == 9:
                raise Exception("Failed to reset OCM")
            sleep_ms(100)

        # Clear IRQ mask
        self._write_reg(RX_P0_ADDR, 0x43, 0x00)
        # Enable auto PD
        self._write_reg(RX_P0_ADDR, AUTO_PD_MODE, (1<<1), mask=0xFF)
	    # Maximum voltage in 100mV units (5000mV)
        self._write_reg(RX_P0_ADDR, MAX_VOLTAGE_SETTING_REG, 0x32)
	    # Maximum Power in 500mW units (1000mW)
        self._write_reg(RX_P0_ADDR, MAX_POWER_SETTING_REG, 0x02)
	    # Minimum Power in 500mW units (500mW)
        self._write_reg(RX_P0_ADDR, MIN_POWER_SETTING_REG, 0x01)
        # Enable try sink/source feature
        self._write_reg(RX_P0_ADDR, AUTO_PD_MODE, mask=~(TRY_SRC_EN | TRY_SNK_EN))
        # Disable DRP
        self._write_reg(RX_P0_ADDR, TCPC_ROLE_CONTROL, mask=~(1<<6));

        # Read firmware version
        print("Firmware Version: ", self._read_reg(RX_P0_ADDR, OCM_FW_VERSION))
        print("Firmware Revision: ", self._read_reg(RX_P0_ADDR, OCM_FW_REVISION))
        print("Waiting for hdmi hot plug event...")

        while True:
            if self._read_status() & HPD_STATUS:
                print("HDMI hot plug event")
                self._write_reg(RX_P1_ADDR, 0xee, mask=0x9f)
                self._write_reg(RX_P1_ADDR, 0xec, 0x10, mask=0xFF)
                self._write_reg(RX_P1_ADDR, 0xff, 0x01, mask=0xFF)
                val = self._read_reg(RX_P1_ADDR, 0x86)
                return True
            sleep_ms(500)
        return False

    def init(self, dc, dt):
        while not self.det_pin():
            print("Waiting for cable DET")
            sleep_ms(500)

        self.power_on()

        # DSC disable
        self._write_reg(RX_P0_ADDR, R_DSC_CTRL_0, mask=~DSC_EN);
        # Swap MIPI-DSI data lane 3 P and N
        self._write_reg(RX_P1_ADDR, MIPI_SWAP, (1 << MIPI_SWAP_CH3), mask=0xFF)
        # Lane count
        self._write_reg(RX_P1_ADDR, MIPI_LANE_CTRL_0, 1, mask=0xfc)
        # DSI clock settings
        self._write_reg(RX_P1_ADDR, MIPI_PHY_CONTROL_3,
                        (1 << MIPI_CLK_RT_MANUAL_PD_EN) | (1 << MIPI_CLK_HS_MANUAL_PD_EN))
        # Enable DSI mode
        self._write_reg(RX_P1_ADDR, MIPI_DIGITAL_PLL_18, SELECT_DSI << MIPI_DPI_SELECT, mask=0xFF)

        # Configure DSI video
        self._video_config(dt);
        
        # Toggle M/N ready
        self._write_reg(RX_P1_ADDR, MIPI_DIGITAL_PLL_6, mask=~(MIPI_M_NUM_READY | MIPI_N_NUM_READY))
        sleep_ms(1);
        self._write_reg(RX_P1_ADDR, MIPI_DIGITAL_PLL_6, (MIPI_M_NUM_READY | MIPI_N_NUM_READY), mask=0xFF)

        # Configure integer stable register
        self._write_reg(RX_P1_ADDR, MIPI_VIDEO_STABLE_CNT, 0x02)

        # Power on MIPI RX
        self._write_reg(RX_P1_ADDR, MIPI_LANE_CTRL_10, 0x00)
        self._write_reg(RX_P1_ADDR, MIPI_LANE_CTRL_10, 0x80)

        # Enable MIPI
        self._write_reg(RX_P0_ADDR, AP_AV_STATUS, AP_MIPI_RX_EN, mask=~AP_MIPI_MUTE);

    def _video_config(self, dt):
        if dt["pixel_clock"] not in self.pll_config:
            raise Exception("Pixel clock not supported")
        m, n, div = self.pll_config[dt["pixel_clock"]]

        # Display timings
        self._write_reg(RX_P0_ADDR, PIXEL_CLOCK_L, (dt["pixel_clock"] // 1000) & 0xFF)
        self._write_reg(RX_P0_ADDR, PIXEL_CLOCK_H, (dt["pixel_clock"] // 1000) >> 8)

        self._write_reg(RX_P2_ADDR, VERTICAL_ACTIVE_LINES_L, dt["vactive"])
        self._write_reg(RX_P2_ADDR, VERTICAL_ACTIVE_LINES_H, dt["vactive"] >> 8)
        self._write_reg(RX_P2_ADDR, VERTICAL_FRONT_PORCH, dt["vfront_porch"])
        self._write_reg(RX_P2_ADDR, VERTICAL_SYNC_WIDTH, dt["vsync_len"])
        self._write_reg(RX_P2_ADDR, VERTICAL_BACK_PORCH, dt["vback_porch"])
 
        htotal = dt["hactive"] + dt["hfront_porch"] + dt["hback_porch"] + dt["hsync_len"]
        self._write_reg(RX_P2_ADDR, HORIZONTAL_TOTAL_PIXELS_L, htotal & 0xFF)
        self._write_reg(RX_P2_ADDR, HORIZONTAL_TOTAL_PIXELS_H, htotal >> 8)
        self._write_reg(RX_P2_ADDR, HORIZONTAL_ACTIVE_PIXELS_L, dt["hactive"] & 0xFF)
        self._write_reg(RX_P2_ADDR, HORIZONTAL_ACTIVE_PIXELS_H, dt["hactive"] >> 8)
        self._write_reg(RX_P2_ADDR, HORIZONTAL_FRONT_PORCH_L, dt["hfront_porch"])
        self._write_reg(RX_P2_ADDR, HORIZONTAL_FRONT_PORCH_H, dt["hfront_porch"] >> 8)
        self._write_reg(RX_P2_ADDR, HORIZONTAL_SYNC_WIDTH_L, dt["hsync_len"])
        self._write_reg(RX_P2_ADDR, HORIZONTAL_SYNC_WIDTH_H, dt["hsync_len"] >> 8)
        self._write_reg(RX_P2_ADDR, HORIZONTAL_BACK_PORCH_L, dt["hback_porch"])
        self._write_reg(RX_P2_ADDR, HORIZONTAL_BACK_PORCH_H, dt["hback_porch"] >> 8)

        # M value
        self._write_reg(RX_P1_ADDR, MIPI_PLL_M_NUM_23_16, (m >> 16) & 0xff)
        self._write_reg(RX_P1_ADDR, MIPI_PLL_M_NUM_15_8, (m >> 8) & 0xff)
        self._write_reg(RX_P1_ADDR, MIPI_PLL_M_NUM_7_0, (m & 0xff))
    
        # N value
        self._write_reg(RX_P1_ADDR, MIPI_PLL_N_NUM_23_16, (n >> 16) & 0xff)
        self._write_reg(RX_P1_ADDR, MIPI_PLL_N_NUM_15_8, (n >> 8) & 0xff)
        self._write_reg(RX_P1_ADDR, MIPI_PLL_N_NUM_7_0, (n & 0xff))
    
        # diff
        self._write_reg(RX_P1_ADDR, MIPI_DIGITAL_ADJ_1, 0x37)

        # Config input reference clock frequency 27MHz/19.2MHz
        self._write_reg(RX_P1_ADDR, MIPI_DIGITAL_PLL_16,
                            (REF_CLK_27000kHz << MIPI_FREF_D_IND), mask=~(1 << MIPI_FREF_D_IND))
        # Config post divider
        self._write_reg(RX_P1_ADDR, MIPI_DIGITAL_PLL_8, (div - 1) << MIPI_POST_DIV_VAL, mask=0x0f)
        # Patch for MIS2-125 (5pcs ANX7625 fail ATE MBIST test)
        self._write_reg(RX_P1_ADDR, MIPI_DIGITAL_PLL_7, mask=~MIPI_PLL_VCO_TUNE_REG_VAL)
        # Reset ODFC PLL
        self._write_reg(RX_P1_ADDR, MIPI_DIGITAL_PLL_7, MIPI_PLL_RESET_N, mask=~MIPI_PLL_RESET_N)
   
    def int_callback(self, pin):
        val = self._read_reg(RX_P0_ADDR, INTERFACE_CHANGE_INT)
        self._write_reg(RX_P0_ADDR, INTERFACE_CHANGE_INT, 0)
        print("INTP detected MASK=", hex(val))
        if val & (1<<0):
            print("RECVD_MSG_INT")
            self._recv_msg()
            self._write_reg(TCPC_CONFIG_ADDR, 0xCC, (1<<5))

    def det_callback(self, pin):
        #self._write_reg(RX_P0_ADDR, 0x44, 0x00)
        print("Cable detected DET=", self.det_pin())

import time
import image
import display
from machine import I2C
import time

#import sensor
#if __name__ == "__main__":
#    lcd = display.DSIDisplay(
#        framesize=display.VGA, portrait=False, refresh=60, controller=ANX7625()
#    )
#
#    sensor.reset()
#    sensor.set_pixformat(sensor.GRAYSCALE)
#    sensor.set_framesize(sensor.QVGA)
#
#    clock = time.clock()
#    while True:
#        clock.tick()
#        lcd.write(sensor.snapshot())

if __name__ == "__main__":
    img = image.Image("/flash/test.bmp")
    #img = image.Image(640, 480, image.RGB565)

    lcd = display.DSIDisplay(
        framesize=display.VGA, portrait=False, refresh=60, controller=ANX7625()
    )

    clock = time.clock()
    while True:
        clock.tick()  # Update the FPS clock.
        # Draw the image on the display.
        lcd.write(img)
        # print(clock.fps())
