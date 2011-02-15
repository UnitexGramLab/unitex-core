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

#ifndef CompressedDicH
#define CompressedDicH

#include "Unicode.h"

/**
 * These are the encodings that may be used to represent offsets and chars
 * in the transducer:
 */
typedef enum {
	BIN_2BYTES,   /* 2 bytes in big-endian */
	BIN_3BYTES,   /* 3 bytes in big-endian */
	BIN_VARIABLE  /* variable length encoding */
} BinEncoding;


typedef enum {
	BIN_CLASSIC,   /* old style .bin/.inf dictionary */
	BIN_BIN2       /* .bin2 dictionary style, with outputs included in the transducer */
} BinType;




/**
 * This structure represents a compressed dictionary.
 */
typedef struct {
	BinType type;
	int header_size;
	long bin_size;

	/* Encodings used to store data in the transducer */
	BinEncoding inf_number_encoding;
	BinEncoding char_encoding;
	BinEncoding offset_encoding;

	/* The binary transducer */
	unsigned char* bin;
	/* The codes contained in the .inf file */
	struct INF_codes* inf;
} Dictionary;


Dictionary* new_Dictionary(const char* bin,const char* inf,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_Dictionary(Dictionary* d,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
int read_dictionary_state(Dictionary*,int,int*,int*,int*);
int read_dictionary_transition(Dictionary*,int,unichar*,int*);
void test(BinEncoding e);


/* Those exports should disappear once AbstractDelaLoad is rewritten to be adapted to the
 * new Dictionary structure */
unsigned char* load_BIN_file(const char*,long*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_BIN_file(unsigned char*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);

#endif

