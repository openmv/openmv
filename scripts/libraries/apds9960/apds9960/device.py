from apds9960.const import *
from apds9960.exceptions import *

from time import sleep_ms

class APDS9960:
    class GestureData:
        def __init__(self):
            self.u_data = [0] * 32
            self.d_data = [0] * 32
            self.l_data = [0] * 32
            self.r_data = [0] * 32
            self.index = 0
            self.total_gestures = 0
            self.in_threshold = 0
            self.out_threshold = 0

    def __init__(self, bus, address=APDS9960_I2C_ADDR, valid_id=APDS9960_DEV_ID):
        # I2C stuff
        self.address = address
        self.bus = bus

        # instance variables for gesture detection
        self.gesture_ud_delta_ = 0
        self.gesture_lr_delta_ = 0

        self.gesture_ud_count_ = 0
        self.gesture_lr_count_ = 0

        self.gesture_near_count_ = 0
        self.gesture_far_count_ = 0

        self.gesture_state_ = 0
        self.gesture_motion_ = APDS9960_DIR_NONE

        self.gesture_data_ = APDS9960.GestureData()

        # check device id
        self.dev_id = self._read_byte_data(APDS9960_REG_ID)
        if not self.dev_id in valid_id:
            raise ADPS9960InvalidDevId(self.dev_id, valid_id)

        # disable all features
        self.setMode(APDS9960_MODE_ALL, False)

        # set default values for ambient light and proximity registers
        self._write_byte_data(APDS9960_REG_ATIME, APDS9960_DEFAULT_ATIME)
        self._write_byte_data(APDS9960_REG_WTIME, APDS9960_DEFAULT_WTIME)
        self._write_byte_data(APDS9960_REG_PPULSE, APDS9960_DEFAULT_PROX_PPULSE)
        self._write_byte_data(APDS9960_REG_POFFSET_UR, APDS9960_DEFAULT_POFFSET_UR)
        self._write_byte_data(APDS9960_REG_POFFSET_DL, APDS9960_DEFAULT_POFFSET_DL)
        self._write_byte_data(APDS9960_REG_CONFIG1, APDS9960_DEFAULT_CONFIG1)
        self.setLEDDrive(APDS9960_DEFAULT_LDRIVE)
        self.setProximityGain(APDS9960_DEFAULT_PGAIN)
        self.setAmbientLightGain(APDS9960_DEFAULT_AGAIN)
        self.setProxIntLowThresh(APDS9960_DEFAULT_PILT)
        self.setProxIntHighThresh(APDS9960_DEFAULT_PIHT)
        self.setLightIntLowThreshold(APDS9960_DEFAULT_AILT)
        self.setLightIntHighThreshold(APDS9960_DEFAULT_AIHT)

        self._write_byte_data(APDS9960_REG_PERS, APDS9960_DEFAULT_PERS)
        self._write_byte_data(APDS9960_REG_CONFIG2, APDS9960_DEFAULT_CONFIG2)
        self._write_byte_data(APDS9960_REG_CONFIG3, APDS9960_DEFAULT_CONFIG3)

        # set default values for gesture sense registers
        self.setGestureEnterThresh(APDS9960_DEFAULT_GPENTH)
        self.setGestureExitThresh(APDS9960_DEFAULT_GEXTH)
        self._write_byte_data(APDS9960_REG_GCONF1, APDS9960_DEFAULT_GCONF1)

        self.setGestureGain(APDS9960_DEFAULT_GGAIN)
        self.setGestureLEDDrive(APDS9960_DEFAULT_GLDRIVE)
        self.setGestureWaitTime(APDS9960_DEFAULT_GWTIME)
        self._write_byte_data(APDS9960_REG_GOFFSET_U, APDS9960_DEFAULT_GOFFSET)
        self._write_byte_data(APDS9960_REG_GOFFSET_D, APDS9960_DEFAULT_GOFFSET)
        self._write_byte_data(APDS9960_REG_GOFFSET_L, APDS9960_DEFAULT_GOFFSET)
        self._write_byte_data(APDS9960_REG_GOFFSET_R, APDS9960_DEFAULT_GOFFSET)
        self._write_byte_data(APDS9960_REG_GPULSE, APDS9960_DEFAULT_GPULSE)
        self._write_byte_data(APDS9960_REG_GCONF3, APDS9960_DEFAULT_GCONF3)
        self.setGestureIntEnable(APDS9960_DEFAULT_GIEN)


    def getMode(self):
        return self._read_byte_data(APDS9960_REG_ENABLE)

    def setMode(self, mode, enable=True):
        # read ENABLE register
        reg_val = self.getMode()

        if mode < 0 or mode > APDS9960_MODE_ALL:
            raise ADPS9960InvalidMode(mode)

        # change bit(s) in ENABLE register */
        if mode == APDS9960_MODE_ALL:
            if enable:
                reg_val = 0x7f
            else:
                reg_val = 0x00
        else:
            if enable:
                reg_val |= (1 << mode);
            else:
                reg_val &= ~(1 << mode);

        # write value to ENABLE register
        self._write_byte_data(APDS9960_REG_ENABLE, reg_val)


    # start the light (R/G/B/Ambient) sensor
    def enableLightSensor(self, interrupts=True):
        self.setAmbientLightGain(APDS9960_DEFAULT_AGAIN)
        self.setAmbientLightIntEnable(interrupts)
        self.enablePower()
        self.setMode(APDS9960_MODE_AMBIENT_LIGHT, True)

    # stop the light sensor
    def disableLightSensor(self):
        self.setAmbientLightIntEnable(False)
        self.setMode(APDS9960_MODE_AMBIENT_LIGHT, False)


    # start the proximity sensor
    def enableProximitySensor(self, interrupts=True):
        self.setProximityGain(APDS9960_DEFAULT_PGAIN)
        self.setLEDDrive(APDS9960_DEFAULT_LDRIVE)
        self.setProximityIntEnable(interrupts)
        self.enablePower()
        self.setMode(APDS9960_MODE_PROXIMITY, True)

    # stop the proximity sensor
    def disableProximitySensor(self):
        self.setProximityIntEnable(False)
        self.setMode(APDS9960_MODE_PROXIMITY, False)


    # start the gesture recognition engine
    def enableGestureSensor(self, interrupts=True):
        self.resetGestureParameters()
        self._write_byte_data(APDS9960_REG_WTIME, 0xff)
        self._write_byte_data(APDS9960_REG_PPULSE, APDS9960_DEFAULT_GESTURE_PPULSE)
        self.setLEDBoost(APDS9960_LED_BOOST_300)
        self.setGestureIntEnable(interrupts)
        self.setGestureMode(True)
        self.enablePower()
        self.setMode(APDS9960_MODE_WAIT, True)
        self.setMode(APDS9960_MODE_PROXIMITY, True)
        self.setMode(APDS9960_MODE_GESTURE, True)

    # stop the gesture recognition engine
    def disableGestureSensor(self):
        self.resetGestureParameters()
        self.setGestureIntEnable(False)
        self.setGestureMode(False)
        self.setMode(APDS9960_MODE_GESTURE, False)


    # check if there is a gesture available
    def isGestureAvailable(self):
        val = self._read_byte_data(APDS9960_REG_GSTATUS)

        # shift and mask out GVALID bit
        val &= APDS9960_BIT_GVALID;

        return (val == APDS9960_BIT_GVALID)


    # processes a gesture event and returns best guessed gesture
    def readGesture(self):
        fifo_level = 0
        bytes_read = 0
        fifo_data = []

        # make sure that power and gesture is on and data is valid
        if not (self.getMode() & 0b01000001) or not self.isGestureAvailable():
            return APDS9960_DIR_NONE

        # keep looping as long as gesture data is valid
        while(self.isGestureAvailable()):
            # read the current FIFO level
            fifo_level = self._read_byte_data(APDS9960_REG_GFLVL)

            # if there's stuff in the FIFO, read it into our data block
            if fifo_level > 0:
                fifo_data = []
                for i in range(0, fifo_level):
                    fifo_data += self._read_i2c_block_data(APDS9960_REG_GFIFO_U, 4)

                # if at least 1 set of data, sort the data into U/D/L/R
                if len(fifo_data) >= 4:
                    for i in range(0, len(fifo_data), 4):
                        self.gesture_data_.u_data[self.gesture_data_.index] = fifo_data[i + 0]
                        self.gesture_data_.d_data[self.gesture_data_.index] = fifo_data[i + 1]
                        self.gesture_data_.l_data[self.gesture_data_.index] = fifo_data[i + 2]
                        self.gesture_data_.r_data[self.gesture_data_.index] = fifo_data[i + 3]
                        self.gesture_data_.index += 1
                        self.gesture_data_.total_gestures += 1

                    # filter and process gesture data, decode near/far state
                    if self.processGestureData():
                        if self.decodeGesture():
                            #***TODO: U-Turn Gestures
                            pass

                    # reset data
                    self.gesture_data_.index = 0
                    self.gesture_data_.total_gestures = 0

            # wait some time to collect next batch of FIFO data
            sleep_ms(APDS9960_TIME_FIFO_PAUSE)

        # determine best guessed gesture and clean up
        sleep_ms(APDS9960_TIME_FIFO_PAUSE)
        self.decodeGesture()
        motion = self.gesture_motion_

        self.resetGestureParameters()
        return motion


    # turn the APDS-9960 on
    def enablePower(self):
        self.setMode(APDS9960_MODE_POWER, True)


    def disablePower(self):
        # turn the APDS-9960 off
        self.setMode(APDS9960_MODE_POWER, False)



    # *******************************************************************************
    # ambient light and color sensor controls
    # *******************************************************************************

    # reads the ambient (clear) light level as a 16-bit value
    def readAmbientLight(self):
        # read value from clear channel, low byte register
        l = self._read_byte_data(APDS9960_REG_CDATAL)

        # read value from clear channel, high byte register
        h = self._read_byte_data(APDS9960_REG_CDATAH)

        return l + (h << 8)

    # reads the red light level as a 16-bit value
    def readRedLight(self):
        # read value from red channel, low byte register
        l = self._read_byte_data(APDS9960_REG_RDATAL)

        # read value from red channel, high byte register
        h = self._read_byte_data(APDS9960_REG_RDATAH)

        return l + (h << 8)

    # reads the green light level as a 16-bit value
    def readGreenLight(self):
        # read value from green channel, low byte register
        l = self._read_byte_data(APDS9960_REG_GDATAL)

        # read value from green channel, high byte register
        h = self._read_byte_data(APDS9960_REG_GDATAH)

        return l + (h << 8)

    # reads the blue light level as a 16-bit value
    def readBlueLight(self):
        # read value from blue channel, low byte register
        l = self._read_byte_data(APDS9960_REG_BDATAL)

        # read value from blue channel, high byte register
        h = self._read_byte_data(APDS9960_REG_BDATAH)

        return l + (h << 8)


    # *******************************************************************************
    # Proximity sensor controls
    # *******************************************************************************

    # reads the proximity level as an 8-bit value
    def readProximity(self):
        return self._read_byte_data(APDS9960_REG_PDATA)


    # *******************************************************************************
    # High-level gesture controls
    # *******************************************************************************

    # resets all the parameters in the gesture data member
    def resetGestureParameters(self):
        self.gesture_data_.index = 0
        self.gesture_data_.total_gestures = 0

        self.gesture_ud_delta_ = 0
        self.gesture_lr_delta_ = 0

        self.gesture_ud_count_ = 0
        self.gesture_lr_count_ = 0

        self.gesture_near_count_ = 0
        self.gesture_far_count_ = 0

        self.gesture_state_ = 0
        self.gesture_motion_ = APDS9960_DIR_NONE


    def processGestureData(self):
        """Processes the raw gesture data to determine swipe direction

            Returns:
                bool: True if near or far state seen, False otherwise.
        """
        u_first = 0
        d_first = 0
        l_first = 0
        r_first = 0
        u_last = 0
        d_last = 0
        l_last = 0
        r_last = 0

        # if we have less than 4 total gestures, that's not enough
        if self.gesture_data_.total_gestures <= 4:
            return False

        # check to make sure our data isn't out of bounds
        if self.gesture_data_.total_gestures <= 32 and self.gesture_data_.total_gestures > 0:
            # find the first value in U/D/L/R above the threshold
            for i in range(0, self.gesture_data_.total_gestures):
                if self.gesture_data_.u_data[i] > APDS9960_GESTURE_THRESHOLD_OUT and \
                    self.gesture_data_.d_data[i] > APDS9960_GESTURE_THRESHOLD_OUT and \
                    self.gesture_data_.l_data[i] > APDS9960_GESTURE_THRESHOLD_OUT and \
                    self.gesture_data_.r_data[i] > APDS9960_GESTURE_THRESHOLD_OUT:

                    u_first = self.gesture_data_.u_data[i]
                    d_first = self.gesture_data_.d_data[i]
                    l_first = self.gesture_data_.l_data[i]
                    r_first = self.gesture_data_.r_data[i]
                    break

            # if one of the _first values is 0, then there is no good data
            if u_first == 0 or  d_first == 0 or l_first == 0 or r_first == 0:
                return False

            # find the last value in U/D/L/R above the threshold
            for i in reversed(range(0, self.gesture_data_.total_gestures)):
                if self.gesture_data_.u_data[i] > APDS9960_GESTURE_THRESHOLD_OUT and \
                    self.gesture_data_.d_data[i] > APDS9960_GESTURE_THRESHOLD_OUT and \
                    self.gesture_data_.l_data[i] > APDS9960_GESTURE_THRESHOLD_OUT and \
                    self.gesture_data_.r_data[i] > APDS9960_GESTURE_THRESHOLD_OUT:

                    u_last = self.gesture_data_.u_data[i]
                    d_last = self.gesture_data_.d_data[i]
                    l_last = self.gesture_data_.l_data[i]
                    r_last = self.gesture_data_.r_data[i]
                    break

            # calculate the first vs. last ratio of up/down and left/right
            ud_ratio_first = ((u_first - d_first) * 100) / (u_first + d_first)
            lr_ratio_first = ((l_first - r_first) * 100) / (l_first + r_first)
            ud_ratio_last = ((u_last - d_last) * 100) / (u_last + d_last)
            lr_ratio_last = ((l_last - r_last) * 100) / (l_last + r_last)

            # determine the difference between the first and last ratios
            ud_delta = ud_ratio_last - ud_ratio_first
            lr_delta = lr_ratio_last - lr_ratio_first

            # accumulate the UD and LR delta values
            self.gesture_ud_delta_ += ud_delta
            self.gesture_lr_delta_ += lr_delta

            # determine U/D gesture
            if self.gesture_ud_delta_ >= APDS9960_GESTURE_SENSITIVITY_1:
                self.gesture_ud_count_ = 1
            elif self.gesture_ud_delta_ <= -APDS9960_GESTURE_SENSITIVITY_1:
                self.gesture_ud_count_ = -1
            else:
                self.gesture_ud_count_ = 0

            # determine L/R gesture
            if self.gesture_lr_delta_ >= APDS9960_GESTURE_SENSITIVITY_1:
                self.gesture_lr_count_ = 1
            elif self.gesture_lr_delta_ <= -APDS9960_GESTURE_SENSITIVITY_1:
                self.gesture_lr_count_ = -1
            else:
                self.gesture_lr_count_ = 0

            # determine Near/Far gesture
            if self.gesture_ud_count_ == 0 and self.gesture_lr_count_ == 0:
                if abs(ud_delta) < APDS9960_GESTURE_SENSITIVITY_2 and \
                    abs(lr_delta) < APDS9960_GESTURE_SENSITIVITY_2:

                    if ud_delta == 0 and lr_delta == 0:
                        self.gesture_near_count_ += 1
                    elif ud_delta != 0 or lr_delta != 0:
                        self.gesture_far_count_ += 1

                    if self.gesture_near_count_ >= 10 and self.gesture_far_count_ >= 2:
                        if ud_delta == 0 and lr_delta == 0:
                            self.gesture_state_ = APDS9960_STATE_NEAR
                        elif ud_delta != 0 and lr_delta != 0:
                            self.gesture_state_ = APDS9960_STATE_FAR
                        return True
            else:
                if abs(ud_delta) < APDS9960_GESTURE_SENSITIVITY_2 and \
                    abs(lr_delta) < APDS9960_GESTURE_SENSITIVITY_2:

                        if ud_delta == 0 and lr_delta == 0:
                            self.gesture_near_count_ += 1

                        if self.gesture_near_count_ >= 10:
                            self.gesture_ud_count_ = 0
                            self.gesture_lr_count_ = 0
                            self.gesture_ud_delta_ = 0
                            self.gesture_lr_delta_ = 0

        return False

    def decodeGesture(self):
        """Determines swipe direction or near/far state.
        """

        # return if near or far event is detected
        if self.gesture_state_ == APDS9960_STATE_NEAR:
            self.gesture_motion_ = APDS9960_DIR_NEAR
            return True

        if self.gesture_state_ == APDS9960_STATE_FAR:
            self.gesture_motion_ = APDS9960_DIR_FAR
            return True

        # determine swipe direction
        if self.gesture_ud_count_ == -1 and self.gesture_lr_count_ == 0:
            self.gesture_motion_ = APDS9960_DIR_UP
        elif self.gesture_ud_count_ == 1 and self.gesture_lr_count_ == 0:
            self.gesture_motion_ = APDS9960_DIR_DOWN
        elif self.gesture_ud_count_ == 0 and self.gesture_lr_count_ == 1:
            self.gesture_motion_ = APDS9960_DIR_RIGHT
        elif self.gesture_ud_count_ == 0 and self.gesture_lr_count_ == -1:
            self.gesture_motion_ = APDS9960_DIR_LEFT
        elif self.gesture_ud_count_ == -1 and self.gesture_lr_count_ == 1:
            if abs(self.gesture_ud_delta_) > abs(self.gesture_lr_delta_):
                self.gesture_motion_ = APDS9960_DIR_UP
            else:
                self.gesture_motion_ = APDS9960_DIR_DOWN
        elif self.gesture_ud_count_ == 1 and self.gesture_lr_count_ == -1:
            if abs(self.gesture_ud_delta_) > abs(self.gesture_lr_delta_):
                self.gesture_motion_ = APDS9960_DIR_DOWN
            else:
                self.gesture_motion_ = APDS9960_DIR_LEFT
        elif self.gesture_ud_count_ == -1 and self.gesture_lr_count_ == -1:
            if abs(self.gesture_ud_delta_) > abs(self.gesture_lr_delta_):
                self.gesture_motion_ = APDS9960_DIR_UP
            else:
                self.gesture_motion_ = APDS9960_DIR_LEFT
        elif self.gesture_ud_count_ == 1 and self.gesture_lr_count_ == 1:
            if abs(self.gesture_ud_delta_) > abs(self.gesture_lr_delta_):
                self.gesture_motion_ = APDS9960_DIR_DOWN
            else:
                self.gesture_motion_ = APDS9960_DIR_RIGHT
        else:
            return False

        return True


    # *******************************************************************************
    # Getters and setters for register values
    # *******************************************************************************

    def getProxIntLowThresh(self):
        """Returns the lower threshold for proximity detection
        """
        return self._read_byte_data(APDS9960_REG_PILT)

    def setProxIntLowThresh(self, threshold):
        """Sets the lower threshold for proximity detection.
        """
        self._write_byte_data(APDS9960_REG_PILT, threshold)


    def getProxIntHighThresh(self):
        """Returns the high threshold for proximity detection.
        """
        return self._read_byte_data(APDS9960_REG_PIHT)

    def setProxIntHighThresh(self, threshold):
        """Sets the high threshold for proximity detection.
        """
        self._write_byte_data(APDS9960_REG_PIHT, threshold)


    def getLEDDrive(self):
        """Returns LED drive strength for proximity and ALS.

            Value    LED Current
              0        100 mA
              1         50 mA
              2         25 mA
              3         12.5 mA

            Returns:
                int: the value of the LED drive strength
        """
        val = self._read_byte_data(APDS9960_REG_CONTROL)

        # shift and mask out LED drive bits
        return (val >> 6) & 0b00000011

    def setLEDDrive(self, drive):
        """Sets LED drive strength for proximity and ALS.

            Value    LED Current
              0        100 mA
              1         50 mA
              2         25 mA
              3         12.5 mA

            Args:
                drive (int): value for the LED drive strength
        """
        val = self._read_byte_data(APDS9960_REG_CONTROL)

        # set bits in register to given value
        drive &= 0b00000011
        drive = drive << 6
        val &= 0b00111111
        val |= drive

        self._write_byte_data(APDS9960_REG_CONTROL, val)


    def getProximityGain(self):
        """Returns receiver gain for proximity detection.

            Value    Gain
              0       1x
              1       2x
              2       4x
              3       8x

            Returns:
                int: the value of the proximity gain
        """
        val = self._read_byte_data(APDS9960_REG_CONTROL)

        # shift and mask out PDRIVE bits
        return (val >> 2) & 0b00000011

    def setProximityGain(self, drive):
        """Returns receiver gain for proximity detection.

            Value    Gain
              0       1x
              1       2x
              2       4x
              3       8x

            Args:
                drive (int): value for the proximity gain
        """
        val = self._read_byte_data(APDS9960_REG_CONTROL)

        # set bits in register to given value
        drive &= 0b00000011
        drive = drive << 2
        val &= 0b11110011
        val |= drive

        self._write_byte_data(APDS9960_REG_CONTROL, val)


    def getAmbientLightGain(self):
        """Returns receiver gain for the ambient light sensor (ALS).

            Value    Gain
              0       1x
              1       4x
              2       16x
              3       64x

            Returns:
                int: the value of the ALS gain
        """
        val = self._read_byte_data(APDS9960_REG_CONTROL)

        # shift and mask out ADRIVE bits
        return (val & 0b00000011)

    def setAmbientLightGain(self, drive):
        """Sets the receiver gain for the ambient light sensor (ALS).

            Value    Gain
              0       1x
              1       4x
              2       16x
              3       64x

            Args:
                drive (int): value for the ALS gain
        """
        val = self._read_byte_data(APDS9960_REG_CONTROL)

        # set bits in register to given value
        drive &= 0b00000011
        val &= 0b11111100
        val |= drive

        self._write_byte_data(APDS9960_REG_CONTROL, val)


    def getLEDBoost(self):
        """Get the current LED boost value.

            Value    Gain
              0        100%
              1        150%
              2        200%
              3        300%

            Returns:
                int: the LED boost value
        """
        val = self._read_byte_data(APDS9960_REG_CONFIG2)

        # shift and mask out LED_BOOST bits
        return (val >> 4) & 0b00000011

    def setLEDBoost(self, boost):
        """Sets the LED current boost value.

            Value    Gain
              0        100%
              1        150%
              2        200%
              3        300%

            Args:
                boost (int): value for the LED boost
        """
        val = self._read_byte_data(APDS9960_REG_CONFIG2)

        # set bits in register to given value
        boost &= 0b00000011
        boost = boost << 4
        val &= 0b11001111
        val |= boost

        self._write_byte_data(APDS9960_REG_CONFIG2, val)


    def getProxGainCompEnable(self):
        """Gets proximity gain compensation enable.

            Returns:
                bool: True if compensation is enabled, False if not
        """
        val = self._read_byte_data(APDS9960_REG_CONFIG3)

        # Shift and mask out PCMP bits
        val = (val >> 5) & 0b00000001
        return val == 1

    def setProxGainCompEnable(self, enable):
        """Sets the proximity gain compensation enable.

            Args:
                enable (bool): True to enable compensation, False to disable
        """
        val = self._read_byte_data(APDS9960_REG_CONFIG3)

        # set bits in register to given value
        val &= 0b11011111
        if enable:
            val |= 0b00100000

        self._write_byte_data(APDS9960_REG_CONFIG3, val)


    def getProxPhotoMask(self):
        """Gets the current mask for enabled/disabled proximity photodiodes.

            Bit    Photodiode
             3       UP
             2       DOWN
             1       LEFT
             0       RIGHT

            1 = disabled, 0 = enabled

            Returns:
                int: Current proximity mask for photodiodes.
        """
        val = self._read_byte_data(APDS9960_REG_CONFIG3)

        # mask out photodiode enable mask bits
        return val & 0b00001111

    def setProxPhotoMask(self, mask):
        """Sets the mask for enabling/disabling proximity photodiodes.

            Bit    Photodiode
             3       UP
             2       DOWN
             1       LEFT
             0       RIGHT

            1 = disabled, 0 = enabled

            Args:
                mask (int): 4-bit mask value
        """
        val = self._read_byte_data(APDS9960_REG_CONFIG3)

        # set bits in register to given value
        mask &= 0b00001111
        val &= 0b11110000
        val |= mask

        self._write_byte_data(APDS9960_REG_CONFIG3, val)


    def getGestureEnterThresh(self):
        """Gets the entry proximity threshold for gesture sensing.

            Returns:
                int: current entry proximity threshold
        """
        return self._read_byte_data(APDS9960_REG_GPENTH)

    def setGestureEnterThresh(self, threshold):
        """Sets the entry proximity threshold for gesture sensing.

            Args:
                threshold (int): threshold proximity value needed to start gesture mode
        """
        self._write_byte_data(APDS9960_REG_GPENTH, threshold)


    def getGestureExitThresh(self):
        """Gets the exit proximity threshold for gesture sensing.

            Returns:
                int: current exit proximity threshold
        """
        return self._read_byte_data(APDS9960_REG_GEXTH)

    def setGestureExitThresh(self, threshold):
        """Sets the exit proximity threshold for gesture sensing.

            Args:
                threshold (int): threshold proximity value needed to end gesture mode
        """
        self._write_byte_data(APDS9960_REG_GEXTH, threshold)


    def getGestureGain(self):
        """Gets the gain of the photodiode during gesture mode.

            Value    Gain
              0       1x
              1       2x
              2       4x
              3       8x

            Returns:
                int: the current photodiode gain
        """
        val = self._read_byte_data(APDS9960_REG_GCONF2)

        # shift and mask out PDRIVE bits
        return (val >> 5) & 0b00000011

    def setGestureGain(self, gain):
        """Sets the gain of the photodiode during gesture mode.

            Value    Gain
              0       1x
              1       2x
              2       4x
              3       8x

            Args:
                gain (int): the value for the photodiode gain
        """
        val = self._read_byte_data(APDS9960_REG_GCONF2)

        # set bits in register to given value
        gain &= 0b00000011
        gain = gain << 5
        val &= 0b10011111
        val |= gain

        self._write_byte_data(APDS9960_REG_GCONF2, val)


    def getGestureLEDDrive(self):
        """Gets the drive current of the LED during gesture mode.

            Value    LED Current
              0        100 mA
              1         50 mA
              2         25 mA
              3         12.5 mA

            Returns:
                int: the LED drive current value
        """
        val = self._read_byte_data(APDS9960_REG_GCONF2)

        # shift and mask out LED drive bits
        return (val >> 3) & 0b00000011

    def setGestureLEDDrive(self, drive):
        """Sets LED drive strength for proximity and ALS.

            Value    LED Current
              0        100 mA
              1         50 mA
              2         25 mA
              3         12.5 mA

            Args:
                drive (int): value for the LED drive current
        """
        val = self._read_byte_data(APDS9960_REG_GCONF2)

        # set bits in register to given value
        drive &= 0b00000011;
        drive = drive << 3;
        val &= 0b11100111;
        val |= drive;

        self._write_byte_data(APDS9960_REG_GCONF2, val)


    def getGestureWaitTime(self):
        """Gets the time in low power mode between gesture detections.

            Value    Wait time
              0          0 ms
              1          2.8 ms
              2          5.6 ms
              3          8.4 ms
              4         14.0 ms
              5         22.4 ms
              6         30.8 ms
              7         39.2 ms

            Returns:
                int: the current wait time between gestures
        """
        val = self._read_byte_data(APDS9960_REG_GCONF2)

        # shift and mask out LED drive bits
        return val & 0b00000111

    def setGestureWaitTime(self, time):
        """Sets the time in low power mode between gesture detections.

            Value    Wait time
              0          0 ms
              1          2.8 ms
              2          5.6 ms
              3          8.4 ms
              4         14.0 ms
              5         22.4 ms
              6         30.8 ms
              7         39.2 ms

            Args:
                time (int): value for the wait time
        """
        val = self._read_byte_data(APDS9960_REG_GCONF2)

        # set bits in register to given value
        time &= 0b00000111
        val &= 0b11111000
        val |= time

        self._write_byte_data(APDS9960_REG_GCONF2, val)


    def getLightIntLowThreshold(self):
        """Gets the low threshold for ambient light interrupts.

            Returns:
                int: threshold current low threshold stored on the APDS9960
        """
        return self._read_byte_data(APDS9960_REG_AILTL) | (self._read_byte_data(APDS9960_REG_AILTH) << 8)

    def setLightIntLowThreshold(self, threshold):
        """Sets the low threshold for ambient light interrupts.

            Args:
                threshold (int): low threshold value for interrupt to trigger
        """
        # break 16-bit threshold into 2 8-bit values
        self._write_byte_data(APDS9960_REG_AILTL, threshold & 0x00ff)
        self._write_byte_data(APDS9960_REG_AILTH, (threshold & 0xff00) >> 8)


    def getLightIntHighThreshold(self):
        """Gets the high threshold for ambient light interrupts.

            Returns:
                int: threshold current low threshold stored on the APDS9960
        """
        return self._read_byte_data(APDS9960_REG_AIHTL) | (self._read_byte_data(APDS9960_REG_AIHTH) << 8)

    def setLightIntHighThreshold(self, threshold):
        """Sets the high threshold for ambient light interrupts.

            Args:
                threshold (int): high threshold value for interrupt to trigger
        """
        # break 16-bit threshold into 2 8-bit values
        self._write_byte_data(APDS9960_REG_AIHTL, threshold & 0x00ff)
        self._write_byte_data(APDS9960_REG_AIHTH, (threshold & 0xff00) >> 8)


    def getProximityIntLowThreshold(self):
        """Gets the low threshold for proximity interrupts.

            Returns:
                int: threshold current low threshold stored on the APDS9960
        """
        return self._read_byte_data(APDS9960_REG_PILT)

    def setProximityIntLowThreshold(self, threshold):
        """Sets the low threshold for proximity interrupts.

            Args:
                threshold (int): low threshold value for interrupt to trigger
        """
        self._write_byte_data(APDS9960_REG_PILT, threshold)


    def getProximityIntHighThreshold(self):
        """Gets the high threshold for proximity interrupts.

            Returns:
                int: threshold current high threshold stored on the APDS9960
        """
        return self._read_byte_data(APDS9960_REG_PIHT)

    def setProximityIntHighThreshold(self, threshold):
        """Sets the high threshold for proximity interrupts.

            Args:
                threshold (int): high threshold value for interrupt to trigger
        """
        self._write_byte_data(APDS9960_REG_PIHT, threshold)


    def getAmbientLightIntEnable(self):
        """Gets if ambient light interrupts are enabled or not.

            Returns:
                bool: True if interrupts are enabled, False if not
        """
        val = self._read_byte_data(APDS9960_REG_ENABLE)
        return (val >> 4) & 0b00000001 == 1

    def setAmbientLightIntEnable(self, enable):
        """Turns ambient light interrupts on or off.

            Args:
                enable (bool): True to enable interrupts, False to turn them off
        """
        val = self._read_byte_data(APDS9960_REG_ENABLE)

        # set bits in register to given value
        val &= 0b11101111;
        if enable:
            val |= 0b00010000

        self._write_byte_data(APDS9960_REG_ENABLE, val)


    def getProximityIntEnable(self):
        """Gets if proximity interrupts are enabled or not.

            Returns:
                bool: True if interrupts are enabled, False if not
        """
        val = self._read_byte_data(APDS9960_REG_ENABLE)
        return (val >> 5) & 0b00000001 == 1

    def setProximityIntEnable(self, enable):
        """Turns proximity interrupts on or off.

            Args:
                enable (bool): True to enable interrupts, False to turn them off
        """
        val = self._read_byte_data(APDS9960_REG_ENABLE)

        # set bits in register to given value
        val &= 0b11011111;
        if enable:
            val |= 0b00100000

        self._write_byte_data(APDS9960_REG_ENABLE, val)


    def getGestureIntEnable(self):
        """Gets if gesture interrupts are enabled or not.

            Returns:
                bool: True if interrupts are enabled, False if not
        """
        val = self._read_byte_data(APDS9960_REG_GCONF4)
        return (val >> 1) & 0b00000001 == 1

    def setGestureIntEnable(self, enable):
        """Turns gesture-related interrupts on or off.

            Args:
                enable (bool): True to enable interrupts, False to turn them off
        """
        val = self._read_byte_data(APDS9960_REG_GCONF4)

        # set bits in register to given value
        val &= 0b11111101
        if enable:
            val |= 0b00000010

        self._write_byte_data(APDS9960_REG_GCONF4, val)


    def clearAmbientLightInt(self):
        """Clears the ambient light interrupt.
        """
        self._read_byte_data(APDS9960_REG_AICLEAR)


    def clearProximityInt(self):
        """Clears the proximity interrupt.
        """
        self._read_byte_data(APDS9960_REG_PICLEAR)


    def getGestureMode(self):
        """Tells if the gesture state machine is currently running.

            Returns:
                bool: True if gesture state machine is running, False if not
        """
        val = self._read_byte_data(APDS9960_REG_GCONF4)
        return val & 0b00000001 == 1

    def setGestureMode(self, enable):
        """Turns gesture-related interrupts on or off.

            Args:
                enable (bool): True to enter gesture state machine, False to turn them off
        """
        val = self._read_byte_data(APDS9960_REG_GCONF4)

        # set bits in register to given value
        val &= 0b11111110
        if enable:
            val |= 0b00000001

        self._write_byte_data(APDS9960_REG_GCONF4, val)  


    # *******************************************************************************
    # Raw I2C Reads and Writes
    # *******************************************************************************

    def _read_byte_data(self, cmd):
        return self.bus.read_byte_data(self.address, cmd)

    def _write_byte_data(self, cmd, val):
        return self.bus.write_byte_data(self.address, cmd, val)


    def _read_i2c_block_data(self, cmd, num):
        return self.bus.read_i2c_block_data(self.address, cmd, num)



class uAPDS9960(APDS9960):
    """
    APDS9960 for MicroPython

    sensor = uAPDS9960(bus=I2C_instance,
                       address=APDS9960_I2C_ADDR, valid_id=APDS9960_DEV_ID)
    """
    def _read_byte_data(self, cmd):
        return self.bus.readfrom_mem(self.address, cmd, 1)[0]

    def _write_byte_data(self, cmd, val):
        self.bus.writeto_mem(self.address, cmd, bytes([val]))

    def _read_i2c_block_data(self, cmd, num):
        return self.bus.readfrom_mem(self.address, cmd, num)
