# Need to redefine these variables here.
set(TOP_DIR                 $ENV{TOP_DIR})
set(TARGET                  $ENV{TARGET})
set(PORT                    rp2)
set(BUILD                   ${TOP_DIR}/build/rp2)
set(BIN_DIR                 ${TOP_DIR}/build/bin)
set(OMV_DIR                 omv)
set(UVC_DIR                 uvc)
set(CM4_DIR                 cm4)
set(BOOTLDR_DIR             bootloader)
set(CUBEAI_DIR              stm32cubeai)
set(CMSIS_DIR               hal/cmsis)
#set(MICROPY_DIR micropython)
set(LEPTON_DIR              drivers/lepton)
set(LSM6DS3_DIR             drivers/lsm6ds3)
set(WINC1500_DIR            drivers/winc1500)
set(NINAW10_DIR             drivers/ninaw10)
set(MLX90621_DIR            drivers/mlx90621)
set(MLX90640_DIR            drivers/mlx90640)
set(MLX90641_DIR            drivers/mlx90641)
set(OPENPDM_DIR             ${TOP_DIR}/lib/openpdm)
set(TENSORFLOW_DIR          ${TOP_DIR}/lib/libtf)
set(OMV_BOARD_CONFIG_DIR    ${TOP_DIR}/${OMV_DIR}/boards/${TARGET}/)
#set(MP_BOARD_CONFIG_DIR    ${TOP_DIR}/${MICROPY_DIR}/ports/${PORT}/boards/${TARGET}/
set(MPY_LIB_DIR             ${TOP_DIR}/../scripts/libraries)
set(FROZEN_MANIFEST         ${OMV_BOARD_CONFIG_DIR}/manifest.py)
set(OMV_COMMON_DIR          ${TOP_DIR}/${OMV_DIR}/common)
set(PORT_DIR                ${TOP_DIR}/${OMV_DIR}/ports/${PORT})

# Include board cmake fragment
include(${OMV_BOARD_CONFIG_DIR}/omv_boardconfig.cmake)

target_compile_definitions(${MICROPY_TARGET} PRIVATE
    ARM_MATH_CM0PLUS
    ${OMV_BOARD_MODULES_DEFINITIONS}
)

target_link_libraries(${MICROPY_TARGET}
    pico_bootsel_via_double_reset
)

# Linker script
add_custom_command(
    OUTPUT ${BUILD}/rp2.ld
    COMMENT "Preprocessing linker script"
    COMMAND "${CMAKE_C_COMPILER}" -P -E -I ${OMV_BOARD_CONFIG_DIR} -DLINKER_SCRIPT ${PORT_DIR}/rp2.ld.S > ${BUILD}/rp2.ld
    DEPENDS ${OMV_BOARD_CONFIG_DIR}/omv_boardconfig.h ${PORT_DIR}/rp2.ld.S
    VERBATIM
)
add_custom_target(
    LinkerScript
    ALL DEPENDS ${BUILD}/rp2.ld
    VERBATIM
)
pico_set_linker_script(${MICROPY_TARGET} ${BUILD}/rp2.ld)

# Add OMV qstr sources
file(GLOB OMV_SRC_QSTR1 ${TOP_DIR}/${OMV_DIR}/modules/*.c)
file(GLOB OMV_SRC_QSTR2 ${TOP_DIR}/${OMV_DIR}/ports/${PORT}/modules/*.c)
list(APPEND MICROPY_SOURCE_QSTR ${OMV_SRC_QSTR1} ${OMV_SRC_QSTR2})

# Override manifest file
set(MICROPY_FROZEN_MANIFEST ${FROZEN_MANIFEST})

target_include_directories(${MICROPY_TARGET} PRIVATE
    ${TOP_DIR}/${CMSIS_DIR}/include/
    ${TOP_DIR}/${CMSIS_DIR}/include/rpi/

    ${MICROPY_DIR}/py/
    ${MICROPY_DIR}/lib/oofatfs/
    ${MICROPY_DIR}/lib/tinyusb/src/

    ${TOP_DIR}/${OMV_DIR}/
    ${TOP_DIR}/${OMV_DIR}/alloc/
    ${TOP_DIR}/${OMV_DIR}/common/
    ${TOP_DIR}/${OMV_DIR}/imlib/
    ${TOP_DIR}/${OMV_DIR}/modules/
    ${TOP_DIR}/${OMV_DIR}/sensors/
    ${TOP_DIR}/${OMV_DIR}/ports/${PORT}/
    ${TOP_DIR}/${OMV_DIR}/ports/${PORT}/modules/

    ${TOP_DIR}/${LEPTON_DIR}/include/
    ${TOP_DIR}/${LSM6DS3_DIR}/include/
    ${TOP_DIR}/${WINC1500_DIR}/include/
    ${TOP_DIR}/${NINAW10}/include/
    ${TOP_DIR}/${MLX90621_DIR}/include/
    ${TOP_DIR}/${MLX90640_DIR}/include/
    ${TOP_DIR}/${MLX90641_DIR}/include/
    ${OMV_BOARD_CONFIG_DIR}
)

file(GLOB OMV_USER_MODULES ${TOP_DIR}/${OMV_DIR}/modules/*.c)

target_sources(${MICROPY_TARGET} PRIVATE
    ${TOP_DIR}/${OMV_DIR}/alloc/xalloc.c
    ${TOP_DIR}/${OMV_DIR}/alloc/fb_alloc.c
    ${TOP_DIR}/${OMV_DIR}/alloc/umm_malloc.c
    ${TOP_DIR}/${OMV_DIR}/alloc/dma_alloc.c
    ${TOP_DIR}/${OMV_DIR}/alloc/unaligned_memcpy.c

    ${TOP_DIR}/${OMV_DIR}/common/array.c
    ${TOP_DIR}/${OMV_DIR}/common/ff_wrapper.c
    ${TOP_DIR}/${OMV_DIR}/common/ini.c
    ${TOP_DIR}/${OMV_DIR}/common/ringbuf.c
    ${TOP_DIR}/${OMV_DIR}/common/trace.c
    ${TOP_DIR}/${OMV_DIR}/common/mutex.c
    ${TOP_DIR}/${OMV_DIR}/common/usbdbg.c
    ${TOP_DIR}/${OMV_DIR}/common/sensor_utils.c

    ${TOP_DIR}/${OMV_DIR}/sensors/ov2640.c
    ${TOP_DIR}/${OMV_DIR}/sensors/ov5640.c
    ${TOP_DIR}/${OMV_DIR}/sensors/ov7670.c
    ${TOP_DIR}/${OMV_DIR}/sensors/ov7690.c
    ${TOP_DIR}/${OMV_DIR}/sensors/ov7725.c
    ${TOP_DIR}/${OMV_DIR}/sensors/ov9650.c
    ${TOP_DIR}/${OMV_DIR}/sensors/mt9v034.c
    ${TOP_DIR}/${OMV_DIR}/sensors/mt9m114.c
    ${TOP_DIR}/${OMV_DIR}/sensors/lepton.c
    ${TOP_DIR}/${OMV_DIR}/sensors/hm01b0.c
    ${TOP_DIR}/${OMV_DIR}/sensors/gc2145.c

    ${TOP_DIR}/${OMV_DIR}/imlib/agast.c
    ${TOP_DIR}/${OMV_DIR}/imlib/apriltag.c
    ${TOP_DIR}/${OMV_DIR}/imlib/bayer.c
    ${TOP_DIR}/${OMV_DIR}/imlib/binary.c
    ${TOP_DIR}/${OMV_DIR}/imlib/blob.c
    ${TOP_DIR}/${OMV_DIR}/imlib/bmp.c
    ${TOP_DIR}/${OMV_DIR}/imlib/clahe.c
    ${TOP_DIR}/${OMV_DIR}/imlib/collections.c
    ${TOP_DIR}/${OMV_DIR}/imlib/dmtx.c
    ${TOP_DIR}/${OMV_DIR}/imlib/draw.c
    ${TOP_DIR}/${OMV_DIR}/imlib/edge.c
    ${TOP_DIR}/${OMV_DIR}/imlib/eye.c
    ${TOP_DIR}/${OMV_DIR}/imlib/fast.c
    ${TOP_DIR}/${OMV_DIR}/imlib/fft.c
    ${TOP_DIR}/${OMV_DIR}/imlib/filter.c
    ${TOP_DIR}/${OMV_DIR}/imlib/fmath.c
    ${TOP_DIR}/${OMV_DIR}/imlib/font.c
    ${TOP_DIR}/${OMV_DIR}/imlib/framebuffer.c
    ${TOP_DIR}/${OMV_DIR}/imlib/fsort.c
    ${TOP_DIR}/${OMV_DIR}/imlib/gif.c
    ${TOP_DIR}/${OMV_DIR}/imlib/haar.c
    ${TOP_DIR}/${OMV_DIR}/imlib/hog.c
    ${TOP_DIR}/${OMV_DIR}/imlib/hough.c
    ${TOP_DIR}/${OMV_DIR}/imlib/imlib.c
    ${TOP_DIR}/${OMV_DIR}/imlib/integral.c
    ${TOP_DIR}/${OMV_DIR}/imlib/integral_mw.c
    ${TOP_DIR}/${OMV_DIR}/imlib/jpegd.c
    ${TOP_DIR}/${OMV_DIR}/imlib/jpeg.c
    ${TOP_DIR}/${OMV_DIR}/imlib/kmeans.c
    ${TOP_DIR}/${OMV_DIR}/imlib/lab_tab.c
    ${TOP_DIR}/${OMV_DIR}/imlib/lbp.c
    ${TOP_DIR}/${OMV_DIR}/imlib/line.c
    ${TOP_DIR}/${OMV_DIR}/imlib/lsd.c
    ${TOP_DIR}/${OMV_DIR}/imlib/mathop.c
    ${TOP_DIR}/${OMV_DIR}/imlib/mjpeg.c
    ${TOP_DIR}/${OMV_DIR}/imlib/orb.c
    ${TOP_DIR}/${OMV_DIR}/imlib/phasecorrelation.c
    ${TOP_DIR}/${OMV_DIR}/imlib/point.c
    ${TOP_DIR}/${OMV_DIR}/imlib/pool.c
    ${TOP_DIR}/${OMV_DIR}/imlib/ppm.c
    ${TOP_DIR}/${OMV_DIR}/imlib/qrcode.c
    ${TOP_DIR}/${OMV_DIR}/imlib/qsort.c
    ${TOP_DIR}/${OMV_DIR}/imlib/rainbow_tab.c
    ${TOP_DIR}/${OMV_DIR}/imlib/rectangle.c
    ${TOP_DIR}/${OMV_DIR}/imlib/selective_search.c
    ${TOP_DIR}/${OMV_DIR}/imlib/sincos_tab.c
    ${TOP_DIR}/${OMV_DIR}/imlib/stats.c
    ${TOP_DIR}/${OMV_DIR}/imlib/template.c
    ${TOP_DIR}/${OMV_DIR}/imlib/xyz_tab.c
    ${TOP_DIR}/${OMV_DIR}/imlib/zbar.c

    ${TOP_DIR}/${OMV_DIR}/ports/${PORT}/main.c
    ${TOP_DIR}/${OMV_DIR}/ports/${PORT}/cambus.c
    ${TOP_DIR}/${OMV_DIR}/ports/${PORT}/sensor.c

    ${OMV_USER_MODULES}
)

if(MICROPY_PY_SENSOR)
    target_compile_definitions(${MICROPY_TARGET} PRIVATE
        MICROPY_PY_SENSOR=1
    )

    # Generate DCMI PIO header
    pico_generate_pio_header(${MICROPY_TARGET}
        ${OMV_BOARD_CONFIG_DIR}/dcmi.pio
    )
endif()

if(MICROPY_PY_AUDIO)
    target_include_directories(${MICROPY_TARGET} PRIVATE
        ${OPENPDM_DIR}/
    )

    set(AUDIO_SOURCES
	    ${OPENPDM_DIR}/OpenPDMFilter.c
        ${TOP_DIR}/${OMV_DIR}/ports/${PORT}/modules/py_audio.c
    )

    target_sources(${MICROPY_TARGET} PRIVATE ${AUDIO_SOURCES})

    target_compile_definitions(${MICROPY_TARGET} PRIVATE
        MICROPY_PY_AUDIO=1
        #USE_LUT
    )

    pico_generate_pio_header(${MICROPY_TARGET}
        ${TOP_DIR}/${OMV_DIR}/ports/${PORT}/pdm.pio
    )
endif()

if(MICROPY_PY_ULAB)
    set(MICROPY_ULAB_DIR "${TOP_DIR}/${OMV_DIR}/modules/ulab")

    target_include_directories(${MICROPY_TARGET} PRIVATE
        ${MICROPY_ULAB_DIR}/code/
    )

    set(ULAB_SOURCES
	    ${MICROPY_ULAB_DIR}/code/scipy/optimize/optimize.c
	    ${MICROPY_ULAB_DIR}/code/scipy/signal/signal.c
	    ${MICROPY_ULAB_DIR}/code/scipy/special/special.c
	    ${MICROPY_ULAB_DIR}/code/ndarray_operators.c
	    ${MICROPY_ULAB_DIR}/code/ulab_tools.c
	    ${MICROPY_ULAB_DIR}/code/ndarray.c
	    ${MICROPY_ULAB_DIR}/code/numpy/approx/approx.c
	    ${MICROPY_ULAB_DIR}/code/numpy/compare/compare.c
	    ${MICROPY_ULAB_DIR}/code/ulab_create.c
	    ${MICROPY_ULAB_DIR}/code/numpy/fft/fft.c
	    ${MICROPY_ULAB_DIR}/code/numpy/fft/fft_tools.c
	    ${MICROPY_ULAB_DIR}/code/numpy/filter/filter.c
	    ${MICROPY_ULAB_DIR}/code/numpy/linalg/linalg.c
	    ${MICROPY_ULAB_DIR}/code/numpy/linalg/linalg_tools.c
	    ${MICROPY_ULAB_DIR}/code/numpy/numerical/numerical.c
	    ${MICROPY_ULAB_DIR}/code/numpy/poly/poly.c
	    ${MICROPY_ULAB_DIR}/code/numpy/vector/vector.c
	    ${MICROPY_ULAB_DIR}/code/user/user.c
	    ${MICROPY_ULAB_DIR}/code/numpy/numpy.c
	    ${MICROPY_ULAB_DIR}/code/scipy/scipy.c
	    ${MICROPY_ULAB_DIR}/code/ulab.c
    )

    target_sources(${MICROPY_TARGET} PRIVATE ${ULAB_SOURCES})
    list(APPEND MICROPY_SOURCE_QSTR ${ULAB_SOURCES})

    target_compile_definitions(${MICROPY_TARGET} PRIVATE
        MICROPY_PY_ULAB=1
        MODULE_ULAB_ENABLED=1
        ULAB_CONFIG_FILE="${OMV_BOARD_CONFIG_DIR}/ulab_config.h"
    )
endif()

if(MICROPY_PY_NINAW10)
    target_include_directories(${MICROPY_TARGET} PRIVATE
        ${TOP_DIR}/${NINAW10_DIR}/include/
    )

    set(NINA_SOURCES
        ${TOP_DIR}/${NINAW10_DIR}/src/nina.c
        ${TOP_DIR}/${OMV_DIR}/ports/${PORT}/nina_bsp.c
        ${TOP_DIR}/${OMV_DIR}/ports/${PORT}/modules/py_nina.c
    )

    target_sources(${MICROPY_TARGET} PRIVATE ${NINA_SOURCES})

    target_compile_definitions(${MICROPY_TARGET} PRIVATE
        NINA_DEBUG=0
        MICROPY_PY_USOCKET=1
        MICROPY_PY_NETWORK=1
        MICROPY_PY_NINAW10=1
    )
endif()

add_custom_command(TARGET ${MICROPY_TARGET}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy firmware.elf firmware.bin firmware.uf2 ${BIN_DIR}
    VERBATIM
)
