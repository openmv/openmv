#!/usr/bin/env python3
#pylint: disable=invalid-name,superfluous-parens,anomalous-unicode-escape-in-string
#pylint: disable=import-error,wrong-import-position,wildcard-import,undefined-variable
"""
    Maintenance mode tool

    - Uses synchronous ISP commands

     __author__ = "ronyett"
     __copyright__ = "ALIF Seminconductor"
     __version__ = "0.06.000"
     __status__ = "Dev"    "
"""

import os
import sys
import argparse
sys.path.append("./isp")
from serialport import serialPort # ISP Serial support
from serialport import COM_TIMEOUT_RX_DEFAULT
from isp_protocol import *        # ISP protocol constants
from isp_core import *
from isp_util import *
from isp_print import *
import device_probe
#from isp_print import isp_print_color, isp_print_cursor_enable
#from isp_print import isp_print_cursor_disable,isp_print_terminal_reset
from toc_decode import *                 # ISP TOC support
from power_decode import *               # ISP POWER support
from utils.config import *
from utils.user_validations import validateArgList
from recovery import recovery_action, recovery_action_no_reset


# Define Version constant for each separate tool
# 0.01.000    Concept+realisation
# 0.02.000    add get TOC
# 0.03.000    add get CPU status
# 0.04.000    add get MRAM contents
# 0.05.000    add get SERAM metrics and terminal mode
# 0.06.000    add ERASE app mram feature
# 0.07.000    add FAST ERASE app mram feature
# 0.07.001    Cursor reset after terminal mode
# 0.08.001    add groups for menu items
# 0.08.002    Cursor reset after Hard Maintenance mode
# 0.08.003    added Ctrl-C in showAndSelectOptions()
# 0.08.004    added python error exit code
# 0.09.000    added menu configurations
# 0.10.000    Integrated recoery mode
# 0.11.000    added get POWER
# 0.12.000    add command line option
# 0.13.000    get baudrate from DBs
# 0.14.000    add get clock data
# 0.15.000    add Fast Erase including NTOC
# 0.16.000    add Recovery MRAM_READ
# 0.17.000    add different menues for SEROM/SERAM stages
TOOL_VERSION = "0.17.000"

EXIT_WITH_ERROR   = 1
TARGET_RESPONDED  = 1
FAST_ERASE_SIZE   = 16

def reset_action(isp):
    """
        reset_action
            reset the device, send ISP request to reset
    """
    isp_reset(isp)

def get_revision_action(isp):
    """
        get device revision_action
            obtain the device revision info
    """
    isp_start(isp)
    isp_get_revision(isp)
    isp_stop(isp)

def get_toc_action(isp):
    """
        get toc action
            obtain the TOC info from target
    """
    isp_start(isp)
    isp_get_toc_data(isp)
    isp_stop(isp)

def get_power_action(isp):
    """
        get power action
            obtain the POWER info from target
    """
    isp_start(isp)
    isp_get_power_data(isp)
    isp_stop(isp)

def get_clock_action(isp):
    """
        get clock action
            obtain the Clock, XTal, PLL info from target
    """
    isp_start(isp)
    isp_get_clock_data(isp)
    isp_stop(isp)

def get_banner_action(isp):
    """
        get_banner_action
            obtain the SERAM banner
    """
    isp_start(isp)
    isp_get_banner(isp)
    isp_stop(isp)

def get_cpu_status_action(isp):
    """
        get cpu status_action
            obtain the cpu boot info
    """
    isp_start(isp)
    isp_get_cpu_status(isp)
    isp_stop(isp)

def get_enquiry_action(isp):
    """
        get_enquiry_action
            Enquire what is at the end of the cable
    """
    isp_start(isp)
    isp_enquiry(isp)
    isp_stop(isp)

def get_mram_info_action(isp):
    """
        get the real MRAM details
    """
    isp_start(isp)
    isp_get_mram_contents(isp)
    isp_stop(isp)

def get_seram_info_action(isp):
    """
        get_seram_info_action
            get seram metrics
    """
    isp_start(isp)
    isp_get_seram_metrics(isp)
    isp_stop(isp)

def get_mram_read_action(isp):
    """
        get MRAMvalue
    """
    offset = input('Enter hex word offset: ')
    try:
        offset_hex = int(offset,16)
    except EOFError:
        print("[Ctrl-C EOF]")
        sys.exit()
    except KeyboardInterrupt:
        print("[Ctrl-C]")
        sys.exit()
    except:
        print("[ERROR] not a valid offset")
        return

    isp_start(isp)
    isp_read_mram(isp,offset_hex)
    isp_stop(isp)

def get_otp_read(isp):
    """
        get OTP value
    """
    offset = input('Enter word addr(hex): ')
    try:
        offset_hex = int(offset,16)
    except EOFError:
        print("[Ctrl-C EOF]")
        sys.exit()
    except KeyboardInterrupt:
        print("[Ctrl-C]")
        sys.exit()
    except:
        print("[ERROR] not a valid hex offset")
        return

    isp_start(isp)
    isp_read_otp(isp,offset_hex)
    isp_stop(isp)

def get_address_action(isp):
    """
        get address value
    """
    address = input('Enter addr(hex): ')
    try:
        address_hex = int(address,16)
    except EOFError:
        print("[Ctrl-C EOF]")
        sys.exit()
    except KeyboardInterrupt:
        print("[Ctrl-C]")
        sys.exit()
    except:
        print("[ERROR] not a valid address")
        return

    isp_start(isp)
    isp_get_address(isp,address_hex)
    isp_stop(isp)

def set_address_action(isp):
    """
        set address value
    """
    address = input('Enter addr(hex): ')
    try:
        address_hex = int(address,16)
    except EOFError:
        print("[Ctrl-C EOF]")
        sys.exit()
    except KeyboardInterrupt:
        print("[Ctrl-C]")
        sys.exit()
    except:
        print("[ERROR] not a valid address")
        return

    data = input('Enter data(hex): ')
    try:
        data_hex = int(data,16)
    except EOFError:
        print("[Ctrl-C EOF]")
        sys.exit()
    except KeyboardInterrupt:
        print("[Ctrl-C]")
        sys.exit()
    except:
        print("[ERROR] not a valid address")
        return

    isp_set_address(isp,address_hex,data_hex)

def get_trace_buffer(isp):
    """
        get SEROM trace buffer
    """
    isp_start(isp)
    isp_get_trace_buffer(isp)
    isp_stop(isp)

def get_seram_trace_buffer(isp):
    """
        get SERAM trace buffer
    """
    isp_start(isp)
    try:
        isp_get_seram_trace_buffer(isp)
    except:
        print('[ERROR] Unable to get SERAM trace when SEROM is in recovery mode')
    isp_stop(isp)

def set_print_enable_action(isp):
    """
        set print enable
    """
    isp_set_print_enable(isp, True)

def set_print_disable_action(isp):
    """
        set print disable
    """
    isp_set_print_enable(isp, False)

def set_logger_enable_action(isp):
    """
        set Logger enable
    """
    isp_set_log_enable(isp, True)

def set_logger_disable_action(isp):
    """
        set Logger disable
    """
    isp_set_log_enable(isp, False)

def terminal_mode(isp):
    """
        terminal_mode
            Enter terminal mode, use [CTRL-C] to exit
    """
    ctrlc = CtrlCHandler()        # [CTRL-C] handling

    print("[TERMINAL] Ctrl-C to exit")
    going = True

    while going:
        output_line = isp.readSerial(ISP_MAXIMUM_PACKET_SIZE)
        raw_print = bytearray()
        for i in range(len(output_line)):
            raw_print.append(output_line[i])
        try:
            ascii_char = raw_print.decode('ascii')
            print(ascii_char,flush=True,end='')
            sys.stdout.flush()
        except UnicodeDecodeError:
            pass
        if ctrlc.Handler_exit():
            print("[INFO] CTRL-C")
            break

    print("[TERMINAL] Ends")
    isp_print_cursor_enable()
    isp_print_terminal_reset()

def application_mram_erase(isp, address, erase_length, reset):
    """
        erase_application_mram_action
            erase the Application area of MRAM based on the device details
    """
    global baud_rate

    erase_length_fmt = "{:,}".format(erase_length)
    pattern = 0x00000000

    print("[INFO] erasing 0x%x %s bytes" %(address,erase_length_fmt))

    put_target_in_maintenance_mode(isp, baud_rate,False)

    isp_start(isp)
    isp_mram_erase(isp,address,erase_length,pattern)
    isp_stop(isp)
    if reset:
        isp_reset(isp)


def full_erase_application_mram_action(isp):
    """
        full_erase_application_mram_action
            Full erase the Application area of MRAM based on the device details
    """
    global OEM_BASE_ADDRESS
    global OEM_MRAM_SIZE

    address = OEM_BASE_ADDRESS
    erase_length = OEM_MRAM_SIZE
    application_mram_erase(isp, address, erase_length, True)
    print("[INFO] Full Erase done")

def fast_erase_app_include_start_mram_action(isp):
    """
        fast_erase_app_include_start_mram_action
            Fast erase the Application area of MRAM (16 bytes) based on the device details
            Plus the beginning of MRAM (0x8000-0000)
    """
    global ALIF_BASE_ADDRESS
    global OEM_BASE_ADDRESS

    # erase APP TOC
    address = ALIF_BASE_ADDRESS - FAST_ERASE_SIZE
    erase_length = FAST_ERASE_SIZE
    # this function can't be called twice as target stops to respond
    application_mram_erase(isp, address, erase_length, False)

    # erase start of mram (remove residuals from applications that start at 0x8000-0000)
    address = OEM_BASE_ADDRESS
    erase_length = FAST_ERASE_SIZE
    # this function can't be called twice as target stops to respond
    application_mram_erase(isp, address, erase_length, True)

    print("[INFO] Fast Erase done (incl. NTOC)")

def fast_erase_application_mram_action(isp):
    """
        fast_erase_application_mram_action
            Fast erase the Application area of MRAM (16 bytes) based on the device details
    """
    global ALIF_BASE_ADDRESS

    address = ALIF_BASE_ADDRESS - FAST_ERASE_SIZE
    erase_length = FAST_ERASE_SIZE

    application_mram_erase(isp, address, erase_length, True)
    print("[INFO] Fast Erase done")

def process_log_data(log_data):
    """
        process_log_data
    """
    #print("log_data: ", log_data)
    max_index = log_data[0]
    head = log_data[1]
    index = log_data[2] # start from the tail
    data = log_data[3]
    ordered_data = []
    while True:
        if index == max_index:
            index = 0
        if index == head:
            break
        ordered_data.append(chr(data[index]))
        index += 1

    #print("ordered data: ", ordered_data)
    isp_print_color("blue", "........\n\n")
    display_str = ""
    for ch in ordered_data:
        if ch == '\00':
            if display_str != "":
                severity = display_str[0]
                if severity >= '0' and severity <= '9':
                    display_str = display_str[1:]
                isp_print_color("blue", display_str)
            display_str = ""
        else:
            display_str += ch

def get_log_data_action(isp):
    """
        get_log_data_action
    """
    isp_start(isp)
    log_data = isp_get_log_data(isp)
    if log_data != None:
        process_log_data(log_data)
    isp_stop(isp)

def maintenance_mode(isp):
    """
        maintenance_mode
            Enter Manitenance mode
    """
    global handler
    global baud_rate

    Flicks="|/-\\|/-\\"
    Flick = 0

    isp.setTimeout(0.1)
    isp.setBaudRate(baud_rate)

    isp_print_cursor_disable()

    print_count = 0
    se_chars = [b'S', b'E', b'P', b'C', b'T', b'L']
    se_index = 0
    expected_char = b'\0'
    while True:
        send_data = bytearray(b'ALIF')
        isp.writeSerial(send_data)
        sys.stdout.flush()

        recv_data = bytearray(isp.readSerial(1))

        print("Waiting for Target..[RESET Platform] " + Flicks[Flick] +
              '\r', flush=True, end="")
        sys.stdout.flush()

        # single character version
        #if (recv_data == b'\xF0'):
        #    break

        # looking for the sequence in se_chars
        if se_index == 0:
            if recv_data == se_chars[0]:
                # start of sequence
                se_index = 1
                expected_char = se_chars[se_index]
        elif recv_data != expected_char:
            # unexpected character, reset the sequence
            se_index = 0
            expected_char = b'\0'
        else:
            # received the expected character
            if se_index >= (len(se_chars) - 1):
                # the entire sequence is received
                break
            se_index += 1
            expected_char = se_chars[se_index]

        if handler.Handler_exit():
            print("\n[INFO] CTRL-C")
            break

        print_count = print_count + 1
        if print_count > 5: # "Fake" delay, time.sleep() will halt
            Flick = Flick + 1
            if Flick == 8:
                Flick -= 8
            print_count = 0

    send_data = bytearray(b'ALIF')
    isp.writeSerial(send_data)

    sys.stdout.flush()

    # switch back to post pll baud rate
    isp.setTimeout(COM_TIMEOUT_RX_DEFAULT)
    isp.setBaudRate(baud_rate)
    isp_print_cursor_enable()

def soft_maintenance_mode(isp):
    """
        soft_maintenance_mode
            enter Soft mode
    """
    global baud_rate

    put_target_in_maintenance_mode(isp, baud_rate,False)

def get_ecc_key(isp):
    """
        get_ecc_key from device
    """
    isp_start(isp)
    isp_get_ecc_key(isp)
    isp_stop(isp)

def get_firewall_config(isp):
    """
        get_firewall_config
            Pull back Firewall config from target
    """
    isp_start(isp)
    isp_get_firewall_config(isp)
    isp_stop(isp)

def show_help(supported_commands):
    """
        show_help
            display all supported commands
    """
    print("Supported commands:")
    for item in sorted(supported_commands):
        print('\t{}'.format(supported_commands[item][0]))

def maintenance_menu(supported_commands, isp):
    """
        maintenance_mode
            Show Menu options and get user input
    """

    option = 'x'
    while option == 'x':
        optList = []
        print('\nAvailable options:\n')
        i = 1
        for item in supported_commands:
            print("%2s - %s" % (str(i), item))
            i += 1
            optList.append(item)

        try:
            option = input('\nSelect an option (Enter to return): ')
        except EOFError:
            print("[Ctrl-C EOF]")
            sys.exit()
        except KeyboardInterrupt:
            print("[Ctrl-C]")
            sys.exit()

        if option == '':
            return

        try:
            idx = int(option)
        except ValueError:
            print('Invalid option - Please try again')
            option = 'x'
            continue

        if idx < 1 or idx > len(optList):
            print('Invalid option - Please try again')
            option = 'x'
            continue
        try:
            func = globals()[supported_commands[optList[idx-1]]]
        except ValueError:
            print("oops")
        func(isp)
        option = 'x'

def showAndSelectOptions(list):
    """
        showAndSelectOptions
    """
    option = 'x'
    while option == 'x':
        print('\nAvailable options:\n')
        i = 1
        optList = []
        for opt in list:
            print("%2s - %s" % (str(i), opt))
            i += 1
            optList.append(opt)
        try:
            option = input('\nSelect an option (Enter to exit): ')
        except EOFError:
            print("[Ctrl-C EOF]")
            sys.exit(0)

        if option == '':
            sys.exit(0)
        try:
            idx = int(option)
        except ValueError:
            print('[ERROR] Invalid option - Please try again')
            option = 'x'
            continue

        if idx < 1 or idx > len(list):
            print('[ERROR] Invalid option - Please try again')
            option = 'x'

    return optList[idx-1]

def read_json_file(file):
    """
        read_json_file
    """
    f = open(file, 'r')
    try:
        data = json.load(f)

    except JSONDecodeError as e:
        print("[ERROR] in JSON file.")
        print(str(e))
        sys.exit(EXIT_WITH_ERROR)

    except ValueError as v:
        print("[ERROR] in JSON file:")
        print(str(v))
        sys.exit(EXIT_WITH_ERROR)
    except:
        print("[ERROR] Unknown error loading JSON file")
        sys.exit(EXIT_WITH_ERROR)

    f.close()
    return data

def checkRestrictions(menu_item, menu_config):
    """
        checkRestrictions
    """
    if menu_item in menu_config.keys():
        for item in menu_config[menu_item]:
            if globals()[item] in menu_config[menu_item][item]:
                return True
    return False

def command_line_mode(isp,cli_option):
    """
        command_line_mode
        - operate on commands passed in rather than the menus
    """
    command_line_lut = {
            "sesbanner" : get_banner_action
            }

    if cli_option in command_line_lut:
        command_line_lut[cli_option](isp)
    else:
        print("[ERROR] Command %s not recognized" %cli_option)
        sys.exit(EXIT_WITH_ERROR)

def main():
    """
        Maintenance tool
    """
    global ALIF_BASE_ADDRESS
    global OEM_BASE_ADDRESS
    global OEM_MRAM_SIZE
    global DEVICE_FEATURE_SET
    global DEVICE_FAMILY
    global DEVICE_REVISION
    global DEVICE_PART_NUMBER
    global DEVICE_FEATURE_SET_REVISION
    global handler
    global baud_rate

    if sys.version_info.major == 2:
        print("[ERROR] You need Python 3 for this application!")
        sys.exit(EXIT_WITH_ERROR)

    # Deal with Command Line
    parser = argparse.ArgumentParser(description=
                                     'FUSION Maintenance Tool')
    parser.add_argument("-b", "--baudrate",
                        help="serial port baud rate",
                        type=int)
    parser.add_argument("-d" , "--discover", action='store_true',
                        default=False, help="COM port discovery")
    parser.add_argument("-opt" ,"--option",
                        type=str,default="",
                        help="call option [sesbanner]")
    parser.add_argument("-V" , "--version",
                        help="Display Version Number",
                        action="store_true")
    parser.add_argument("-v" , "--verbose",
                        help="verbosity mode",
                        action="store_true")
    args = parser.parse_args()
    if args.version:
        print(TOOL_VERSION)
        sys.exit()

    # memory defines for Alif/OEM MRAM Addresses and Sizes
    load_global_config()
    ALIF_BASE_ADDRESS = utils.config.ALIF_BASE_ADDRESS
    OEM_BASE_ADDRESS = utils.config.APP_BASE_ADDRESS
    OEM_MRAM_SIZE = utils.config.APP_MRAM_SIZE
    DEVICE_FEATURE_SET = utils.config.DEVICE_FEATURE_SET
    DEVICE_FAMILY = utils.config.DEVICE_FAMILY
    DEVICE_REVISION = utils.config.DEVICE_REVISION
    DEVICE_PART_NUMBER = utils.config.DEVICE_PART_NUMBER
    DEVICE_REV_BAUD_RATE = utils.config.DEVICE_REV_BAUD_RATE

    # create a compound variable to address use cases as Fusion Rev B0
    DEVICE_FEATURE_SET_REVISION = DEVICE_FEATURE_SET + '_' + DEVICE_REVISION

    baud_rate = DEVICE_REV_BAUD_RATE[DEVICE_REVISION]
    if args.baudrate is not None:
        baud_rate = args.baudrate

    handler = CtrlCHandler()        # [CTRL-C] handling

    os.system('')                   # Help MS-DOS window with ESC sequences

    # Serial dabbling open up port.
    isp = serialPort(baud_rate)              # Create ISP session
    isp.setVerbose(args.verbose)    # turn on or off verbose mode for the host
    isp.CTRLCHandler = handler      # CTRL C Handler for this ISP session

    if args.discover:               # discover the COM ports if requested
        print('Discover')
        isp.discoverSerialPorts()

    errorCode = isp.openSerial()
    if errorCode is False:
        print("[ERROR] isp openSerial failed for %s" %isp.getPort())
        sys.exit(EXIT_WITH_ERROR)
    print("[INFO] %s open Serial port success" %isp.getPort())

    # Set up the baud rate ((same calls as in recovery.py))
    #if baud_rate != COM_BAUD_RATE_DEFAULT:
    #    isp.setBaudRate(baud_rate)
    print("[INFO] baud rate", isp.getBaudRate())

    test_target = isp_test_target(baud_rate, isp)
    print('[INFO] Connecting to target...', end='')
    if test_target == TARGET_RESPONDED:
        # be sure device is not in SEROM Recovery Mode
        device = device_probe.device_get_attributes(isp)    
        if device.stage == device_probe.STAGE_SEROM:
            print('Device connected in Recovery')
            MENU_DB  = 'utils/maint-recoveryDB.db'     
        else:
            # target responded and in SERAM stage
            print('Device connected')
            MENU_DB  = 'utils/maintDB.db'  
    else:
        # target did not respond... set full menu
        print('Device did not respond')
        MENU_DB  = 'utils/maintDB.db'

    MENU_CFG = 'utils/menuconfDB.db'

    # Process Maintenance Grouped Menus
    menuDB = read_json_file(MENU_DB)
    groupsDB = menuDB['groups']

    # load menu configurations
    menuCfgDB = read_json_file(MENU_CFG)

    if args.option:
        command_line_mode(isp,args.option)
    else:
        opt = '*'
        while opt != '':
            opt = showAndSelectOptions(groupsDB)

            # filter available commands based on silicon, revision, etc.
            supported_commands = menuDB[groupsDB[opt]]
            menu_options = {}
            for item in supported_commands:
                if not checkRestrictions(supported_commands[item], menuCfgDB):
                    menu_options[item] = supported_commands[item]
            maintenance_menu(menu_options, isp)

    isp.closeSerial()

    # Reset the Terminal of any ANSI Escape sequence debris
    isp_print_cursor_enable()
    isp_print_terminal_reset()

if __name__ == "__main__":
    main()
