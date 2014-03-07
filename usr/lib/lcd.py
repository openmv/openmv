import spi, gpio
from time import sleep

rst = gpio.GPIO(gpio.PA8)
rs  = gpio.GPIO(gpio.PC9)
cs  = gpio.GPIO(gpio.PA15)

def reset():
    rst.low()
    sleep(100)
    rst.high()
    sleep(100)

def write_command(c):
    cs.low()
    rs.low()
    spi.write(c)
    cs.high()

def write_data(data):
    cs.low()
    rs.high()
    for b in data:
        spi.write(b)
    cs.high()

def clear(c=0x00):
    write_command(0x2C)
    for i in range(120*160):
        write_data(c)
        write_data(c)

def write_image(image):
    write_command(0x2C)
    cs.low()
    rs.high()
    spi.write_image(image)
    cs.high()

def init():
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
    write_data(0x60)

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
    write_data(0x9F)

    # set row address
    write_command(0x2b)
    write_data(0x00)
    write_data(0x00)
    write_data(0x00)
    write_data(0x77)

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

