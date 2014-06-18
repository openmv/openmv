# Sources
SRCS += $(addprefix ,   \
	main.c              \
	xalloc.c            \
	array.c             \
	usbdbg.c            \
	systick.c           \
	sccb.c              \
	ov9650.c            \
	ov2640.c            \
	sensor.c            \
	led.c               \
	rng.c               \
   )

SRCS += $(addprefix img/,   \
	blob.c                  \
	fmath.c                 \
	haar.c                  \
	imlib.c                 \
	integral.c              \
	kmeans.c                \
	lab.c                   \
	median.c                \
	point.c                 \
	ppm.c                   \
	rectangle.c             \
	surf.c                  \
	template.c              \
   )

SRCS += $(addprefix py/,    \
	py_led.c                \
	py_sensor.c             \
	py_image.c              \
	py_file.c               \
	py_time.c               \
	py_clock.c              \
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
