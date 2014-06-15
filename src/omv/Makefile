# Sources
SRCS += $(addprefix ./,\
	main.c\
	led.c\
   )

SRCS += $(addprefix ./py/,\
	py_led.c\
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
