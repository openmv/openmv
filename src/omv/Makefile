# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# OMV Makefile

SRCS += $(addprefix ,   \
	main.c              \
	xalloc.c            \
	fb_alloc.c          \
	umm_malloc.c        \
	ff_wrapper.c        \
	ini.c               \
	framebuffer.c       \
	array.c             \
	usbdbg.c            \
	wifidbg.c           \
	cambus.c            \
	ov2640.c            \
	ov5640.c            \
	ov7690.c            \
	ov7725.c            \
	ov9650.c            \
	mt9v034.c           \
	lepton.c            \
	hm01b0.c            \
	sensor.c            \
	stm32fxxx_hal_msp.c \
	soft_i2c.c          \
	mutex.c             \
	trace.c             \
   )

SRCS += $(addprefix img/,   \
	binary.c                \
	blob.c                  \
	clahe.c                 \
	draw.c                  \
	qrcode.c                \
	apriltag.c              \
	dmtx.c                  \
	zbar.c                  \
	fmath.c                 \
	fsort.c                 \
	qsort.c                 \
	fft.c                   \
	filter.c                \
	haar.c                  \
	imlib.c                 \
	collections.c           \
	stats.c                 \
	integral.c              \
	integral_mw.c           \
	kmeans.c                \
	lab_tab.c               \
	xyz_tab.c               \
	yuv_tab.c               \
	rainbow_tab.c           \
	rgb2rgb_tab.c           \
	invariant_tab.c         \
	mathop.c                \
	pool.c                  \
	point.c                 \
	rectangle.c             \
	bmp.c                   \
	ppm.c                   \
	gif.c                   \
	mjpeg.c                 \
	fast.c                  \
	agast.c                 \
	orb.c                   \
	template.c              \
	phasecorrelation.c      \
	shadow_removal.c        \
	font.c                  \
	jpeg.c                  \
	lbp.c                   \
	eye.c                   \
	hough.c                 \
	line.c                  \
	lsd.c                   \
	sincos_tab.c            \
	edge.c                  \
	hog.c                   \
	selective_search.c      \
   )

SRCS += $(addprefix nn/,    \
	nn.c                    \
   )


SRCS += $(addprefix py/, \
	py_helper.c             \
	py_omv.c                \
	py_sensor.c             \
	py_image.c              \
	py_time.c               \
	py_lcd.c                \
	py_tv.c                 \
	py_fir.c                \
	py_gif.c                \
	py_mjpeg.c              \
	py_winc.c               \
	py_cpufreq.c            \
	py_nn.c                 \
	py_tf.c                 \
	py_imu.c                \
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

-include $(OBJS:%.o=%.d)
