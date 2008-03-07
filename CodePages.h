 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#define ONE_BYTE_ENCODING 0
#define UTF8 1
#define UTF16_LE 2
#define UTF16_BE 3
#ifndef HGH_INSERT
#define MBCS_KR    4
#endif // HGH_INSERT

/*
 * Error codes that may be returned by the 'convert' function
 */
#define CONVERSION_OK 0
#define INPUT_FILE_NOT_IN_UTF16_LE 1
#define INPUT_FILE_NOT_IN_UTF16_BE 2
#define UNSUPPORTED_INPUT_ENCODING 3
#define ERROR_IN_HTML_CHARACTER_NAME 4


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
	int (*input_function)(FILE*);
	int (*output_function)(unichar,FILE*);
	
	/* The usage function for this encoding */
	void (*usage_function)(void);
	/* This function returns 1 if the given char can be encoded with this encoding */
	int (*can_be_encoded_function)(unichar);
};


void install_all_encodings();
int convert(FILE*,FILE*,struct encoding*,struct encoding*,int,int,int,int);
struct encoding* get_encoding(char*);

void print_encoding_main_names();
void print_encoding_aliases();
void print_encoding_infos(char*);
void print_information_for_all_encodings();

#endif
