#!/usr/bin/python3
"""
    ISP printing
     - Created to avoid circular references as used in more than place

    In System Programming (ISP) protocol implementation
   __author__ onyettr
"""
# pylint: disable=unused-argument, invalid-name
from isp_protocol import ISP_PACKET_DATA_FIELD

# Colour table for ANSI terminal printing
ansi_fg_colour = {
            "white"  : "\033[97m",
            "cyan"   : "\033[96m",
            "header" : "\033[95m]",
            "blue"   : "\033[94m",
            "yellow" : "\033[93m",
            "green"  : "\033[92m",
            "red"    : "\033[91m",
            "black"  : "\033[90m",
            "reset"  : "\033[0m"
}

def isp_print_color(fg, message_string):
    """
        print a message
    """
    print(ansi_fg_colour[fg],end='')
    print(message_string, end='')
    print(ansi_fg_colour["reset"],end='')

def isp_print_response(fg, message):
    """
        print a data response packet
        This is an unknown response format so we just print each elementc
    """
    print_message = message[ISP_PACKET_DATA_FIELD:len(message)-1]

    print(ansi_fg_colour[fg],end='')
    for x in print_message:
        print(hex(x), end='')
        print(' ', end='')
    print(ansi_fg_colour["reset"])

def isp_print_message(fg, message):
    """
        print a PRINT_DATA message
        This is a NULL terminated string
    """
    print_message = bytes(message[ISP_PACKET_DATA_FIELD:len(message)-1])
    eoln = print_message.find(0)
    print_message = print_message[:eoln]
    print(ansi_fg_colour[fg], print_message.decode('utf-8'), \
          ansi_fg_colour["reset"])

def isp_print_terminal_reset():
    """
        isp_print_terminal_reset
            reset the ANSI graphics Terminal
    """
    print("\033[0m")

def isp_print_cursor_disable():
    """
        isp_print_cursor_disable
            Stop Cursor Blinking
    """
    print("\033[?25l") # Cursor off

def isp_print_cursor_enable():
    """
        isp_print_cursor_enable
            reset the Cursor to Blinking
    """
    print("\033[?25h")  # Flicker enables Cusror hide DECTCEM, this reenables
