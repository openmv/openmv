
/* FMT_ENTRY( exp_(0/1 only), name_, type_id_, 
 *            sign_bit_, float_bit_, pbits_, bits_, fbits_, ldiv_bits_)
 * Specifications (in order of the bit fields, little endian):
   - name_ : it is the enum used to define both the ai_array_format and
      ai_buffer_format.
   - exp_ (1bit) : it is a boolean flag (0 or 1) indicating whether the format
      is available as a public APIs ai_buffer format. in this case the field
      exp_name_ indicates the enum name of the ai_buffer format
   - (7 bits): reserved for flags
   - sign_bit_ (1bit) : codes whether or not the format is of a signed type
   - float_bit_ (1bit) : codes if the format is float
   - ldiv_bits (2 bits) : right shift value for computing the byte size of the
      format
   - type_id_ (4bits) : it is used to define the "family" of the format: 
      see @ref AI_FMT_Q as an example. Currently supported types are: 
      AI_FMT_Q (fixed point types), AI_FMT_FLOAT (floating point values),
      AI_FMT_LUT4 or AI_FMT_LUT8 (compressed formats)
   - pbits_ (3bits) : number of padding bits for the format
   - bits_ (7bits)  : size in bits of the format (NB: integer+fractional bits)
   - fbits_ (7bits) : number of fractional bits for the format (for AI_FMT_Q only)
   
  */

/* Macro tricks are here:
 * https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms
 */

/* Format none entry */
FMT_ENTRY(1, NONE, AI_FMT_NONE, 0, 0, 0, 0, 0, 0)

/* Floating point formats */
FMT_ENTRY(1, FLOAT,   AI_FMT_FLOAT, 1, 1, 0, 32,  0, 0)
FMT_ENTRY(0, FLOAT64, AI_FMT_FLOAT, 1, 1, 0, 64,  0, 0)
FMT_ENTRY(0, FLOAT16, AI_FMT_FLOAT, 1, 1, 0, 16,  0, 0)

/* Integer formats (i.e. fractional bits = 0!) */
FMT_ENTRY(1, U8,  AI_FMT_Q, 0, 0, 0, 8,  0, 0)
FMT_ENTRY(1, U16, AI_FMT_Q, 0, 0, 0, 16, 0, 0)
FMT_ENTRY(0, U32, AI_FMT_Q, 0, 0, 0, 32, 0, 0)
FMT_ENTRY(0, U64, AI_FMT_Q, 0, 0, 0, 64, 0, 0)
FMT_ENTRY(0, U4,  AI_FMT_Q, 0, 0, 0, 4,  0, 0)

FMT_ENTRY(1, S8,  AI_FMT_Q, 1, 0, 0, 8,  0, 0)
FMT_ENTRY(1, S16, AI_FMT_Q, 1, 0, 0, 16, 0, 0)
FMT_ENTRY(0, S32, AI_FMT_Q, 1, 0, 0, 32, 0, 0)
FMT_ENTRY(0, S64, AI_FMT_Q, 1, 0, 0, 64, 0, 0)
FMT_ENTRY(0, S4,  AI_FMT_Q, 1, 0, 0, 4,  0, 0)

/* Fixed-point formats including ARM CMSIS Q7, Q15, Q31 ones */
FMT_ENTRY(1, Q,    AI_FMT_Q, 1, 0, 0, 0,   0, 0)
FMT_ENTRY(1, Q7,   AI_FMT_Q, 1, 0, 0, 8,   7, 0)
FMT_ENTRY(1, Q15,  AI_FMT_Q, 1, 0, 0, 16, 15, 0)
FMT_ENTRY(0, Q31,  AI_FMT_Q, 1, 0, 0, 32, 31, 0)

FMT_ENTRY(1, UQ,   AI_FMT_Q, 0, 0, 0, 0,   0, 0)
FMT_ENTRY(1, UQ7,  AI_FMT_Q, 0, 0, 0, 8,   7, 0)
FMT_ENTRY(1, UQ15, AI_FMT_Q, 0, 0, 0, 16, 15, 0)
FMT_ENTRY(0, UQ31, AI_FMT_Q, 0, 0, 0, 32, 31, 0)

/* Compressed formats */
FMT_ENTRY(0, LUT4_FLOAT, AI_FMT_LUT4, 1, 1, 0, 32, 0, 3)
FMT_ENTRY(0, LUT8_FLOAT, AI_FMT_LUT8, 1, 1, 0, 32, 0, 2)
FMT_ENTRY(0, LUT4_Q15,   AI_FMT_LUT4, 1, 0, 0, 16, 15, 2)
FMT_ENTRY(0, LUT8_Q15,   AI_FMT_LUT8, 1, 0, 0, 16, 15, 1)
FMT_ENTRY(0, LUT4_UQ15,  AI_FMT_LUT4, 0, 0, 0, 16, 15, 2)
FMT_ENTRY(0, LUT8_UQ15,  AI_FMT_LUT8, 0, 0, 0, 16, 15, 1)

#undef FMT_ENTRY
