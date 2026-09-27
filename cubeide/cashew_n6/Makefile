# Command-line build of the cashew_n6 STM32CubeIDE project, with the same compiler and linker
# options as the IDE's Release configuration. Uses arm-none-eabi-gcc from PATH, or set
#   make GCC_PATH=/path/to/gcc/bin
# Outputs build/cashew_n6.elf, .bin (FSBL image to sign), .map and a size summary.

TARGET   := cashew_n6
BUILD    := build
GCC_PATH ?=
PREFIX   := $(if $(GCC_PATH),$(GCC_PATH)/,)arm-none-eabi-
CC       := $(PREFIX)gcc
OBJCOPY  := $(PREFIX)objcopy
SIZE     := $(PREFIX)size

LDSCRIPT := STM32N657X0HXQ_cashew_tcm.ld

C_SOURCES := \
  $(wildcard Core/Src/*.c) \
  Cashew/cashew_core.c \
  $(wildcard Drivers/STM32N6xx_HAL_Driver/Src/*.c)
ASM_SOURCES := Core/Startup/startup_stm32n657xx_fsbl.s

INCLUDES := \
  -ICore/Inc \
  -ICashew \
  -IDrivers/vd6g \
  -IDrivers/STM32N6xx_HAL_Driver/Inc \
  -IDrivers/STM32N6xx_HAL_Driver/Inc/Legacy \
  -IDrivers/CMSIS/Device/ST/STM32N6xx/Include \
  -IDrivers/CMSIS/Include

DEFS  := -DUSE_HAL_DRIVER -DSTM32N657xx
ARCH  := -mcpu=cortex-m55 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -mcmse
OPT   ?= -O2 -g3

CFLAGS := $(ARCH) -std=gnu11 $(OPT) $(DEFS) $(INCLUDES) \
  -ffunction-sections -fdata-sections -fno-math-errno -fstack-usage \
  -Wall -MMD -MP --specs=nano.specs
ASFLAGS := $(ARCH) -g3 -x assembler-with-cpp --specs=nano.specs
LDFLAGS := $(ARCH) -T$(LDSCRIPT) -nostartfiles --specs=nosys.specs --specs=nano.specs \
  -Wl,-Map=$(BUILD)/$(TARGET).map -Wl,--gc-sections -static -Wl,--print-memory-usage \
  -Wl,--start-group -lc -lm -Wl,--end-group

OBJECTS := $(addprefix $(BUILD)/,$(C_SOURCES:.c=.o)) $(addprefix $(BUILD)/,$(ASM_SOURCES:.s=.o))

all: $(BUILD)/$(TARGET).bin
	@$(SIZE) -A -x $(BUILD)/$(TARGET).elf | grep -E "itcm|isr|\.text|rodata|\.data|\.bss|sram_bss|frames" || true

$(BUILD)/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/%.o: %.s Makefile
	@mkdir -p $(dir $@)
	$(CC) -c $(ASFLAGS) $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD)

-include $(OBJECTS:.o=.d)

.PHONY: all clean
