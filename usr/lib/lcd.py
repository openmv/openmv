from time import sleep
from pyb import Pin, SPI

rst = Pin('PD12', Pin.OUT_PP, Pin.PULL_UP)
rs  = Pin('PD13', Pin.OUT_PP, Pin.PULL_UP)
cs  = Pin('PB12', Pin.OUT_PP, Pin.PULL_UP)
bl  = Pin('PA5',  Pin.OUT_PP, Pin.PULL_UP)
spi = SPI(2, SPI.MASTER, baudrate=22500000, polarity=0, phase=0) 

def reset():
    rst.low()
    sleep(100)
    rst.high()
    sleep(100)

def write_command(c):
    cs.low()
    rs.low()
    spi.send(c)
    cs.high()

def write_data(c):
    cs.low()
    rs.high()
    spi.send(c)
    cs.high()

def clear(c=0x00):
    write_command(0x2C)
    for i in range(128*160):
        write_data(c)
        write_data(c)

def write_image(image):
    write_command(0x2C)
    cs.low()
    rs.high()
    spi.send(image)
    cs.high()

def set_backlight(on):
    if (on):
        bl.high()
    else:
        bl.low()

def init(madctl=0xC0):
    #HW reset
    reset()

    #LCD init sequence
    #Sleep exit
    write_command(0x11)
    sleep (120)

    #ST7735R Frame Rate
    write_command(0xB1)
    write_data(0x01)
    write_data(0x2C)
    write_data(0x2D)

    write_command(0xB2)
    write_data(0x01)
    write_data(0x2C)
    write_data(0x2D)

    write_command(0xB3)
    write_data(0x01)
    write_data(0x2C)
    write_data(0x2D)
    write_data(0x01)
    write_data(0x2C)
    write_data(0x2D)

    write_command(0xB4)
    #Column inversion
    write_data(0x07)

    #ST7735R Power Sequence
    write_command(0xC0)
    write_data(0xA2)
    write_data(0x02)
    write_data(0x84)

    write_command(0xC1)
    write_data(0xC5)
    write_command(0xC2)
    write_data(0x0A)
    write_data(0x00)

    write_command(0xC3)
    write_data(0x8A)
    write_data(0x2A)

    write_command(0xC4)
    write_data(0x8A)
    write_data(0xEE)

    #VCOM
    write_command(0xC5)
    write_data(0x0E)

    #MX, MY, MV, RGB mode
    write_command(0x36)
    write_data(madctl)

    #ST7735R Gamma Sequence
    write_command(0xe0)
    write_data(0x0f)
    write_data(0x1a)
    write_data(0x0f)
    write_data(0x18)
    write_data(0x2f)
    write_data(0x28)
    write_data(0x20)
    write_data(0x22)
    write_data(0x1f)
    write_data(0x1b)
    write_data(0x23)
    write_data(0x37)
    write_data(0x00)
    write_data(0x07)
    write_data(0x02)
    write_data(0x10)

    write_command(0xe1)
    write_data(0x0f)
    write_data(0x1b)
    write_data(0x0f)
    write_data(0x17)
    write_data(0x33)
    write_data(0x2c)
    write_data(0x29)
    write_data(0x2e)
    write_data(0x30)
    write_data(0x30)
    write_data(0x39)
    write_data(0x3f)
    write_data(0x00)
    write_data(0x07)
    write_data(0x03)
    write_data(0x10)

    # set column address
    write_command(0x2a)
    write_data(0x00)
    write_data(0x00)
    write_data(0x00)
    write_data(128-1)

    # set row address
    write_command(0x2b)
    write_data(0x00)
    write_data(0x00)
    write_data(0x00)
    write_data(160-1)

    #Enable test command
    write_command(0xF0)
    write_data(0x01)

    #Disable ram power save mode
    write_command(0xF6)
    write_data(0x00)

    #65k mode
    write_command(0x3A)
    write_data(0x05)

    #Display on
    write_command(0x29)

