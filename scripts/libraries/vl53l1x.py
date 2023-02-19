import pyb

VL51L1X_DEFAULT_CONFIGURATION = bytes(
    [
        0x00,  # 0x2d : set bit 2 and 5 to 1 for fast plus mode (1MHz I2C), else don't touch
        0x00,  # 0x2e : bit 0 if I2C pulled up at 1.8V, else set bit 0 to 1 (pull up at AVDD)
        0x00,  # 0x2f : bit 0 if GPIO pulled up at 1.8V, else set bit 0 to 1 (pull up at AVDD)
        0x01,  # 0x30 : set bit 4 to 0 for active high interrupt and 1 for active low
               # (bits 3:0 must be 0x1), use SetInterruptPolarity()
        0x02,  # 0x31 : bit 1 = interrupt depending on the polarity, use CheckForDataReady()
        0x00,  # 0x32 : not user-modifiable
        0x02,  # 0x33 : not user-modifiable
        0x08,  # 0x34 : not user-modifiable
        0x00,  # 0x35 : not user-modifiable
        0x08,  # 0x36 : not user-modifiable
        0x10,  # 0x37 : not user-modifiable
        0x01,  # 0x38 : not user-modifiable
        0x01,  # 0x39 : not user-modifiable
        0x00,  # 0x3a : not user-modifiable
        0x00,  # 0x3b : not user-modifiable
        0x00,  # 0x3c : not user-modifiable
        0x00,  # 0x3d : not user-modifiable
        0xFF,  # 0x3e : not user-modifiable
        0x00,  # 0x3f : not user-modifiable
        0x0F,  # 0x40 : not user-modifiable
        0x00,  # 0x41 : not user-modifiable
        0x00,  # 0x42 : not user-modifiable
        0x00,  # 0x43 : not user-modifiable
        0x00,  # 0x44 : not user-modifiable
        0x00,  # 0x45 : not user-modifiable
        0x20,  # 0x46 : interrupt configuration 0->level low detection,
               # 1-> level high, 2-> Out of window, 3->In window, 0x20-> New sample ready , TBC
        0x0B,  # 0x47 : not user-modifiable
        0x00,  # 0x48 : not user-modifiable
        0x00,  # 0x49 : not user-modifiable
        0x02,  # 0x4a : not user-modifiable
        0x0A,  # 0x4b : not user-modifiable
        0x21,  # 0x4c : not user-modifiable
        0x00,  # 0x4d : not user-modifiable
        0x00,  # 0x4e : not user-modifiable
        0x05,  # 0x4f : not user-modifiable
        0x00,  # 0x50 : not user-modifiable
        0x00,  # 0x51 : not user-modifiable
        0x00,  # 0x52 : not user-modifiable
        0x00,  # 0x53 : not user-modifiable
        0xC8,  # 0x54 : not user-modifiable
        0x00,  # 0x55 : not user-modifiable
        0x00,  # 0x56 : not user-modifiable
        0x38,  # 0x57 : not user-modifiable
        0xFF,  # 0x58 : not user-modifiable
        0x01,  # 0x59 : not user-modifiable
        0x00,  # 0x5a : not user-modifiable
        0x08,  # 0x5b : not user-modifiable
        0x00,  # 0x5c : not user-modifiable
        0x00,  # 0x5d : not user-modifiable
        0x01,  # 0x5e : not user-modifiable
        0xDB,  # 0x5f : not user-modifiable
        0x0F,  # 0x60 : not user-modifiable
        0x01,  # 0x61 : not user-modifiable
        0xF1,  # 0x62 : not user-modifiable
        0x0D,  # 0x63 : not user-modifiable
        0x01,  # 0x64 : Sigma threshold MSB (mm in 14.2 format for MSB+LSB),
               # use SetSigmaThreshold(), default value 90 mm
        0x68,  # 0x65 : Sigma threshold LSB
        0x00,  # 0x66 : Min count Rate MSB (MCPS in 9.7 format for MSB+LSB), use SetSignalThreshold()
        0x80,  # 0x67 : Min count Rate LSB
        0x08,  # 0x68 : not user-modifiable
        0xB8,  # 0x69 : not user-modifiable
        0x00,  # 0x6a : not user-modifiable
        0x00,  # 0x6b : not user-modifiable
        0x00,  # 0x6c : Intermeasurement period MSB, 32 bits register, use SetIntermeasurementInMs()
        0x00,  # 0x6d : Intermeasurement period
        0x0F,  # 0x6e : Intermeasurement period
        0x89,  # 0x6f : Intermeasurement period LSB
        0x00,  # 0x70 : not user-modifiable
        0x00,  # 0x71 : not user-modifiable
        0x00,  # 0x72 : distance threshold high MSB (in mm, MSB+LSB), use SetD:tanceThreshold()
        0x00,  # 0x73 : distance threshold high LSB
        0x00,  # 0x74 : distance threshold low MSB ( in mm, MSB+LSB), use SetD:tanceThreshold()
        0x00,  # 0x75 : distance threshold low LSB
        0x00,  # 0x76 : not user-modifiable
        0x01,  # 0x77 : not user-modifiable
        0x0F,  # 0x78 : not user-modifiable
        0x0D,  # 0x79 : not user-modifiable
        0x0E,  # 0x7a : not user-modifiable
        0x0E,  # 0x7b : not user-modifiable
        0x00,  # 0x7c : not user-modifiable
        0x00,  # 0x7d : not user-modifiable
        0x02,  # 0x7e : not user-modifiable
        0xC7,  # 0x7f : ROI center, use SetROI()
        0xFF,  # 0x80 : XY ROI (X=Width, Y=Height), use SetROI()
        0x9B,  # 0x81 : not user-modifiable
        0x00,  # 0x82 : not user-modifiable
        0x00,  # 0x83 : not user-modifiable
        0x00,  # 0x84 : not user-modifiable
        0x01,  # 0x85 : not user-modifiable
        0x01,  # 0x86 : clear interrupt, use ClearInterrupt()
        0x40,  # 0x87 : start ranging, use StartRanging() or StopRanging(), If you want
               # an automatic start after VL53L1X_init() call, put 0x40 in location 0x87
    ]
)


class VL53L1X:
    def __init__(self, i2c, address=0x29):
        self.i2c = i2c
        self.address = address
        self.reset()
        pyb.delay(1)
        if self.read_model_id() != 0xEACC:
            raise RuntimeError("Failed to find expected ID register values. Check wiring!")
        # write default configuration
        self.i2c.writeto_mem(self.address, 0x2D, VL51L1X_DEFAULT_CONFIGURATION, addrsize=16)
        # pyb.delay(100)
        # the API triggers this change in VL53L1_init_and_start_range() once a
        # measurement is started; assumes MM1 and MM2 are disabled
        self.writeReg16Bit(0x001E, self.readReg16Bit(0x0022) * 4)
        pyb.delay(200)

    def writeReg(self, reg, value):
        return self.i2c.writeto_mem(self.address, reg, bytes([value]), addrsize=16)

    def writeReg16Bit(self, reg, value):
        return self.i2c.writeto_mem(
            self.address, reg, bytes([(value >> 8) & 0xFF, value & 0xFF]), addrsize=16
        )

    def readReg(self, reg):
        return self.i2c.readfrom_mem(self.address, reg, 1, addrsize=16)[0]

    def readReg16Bit(self, reg):
        data = self.i2c.readfrom_mem(self.address, reg, 2, addrsize=16)
        return (data[0] << 8) + data[1]

    def read_model_id(self):
        return self.readReg16Bit(0x010F)

    def reset(self):
        self.writeReg(0x0000, 0x00)
        pyb.delay(100)
        self.writeReg(0x0000, 0x01)

    def read(self):
        data = self.i2c.readfrom_mem(self.address, 0x0089, 17, addrsize=16)  # RESULT__RANGE_STATUS
        # range_status = data[0]
        # report_status = data[1]
        # stream_count = data[2]
        # dss_actual_effective_spads_sd0 = (data[3] << 8) + data[4]
        # peak_signal_count_rate_mcps_sd0 = (data[5]<<8) + data[6]
        # ambient_count_rate_mcps_sd0 = (data[7] << 8) + data[8]
        # sigma_sd0 = (data[9]<<8) + data[10]
        # phase_sd0 = (data[11]<<8) + data[12]
        final_crosstalk_corrected_range_mm_sd0 = (data[13] << 8) + data[14]
        # peak_signal_count_rate_crosstalk_corrected_mcps_sd0 = (data[15] << 8) + data[16]
        # status = None
        # if range_status in (17, 2, 1, 3):
        # status = "HardwareFail"
        # elif range_status == 13:
        # status = "MinRangeFail"
        # elif range_status == 18:
        # status = "SynchronizationInt"
        # elif range_status == 5:
        # status = "OutOfBoundsFail"
        # elif range_status == 4:
        # status = "SignalFail"
        # elif range_status == 6:
        # status = "SignalFail"
        # elif range_status == 7:
        # status = "WrapTargetFail"
        # elif range_status == 12:
        # status = "XtalkSignalFail"
        # elif range_status == 8:
        # status = "RangeValidMinRangeClipped"
        # elif range_status == 9:
        # if stream_count == 0:
        # status = "RangeValidNoWrapCheckFail"
        # else:
        # status = "OK"
        return final_crosstalk_corrected_range_mm_sd0
