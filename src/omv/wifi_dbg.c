/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2018 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include <stdint.h>
#include "wifi_dbg.h"

// To avoid writing extra code just re-use the USBDBG code as the interface that does the actual work.
// The only tricky part is that you don't have USB transactions anymore. So, you need to buffer 6 bytes
// in a fifo until you have 6 of them to execute a control phase. Then, push the rest into the data phase
// as described by the length in the control phase header. Buffering on the wifi network will definately
// cause all RX bytes to be in one long giant burst. You have to write the code to handle this...

extern void usbdbg_data_in(void *buffer, int length);
extern void usbdbg_data_out(void *buffer, int length);
extern void usbdbg_control(void *buffer, uint8_t brequest, uint32_t wlength);

void wifi_dbg_init()
{
    // Get wifi debug into a known state. Since this is called by all OpenMV Cams you should not toggle any
    // pins unless you detect the wifi module was previously setup...
    // Also, reset regular USB CDC DBG access if things were setup before.
}

void wifi_dbg_apply_config(wifi_dbg_config_t *config)
{
    switch(config->wifi_mode) {
        case WIFI_DBG_DISABLED: // Don't init anything.
            break;
        case WIFI_DBG_ENABLED_CLIENT_MODE:
            // Start the wifi module up in client mode.
            // Additionally, make network.WINC() return a python object of the setup wifi module and make
            // winc.connect()/winc.disconnect() do nothing. This should allow the user to use the wifi module
            // normally in their scripts in debug mode.
            // You should have all the settings you need to init the wifi module in the config struct.
            // Disable regular USB CDC DBG if this works... two folks can't share the interface.
            // For data transfer create a TCP server after setting up the wifi. You will then UDP broadcast
            // your IP address along with the port you chose for the TCP server. OpenMV IDE will connect to this.
            break;
        case WIFI_DBG_ENABLED_AP_MODE:
            // Start the wifi module up in ap mode.
            // Additionally, make network.WINC() return a python object of the setup wifi module and make
            // winc.connect()/winc.disconnect() do nothing. This should allow the user to use the wifi module
            // normally in their scripts in debug mode.
            // You should have all the settings you need to init the wifi module in the config struct.
            // Disable regular USB CDC DBG if this works... two folks can't share the interface.
            // For data transfer create a TCP server after setting up the wifi. You will then UDP broadcast
            // your IP address along with the port you chose for the TCP server. OpenMV IDE will connect to this.
            break;
        default: // Don't init anything.
            break;
    }
}

void wifi_dbg_beacon()
{
    // Call this every second via a timer calback or something if wifi is enabled.
    // You need to broadcast a UDP message on the OPENMVCAM_BROADCAST_PORT port.
    // The contents of the message should be "ip:port:board_name", e.g. "192.168.2.1:4321:Ibrahim's cam".
    // OpenMV splits the message using the ":" character. The port can by anything (whatever the module picks randomly).
    // The IP address must be in dot format. Finally, the string can have whatever in it.
    // If broadcasting is working the IDE should display a new board option to connect to when you hit connect.
    // I'd get this working first for both AP and client mode before working on the rest of the code.
}
