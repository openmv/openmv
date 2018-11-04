import pyb

class Stepper():
    def __init__(self, stepnumber=200, rpms=2, power=50):
        self.stepnumber = stepnumber
        self.pin1 = pyb.Pin('P3', pyb.Pin.OUT_PP, pyb.Pin.PULL_NONE)
        self.pin2 = pyb.Pin('P2', pyb.Pin.OUT_PP, pyb.Pin.PULL_NONE)
        self.pin3 = pyb.Pin('P1', pyb.Pin.OUT_PP, pyb.Pin.PULL_NONE)
        self.pin4 = pyb.Pin('P0', pyb.Pin.OUT_PP, pyb.Pin.PULL_NONE)
        tim = pyb.Timer(4, freq=1000)
        self.power1 = tim.channel(1, pyb.Timer.PWM, pin=pyb.Pin("P7"), pulse_width_percent=100)
        self.power2 = tim.channel(2, pyb.Timer.PWM, pin=pyb.Pin("P8"), pulse_width_percent=100)
        self.set_speed(rpms)
        self.set_power(power)
        self.phase = self.phase_list()

    def phase_list(self):
        phase_list = [(1,0,0,0), (0,0,1,0), (0,1,0,0), (0,0,0,1)]
        while True:
            for p in phase_list:
                yield p

    def set_speed(self, rpms):
        self.delay_time = int(1000000/(rpms*self.stepnumber)/2)

    def set_power(self, power):
        self.power1.pulse_width_percent(power)
        self.power2.pulse_width_percent(power)

    def step(self, num):
        for i in range(num):
            phase = self.phase.__next__()
            self.pin1.value(phase[0])
            self.pin2.value(phase[1])
            self.pin3.value(phase[2])
            self.pin4.value(phase[3])
            pyb.udelay(self.delay_time)
