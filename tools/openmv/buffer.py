# SPDX-License-Identifier: MIT
#
# Copyright (c) 2025 OpenMV, LLC.
#
# OpenMV Protocol Ring Buffer
#
# This module provides an efficient ring buffer implementation for
# packet parsing with optimized memory operations using memoryview.

import struct

class OMVRingBuffer:
    """Efficient ring buffer for packet parsing"""
    def __init__(self, size=4096):
        self.size = size
        self.data = memoryview(bytearray(size))
        self.start = 0  # Read position
        self.end = 0    # Write position
        self.count = 0  # Number of bytes in buffer
    
    def __len__(self):
        return self.count
    
    def extend(self, data):
        """Add data to buffer - optimized with memoryview"""
        data_len = len(data)
        data_view = memoryview(data)
        if data_len > self.size - self.count:
            raise BufferError(f"Buffer overflow:")

        # Calculate contiguous space from end to buffer boundary
        space_to_end = self.size - self.end
        
        if data_len <= space_to_end:
            # All data fits without wrapping
            self.data[self.end:self.end + data_len] = data_view[:data_len]
            self.end = (self.end + data_len) % self.size
        else:
            # First part: from end to buffer boundary
            self.data[self.end:self.size] = data_view[:space_to_end]
            # Second part: from buffer start
            remaining = data_len - space_to_end
            self.data[:remaining] = data_view[space_to_end:data_len]
            self.end = remaining
        
        self.count += data_len
    
    def peek(self, size):
        """Peek at data without consuming - returns memoryview when possible"""
        if size > self.count:
            return None
        
        if self.start + size <= self.size:
            # No wrap-around - return memoryview (zero-copy)
            return self.data[self.start:self.start + size]
        else:
            # Wrap-around case - must create new bytes object
            first_part = self.size - self.start
            # Use tobytes() for memoryview concatenation
            return (self.data[self.start:].tobytes() + 
                    self.data[:size - first_part].tobytes())
    
    def peek16(self):
        """Peek at 16-bit value at start of buffer"""
        if self.count < 2:
            return None
        elif self.start + 2 <= self.size:
            # No wrap-around - use struct directly on memoryview
            return struct.unpack('<H', self.data[self.start:self.start + 2])[0]
        else:
            # Wrap-around case - need to combine bytes
            return self.data[self.start] | (self.data[(self.start + 1) % self.size] << 8)
    
    def consume(self, count):
        """Consume count bytes from buffer"""
        count = min(count, self.count)
        self.start = (self.start + count) % self.size
        self.count -= count
    
    def read(self, size):
        """Read and consume data"""
        data = self.peek(size)
        if data is not None:
            self.consume(size)
        return data
