import struct

class ModbusRTU():
    def __init__(self, uart, slave_id=0x01, register_num=30):
        self.SLAVE_ID = slave_id
        self.uart = uart
        self.register_num = register_num
        self.REGISTER = [0]*self.register_num
        self.CRC16_TABLE = [
            0x0000,0xC0C1,0xC181,0x0140,0xC301,0x03C0,0x0280,0xC241,0xC601,
            0x06C0,0x0780,0xC741,0x0500,0xC5C1,0xC481,0x0440,0xCC01,0x0CC0,
            0x0D80,0xCD41,0x0F00,0xCFC1,0xCE81,0x0E40,0x0A00,0xCAC1,0xCB81,
            0x0B40,0xC901,0x09C0,0x0880,0xC841,0xD801,0x18C0,0x1980,0xD941,
            0x1B00,0xDBC1,0xDA81,0x1A40,0x1E00,0xDEC1,0xDF81,0x1F40,0xDD01,
            0x1DC0,0x1C80,0xDC41,0x1400,0xD4C1,0xD581,0x1540,0xD701,0x17C0,
            0x1680,0xD641,0xD201,0x12C0,0x1380,0xD341,0x1100,0xD1C1,0xD081,
            0x1040,0xF001,0x30C0,0x3180,0xF141,0x3300,0xF3C1,0xF281,0x3240,
            0x3600,0xF6C1,0xF781,0x3740,0xF501,0x35C0,0x3480,0xF441,0x3C00,
            0xFCC1,0xFD81,0x3D40,0xFF01,0x3FC0,0x3E80,0xFE41,0xFA01,0x3AC0,
            0x3B80,0xFB41,0x3900,0xF9C1,0xF881,0x3840,0x2800,0xE8C1,0xE981,
            0x2940,0xEB01,0x2BC0,0x2A80,0xEA41,0xEE01,0x2EC0,0x2F80,0xEF41,
            0x2D00,0xEDC1,0xEC81,0x2C40,0xE401,0x24C0,0x2580,0xE541,0x2700,
            0xE7C1,0xE681,0x2640,0x2200,0xE2C1,0xE381,0x2340,0xE101,0x21C0,
            0x2080,0xE041,0xA001,0x60C0,0x6180,0xA141,0x6300,0xA3C1,0xA281,
            0x6240,0x6600,0xA6C1,0xA781,0x6740,0xA501,0x65C0,0x6480,0xA441,
            0x6C00,0xACC1,0xAD81,0x6D40,0xAF01,0x6FC0,0x6E80,0xAE41,0xAA01,
            0x6AC0,0x6B80,0xAB41,0x6900,0xA9C1,0xA881,0x6840,0x7800,0xB8C1,
            0xB981,0x7940,0xBB01,0x7BC0,0x7A80,0xBA41,0xBE01,0x7EC0,0x7F80,
            0xBF41,0x7D00,0xBDC1,0xBC81,0x7C40,0xB401,0x74C0,0x7580,0xB541,
            0x7700,0xB7C1,0xB681,0x7640,0x7200,0xB2C1,0xB381,0x7340,0xB101,
            0x71C0,0x7080,0xB041,0x5000,0x90C1,0x9181,0x5140,0x9301,0x53C0,
            0x5280,0x9241,0x9601,0x56C0,0x5780,0x9741,0x5500,0x95C1,0x9481,
            0x5440,0x9C01,0x5CC0,0x5D80,0x9D41,0x5F00,0x9FC1,0x9E81,0x5E40,
            0x5A00,0x9AC1,0x9B81,0x5B40,0x9901,0x59C0,0x5880,0x9841,0x8801,
            0x48C0,0x4980,0x8941,0x4B00,0x8BC1,0x8A81,0x4A40,0x4E00,0x8EC1,
            0x8F81,0x4F40,0x8D01,0x4DC0,0x4C80,0x8C41,0x4400,0x84C1,0x8581,
            0x4540,0x8701,0x47C0,0x4680,0x8641,0x8201,0x42C0,0x4380,0x8341,
            0x4100,0x81C1,0x8081,0x4040]
    def any(self):
        return self.uart.any()
    def clear(self):
        self.REGISTER = [0]*self.register_num
    def crc16(self, data):
        crc = 0xFFFF
        for char in data:
            crc = (crc >> 8) ^ self.CRC16_TABLE[((crc) ^ char) & 0xFF]
        return struct.pack('<H',crc)
    def handle(self, debug = False):
        REQUEST = self.uart.read()
        if debug:
            print("GOT REQUEST: ", REQUEST)
        additional_address = REQUEST[0]
        error_check = REQUEST[-2:]
        function_code = REQUEST[1]
        data = REQUEST[2:]
        RESPONSE = struct.pack('b', self.SLAVE_ID)
        if self.crc16(REQUEST[:-2]) != error_check:
            if debug:
                print("crc not match")
            return 0 # do nothing
        if self.SLAVE_ID != additional_address:
            if debug:
                print("OpenMV slave id: ", self.SLAVE_ID)
                print("got cmd id: ", additional_address)
            return 0 # do nothing
        if function_code == 0x03:
            starting_address = struct.unpack('>h', data[:2])[0]
            quantity_of_registers = struct.unpack('>h', data[2:4])[0]
            response_registers = []
            try:
                for i in range(starting_address, starting_address+quantity_of_registers):
                    response_registers.append(self.REGISTER[i])
            except IndexError as err:
                RESPONSE += struct.pack('b', function_code|0x80)
                RESPONSE += struct.pack('b', 0x02) # Illegal Data Address
                if debug:
                    print("Illegal Data Address: ")
                    print(err)
            else:
                RESPONSE += struct.pack('b', function_code)
                RESPONSE += struct.pack('b', 2*quantity_of_registers)
                for b in response_registers:
                    RESPONSE += struct.pack('>h', b)
        elif function_code == 0x06:
            register_address = struct.unpack('>h', data[:2])[0]
            register_value = struct.unpack('>h', data[2:4])[0]
            try:
                self.REGISTER[register_address] = register_value
            except IndexError as err:
                RESPONSE += struct.pack('b', function_code|0x80)
                RESPONSE += struct.pack('b', 0x02) # Illegal Data Address
                if debug:
                    print("Illegal Data Address: ")
                    print(err)
            else:
                RESPONSE += struct.pack('b', function_code)
                RESPONSE += struct.pack('>h', register_address)
                RESPONSE += struct.pack('>h', self.REGISTER[register_address])
        elif function_code == 0x10:
            starting_address = struct.unpack('>h', data[:2])[0]
            quantity_of_registers = struct.unpack('>h', data[2:4])[0]
            byte_of_registers = struct.unpack('b', data[4:5])[0]
            try:
                if byte_of_registers != 2*quantity_of_registers:
                    raise struct.error
                values = struct.unpack('>%dh'%quantity_of_registers, data[5:])
            except struct.error as err:
                RESPONSE += struct.pack('b', function_code|0x80)
                RESPONSE += struct.pack('b', 0x03) # Illegal Data Value
                if debug:
                    print("Illegal Data Value, data length error")
            else:
                try:
                    for i in range(quantity_of_registers):
                        self.REGISTER[starting_address+i] = values[i]
                except IndexError as err:
                    RESPONSE += struct.pack('b', function_code|0x80)
                    RESPONSE += struct.pack('b', 0x02) # Illegal Data Address
                    if debug:
                        print("Illegal Data Address: ")
                        print(err)
                else:
                    RESPONSE += struct.pack('b', function_code)
                    RESPONSE += struct.pack('>h', starting_address)
                    RESPONSE += struct.pack('>h', quantity_of_registers)
        else:
            RESPONSE += struct.pack('b', function_code|0x80)
            RESPONSE += struct.pack('b', 0x01) # Illegal Function
        RESPONSE += self.crc16(RESPONSE)
        if debug:
            print("FUNCTION CODE: ", function_code)
            print("RESPONSE: ", RESPONSE)
        self.uart.write(RESPONSE)
