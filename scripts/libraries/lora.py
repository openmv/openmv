# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
# Copyright (c) 2021 Arduino SA
#
# Authors:
# Ibrahim Abdelkader <iabdalkader@openmv.io>
# Sebastian Romero <s.romero@arduino.cc>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# LoRa library for Arduino Portenta.

from time import sleep_ms, ticks_ms
from pyb import UART, Pin

MODE_ABP = 0
MODE_OTAA = 1

RF_MODE_RFO = 0
RF_MODE_PABOOST = 1

BAND_AS923 = 0
BAND_AU915 = 1
BAND_EU868 = 5
BAND_KR920 = 6
BAND_IN865 = 7
BAND_US915 = 8
BAND_US915_HYBRID = 9

CLASS_A = "A"
CLASS_B = "B"
CLASS_C = "C"


class LoraError(Exception):
    pass


class LoraErrorTimeout(LoraError):
    pass


class LoraErrorParam(LoraError):
    pass


class LoraErrorBusy(LoraError):
    pass


class LoraErrorOverflow(LoraError):
    pass


class LoraErrorNoNetwork(LoraError):
    pass


class LoraErrorRX(LoraError):
    pass


class LoraErrorUnknown(LoraError):
    pass


class Lora:
    LoraErrors = {
        "": LoraErrorTimeout,  # empty buffer
        "+ERR": LoraError,
        "+ERR_PARAM": LoraErrorParam,
        "+ERR_BUSY": LoraErrorBusy,
        "+ERR_PARAM_OVERFLOW": LoraErrorOverflow,
        "+ERR_NO_NETWORK": LoraErrorNoNetwork,
        "+ERR_RX": LoraErrorRX,
        "+ERR_UNKNOWN": LoraErrorUnknown,
    }

    def __init__(
        self, uart=None, rst_pin=None, boot_pin=None, band=BAND_EU868, poll_ms=300000, debug=False
    ):
        self.debug = debug
        self.uart = uart
        self.rst_pin = rst_pin
        self.boot_pin = boot_pin
        self.band = band
        self.poll_ms = poll_ms
        self.last_poll_ms = ticks_ms()

        self.init_modem()

        # Reset module
        self.boot_pin.value(0)
        self.rst_pin.value(1)
        sleep_ms(200)
        self.rst_pin.value(0)
        sleep_ms(200)
        self.rst_pin.value(1)

        # Restart module
        self.restart()

    def init_modem(self):
        # Portenta vision shield configuration
        if not self.rst_pin:
            self.rst_pin = Pin("PC6", Pin.OUT_PP, Pin.PULL_UP, value=1)
        if not self.boot_pin:
            self.boot_pin = Pin("PG7", Pin.OUT_PP, Pin.PULL_DOWN, value=0)
        if not self.uart:
            self.uart = UART(8, 19200)
            # self.uart = UART(1, 19200) # Use external module
            self.uart.init(19200, bits=8, parity=None, stop=2, timeout=250, timeout_char=100)

    def debug_print(self, data):
        if self.debug:
            print(data)

    def is_arduino_firmware(self):
        return "ARD-078" in self.fw_version

    def configure_class(self, _class):
        self.send_command("+CLASS=", _class)

    def configure_band(self, band):
        self.send_command("+BAND=", band)
        if band == BAND_EU868 and self.is_arduino_firmware():
            self.send_command("+DUTYCYCLE=", 1)
        return True

    def set_baudrate(self, baudrate):
        self.send_command("+UART=", baudrate)

    def set_autobaud(self, timeout=10000):
        start = ticks_ms()
        while (ticks_ms() - start) < timeout:
            if self.send_command("", timeout=200, raise_error=False) == "+OK":
                sleep_ms(200)
                while self.uart.any():
                    self.uart.readchar()
                return True
        return False

    def get_fw_version(self):
        dev = self.send_command("+DEV?")
        fw_ver = self.send_command("+VER?")
        return dev + " " + fw_ver

    def get_device_eui(self):
        return self.send_command("+DEVEUI?")

    def factory_default(self):
        self.send_command("+FACNEW")

    def restart(self):
        if self.set_autobaud() is False:
            raise (LoraError("Failed to set autobaud"))

        # Different delimiter as REBOOT response EVENT doesn't end with '\r'.
        if self.send_command("+REBOOT", delimiter="+EVENT=0,0", timeout=10000) != "+EVENT=0,0":
            raise (LoraError("Failed to reboot module"))
        sleep_ms(1000)
        self.fw_version = self.get_fw_version()
        self.configure_band(self.band)

    def set_rf_power(self, mode, power):
        self.send_command("+RFPOWER=", mode, ",", power)

    def set_port(self, port):
        self.send_command("+PORT=", port)

    def set_public_network(self, enable):
        self.send_command("+NWK=", int(enable))

    def sleep(self, enable):
        self.send_command("+SLEEP=", int(enable))

    def format(self, hexMode):
        self.send_command("+DFORMAT=", int(hexMode))

    def set_datarate(self, dr):
        self.send_command("+DR=", dr)

    def get_datarate(self):
        return int(self.send_command("+DR?"))

    def set_adr(self, adr):
        self.send_command("+ADR=", int(adr))

    def get_adr(self):
        return int(self.send_command("+ADR?"))

    def get_devaddr(self):
        return self.send_command("+DEVADDR?")

    def get_nwk_skey(self):
        return self.send_command("+NWKSKEY?")

    def get_appskey(self):
        return self.send_command("+APPSKEY?")

    def get_rx2dr(self):
        return int(self.send_command("+RX2DR?"))

    def set_rx2dr(self, dr):
        self.send_command("+RX2DR=", dr)

    def get_ex2freq(self):
        return int(self.send_command("+RX2FQ?"))

    def set_rx2freq(self, freq):
        self.send_command("+RX2FQ=", freq)

    def set_fcu(self, fcu):
        self.send_command("+FCU=", fcu)

    def get_fcu(self):
        return int(self.send_command("+FCU?"))

    def set_fcd(self, fcd):
        self.send_command("+FCD=", fcd)

    def get_fcd(self):
        return int(self.send_command("+FCD?"))

    def change_mode(self, mode):
        self.send_command("+MODE=", mode)

    def join(self, timeout_ms):
        if self.send_command("+JOIN", timeout=timeout_ms) != "+ACK":
            return False
        response = self.receive("\r", timeout=timeout_ms)
        return response == "+EVENT=1,1"

    def get_join_status(self):
        return int(self.send_command("+NJS?")) == 1

    def get_max_size(self):
        if self.is_arduino_firmware():
            return 64
        return int(self.send_command("+MSIZE?", timeout=2000))

    def poll(self):
        if (ticks_ms() - self.last_poll_ms) > self.poll_ms:
            self.last_poll_ms = ticks_ms()
            # Triggers a fake write
            self.send_data("\0", True)

    def send_data(self, buff, confirmed=True):
        max_len = self.get_max_size()
        if len(buff) > max_len:
            raise (LoraError("Packet exceeds max length"))
        if self.send_command("+CTX " if confirmed else "+UTX ", len(buff), data=buff) != "+OK":
            return False
        if confirmed:
            response = self.receive("\r", timeout=10000)
            return response == "+ACK"
        return True

    def receive_data(self, timeout=1000):
        response = self.receive("\r", timeout=timeout)
        if response.startswith("+RECV"):
            params = response.split("=")[1].split(",")
            port = params[0]
            length = int(params[1])
            dummy_data_length = 2  # Data starts with \n\n sequence
            data = self.receive(max_bytes=length + dummy_data_length, timeout=timeout)[
                dummy_data_length:
            ]
            return {"port": port, "data": data}

    def receive(self, delimiter=None, max_bytes=None, timeout=1000):
        buf = []
        start = ticks_ms()
        while (ticks_ms() - start) < timeout:
            while self.uart.any():
                buf += chr(self.uart.readchar())

                if max_bytes and len(buf) == max_bytes:
                    data = "".join(buf)
                    self.debug_print(data)
                    return data
                if len(buf) and delimiter is not None:
                    data = "".join(buf)
                    trimmed = data[0:-1] if data[-1] == "\r" else data

                    if isinstance(delimiter, str) and len(delimiter) == 1 and buf[-1] == delimiter:
                        self.debug_print(trimmed)
                        return trimmed
                    if isinstance(delimiter, str) and trimmed == delimiter:
                        self.debug_print(trimmed)
                        return trimmed
                    if isinstance(delimiter, list) and trimmed in delimiter:
                        self.debug_print(trimmed)
                        return trimmed

        data = "".join(buf)
        self.debug_print(data)
        return data[0:-1] if len(data) != 0 and data[-1] == "\r" else data

    def available(self):
        return self.uart.any()

    def join_OTAA(self, appEui, appKey, devEui=None, timeout=60000):
        self.change_mode(MODE_OTAA)
        self.send_command("+APPEUI=", appEui)
        self.send_command("+APPKEY=", appKey)
        if devEui:
            self.send_command("+DEVEUI=", devEui)
        network_joined = self.join(timeout)
        # This delay was in MKRWAN.h
        # delay(1000);
        return network_joined

    def join_ABP(self, nwkId, devAddr, nwkSKey, appSKey, timeout=60000):
        self.change_mode(MODE_ABP)
        # Commented in MKRWAN.h
        # self.send_command("+IDNWK=", nwkId)
        self.send_command("+DEVADDR=", devAddr)
        self.send_command("+NWKSKEY=", nwkSKey)
        self.send_command("+APPSKEY=", appSKey)
        self.join(timeout)
        return self.get_join_status()

    def handle_error(self, command, data):
        if not data.startswith("+ERR") and data != "":
            return
        if data in self.LoraErrors:
            raise (self.LoraErrors[data]('Command "%s" has failed!' % command))
        raise (LoraError('Command: "%s" failed with unknown status: "%s"' % (command, data)))

    def send_command(self, cmd, *args, delimiter="\r", data=None, timeout=1000, raise_error=True):
        # Write command and args
        uart_cmd = "AT" + cmd + "".join([str(x) for x in args]) + "\r"
        self.debug_print(uart_cmd)
        self.uart.write(uart_cmd)

        # Attach raw data
        if data:
            self.debug_print(data)
            self.uart.write(data)

        # Read status and value (if any)
        response = self.receive(delimiter, timeout=timeout)

        # Error handling
        if raise_error:
            self.handle_error(cmd, response)

        if cmd.endswith("?"):
            return response.split("=")[1]
        return response
