 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

/* This line is used to prevent people from using printf and scanf. We do
 * that because we want to parametrize I/O operations with encodings. */
#define printf DONT_USE_PRINTF_BUT_U_PRINTF
#define scanf DONT_USE_SCANF_BUT_U_SCANF


/**
 * This library provides basic I/O unicode operations. For internal data
 * representations, UTF16-LE is used. It is also the default encoding for
 * reading and writing text files. UTF8 is the default encoding for writing
 * to the standard and error outputs.
 */


/**
 * These are the binary open modes for unicode text files.
 */
#define U_READ "rb"
#define U_WRITE "wb"
#define U_APPEND "ab"
#define U_MODIFY "r+b"


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
typedef unsigned short int unichar;


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
 * We define here the I/O encodings that are supported by the Unicode library.
 */
typedef enum {
   UTF16_LE,
   UTF16_BE,
   UTF8
} Encoding;

/**
 * Here are defined the default encodings.
 */
extern Encoding STDIN_ENC8;
extern Encoding STDOUT_ENC;
extern Encoding STDERR_ENC;
extern Encoding FILE_ENC;


/* ------------------- Some aliases, mainly for default UTF16-LE use ------------------- */
FILE* u_fopen(const char*,const char*);
int u_fempty(const char*);

int u_fgetc_raw(FILE*);
int u_fgetc_UTF16LE(FILE*);
int u_fgetc_UTF16BE(FILE*);
int u_fgetc_UTF8(FILE*);
int u_fgetc(FILE*);
int u_fgetc_CR(FILE*);
int u_fread_raw(unichar*,int,FILE*);
int u_fread(unichar*,int,FILE*,int*);

int u_fputc_raw(unichar,FILE*);
int u_fputc_UTF16LE(unichar,FILE*);
int u_fputc_UTF16BE(unichar,FILE*);
int u_fputc_UTF8(unichar,FILE*);
int u_fputc(unichar,FILE*);
int u_ungetc(unichar,FILE*);

int u_fwrite_raw(unichar*,int,FILE*);
int u_fwrite(unichar*,int,FILE*);

int u_fgets(unichar*,FILE*);
int u_fgets(unichar*,int,FILE*);
int u_fgets2(unichar*,FILE*);

int u_printf(const char*,...);
int u_fprintf(FILE*,const char*,...);

int u_scanf(const char*,...);
int u_fscanf(FILE*,const char*,...);

/* The u_prints and u_fprints functions should not be visible from the
 * outside of this library. People should use u_printf and u_fprintf */
//void u_prints(unichar*);
//void u_fprints(unichar*,FILE*);
//void u_fprints(char*,FILE*);
//void u_fprints(Encoding,unichar*,FILE*);
//void u_fprints(Encoding,char*,FILE*);

/* ------------------- File functions ------------------- */
FILE* u_fopen(Encoding,const char*,const char*);
int u_fclose(FILE*);
int u_fempty(Encoding,const char*);
int u_is_UTF16(const char*);

int u_fgetc_UTF16LE_raw(FILE*);
int u_fgetc_UTF16BE_raw(FILE*);
int u_fgetc_UTF8_raw(FILE*);
int u_fgetc_raw(Encoding,FILE*);
int u_fgetc(Encoding,FILE*);
int u_fgetc_CR(Encoding,FILE*);

int u_fread_raw(Encoding,unichar*,int,FILE*);
int u_fread(Encoding,unichar*,int,FILE*,int*);

int u_fputc_UTF16LE_raw(unichar,FILE*);
int u_fputc_UTF16BE_raw(unichar,FILE*);
int u_fputc_UTF8_raw(unichar,FILE*);
int u_fputc_raw(Encoding,unichar,FILE*);
int u_fputc(Encoding,unichar,FILE*);

int u_ungetc_UTF16LE_raw(FILE*);
int u_ungetc_UTF16BE_raw(FILE*);
int u_ungetc_UTF8_raw(unichar,FILE*);
int u_ungetc_raw(Encoding,unichar,FILE*);
int u_ungetc(Encoding,unichar,FILE*);

int u_fwrite_raw(Encoding,unichar*,int,FILE*);
int u_fwrite(Encoding,unichar*,int,FILE*);

int u_fgets(Encoding,unichar*,FILE*);
int u_fgets(Encoding,unichar*,int,FILE*);
int u_fgets2(Encoding encoding,unichar* line,FILE* f);

int u_fprintf(Encoding,FILE*,const char*,...);
int u_vfprintf(Encoding,FILE*,const char*,va_list);
int u_sprintf(unichar*,const char*,...);
int u_vsprintf(unichar*,const char*,va_list);

int u_fscanf(Encoding,FILE*,const char*,...);
int u_vfscanf(Encoding,FILE*,const char*,va_list);
int u_sscanf(unichar*,const char*,...);
int u_vsscanf(unichar*,const char*,va_list);

/* ------------------- String functions ------------------- */
int u_strlen(const unichar*);
unichar* u_strcpy(unichar*,const unichar*);
unichar* u_strcpy(unichar*,const char*);
unichar* u_strcat(unichar*,const unichar*);
unichar* u_strcat(unichar*,const char*);
int u_strcmp(const unichar*, const unichar*);
int u_equal(const unichar*, const unichar*);
int u_strcmp(const unichar*,const char*);
unichar* u_strdup(const unichar*);
unichar* u_strdup(const unichar*,int);
unichar* u_strdup(const char*);
unichar* u_strchr(const unichar*,unichar,int);
unichar* u_strchr(const unichar*,unichar);
const char* u_strchr(const char*,unichar);
unichar* u_strpbrk(const unichar*,unichar*);
unichar* u_strpbrk(const unichar*,char*);
int u_starts_with(const unichar*,const unichar*);
int u_starts_with(const unichar*,const char*);
int u_ends_with(const unichar*,const unichar*);
int u_ends_with(const unichar*,const char*);

void u_to_char(char*,unichar*);
int htmlize(unichar*,unichar*);
int mirror(unichar*,unichar*);
int mirror(unichar*);
int get_longuest_prefix(unichar*,unichar*);
unsigned int hash_unichar(unichar*);


/* ------------------- Character functions ------------------- */
int u_is_digit(unichar);
int u_is_basic_latin_letter(unichar);
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
int u_is_greek_extended_letter(unichar);	//$CD:20021115
//--------Beginning of Hyungue's inserts------------------
int u_is_korea_syllabe_letter(unichar c);
int u_is_CJK_Unified_Ideographs(unichar c);
int u_is_cjk_compatibility_ideographs(unichar c);
int u_is_Hangul_Compatility_Jamo(unichar c);
int u_is_Hangul_Jamo(unichar c);
//--------End of Hyungue's inserts----------------
int u_is_letter(unichar);
int u_is_word(unichar*);



int u_parse_int(unichar * str, unichar ** next = NULL);


// Sebastian Nagel's functions
unichar u_toupper(unichar);
unichar u_tolower(unichar);
unichar u_deaccentuate(unichar);
// end of Sebastian Nagel's functions


#endif

