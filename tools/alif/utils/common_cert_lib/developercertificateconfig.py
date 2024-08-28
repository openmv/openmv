# Copyright (c) 2001-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause OR Arm’s non-OSI source license
#

import configparser

from common.util_logger import UtilLogger
from common.exceptions import ConfigParsingError


class DeveloperDebugCertificateConfig:
    CFG_SECTION_NAME = "DEVELOPER-DBG-CFG"

    def __init__(self, config_filename, app_logger=None):
        """
        Parses the DEVELOPER-DBG-CFG developer debug certificate cfg file.
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
        self._soc_id = None
        self._debug_masks = [0, 0, 0, 0]
        self._enabler_cert_pkg = None
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
    def soc_id(self):
        return self._soc_id

    @soc_id.setter
    def soc_id(self, value):
        if isinstance(value, str) is False:
            raise TypeError("Config parameter soc_id must be a string")
        else:
            self._soc_id = value

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
    def enabler_cert_pkg(self):
        return self._enabler_cert_pkg

    @enabler_cert_pkg.setter
    def enabler_cert_pkg(self, value):
        if isinstance(value, str) is False:
            raise TypeError("Config parameter enabler_cert_pkg must be a string")
        elif value == "":
            raise ValueError("Config parameter enabler_cert_pkg cannot be an empty string!")
        else:
            self._enabler_cert_pkg = value

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
                self._logger.warning("section [" + self.section_name + "] wasn't found in cfg file")
            raise ConfigParsingError

        if not self._parser.has_option(self.section_name, 'cert-keypair'):
            if self._logger is not None:
                self._logger.error("parameter cert-keypair not found in cfg file")
            raise ConfigParsingError
        else:
            self.cert_keypair = self._parser.get(self.section_name, 'cert-keypair')

        if self._parser.has_option(self.section_name, 'cert-keypair-pwd'):  # used for testing
            self.cert_keypair_pwd = self._parser.get(self.section_name, 'cert-keypair-pwd')
        else:
            self.cert_keypair_pwd = ''

        if not self._parser.has_option(self.section_name, 'soc-id'):
            if self._logger is not None:
                self._logger.error("parameter soc-id not found in cfg file")
            raise ConfigParsingError
        else:
            self.soc_id = self._parser.get(self.section_name, 'soc-id')

        if (self._parser.has_option(self.section_name, 'debug-mask[0-31]')
                and self._parser.has_option(self.section_name, 'debug-mask[32-63]')
                and self._parser.has_option(self.section_name, 'debug-mask[64-95]')
                and self._parser.has_option(self.section_name, 'debug-mask[96-127]')):
            debug_masks = [int(self._parser.get(self.section_name, 'debug-mask[0-31]'), 16),
                           int(self._parser.get(self.section_name, 'debug-mask[32-63]'), 16),
                           int(self._parser.get(self.section_name, 'debug-mask[64-95]'), 16),
                           int(self._parser.get(self.section_name, 'debug-mask[96-127]'), 16)]
            self.debug_masks = debug_masks

        if not self._parser.has_option(self.section_name, 'enabler-cert-pkg'):
            if self._logger is not None:
                self._logger.error("parameter enabler_cert_pkg not found in cfg file")
            raise ConfigParsingError
        else:
            self.enabler_cert_pkg = self._parser.get(self.section_name, 'enabler-cert-pkg')

        if not self._parser.has_option(self.section_name, 'cert-pkg'):
            if self._logger is not None:
                self._logger.error("parameter cert_pkg not found in cfg file")
            raise ConfigParsingError
        else:
            self.cert_pkg = self._parser.get(self.section_name, 'cert-pkg')
