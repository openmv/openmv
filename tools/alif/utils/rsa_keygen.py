#!/usr/bin/env python3
#
# Copyright (c) 2001-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause OR Arm’s non-OSI source license
#

import optparse
import sys
from utils.common import util_logger
from utils.common import cryptolayer

# Util's log file
LOG_FILENAME = "key_gen_log.log"


class ArgumentParser:
    def __init__(self):
        self.private_key_filename = None
        self.private_key_pwd_filename = None
        self.public_key_filename = None
        self.log_filename = None
        #self.parser = optparse.OptionParser(usage="usage: %prog <private_key_filename> [options]",
        #                                    description="%prog generates a PEM encoded RSA private key and writes it "
        #                                                "to the file specified by parameter <private_key_filename>. "
        #                                                "Optionally it can also generate the corresponding public key "
        #                                                "to the file specified by option -k.")
        # Removed above line of code and added below line      ---------------------- Swathi
        self.parser = optparse.OptionParser()
        self.parser.add_option("-p", "--pass_file", dest="private_key_pwd_filename",
                               help="Filename containing the passphrase for creating the private key, "
                                    "in plaintext format. For security considerations, this parameter can be omitted, "
                                    "in which case this utility will prompt for direct input.")
        self.parser.add_option("-k", "--pubkey", dest="public_key_filename",
                               help="If given, the public key will be also extracted into the PEM file specified "
                                    "by this parameter")
        self.parser.add_option("-l", "--log", dest="log_filename", default=LOG_FILENAME,
                               metavar="FILE",
                               help="Writes event log to FILE [default: %default]")

    def parse_arguments(self, args):
        (options, args) = self.parser.parse_args(args)
        if len(args) != 1:
            self.parser.error("incorrect number of positional arguments, use option -h for help")
        self.private_key_filename = args[0]
        self.private_key_pwd_filename = options.private_key_pwd_filename
        self.public_key_filename = options.public_key_filename
        self.log_filename = options.log_filename


class KeyGenerator:
    def __init__(self, argument_parser, logger):
        self.argument_parser = argument_parser
        self.logger = logger

    def generate_key(self):
        key = cryptolayer.RsaCrypto.generate_rsa_pem_key(self.argument_parser.private_key_filename,
                                                         self.argument_parser.private_key_pwd_filename,
                                                         self.logger)
        if self.argument_parser.public_key_filename is not None:
            cryptolayer.RsaCrypto.extract_public_rsa_pem_key(key, self.argument_parser.public_key_filename, self.logger)


def main(args):
    if not (sys.version_info.major == 3 and sys.version_info.minor >= 5):
        print("The script requires Python3.5 or later!")
        sys.exit(-1)
    # parse arguments
    the_argument_parser = ArgumentParser()
    the_argument_parser.parse_arguments(args)
    # get logging up and running
    the_logger = util_logger.UtilLogger(the_argument_parser.log_filename, "RSA Key Generator")
    # perform main task
    generator = KeyGenerator(the_argument_parser, the_logger)
    generator.generate_key()
    the_logger.info("Script completed successfully")
