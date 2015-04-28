# Sources
SRCS += $(addprefix ,   \
	main.c              \
	xalloc.c            \
	array.c             \
	usbdbg.c            \
	sccb.c              \
	ov9650.c            \
	ov2640.c            \
	sensor.c            \
	stm32f4xx_hal_msp.c \
	soft_i2c.c          \
	mutex.c             \
   )

ifeq ($(TARGET), OPENMV1)
SRCS += $(addprefix ,   \
	sdcard_spi.c        \
   )
else
SRCS += $(addprefix ,   \
	sdram.c             \
	sdcard_sdio.c       \
   )
endif

SRCS += $(addprefix img/,   \
	blob.c                  \
	fmath.c                 \
	haar.c                  \
	imlib.c                 \
	integral.c              \
	kmeans.c                \
	lab.c                   \
	xyz_tab.c               \
	rainbow_tab.c           \
	median.c                \
	point.c                 \
	ppm.c                   \
	rectangle.c             \
	fast.c                  \
	freak.c                 \
	template.c              \
	font.c                  \
	jpeg.c                  \
	lbp.c                   \
	eye.c                   \
   )

SRCS += $(addprefix py/,    \
	py_sensor.c             \
	py_image.c              \
	py_time.c               \
	mlx90620.c              \
   )

OBJS = $(addprefix $(BUILD)/, $(SRCS:.c=.o))
OBJ_DIRS = $(sort $(dir $(OBJS)))

all: | $(OBJ_DIRS) $(OBJS)
$(OBJ_DIRS):
	$(MKDIR) -p $@

$(BUILD)/%.o : %.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@
