BIN     = "openmv"
OBJ     = main.o ov9650.o sccb.o rgb_led.o usart.o imlib.o systick.o array.o
LIB     = -lc -lcm4 -lm
FLAGS   = -mcpu=cortex-m4 -mthumb -mcpu=cortex-m4 -mthumb -mthumb-interwork -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16
AFLAGS  = $(FLAGS)  
CFLAGS  = $(FLAGS) -O0 -ggdb -Wall -I. -I../include
LDFLAGS = $(FLAGS) -Wl,-Map=$(BIN).map -T../stm32_flash.ld -L. -L../lib

CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
AR = arm-none-eabi-ar
RM = rm -f 
SIZE    = arm-none-eabi-size
STRIP   = arm-none-eabi-strip -s
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump

all:: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) $(LIB) -o $(BIN).elf
	$(OBJCOPY) -Oihex -j .text -j .data $(BIN).elf $(BIN).hex
	$(OBJCOPY) -Obinary  -j .text -j .data $(BIN).elf $(BIN).bin
	$(OBJDUMP) -d $(BIN).elf > $(BIN).dis

stats: $(BIN).elf
	$(SIZE) $(BIN).elf

clean:
	$(RM) *.o *.elf *.bin *.map *.hex *.dis

.c.o :
	$(CC) $(CFLAGS) -c $<

.s.o :
	$(AS) $(AFLAGS) $< -o $@

flash::
	dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -s 0x08000000 -D $(BIN).bin
