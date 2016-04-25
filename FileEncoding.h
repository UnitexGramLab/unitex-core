/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 *
 */

#ifndef FileEncodingH
#define FileEncodingH

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * We define here the I/O encodings that are supported by the Unicode library.
 */
typedef enum {
   UTF16_LE,
   /* We give such a name to UTF16_BE in order to avoid encoding errors due to completion */
   BIG_ENDIAN_UTF16,
   UTF8,
   ASCII,
   /* utf-16-platform option to create a file in native platform ordering byte */
   PLATFORM_DEPENDENT_UTF16,
   BINARY=ASCII
} Encoding;


/* now the mask of compatibility for opening file
 */
#define USE_ENCODING_VALUE 0
#define UTF16_LE_BOM_POSSIBLE 0x0001
#define BIG_ENDIAN_UTF16_BOM_POSSIBLE 0x0002
#define UTF8_BOM_POSSIBLE 0x0004
/* there 4 value are exclusive, because we have no way to determinate which use */
#define UTF16_LE_NO_BOM_POSSIBLE 0x0010
#define BIG_ENDIAN_UTF16_NO_BOM_POSSIBLE 0x0020
#define UTF8_NO_BOM_POSSIBLE 0x0040
#define ASCII_NO_BOM_POSSIBLE 0x0080

#define ALL_ENCODING_BOM_POSSIBLE 0x0007




#define DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT (UTF16_LE_BOM_POSSIBLE | BIG_ENDIAN_UTF16_BOM_POSSIBLE | UTF8_BOM_POSSIBLE | UTF8_NO_BOM_POSSIBLE)
#define DEFAULT_ENCODING_OUTPUT (UTF16_LE)
#define DEFAULT_BOM_OUTPUT (2)

#define VersatileEncodingConfigDefined 1

#define VEC_DEFAULT {DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT,DEFAULT_ENCODING_OUTPUT,DEFAULT_BOM_OUTPUT}

typedef struct {
    /* Describes the encodings that are supported in input files */
    int mask_encoding_compatibility_input;
    /* Encoding output for file creation */
    Encoding encoding_output;
    /* Do we need to write BOM when creating a file ?
     * 0=no 1=yes 2=yes for UTF16LE and UTF16BE */
    int bom_output;
} VersatileEncodingConfig;

} // namespace unitex

#endif
