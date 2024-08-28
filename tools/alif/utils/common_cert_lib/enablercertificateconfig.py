# Copyright (c) 2001-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause OR Arm’s non-OSI source license
#

import configparser

from common.util_logger import UtilLogger
from common.exceptions import ConfigParsingError


class EnablerDebugCertificateConfig:
    CFG_SECTION_NAME = "ENABLER-DBG-CFG"

    def __init__(self, config_filename, app_logger=None):
        """
        Parses the ENABLER-DBG-CFG enabler debug certificate cfg file.
        Raises ConfigParsingError when cfg has been written incorrectly.
        Raises TypeError or ValueError when a cfg value is incorrect on its own.

        :param str config_filename: name of cfg file to parse
        :param UtilLogger app_logger: instance of the application logger
        """
        self._config_filename = config_filename
        self._logger = app_logger
        self._parser = configparser.ConfigParser()

        self._cert_keypair = None
        self._cert_keypair_pwd = None
        self._lcs = None
        self._rma_mode = None
        self._debug_masks = [0, 0, 0, 0]
        self._debug_locks = [0, 0, 0, 0]
        self._hbk_id = None
        self._key_cert_pkg = None
        self._next_cert_pubkey = None
        self._cert_pkg = None
        self._parse_config()

    @property
    def section_name(self):
        return self.CFG_SECTION_NAME

    @property
    def cert_keypair(self):
        return self._cert_keypair

    @cert_keypair.setter
    def cert_keypair(self, value):
        if isinstance(value, str) is False:
            raise TypeError("Config parameter cert-keypair must be a string")
        elif value == "":
            raise ValueError("Config parameter cert-keypair cannot be an empty string!")
        else:
            self._cert_keypair = value

    @property
    def cert_keypair_pwd(self):
        return self._cert_keypair_pwd

    @cert_keypair_pwd.setter
    def cert_keypair_pwd(self, value):
        if isinstance(value, str) is False:
            raise TypeError("Config parameter cert-keypair-pwd must be a string")
        else:
            self._cert_keypair_pwd = value

    @property
    def lcs(self):
        return self._lcs

    @lcs.setter
    def lcs(self, value):
        if isinstance(value, int) is False:
            raise TypeError("Config parameter lcs must be an integer")
        elif value not in [0, 1, 5, 7]:
            raise ValueError("invalid input value for config parameter lcs")
        else:
            self._lcs = value

    @property
    def rma_mode(self):
        return self._rma_mode

    @rma_mode.setter
    def rma_mode(self, value):
        if isinstance(value, int) is False:
            raise TypeError("Config parameter rma_mode must be an integer")
        elif value not in [0, 1]:
            raise ValueError("invalid input value for config parameter rma_mode")
        else:
            self._rma_mode = value

    @property
    def debug_masks(self):
        return self._debug_masks

    @debug_masks.setter
    def debug_masks(self, value):
        if isinstance(value, list) is False:
            raise TypeError("Config parameter debug_masks must be a list")
        elif len(value) != 4 or isinstance(value[0], int) is False or isinstance(value[1], int) is False or \
                isinstance(value[2], int) is False or isinstance(value[3], int) is False:
            raise TypeError("Config parameter debug_masks must be a list of 4 integers")
        else:
            for item in value:
                if not 0 <= item <= 0xFFFFFFFF:
                    raise ValueError("invalid input value for config parameter debug_masks")
            self._debug_masks = value

    @property
    def debug_locks(self):
        return self._debug_locks

    @debug_locks.setter
    def debug_locks(self, value):
        if isinstance(value, list) is False:
            raise TypeError("Config parameter debug_locks must be a list")
        elif len(value) != 4 or isinstance(value[0], int) is False or isinstance(value[1], int) is False or \
                isinstance(value[2], int) is False or isinstance(value[3], int) is False:
            raise TypeError("Config parameter debug_locks must be a list of 4 integers")
        else:
            for item in value:
                if not 0 <= item <= 0xFFFFFFFF:
                    raise ValueError("invalid input value for config parameter debug_locks")
            self._debug_locks = value

    @property
    def hbk_id(self):
        return self._hbk_id

    @hbk_id.setter
    def hbk_id(self, value):
        if isinstance(value, int) is False:
            raise TypeError("Config parameter hbk_id must be an integer")
        elif value not in [0, 1, 2, 0xF]:
            raise ValueError("invalid input value for config parameter hbk_id")
        else:
            self._hbk_id = value

    @property
    def key_cert_pkg(self):
        return self._key_cert_pkg

    @key_cert_pkg.setter
    def key_cert_pkg(self, value):
        if isinstance(value, str) is False:
            raise TypeError("Config parameter key_cert_pkg must be a string")
        else:
            self._key_cert_pkg = value

    @property
    def next_cert_pubkey(self):
        return self._next_cert_pubkey

    @next_cert_pubkey.setter
    def next_cert_pubkey(self, value):
        if isinstance(value, str) is False:
            raise TypeError("Config parameter next_cert_pubkey must be a string")
        elif value == "":
            raise ValueError("Config parameter next_cert_pubkey cannot be an empty string!")
        else:
            self._next_cert_pubkey = value

    @property
    def cert_pkg(self):
        return self._cert_pkg

    @cert_pkg.setter
    def cert_pkg(self, value):
        if isinstance(value, str) is False:
            raise TypeError("Config parameter cert_pkg must be a string")
        elif value == "":
            raise ValueError("Config parameter cert_pkg cannot be an empty string!")
        else:
            self._cert_pkg = value

    def _parse_config(self):
        if self._logger is not None:
            self._logger.info("Parsing config file: " + self._config_filename)
        self._parser.read(self._config_filename, encoding='cp1250')
        self._logger.info("Parsed items:\n" + str(self._parser.items(self.CFG_SECTION_NAME)))
        with open(self._config_filename, "r") as config_file:
            raw_data = config_file.read()
            self._logger.info("Raw content of config file:\n" + raw_data)

        if not self._parser.has_section(self.section_name):
            if self._logger is not None:
                self._logger.error("section " + self.section_name + " wasn't found in cfg file")
            raise ConfigParsingError

        is_debug_exist = 0
        if (self._parser.has_option(self.section_name, 'debug-mask[0-31]')
                and self._parser.has_option(self.section_name, 'debug-mask[32-63]')
                and self._parser.has_option(self.section_name, 'debug-mask[64-95]')
                and self._parser.has_option(self.section_name, 'debug-mask[96-127]')
                and self._parser.has_option(self.section_name, 'debug-lock[0-31]')
                and self._parser.has_option(self.section_name, 'debug-lock[32-63]')
                and self._parser.has_option(self.section_name, 'debug-lock[64-95]')
                and self._parser.has_option(self.section_name, 'debug-lock[96-127]')):
            debug_masks = [int(self._parser.get(self.section_name, 'debug-mask[0-31]'), 16),
                           int(self._parser.get(self.section_name, 'debug-mask[32-63]'), 16),
                           int(self._parser.get(self.section_name, 'debug-mask[64-95]'), 16),
                           int(self._parser.get(self.section_name, 'debug-mask[96-127]'), 16)]
            self.debug_masks = debug_masks

            debug_locks = [int(self._parser.get(self.section_name, 'debug-lock[0-31]'), 16),
                           int(self._parser.get(self.section_name, 'debug-lock[32-63]'), 16),
                           int(self._parser.get(self.section_name, 'debug-lock[64-95]'), 16),
                           int(self._parser.get(self.section_name, 'debug-lock[96-127]'), 16)]
            self.debug_locks = debug_locks
            is_debug_exist = 1

        if self._parser.has_option(self.section_name, 'rma-mode'):
            self.rma_mode = self._parser.getint(self.section_name, 'rma-mode')
        else:
            self.rma_mode = 0

        is_hbk_exist = self._parser.has_option(self.section_name, 'hbk-id')
        if is_hbk_exist:
            self.hbk_id = self._parser.getint(self.section_name, 'hbk-id')
        else:
            self.hbk_id = 0xF  # dummy value (from original) - certificate always need a value

        is_key_pkg_exist = 0
        if self._parser.has_option(self.section_name, 'key-cert-pkg'):
            self.key_cert_pkg = self._parser.get(self.section_name, 'key-cert-pkg')
            is_key_pkg_exist = 1

        if self.rma_mode == 1 and is_debug_exist == 1:
            if self._logger is not None:
                self._logger.error("Both RMA and debug mode are defined")
            raise ConfigParsingError
        if self.rma_mode == 0 and is_debug_exist == 0:
            if self._logger is not None:
                self._logger.error("Neither RMA nor Debug mode is defined")
            raise ConfigParsingError
        if is_key_pkg_exist == 0 and not is_hbk_exist:
            if self._logger is not None:
                self._logger.error("Neither hbk-id nor key-cert-pkg is defined")
            raise ConfigParsingError
        if is_key_pkg_exist == 1 and is_hbk_exist:
            if self._logger is not None:
                self._logger.error("Both hbk-id and key-cert-pkg is defined")
            raise ConfigParsingError
        if not is_hbk_exist and self.rma_mode == 1:
            if self._logger is not None:
                self._logger.error("hbk-id must exist if RMA mode is defined")
            raise ConfigParsingError

        if not self._parser.has_option(self.section_name, 'cert-keypair'):
            if self._logger is not None:
                self._logger.error("cert-keypair not found")
            raise ConfigParsingError
        else:
            self.cert_keypair = self._parser.get(self.section_name, 'cert-keypair')

        if self._parser.has_option(self.section_name, 'cert-keypair-pwd'):  # used for testing
            self.cert_keypair_pwd = self._parser.get(self.section_name, 'cert-keypair-pwd')
        else:
            self.cert_keypair_pwd = ''

        if not self._parser.has_option(self.section_name, 'lcs'):
            if self._logger is not None:
                self._logger.error("no lcs parameter found")
            raise ConfigParsingError
        else:
            self.lcs = self._parser.getint(self.section_name, 'lcs')

        if not self._parser.has_option(self.section_name, 'next-cert-pubkey'):
            if self._logger is not None:
                self._logger.error("no next-cert-pubkey parameter found")
            raise ConfigParsingError
        else:
            self.next_cert_pubkey = self._parser.get(self.section_name, 'next-cert-pubkey')

        if not self._parser.has_option(self.section_name, 'cert-pkg'):
            if self._logger is not None:
                self._logger.error("no cert-pkg parameter found")
            raise ConfigParsingError
        else:
            self.cert_pkg = self._parser.get(self.section_name, 'cert-pkg')
