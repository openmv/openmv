# Sources
SRCS += $(addprefix ,   \
	main.c              \
	xalloc.c            \
	fb_alloc.c          \
	ff_wrapper.c        \
	array.c             \
	usbdbg.c            \
	sccb.c              \
	ov9650.c            \
	ov2640.c            \
	ov7725.c            \
	sensor.c            \
	stm32f4xx_hal_msp.c \
	soft_i2c.c          \
	mutex.c             \
   )

SRCS += $(addprefix img/,   \
	blob.c                  \
	fmath.c                 \
	fsort.c                 \
	haar.c                  \
	imlib.c                 \
	stats.c                 \
	morph.c                 \
	integral.c              \
	integral_mw.c           \
	kmeans.c                \
	lab_tab.c               \
	xyz_tab.c               \
	yuv_tab.c               \
	rainbow_tab.c           \
	rgb2rgb_tab.c           \
	midpoint.c              \
	mean.c                  \
	mode.c                  \
	median.c                \
	point.c                 \
	rectangle.c             \
	bmp.c                   \
	ppm.c                   \
	gif.c                   \
	mjpeg.c                 \
	fast.c                  \
	freak.c                 \
	template.c              \
	font.c                  \
	jpeg.c                  \
	lbp.c                   \
	eye.c                   \
   )

SRCS += $(addprefix py/, \
	py_helper.c             \
	py_sensor.c             \
	py_image.c              \
	py_time.c               \
	py_lcd.c                \
	py_fir.c                \
	py_gif.c                \
	py_mjpeg.c              \
	py_winc.c               \
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
