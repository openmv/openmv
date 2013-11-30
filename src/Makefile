CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
AR = arm-none-eabi-ar
RM = rm -f 
SIZE    = arm-none-eabi-size
STRIP   = arm-none-eabi-strip -s
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump

#Debugging/Optimization 
ifeq ($(DEBUG), 1)
CFLAGS = -O2
else
CFLAGS = -O0 -ggdb
endif

#Compiler Flags
CFLAGS += -Wall -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4 
CFLAGS += -fsingle-precision-constant -Wdouble-promotion -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -I. -I../include/CMSIS -I../include/StdPeriph -I../include/USB_OTG -DSTM32F40_41xxx -DUSE_USB_OTG_FS

#Linker Flags
LDFLAGS = -mcpu=cortex-m4 -mthumb -mcpu=cortex-m4 -mthumb -mthumb-interwork -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16
LDFLAGS += -Wl,-Map=$(BIN).map -T../stm32_flash.ld -L. -L../lib

# Sources
BIN  = "openmv"
SRCS = $(wildcard *.c) 
OBJS = $(SRCS:.c=.o) 
#Libraries
LIB = -lc -lm -lstdperiph -lusbgeneric -lusbdevcore -lusbcore 

all:: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIB) -o $(BIN).elf
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
