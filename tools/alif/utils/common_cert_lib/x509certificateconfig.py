# Copyright (c) 2001-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause OR Arm’s non-OSI source license
#

import configparser
import sys
import os

from common.util_logger import UtilLogger


class X509CertificateConfig:

    def __init__(self, config_filename, config_section_name, app_logger=None):
        """
        Parses the X509 part of a certificate config file.
        Raises ConfigParsingError when cfg has been written incorrectly.
        Raises TypeError or ValueError when a cfg value is incorrect on its own.

        :param str config_filename: name of cfg file to parse
        :param str config_section_name: name of the section holding the values of the cfg file
        :param UtilLogger app_logger: instance of the application logger
        """
        self._config_filename = config_filename
        self._section_name = config_section_name
        self._logger = app_logger
        self._parser = configparser.ConfigParser()

        self._issuer_name = ""
        self._subject_name = ""
        self._not_before = 0
        self._not_after = 0
        self._serial_num = 0
        self._parse_config()

    @property
    def issuer_name(self):
        return self._issuer_name

    @issuer_name.setter
    def issuer_name(self, value):
        if isinstance(value, str) is False:
            raise TypeError("Config parameter issuer-name must be a string")
        else:
            self._issuer_name = value

    @property
    def subject_name(self):
        return self._subject_name

    @subject_name.setter
    def subject_name(self, value):
        if isinstance(value, str) is False:
            raise TypeError("Config parameter subject-name must be a string")
        else:
            self._subject_name = value

    @property
    def not_before(self):
        return self._not_before

    @not_before.setter
    def not_before(self, value):
        if isinstance(value, int) is False:
            raise TypeError("Config parameter not-before must be an integer")
        else:
            self._not_before = value

    @property
    def not_after(self):
        return self._not_after

    @not_after.setter
    def not_after(self, value):
        if isinstance(value, int) is False:
            raise TypeError("Config parameter not-after must be an integer")
        else:
            self._not_after = value

    @property
    def serial_num(self):
        return self._serial_num

    @serial_num.setter
    def serial_num(self, value):
        if isinstance(value, int) is False:
            raise TypeError("Config parameter serial-num must be an integer")
        else:
            self._serial_num = value

    def _parse_config(self):
        self._parser.read(self._config_filename, encoding='cp1250')

        if self._parser.has_option(self._section_name, 'issuer-name'):
            self.issuer_name = self._parser.get(self._section_name, 'issuer-name')

        if self._parser.has_option(self._section_name, 'subject-name'):
            self.subject_name = self._parser.get(self._section_name, 'subject-name')

        if self._parser.has_option(self._section_name, 'not-before'):
            self.not_before = self._parser.getint(self._section_name, 'not-before')

        if self._parser.has_option(self._section_name, 'not-after'):
            self.not_after = self._parser.getint(self._section_name, 'not-after')

        if self._parser.has_option(self._section_name, 'serial-num'):
            self.serial_num = self._parser.getint(self._section_name, 'serial-num')
