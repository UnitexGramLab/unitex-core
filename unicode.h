 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#define UnicodeH


//
// these are the binary open modes for unicode text file
//
#define U_READ "rb"
#define U_WRITE "wb"
#define U_APPEND "ab"


// this caracter is used as the first one of a unicode text file
#define U_BYTE_ORDER_MARK 0xFEFF
// and this one is used by contrast with 0xFEFF to determine the byte order
// in the file
#define U_NOT_A_CHAR 0xFFFE

#define FILE_DOES_NOT_EXIST 0
#define NOT_A_UNICODE_FILE 1
#define UNICODE_LITTLE_ENDIAN_FILE 2
#define UNICODE_BIG_ENDIAN_FILE 3



//extern char CR[2048];
extern char* NBSP;

typedef unsigned short int unichar;

/* This constant must be used instead of 0xFFFF or 65536 in order
 * to avoid problems if one day the type unichar becomes larger
 * than 2 bytes */
#define MAX_NUMBER_OF_UNICODE_CHARS (1<<sizeof(unichar)*8)


void u_strcpy(unichar*, const unichar*);
unichar* u_strncpy(unichar*, const unichar*, int);
void u_strcpy_char(unichar*, const char*);
void u_strcpy_char_with_no_zero(unichar*, const char*);
int u_strlen(const unichar*);
unichar* u_strcat(unichar*,unichar*);
void u_strcat_char(unichar*,char*);
int u_strcmp(const unichar*, const unichar*);
int u_strcmp_char(const unichar*,const char*);
void u_get_substring(const unichar*,int,int,unichar*);
int u_fgetc(FILE*);
int u_fgetc_raw(FILE*);
int u_fgetc_raw_big_endian(FILE*);
int u_fgetc_big_endian(FILE*);
int u_fgetc_normalized_carridge_return(FILE*);
int u_fgetc_normalized_carridge_return_big_endian(FILE*);
int fgetc_normalized_carridge_return(FILE*);
int u_fputc(unichar,FILE*);
int u_fputc_raw(unichar,FILE*);
int u_fputc_big_endian(unichar,FILE*);
int u_fputc_raw_big_endian(unichar,FILE*);
int u_fputc_utf8(unichar,FILE*);
int u_fputc_utf8_diese(unichar c,FILE *f);

FILE* u_fopen(char*,char*);
int u_fclose(FILE*);
int u_fread(unichar*,int,FILE*);
int u_fwrite(unichar*,int,FILE*);
int u_is_a_unicode_file(char*);
void u_prints(unichar*);
void u_fprints(unichar*,FILE*);
void u_fprints_reverse(unichar*,FILE*);
void u_fprints_utf8(unichar*,FILE*);
void u_fprints_utf8_reverse(unichar*,FILE*);
void u_fprints_html(unichar*,FILE*);
void u_fprints_html_reverse(unichar*,FILE*);
void u_fprints_html_like(unichar*,FILE*);
void u_fprints_html_ascii(unichar*, FILE*);
inline void unichar2htmlEnt(char*, unichar);
void u_fprints_char(char*,FILE*);
void u_to_char(char*,unichar*);
int u_read_int(FILE*);
void u_char_to_hexa(unichar,unichar*);
void u_char_to_hexa_or_code(unichar,unichar*);
void u_int_to_string(int,unichar*);
int u_read_line(FILE*,unichar*);
int only_spaces(unichar*);
void u_reverse_string(unichar*);
int u_strlen_thai_without_diacritic(unichar*);
int are_equivalent_quotes(unichar,unichar);

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
int u_is_diacritic_thai(unichar);
int u_is_to_be_ignored_thai(unichar);
int u_is_vowel_thai(unichar);
long int u_file_size(char*);
//char make_CR();
void u_strcpy_without_space(unichar*,unichar*);
void remove_prefix(unichar*,unichar);


// Olivier Blanc's functions

int u_fgets(unichar * str, int size, FILE * f);
int u_parse_int(unichar * str, unichar ** next = NULL);
unichar * u_strdup(const unichar * str);
unichar * u_strdup_char(const char * str);

void u_sprintf(unichar * str, char * fmt, ...);
void u_vsprintf(unichar * str, char * fmt, va_list plist);
void i_sprintf(char * f, char * fmt, ...);

void u_fprintf(FILE * f, char * fmt, ...);
void i_fprintf(FILE * f, char * fmt, ...);
void i_vfprintf(FILE * f, char * fmt, va_list plist);

unichar * u_strchr(const unichar * str, unichar c);
unichar * u_strtok_char(unichar * str, char * delim);
unichar * u_strpbrk(const unichar * str, char * E);


int calc_printf_size(char * fmt, va_list plist);

// end of Olivier Blanc's functions


// Sebastian Nagel's functions
unichar u_toupper(unichar);
unichar u_tolower(unichar);


#endif

