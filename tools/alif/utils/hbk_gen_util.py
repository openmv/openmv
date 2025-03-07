#!/usr/bin/env python3
#
# Copyright (c) 2001-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause OR Arm’s non-OSI source license
#

####################################################################
# Filename - hbk_gen_util.py
# Description - This file is responsible for creation of public key
#               HASH and its corresponding number of Zeroes
#               as it should be saved in the OTP/NVM
####################################################################

import optparse
import sys
import os
from utils.common import util_logger
from utils.common import cryptolayer

# Prim key hash file name
PRIM_KEY_HASH_FILE_NAME = "prim_key_hash.txt"
ZEROS_NUM_FILES_NAME = "zero_bits_in_hash.txt"
# Util's log file
LOG_FILENAME = "gen_hbk_log.log"

OUTPUT_SIZE_SHA_256 = 32
OUTPUT_SIZE_SHA_256_TRUNC = int(OUTPUT_SIZE_SHA_256/2)
BITS_WITHIN_WORD = 32
BYTES_WITHIN_WORD = 4


def create_word_list(byte_array, byte_array_word_count, endianness):
    """
    Creates a hexadecimal (32 bits) word list out of a byte array representation of an input number
    :param byte_array: input number represented as a list of the bytes it consists of
    :param byte_array_word_count: number of (32 bit) words in input array
    :param endianness: output byte order -> little endian 'L' or big endian 'B'
    :return: 32 bit word list
    """
    word_list = []
    if not isinstance(byte_array, list) or not isinstance(byte_array_word_count, int):
        raise TypeError("input parameter byte_array or byte_array_word_count has wrong type")
    if endianness != 'L' and endianness != 'B':
        raise ValueError("input parameter endianness has wrong value")
    else:
        for i in range(byte_array_word_count):
            if endianness == 'L':
                current_word = byte_array[i * 4]
                current_word = current_word + (byte_array[i * 4 + 1] << 8)
                current_word = current_word + (byte_array[i * 4 + 2] << 16)
                current_word = current_word + (byte_array[i * 4 + 3] << 24)
            else:
                current_word = byte_array[i * 4 + 3]
                current_word = current_word + (byte_array[i * 4 + 2] << 8)
                current_word = current_word + (byte_array[i * 4 + 1] << 16)
                current_word = current_word + (byte_array[i * 4] << 24)
            word_list.append(current_word)
        return word_list


def count_zero_bits_in_bytes(bytes_input, bit_size):
    """
    Calculate the number of zero bits in a number given in a bytes representation.

    :param bytes_input: (bytes) input number
    :param bit_size: size of bytes_input in bits
    :return: (int) number of zero bits

    Returns TypeError if parameter bytes_input is not "bytes"
    Return TypeError if parameter bit_size is not int
    """
    # converting to str in binary representation - byteorder/endianness is irrelevant for this function
    binary_input = format(int.from_bytes(bytes_input, byteorder="big"), 'b').zfill(bit_size)
    return binary_input.count("0")


class ArgumentParser:
    def __init__(self):
        self.key_filename = None
        self.output_endianness = None
        self.hash_format = None
        self.result_hash_filename = None
        self.result_zeros_filename = None
        self.log_filename = None
        self.parser = optparse.OptionParser()
        self.parser.add_option("-k", "--key", dest="key_filename",
                               help="!Mandatory! The name of the PEM file holding the RSA public key")
        self.parser.add_option("-e", "--endian", dest="output_endianness", default="L",
                               metavar="END",
                               help="Sets the output endianness: END=B for Big Endian or END=L for Little Endian ["
                                    "default: %default]")
        self.parser.add_option("-f", "--format", dest="hash_format", default="SHA256",
                               metavar="HASHMODE",
                               help="Sets the hash format: HASHMODE=SHA256 for full SHA-256 hash (Hbk) or "
                                    "HASHMODE=SHA256_TRUNC for truncated SHA-256 hash (Hbk0 or Hbk1) [default: "
                                    "%default]")
        self.parser.add_option("-o", "--out-hash", dest="result_hash_filename", default=PRIM_KEY_HASH_FILE_NAME,
                               metavar="FILE",
                               help="Writes hash result to FILE [default: %default]")
        self.parser.add_option("-z", "--out-zeros", dest="result_zeros_filename", default=ZEROS_NUM_FILES_NAME,
                               metavar="FILE",
                               help="Writes number of zero bits in the hash result to FILE [default: %default]")
        self.parser.add_option("-l", "--log", dest="log_filename", default=LOG_FILENAME,
                               metavar="FILE",
                               help="Writes event log to FILE [default: %default]")

    def parse_arguments(self, args):
        (options, args) = self.parser.parse_args(args)
        if len(args) > 0:
            self.parser.error("incorrect number of positional arguments, use option -h for help")

        self.key_filename = options.key_filename
        if self.key_filename is None:
            self.parser.error("Option -k must be given, use option -h for help")

        self.output_endianness = options.output_endianness
        if self.output_endianness is not None and self.output_endianness != "B" and self.output_endianness != "L":
            self.parser.error("Illegal argument for output endianness (option -e)")

        self.hash_format = options.hash_format
        if self.hash_format is not None and self.hash_format != "SHA256" and self.hash_format != "SHA256_TRUNC":
            self.parser.error("Illegal argument for hash format (option -f)")

        self.result_hash_filename = options.result_hash_filename
        self.result_zeros_filename = options.result_zeros_filename
        self.log_filename = options.log_filename


class HbkGenerator:
    def __init__(self, argument_parser, logger):
        self.argument_parser = argument_parser
        self.logger = logger
        self.hash_digest = None
        self.hash_word_list = None
        self.zero_bits = None

        # decide hash output size
        if self.argument_parser.hash_format == "SHA256":
            self.outputSize = OUTPUT_SIZE_SHA_256
        else:
            self.outputSize = OUTPUT_SIZE_SHA_256_TRUNC
        self.output_word_count = self.outputSize // BYTES_WITHIN_WORD
        self.output_bit_count = self.output_word_count * BITS_WITHIN_WORD

    def generate_hbk(self):
        self.logger.info("Step 1: Calculating hash")
        self.hash_digest = cryptolayer.Common.get_hashed_n_and_np_from_public_key(self.argument_parser.key_filename,
                                                                                  self.logger)
        self.hash_digest = self.hash_digest[:self.outputSize]
        self.hash_word_list = create_word_list(list(self.hash_digest), self.output_word_count,
                                                            self.argument_parser.output_endianness)
        self.logger.info("Step 2: Calculate num of zero bits over the hash")
        self.zero_bits = count_zero_bits_in_bytes(self.hash_digest, self.output_bit_count)

    def write_output_files(self):
        hex_list = [format(item, "#08x") for item in self.hash_word_list]
        hash_output_string = ", ".join(hex_list)
        with open(self.argument_parser.result_hash_filename, 'w') as hash_output_file:
            hash_output_file.write(hash_output_string)
        #with open(self.argument_parser.result_zeros_filename, 'w') as zeros_output_file:
        #    zeros_output_file.write(hex(self.zero_bits))


def main(args):
    if not (sys.version_info.major == 3 and sys.version_info.minor >= 5):
        print("The script requires Python3.5 or later!")
        sys.exit(-1)
    # parse arguments
    the_argument_parser = ArgumentParser()
    the_argument_parser.parse_arguments(args)
    # get logging up and running
    the_logger = util_logger.UtilLogger(the_argument_parser.log_filename, "HBK Generator")
    # perform main task
    generator = HbkGenerator(the_argument_parser, the_logger)
    generator.generate_hbk()
    generator.write_output_files()
    the_logger.info("Script completed successfully")
