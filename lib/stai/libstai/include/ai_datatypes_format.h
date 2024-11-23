/**
  ******************************************************************************
  * @file    ai_datatypes_format.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   Definitions of AI platform private format handling routines
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef AI_DATATYPES_FORMAT_H
#define AI_DATATYPES_FORMAT_H

#include "ai_platform.h"
#include "ai_datatypes_defines.h"


/*!
 * @defgroup ai_datatypes_format Definiton and Macro of array and buffer formats
 * @brief Type definition and implementation of internal @ref ai_array and
 * @ref ai_buffer formats.
 * @details The library handles 2 different kind of formats: an internal format
 * that is part of the @ref ai_array struct that is a packed 32bit representation
 * of the format attributes, and a public format (used in public APIs) associated
 * with @ref ai_buffer struct , defined as enum in @ref ai_platform.h,
 * that is just an enum type. Converters are provided in this header file to
 * convert from one format representation to another.
 * Some MSB bits are reserved in both formats to code some bit flag useful to
 * declare some special attribute. Three flags are actually implemented in both
 * formats: the @ref AI_BUFFER_FMT_FLAG_CONST and @ref AI_FMT_FLAG_CONST used
 * to tag read-only memory buffers, @ref AI_BUFFER_FMT_FLAG_STATIC and
 * @ref AI_FMT_FLAG_STATIC to mark statically allocated memory buffers and
 * @ref AI_FMT_FLAG_SCRATCH_BUFFER to tag temporary scratch buffers.
 * All the formats are declared in a proper tuple organize table header named
 * @ref format_lists.h that enumerates all the formats available for the library.
 * A new format could be added easily by adding a new FMY_ENTRY() as required.
 * The preprocessor automatically generates the code for the handling of the
 * format according to this tuples entry. A rational for the methodology could
 * be found here:
 *   - https://codecraft.co/2012/10/29/how-enums-spread-disease-and-how-to-cure-it/
 *
 * The 32bits internal format fields are organized as follows:
 *
 * MSB                                                                       LSB
 * 31             25       24       23      21      17        14       7       0
 * /---------------------------------------------------------------------------/
 * / ATTR. FLAGS   |COMPLEX |  SIGN  |  LDIV |  TYPE |  PMASK  |  BITS | FBITS /
 * /---------------------------------------------------------------------------/
 * Where:
 * - FLAGS: is the reserved bits to store additional format attributes (e.g.
 *   I/O / STATIC flags. etc.)
 * - COMPLEX: 1 bit mark the format as complex type
 * - SIGN : 1 bit mark the format as signed type
 * - LDIV : 2 bits is a log2 value that is used to compute elements size
 *      with some special format such as the compressed ones. It is a shift
 *      factor usually set to zero
 * - TYPE : 4 bits mark the format "family" type. Actually 5 families are coded,
 *      @ref AI_FMT_FLOAT (float types)
 *      @ref AI_FMT_Q (fixed-point types in Qm.n format)
 *      @ref AI_FMT_BOOL (boolean type)
 *      @ref AI_FMT_LUT_FLOAT (compressed float lookup formats)
 *      @ref AI_FMT_LUT_Q (compressed Qmn lookup formats)
 * - PMASK 3 bits padding mask used to set the optional dimension for padding
 *      to handle special aligned formats/ E.g. a 1 bit format
 *      Usually this is set to 0x0
 * - BITS 7 bits set the total number of bits of the element, padding bits
 *      excluded. The bits are thus = sign bit + fractional bits + integer bits
 *      The number of integer bits could thus be known using the @ref
 *      AI_FMT_GET_IBITS() macro.
 * - FBITS 7 bits set the number of fractional bits in the format
 *
 *
 * A reference code snippet for usage is the test unit that uses this header:
 *
 * \include test/test_lcut_formats.cpp
 *
 */

/*!
 * Format bitfields definition.  NOTE: 7 MSB are masked off
 * for (optional) atributes setting using flags. see @ref AI_FMT_FLAG_CONST that
 * is used for marking a data as constant readonly
 */

/* 1 bit field to identify floating point values*/
#define _FMT_COMPLEX_MASK       (0x1)
#define _FMT_COMPLEX_BITS       (24)

/*! 1 bit sign info */
#define _FMT_SIGN_MASK        (0x1)
#define _FMT_SIGN_BITS        (23)

/*! fractional bits field (i.e. for Q formats see @ref AI_FMT_Q) */
#define _FMT_FBITS_MASK       (0x7F)
#define _FMT_FBITS_BITS       (0)
#define _FMT_FBITS_BIAS       ((_FMT_FBITS_MASK+1) >> 1)

/*! TOTAL number of bits (fractional+integer+sign) (excluded padding ones) */
#define _FMT_BITS_MASK        (0x7F)
#define _FMT_BITS_BITS        (7)
#define _FMT_BITS_BIAS        (0)

/*! Padding bits for handling formats not aligned to multiples of 8 bits */
#define _FMT_PMASK_MASK       (0x7)
#define _FMT_PMASK_BITS       (14)

/*! bits reserved for identifying the family format, e.g. float, fixed-point..*/
#define _FMT_TYPE_MASK        (0xF)
#define _FMT_TYPE_BITS        (17)

#define _FMT_LDIV_MASK        (0x3)
#define _FMT_LDIV_BITS        (21)


/******************************************************************************/
#define AI_FMT_OBJ(fmt_)      ((ai_array_format)(fmt_))

/*!
 * Only 25 LSB bits are used for storing actual format bits. 7 bits are reserved
 * for format attributes, see @ref AI_FMT_FLAG_CONST flag
 */
#define AI_FMT_FLAG_BITS              (25)
#define AI_FMT_MASK                   ((0x1<<AI_FMT_FLAG_BITS)-1)

#define AI_FMT_FLAG_CONST             (0x1<<30)
#define AI_FMT_FLAG_STATIC            (0x1<<29)
#define AI_FMT_FLAG_SCRATCH_BUFFER    (0x1<<28)
#define AI_FMT_FLAG_IS_IO             (0x1<<27)
#define AI_FMT_FLAG_VISITED           (0x1<<26)

/******************************************************************************/
/*!
 * Format "Class" type : this identify the family of the format:
 * float, integer, fixed point (i.e. Q format), compressed via lookup table
 */
#define AI_FMT_NONE                   (0x0)
#define AI_FMT_FLOAT                  (0x1)
#define AI_FMT_Q                      (0x2)
#define AI_FMT_BOOL                   (0x3)
#define AI_FMT_LUT_Q                  (0x4)
#define AI_FMT_LUT_FLOAT              (0x8)

#define AI_FMT_QMASK \
  ( (_FMT_FBITS_MASK<<_FMT_FBITS_BITS) | \
    (_FMT_BITS_MASK<<_FMT_BITS_BITS) | \
    (_FMT_PMASK_MASK<<_FMT_PMASK_BITS) )

#define AI_FMT_BINARY_MASK \
  (AI_FMT_MASK & (~(_FMT_SIGN_MASK<<_FMT_SIGN_BITS)))

#define AI_FMT_IS_BINARY(val_) \
  (((val_) & AI_FMT_BINARY_MASK) == AI_ARRAY_FORMAT_U1)

#define AI_FMT_GET(val_) \
  ( (AI_FMT_OBJ(val_)) & AI_FMT_MASK )

#define AI_FMT_MASK_Q(val_) \
  ( AI_FMT_OBJ(val_) & (~(AI_FMT_QMASK)) )

#define AI_FMT_GET_Q(val_) \
  ( AI_FMT_MASK_Q(val_) | AI_FMT_SET_BITS(0) | AI_FMT_SET_FBITS(0) )

#define AI_FMT_GET_FLAGS(val_) \
  ( ((AI_FMT_OBJ(val_)) & (~AI_FMT_MASK)) >> AI_FMT_FLAG_BITS )

#define AI_FMT_SAME(fmt1_, fmt2_) \
  ( AI_FMT_GET(fmt1_) == AI_FMT_GET(fmt2_) )

#define _FMT_SET(val, mask, bits)   AI_FMT_OBJ(((val)&(mask))<<(bits))
#define _FMT_GET(fmt, mask, bits)   ((AI_FMT_OBJ(fmt)>>(bits))&(mask))

#define AI_FMT_SET_COMPLEX(val)  _FMT_SET(val, _FMT_COMPLEX_MASK, _FMT_COMPLEX_BITS)
#define AI_FMT_GET_COMPLEX(fmt)  _FMT_GET(fmt, _FMT_COMPLEX_MASK, _FMT_COMPLEX_BITS)
#define AI_FMT_SET_SIGN(val)     _FMT_SET(val, _FMT_SIGN_MASK, _FMT_SIGN_BITS)
#define AI_FMT_GET_SIGN(fmt)     _FMT_GET(fmt, _FMT_SIGN_MASK, _FMT_SIGN_BITS)
#define AI_FMT_SET_PMASK(val)    _FMT_SET(val, _FMT_PMASK_MASK, _FMT_PMASK_BITS)
#define AI_FMT_GET_PMASK(fmt)    _FMT_GET(fmt, _FMT_PMASK_MASK, _FMT_PMASK_BITS)
#define AI_FMT_SET_TYPE(val)     _FMT_SET(val, _FMT_TYPE_MASK, _FMT_TYPE_BITS)
#define AI_FMT_GET_TYPE(fmt)     _FMT_GET(fmt, _FMT_TYPE_MASK, _FMT_TYPE_BITS)
#define AI_FMT_SET_LDIV(val)     _FMT_SET(val, _FMT_LDIV_MASK, _FMT_LDIV_BITS)
#define AI_FMT_GET_LDIV(fmt)     _FMT_GET(fmt, _FMT_LDIV_MASK, _FMT_LDIV_BITS)


#define AI_FMT_SET_BITS(val) \
  _FMT_SET((val) + _FMT_BITS_BIAS, _FMT_BITS_MASK, _FMT_BITS_BITS)
#define AI_FMT_GET_BITS(fmt) \
  ((ai_i8)_FMT_GET(fmt, _FMT_BITS_MASK, _FMT_BITS_BITS) - _FMT_BITS_BIAS)
#define AI_FMT_SET_FBITS(val) \
  _FMT_SET((val) + _FMT_FBITS_BIAS, _FMT_FBITS_MASK, _FMT_FBITS_BITS)
#define AI_FMT_GET_FBITS(fmt) \
  ((ai_i8)_FMT_GET(fmt, _FMT_FBITS_MASK, _FMT_FBITS_BITS) - _FMT_FBITS_BIAS)

/*!
 * The total number of bits for a given format is supposed to be the sum of the
 * bits + padding bits. This means that the number of integer bits is derived
 * as follow: int_bits = bits - fbits (fractional bits) - 1 (for the sign)
 */
#define AI_FMT_GET_BITS_SIZE(fmt_) \
  AI_FMT_GET_BITS(fmt_)

/*! Macro used to compute the integer bits for a format */
#define AI_FMT_GET_IBITS(fmt_) \
  ((ai_i16)AI_FMT_GET_BITS(fmt_)-AI_FMT_GET_FBITS(fmt_)-AI_FMT_GET_SIGN(fmt_))

/*! ai_buffer format handlers section *****************************************/

#define AI_BUFFER_FMT_MASK_Q(fmt_) \
  ( AI_BUFFER_FMT_OBJ(fmt_) & 0xFFFFC000 )

#define AI_BUFFER_FMT_GET_Q(fmt_) \
  ( AI_BUFFER_FMT_MASK_Q(fmt_) | AI_BUFFER_FMT_SET_FBITS(0) | \
    AI_BUFFER_FMT_SET_FBITS(0) )

#define AI_BUFFER_FMT_SET_Q(bits_, fbits_) \
  AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0,  bits_, fbits_)

#define AI_BUFFER_FMT_IS_Q(fmt_) \
  ( (AI_BUFFER_FMT_TYPE_Q==AI_BUFFER_FMT_GET_TYPE(fmt_)) && \
    (1==AI_BUFFER_FMT_GET_SIGN(fmt_)) )

#define AI_BUFFER_FMT_SET_UQ(bits_, fbits_) \
  AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0,  bits_, fbits_)

#define AI_BUFFER_FMT_IS_UQ(fmt_) \
  ( (AI_BUFFER_FMT_TYPE_Q==AI_BUFFER_FMT_GET_TYPE(fmt_)) && \
    (0==AI_BUFFER_FMT_GET_SIGN(fmt_)) )

/*! Q ai_array format handlers ************************************************/
#define AI_ARRAY_FMT_Q(bits_, fbits_) \
  ( AI_FMT_MASK_Q(AI_ARRAY_FORMAT_Q) | AI_FMT_SET_BITS(bits_) | AI_FMT_SET_FBITS(fbits_) )

#define AI_ARRAY_FMT_SET_Q(bits_, fbits_) \
  AI_ARRAY_FMT_Q(bits_, fbits_)

#define AI_ARRAY_FMT_IS_Q(fmt_) \
  ( AI_FMT_GET(AI_FMT_MASK_Q(AI_ARRAY_FORMAT_Q))==AI_FMT_GET(AI_FMT_MASK_Q(fmt_)) )

#define AI_ARRAY_FMT_UQ(bits_, fbits_) \
  ( AI_FMT_MASK_Q(AI_ARRAY_FORMAT_UQ) | AI_FMT_SET_BITS(bits_) | AI_FMT_SET_FBITS(fbits_) )

#define AI_ARRAY_FMT_SET_UQ(bits_, fbits_) \
  AI_ARRAY_FMT_UQ(bits_, fbits_)

#define AI_ARRAY_FMT_IS_UQ(fmt_) \
  ( AI_FMT_GET(AI_FMT_MASK_Q(AI_ARRAY_FORMAT_UQ))==AI_FMT_GET(AI_FMT_MASK_Q(fmt_)) )

AI_DEPRECATED
/* Alias for AI_ARRAY_FMT_SET_Q */
#define AI_ARRAY_FMT_SET_SQ(bits_, fbits_) \
  AI_ARRAY_FMT_SET_Q(bits_, fbits_)

AI_DEPRECATED
/* Alias for AI_ARRAY_FMT_IS_Q */
#define AI_ARRAY_FMT_IS_SQ(fmt_) \
  AI_ARRAY_FMT_IS_Q(fmt_)

/*! ai_array section **********************************************************/
#define AI_ARRAY_FMT_ENTRY(name_) \
  AI_CONCAT(AI_ARRAY_FORMAT_, name_)

#define AI_ARRAY_FMT_NAME(fmt_) \
  ai_array_fmt_name(fmt_)

#define AI_ARRAY_FMT_VALID(fmt_) \
  ai_array_fmt_valid(fmt_)

#define AI_ARRAY_FMT_EXPORTED(fmt_) \
  ai_array_fmt_exported(fmt_)

#define AI_ARRAY_FMT_GET_FORMATS(formats_) \
  ai_array_fmt_get_formats(formats_)

#define AI_ARRAY_TO_BUFFER_FMT(fmt_) \
  ai_array_to_buffer_fmt(fmt_)

#define AI_ARRAY_GET_BYTE_SIZE(fmt_, count_) \
  ai_array_get_byte_size(fmt_, count_)

#define AI_ARRAY_GET_DATA_BYTE_SIZE(fmt_, count_) \
  ai_array_get_data_byte_size(fmt_, count_)

#define AI_ARRAY_GET_ELEMS_FROM_SIZE(fmt_, size_) \
  ai_array_get_elems_from_size(fmt_, size_)


/* Compile sanity checks for formats field consistency */
#if (AI_FMT_MASK != AI_BUFFER_FMT_MASK)
#error "AI_FMT_MASK != AI_BUFFER_FMT_MASK"
#endif
#if (AI_FMT_NONE != AI_BUFFER_FMT_TYPE_NONE)
#error "AI_FMT_NONE != AI_BUFFER_FMT_TYPE_NONE"
#endif
#if (AI_FMT_FLOAT != AI_BUFFER_FMT_TYPE_FLOAT)
#error "AI_FMT_FLOAT != AI_BUFFER_FMT_TYPE_FLOAT"
#endif
#if (AI_FMT_Q != AI_BUFFER_FMT_TYPE_Q)
#error "AI_FMT_Q != AI_BUFFER_FMT_TYPE_Q"
#endif
#if (AI_FMT_BOOL != AI_BUFFER_FMT_TYPE_BOOL)
#error "AI_FMT_BOOL != AI_BUFFER_FMT_TYPE_BOOL"
#endif
#if (AI_FMT_FLAG_CONST != AI_BUFFER_FMT_FLAG_CONST)
#error "AI_FMT_FLAG_CONST != AI_BUFFER_FMT_FLAG_CONST"
#endif
#if (AI_FMT_FLAG_STATIC != AI_BUFFER_FMT_FLAG_STATIC)
#error "AI_FMT_FLAG_STATIC != AI_BUFFER_FMT_FLAG_STATIC"
#endif
#if (AI_FMT_FLAG_IS_IO != AI_BUFFER_FMT_FLAG_IS_IO)
#error "AI_FMT_FLAG_IS_IO != AI_BUFFER_FMT_FLAG_IS_IO"
#endif
#if (AI_FMT_FLAG_STATIC != AI_BUFFER_FMT_FLAG_PERSISTENT)
#error "AI_FMT_FLAG_STATIC != AI_BUFFER_FMT_FLAG_PERSISTENT"
#endif


AI_API_DECLARE_BEGIN

/*!
 * @typedef ai_array_format
 * @ingroup ai_datatypes_format
 * @brief Generic Data Format Specifier for @ref ai_array (32bits packed info)
 */
typedef int32_t ai_array_format;

/*!
 * @enum internal data format enums
 * @ingroup ai_datatypes_format
 * @brief Generic Data Format Specifier (32bits packed info)
 */
typedef enum {
#define FMT_ENTRY(exp_, name_, type_id_, sign_bit_, complex_bit_, \
  pmask_, bits_, fbits_, ldiv_bits_) \
    AI_ARRAY_FMT_ENTRY(name_) = (AI_FMT_SET_COMPLEX(complex_bit_) | \
                                 AI_FMT_SET_SIGN(sign_bit_) | \
                                 AI_FMT_SET_BITS(bits_) | \
                                 AI_FMT_SET_FBITS(fbits_) | \
                                 AI_FMT_SET_PMASK(pmask_) | \
                                 AI_FMT_SET_TYPE(type_id_) | \
                                 AI_FMT_SET_LDIV(ldiv_bits_)),
#include "formats_list.h"
} ai_array_format_entry;

/*!
 * @brief Get a human readable string from the format ID value
 * @ingroup ai_datatypes_format
 * @param[in] type the @ref ai_array_format to print out
 * @return a string with a human readable name of the format
 */
AI_INTERNAL_API
const char* ai_array_fmt_name(const ai_array_format type);

/*!
 * @brief Check if @ref ai_array_format is a exportable to an @ref ai_buffer_format
 * @ingroup ai_datatypes_format
 * @param[in] type the ai_array_format to check
 * @return true if the format is exported, false otherwise
 */
AI_INTERNAL_API
ai_bool ai_array_fmt_exported(const ai_array_format type);

/*!
 * @brief Check if @ref ai_array_format is a valid format present in the list of
 * supported formats
 * @ingroup ai_datatypes_format
 * @param[in] type the ai_array_format to check
 * @return true if the format is valid, false otherwise
 */
AI_INTERNAL_API
ai_bool ai_array_fmt_valid(const ai_array_format type);

/*!
 * @brief Get the complete list of supported @ref ai_array_format formats
 * @ingroup ai_datatypes_format
 * @param[out] formats a pointer to an array withj all supported formats listed
 * @return the number of supported formats
 */
AI_INTERNAL_API
ai_size ai_array_fmt_get_formats(const ai_array_format** formats);

/*! ai_buffer section *********************************************************
 * Only 25 LSB bits are used for storing actual format bits. 7 bits are reserved
 * for format atrtributes, see @ref AI_FMT_FLAG_CONST flag
 */

#define AI_BUFFER_FMT_ENTRY(name_) \
  AI_CONCAT(AI_BUFFER_FORMAT_, name_)

#define AI_BUFFER_FMT_NAME(type_) \
  ai_buffer_fmt_name(type_)

#define AI_BUFFER_FMT_VALID(type_) \
  ai_buffer_fmt_valid(type_)

#define AI_BUFFER_FMT_GET_FORMATS(formats_) \
  ai_buffer_fmt_get_formats(formats_)

#define AI_BUFFER_TO_ARRAY_FMT(fmt_) \
  ai_buffer_to_array_fmt(fmt_)

#define AI_BUFFER_GET_BITS_SIZE(fmt) \
  AI_ARRAY_GET_BITS_SIZE(AI_BUFFER_TO_ARRAY_FMT(fmt))


/*!
 * @brief Get a human readable string from the format ID value
 * @ingroup ai_datatypes_format
 * @param[in] type the @ref ai_buffer_format to print out
 * @return a string with a human readable name of the format
 */
AI_INTERNAL_API
const char* ai_buffer_fmt_name(
  const ai_buffer_format type);

/*!
 * @brief Check if @ref ai_buffer_format is a valid format present in the list
 * of supported formats
 * @ingroup ai_datatypes_format
 * @param[in] type the @ref ai_buffer_format to check
 * @return true if the format is valid, false otherwise
 */
AI_INTERNAL_API
ai_bool ai_buffer_fmt_valid(
  const ai_buffer_format type);

/*!
 * @brief Get the complete list of supported @ref ai_buffer_format formats
 * @ingroup ai_datatypes_format
 * @param[out] formats a pointer to an array with all supported formats listed
 * @return the number of supported formats
 */
AI_INTERNAL_API
ai_size ai_buffer_fmt_get_formats(
  const ai_buffer_format** formats);

/*! Conversions section *******************************************************/
/*!
 * @brief Convert from ai_array_format to ai_buffer_format.
 * @ingroup ai_datatypes_format
 * @param fmt the input ai_array_format to convert
 * @return the converted format as a ai_buffer_format
 */
AI_INTERNAL_API
ai_buffer_format ai_array_to_buffer_fmt(
  const ai_array_format fmt);

/*!
 * @brief Convert from ai_buffer_format to ai_array_format.
 * @ingroup ai_datatypes_format
 * @param fmt the input ai_buffer_format to convert
 * @return the converted format as a ai_array_format
 */
AI_INTERNAL_API
ai_array_format ai_buffer_to_array_fmt(
  const ai_buffer_format fmt);

/** helpers section ***********************************************************/
/*!
 * @brief Computes the size in bytes given an ai_array_format and number of
 * array elements.
 * @details This routine computes from the number of elements of the array its
 * size in bytes. If the array is referred by a tensor structure, it is the task
 * of the latter to handle per-dimension padding (e.g. to align odd rows in a
 * 4-bit matrix. At array level the padding elements MUST be included in the
 * number of elements.
 * @ingroup ai_datatypes_format
 * @param[in] fmt the input array format as an ai_array_format
 * @param[in] count the number of elements stored in the data array
 * @return the size in bytes of the array given the specific format and number
 * of elements (including padding elements)
 */
AI_INTERNAL_API
ai_size ai_array_get_byte_size(
  const ai_array_format fmt, const ai_size count);

/*!
 * @brief Computes the size in bytes given an ai_array_format and number of
 * array elements of the data fields (e.g. LUT table size excluded).
 * @details This routine computes from the number of elements of the array its
 * size in bytes. If the array is referred by a tensor structure, it is the task
 * of the latter to handle per-dimension padding (e.g. to align odd rows in a
 * 4-bit matrix. At array level the padding elements MUST be included in the
 * number of elements.
 * @ingroup ai_datatypes_format
 * @param[in] fmt the input array format as an ai_array_format
 * @param[in] count the number of elements stored in the data array
 * @return the size in bytes of the array given the specific format and number
 * of elements (including padding elements)
 */
AI_INTERNAL_API
ai_size ai_array_get_data_byte_size(
  const ai_array_format fmt, const ai_size count);

/*!
 * @brief Computes the number of elements from ai_array_format and
 * the size in byte of the array.
 * @ingroup ai_datatypes_format
 * @param fmt the input array format as an ai_array_format
 * @param size the size in bytes of the array
 * @return the number of elements that could be stored given the format
 */
AI_INTERNAL_API
ai_size ai_array_get_elems_from_size(
  const ai_array_format fmt, const ai_size byte_size);

AI_API_DECLARE_END

#endif /*AI_DATATYPES_FORMAT_H*/
