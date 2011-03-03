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

#include "CompressedDic.h"
#include "Error.h"
#include "AbstractAllocator.h"
#include "File.h"
#include "Ustring.h"
#include "StringParsing.h"
#include "List_ustring.h"
#include "LoadInf.h"
#include "AbstractDelaLoad.h"

static int read_bin_header(Dictionary*);

/**
 * Loads and returns a compressed dictionary.
 */
Dictionary* new_Dictionary(const char* bin,const char* inf,Abstract_allocator prv_alloc) {
Dictionary* d=(Dictionary*)malloc_cb(sizeof(Dictionary),prv_alloc);
if (d==NULL) {
	fatal_alloc_error("new_Dictionary");
}
d->bin=load_abstract_BIN_file(bin,&d->bin_size,&d->bin_free);
if (d->bin==NULL) {
	free(d);
	return NULL;
}
if (!read_bin_header(d)) {
	free_abstract_BIN(d->bin,&d->bin_free);
	free(d);
	return NULL;
}
d->inf=NULL;
if (d->type==BIN_CLASSIC) {
	if (inf==NULL) {
		error("NULL .inf file in new_Dictionary\n");
		free_abstract_BIN(d->bin,&d->bin_free);
		free(d);
		return NULL;
	}
	d->inf=load_abstract_INF_file(inf,&d->inf_free);
	if (d->inf==NULL) {
		free_abstract_BIN(d->bin,&d->bin_free);
		free(d);
		return NULL;
	}
}
return d;
}


/**
 * Frees all resources associated to the given dictionary
 */
void free_Dictionary(Dictionary* d,Abstract_allocator /* prv_alloc*/) {
if (d==NULL) return;
if (d->bin!=NULL) free_abstract_BIN(d->bin,&d->bin_free);
if (d->inf!=NULL) {
	free_abstract_INF(d->inf,&d->inf_free);
}
free(d);
}


/**
 * Reads a 2 byte-value. Updates the offset.
 */
static int bin_read_2bytes(const unsigned char* bin,int *offset) {
const unsigned char* bindata=bin+(*offset);
int v=(((int)bindata[0])<<8) | bindata[1];
(*offset)+=2;
return v;
}


/**
 * Reads a 3 byte-value. Updates the offset.
 */
static int bin_read_3bytes(const unsigned char* bin,int *offset) {
const unsigned char* bindata=bin+(*offset);
int v=(((int)bindata[0])<<16) | (((int)bindata[1])<<8) | bindata[2];
(*offset)+=3;
return v;
}


/**
 * Reads a 4 byte-value. Updates the offset.
 */
static int bin_read_4bytes(const unsigned char* bin,int *offset) {
const unsigned char* bindata=bin+(*offset);
int v=(((int)bindata[0])<<24) | (((int)bindata[1])<<16) | (((int)bindata[2])<<8) | bindata[3];
(*offset)+=4;
return v;
}


/**
 * Reads a variable length value. Updates the offset.
 */
static const unsigned int mask_from_sizebits[]={ 0x7f,0x7f,0x7f,0x7f, 0x1fff,0x1fffff,0x1fffffff,0xffffffff };
static int bin_read_variable_length (const unsigned char* bin,int *offset) {
const unsigned char* bindata=bin+(*offset);

unsigned char c0=bindata[0];
if ((c0 & 0x80)==0)
{
	(*offset)++;
	return (c0);
}

(*offset)+=(c0>>5)-2;
return ((c0 & 0x80) == 0) ? ((int)c0) :
		(int)(((c0 & 0x1f) | (((unsigned int)bindata[1])<<5) | (((unsigned int)bindata[2])<<13) |
		                     (((unsigned int)bindata[3])<<21) | (((unsigned int)bindata[4])<<29))
		  & mask_from_sizebits[c0>>5]);
}


/**
 * Writes a variable length value. Updates the offset.
 */

int bin_get_value_variable_length(int value) {
unsigned int uivalue=(unsigned int)value;
if (uivalue == ((uivalue) & 0x7f)) {
	return 1;
}
if (uivalue == ((uivalue) & 0x1fff)) {
	return 2;
}
if (uivalue == ((uivalue) & 0x1fffff)) {
	return 3;
}
if (uivalue == ((uivalue) & 0x1fffffff)) {
	return 4;
}
return 5;
}


static void bin_write_variable_length(unsigned char* bin,int value,int *offset) {
unsigned char* bindata=bin+(*offset);
unsigned int uivalue=(unsigned int)value;
if (uivalue == ((uivalue) & 0x7f)) {
	bindata[0]=(unsigned char)((uivalue));
	(*offset)+=1;
	return;
}
if (uivalue == ((uivalue) & 0x1fff)) {
	bindata[1]=(unsigned char)((uivalue>>5));
	bindata[0]=(unsigned char)((uivalue & 0x1f)| ((unsigned char)0x80));
	(*offset)+=2;
	return;
}
if (uivalue == ((uivalue) & 0x1fffff)) {
	bindata[0]=(unsigned char)((uivalue & 0x1f)| ((unsigned char)0xa0));
	bindata[1]=(unsigned char)((uivalue>>5));
	bindata[2]=(unsigned char)((uivalue>>13));
	(*offset)+=3;
	return;
}
if (uivalue == ((uivalue) & 0x1fffffff)) {
	bindata[0]=(unsigned char)((uivalue & 0x1f)| ((unsigned char)0xc0));
	bindata[1]=(unsigned char)((uivalue>>5));
	bindata[2]=(unsigned char)((uivalue>>13));
	bindata[3]=(unsigned char)((uivalue>>21));
	(*offset)+=4;
	return;
}
bindata[0]=(unsigned char)((uivalue & 0x1f)| ((unsigned char)0xe0));
bindata[1]=(unsigned char)((uivalue>>5));
bindata[2]=(unsigned char)((uivalue>>13));
bindata[3]=(unsigned char)((uivalue>>21));
bindata[4]=(unsigned char)((uivalue>>29));
(*offset)+=5;
}


/**
  * Select the bin_read_* function for a BinEncoding
  */
static t_fnc_bin_read_bytes get_bin_read_function_for_encoding(BinEncoding e) {
switch (e) {
case BIN_2BYTES: return &bin_read_2bytes;
case BIN_3BYTES: return &bin_read_3bytes;
case BIN_4BYTES: return &bin_read_4bytes;
case BIN_VARIABLE: return &bin_read_variable_length;
default: fatal_error("Illegal encoding value in get_bin_read_function_for_encoding\n");
}
return NULL;
}

/**
 * Writes a 2 byte-value. Updates the offset.
 */
static void bin_write_2bytes(unsigned char* bin,int value,int *offset) {
unsigned char* bindata=bin+(*offset);
bindata[0]=(value>>8) & 255;
bindata[1]=value & 255;
(*offset)+=2;
}


/**
 * Writes a 3 byte-value. Updates the offset.
 */
static void bin_write_3bytes(unsigned char* bin,int value,int *offset) {
unsigned char* bindata=bin+(*offset);
bindata[0]=(value>>16) & 255;
bindata[1]=(value>>8) & 255;
bindata[2]=value & 255;
(*offset)+=3;
}


/**
 * Writes a 4 byte-value. Updates the offset.
 */
void bin_write_4bytes(unsigned char* bin,int value,int *offset) {
unsigned char* bindata=bin+(*offset);
bindata[0]=(value>>24) & 255;
bindata[1]=(value>>16) & 255;
bindata[2]=(value>>8) & 255;
bindata[3]=value & 255;
(*offset)+=4;
}




/**
  * Select the bin_write_* function for a BinEncoding
  */
t_fnc_bin_write_bytes get_bin_write_function_for_encoding(BinEncoding e) {
switch (e) {
case BIN_2BYTES: return &bin_write_2bytes;
case BIN_3BYTES: return &bin_write_3bytes;
case BIN_4BYTES: return &bin_write_4bytes;
case BIN_VARIABLE: return &bin_write_variable_length;
default: fatal_error("Illegal encoding value in get_bin_write_function_for_encoding\n");
}
return NULL;
}

/**
 * Reads the information associated to the current state in the dictionary, i.e.
 * finality and number of outgoing transitions. Returns the new position.
 * If the state is final and if it is a classic .bin, we store the inf code
 * in *code.
 */
int read_dictionary_state(const Dictionary* d,int pos,int *final,int *n_transitions,int *code) {
if (d->state_encoding==BIN_CLASSIC_STATE) {
	*final=!(d->bin[pos] & 128);
	*n_transitions=((d->bin[pos] & 127)<<8) | (d->bin[pos+1]);
	pos=pos+2;
	if (*final) {
		*code=(d->inf_number_read_bin_func)(d->bin,&pos);
	} else {
		*code=-1;
	}
	return pos;
}

if ((d->state_encoding==BIN_NEW_STATE) || (d->state_encoding==BIN_BIN2_STATE)) {
int value=bin_read_variable_length(d->bin,&pos);
*final=value & 1;
*n_transitions=value>>1;
if (*final && d->state_encoding==BIN_NEW_STATE) {
	*code=(d->inf_number_read_bin_func)(d->bin,&pos);
} else {
	*code=-1;
}
return pos;
}

fatal_error("read_dictionary_state: unsupported state encoding");
return 0;
}

/**
 * Writes the information associated to the current state in the dictionary, i.e.
 * finality and number of outgoing transitions. Updates the position.
 */
void write_dictionary_state(unsigned char* bin,BinStateEncoding state_encoding,
							t_fnc_bin_write_bytes inf_number_write_function,int *pos,int final,int n_transitions,int code) {
if (state_encoding==BIN_CLASSIC_STATE) {
	int value=n_transitions & ((1<<15)-1);
	if (!final) value=value | (1<<15);
	bin_write_2bytes(bin,value,pos);
	if (final) {
		(*inf_number_write_function)(bin,code,pos);
	}
	return;
}
if (state_encoding!=BIN_NEW_STATE && state_encoding!=BIN_BIN2_STATE) {
	fatal_error("read_dictionary_state: unsupported state encoding\n");
}
int value=(n_transitions<<1);
if (final) value=value | 1;
bin_write_variable_length(bin,value,pos);
if (final && state_encoding==BIN_NEW_STATE) {
	(*inf_number_write_function)(bin,code,pos);
}
}


/**
 * Reads the information associated to the current transition in the dictionary.
 * Returns the new position.
 */
int read_dictionary_transition(const Dictionary* d,int pos,unichar *c,int *dest,Ustring* output) {
*c=(unichar)(d->char_read_bin_func)(d->bin,&pos);
*dest=(d->offset_read_bin_func)(d->bin,&pos);
if (d->type==BIN_CLASSIC) return pos;
if (d->type==BIN_BIN2) {
	int is_output=(*dest) & 1;
	(*dest)=(*dest)>>1;
	if (is_output) {
		int tmp;
		while ((tmp=((d->char_read_bin_func)(d->bin,&pos)))!='\0') {
			u_strcat(output,(unichar)tmp);
		}
	}
	return pos;
}
fatal_error("read_dictionary_state: unsupported dictionary type\n");
return 0;
}

/**
 * Writes the information associated to the current transition in the dictionary.
 * Updates the position.
 */
void write_dictionary_transition(unsigned char* bin,int *pos,t_fnc_bin_write_bytes char_write_function,
								t_fnc_bin_write_bytes offset_write_function,unichar c,int dest,
								BinType bin_type,unichar* output) {
(*char_write_function)(bin,c,pos);
if (bin_type==BIN_CLASSIC) {
	(*offset_write_function)(bin,dest,pos);
	return;
}
/* For .bin2, we have a bit to indicate whether there is an output or not */
dest=dest<<1;
if (output!=NULL && output[0]!='\0') {
	dest=dest|1;
}
(*offset_write_function)(bin,dest,pos);
if (dest & 1) {
	int i=-1;
	do {
		i++;
		(*char_write_function)(bin,output[i],pos);
	} while (output[i]!='\0');
}
}


/**
 * Returns the number of bytes required to save the given value.
 */
int bin_get_value_length(int v,BinEncoding e) {
switch (e) {
case BIN_2BYTES: return 2;
case BIN_3BYTES: return 3;
case BIN_4BYTES: return 4;
case BIN_VARIABLE: return bin_get_value_variable_length(v);
default: fatal_error("bin_get_value_length: unsupported encoding\n");
}
return -1;
}


/**
 * Returns the number of bytes required to save the given value.
 */
int bin_get_value_length(int v,t_fnc_bin_write_bytes func) {
if (func==(&bin_write_2bytes)) return 2;
if (func==(&bin_write_3bytes)) return 3;
if (func==(&bin_write_variable_length)) return bin_get_value_variable_length(v);

fatal_error("bin_get_value_length: unsupported function\n");
return -1;
}


/**
 * Returns the length in bytes of the given string, including the \0.
 */
int bin_get_string_length(unichar* s,BinEncoding char_encoding) {
int n=0,i=-1;
do {
	i++;
	n+=bin_get_value_length(s[i],char_encoding);
} while (s[i]!='\0');
return n;
}

int bin_get_string_length(unichar* s,t_fnc_bin_write_bytes char_encoding_func) {
int n=0,i=-1;
do {
	i++;
	n+=bin_get_value_length(s[i],char_encoding_func);
} while (s[i]!='\0');
return n;
}

static void select_bin_read_function(Dictionary* d) {
	d->inf_number_read_bin_func=get_bin_read_function_for_encoding(d->inf_number_encoding);
	d->char_read_bin_func=get_bin_read_function_for_encoding(d->char_encoding);
	d->offset_read_bin_func=get_bin_read_function_for_encoding(d->offset_encoding);
}

/**
 * This function assumes that the .bin file has already been loaded.
 * It analyzes its header to determine the kind of .bin it is.
 * Returns 0 in case of error; 1 otherwise.
 */
static int read_bin_header(Dictionary* d) {
if (d->bin[0]==0) {
	/* Type 1: old style .bin/.inf dictionary */
	d->initial_state_offset=4;
	if (d->bin_size<d->initial_state_offset+2) {
		/* The minimal empty dictionary is made of the header plus a 2 bytes empty state */
		error("Invalid .bin size\n");
		return 0;
	}
	/* If we have an old style .bin */
	d->type=BIN_CLASSIC;
	d->state_encoding=BIN_CLASSIC_STATE;
	d->inf_number_encoding=BIN_3BYTES;
	d->char_encoding=BIN_2BYTES;
	d->offset_encoding=BIN_3BYTES;
	select_bin_read_function(d);
	return 1;
}
if (d->bin[0]==1) {
	/* Type 2: modern style .bin/.inf dictionary with no limit on .bin size */
	d->type=BIN_CLASSIC;
	d->state_encoding=(BinStateEncoding)d->bin[1];
	d->inf_number_encoding=(BinEncoding)d->bin[2];
	d->char_encoding=(BinEncoding)d->bin[3];
	d->offset_encoding=(BinEncoding)d->bin[4];
	select_bin_read_function(d);
	int offset=5;
	d->initial_state_offset=bin_read_4bytes(d->bin,&offset);
	return 1;
}
if (d->bin[0]==2) {
	/* Type 3: .bin2 dictionary */
	d->type=BIN_BIN2;
	d->state_encoding=(BinStateEncoding)d->bin[1];
	d->inf_number_encoding=(BinEncoding)d->bin[2];
	d->char_encoding=(BinEncoding)d->bin[3];
	d->offset_encoding=(BinEncoding)d->bin[4];
	select_bin_read_function(d);
	int offset=5;
	d->initial_state_offset=bin_read_4bytes(d->bin,&offset);
	return 1;
}
error("Unknown dictionary type: %d\n",d->bin[0]);
return 0;
}


/**
 * Writes the .bin header for a v2 .bin or a .bin2
 */
void write_new_bin_header(BinType bin_type,unsigned char* bin,int *pos,BinStateEncoding state_encoding,
		BinEncoding char_encoding,BinEncoding inf_number_encoding,
		BinEncoding offset_encoding,int initial_state_offset) {
bin[(*pos)++]=(bin_type==BIN_CLASSIC)?1:2;
bin[(*pos)++]=(unsigned char)state_encoding;
bin[(*pos)++]=(unsigned char)inf_number_encoding;
bin[(*pos)++]=(unsigned char)char_encoding;
bin[(*pos)++]=(unsigned char)offset_encoding;
bin_write_4bytes(bin,initial_state_offset,pos);
}


/**
 * Returns the current length of s or -1 if NULL.
 */
int save_output(Ustring* s) {
if (s==NULL) return -1;
return s->len;
}


/**
 * If s is not NULL, sets its size to n.
 */
void restore_output(int n,Ustring* s) {
if (s!=NULL && n!=-1) {
	s->len=n;
	s->str[n]='\0';
}
}


/**
 * This function stores in *inf_codes the inf code list associated either to the inf number
 * or to the given output, if the dictionary is a .bin2 one. The function returns 1
 * if *inf_codes should be freed (.bin2), 0 if not (.bin).
 */
int get_inf_codes(Dictionary* d,int inf_number,Ustring* output,struct list_ustring* *inf_codes,
				int base) {
*inf_codes=NULL;
if (d->type==BIN_CLASSIC) {
	if (inf_number!=-1) {
		*inf_codes=d->inf->codes[inf_number];
	}
	return 0;
}
if (d->type!=BIN_BIN2) {
	fatal_error("get_inf_codes: unsupported dictionary type\n");
}
if (output!=NULL && output->str[base]!='\0') {
	*inf_codes=tokenize_compressed_info(output->str+base);
}
return 1;
}

