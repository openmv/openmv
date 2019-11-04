/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2018 kaizhi
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * TV Python module.
 *
 */
#include <mp.h>
#include <objstr.h>
#include <spi.h>
#include <systick.h>
#include "imlib.h"
#include "fb_alloc.h"
#include "ff_wrapper.h"
#include "py_assert.h"
#include "py_helper.h"
#include "py_image.h"

// Crystal frequency in MHZ (float, observe accuracy)
// In fact I use 28.63636 MHZ crystal and disable 8x PLL. 
// But calculation worong if I change XTAL_MHZ value.
#define XTAL_MHZ 3.579545
// Line length in microseconds (float, observe accuracy)
#define LINE_LENGTH_US 63.5555
// Frame length in lines (visible lines + nonvisible lines)
// Amount has to be odd for NTSC and RGB colors
#define TOTAL_LINES 263
// Number of lines used after the VSYNC but before visible area.
#define FRONT_PORCH_LINES 3
// Width, in PLL clocks, of each pixel
// Used 4 to 8 for 160x120 pics
#define PLLCLKS_PER_PIXEL 9 // 4 is too short.
// Extra bytes can be added to end of picture lines to prevent pic-to-proto 
// border artifacts. 8 is a good value. 0 can be tried to test, if there is 
// no need for extra bytes.        
#define BEXTRA 8

//// Protolines ////

// Reserve memory for this number of different prototype lines
// (prototype lines are used for sync timing, porch and border area)
#define PROTOLINES 3
// if your real protoline lenght is longer than one slot, you must 
// use several slots per proto and there are total 16 slots
#define PROTOLINE_LENGTH_WORDS 512

// Protoline 0 starts always at address 0
#define PROTOLINE_BYTE_ADDRESS(n) (PROTOLINE_LENGTH_WORDS) *2 *(n)) // 512 * 2 * n = 1024*n
#define PROTOLINE_WORD_ADDRESS(n) (PROTOLINE_LENGTH_WORDS * (n)) // 512 * n = 512*n

// These are for proto lines and so format is VVVVUUUUYYYYYYYY 
// Sync is always 0
#define SYNC_LEVEL  0x0000
// 285 mV to 75 ohm load
#define BLANK_LEVEL 0x0066
// 339 mV to 75 ohm load
#define BLACK_LEVEL 0x0066
// Color burst
#define BURST_LEVEL (0x0d00 + BLACK_LEVEL)
#define WHITE_LEVEL 0x00ff

//// NTSC video timing constants ////
// NTSC short sync duration is 2.542 us
#define SHORT_SYNC_US 2.542
// For the start of the line, the first 10 extra PLLCLK sync (0) cycles
// are subtracted.
#define SHORTSYNC ((uint16_t)(SHORT_SYNC_US*XTAL_MHZ-10.0/8.0))
// For the middle of the line the whole duration of sync pulse is used.
#define SHORTSYNCM ((uint16_t)(SHORT_SYNC_US*XTAL_MHZ))
// NTSC long sync duration is 27.3 us
#define LONG_SYNC_US 27.33275
#define LONGSYNC ((uint16_t)(LONG_SYNC_US*XTAL_MHZ))
#define LONGSYNCM ((uint16_t)(LONG_SYNC_US*XTAL_MHZ))
// Normal visible picture line sync length is 4.7 us
#define SYNC_US 4.7
#define SYNC ((uint16_t)(SYNC_US*XTAL_MHZ-10.0/8.0))
// Color burst starts at 5.6 us
#define BURST_US 5.3
#define BURST ((uint16_t)(BURST_US*XTAL_MHZ-10.0/8.0))
// Color burst duration is 2.25 us
#define BURST_DUR_US 2.67
#define BURSTDUR ((uint16_t)(BURST_DUR_US*XTAL_MHZ))
// NTSC sync to blanking end time is 10.5 us
#define BLANK_END_US 9.155
#define BLANKEND ((uint16_t)(BLANK_END_US*XTAL_MHZ-10.0/8.0))
// Front porch starts at the end of the line, at 62.5us 
#define FRPORCH_US 61.8105
#define FRPORCH ((uint16_t)(FRPORCH_US*XTAL_MHZ-10.0/8.0))

/*
/// PAL video timing constants
/// PAL short sync duration is 2.35 us
#define SHORT_SYNC_US 2.35
/// For the start of the line, the first 10 extra PLLCLK sync (0) cycles
/// are subtracted.
#define SHORTSYNC ((uint16_t)(SHORT_SYNC_US*XTAL_MHZ-10.0/8.0))
/// For the middle of the line the whole duration of sync pulse is used.
#define SHORTSYNCM ((uint16_t)(SHORT_SYNC_US*XTAL_MHZ))
/// PAL long sync duration is 27.3 us
#define LONG_SYNC_US 27.3
#define LONGSYNC ((uint16_t)(LONG_SYNC_US*XTAL_MHZ))
#define LONGSYNCM ((uint16_t)(LONG_SYNC_US*XTAL_MHZ))
/// Normal visible picture line sync length is 4.7 us
#define SYNC_US 4.7
#define SYNC ((uint16_t)(SYNC_US*XTAL_MHZ-10.0/8.0))
/// Color burst starts at 5.6 us
#define BURST_US 5.6
#define BURST ((uint16_t)(BURST_US*XTAL_MHZ-10.0/8.0))
/// Color burst duration is 2.25 us
#define BURST_DUR_US 2.25
#define BURSTDUR ((uint16_t)(BURST_DUR_US*XTAL_MHZ))
/// PAL sync to blanking end time is 10.5 us
#define BLANK_END_US 10.5
#define BLANKEND ((uint16_t)(BLANK_END_US*XTAL_MHZ-10.0/8.0))
/// Front porch starts at the end of the line, at 62.5us 
#define FRPORCH_US 62.5
#define FRPORCH ((uint16_t)(FRPORCH_US*XTAL_MHZ-10.0/8.0))
*/

//// Definitions for picture lines ////

// On which line the picture area begins, the Y direction.
#define STARTLINE (FRONT_PORCH_LINES + 18)
#define YPIXELS 120
// The last picture area line
#define ENDLINE STARTLINE + YPIXELS
// The first pixel of the picture area, the X direction.
#define STARTPIX (BLANKEND+6)
// The last pixel of the picture area. Set PIXELS to wanted value and suitable 
// ENDPIX value is calculated.
#define XPIXELS 160
#define ENDPIX ((uint16_t)(STARTPIX+PLLCLKS_PER_PIXEL*XPIXELS/8))

// PLL frequency
#define PLL_MHZ (XTAL_MHZ * 8.0)
// 10 first pllclks, which are not in the counters are dePICLINE_LENGTH_BYTEScremented here
#define PLLCLKS_PER_LINE ((uint16_t)((LINE_LENGTH_US * PLL_MHZ)+0.5-10))
// 10 first pllclks, which are not in the counters are decremented here
#define COLORCLKS_PER_LINE ((uint16_t)((LINE_LENGTH_US * XTAL_MHZ)+0.5-10.0/8.0))
#define COLORCLKS_LINE_HALF ((uint16_t)((LINE_LENGTH_US * XTAL_MHZ)/2+0.5-10.0/8.0))

// Calculate picture lengths in pixels and bytes, coordinate areas for picture area
#define PICLENGTH (ENDPIX - STARTPIX)
#define PICX ((uint16_t)(PICLENGTH * 8 / PLLCLKS_PER_PIXEL))
#define PICY (ENDLINE-STARTLINE)

#define PICLINE_LENGTH_BYTES (XPIXELS*2)
// Picture area memory start point
#define PICLINE_START ((INDEX_START_BYTES + TOTAL_LINES*3+1)+1)

// Picture area line start addresses
#define PICLINE_WORD_ADDRESS(n) (PICLINE_START/2+(PICLINE_LENGTH_BYTES/2+BEXTRA/2)*(n))
#define PICLINE_BYTE_ADDRESS(n) ((uint32_t)(PICLINE_START+((uint32_t)(PICLINE_LENGTH_BYTES)+BEXTRA)*(n)))



//// Index start /////

#define PROTO_AREA_WORDS (PROTOLINE_LENGTH_WORDS * PROTOLINES) 
#define INDEX_START_LONGWORDS ((PROTO_AREA_WORDS+1)/2)
#define INDEX_START_WORDS (INDEX_START_LONGWORDS * 2)
#define INDEX_START_BYTES (INDEX_START_WORDS * 2)              

//// Pattern generator microcode ////

// Bits 7:6
#define PICK_A (0<<6) // 00=a
#define PICK_B (1<<6) // 01=b
#define PICK_Y (2<<6) // 10=y
#define PICK_NOTHING (3<<6) // 11=-

// Bits 5:3
// Pick 1..8
#define PICK_BITS(a) (((a)-1)<<3) 

// Bits 2:0  
// Shift 0..6
#define SHIFT_BITS(a) (a)

// The microcode is given as a 32-bit parameter to the SpiWrite-function, and must
// therefore be typecasted to unsigned long. Otherwise, if using Arduino, the values 
// shifted beyond 16-bit range are lost. VS1005 and VS1010 would not require typecasting
// in this instance.
// b=>u
// a=>v
// y=>luminance
#define OP1 (unsigned long)(PICK_B + PICK_BITS(4) + SHIFT_BITS(4))
#define OP2 (unsigned long)(PICK_A + PICK_BITS(4) + SHIFT_BITS(4))
#define OP3 (unsigned long)(PICK_Y + PICK_BITS(8) + SHIFT_BITS(6))
#define OP4 (unsigned long)(PICK_NOTHING + SHIFT_BITS(2))

//// VS23 SPI Commands ////

// General VS23 commands
#define WRITE_STATUS 0x01 // Write Status Register
#define WRITE 0x02 // Write SRAM
#define READ 0x03 // Read SRAM
#define WRITE_MULTIIC 0xb8 // Write Multi-IC Access Control
#define READ_MULTIIC 0xb7 // Read Multi-IC Access Control
#define READ_ID 0x9f // Read Manufacturer and Device ID


// Bit definitions
#define VDCTRL1 0x2B
#define VDCTRL1_UVSKIP (1<<0)
#define VDCTRL1_DACDIV (1<<3)
#define VDCTRL1_PLL_ENABLE (1<<12)
#define VDCTRL1_SELECT_PLL_CLOCK (1<<13)
#define VDCTRL1_USE_UVTABLE (1<<14)
#define VDCTRL1_DIRECT_DAC (1<<15)

#define VDCTRL2 0x2D
#define VDCTRL2_LINECOUNT ( (TOTAL_LINES-1) << 0)
#define VDCTRL2_PROGRAM_LENGTH ((PLLCLKS_PER_PIXEL-1)<<10)
#define VDCTRL2_NTSC (0<<14)
#define VDCTRL2_PAL (1<<14)
#define VDCTRL2_ENABLE_VIDEO (1<<15)

#define BLOCKMVC1_PYF (1<<4)

// VS23 video commands
#define PROGRAM 0x30
#define PICSTART 0x28
#define PICEND 0x29
#define LINELEN 0x2a
#define LINELEN_VGP_OUTPUT (1<<15)
#define YUVBITS 0x2b
#define INDEXSTART 0x2c
#define LINECFG 0x2d
#define VTABLE 0x2e
#define UTABLE 0x2f
#define BLOCKMVC1 0x34
#define CURLINE 0x53
#define GPIOCTL 0x82

#define CS_PORT             GPIOB
#define CS_PIN              GPIO_PIN_12
#define CS_PIN_WRITE(bit)   HAL_GPIO_WritePin(CS_PORT, CS_PIN, bit);

extern mp_obj_t pyb_spi_send(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);
//extern mp_obj_t pyb_spi_send_recv(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);
extern mp_obj_t pyb_spi_make_new(mp_obj_t type_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args);
extern mp_obj_t pyb_spi_deinit(mp_obj_t self_in);

static mp_obj_t spi_port = NULL;
static enum { TV_NONE, TV_SHIELD } type = TV_NONE;

static mp_obj_t SpiSendByte(register uint16_t b) {
    mp_map_t arg_map;
    arg_map.all_keys_are_qstrs = true;
    arg_map.is_fixed = true;
    arg_map.is_ordered = true;
    arg_map.used = 0;
    arg_map.alloc = 0;
    arg_map.table = NULL;
    mp_obj_t result;
    result = pyb_spi_send(
        2, (mp_obj_t []) {
            spi_port,
            mp_obj_new_int(b)
        },
        &arg_map
    );
    return result;
}
static mp_obj_t SpiSendLine(uint8_t *line, uint16_t length) {
    mp_map_t arg_map;
    arg_map.all_keys_are_qstrs = true;
    arg_map.is_fixed = true;
    arg_map.is_ordered = true;
    arg_map.used = 0;
    arg_map.alloc = 0;
    arg_map.table = NULL;
    pyb_spi_send(
        2, (mp_obj_t []) {
            spi_port,
            mp_obj_new_bytes(line, length)
        },
        &arg_map
    );
    return mp_const_none;
}
static mp_obj_t SpiSendWord(register uint16_t b) {
    mp_map_t arg_map;
    arg_map.all_keys_are_qstrs = true;
    arg_map.is_fixed = true;
    arg_map.is_ordered = true;
    arg_map.used = 0;
    arg_map.alloc = 0;
    arg_map.table = NULL;
    mp_obj_t result;

    uint8_t data[2] = {b >> 8, b & 0xff};

    result = pyb_spi_send(
        2, (mp_obj_t []) {
            spi_port,
            mp_obj_new_bytes(data, 2)
        },
        &arg_map
    );
    return result;
}
// SpiWrite uses SPI to write the VS23 registers, and to write VS23 SRAM
// addresses.
// The opcode-parameter determines the performed action. The opcodes
// have been defined, and their descriptions can be found in the
// VS23 datasheet.
// The address-parameter is only used when writing to SRAM with WRITE opcode.
// The data-parameter is used when writing to registers or memory. Generally
// only one or two bytes are written, with the exception of writing
// the microcode (PROGRAM opcode).
// The is16b-parameter must be non-zero if the write OR read value
// is a 16-bit word. Otherwise only a byte of the sent or received data is
// used. With the PROGRAM opcode the is16b parameter does not affect any
// functionality, and can be set to any value.
// If a register read opcode was given, the data will be set to the returned
// result-variable.
static mp_obj_t SpiWrite(register uint16_t opcode, register uint32_t address,
              register uint32_t data, uint16_t is16b)
{
  mp_obj_t result = mp_const_none;
  CS_PIN_WRITE(false);
  SpiSendByte(opcode);

  // Write the microcode, 4 bytes.
  // No need for result because operation is always a write.
  // The regular data write/read functions are skipped with the goto.
  if (opcode == PROGRAM) {
    SpiSendWord(data >> 16);
    SpiSendWord(data);
    goto END; // I don't like goto, but I don't want to change it.

    // With an SRAM write, the address is sent first. Data will be sent after
    // the else-if structure (goto END is not used, unlike with PROGRAM).
  } else if ( (opcode == WRITE) | (opcode == READ) ) {
    if (is16b) address = address << 1;
    // SRAM is 131072 bytes, making the last address 0x1FFFF (17 bits)
    SpiSendByte((address >> 16));
    SpiSendWord((address));
  }

  // Send or receive either one or two bytes of data. If a register or SRAM
  // write is done, the result-variable will not receive any return values.
  // Similarly, if a read is done the data-variable should be 0.
  if (is16b) result = SpiSendWord(data);
  else result = SpiSendByte(data);

END:
  CS_PIN_WRITE(true);
  return result; // Can be ignored if a write operation was done.
}
void protoline(uint16_t line, uint16_t offset, uint16_t limit, uint16_t data)
{
    uint16_t i = 0;
    uint16_t w = PROTOLINE_WORD_ADDRESS(line) + offset;
    if (offset == BLANKEND) i = BLANKEND;
    for (; i<=limit; i++) SpiWrite(WRITE, (uint16_t)w++, data, 1);
}
void SetLineIndex(uint16_t line, uint16_t wordAddress)
{
    uint32_t indexAddr = INDEX_START_BYTES + line*3;
    SpiWrite(WRITE, indexAddr++, 0, 0);
    SpiWrite(WRITE, indexAddr++, wordAddress, 0);
    SpiWrite(WRITE, indexAddr++, wordAddress >> 8, 0);
}
void SetPicIndex(uint16_t line, uint32_t byteAddress, uint16_t protoAddress)
{
    uint32_t indexAddr = INDEX_START_BYTES + line*3;
    SpiWrite(WRITE, indexAddr++, ((byteAddress << 7) & 0x80) | (protoAddress & 0xf), 0);
    SpiWrite(WRITE, indexAddr++, (byteAddress >> 1), 0);
    SpiWrite(WRITE, indexAddr, (byteAddress >> 9), 0);
}
// Draws a filled rectangle, with the specified color, from coordinates (x1,y1) to (x2,y2)
void FilledRectangle (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    uint16_t width;
    uint32_t address;

    if (y1 >= PICY) return; // If starting y-coordinate is past the picture borders, nothing is done
    if (y2 >= PICY) y2=PICY-1; // The rectangle stops at the bottom border of the image

    width = (x2-x1)+1;
    if (width>400) width=400;

    // Loops through one horizontal line at a time.
    while (y1 <= y2) {
        address = PICLINE_BYTE_ADDRESS(y1) + x1;
        CS_PIN_WRITE(false);
        SpiSendByte(WRITE);
        SpiSendByte(address >> 16);
        SpiSendWord(address);

        // Color values of each x coordinate of the horizontal line
        for (int i=0; i<width; i++) {
          SpiSendByte(color);
        }
        CS_PIN_WRITE(true);
        y1++;
    }
}
void VS23Init()
{
    SpiWrite(WRITE_MULTIIC, 0, 0xe, 0);
    // Set SPI memory address to sequential (autoincrementing) operation.
    SpiWrite(WRITE_STATUS, 0, 0x40, 0);
    // set GPIO output, high
    SpiWrite(GPIOCTL, 0, 0xFF, 0);
    // Write picture start and end values. These are the 
    // left and right limits of the visible picture.
    SpiWrite(PICSTART, 0, (STARTPIX-1), 1);
    SpiWrite(PICEND, 0, (ENDPIX-1), 1);
                
    // Enable and select PLL clock.
    // SpiWrite(VDCTRL1, 0, (VDCTRL1_PLL_ENABLE) | (VDCTRL1_SELECT_PLL_CLOCK), 1);
    SpiWrite(VDCTRL1, 0, VDCTRL1_PLL_ENABLE, 1);

    // Clear memory by filling it with 0. Memory is 65536 16-bit words, and first 24-bits
    // are used for the starting address. The address then autoincrements when the zero 
    // data is being sent.
    // this is slow, Can not clear.
    // CS_PIN_WRITE(false);
    // SpiSendByte(WRITE); // Send opcode
    // for (int i=0; i<65539; i++) SpiSendWord(0); // Address and data.
    // CS_PIN_WRITE(true);

    // Set length of one complete line (in PLL (VClk) clocks). 
    // Does not include the fixed 10 cycles of sync level at the beginning 
    // of the lines. 
    SpiWrite(LINELEN, 0, PLLCLKS_PER_LINE, 1);
    
    // Set microcode program for picture lines. Each OP is one VClk cycle.
    SpiWrite(PROGRAM, 0, ((OP4 << 24) | (OP3 << 16) | (OP2 << 8) | (OP1)), 0);

    // Define where Line Indexes are stored in memory
    SpiWrite(INDEXSTART, 0, INDEX_START_LONGWORDS, 1);

    // Set all line indexes to point to protoline 0 (which by definition
    // is in the beginning of the SRAM)
    for (int i=0; i<TOTAL_LINES; i++) SetLineIndex(i, PROTOLINE_WORD_ADDRESS(0));

    // Construct protoline 0
    protoline(0, 0, COLORCLKS_PER_LINE, BLANK_LEVEL);
    protoline(0, BLANKEND, FRPORCH, BLACK_LEVEL); // Set the color level to black
    protoline(0, 0, SYNC, SYNC_LEVEL); // Set HSYNC
    protoline(0, BURST, BURSTDUR, BURST_LEVEL); // Set color burst
    
    // Construct protoline 1. This is a short+short VSYNC line
    protoline(1, 0, COLORCLKS_PER_LINE, BLANK_LEVEL);
    protoline(1, 0, SHORTSYNC, SYNC_LEVEL); // Short sync at the beginning of line
    protoline(1, COLORCLKS_LINE_HALF, SHORTSYNCM, SYNC_LEVEL); // Short sync at the middle of line
    
    // Construct protoline 2. This is a long+long VSYNC line
    protoline(2, 0, COLORCLKS_PER_LINE, BLANK_LEVEL);    
    protoline(2, 0, LONGSYNC, SYNC_LEVEL);    // Long sync at the beginning of line
    protoline(2, COLORCLKS_LINE_HALF, LONGSYNCM, SYNC_LEVEL); // Long sync at the middle of line

    // Now set first lines of frame to point to prototype lines
    for (int i=1; i<4; i++) SetLineIndex(i, PROTOLINE_WORD_ADDRESS(1)); // Lines 1 to 3
    for (int i=4; i<7; i++) SetLineIndex(i, PROTOLINE_WORD_ADDRESS(2)); // Lines 4 to 6
    for (int i=7; i<10; i++) SetLineIndex(i, PROTOLINE_WORD_ADDRESS(1)); // Lines 7 to 9

    // Set pic line indexes to point to protoline 0 and their individual picture line.
    // for (int i=0; i<ENDLINE-STARTLINE; i++) SetPicIndex(i + STARTLINE, PICLINE_BYTE_ADDRESS(i),0);
    for (int i=0; i<(ENDLINE-STARTLINE)*2; i++) SetPicIndex(i + STARTLINE, PICLINE_BYTE_ADDRESS(i/2),0);
    // Enable Video Display Controller, set video mode to NTSC, set program length and linecount.
    SpiWrite(VDCTRL2, 0, 
        VDCTRL2_ENABLE_VIDEO | 
        VDCTRL2_NTSC |
        VDCTRL2_PROGRAM_LENGTH |
        VDCTRL2_LINECOUNT, 1);
}
static mp_obj_t py_tv_deinit()
{
    switch (type) {
        case TV_NONE:
            return mp_const_none;
        case TV_SHIELD:
            HAL_GPIO_DeInit(CS_PORT, CS_PIN);
            pyb_spi_deinit(spi_port);
            spi_port = NULL;
            type = TV_NONE;
            return mp_const_none;
    }
    return mp_const_none;
}
static mp_obj_t py_tv_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    py_tv_deinit();
    switch (py_helper_keyword_int(n_args, args, 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_type), TV_SHIELD)) {
        case TV_NONE:
            return mp_const_none;
        case TV_SHIELD:
        {
            GPIO_InitTypeDef GPIO_InitStructure;
            GPIO_InitStructure.Pull  = GPIO_NOPULL;
            GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
            GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStructure.Pin = CS_PIN;
            CS_PIN_WRITE(true); // Set first to prevent glitches.
            HAL_GPIO_Init(CS_PORT, &GPIO_InitStructure);

            spi_port = pyb_spi_make_new(NULL,
                2, // n_args
                3, // n_kw
                (mp_obj_t []) {
                    MP_OBJ_NEW_SMALL_INT(2), // SPI Port
                    MP_OBJ_NEW_SMALL_INT(SPI_MODE_MASTER),
                    MP_OBJ_NEW_QSTR(MP_QSTR_baudrate),
                    MP_OBJ_NEW_SMALL_INT(15000000), // todo: 35MHZ
                    MP_OBJ_NEW_QSTR(MP_QSTR_polarity),
                    MP_OBJ_NEW_SMALL_INT(0),
                    MP_OBJ_NEW_QSTR(MP_QSTR_phase),
                    MP_OBJ_NEW_SMALL_INT(0)
                }
            );
            type = TV_SHIELD;
            VS23Init();
            return mp_const_none;
        }
    }
    return mp_const_none;
}
static mp_obj_t py_tv_type()
{
    if (type == TV_NONE) return mp_const_none;
    return mp_obj_new_int(type);
}
static mp_obj_t py_tv_channel(mp_obj_t c)
{
    uint8_t channel = mp_obj_get_int(c);
    if (channel > 8 || channel < 1)
    {
        PY_ASSERT_TRUE_MSG(false, "channel should range 1~8");
    }
    uint8_t data = (channel-1) | 0xF0;
    SpiWrite(GPIOCTL, 0, data, 0);
    return mp_const_none;
}
static mp_obj_t py_tv_display(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t rect;
    py_helper_keyword_rectangle_roi(arg_img, n_args, args, 1, kw_args, &rect);

    const uint16_t x1 = rect.x;
    const uint16_t y1 = rect.y;
    const uint16_t w = rect.w < XPIXELS? rect.w : XPIXELS;
    const uint16_t h = rect.h < YPIXELS? rect.h : YPIXELS;
    const uint16_t y2 = y1 + h;

    uint32_t address;
    uint16_t x = x1;
    uint16_t y = y1;

    fb_alloc_mark();
    uint8_t *line = fb_alloc(w*2, FB_ALLOC_NO_HINT);

    while (y < y2) {
        address = PICLINE_BYTE_ADDRESS(y) + x1;
        CS_PIN_WRITE(false);
        SpiSendByte(WRITE);
        SpiSendByte(address >> 16);
        SpiSendWord(address);
        for(int i = 0; i < w; i++)
        {
            x = x1 + i;
            if (IM_IS_GS(arg_img)) {
                line[2*i] = 0;
                line[2*i + 1] = IM_GET_GS_PIXEL(arg_img, x, y);;
            } else {
                // b=>u
                // a=>v
                // y=>luminance
                uint16_t pixel = IM_GET_RGB565_PIXEL(arg_img, x, y);
                uint8_t b4 = (COLOR_RGB565_TO_U(pixel)) & 0xF0;
                uint8_t a4 = ((-COLOR_RGB565_TO_V(pixel))>>4) & 0x0F;
                uint8_t y8 = ((COLOR_RGB565_TO_Y(pixel)+128));
                line[2*i] = b4 | a4;
                line[2*i + 1] = y8;
            }
        }
        SpiSendLine(line, w*2);
        CS_PIN_WRITE(true);
        y++;
    }
    fb_alloc_free_till_mark();
    return mp_const_none;
}
static mp_obj_t py_tv_palettes()
{
    for (int i=0; i<16; i++) for (int j=0; j<16; j++) {
        FilledRectangle((i*20), (j*10), (i*20)+19, (j*10)+9, (j*16)+i); // Draw colored rectangles
        FilledRectangle((i*20), (j*10)+9, (i*20)+19, (j*10)+9, 0); // Draw black horizontal line
        FilledRectangle((i*20)+19, (j*10), (i*20)+19, (j*10)+9, 0); // Draw black vertical line
    }
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tv_init_obj, 0, py_tv_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tv_deinit_obj, py_tv_deinit);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tv_channel_obj, py_tv_channel);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tv_type_obj, py_tv_type);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tv_display_obj, 1, py_tv_display);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tv_palettes_obj, py_tv_palettes);
static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_tv) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_init),            (mp_obj_t)&py_tv_init_obj          },
    { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),          (mp_obj_t)&py_tv_deinit_obj        },
    { MP_OBJ_NEW_QSTR(MP_QSTR_channel),          (mp_obj_t)&py_tv_channel_obj        },
    { MP_OBJ_NEW_QSTR(MP_QSTR_type),            (mp_obj_t)&py_tv_type_obj          },
    { MP_OBJ_NEW_QSTR(MP_QSTR_display),         (mp_obj_t)&py_tv_display_obj       },
    { MP_OBJ_NEW_QSTR(MP_QSTR_palettes),            (mp_obj_t)&py_tv_palettes_obj         },
    { NULL, NULL },
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t tv_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t)&globals_dict,
};

void py_tv_init0()
{
    py_tv_deinit();
}
