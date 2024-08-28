#!/usr/bin/python3
"""
   @brief Clock Info decode and display

   __author__ onyettr
"""
# pylint: disable=unused-argument, invalid-name, consider-using-f-string
import struct
import ctypes
from isp_print import isp_print_color

c_uint32 = ctypes.c_uint32

PLL_CLK_STATUS_XTAL_STARTED       = (1 << 0)
PLL_CLK_STATUS_PLL_LOCKED         = (1 << 1)
PLL_CLK_STATUS_SE_CLOCK_PLL       = (1 << 2)

CLOCK_FREQUENCY_800MHZ            = 0
CLOCK_FREQUENCY_400MHZ            = 1
CLOCK_FREQUENCY_300MHZ            = 2
CLOCK_FREQUENCY_200MHZ            = 3
CLOCK_FREQUENCY_160MHZ            = 4
CLOCK_FREQUENCY_120MHZ            = 5
CLOCK_FREQUENCY_80MHZ             = 6
CLOCK_FREQUENCY_60MHZ             = 7
CLOCK_FREQUENCY_100MHZ            = 8
CLOCK_FREQUENCY_50MHZ             = 9
CLOCK_FREQUENCY_20MHZ             = 10
CLOCK_FREQUENCY_10MHZ             = 11
CLOCK_FREQUENCY_76_8_RC_MHZ       = 12
CLOCK_FREQUENCY_38_4_RC_MHZ       = 13
CLOCK_FREQUENCY_76_8_XO_MHZ       = 14
CLOCK_FREQUENCY_38_4_XO_MHZ       = 15
CLOCK_FREQUENCY_DISABLED          = 16

clk_frequency_lut = {
    CLOCK_FREQUENCY_800MHZ : "800Mhz",
    CLOCK_FREQUENCY_400MHZ : "400Mhz",
    CLOCK_FREQUENCY_300MHZ : "300Mhz",
    CLOCK_FREQUENCY_200MHZ : "200Mhz",
    CLOCK_FREQUENCY_160MHZ : "160Mhz",
    CLOCK_FREQUENCY_120MHZ : "120Mhz",
    CLOCK_FREQUENCY_80MHZ : "80Mhz",
    CLOCK_FREQUENCY_60MHZ : "60Mhz",
    CLOCK_FREQUENCY_100MHZ : "100Mhz",
    CLOCK_FREQUENCY_50MHZ : "50Mhz",
    CLOCK_FREQUENCY_20MHZ : "20Mhz" ,
    CLOCK_FREQUENCY_10MHZ : "10Mhz",
    CLOCK_FREQUENCY_76_8_RC_MHZ : "76.8Mhz RC",
    CLOCK_FREQUENCY_38_4_RC_MHZ : "38.4Mhz RC",
    CLOCK_FREQUENCY_76_8_XO_MHZ : "76.8Mhz XO",
    CLOCK_FREQUENCY_38_4_XO_MHZ : "38.4Mhz XO",
    CLOCK_FREQUENCY_DISABLED : "Disabled"
}

register_names = [
  "HOSTCPUCLK_CTRL", "HOSTCPUCLK_DIV0", "HOSTCPUCLK_DIV1", "ACLK_CTRL", "ACLK_DIV0", 
  "OSC_CTRL", "PLL_LOCK_CTRL", "PLL_CLK_SEL", "ESCLK_SEL", "CLK_ENA", 
  "SYSTOP_CLK_DIV", "MISC_REG1", "XO_REG1", "PD4_CLK_SEL", "PD4_CLK_PLL",
  "MISC_CTRL", "DCDC_REG1", "DCDC_REG2", "VBAT_ANA_REG1", "VBAT_ANA_REG2"
]

class xo_reg1_bits(ctypes.LittleEndianStructure):
    _fields_ = [
        ("en_xtal"                       , c_uint32, 1),
        ("faststart"                     , c_uint32, 1),
        ("__reserved__0"                 , c_uint32, 1),
        ("en_bfr_clkpll"                 , c_uint32, 1),
        ("en_bfr_dig_2x"                 , c_uint32, 1),
        ("en_bfr_dig"                    , c_uint32, 1),
        ("boost"                         , c_uint32, 1),
        ("xtal_cap"                      , c_uint32, 4),
        ("gm_pfet"                       , c_uint32, 5),
        ("gm_nfet"                       , c_uint32, 5),
        ("sel_doubler_output_duty_cycle" , c_uint32, 2),
        ("sel_doubler_input_duty_cycle"  , c_uint32, 5),
        ("sel_ibg"                       , c_uint32, 1),
        ("ibres_cont"                    , c_uint32, 2),
        ("__reserved__1"                 , c_uint32, 1)
    ]
class XO_REG1(ctypes.Union):
    _fields_ = [("b", xo_reg1_bits), ("w", c_uint32)]

class vbat_ana_reg1_bits(ctypes.LittleEndianStructure):
    _fields_ = [
        ("osc_rc_32k_freq_cont"     , c_uint32, 4),
        ("ret_ldo_cont_3_0"         , c_uint32, 4),
        ("ret_ldo_vbat_en"          , c_uint32, 1),
        ("ret_ldo_vbat_shunt_en"    , c_uint32, 1),
        ("ret_ldo_vddmain_en"       , c_uint32, 1),
        ("ret_ldo_vdd_main_shunt_en", c_uint32, 1),
        ("xtal32k_en"               , c_uint32, 1),
        ("xtal32k_kick"             , c_uint32, 1),
        ("lpcomp_clk32k_en"         , c_uint32, 1),
        ("xtal32k_gm_cont"          , c_uint32, 4),
        ("xtal32k_cap_cont"         , c_uint32, 6),
        ("bor_en"                   , c_uint32, 1),
        ("bor_hyst"                 , c_uint32, 3),
        ("bor_thresh"               , c_uint32, 3)
    ]
class VBAT_ANA_REG1(ctypes.Union):
    _fields_ = [("b", vbat_ana_reg1_bits), ("w", c_uint32)]

class vbat_ana_reg2_bits(ctypes.LittleEndianStructure):
    _fields_ = [
        ("__reserved__"          , c_uint32, 1),
        ("pmubg_vref_cont"       , c_uint32, 4),
        ("dig_ldo_18_en"         , c_uint32, 1),
        ("dig_ldo_cont"          , c_uint32, 4),
        ("osc_76Mrc_cont_bit0"   , c_uint32, 1),
        ("osc_76M_div_cont_fast" , c_uint32, 3),
        ("osc_76Mrc_cont"        , c_uint32, 5),
        ("osc_76M_div_cont_slow" , c_uint32, 3),
        ("ana_periph_bg_ena"     , c_uint32, 1),
        ("ana_periph_LDO_en"     , c_uint32, 1),
        ("comp_lp_en"            , c_uint32, 1),
        ("comp_lp0_in_p_sel"     , c_uint32, 2),
        ("comp_lp0_in_m_sel"     , c_uint32, 2),
        ("comp_lp0_hyst"         , c_uint32, 3)
    ]

class VBAT_ANA_REG2(ctypes.Union):
    _fields_ = [("b", vbat_ana_reg2_bits), ("w", c_uint32)]


def clk_frequency_to_string(clk_frequency):
    """
    Convert input to string

    @param clk_frequency:
    @return: string based on input
    """

    return clk_frequency_lut.get(clk_frequency)

def clk_status_to_string(clk_status):
    """
    parse the bit fields and return a printable string
    @param clk_status: bit encoded values
    @return: string based on input
    """
    status_string = ''
    if clk_status & PLL_CLK_STATUS_XTAL_STARTED:
        status_string += "  HFXTAL STARTED\n"
    else:
        status_string += "  HFXTAL OFF\n"
    if clk_status & PLL_CLK_STATUS_PLL_LOCKED:
        status_string += "  PLL LOCKED\n"
    else:
        status_string += "  PLL OFF\n"
    if clk_status & PLL_CLK_STATUS_SE_CLOCK_PLL:
        status_string += "  SE CLOCK: PLL\n"
    else:
        status_string += "  SE CLOCK: HFRC\n"

    return status_string

def display_clock_info(message):
    """
        display clock, pll, xtal entries sent back from Target
        message    -- ISP data to parse
    """

    # word 0 is the status word
    (status,) = struct.unpack("<I",bytes(message[0:4]))
    isp_print_color("blue","Clock Status\n")
    isp_print_color("blue" ,"%s" % clk_status_to_string(status))
    # A32 frequency
    frequency = (status & 0x0000FF00) >> 8
    isp_print_color("blue" ,"CLK freq A32 %s\n" % (clk_frequency_to_string(frequency)))
    # M55-HP frequency
    frequency = (status & 0x00FF0000) >> 16
    isp_print_color("blue" ,"CLK freq ES0 %s\n" % (clk_frequency_to_string(frequency)))
    # M55-HE frequency
    frequency = (status & 0xFF000000) >> 24
    isp_print_color("blue" ,"CLK freq ES1 %s\n" % (clk_frequency_to_string(frequency)))

    # word 1 is the CM0+ frequency
    (cm0_freq,) = struct.unpack("<f",bytes(message[4:8]))
    isp_print_color("blue" ,"SE frequency %0.2fMHz\n" % cm0_freq)
    
    isp_print_color("blue" ,"\nRegisters:\n")
    for counter in range(len(register_names)):
        index = (counter + 2) * 4  # skip the first two words
        (value,) = struct.unpack("<I",bytes(message[index:index+4]))
        value_str = "{:<16}: 0x{:08X}".format(register_names[counter], value)
        isp_print_color("blue" , value_str)
        print()
        if register_names[counter] == 'XO_REG1':
            xo_reg1 = XO_REG1()
            xo_reg1.w = value
            value_str = "   xtal_cap:{:d}".format(xo_reg1.b.xtal_cap)
            value_str = value_str + "  gm_pfet:{:d}".format(xo_reg1.b.gm_pfet)
            value_str = value_str + "  gm_nfet:{:d}".format(xo_reg1.b.gm_nfet)
            print(value_str)
        elif register_names[counter] == 'VBAT_ANA_REG1':
            ana_reg1 = VBAT_ANA_REG1()
            ana_reg1.w = value
            value_str = "   osc_rc_32k_freq_cont:{:d}".format(ana_reg1.b.osc_rc_32k_freq_cont)
            value_str = value_str + "  xtal32k_en:{:d}".format(ana_reg1.b.xtal32k_en)
            value_str = value_str + "  xtal32k_gm_cont:{:d}".format(ana_reg1.b.xtal32k_gm_cont)
            value_str = value_str + "  xtal32k_cap_cont:{:d}".format(ana_reg1.b.xtal32k_cap_cont)
            value_str = value_str + "  bor_en:{:d}".format(ana_reg1.b.bor_en)
            value_str = value_str + "  bor_hyst:{:d}".format(ana_reg1.b.bor_hyst)
            value_str = value_str + "  bor_thresh:{:d}".format(ana_reg1.b.bor_thresh)
            print(value_str)
        elif register_names[counter] == 'VBAT_ANA_REG2':
            ana_reg2 = VBAT_ANA_REG2()
            ana_reg2.w = value
            value_str = "   pmubg_vref_cont:{:d}".format(ana_reg2.b.pmubg_vref_cont)
            value_str = value_str + "  osc_76Mrc_cont_bit0:{:d}".format(ana_reg2.b.osc_76Mrc_cont_bit0)
            value_str = value_str + "  osc_76M_div_cont_fast:{:d}".format(ana_reg2.b.osc_76M_div_cont_fast)
            value_str = value_str + "  osc_76Mrc_cont:{:d}".format(ana_reg2.b.osc_76Mrc_cont)
            value_str = value_str + "  osc_76M_div_cont_slow:{:d}".format(ana_reg2.b.osc_76M_div_cont_slow)
            print(value_str)


