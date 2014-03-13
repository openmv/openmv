CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
AR = arm-none-eabi-ar
RM = rm
SIZE    = arm-none-eabi-size
STRIP   = arm-none-eabi-strip -s
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump

# General
BIN=openmv
BUILD_DIR=build

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS = -O0 -g
else
CFLAGS = -O2 -g
endif

# Compiler Flags
CFLAGS += -Wall -mlittle-endian -mthumb -nostartfiles -mcpu=cortex-m4 -mabi=aapcs-linux
CFLAGS += -fsingle-precision-constant -Wdouble-promotion -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fshort-enums
CFLAGS += -I. -I../include/CMSIS -I../include/StdPeriph -I../include/USB_OTG -I../include/FatFS -I../include/CC3K -I../include/MicroPython\
          -I../include/MicroPython/py -I./py -I./img -DSTM32F40_41xxx -DUSE_USB_OTG_FS -DARM_MATH_CM4 -D__FPU_PRESENT -std=gnu99 -DOSC=12

# Linker Flags
LDFLAGS = -mcpu=cortex-m4 -mabi=aapcs-linux -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16
#LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(BIN).map -Tstm32f4xx.ld -L. -L../lib
#LDFLAGS += -nostdlib -Tstm32f4xx.ld -L. -L../lib
LDFLAGS += -nostdlib -Tstm32f4xx.ld -L. -L../lib


# Sources
SRCS = $(wildcard *.c) $(wildcard py/*.c) $(wildcard img/*.c)
OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))

# Libraries
#LIB = -lmp -lc -lm -ldsp -lstdperiph -lusbcore -lusbdevcore
LIB = -lmp -ldsp -lcc3k -lstdperiph

all:: $(BUILD_DIR) $(BUILD_DIR) $(BUILD_DIR)/$(BIN).bin
	$(SIZE) $(BUILD_DIR)/$(BIN).elf

$(BUILD_DIR):
	mkdir $@
	mkdir $@/py
	mkdir $@/img

$(BUILD_DIR)/$(BIN).bin: $(BUILD_DIR)/$(BIN).elf
	$(OBJCOPY) -Obinary $^ $@

$(BUILD_DIR)/$(BIN).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIB) -o $@

stats: $(BIN).elf
	$(SIZE) $(BUILD_DIR)/$(BIN).elf

clean:
	$(RM) -fr $(BUILD_DIR)

$(BUILD_DIR)/%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o : %.s
	$(AS) $(AFLAGS) $< -o $@

flash::
	dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -s 0x08000000 -D $(BUILD_DIR)/$(BIN).bin
