/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2020 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2020 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * Copyright (c) 2013-2020 Kaizhi Wong <kidswong999@gmail.com>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * TV Python module.
 */
#include "py/obj.h"
#include "py/nlr.h"
#include "py/runtime.h"
#include "spi.h"

#include "py_helper.h"
#include "omv_boardconfig.h"
#include STM32_HAL_H

#if MICROPY_PY_TV

#define TV_WIDTH    352
#define TV_HEIGHT   240
#define TV_REFRESH  60

#if ((TV_WIDTH) % 2)
#error "TV_WIDTH not even"
#endif

#if ((TV_HEIGHT) % 2)
#error "TV_HEIGHT not even"
#endif

#ifdef OMV_SPI_LCD_CONTROLLER
/////////////////////////////////////////////////////////////
// http://www.vsdsp-forum.com/phpbb/viewtopic.php?f=14&t=1801
/////////////////////////////////////////////////////////////

// Crystal frequency in MHZ (float, observe accuracy)
#define XTAL_MHZ 3.579545

// Line length in microseconds (float, observe accuracy)
#define LINE_LENGTH_US 63.556

#define FIXED_VCLK_CYCLES 10
#define FIXED_CSCLK_CYCLES ((FIXED_VCLK_CYCLES) / 8.0)

// Normal visible picture line sync length is 4.7 us
#define SYNC_US 4.7
#define SYNC ((uint16_t) (((SYNC_US) * (XTAL_MHZ)) - (FIXED_CSCLK_CYCLES) + 0.5))

// Color burst starts at 5.3 us
#define BURST_US 5.3
#define BURST ((uint16_t) (((BURST_US) * (XTAL_MHZ)) - (FIXED_CSCLK_CYCLES) + 0.5))

// Color burst duration is 2.5 us
#define BURST_DUR_US 2.5
#define BURST_DUR ((uint16_t) (((BURST_DUR_US) * (XTAL_MHZ)) + 0.5))

// Black video starts at 9.4 us
#define BLACK_US 9.4
#define BLACK ((uint16_t) (((BLACK_US) * (XTAL_MHZ)) - (FIXED_CSCLK_CYCLES) + 0.5))

// Black video duration is 52.656 us
#define BLACK_DUR_US 52.656
#define BLACK_DUR ((uint16_t) (((BLACK_DUR_US) * (XTAL_MHZ)) + 0.5))

// Define NTSC video timing constants
// NTSC short sync duration is 2.3 us
#define SHORT_SYNC_US 2.3

// For the start of the line, the first 10 extra PLLCLK sync (0) cycles are subtracted.
#define SHORTSYNC ((uint16_t) (((SHORT_SYNC_US) * (XTAL_MHZ)) - (FIXED_CSCLK_CYCLES) + 0.5 ))

// For the middle of the line the whole duration of sync pulse is used.
#define SHORTSYNCM ((uint16_t) (((SHORT_SYNC_US) * (XTAL_MHZ)) + 0.5))

// NTSC long sync duration is 27.078 us
#define LONG_SYNC_US 27.078
#define LONGSYNC ((uint16_t) (((LONG_SYNC_US) * (XTAL_MHZ)) - (FIXED_CSCLK_CYCLES) + 0.5))
#define LONGSYNCM ((uint16_t) (((LONG_SYNC_US) * (XTAL_MHZ)) + 0.5))

// Number of lines used after the VSYNC but before visible area.
#define VSYNC_LINES 9
#define FRONT_PORCH_LINES 13

// Definitions for picture lines
// On which line the picture area begins, the Y direction.
#define STARTLINE ((VSYNC_LINES) + (FRONT_PORCH_LINES))

// Frame length in lines (visible lines + nonvisible lines)
// Amount has to be odd for NTSC and RGB colors
#define TOTAL_LINES ((STARTLINE) + (TV_HEIGHT) + 1)
#if ((TOTAL_LINES) != 263)
#error "Progressive NTSC must have 263 lines!"
#endif

// Width, in PLL clocks, of each pixel.
#define PLLCLKS_PER_PIXEL 4

// The first pixel of the picture area, the X direction.
#define STARTPIX ((BLACK) + 7)

// The last pixel of the picture area.
#define ENDPIX ((uint16_t) ((STARTPIX) + (((PLLCLKS_PER_PIXEL) * (TV_WIDTH)) / 8)))

// Reserve memory for this number of different prototype lines
// (prototype lines are used for sync timing, porch and border area)
#define PROTOLINES 3

// PLL frequency
#define PLL_MHZ ((XTAL_MHZ) * 8)

// 10 first pllclks, which are not in the counters are decremented here
#define PLLCLKS_PER_LINE ((uint16_t) (((LINE_LENGTH_US) * (PLL_MHZ)) - (FIXED_VCLK_CYCLES)))

// 10 first pllclks, which are not in the counters are decremented here
#define COLORCLKS_PER_LINE ((uint16_t) ((((((LINE_LENGTH_US) * (PLL_MHZ)) / 1) + 7) / 8) - (FIXED_CSCLK_CYCLES)))
#define COLORCLKS_LINE_HALF ((uint16_t) ((((((LINE_LENGTH_US) * (PLL_MHZ)) / 2) + 7) / 8) - (FIXED_CSCLK_CYCLES)))

#define PROTO_AREA_WORDS ((COLORCLKS_PER_LINE) * (PROTOLINES))
#define INDEX_START_LONGWORDS (((PROTO_AREA_WORDS) + 1) / 2)
#define INDEX_START_BYTES ((INDEX_START_LONGWORDS) * 4)

// Protoline 0 starts always at address 0
#define PROTOLINE_BYTE_ADDRESS(n) ((COLORCLKS_PER_LINE) * 2 * (n))
#define PROTOLINE_WORD_ADDRESS(n) ((COLORCLKS_PER_LINE) * 1 * (n))

// Calculate picture lengths in pixels and bytes, coordinate areas for picture area
#define PICBITS 12
#define PICLINE_LENGTH_BYTES (((TV_WIDTH) * (PICBITS)) / 8)

#define LINE_INDEX_BYTE_SIZE 3

// Picture area memory start point
#define PICLINE_START ((INDEX_START_BYTES) + ((TOTAL_LINES) * (LINE_INDEX_BYTE_SIZE)))

// Picture area line start addresses
#define PICLINE_BYTE_ADDRESS(n) ((PICLINE_START) + ((PICLINE_LENGTH_BYTES) * (n)))

// Pattern generator microcode
// ---------------------------
// Bits 7:6  a=00|b=01|y=10|-=11
// Bits 5:3  n pick bits 1..8
// bits 2:0  shift 0..6
#define PICK_A (0 << 6)
#define PICK_B (1 << 6)
#define PICK_Y (2 << 6)
#define PICK_NOTHING (3 << 6)
#define PICK_BITS(a) (((a) - 1) << 3)
#define SHIFT_BITS(a) (a)

// 16 bits per pixel, U4 V4 Y8
// PICK_B is U
#define OP1 (PICK_B + PICK_BITS(4) + SHIFT_BITS(4))
// PICK_A is V
#define OP2 (PICK_A + PICK_BITS(4) + SHIFT_BITS(4))
#define OP3 (PICK_Y + PICK_BITS(8) + SHIFT_BITS(6))
#define OP4 (PICK_NOTHING + SHIFT_BITS(2))

// General VS23 commands
#define WRITE_STATUS 0x01
#define WRITE_SRAM 0x02
#define WRITE_GPIO 0x82
#define READ_GPIO 0x84
#define WRITE_MULTIIC 0xb8
#define WRITE_BLOCKMVC1 0x34

// Bit definitions
#define VDCTRL1 0x2B
#define VDCTRL1_UVSKIP (1 << 0)
#define VDCTRL1_PLL_ENABLE (1 << 12)
#define VDCTRL2 0x2D
#define VDCTRL2_LINECOUNT (1 << 0)
#define VDCTRL2_PIXEL_WIDTH (1 << 10)
#define VDCTRL2_ENABLE_VIDEO (1 << 15)
#define BLOCKMVC1_PYF (1 << 4)

// VS23 video commands
#define PROGRAM 0x30
#define PICSTART 0x28
#define PICEND 0x29
#define LINELEN 0x2a
#define INDEXSTART 0x2c

// Sync, blank, burst and white level definitions, here are several options
// These are for proto lines and so format is VVVVUUUUYYYYYYYY

// Sync is always 0
#define SYNC_LEVEL  0x0000

// 285 mV to 75 ohm load
#define BLANK_LEVEL 0x0066

// 285 mV burst
#define BURST_LEVEL 0x0d66

#define SPI_RAM_SIZE (128 * 1024)

// COLORCLKS_PER_LINE can't be used in pre-processor logic.
#if ((((((227 * (PROTOLINES)) + 1) / 2) * 4) + ((TOTAL_LINES) * (LINE_INDEX_BYTE_SIZE)) + \
    ((PICLINE_LENGTH_BYTES) * (TV_HEIGHT))) > (SPI_RAM_SIZE))
#error "TV_WIDTH * TV_HEIGHT is too big!"
#endif

static void SpiRamWriteByteRegister(int opcode, int data)
{
    uint8_t packet[2] = {opcode, data};

    OMV_SPI_LCD_CS_LOW();
    HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, packet, sizeof(packet), HAL_MAX_DELAY);
    OMV_SPI_LCD_CS_HIGH();
}

static int SpiRamReadByteRegister(int opcode)
{
    uint8_t packet[2] = {opcode, 0};

    OMV_SPI_LCD_CS_LOW();
    HAL_SPI_TransmitReceive(OMV_SPI_LCD_CONTROLLER->spi, packet, packet, sizeof(packet), HAL_MAX_DELAY);
    OMV_SPI_LCD_CS_HIGH();

    return packet[1];
}

static void SpiRamWriteWordRegister(int opcode, int data)
{
    uint8_t packet[3] = {opcode, data >> 8, data};

    OMV_SPI_LCD_CS_LOW();
    HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, packet, sizeof(packet), HAL_MAX_DELAY);
    OMV_SPI_LCD_CS_HIGH();
}

static void SpiClearRam()
{
    uint8_t packet[4] = {WRITE_SRAM, 0, 0, 0};

    OMV_SPI_LCD_CS_LOW();
    HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, packet, sizeof(packet), HAL_MAX_DELAY);

    packet[0] = 0;

    for (int i = 0; i < (SPI_RAM_SIZE / sizeof(packet)); i++) {
        HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, packet, sizeof(packet), HAL_MAX_DELAY);
    }

    OMV_SPI_LCD_CS_HIGH();
}

static void SpiRamWriteProgram(int data0, int data1, int data2, int data3)
{
    uint8_t packet[5] = {PROGRAM, data3, data2, data1, data0};

    OMV_SPI_LCD_CS_LOW();
    HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, packet, sizeof(packet), HAL_MAX_DELAY);
    OMV_SPI_LCD_CS_HIGH();
}

static void SpiRamWriteLowPassFilter(int data)
{
    uint8_t packet[6] = {WRITE_BLOCKMVC1, 0, 0, 0, 0, data};

    OMV_SPI_LCD_CS_LOW();
    HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, packet, sizeof(packet), HAL_MAX_DELAY);
    OMV_SPI_LCD_CS_HIGH();
}

static void SpiRamWriteWord(int w_address, int data)
{
    int address = w_address * sizeof(uint16_t);
    uint8_t packet[6] = {WRITE_SRAM, address >> 16, address >> 8, address, data >> 8, data};

    OMV_SPI_LCD_CS_LOW();
    HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, packet, sizeof(packet), HAL_MAX_DELAY);
    OMV_SPI_LCD_CS_HIGH();
}

static void SpiRamWriteVSyncProtoLine(int line, int length_1, int length_2)
{
    int w0 = PROTOLINE_WORD_ADDRESS(line);
    for (int i = 0; i < COLORCLKS_PER_LINE; i++) {
        SpiRamWriteWord(w0++, BLANK_LEVEL);
    }

    int w1 = PROTOLINE_WORD_ADDRESS(line);
    for (int i = 0; i < length_1; i++) {
        SpiRamWriteWord(w1++, SYNC_LEVEL);
    }

    int w2 = PROTOLINE_WORD_ADDRESS(line) + COLORCLKS_LINE_HALF;
    for (int i = 0; i < length_2; i++) {
        SpiRamWriteWord(w2++, SYNC_LEVEL);
    }
}

static void SpiRamWriteLine(int line, int index)
{
    int address = INDEX_START_BYTES + (line * LINE_INDEX_BYTE_SIZE);
    int data = index << 7;
    uint8_t packet[7] = {WRITE_SRAM, address >> 16, address >> 8, address, data, data >> 8, data >> 16};

    OMV_SPI_LCD_CS_LOW();
    HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, packet, sizeof(packet), HAL_MAX_DELAY);
    OMV_SPI_LCD_CS_HIGH();
}

static void SpiRamVideoInit()
{
    // Select the first VS23 for following commands in case there
    // are several VS23 ICs connected to same SPI bus.
    SpiRamWriteByteRegister(WRITE_MULTIIC, 0xe);

    // Set SPI memory address autoincrement
    SpiRamWriteByteRegister(WRITE_STATUS, 0x40);

    // Reset the video display controller
    SpiRamWriteWordRegister(VDCTRL1, 0);
    SpiRamWriteWordRegister(VDCTRL2, 0);

    // Write picture start and end
    SpiRamWriteWordRegister(PICSTART, (STARTPIX - 1));
    SpiRamWriteWordRegister(PICEND, (ENDPIX - 1));

    // Enable PLL clock
    SpiRamWriteWordRegister(VDCTRL1, VDCTRL1_PLL_ENABLE | VDCTRL1_UVSKIP);

    // Clear the video memory
    SpiClearRam();

    // Set length of one complete line (unit: PLL clocks)
    SpiRamWriteWordRegister(LINELEN, PLLCLKS_PER_LINE);

    // Set microcode program for picture lines
    SpiRamWriteProgram(OP1, OP2, OP3, OP4);

    // Define where Line Indexes are stored in memory
    SpiRamWriteWordRegister(INDEXSTART, INDEX_START_LONGWORDS);

    // At this time, the chip would continuously output the proto line 0.
    // This protoline will become our most "normal" horizontal line.
    // For TV-Out, fill the line with black level,
    // and insert a few pixels of sync level (0) and color burst to the beginning.
    // Note that the chip hardware adds black level to all nonproto areas so
    // protolines and normal picture have different meaning for the same Y value.
    // In protolines, Y=0 is at sync level and in normal picture Y=0 is at black level (offset +102).

    // In protolines, each pixel is 8 PLLCLKs, which in TV-out modes means one color
    // subcarrier cycle. Each pixel has 16 bits (one word): VVVVUUUUYYYYYYYY.

    SpiRamWriteVSyncProtoLine(0, SYNC, 0);

    int w = PROTOLINE_WORD_ADDRESS(0) + BURST;
    for (int i = 0; i < BURST_DUR; i++) {
        SpiRamWriteWord(w++, BURST_LEVEL);
    }

    // short_low + long_high + short_low + long_high
    SpiRamWriteVSyncProtoLine(1, SHORTSYNC, SHORTSYNCM);

    // long_low + short_high + long_low + short_high
    SpiRamWriteVSyncProtoLine(2, LONGSYNC, LONGSYNCM);

    for (int i = 0; i <= 2; i++) {
        SpiRamWriteLine(i, PROTOLINE_BYTE_ADDRESS(1)); // short_low + long_high + short_low + long_high
    }

    for (int i = 3; i <= 5; i++) {
        SpiRamWriteLine(i, PROTOLINE_BYTE_ADDRESS(2)); // long_low + short_high + long_low + short_high
    }

    for (int i = 6; i <= 8; i++) {
        SpiRamWriteLine(i, PROTOLINE_BYTE_ADDRESS(1)); // short_low + long_high + short_low + long_high
    }

    // Set pic line indexes to point to protoline 0 and their individual picture line.
    for (int i = 0; i < TV_HEIGHT; i++) {
        SpiRamWriteLine(STARTLINE + i, PICLINE_BYTE_ADDRESS(i));
    }

    // Set number of lines, length of pixel and enable video generation
    SpiRamWriteWordRegister(VDCTRL2, (VDCTRL2_LINECOUNT * (TOTAL_LINES - 1))
    | (VDCTRL2_PIXEL_WIDTH * (PLLCLKS_PER_PIXEL - 1))
    | (VDCTRL2_ENABLE_VIDEO));

    // Enable the low-pass Y filter.
    SpiRamWriteLowPassFilter(BLOCKMVC1_PYF);
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
#endif

// TV lines are converted from 16-bit RGB565 to 12-bit YUV.
#define TV_WIDTH_RGB565 ((TV_WIDTH) * 2) // bytes

#if ((PICLINE_LENGTH_BYTES) > (TV_WIDTH_RGB565))
#error "PICLINE_LENGTH_BYTES > TV_WIDTH_RGB565"
#endif

#define FRAMEBUFFER_COUNT 3
static int framebuffer_head = 0;
static volatile int framebuffer_tail = 0;
static uint16_t *framebuffers[FRAMEBUFFER_COUNT] = {};

static enum {
    TV_NONE,
    TV_SHIELD,
} tv_type = TV_NONE;

static bool tv_triple_buffer = false;

#ifdef OMV_SPI_LCD_CONTROLLER
static DMA_HandleTypeDef spi_tx_dma = {};

static volatile enum {
    SPI_TX_CB_IDLE,
    SPI_TX_CB_MEMORY_WRITE_CMD,
    SPI_TX_CB_MEMORY_WRITE
} spi_tx_cb_state = SPI_TX_CB_IDLE;

static void spi_config_deinit()
{
    if (tv_triple_buffer) {
        HAL_SPI_Abort(OMV_SPI_LCD_CONTROLLER->spi);
        spi_tx_cb_state = SPI_TX_CB_IDLE;
        fb_alloc_free_till_mark_past_mark_permanent();
    }

    spi_deinit(OMV_SPI_LCD_CONTROLLER);

    // Do not put in HAL_SPI_MspDeinit as other modules share the SPI2 bus.

    HAL_GPIO_DeInit(OMV_SPI_LCD_MOSI_PORT, OMV_SPI_LCD_MOSI_PIN);
    HAL_GPIO_DeInit(OMV_SPI_LCD_MISO_PORT, OMV_SPI_LCD_MISO_PIN);
    HAL_GPIO_DeInit(OMV_SPI_LCD_SCLK_PORT, OMV_SPI_LCD_SCLK_PIN);

    HAL_GPIO_DeInit(OMV_SPI_LCD_CS_PORT, OMV_SPI_LCD_CS_PIN);

    ///////////////////////////////////////////////////////////////////////
}

static void spi_tv_callback(SPI_HandleTypeDef *hspi);

static void spi_config_init(bool triple_buffer)
{
    OMV_SPI_LCD_CONTROLLER->spi->Init.Mode = SPI_MODE_MASTER;
    OMV_SPI_LCD_CONTROLLER->spi->Init.Direction = SPI_DIRECTION_2LINES;
    OMV_SPI_LCD_CONTROLLER->spi->Init.NSS = SPI_NSS_SOFT;
    OMV_SPI_LCD_CONTROLLER->spi->Init.TIMode = SPI_TIMODE_DISABLE;
    OMV_SPI_LCD_CONTROLLER->spi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi_set_params(OMV_SPI_LCD_CONTROLLER, 0xffffffff,
                   TV_WIDTH * TV_HEIGHT * TV_REFRESH * PICBITS,
                   0, 0, 8, 0);
    spi_init(OMV_SPI_LCD_CONTROLLER, true);
    HAL_SPI_RegisterCallback(OMV_SPI_LCD_CONTROLLER->spi, HAL_SPI_TX_COMPLETE_CB_ID, spi_tv_callback);

    // Do not put in HAL_SPI_MspInit as other modules share the SPI2 bus.

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pull      = GPIO_NOPULL;
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_MEDIUM;

    GPIO_InitStructure.Alternate = OMV_SPI_LCD_MOSI_ALT;
    GPIO_InitStructure.Pin       = OMV_SPI_LCD_MOSI_PIN;
    HAL_GPIO_Init(OMV_SPI_LCD_MOSI_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Alternate = OMV_SPI_LCD_MISO_ALT;
    GPIO_InitStructure.Pin       = OMV_SPI_LCD_MISO_PIN;
    HAL_GPIO_Init(OMV_SPI_LCD_MISO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Alternate = OMV_SPI_LCD_SCLK_ALT;
    GPIO_InitStructure.Pin       = OMV_SPI_LCD_SCLK_PIN;
    HAL_GPIO_Init(OMV_SPI_LCD_SCLK_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStructure.Pin       = OMV_SPI_LCD_CS_PIN;
    HAL_GPIO_Init(OMV_SPI_LCD_CS_PORT, &GPIO_InitStructure);
    OMV_SPI_LCD_CS_HIGH();

    /////////////////////////////////////////////////////////////////////

    SpiRamVideoInit();

    // Set default channel.
    SpiRamWriteByteRegister(WRITE_GPIO, 0x77);

    if (triple_buffer) {
        fb_alloc_mark();

        framebuffer_head = 0;
        framebuffer_tail = 0;

        for (int i = 0; i < FRAMEBUFFER_COUNT; i++) {
            framebuffers[i] = (uint16_t *) fb_alloc0(TV_WIDTH_RGB565 * TV_HEIGHT, FB_ALLOC_CACHE_ALIGN);
        }

        dma_init(&spi_tx_dma, OMV_SPI_LCD_CONTROLLER->tx_dma_descr, DMA_MEMORY_TO_PERIPH, OMV_SPI_LCD_CONTROLLER->spi);
        OMV_SPI_LCD_CONTROLLER->spi->hdmatx = &spi_tx_dma;
        OMV_SPI_LCD_CONTROLLER->spi->hdmarx = NULL;
#if defined(MCU_SERIES_H7)
        spi_tx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
#else
        spi_tx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
#endif
        spi_tx_dma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        spi_tx_dma.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
        spi_tx_dma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        spi_tx_dma.Init.MemBurst = DMA_MBURST_INC4;
#if defined(MCU_SERIES_H7)
        spi_tx_dma.Init.PeriphBurst = DMA_PBURST_INC4;
#else
        spi_tx_dma.Init.PeriphBurst = DMA_PBURST_SINGLE;
#endif
#if defined(MCU_SERIES_H7)
        ((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->CR =
            (((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->CR & ~DMA_SxCR_PSIZE_Msk) | DMA_PDATAALIGN_WORD;
#else
        ((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->CR =
            (((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->CR & ~DMA_SxCR_PSIZE_Msk) | DMA_PDATAALIGN_BYTE;
#endif
        ((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->CR =
            (((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->CR & ~DMA_SxCR_MSIZE_Msk) | DMA_MDATAALIGN_WORD;
        ((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->FCR =
            (((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->FCR & ~DMA_SxFCR_DMDIS_Msk) | DMA_FIFOMODE_ENABLE;
        ((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->FCR =
            (((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->FCR & ~DMA_SxFCR_FTH_Msk) | DMA_FIFO_THRESHOLD_FULL;
        ((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->CR =
            (((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->CR & ~DMA_SxCR_MBURST_Msk) | DMA_MBURST_INC4;
#if defined(MCU_SERIES_H7)
        ((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->CR =
            (((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->CR & ~DMA_SxCR_PBURST_Msk) | DMA_PBURST_INC4;
#else
        ((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->CR =
            (((DMA_Stream_TypeDef *) spi_tx_dma.Instance)->CR & ~DMA_SxCR_PBURST_Msk) | DMA_PBURST_SINGLE;
#endif
        fb_alloc_mark_permanent();
    }
}

static const uint8_t write_sram[] = { // Cannot be allocated on the stack for HAL_SPI_Transmit_IT().
    WRITE_SRAM,
    (uint8_t) (PICLINE_BYTE_ADDRESS(0) >> 16),
    (uint8_t) (PICLINE_BYTE_ADDRESS(0) >> 8),
    (uint8_t) (PICLINE_BYTE_ADDRESS(0) >> 0)
};

static void spi_tv_callback(SPI_HandleTypeDef *hspi)
{
    if (tv_type == TV_SHIELD) {
        static uint8_t *spi_tx_cb_state_memory_write_addr = NULL;
        static size_t spi_tx_cb_state_memory_write_count = 0;

        switch (spi_tx_cb_state) {
            case SPI_TX_CB_MEMORY_WRITE_CMD: {
                OMV_SPI_LCD_CS_HIGH();
                spi_tx_cb_state = SPI_TX_CB_MEMORY_WRITE;
                spi_tx_cb_state_memory_write_addr = (uint8_t *) framebuffers[framebuffer_head];
                spi_tx_cb_state_memory_write_count = PICLINE_LENGTH_BYTES * TV_HEIGHT;
                framebuffer_tail = framebuffer_head;
#if defined(MCU_SERIES_H7)
                OMV_SPI_LCD_CONTROLLER->spi->Instance->CFG1 =
                    (OMV_SPI_LCD_CONTROLLER->spi->Instance->CFG1 & ~SPI_CFG1_FTHLV_Msk) | SPI_FIFO_THRESHOLD_01DATA;
#endif
                OMV_SPI_LCD_CS_LOW();
                // When starting the interrupt chain the first HAL_SPI_Transmit_IT is not executed
                // in interrupt context. So, disable interrupts for the first HAL_SPI_Transmit_IT so
                // that it completes first and unlocks the SPI bus before allowing the interrupt
                // it causes to trigger starting the interrupt chain.
                uint32_t irq_state = disable_irq();
                HAL_SPI_Transmit_IT(OMV_SPI_LCD_CONTROLLER->spi, (uint8_t *) write_sram, sizeof(write_sram));
                enable_irq(irq_state);
                break;
            }
            case SPI_TX_CB_MEMORY_WRITE: {
                uint8_t *addr = spi_tx_cb_state_memory_write_addr;
                size_t count = IM_MIN(spi_tx_cb_state_memory_write_count, (65536-16));
                spi_tx_cb_state = (spi_tx_cb_state_memory_write_count > (65536-16))
                        ? SPI_TX_CB_MEMORY_WRITE
                        : SPI_TX_CB_MEMORY_WRITE_CMD;
                spi_tx_cb_state_memory_write_addr += count;
                spi_tx_cb_state_memory_write_count -= count;
#if defined(MCU_SERIES_H7)
                OMV_SPI_LCD_CONTROLLER->spi->Instance->CFG1 =
                    (OMV_SPI_LCD_CONTROLLER->spi->Instance->CFG1 & ~SPI_CFG1_FTHLV_Msk) | SPI_FIFO_THRESHOLD_16DATA;
#endif
                HAL_SPI_Transmit_DMA(OMV_SPI_LCD_CONTROLLER->spi, addr, count);
                break;
            }
            default: {
                break;
            }
        }
    }
}

// Convert a 8-bit Grayscale line of pixels to 12-bit YUV422 with padding (line is 16-bit per pixel).
static void spi_tv_draw_image_cb_convert_grayscale(uint8_t *row_pointer_i, uint8_t *row_pointer_o)
{
    for (int i = TV_WIDTH - 2, j = ((TV_WIDTH * 3) / 2) - 3; i >= 0; i -= 2, j -= 3) {
        int y0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_pointer_i, i);
        int y1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_pointer_i, i + 1);
        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_pointer_o, j, 0);
        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_pointer_o, j + 1, y0);
        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_pointer_o, j + 2, y1);
    }
}

// Convert a 16-bit RGB565 line of pixels to 12-bit YUV422 with padding (line is 16-bit per pixel).
static void spi_tv_draw_image_cb_convert_rgb565(uint16_t *row_pointer_i, uint8_t *row_pointer_o)
{
    for (int i = 0, j = 0; i < TV_WIDTH; i += 2, j += 3) {
        #if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7) || defined(MCU_SERIES_H7)

        int pixels = *((uint32_t *) (row_pointer_i + i));
        int r_pixels = ((pixels >> 8) & 0xf800f8) | ((pixels >> 13) & 0x70007);
        int g_pixels = ((pixels >> 3) & 0xfc00fc) | ((pixels >> 9) & 0x30003);
        int b_pixels = ((pixels << 3) & 0xf800f8) | ((pixels >> 2) & 0x70007);

        int y = ((r_pixels * 38) + (g_pixels * 75) + (b_pixels * 15)) >> 7;
        int u = __SSUB16(b_pixels * 64, (r_pixels * 21) + (g_pixels * 43));
        int v = __SSUB16(r_pixels * 64, (g_pixels * 54) + (b_pixels * 10));

        int y0 = __UXTB_RORn(y, 0), y1 = __UXTB_RORn(y, 16);

        int u_avg = __SMUAD(u, 0x00010001) >> 7;
        int v_avg = __SMUAD(v, 0x00010001) >> 7;

        #else

        int pixel0 = IMAGE_GET_RGB565_PIXEL_FAST(row_pointer_i, i);
        int r0 = COLOR_RGB565_TO_R8(pixel0);
        int g0 = COLOR_RGB565_TO_G8(pixel0);
        int b0 = COLOR_RGB565_TO_B8(pixel0);
        int y0 = COLOR_RGB888_TO_Y(r0, g0, b0);
        int u0 = COLOR_RGB888_TO_U(r0, g0, b0);
        int v0 = COLOR_RGB888_TO_V(r0, g0, b0);

        int pixel1 = IMAGE_GET_RGB565_PIXEL_FAST(row_pointer_i, i + 1);
        int r1 = COLOR_RGB565_TO_R8(pixel1);
        int g1 = COLOR_RGB565_TO_G8(pixel1);
        int b1 = COLOR_RGB565_TO_B8(pixel1);
        int y1 = COLOR_RGB888_TO_Y(r1, g1, b1);
        int u1 = COLOR_RGB888_TO_U(r1, g1, b1);
        int v1 = COLOR_RGB888_TO_V(r1, g1, b1);

        int u_avg = u0 + u1;
        int v_avg = v0 + v1;

        #endif

        int uv = ((u_avg >> 1) & 0xf0) | (((-v_avg) >> 5) & 0xf);

        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_pointer_o, j, uv);
        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_pointer_o, j + 1, y0);
        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_pointer_o, j + 2, y1);
    }
}

static void spi_tv_draw_image_cb_grayscale(int x_start, int x_end, int y_row, imlib_draw_row_data_t *data)
{
    spi_tv_draw_image_cb_convert_grayscale((uint8_t *) data->dst_row_override, (uint8_t *) data->dst_row_override);
    HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, data->dst_row_override, PICLINE_LENGTH_BYTES, HAL_MAX_DELAY);
}

static void spi_tv_draw_image_cb_rgb565(int x_start, int x_end, int y_row, imlib_draw_row_data_t *data)
{
    spi_tv_draw_image_cb_convert_rgb565((uint16_t *) data->dst_row_override, (uint8_t *) data->dst_row_override);
    HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, data->dst_row_override, PICLINE_LENGTH_BYTES, HAL_MAX_DELAY);
}

static void spi_tv_display(image_t *src_img, int dst_x_start, int dst_y_start, float x_scale, float y_scale,
                           rectangle_t *roi, int rgb_channel, int alpha,
                           const uint16_t *color_palette, const uint8_t *alpha_palette,
                           image_hint_t hint)
{
    bool rgb565 = ((rgb_channel == -1) && IMAGE_IS_COLOR(src_img)) || color_palette;
    imlib_draw_row_callback_t cb = rgb565 ? spi_tv_draw_image_cb_rgb565 : spi_tv_draw_image_cb_grayscale;

    image_t dst_img;
    dst_img.w = TV_WIDTH;
    dst_img.h = TV_HEIGHT;
    dst_img.pixfmt = rgb565 ? PIXFORMAT_RGB565 : PIXFORMAT_GRAYSCALE;

    int x0, x1, y0, y1;
    bool black = !imlib_draw_image_rectangle(&dst_img, src_img, dst_x_start, dst_y_start, x_scale, y_scale,
                                             roi, alpha, alpha_palette, hint, &x0, &x1, &y0, &y1);

    if (!tv_triple_buffer) {
        dst_img.data = fb_alloc0(TV_WIDTH_RGB565, FB_ALLOC_NO_HINT);
        OMV_SPI_LCD_CS_LOW();

        HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, (uint8_t *) write_sram, sizeof(write_sram), HAL_MAX_DELAY);

        if (black) { // zero the whole image
            for (int i = 0; i < TV_HEIGHT; i++) {
                HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, dst_img.data, PICLINE_LENGTH_BYTES, HAL_MAX_DELAY);
            }
        } else {
            // Zero the top rows
            for (int i = 0; i < y0; i++) {
                HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, dst_img.data, PICLINE_LENGTH_BYTES, HAL_MAX_DELAY);
            }

            // Transmits left/right parts already zeroed...
            imlib_draw_image(&dst_img, src_img, dst_x_start, dst_y_start, x_scale, y_scale, roi,
                             rgb_channel, alpha, color_palette, alpha_palette, hint | IMAGE_HINT_BLACK_BACKGROUND,
                             cb, dst_img.data);

            // Zero the bottom rows
            if (y1 < TV_HEIGHT) {
                memset(dst_img.data, 0, TV_WIDTH_RGB565);
            }

            for (int i = y1; i < TV_HEIGHT; i++) {
                HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, dst_img.data, PICLINE_LENGTH_BYTES, HAL_MAX_DELAY);
            }
        }

        OMV_SPI_LCD_CS_HIGH();
        fb_free();
    } else {
        // For triple buffering we are never drawing where head or tail (which may instantly update to
        // to be equal to head) is.
        int new_framebuffer_head = (framebuffer_head + 1) % FRAMEBUFFER_COUNT;
        if (new_framebuffer_head == framebuffer_tail) {
            new_framebuffer_head = (new_framebuffer_head + 1) % FRAMEBUFFER_COUNT;
        }

        dst_img.data = (uint8_t *) framebuffers[new_framebuffer_head];

        if (rgb565) {
            if (black) { // zero the whole image
                memset(dst_img.data, 0, TV_WIDTH * TV_HEIGHT * sizeof(uint16_t));
            } else {
                // Zero the top rows
                if (y0) {
                    memset(dst_img.data, 0, TV_WIDTH * y0 * sizeof(uint16_t));
                }

                if (x0) {
                    for (int i = y0; i < y1; i++) { // Zero left
                        memset(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&dst_img, i), 0, x0 * sizeof(uint16_t));
                    }
                }

                imlib_draw_image(&dst_img, src_img, dst_x_start, dst_y_start, x_scale, y_scale, roi,
                                rgb_channel, alpha, color_palette, alpha_palette, hint | IMAGE_HINT_BLACK_BACKGROUND,
                                NULL, NULL);

                if (TV_WIDTH - x1) {
                    for (int i = y0; i < y1; i++) { // Zero right
                        memset(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&dst_img, i) + x1, 0,
                            (TV_WIDTH - x1) * sizeof(uint16_t));
                    }
                }

                // Zero the bottom rows
                if (TV_HEIGHT - y1) {
                    memset(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&dst_img, y1), 0,
                        TV_WIDTH * (TV_HEIGHT - y1) * sizeof(uint16_t));
                }
            }

            for (int i = 0; i < TV_HEIGHT; i++) { // Convert the image.
                spi_tv_draw_image_cb_convert_rgb565(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&dst_img, i),
                                                    dst_img.data + (PICLINE_LENGTH_BYTES * i));
            }
        } else {
            if (black) { // zero the whole image
                memset(dst_img.data, 0, TV_WIDTH * TV_HEIGHT * sizeof(uint8_t));
            } else {
                // Zero the top rows
                if (y0) {
                    memset(dst_img.data, 0, TV_WIDTH * y0 * sizeof(uint8_t));
                }

                if (x0) {
                    for (int i = y0; i < y1; i++) { // Zero left
                        memset(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&dst_img, i), 0, x0 * sizeof(uint8_t));
                    }
                }

                imlib_draw_image(&dst_img, src_img, dst_x_start, dst_y_start, x_scale, y_scale, roi,
                                rgb_channel, alpha, color_palette, alpha_palette, hint | IMAGE_HINT_BLACK_BACKGROUND,
                                NULL, NULL);

                if (TV_WIDTH - x1) {
                    for (int i = y0; i < y1; i++) { // Zero right
                        memset(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&dst_img, i) + x1, 0,
                            (TV_WIDTH - x1) * sizeof(uint8_t));
                    }
                }

                // Zero the bottom rows
                if (TV_HEIGHT - y1) {
                    memset(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&dst_img, y1), 0,
                        TV_WIDTH * (TV_HEIGHT - y1) * sizeof(uint8_t));
                }
            }

            for (int i = TV_HEIGHT - 1; i >= 0; i--) { // Convert the image.
                spi_tv_draw_image_cb_convert_grayscale(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&dst_img, i),
                                                       dst_img.data + (PICLINE_LENGTH_BYTES * i));
            }
        }

        #ifdef __DCACHE_PRESENT
        // Flush data for DMA
        SCB_CleanDCache();
        #endif

        // Update head which means a new image is ready.
        framebuffer_head = new_framebuffer_head;

        // Kick off an update of the display.
        if (spi_tx_cb_state == SPI_TX_CB_IDLE) {
            spi_tx_cb_state = SPI_TX_CB_MEMORY_WRITE_CMD;
            spi_tv_callback(OMV_SPI_LCD_CONTROLLER->spi);
        }
    }
}
#endif

STATIC mp_obj_t py_tv_deinit()
{
    switch (tv_type) {
        #ifdef OMV_SPI_LCD_CONTROLLER
        case TV_SHIELD: {
            spi_config_deinit();
            break;
        }
        #endif
        default: {
            break;
        }
    }

    tv_triple_buffer = false;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tv_deinit_obj, py_tv_deinit);

STATIC mp_obj_t py_tv_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    py_tv_deinit();

    int type = py_helper_keyword_int(n_args, args, 0, kw_args,
                                     MP_OBJ_NEW_QSTR(MP_QSTR_type), TV_SHIELD);

    switch (type) {
        #ifdef OMV_SPI_LCD_CONTROLLER
        case TV_SHIELD: {
            bool triple_buffer = py_helper_keyword_int(n_args, args, 1, kw_args,
                                                       MP_OBJ_NEW_QSTR(MP_QSTR_triple_buffer), false);
            spi_config_init(triple_buffer);
            tv_type = TV_SHIELD;
            tv_triple_buffer = triple_buffer;
            break;
        }
        #endif
        default: {
            break;
        }
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tv_init_obj, 0, py_tv_init);

STATIC mp_obj_t py_tv_width()
{
    if (tv_type == TV_NONE) {
        return mp_const_none;
    }

    return mp_obj_new_int(TV_WIDTH);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tv_width_obj, py_tv_width);

STATIC mp_obj_t py_tv_height()
{
    if (tv_type == TV_NONE) {
        return mp_const_none;
    }

    return mp_obj_new_int(TV_HEIGHT);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tv_height_obj, py_tv_height);

STATIC mp_obj_t py_tv_type()
{
    if (tv_type == TV_NONE) {
        return mp_const_none;
    }

    return mp_obj_new_int(tv_type);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tv_type_obj, py_tv_type);

STATIC mp_obj_t py_tv_triple_buffer()
{
    if (tv_type == TV_NONE) {
        return mp_const_none;
    }

    return mp_obj_new_int(tv_triple_buffer);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tv_triple_buffer_obj, py_tv_triple_buffer);

STATIC mp_obj_t py_tv_refresh()
{
    if (tv_type == TV_NONE) {
        return mp_const_none;
    }

    return mp_obj_new_int(TV_REFRESH);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tv_refresh_obj, py_tv_refresh);

STATIC mp_obj_t py_tv_channel(uint n_args, const mp_obj_t *args)
{
    if (tv_type == TV_NONE) {
        return mp_const_none;
    }

    #ifdef OMV_SPI_LCD_CONTROLLER
    if (tv_triple_buffer) {
        HAL_SPI_Abort(OMV_SPI_LCD_CONTROLLER->spi);
        spi_tx_cb_state = SPI_TX_CB_IDLE;
        OMV_SPI_LCD_CS_HIGH();
    }

    if (n_args) {
        int channel = mp_obj_get_int(*args);

        if ((channel < 1) || (8 < channel)) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("1 <= channel <= 8!"));
        }

        SpiRamWriteByteRegister(WRITE_GPIO, 0x70 | (channel - 1));
    } else {
        int channel = SpiRamReadByteRegister(READ_GPIO);
        return mp_obj_new_int((channel & 0x7) + 1);
    }
    #endif

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_tv_channel_obj, 0, 1, py_tv_channel);

STATIC mp_obj_t py_tv_display(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_helper_arg_to_image_mutable_bayer_jpeg(args[0]);

    int arg_x_off = 0;
    int arg_y_off = 0;
    uint offset = 1;
    if (n_args > 1) {
        if (MP_OBJ_IS_TYPE(args[1], &mp_type_tuple) || MP_OBJ_IS_TYPE(args[1], &mp_type_list)) {
            mp_obj_t *arg_vec;
            mp_obj_get_array_fixed_n(args[1], 2, &arg_vec);
            arg_x_off = mp_obj_get_int(arg_vec[0]);
            arg_y_off = mp_obj_get_int(arg_vec[1]);
            offset = 2;
        } else if (n_args > 2) {
            arg_x_off = mp_obj_get_int(args[1]);
            arg_y_off = mp_obj_get_int(args[2]);
            offset = 3;
        } else if (n_args > 1) {
            mp_raise_msg(&mp_type_TypeError, MP_ERROR_TEXT("Expected x and y offset!"));
        }
    }

    float arg_x_scale = 1.f;
    bool got_x_scale = py_helper_keyword_float_maybe(n_args, args, offset, kw_args,
                                                     MP_OBJ_NEW_QSTR(MP_QSTR_x_scale), &arg_x_scale);

    float arg_y_scale = 1.f;
    bool got_y_scale = py_helper_keyword_float_maybe(n_args, args, offset + 1, kw_args,
                                                     MP_OBJ_NEW_QSTR(MP_QSTR_y_scale), &arg_y_scale);

    rectangle_t arg_roi;
    py_helper_keyword_rectangle_roi(arg_img, n_args, args, offset + 2, kw_args, &arg_roi);

    int arg_rgb_channel = py_helper_keyword_int(n_args, args, offset + 3, kw_args,
                                                MP_OBJ_NEW_QSTR(MP_QSTR_rgb_channel), -1);

    if ((arg_rgb_channel < -1) || (2 < arg_rgb_channel)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("-1 <= rgb_channel <= 2!"));
    }

    int arg_alpha = py_helper_keyword_int(n_args, args, offset + 4, kw_args,
                                          MP_OBJ_NEW_QSTR(MP_QSTR_alpha), 256);

    if ((arg_alpha < 0) || (256 < arg_alpha)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= alpha <= 256!"));
    }

    const uint16_t *color_palette = py_helper_keyword_color_palette(n_args, args, offset + 5, kw_args, NULL);
    const uint8_t *alpha_palette = py_helper_keyword_alpha_palette(n_args, args, offset + 6, kw_args, NULL);

    image_hint_t hint = py_helper_keyword_int(n_args, args, offset + 7, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_hint), 0);

    int arg_x_size;
    bool got_x_size = py_helper_keyword_int_maybe(n_args, args, offset + 8, kw_args,
                                                  MP_OBJ_NEW_QSTR(MP_QSTR_x_size), &arg_x_size);

    int arg_y_size;
    bool got_y_size = py_helper_keyword_int_maybe(n_args, args, offset + 9, kw_args,
                                                  MP_OBJ_NEW_QSTR(MP_QSTR_y_size), &arg_y_size);

    if (got_x_scale && got_x_size) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Choose either x_scale or x_size not both!"));
    }

    if (got_y_scale && got_y_size) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Choose either y_scale or y_size not both!"));
    }

    if (got_x_size) {
        arg_x_scale = arg_x_size / ((float) arg_roi.w);
    }

    if (got_y_size) {
        arg_y_scale = arg_y_size / ((float) arg_roi.h);
    }

    if ((!got_x_scale) && (!got_x_size) && got_y_size) {
        arg_x_scale = arg_y_scale;
    }

    if ((!got_y_scale) && (!got_y_size) && got_x_size) {
        arg_y_scale = arg_x_scale;
    }

    switch (tv_type) {
        #ifdef OMV_SPI_LCD_CONTROLLER
        case TV_SHIELD: {
            fb_alloc_mark();
            spi_tv_display(arg_img, arg_x_off, arg_y_off, arg_x_scale, arg_y_scale, &arg_roi,
                           arg_rgb_channel, arg_alpha, color_palette, alpha_palette, hint);
            fb_alloc_free_till_mark();
            break;
        }
        #endif
        default: {
            break;
        }
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tv_display_obj, 1, py_tv_display);

STATIC mp_obj_t py_tv_clear()
{
    switch (tv_type) {
        #ifdef OMV_SPI_LCD_CONTROLLER
        case TV_SHIELD: {
            fb_alloc_mark();
            spi_tv_display(NULL, 0, 0, 1.f, 1.f, NULL,
                            0, 0, NULL, NULL, 0);
            fb_alloc_free_till_mark();
            break;
        }
        #endif
        default: {
            break;
        }
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tv_clear_obj, py_tv_clear);

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),                MP_OBJ_NEW_QSTR(MP_QSTR_tv)                     },
    { MP_ROM_QSTR(MP_QSTR_TV_NONE),                 MP_ROM_INT(TV_NONE)                             },
    { MP_ROM_QSTR(MP_QSTR_TV_SHIELD),               MP_ROM_INT(TV_SHIELD)                           },
    { MP_ROM_QSTR(MP_QSTR_init),                    MP_ROM_PTR(&py_tv_init_obj)                     },
    { MP_ROM_QSTR(MP_QSTR_deinit),                  MP_ROM_PTR(&py_tv_deinit_obj)                   },
    { MP_ROM_QSTR(MP_QSTR_width),                   MP_ROM_PTR(&py_tv_width_obj)                    },
    { MP_ROM_QSTR(MP_QSTR_height),                  MP_ROM_PTR(&py_tv_height_obj)                   },
    { MP_ROM_QSTR(MP_QSTR_type),                    MP_ROM_PTR(&py_tv_type_obj)                     },
    { MP_ROM_QSTR(MP_QSTR_triple_buffer),           MP_ROM_PTR(&py_tv_triple_buffer_obj)            },
    { MP_ROM_QSTR(MP_QSTR_refresh),                 MP_ROM_PTR(&py_tv_refresh_obj)                  },
    { MP_ROM_QSTR(MP_QSTR_channel),                 MP_ROM_PTR(&py_tv_channel_obj)                  },
    { MP_ROM_QSTR(MP_QSTR_display),                 MP_ROM_PTR(&py_tv_display_obj)                  },
    { MP_ROM_QSTR(MP_QSTR_clear),                   MP_ROM_PTR(&py_tv_clear_obj)                    },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t tv_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

void py_tv_init0()
{
    py_tv_deinit();
}

MP_REGISTER_MODULE(MP_QSTR_tv, tv_module, MICROPY_PY_TV);
#endif // MICROPY_PY_TV
