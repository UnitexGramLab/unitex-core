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

//---------------------------------------------------------------------------
#ifndef CodePagesH
#define CodePagesH

#include <stdio.h>

//------- encodings --------------
#define ONE_BYTE_ENCODING 0
#define UTF8 1
#define UTF16_LE 2
#define UTF16_BE 3
#ifndef HGH_INSERT
#define MBCS_KR    4
#endif // HGH_INSERT
//------- error codes for conversion --------------
#define CONVERSION_OK 0
#define INPUT_FILE_NOT_IN_UTF16_LE 1
#define INPUT_FILE_NOT_IN_UTF16_BE 2
#define UNSUPPORTED_INPUT_ENCODING 3


extern unichar unicode_src[0xFF];
extern unichar unicode_dest[0xFF];
extern unsigned char ascii_dest[MAX_NUMBER_OF_UNICODE_CHARS];

void init_thai(unichar unicode[]);
void init_ansi(unichar unicode[]);
void init_grec(unichar unicode[]);
void init_tcheque(unichar unicode[]);
void init_windows1250(unichar unicode[]);
void init_windows1257(unichar unicode[]);
void init_windows1251(unichar unicode[]);
void init_windows1254(unichar unicode[]);
void init_windows1258(unichar unicode[]);
void init_iso88591(unichar unicode[]);
void init_iso885915(unichar unicode[]);
void init_iso88592(unichar unicode[]);
void init_iso88593(unichar unicode[]);
void init_iso88594(unichar unicode[]);
void init_iso88595(unichar unicode[]);
void init_iso88597(unichar unicode[]);
void init_iso88599(unichar unicode[]);
void init_iso885910(unichar unicode[]);
void init_nextstep(unichar unicode[]);
#ifndef HGH_INSERT
void init_windows949();    // korean wangsung code EUC-KR
#endif // HGH_INSERT



void init_uni2asc_code_page_array();

int convert(FILE* input,FILE* output,int INPUT_ENCODING,int OUTPUT_ENCODING);
void convert_unicode_to_ascii(FILE*,FILE*);
void convert_ascii_to_unicode(FILE*,FILE*);
void convert_big_to_little_endian(FILE*,FILE*,char*);
void convert_little_to_big_endian(FILE*,FILE*);
void convert_unicode_to_UTF_8(FILE *entree,FILE *sortie);

void usage();
void usage_LATIN();
void usage_windows1250();
void usage_windows1257();
void usage_windows1251();
void usage_windows1254();
void usage_windows1258();
void usage_iso88591();
void usage_iso885915();
void usage_iso88592();
void usage_iso88593();
void usage_iso88594();
void usage_iso88595();
void usage_iso88597();
void usage_iso88599();
void usage_iso885910();
#ifndef HGH_INSERT
void usage_windows949();    // korean wangsung code EUC-KR
#endif // HGH_INSERT
#endif
