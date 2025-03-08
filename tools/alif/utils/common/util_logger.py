# Copyright (c) 2001-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause OR Arm’s non-OSI source license
#

import sys
import logging

#from utils.printInfo import *

class UtilLogger:
    def __init__(self, log_filename, utility_name):
        self.log_filename = log_filename
        self.root_logger = logging.getLogger()
        self.log_formatter = logging.Formatter("%(asctime)s - %(message)s")
        self.log_file_handler = logging.FileHandler(self.log_filename)
        self.log_file_handler.setFormatter(self.log_formatter)
        # Added if condition to avoid duplicate meessages or instances
        if not self.root_logger.handlers:    
            log_console_handler = logging.StreamHandler(sys.stdout)
            log_console_handler.setFormatter(self.log_formatter)
            self.root_logger.addHandler(log_console_handler)
        
        self.root_logger.addHandler(self.log_file_handler)
        self.root_logger.setLevel(logging.INFO)
        #if verboseModeGet():
        self.root_logger.info(utility_name + " Utility started (Logging to " + self.log_filename + ")")

    def info(self, message):
        self.root_logger.info(message)

    def warning(self, message):
        self.root_logger.warning(message)

    def error(self, message):
        self.root_logger.error(message)

    def critical(self, message):
        self.root_logger.critical(message)
