/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef CodePagesH
#define CodePagesH

#include <stdio.h>

/*
 * Encoding types
 */
#define E_ONE_BYTE_ENCODING 0
#define E_UTF8 1
#define E_UTF16_LE 2
#define E_UTF16_BE 3
#define E_UTF8_BOM 0x101
#define E_UTF16_LE_BOM 0x102
#define E_UTF16_BE_BOM 0x103
#define E_UTF8_NO_BOM 0x201
#define E_UTF16_LE_NO_BOM 0x202
#define E_UTF16_BE_NO_BOM 0x203

/*
 * Error codes that may be returned by the 'convert' function
 */
#define CONVERSION_OK 0
#define INPUT_FILE_NOT_IN_UTF16_LE 1
#define INPUT_FILE_NOT_IN_UTF16_BE 2
#define UNSUPPORTED_INPUT_ENCODING 3
#define ERROR_IN_HTML_CHARACTER_NAME 4
#define INPUT_FILE_NOT_IN_UTF8 5

#define CONV_REGULAR_FILE 0
#define CONV_DELAS_FILE 1
#define CONV_DELAF_FILE 2


/**
 * This structure represents a one byte encoding.
 */
struct encoding {
	/* The type defines if we have a one byte encoding,
	 * a UTF-16 one, a UTF-8 one, etc. */
	int type;
	/* Main name of the encoding ("iso-8859-1") */
	char* name;
	/* Other names for this encoding ("latin1","latin-1") */
	char** aliases;
	/* Size of 'aliases' */
	int number_of_aliases;

	/* The code page initialization function for this encoding.
	 * This function is used only if the encoding type is ON_BYTE_ENCODING */
	void (*init_function)(unichar*);
	/*
	 * If the encoding type is not ON_BYTE_ENCODING, we must define
	 * an input and output function.
	 */
	int (*input_function)(ABSTRACTFILE*);
	int (*output_function)(unichar,ABSTRACTFILE*);

	int (*input_function_ctx)(ABSTRACTFILE*,const void*);
	int (*output_function_ctx)(unichar,ABSTRACTFILE*,const void*);

	/* The usage function for this encoding */
	void (*usage_function)(void);
	/* This function returns 1 if the given char can be encoded with this encoding */
	int (*can_be_encoded_function)(unichar,const unsigned char*);
};


void* install_all_encodings();
void free_encodings_context(void*);
int convert(const void*,U_FILE*,U_FILE*,const struct encoding*,const struct encoding*,int,int,int,int,int);
const struct encoding* get_encoding(const void*,const char*);

void print_encoding_main_names(const void*);
void print_encoding_aliases(const void*);
void print_encoding_infos(const void*,const char*);
void print_information_for_all_encodings(const void*);

#endif
