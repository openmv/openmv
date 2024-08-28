#!/usr/bin/python3
"""
   @brief Power Info decode and display

   __author__ onyettr
"""
# pylint: disable=unused-argument, invalid-name, consider-using-f-string
import struct
from isp_print import isp_print_color

# PCSMPSTATE [3:0]
PCSMP_STATE_WARM_RESET  = 0x8
PCSMP_STATE_ON          = 0x8
PCSMP_STATE_DBG_RECOV   = 0x8
PCSMP_STATE_FUNC_RET    = 0x7
PCSMP_STATE_MEM_OFF     = 0x6
PCSMP_STATE_FULL_RET    = 0x5
PCSMP_STATE_LOGIC_RET   = 0x4
PCSMP_STATE_MEM_RET_EMU = 0x3
PCSMP_STATE_MEM_RET     = 0x2
PCSMP_STATE_OFF_EMU     = 0x1
PCSMP_STATE_OFF         = 0x0

# DEVPSTATE[3:0]/PPU_PWPR.PWR_POLICY
DEVPSTATE_STATE_DBG_RECOV   = 0xA
DEVPSTATE_STATE_WARM_RESET  = 0x9
DEVPSTATE_STATE_ON          = 0x8
DEVPSTATE_STATE_FUNC_RET    = 0x7
DEVPSTATE_STATE_MEM_OFF     = 0x6
DEVPSTATE_STATE_FULL_RET    = 0x5
DEVPSTATE_STATE_LOGIC_RET   = 0x4
DEVPSTATE_STATE_MEM_RET_EMU = 0x3
DEVPSTATE_STATE_MEM_RET     = 0x2
DEVPSTATE_STATE_OFF_EMU     = 0x1
DEVPSTATE_STATE_OFF         = 0x0

def ppu_status_to_string(ppu_status):
    """
        parse the bit fields of the ppu status and return a printable string
    """
    status_string = ''
    if ppu_status == DEVPSTATE_STATE_FUNC_RET:
        status_string += " FUNC_RET"
    if ppu_status == DEVPSTATE_STATE_MEM_OFF:
        status_string += " MEM_OFF"
    if ppu_status == DEVPSTATE_STATE_FULL_RET:
        status_string += " FULL_RET"
    if ppu_status == DEVPSTATE_STATE_ON:
        status_string += " ON"
    if ppu_status == DEVPSTATE_STATE_MEM_RET:
        status_string += " MEM_RET"
    if ppu_status == DEVPSTATE_STATE_OFF_EMU:
        status_string += " ON"  # OFF EMU"
    if ppu_status == DEVPSTATE_STATE_OFF:
        status_string += " OFF"

    return status_string

def display_cpu_ppu_info(se_curr, se_prev,
                     a32_0_curr, a32_0_prev,
                     a32_1_curr, a32_1_prev,
                     m55_hp_curr, m55_hp_prev,
                     m55_he_curr, m55_he_prev):

    """ display_cpu_ppu_info

        - show CPU PPU info. 
    """
    isp_print_color("blue","CPU PPU Status\n")
    isp_print_color("blue","+--------+----------+----------+\n")
    isp_print_color("blue","|  CPU   | Current  | Previous |\n")
    isp_print_color("blue","+--------+----------+----------+\n")
    isp_print_color("blue" if se_curr == se_prev else "green",
                    "| SE     |0x%08X|" % (se_curr));
    isp_print_color("blue" ,"0x%08X|\n" % (se_prev))
    
    isp_print_color("blue" if a32_0_curr == a32_0_prev else "green",
                    "| A32_0  |0x%08X|" % (a32_0_curr));
    isp_print_color("blue" ,"0x%08X|\n" % (a32_0_prev))
    
    isp_print_color("blue" if a32_1_curr == a32_1_prev else "green",
                    "| A32_1  |0x%08X|" % (a32_1_curr));
    isp_print_color("blue" ,"0x%08X|\n" % (a32_1_prev))

    isp_print_color("blue" if m55_hp_curr == m55_hp_prev else "green",
                    "| M55_HP |0x%08X|" % (m55_hp_curr));
    isp_print_color("blue" ,"0x%08X|\n" % (m55_hp_prev))

    isp_print_color("blue" if m55_he_curr == m55_hp_prev else "green",
                    "| M55_HE |0x%08X|" % (m55_he_curr));
    isp_print_color("blue" ,"0x%08X|\n" % (m55_he_prev))

    isp_print_color("blue","+--------+----------+----------+\n")

def display_ppu_info(register_string,ppu_register):
    """
        display the ppu info
    """
    on_state_colour = "blue" 
    if (ppu_register == DEVPSTATE_STATE_ON or
        ppu_register == DEVPSTATE_STATE_OFF_EMU):
        on_state_colour = "green"
#   on_state_colour = "green" if (ppu_register == DEVPSTATE_STATE_ON ) else "blue"
    isp_print_color(on_state_colour,"%20s\t0x%08X\t%s\n" % 
                    (register_string,
                     ppu_register,
                     ppu_status_to_string(ppu_register)))

def power_decode_info(isp, message):
    """
        power_decode_info
    """
#    (se_curr, se_prev,) = struct.unpack("<II",bytes(message[0:9]))
#    (a32_0_curr, a32_0_prev,) = struct.unpack("<II",bytes(message[0:9]))
#    (a32_1_curr, a32_1_prev,) = struct.unpack("<II",bytes(message[0:9]))
#    (m55_hp_curr, m55_hp_prev,) = struct.unpack("<II",bytes(message[0:9]))
#    (m55_he_curr, m55_he_prev,) = struct.unpack("<II",bytes(message[0:9]))
#    (modem_curr, moem_prev,) = struct.unpack("<II",bytes(message[0:9]))

def display_power_info(message):
    """
        display power entries sent back from Target
        message    -- ISP data to parse
    """
    if len(message) < 50:
        print("[ERROR] Not enough power info to print %d\n" % 
              len(message))

        return

    # Unpack the power_status_t::PPU from SERAM
    (se_curr, se_prev,) = struct.unpack("<II",bytes(message[0:8]))
    (a32_0_curr, a32_0_prev,) = struct.unpack("<II",bytes(message[8:16]))
    (a32_1_curr, a32_1_prev,) = struct.unpack("<II",bytes(message[16:24]))
    (m55_hp_curr, m55_hp_prev,) = struct.unpack("<II",bytes(message[24:32]))
    (m55_he_curr, m55_he_prev,) = struct.unpack("<II",bytes(message[32:40]))

    (vbat_power_status,) = struct.unpack("<I",bytes(message[48:52]))

    (es0_ppu_status,es1_ppu_status,
    se_ppu_status,fw_ppu_status,
    systop_ppu_status,dbgtop_ppu_status,
    clustop_ppu_status,a32_0_ppu_status,
    a32_1_ppu_status,modem_ppu_status,
    modem_aon_status,sse700_aon_status,) = struct.unpack("<IIIIIIIIIIII",
                                                        bytes(message[52:100]))
    display_ppu_info("es0_ppu_status    ",es0_ppu_status)
    display_ppu_info("es1_ppu_status    ",es1_ppu_status)
    display_ppu_info("se_ppu_status     ",se_ppu_status)
    display_ppu_info("fw_ppu_status     ",fw_ppu_status)
    display_ppu_info("systop_ppu_status ",systop_ppu_status)
    display_ppu_info("dbgtop_ppu_status ",dbgtop_ppu_status)
    display_ppu_info("clustop_ppu_status",clustop_ppu_status)
    display_ppu_info("a32_0_ppu_status  ",a32_0_ppu_status)
    display_ppu_info("a32_1_ppu_status  ",a32_1_ppu_status)
    display_ppu_info("modem_ppu_status  ",modem_ppu_status)
    display_ppu_info("modem_aon_status  ",modem_aon_status)
    display_ppu_info("sse700_aon_status ",sse700_aon_status)