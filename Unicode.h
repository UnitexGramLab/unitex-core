/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef UnicodeH
#define UnicodeH

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include "Af_stdio.h"

#include "FileEncoding.h"

#include "AbstractAllocator.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/* This line is used to prevent people from using printf and scanf. We do
 * that because we want to parametrize I/O operations with encodings. */
#define printf DONT_USE_PRINTF_BUT_U_PRINTF
#define scanf DONT_USE_SCANF_BUT_U_SCANF

#define fopen DONT_USE_FOPEN_BUT_U_FOPEN

/**
 * This library provides basic I/O unicode operations. For internal data
 * representations, UTF16-LE is used. It is also the default encoding for
 * reading and writing text files. UTF8 is the default encoding for writing
 * to the standard and error outputs.
 */


/**
 * These are the binary open modes for unicode text files.
 * We use now an enum in order to keep control on how files are opened.
 */
typedef enum {
   U_READ,   // "rb"
   U_WRITE,  // "wb"
   U_APPEND, // "ab"
   U_MODIFY  // "r+b"
} OpenMode;

/* This caracter is used as the first one of a unicode text file... */
#define U_BYTE_ORDER_MARK 0xFEFF

/* ...and this one is used by contrast with 0xFEFF to determine the byte order */
#define U_NOT_A_CHAR 0xFFFE

/* Markers to indicate if a text file is a UTF16 one or not. */
#define FILE_DOES_NOT_EXIST 0
#define NOT_A_UTF16_FILE 1
#define UTF16_LITTLE_ENDIAN_FILE 2
#define UTF16_BIG_ENDIAN_FILE 3


/**
 * This is the type of a unicode character. Note that it is a 16-bits type,
 * so that it cannot handle characters >= 0xFFFF. Such characters, theoretically
 * represented with low and high surrowgate characters are not handled by Unitex.
 */
typedef uint16_t unichar;


/**
 * This structure is used to represent a file with its encoding.
 */
typedef struct {
    ABSTRACTFILE* f;
    Encoding enc;
} U_FILE;


/**
 * We define here the unicode NULL character and the unicode
 * empty string.
 */
extern const unichar U_NULL;
extern const unichar* U_EMPTY;

extern const unichar EPSILON[4];

/* This constant must be used instead of 65536 (0x10000) in order
 * to avoid problems if one day the type unichar becomes larger
 * than 2 bytes */
#define MAX_NUMBER_OF_UNICODE_CHARS (1<<sizeof(unichar)*8)


/**
 * Here are defined the default encodings.
 */
extern Encoding STDIN_ENC8;
extern Encoding STDOUT_ENC;
extern Encoding STDERR_ENC;
extern Encoding FILE_ENC;

extern U_FILE* U_STDIN;
extern U_FILE* U_STDOUT;
extern U_FILE* U_STDERR;


/* decode encoding parameter, to prepare value for u_fopen */
int decode_reading_encoding_parameter(int*, const char*);
int decode_writing_encoding_parameter(Encoding*, int*, const char*);
int get_reading_encoding_text(char*,size_t,int);
int get_writing_encoding_text(char*,size_t,Encoding,int);

/* Some aliases for U_FILE */
int fseek(U_FILE *stream, long offset, int whence);
long ftell(U_FILE *stream);
void rewind(U_FILE *stream);
int u_feof(U_FILE* stream);
size_t fread(void *ptr,size_t size,size_t nmemb,U_FILE *stream);
size_t fwrite(const void *ptr,size_t size,size_t nmemb,U_FILE *stream);



/* ------------------- Some aliases, mainly for default UTF16-LE use ------------------- */
int u_fgetc_raw(U_FILE*);
int u_fgetc_UTF16LE(ABSTRACTFILE*);
int u_fgetc_UTF16BE(ABSTRACTFILE*);
int u_fgetc_UTF8(ABSTRACTFILE*);
int u_fgetc(U_FILE*);
int u_fgetc_CR(U_FILE*);
int u_fread_raw(unichar*,int,U_FILE*,int*);
int u_fread_raw(unichar*,int,U_FILE*);
int u_fread(unichar*,int,U_FILE*,int*);

int u_fputc_raw(unichar,U_FILE*);
int u_fputc_UTF16LE(unichar,ABSTRACTFILE*);
int u_fputc_UTF16BE(unichar,ABSTRACTFILE*);
int u_fputc_UTF8(unichar,ABSTRACTFILE*);
int u_fputc(unichar,U_FILE*);
int u_ungetc(unichar,U_FILE*);
int u_fputc_conv_lf_to_crlf_option(unichar, U_FILE*, int);

int u_fwrite_raw(const unichar*,int,U_FILE*);
int u_fwrite(const unichar*,int,U_FILE*);

void u_fputs_raw(const unichar*, U_FILE*);
void u_fputs(const unichar*, U_FILE*);
void u_fputs_conv_lf_to_crlf_option(const unichar*, U_FILE*, int);

int u_fgets(unichar*,U_FILE*);
int u_fgets(unichar*,int,U_FILE*);
int u_fgets_limit2(unichar*,int,U_FILE*);
int u_fgets2(unichar*,U_FILE*);
int u_fgets_treat_cr_as_lf(unichar* s,int size,U_FILE* f);
int u_fgets_dynamic_buffer(unichar** line, size_t* buffer_size, U_FILE* f, int treat_CR_as_LF = 0);

int u_printf(const char*,...);
int u_fprintf(U_FILE*,const char*,...);
int u_fprintf_conv_lf_to_crlf_option(U_FILE* f,int convLFtoCRLF,const char* format,...);

int u_scanf(const char*,...);
int u_fscanf(U_FILE*,const char*,...);

/* The u_prints and u_fprints functions should not be visible from the
 * outside of this library. People should use u_printf and u_fprintf */
//void u_prints(unichar*);
//void u_fprints(unichar*,U_FILE*);
//void u_fprints(char*,U_FILE*);
//void u_fprints(Encoding,unichar*,ABSTRACTFILE*);
//void u_fprints(Encoding,char*,ABSTRACTFILE*);

/* ------------------- File functions ------------------- */


/* NOTE: the u_fopen taking an Encoding should not be used unless the
 *       encoding of the file will NEVER change. It should only be used
 *       for binary files, ascii files and tmp files whose encoding does not
 *       matter since they should not be used outside the programs.
 *
 *       In any other case, prefer the second one taking a VersatileEncodingConfig*.
 *       Doing this, you make all Unitex programs encoding-customizable with options
 *       -q and -k. The default behaviour is to read any file encoded with utf16le-bom,
 *       utf16be-bom and utf8, with or without bom. The default encoding output is
 *       utf16le-bom
 */
U_FILE* u_fopen(Encoding,const char*,OpenMode);
U_FILE* u_fopen(const VersatileEncodingConfig* cfg,const char*,OpenMode);

void u_fsetsizereservation_by_bytes(U_FILE*, long size_planned);
void u_fsetsizereservation_by_chars(U_FILE*, long size_planned);

int u_fclose(U_FILE*);
int u_fempty(Encoding,int,const char*);
int u_fempty(const VersatileEncodingConfig*,const char*);
int u_is_UTF16(const char*);
int u_is_UTF16(U_FILE*);

int u_fgetc_UTF16LE_raw(ABSTRACTFILE*);
int u_fgetc_UTF16BE_raw(ABSTRACTFILE*);
int u_fgetc_UTF8_raw(ABSTRACTFILE*);
int u_fgetc_raw(U_FILE*);
int u_fgetc(U_FILE*);
int u_fgetc_CR(U_FILE*);
int u_fskip_line(U_FILE*);

int u_fread_raw(unichar*,int,U_FILE*);
int u_fread(unichar*,int,U_FILE*,int*);

int u_fputc_UTF16LE_raw(unichar,ABSTRACTFILE*);
int u_fputc_UTF16BE_raw(unichar,ABSTRACTFILE*);
int u_fputc_UTF8_raw(unichar,ABSTRACTFILE*);
int u_fputc_raw(unichar,U_FILE*);
int u_fputc(unichar,U_FILE*);

int u_ungetc_UTF16LE_raw(ABSTRACTFILE*);
int u_ungetc_UTF16BE_raw(ABSTRACTFILE*);
int u_ungetc_UTF8_raw(unichar,ABSTRACTFILE*);
int u_ungetc_raw(unichar,U_FILE*);
int u_ungetc(unichar,U_FILE*);

int u_fwrite_raw(const unichar*,int,U_FILE*);
int u_fwrite(const unichar*,int,U_FILE*);

int u_fgets(unichar*,U_FILE*);
int u_fgets(unichar*,int,U_FILE*);
int u_fgets_limit2(unichar*,int,U_FILE*);
int u_fgets2(unichar*,U_FILE*);

int u_fget_unichars_raw(Encoding encoding, unichar* buffer, int size, ABSTRACTFILE* f);
int u_fget_unichars_raw(unichar* buffer, int size, U_FILE* f);

int u_fprintf(U_FILE*,const char*,...);
int u_vfprintf(U_FILE*,const char*,va_list);
int u_vfprintf_conv_lf_to_crlf_option(U_FILE*,int,const char*,va_list);
int u_sprintf(unichar*,const char*,...);
int u_vsprintf(unichar*,const char*,va_list);

int u_fscanf(U_FILE*,const char*,...);
int u_vfscanf(U_FILE*,const char*,va_list);
int u_sscanf(unichar*,const char*,...);
int u_vsscanf(unichar*,const char*,va_list);

/* ------------------- String functions ------------------- */
unsigned int u_strlen(const unichar*);
unsigned int u_strlenWithConvLFtoCRLF(const unichar* s, int convLFtoCRLF);
unichar* u_strcpy(unichar*,const unichar*);
unichar* u_strcpy(unichar*,const char*);
unichar* u_strncpy(unichar *dest,const unichar *src,unsigned int n);
unichar* u_strcpy_sized(unichar*,size_t,const unichar*);
unichar* u_strcpy_sized(unichar*,size_t,const char*);
unichar* u_strcat(unichar*,const unichar*);
unichar* u_strcat(unichar*,const char*);

unichar* u_strcpy_optional_buffer(unichar * original_buffer, size_t original_buffer_size,
    unichar**allocated_buffer, const unichar* add_string, size_t* len = NULL, Abstract_allocator prv_alloc = NULL);
unichar* u_strcpy_optional_buffer(unichar * original_buffer, size_t original_buffer_size,
    unichar**allocated_buffer, const char* add_string, size_t* len = NULL, Abstract_allocator prv_alloc = NULL);
unichar* u_strcat_optional_buffer(unichar * original_buffer, size_t original_buffer_size,
    unichar**allocated_buffer, const unichar* add_string, size_t* len = NULL, Abstract_allocator prv_alloc = NULL);
unichar* u_strcat_optional_buffer(unichar * original_buffer, size_t original_buffer_size,
    unichar**allocated_buffer, const char* add_string, size_t* len = NULL, Abstract_allocator prv_alloc = NULL);
void free_string_optional_buffer(unichar** allocated_buffer, Abstract_allocator prv_alloc = NULL);

int is_str_mono_unichar_string(const unichar*, unichar);
#define is_str_mono_unichar_string(str,c) (((str)!=NULL) && (*(str)==(c)) && (*((str)+1)==0))
int u_strcmp(const unichar*, const unichar*);
int u_strcmp(const unichar*,const char*);
int u_strncmp(const unichar*, const unichar*,size_t num);
int u_strcmp_ignore_case(const unichar*, const unichar*);
int u_strcmp_ignore_case(const unichar*, const char*);
unichar *u_strtok_r(unichar *str, const unichar *delim, unichar **saveptr);
int u_equal(const unichar*, const unichar*);
int u_equal_ignore_case(const unichar*, const unichar*);
unichar* u_strdup(const unichar*);
unichar* u_strndup(const unichar*,int);
unichar* keycopy(unichar*);
unichar* u_strdup(const unichar*,unsigned int);
unichar* u_strdup(const char*);
const unichar* u_strchr(const unichar*,unichar,int);
const unichar* u_strchr(const unichar*,unichar);
unichar* u_strchr(unichar*,unichar);
int u_strrchr(const unichar*,unichar);
int u_strrchr(const unichar*,char);
const char* u_strchr(const char*,unichar);
unichar* u_strpbrk(const unichar*,unichar*);
unichar* u_strpbrk(const unichar*,char*);
int u_starts_with(const unichar*,const unichar*);
int u_starts_with(const unichar*,const char*);
int u_ends_with(const unichar*,const unichar*);
int u_ends_with(const unichar*,const char*);
int u_substr(const unichar*,const unichar*);


unichar* u_strdup(const unichar* str,Abstract_allocator prv_alloc);
unichar* u_strdup(const char* str,Abstract_allocator prv_alloc);
unichar* u_strdup(const unichar* str,int n,Abstract_allocator prv_alloc);

size_t convert_utf8_to_unichar(unichar*dest, size_t nb_unichar_alloc_walk, size_t * p_size_this_string_written,
    const unsigned char*src, size_t buf_size);

// define NO_CPP_TEMPLATE_SUPPORT if you archeological C++ compiler don't support template
#ifndef NO_CPP_TEMPLATE_SUPPORT
template <typename T>
int u_escape(const unichar* source, T* destination);
#endif


void u_to_char(char*,unichar*);
void u_to_char_n(char *, const unichar *, unsigned int);
void u_chomp_new_line(unichar*);
int JSONize(const unichar* source,unichar* destination);
int XMLize(const unichar* source,unichar* destination);
int URLize(const unichar*,unichar*);
int htmlize(const unichar*,unichar*);
int mirror(const unichar*,unichar*);
int mirror(unichar*);
int get_longuest_prefix(unichar*,unichar*);
unsigned int hash_unichar(unichar*);


/* ------------------- Character functions ------------------- */
int u_is_digit(unichar);
int u_is_basic_latin_letter(unichar);
int u_is_ASCII_alphanumeric(unichar);
int u_is_latin1_supplement_letter(unichar);
int u_is_latin_extendedA_letter(unichar);
int u_is_latin_extendedB_letter(unichar);
int u_is_IPA_extensions_letter(unichar);
int u_is_greek_letter(unichar);
int u_is_cyrillic_letter(unichar);
int u_is_armenian_letter(unichar);
int u_is_hebrew_letter(unichar);
int u_is_arabic_letter(unichar);
int u_is_thaana_letter(unichar);
int u_is_devanagari_letter(unichar);
int u_is_bengali_letter(unichar);
int u_is_gurmukhi_letter(unichar);
int u_is_gujarati_letter(unichar);
int u_is_oriya_letter(unichar);
int u_is_tamil_letter(unichar);
int u_is_telugu_letter(unichar);
int u_is_kannada_letter(unichar);
int u_is_malayalam_letter(unichar);
int u_is_sinhala_letter(unichar);
int u_is_thai_letter(unichar);
int u_is_greek_extended_letter(unichar);    //$CD:20021115
//--------Beginning of Hyungue's inserts------------------
int u_is_Hangul(unichar c);
int u_is_CJK_Unified_Ideograph(unichar c);
int u_is_CJK_compatibility_ideograph(unichar c);
int u_is_Hangul_Compatility_Jamo(unichar c);
int u_is_Hangul_Jamo(unichar c);
int u_is_Hangul_Jamo_initial_consonant(unichar c);
int u_is_Hangul_Jamo_final_consonant(unichar c);
int u_is_Hangul_Jamo_consonant(unichar c);
int u_is_Hangul_Jamo_medial_vowel(unichar c);
//--------End of Hyungue's inserts----------------
int u_is_letter(unichar);
int u_is_word(const unichar*);
int u_are_digits(const unichar*);


int u_parse_int(const unichar * str, const unichar ** next = NULL);


// Sebastian Nagel's functions
void u_toupper (unichar* s);
void u_tolower (unichar* s);
void u_deaccentuate(unichar* s);
int u_toupper_ismodified (unichar* s);
int u_tolower_ismodified (unichar* s);
int u_deaccentuate_ismodified(unichar* s);
unichar u_toupper(unichar);
unichar u_tolower(unichar);
unichar u_deaccentuate(unichar);
// end of Sebastian Nagel's functions

} // namespace unitex

#endif
