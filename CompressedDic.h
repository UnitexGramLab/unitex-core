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
#include "AbstractDelaLoad.h"
#include "Ustring.h"

#define BIN_V1_HEADER_SIZE 4
#define BIN_V2_HEADER_SIZE 9

/**
 * These are the encodings that may be used to represent offsets and chars
 * in the transducer:
 */
typedef enum {
	BIN_2BYTES,   /* 2 bytes in big-endian */
	BIN_3BYTES,   /* 3 bytes in big-endian */
	BIN_4BYTES,   /* 4 bytes in big-endian */
	BIN_VARIABLE  /* variable length encoding */
} BinEncoding;


typedef enum {
	BIN_CLASSIC,   /* old style .bin/.inf dictionary */
	BIN_BIN2       /* .bin2 dictionary style, with outputs included in the transducer */
} BinType;


typedef enum {
	BIN_CLASSIC_STATE,   /* old style .bin state encoding on 2 bytes */
	BIN_NEW_STATE,       /* variable length state encoding */
	BIN_BIN2_STATE,      /* .bin2 state encoding */
} BinStateEncoding;



/**
 * This structure represents a compressed dictionary.
 */
typedef struct {
	BinType type;
	int initial_state_offset;
	long bin_size;

	/* Encodings used to store data in the transducer */
	BinStateEncoding state_encoding;
	BinEncoding inf_number_encoding;
	BinEncoding char_encoding;
	BinEncoding offset_encoding;

	/* The binary transducer */
	const unsigned char* bin;
	struct BIN_free_info bin_free;
	/* The codes contained in the .inf file */
	const struct INF_codes* inf;
	struct INF_free_info inf_free;
} Dictionary;


Dictionary* new_Dictionary(const char* bin,const char* inf,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_Dictionary(Dictionary* d,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
int read_dictionary_state(Dictionary*,int,int*,int*,int*);
void write_dictionary_state(unsigned char* bin,BinStateEncoding state_encoding,
							BinEncoding inf_number_encoding,int *pos,int final,int n_transitions,int code);
int read_dictionary_transition(Dictionary*,int,unichar*,int*,Ustring*);
void write_dictionary_transition(unsigned char* bin,int *pos,BinEncoding char_encoding,
								BinEncoding offset_encoding,unichar c,int dest,
								BinType bin_type,unichar* output);
int bin_get_value_length(int,BinEncoding);
int bin_get_string_length(unichar* s,BinEncoding char_encoding);
void bin_write_4bytes(unsigned char* bin,int value,int *offset);
void write_new_bin_header(unsigned char* bin,int *pos,BinStateEncoding state_encoding,
		BinEncoding char_encoding,BinEncoding inf_number_encoding,
		BinEncoding offset_encoding,int initial_state_offset);

#endif

