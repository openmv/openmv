# Copy this module to storage and import it if you want
# to use it in your own scripts. See example usage below.
from time import sleep
from pyb import Pin, SPI

class LCD:
	def reset(self):
		self.rst.low()
		sleep(100)
		self.rst.high()
		sleep(100)

	def __write_data(self, c):
		self.cs.low()
		self.rs.high()
		self.spi.send(c)
		self.cs.high()

	def __write_command(self, c):
		self.cs.low()
		self.rs.low()
		self.spi.send(c)
		self.cs.high()

	def write_command(self, cmd, *data):
		self.__write_command(cmd)
		for a in data:
			self.__write_data(a)

	def write_image(self, image):
		self.write_command(0x2C)
		self.cs.low()
		self.rs.high()
		self.spi.send(image)
		self.cs.high()

	def clear(self, c=0x00):
		self.write_command(0x2C)
		for i in range(128*160):
			self.__write_data(c)
			self.__write_data(c)

	def set_backlight(self, on):
		if (on):
			self.bl.high()
		else:
			self.bl.low()

	def __init__(self, madctl=0xC0):
		self.rst = Pin('P7', Pin.OUT_PP, Pin.PULL_UP)
		self.rs  = Pin('P8', Pin.OUT_PP, Pin.PULL_UP)
		self.cs  = Pin('P3', Pin.OUT_PP, Pin.PULL_UP)
		self.bl  = Pin('P6',  Pin.OUT_PP, Pin.PULL_UP)
		self.spi = SPI(2, SPI.MASTER, baudrate=22500000, polarity=0, phase=0)

		# LCD init sequence
		self.reset() # uHW reset
		self.write_command(0x11) #Sleep exit
		sleep(120)

		# ST7735R Frame Rate
		self.write_command(0xB1, 0x01, 0x2C, 0x2D)
		self.write_command(0xB2,0x01,0x2C,0x2D)
		self.write_command(0xB3,0x01,0x2C,0x2D,0x01,0x2C,0x2D)

		#Column inversion
		self.write_command(0xB4, 0x07)

		#ST7735R Power Sequence
		self.write_command(0xC0,0xA2,0x02,0x84)
		self.write_command(0xC1,0xC5)
		self.write_command(0xC2, 0x0A, 0x00)
		self.write_command(0xC3,0x8A,0x2A)
		self.write_command(0xC4,0x8A,0xEE)

		# VCOM
		self.write_command(0xC5, 0x0E)

		# MX, MY, MV, RGB mode
		self.write_command(0x36, madctl)

		# ST7735R Gamma Sequence
		self.write_command(0xe0, 0x0f, 0x1a, 0x0f, 0x18, 0x2f, 0x28, 0x20,
						   0x22, 0x1f, 0x1b, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10)
		self.write_command(0xe1, 0x0f, 0x1b, 0x0f, 0x17, 0x33, 0x2c, 0x29,
						   0x2e, 0x30, 0x30, 0x39, 0x3f, 0x00, 0x07, 0x03, 0x10)

		# Set column address
		self.write_command(0x2a, 0x00, 0x00, 0x00, 128-1)

		# Set row address
		self.write_command(0x2b, 0x00, 0x00, 0x00, 160-1)

		# Enable test command
		self.write_command(0xF0, 0x01)

		# Disable ram power save mode
		self.write_command(0xF6, 0x00)

		# 65k mode
		self.write_command(0x3A, 0x05)

		# Display on
		self.write_command(0x29)

if __name__ == "__main__":
	import sensor, time
	#from lcd import LCD

	# Reset sensor
	sensor.reset()

	# Sensor settings
	sensor.set_contrast(2)
	sensor.set_brightness(0)
	sensor.set_saturation(2)
	sensor.set_pixformat(sensor.RGB565)

	# LCD resolution (128x160)
	sensor.set_framesize(sensor.QQVGA2)

	# Init LCD
	lcd = LCD()
	lcd.clear(0x00)
	lcd.set_backlight(True)

	clock = time.clock()
	while (True):
		clock.tick()
		# Capture a frame a draw it to LCD
		lcd.write_image(sensor.snapshot())
		print(clock.fps())
