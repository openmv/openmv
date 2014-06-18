# Sources
SRCS += $(addprefix ,\
	main.c\
	systick.c\
	usbdbg.c\
	sccb.c\
	ov9650.c\
	ov2640.c\
	sensor.c\
	led.c\
   )

SRCS += $(addprefix img/,\
   )

SRCS += $(addprefix py/,\
	py_led.c\
	py_sensor.c\
   )

OBJS = $(addprefix $(BUILD)/, $(SRCS:.c=.o))
OBJ_DIRS = $(sort $(dir $(OBJS)))

all: | $(OBJ_DIRS) $(OBJS)
$(OBJ_DIRS):
	$(MKDIR) -p $@

$(BUILD)/%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(AS) $(AFLAGS) $< -o $@
