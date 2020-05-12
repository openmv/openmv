# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2020 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2020 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.

import gc, serial, socket, struct, time

class rpc:

    _COMMAND_HEADER_PACKET_MAGIC = 0x1209
    _COMMAND_DATA_PACKET_MAGIC = 0xABD1
    _RESULT_HEADER_PACKET_MAGIC = 0x9021
    _RESULT_DATA_PACKET_MAGIC = 0x1DBA

    def __def_crc_16(self, data, size): # private
        crc = 0xFFFF
        for i in range(size):
            crc ^= data[i] << 8
            for j in range(8): crc = (crc << 1) ^ (0x1021 if crc & 0x8000 else 0)
        return crc & 0xFFFF

    def _zero(self, buff, size): # private
        for i in range(size): buff[i] = 0

    def _same(self, data, size): # private
        if not size: return False
        old = data[0]
        for i in range(1, size):
            new = data[i]
            if new != old: return False
            old = new
        return True

    # djb2 algorithm; see http://www.cse.yorku.ca/~oz/hash.html
    def _hash(self, data, size): # private
        h = 5381
        for i in range(size):
            h = ((h << 5) + h) ^ ord(data[i])
        return h & 0xFFFFFFFF

    def __init__(self): # private
        self.__crc_16 = self.__def_crc_16
        self._stream_writer_queue_depth_max = 255

    def _get_packet_pre_alloc(self, payload_len=0):
        buff = bytearray(payload_len + 4)
        return (buff, memoryview(buff)[2:-2])

    def _get_packet(self, magic_value, payload_buf_tuple, timeout): # private
        packet = self.get_bytes(payload_buf_tuple[0], timeout)
        if packet is not None:
            magic = packet[0] | (packet[1] << 8)
            crc = packet[-2] | (packet[-1] << 8)
            if magic == magic_value and crc == self.__crc_16(packet, len(packet) - 2):
                return payload_buf_tuple[1]
        return None

    def _set_packet(self, magic_value, payload=bytes()): # private
        new_payload = bytearray(len(payload) + 4)
        new_payload[:2] = struct.pack("<H", magic_value)
        # Fix Python 3.x.
        try: new_payload[2:-2] = payload
        except TypeError: new_payload[2:-2] = payload.encode()
        new_payload[-2:] = struct.pack("<H", self.__crc_16(new_payload, len(payload) + 2))
        return new_payload

    def _flush(self): # protected
        pass

    def get_bytes(self, buff, timeout_ms): # protected
        return bytes()

    def put_bytes(self, data, timeout_ms): # protected
        pass

    def stream_reader(self, call_back, queue_depth=1, read_timeout_ms=5000): # public
        try: self._stream_put_bytes(self._set_packet(0xEDF6, struct.pack("<I", queue_depth)), 1000)
        except OSError: return
        tx_lfsr = 255
        while True:
            packet = self._stream_get_bytes(bytearray(8), 1000)
            if packet is None: return
            magic = packet[0] | (packet[1] << 8)
            crc = packet[-2] | (packet[-1] << 8)
            if magic != 0x542E and crc != self.__crc_16(packet, len(packet) - 2): return
            data = self._stream_get_bytes(bytearray(struct.unpack("<I", packet[2:-2])[0]), read_timeout_ms)
            if data is None: return
            call_back(data)
            try: self._stream_put_bytes(struct.pack("<B", tx_lfsr), 1000)
            except OSError: return
            tx_lfsr = (tx_lfsr >> 1) ^ (0xB8 if tx_lfsr & 1 else 0x00)

    def stream_writer(self, call_back, write_timeout_ms=5000): # public
        packet = self._stream_get_bytes(bytearray(8), 1000)
        if packet is None: return
        magic = packet[0] | (packet[1] << 8)
        crc = packet[-2] | (packet[-1] << 8)
        if magic != 0xEDF6 and crc != self.__crc_16(packet, len(packet) - 2): return
        queue_depth = max(min(struct.unpack("<I", packet[2:-2])[0], self._stream_writer_queue_depth_max), 1)
        rx_lfsr = 255
        credits = queue_depth
        while True:
            if credits <= (queue_depth // 2):
                data = self._stream_get_bytes(bytearray(1), 1000)
                if data is None or data[0] != rx_lfsr: return
                rx_lfsr = (rx_lfsr >> 1) ^ (0xB8 if rx_lfsr & 1 else 0x00)
                credits += 1
            if credits > 0:
                data = call_back()
                try: self._stream_put_bytes(self._set_packet(0x542E, struct.pack("<I", len(data))), 1000)
                except OSError: return
                try: self._stream_put_bytes(data, write_timeout_ms)
                except OSError: return
                credits -= 1

    def _stream_get_bytes(self, buff, timeout_ms): # protected
        return self.get_bytes(buff, timeout_ms)

    def _stream_put_bytes(self, data, timeout_ms): # protected
        self.put_bytes(data, timeout_ms)

class rpc_master(rpc):

    def __init__(self): # private
        rpc.__init__(self)
        self.__in_command_header_buf = self._get_packet_pre_alloc()
        self.__in_command_data_buf = self._get_packet_pre_alloc()
        self.__out_result_header_ack = self._set_packet(self._RESULT_HEADER_PACKET_MAGIC)
        self.__in_result_header_buf = self._get_packet_pre_alloc(4)
        self.__out_result_data_ack = self._set_packet(self._RESULT_DATA_PACKET_MAGIC)
        self._put_short_timeout_reset = 3
        self._get_short_timeout_reset = 3
        self._put_long_timeout = 5000
        self._get_long_timeout = 5000

    def __put_command(self, command, data, timeout): # private
        self._put_short_timeout = self._put_short_timeout_reset
        self._get_short_timeout = self._get_short_timeout_reset
        out_header = self._set_packet(self._COMMAND_HEADER_PACKET_MAGIC, struct.pack("<II", command, len(data)))
        out_data = self._set_packet(self._COMMAND_DATA_PACKET_MAGIC, data)
        start = int(time.time() * 1000)
        while (int(time.time() * 1000) - start) < timeout:
            gc.collect() # Avoid collection during the transfer.
            self._zero(self.__in_command_header_buf[0], len(self.__in_command_header_buf[0]))
            self._zero(self.__in_command_data_buf[0], len(self.__in_command_data_buf[0]))
            self._flush()
            self.put_bytes(out_header, self._put_short_timeout)
            if self._get_packet(self._COMMAND_HEADER_PACKET_MAGIC, self.__in_command_header_buf, self._get_short_timeout) is not None:
                self.put_bytes(out_data, self._put_long_timeout)
                if self._get_packet(self._COMMAND_DATA_PACKET_MAGIC, self.__in_command_data_buf, self._get_short_timeout) is not None:
                    return True
            # Avoid timeout livelocking.
            self._put_short_timeout = min((self._put_short_timeout * 4) // 3, timeout)
            self._get_short_timeout = min((self._get_short_timeout * 4) // 3, timeout)
        return False

    def __get_result(self, timeout): # private
        self._put_short_timeout = self._put_short_timeout_reset
        self._get_short_timeout = self._get_short_timeout_reset
        start = int(time.time() * 1000)
        while (int(time.time() * 1000) - start) < timeout:
            gc.collect() # Avoid collection during the transfer.
            self._zero(self.__in_result_header_buf[0], len(self.__in_result_header_buf[0]))
            self._flush()
            self.put_bytes(self.__out_result_header_ack, self._put_short_timeout)
            packet = self._get_packet(self._RESULT_HEADER_PACKET_MAGIC, self.__in_result_header_buf, self._get_short_timeout)
            if packet is not None:
                in_result_data_buf = self._get_packet_pre_alloc(struct.unpack("<I", packet)[0])
                self.put_bytes(self.__out_result_data_ack, self._put_short_timeout)
                dat_packet = self._get_packet(self._RESULT_DATA_PACKET_MAGIC, in_result_data_buf, self._get_long_timeout)
                if dat_packet is not None:
                    return dat_packet
            # Avoid timeout livelocking.
            self._put_short_timeout = min((self._put_short_timeout * 4) // 3, timeout)
            self._get_short_timeout = min((self._get_short_timeout * 4) // 3, timeout)
        return None

    def call(self, name, data=bytes(), send_timeout=1000, recv_timeout=1000): # public
        return self.__get_result(recv_timeout) if self.__put_command(self._hash(name, len(name)), data, send_timeout) else None

class rpc_slave(rpc):

    def __init__(self): # private
        self.__dict = {}
        self.__schedule_cb = None
        self.__loop_cb = None
        rpc.__init__(self)
        self.__in_command_header_buf = self._get_packet_pre_alloc(8)
        self.__out_command_header_ack = self._set_packet(self._COMMAND_HEADER_PACKET_MAGIC)
        self.__out_command_data_ack = self._set_packet(self._COMMAND_DATA_PACKET_MAGIC)
        self.__in_response_header_buf = self._get_packet_pre_alloc()
        self.__in_response_data_buf = self._get_packet_pre_alloc()
        self._put_short_timeout_reset = 2
        self._get_short_timeout_reset = 2
        self._put_long_timeout = 5000
        self._get_long_timeout = 5000

    def __get_command(self, timeout): # private
        self._put_short_timeout = self._put_short_timeout_reset
        self._get_short_timeout = self._get_short_timeout_reset
        start = int(time.time() * 1000)
        while (int(time.time() * 1000) - start) < timeout:
            gc.collect() # Avoid collection during the transfer.
            self._zero(self.__in_command_header_buf[0], len(self.__in_command_header_buf[0]))
            self._flush()
            packet = self._get_packet(self._COMMAND_HEADER_PACKET_MAGIC, self.__in_command_header_buf, self._get_short_timeout)
            if packet is not None:
                command, datalen = struct.unpack("<II", packet)
                in_command_data_buf = self._get_packet_pre_alloc(datalen)
                self.put_bytes(self.__out_command_header_ack, self._put_short_timeout)
                dat_packet = self._get_packet(self._COMMAND_DATA_PACKET_MAGIC, in_command_data_buf, self._get_long_timeout)
                if dat_packet is not None:
                    self.put_bytes(self.__out_command_data_ack, self._put_short_timeout)
                    return (command, dat_packet)
            # Avoid timeout livelocking.
            self._put_short_timeout = min(self._put_short_timeout + 1, timeout)
            self._get_short_timeout = min(self._get_short_timeout + 1, timeout)
        return (None, None)

    def __put_result(self, data, timeout): # private
        self._put_short_timeout = self._put_short_timeout_reset
        self._get_short_timeout = self._get_short_timeout_reset
        out_header = self._set_packet(self._RESULT_HEADER_PACKET_MAGIC, struct.pack("<I", len(data)))
        out_data = self._set_packet(self._RESULT_DATA_PACKET_MAGIC, data)
        start = int(time.time() * 1000)
        while (int(time.time() * 1000) - start) < timeout:
            gc.collect() # Avoid collection during the transfer.
            self._zero(self.__in_response_header_buf[0], len(self.__in_response_header_buf[0]))
            self._zero(self.__in_response_data_buf[0], len(self.__in_response_data_buf[0]))
            self._flush()
            if self._get_packet(self._RESULT_HEADER_PACKET_MAGIC, self.__in_response_header_buf, self._get_short_timeout) is not None:
                self.put_bytes(out_header, self._put_short_timeout)
                if self._get_packet(self._RESULT_DATA_PACKET_MAGIC, self.__in_response_data_buf, self._get_short_timeout) is not None:
                    self.put_bytes(out_data, self._put_long_timeout)
                    return True
            # Avoid timeout livelocking.
            self._put_short_timeout = min(self._put_short_timeout + 1, timeout)
            self._get_short_timeout = min(self._get_short_timeout + 1, timeout)
        return False

    def register_callback(self, cb): # public
        self.__dict[self._hash(cb.__name__, len(cb.__name__))] = cb

    def schedule_callback(self, cb): # public
        self.__schedule_cb = cb

    def setup_loop_callback(self, cb): # public
        self.__loop_cb = cb

    def loop(self, recv_timeout=1000, send_timeout=1000): # public
        while True:
            command, data = self.__get_command(recv_timeout)
            if command is not None:
                cb = self.__dict.get(command)
                if self.__put_result(cb(data) if cb is not None else bytes(), send_timeout) and self.__schedule_cb is not None:
                    self.__schedule_cb()
                self.__schedule_cb = None
            if self.__loop_cb is not None: self.__loop_cb()

class rpc_usb_vcp_master(rpc_master):

    # We need to do reads this way so that we get a short timeout while waiting for data and then
    # no timeout while data is coming in. pyserial inter_byte_timeout does not work.
    def __get_bytes(self, buff):
        i = 0
        l = len(buff)
        while l:
            data = self.__ser.read(min(l, 1024)) # Starts a new timeout per call.
            data_len = len(data)
            if not data_len: return None
            buff[i:i+data_len] = data
            i += data_len
            l -= data_len
        return buff

    def __init__(self, port): # private
        self.__ser = serial.Serial(port, baudrate=115200, timeout=0.01)
        rpc_master.__init__(self)

    def _flush(self): # protected
        self.__ser.reset_input_buffer()

    def get_bytes(self, buff, timeout_ms): # protected
        if int(self.__ser.timeout * 100) != 1: self.__ser.timeout = 0.01 # Changing this causes control transfers.
        result = self.__get_bytes(buff)
        if result is None: time.sleep(timeout_ms * 0.001)
        return result

    def put_bytes(self, data, timeout_ms): # protected
        self.__ser.write(data)

    def _stream_get_bytes(self, buff, timeout_ms): # protected
        if int(self.__ser.timeout) != 1: self.__ser.timeout = 1 # Changing this causes control transfers.
        return self.__get_bytes(buff)

class rpc_usb_vcp_slave(rpc_slave):

    # We need to do reads this way so that we get a short timeout while waiting for data and then
    # no timeout while data is coming in. pyserial inter_byte_timeout does not work.
    def __get_bytes(self, buff):
        i = 0
        l = len(buff)
        while l:
            data = self.__ser.read(min(l, 1024)) # Starts a new timeout per call.
            data_len = len(data)
            if not data_len: return None
            buff[i:i+data_len] = data
            i += data_len
            l -= data_len
        return buff

    def __init__(self, port): # private
        self.__ser = serial.Serial(port, baudrate=115200, timeout=0.01)
        rpc_slave.__init__(self)

    def _flush(self): # protected
        self.__ser.reset_input_buffer()

    def get_bytes(self, buff, timeout_ms): # protected
        if int(self.__ser.timeout * 100) != 1: self.__ser.timeout = 0.01 # Changing this causes control transfers.
        return self.__get_bytes(buff)

    def put_bytes(self, data, timeout_ms): # protected
        self.__ser.write(data)

    def _stream_get_bytes(self, buff, timeout_ms): # protected
        if int(self.__ser.timeout) != 1: self.__ser.timeout = 1 # Changing this causes control transfers.
        return self.__get_bytes(buff)

class rpc_wifi_or_ethernet_master(rpc_master):

    def __valid_tcp_socket(self): # private
        if self.__tcp__socket is None:
            try:
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.bind(self.__myaddr)
                s.listen(0)
                s.settimeout(1)
                self.__tcp__socket, addr = s.accept()
                s.close()
            except (socket.timeout, socket.error): self.__tcp__socket = None
        return self.__tcp__socket is not None

    def __close_tcp_socket(self): # private
        self.__tcp__socket.close()
        self.__tcp__socket = None

    def __valid_udp_socket(self): # private
        if self.__udp__socket is None:
            try:
                self.__udp__socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                self.__udp__socket.bind(self.__myaddr)
            except (socket.timeout, socket.error): self.__udp__socket = None
        return self.__udp__socket is not None

    def __close_udp_socket(self): # private
        self.__udp__socket.close()
        self.__udp__socket = None

    def __init__(self, slave_ip, my_ip="", port=0x1DBA): # private
        self._udp_limit = 1400
        self._timeout_scale = 10
        self.__myip = my_ip
        self.__myaddr = (self.__myip, port)
        self.__slave_addr = (slave_ip, port)
        self.__tcp__socket = None
        self.__udp__socket = None
        print("IP Address:Port %s:%d\nRunning..." % self.__myaddr)
        rpc_master.__init__(self)

    def _flush(self): # protected
        if self.__valid_udp_socket():
            try:
                self.__udp__socket.settimeout(0.001)
                while(True):
                    data, addr = self.__udp__socket.recvfrom(1400)
                    if not len(data): break
            except socket.timeout: pass
            except socket.error as err: self.__close_udp_socket()
        if self.__tcp__socket is not None:
            try:
                self.__tcp__socket.settimeout(0.001)
                while(True):
                    data = self.__tcp__socket.recvfrom(1400)
                    if not len(data): break
            except socket.timeout: pass
            except socket.error as err: self.__close_tcp_socket()

    def get_bytes(self, buff, timeout_ms): # protected
        i = 0
        l = len(buff)
        if l <= self._udp_limit:
            if self.__valid_udp_socket():
                try:
                    self.__udp__socket.settimeout(self._get_short_timeout * 0.001 * self._timeout_scale)
                    while l:
                        data, addr = self.__udp__socket.recvfrom(min(l, 1400))
                        data_len = len(data)
                        if not data_len: break
                        buff[i:i+data_len] = data
                        i += data_len
                        l -= data_len
                    # We don't need to close the socket on error since it's connectionless.
                except socket.timeout: pass
                except socket.error: self.__close_udp_socket()
        elif self.__valid_tcp_socket():
            try:
                self.__tcp__socket.settimeout(100)
                while l:
                    data = self.__tcp__socket.recv(min(l, 1400))
                    data_len = len(data)
                    if not data_len: break
                    buff[i:i+data_len] = data
                    i += data_len
                    l -= data_len
                if l: self.__close_tcp_socket()
            except (socket.timeout, socket.error): self.__close_tcp_socket()
        return buff if not l else None

    def put_bytes(self, data, timeout_ms): # protected
        i = 0
        l = len(data)
        if l <= self._udp_limit:
            if self.__valid_udp_socket():
                try:
                    self.__udp__socket.settimeout(self._put_short_timeout * 0.001 * self._timeout_scale)
                    while l:
                        data_len = self.__udp__socket.sendto(data[i:i+min(l, 1400)], self.__slave_addr)
                        if not data_len: break
                        i += data_len
                        l -= data_len
                    if l: self.__close_udp_socket()
                except (socket.timeout, socket.error): self.__close_udp_socket()
        elif self.__valid_tcp_socket():
            try:
                self.__tcp__socket.settimeout(100)
                while l:
                    data_len = self.__tcp_socket.send(data[i:i+min(l, 1400)])
                    if not data_len: break
                    i += data_len
                    l -= data_len
                if l: self.__close_tcp_socket()
            except (socket.timeout, socket.error): self.__close_tcp_socket()

    def _stream_get_bytes(self, buff, timeout_ms): # protected
        i = 0
        l = len(buff)
        if self.__valid_tcp_socket():
            try:
                self.__tcp__socket.settimeout(timeout_ms * 0.001)
                while l:
                    data = self.__tcp__socket.recv(min(l, 1400))
                    data_len = len(data)
                    if not data_len: break
                    buff[i:i+data_len] = data
                    i += data_len
                    l -= data_len
                if l: self.__close_tcp_socket()
            except (socket.timeout, socket.error): self.__close_tcp_socket()
        return buff if not l else None

    def _stream_put_bytes(self, data, timeout_ms): # protected
        i = 0
        l = len(data)
        if self.__valid_tcp_socket():
            try:
                self.__tcp__socket.settimeout(timeout_ms * 0.001)
                while l:
                    data_len = self.__tcp__socket.send(data[i:i+min(l, 1400)])
                    if not data_len: break
                    i += data_len
                    l -= data_len
                if l: self.__close_tcp_socket()
            except (socket.timeout, socket.error): self.__close_tcp_socket()
        if l: raise OSError # Stop Stream.

class rpc_wifi_or_ethernet_slave(rpc_slave):

    def __valid_tcp_socket(self): # private
        if self.__tcp__socket is None:
            try:
                self.__tcp__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.__tcp__socket.connect(self.__master_addr)
            except (socket.timeout, socket.error): self.__tcp__socket = None
        return self.__tcp__socket is not None

    def __close_tcp_socket(self): # private
        self.__tcp__socket.close()
        self.__tcp__socket = None

    def __valid_udp_socket(self): # private
        if self.__udp__socket is None:
            try:
                self.__udp__socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                self.__udp__socket.bind(self.__myaddr)
            except (socket.timeout, socket.error): self.__udp__socket = None
        return self.__udp__socket is not None

    def __close_udp_socket(self): # private
        self.__udp__socket.close()
        self.__udp__socket = None

    def __init__(self, my_ip="", port=0x1DBA): # private
        self._udp_limit = 1400
        self._timeout_scale = 10
        self.__myip = my_ip
        self.__myaddr = (self.__myip, port)
        self.__master_addr = None
        self.__tcp__socket = None
        self.__udp__socket = None
        print("IP Address:Port %s:%d\nRunning..." % self.__myaddr)
        rpc_slave.__init__(self)

    def _flush(self): # protected
        if self.__valid_udp_socket():
            try:
                self.__udp__socket.settimeout(0.001)
                while(True):
                    data, addr = self.__udp__socket.recvfrom(1400)
                    if not len(data): break
            except socket.timeout: pass
            except socket.error: self.__close_udp_socket()
        if self.__tcp__socket is not None:
            try:
                self.__tcp__socket.settimeout(0.001)
                while(True):
                    data = self.__tcp__socket.recvfrom(1400)
                    if not len(data): break
            except socket.timeout: pass
            except socket.error: self.__close_tcp_socket()

    def get_bytes(self, buff, timeout_ms): # protected
        i = 0
        l = len(buff)
        if l <= self._udp_limit:
            if self.__valid_udp_socket():
                try:
                    self.__udp__socket.settimeout(self._get_short_timeout * 0.001 * self._timeout_scale)
                    while l:
                        data, addr = self.__udp__socket.recvfrom(min(l, 1400))
                        data_len = len(data)
                        if not data_len: break
                        buff[i:i+data_len] = data
                        self.__master_addr = addr
                        i += data_len
                        l -= data_len
                    # We don't need to close the socket on error since it's connectionless.
                except socket.timeout: pass
                except socket.error: self.__close_udp_socket()
        elif self.__valid_tcp_socket():
            try:
                self.__tcp__socket.settimeout(100)
                while l:
                    data = self.__tcp__socket.recv(min(l, 1400))
                    data_len = len(data)
                    if not data_len: break
                    buff[i:i+data_len] = data
                    i += data_len
                    l -= data_len
                if l: self.__close_tcp_socket()
            except (socket.timeout, socket.error): self.__close_tcp_socket()
        return buff if not l else None

    def put_bytes(self, data, timeout_ms): # protected
        i = 0
        l = len(data)
        if l <= self._udp_limit:
            if self.__valid_udp_socket():
                try:
                    self.__udp__socket.settimeout(self._put_short_timeout * 0.001 * self._timeout_scale)
                    while l:
                        data_len = self.__udp__socket.sendto(data[i:i+min(l, 1400)], self.__master_addr)
                        if not data_len: break
                        i += data_len
                        l -= data_len
                    if l: self.__close_udp_socket()
                except (socket.timeout, socket.error): self.__close_udp_socket()
        elif self.__valid_tcp_socket():
            try:
                self.__tcp__socket.settimeout(100)
                while l:
                    data_len = self.__tcp__socket.send(data[i:i+min(l, 1400)])
                    if not data_len: break
                    i += data_len
                    l -= data_len
                if l: self.__close_tcp_socket()
            except (socket.timeout, socket.error): self.__close_tcp_socket()

    def _stream_get_bytes(self, buff, timeout_ms): # protected
        i = 0
        l = len(buff)
        if self.__valid_tcp_socket():
            try:
                self.__tcp__socket.settimeout(timeout_ms * 0.001)
                while l:
                    data = self.__tcp__socket.recv(min(l, 1400))
                    data_len = len(data)
                    if not data_len: break
                    buff[i:i+data_len] = data
                    i += data_len
                    l -= data_len
                if l: self.__close_tcp_socket()
            except (socket.timeout, socket.error): self.__close_tcp_socket()
        return buff if not l else None

    def _stream_put_bytes(self, data, timeout_ms): # protected
        i = 0
        l = len(data)
        if self.__valid_tcp_socket():
            try:
                self.__tcp__socket.settimeout(timeout_ms * 0.001)
                while l:
                    data_len = self.__tcp__socket.send(data[i:i+min(l, 1400)])
                    if not data_len: break
                    i += data_len
                    l -= data_len
                if l: self.__close_tcp_socket()
            except (socket.timeout, socket.error): self.__close_tcp_socket()
        if l: raise OSError # Stop Stream.
