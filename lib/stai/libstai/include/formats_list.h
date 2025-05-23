/**
  ******************************************************************************
  * @file    format_list.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   Definitions of AI platform public APIs types
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* FMT_ENTRY( exp_(0/1 only), name_, type_id_,
 *            sign_bit_, complex_bit_, pmask_, bits_, fbits_, ldiv_bits_)
 * Specifications (in order of the bit fields, little endian):
   - name_ : it is the enum used to define both the ai_array_format and
      ai_buffer_format.
   - exp_ (1bit) : it is a boolean flag (0 or 1) indicating whether the format
      is available as a public APIs ai_buffer format. in this case the field
      exp_name_ indicates the enum name of the ai_buffer format
   - (7 bits): reserved for flags
   - sign_bit_ (1bit) : codes whether or not the format is of a signed type
   - complex_bit_ (1bit) : codes if the format is of a complex type
   - ldiv_bits (2 bits) : right shift value for computing the byte size of the
      format
   - type_id_ (4bits) : it is used to define the "family" of the format:
      see @ref AI_FMT_Q as an example. Currently supported types are:
      AI_FMT_Q (fixed point types), AI_FMT_FLOAT (floating point values),
      AI_FMT_LUT_FLOAT or AI_FMT_LUT_Q (compressed formats)
   - pmask_ (3bits) : padding mask bits for the format
   - bits_ (7bits)  : size in bits of the format (NB: integer+fractional bits)
   - fbits_ (7bits) : number of fractional bits for the format
     (for AI_FMT_Q only)

  */

/* Format none entry */
FMT_ENTRY(1, NONE, AI_FMT_NONE, 0, 0, 0x0, 0, 0, 0)

/* Floating point formats */
FMT_ENTRY(1, FLOAT,   AI_FMT_FLOAT, 1, 0, 0x0, 32,  0, 0)
FMT_ENTRY(0, FLOAT64, AI_FMT_FLOAT, 1, 0, 0x0, 64,  0, 0)
FMT_ENTRY(0, FLOAT16, AI_FMT_FLOAT, 1, 0, 0x0, 16,  0, 0)

/* Integer formats (i.e. fractional bits = 0!) */
FMT_ENTRY(1, U8,  AI_FMT_Q, 0, 0, 0x0, 8,  0, 0)
FMT_ENTRY(1, U16, AI_FMT_Q, 0, 0, 0x0, 16, 0, 0)
FMT_ENTRY(1, U32, AI_FMT_Q, 0, 0, 0x0, 32, 0, 0)
FMT_ENTRY(0, U64, AI_FMT_Q, 0, 0, 0x0, 64, 0, 0)
FMT_ENTRY(1, U1,  AI_FMT_Q, 0, 0, 0x0, 1,  0, 0)
FMT_ENTRY(0, U4,  AI_FMT_Q, 0, 0, 0x0, 4,  0, 0)

FMT_ENTRY(1, S8,  AI_FMT_Q, 1, 0, 0x0, 8,  0, 0)
FMT_ENTRY(1, S16, AI_FMT_Q, 1, 0, 0x0, 16, 0, 0)
FMT_ENTRY(1, S32, AI_FMT_Q, 1, 0, 0x0, 32, 0, 0)
FMT_ENTRY(0, S64, AI_FMT_Q, 1, 0, 0x0, 64, 0, 0)
FMT_ENTRY(1, S1,  AI_FMT_Q, 1, 0, 0x0, 1,  0, 0)
FMT_ENTRY(0, S4,  AI_FMT_Q, 1, 0, 0x0, 4,  0, 0)

/* Fixed-point formats including ARM CMSIS Q7, Q15, Q31 ones */
FMT_ENTRY(1, Q,    AI_FMT_Q, 1, 0, 0x0, 0,   0, 0)
FMT_ENTRY(1, Q7,   AI_FMT_Q, 1, 0, 0x0, 8,   7, 0)
FMT_ENTRY(1, Q15,  AI_FMT_Q, 1, 0, 0x0, 16, 15, 0)
FMT_ENTRY(0, Q31,  AI_FMT_Q, 1, 0, 0x0, 32, 31, 0)

FMT_ENTRY(1, UQ,   AI_FMT_Q, 0, 0, 0x0, 0,   0, 0)
FMT_ENTRY(1, UQ7,  AI_FMT_Q, 0, 0, 0x0, 8,   7, 0)
FMT_ENTRY(1, UQ15, AI_FMT_Q, 0, 0, 0x0, 16, 15, 0)
FMT_ENTRY(0, UQ31, AI_FMT_Q, 0, 0, 0x0, 32, 31, 0)

/* Compressed formats */
FMT_ENTRY(0, LUT4_FLOAT, AI_FMT_LUT_FLOAT, 1, 0, 0x0, 32, 0, 3)
FMT_ENTRY(0, LUT8_FLOAT, AI_FMT_LUT_FLOAT, 1, 0, 0x0, 32, 0, 2)
FMT_ENTRY(0, LUT4_Q15,   AI_FMT_LUT_Q, 1, 0, 0x0, 16, 15, 2)
FMT_ENTRY(0, LUT8_Q15,   AI_FMT_LUT_Q, 1, 0, 0x0, 16, 15, 1)
FMT_ENTRY(0, LUT4_UQ15,  AI_FMT_LUT_Q, 0, 0, 0x0, 16, 15, 2)
FMT_ENTRY(0, LUT8_UQ15,  AI_FMT_LUT_Q, 0, 0, 0x0, 16, 15, 1)

/* Boolean format */
FMT_ENTRY(1, BOOL, AI_FMT_BOOL, 0, 0, 0x0, 8, 0, 0)

/* Complex formats */
FMT_ENTRY(0, COMPLEX_FLOAT64, AI_FMT_FLOAT, 1, 1, 0x0, 64, 0, 0)
FMT_ENTRY(0, COMPLEX_S64, AI_FMT_Q, 1, 1, 0x0, 64, 0, 0)
FMT_ENTRY(0, COMPLEX_S32, AI_FMT_Q, 1, 1, 0x0, 32, 0, 0)
FMT_ENTRY(0, COMPLEX_S16, AI_FMT_Q, 1, 1, 0x0, 16, 0, 0)

#undef FMT_ENTRY
