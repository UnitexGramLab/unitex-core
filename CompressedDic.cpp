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
int bin_read_2bytes(const unsigned char* bin,int *offset) {
int v=(bin[*offset]<<8)+bin[(*offset)+1];
(*offset)+=2;
return v;
}


/**
 * Reads a 3 byte-value. Updates the offset.
 */
int bin_read_3bytes(const unsigned char* bin,int *offset) {
int v=(bin[*offset]<<16)+(bin[(*offset)+1]<<8)+bin[(*offset)+2];
(*offset)+=3;
return v;
}


/**
 * Reads a 4 byte-value. Updates the offset.
 */
int bin_read_4bytes(const unsigned char* bin,int *offset) {
int v=(bin[*offset]<<24)+(bin[(*offset)+1]<<16)+(bin[(*offset)+2]<<8)+bin[(*offset)+3];
(*offset)+=4;
return v;
}


/**
 * Reads a variable length value. Updates the offset.
 */
int bin_read_variable_length(const unsigned char* bin,int *offset) {
int v=0;
do {
	v=(v<<7) | (bin[(*offset)++] & 127);
} while (bin[(*offset)-1] & 128);
return v;
}


int bin_read(const unsigned char* bin,BinEncoding e,int *offset) {
switch (e) {
case BIN_2BYTES: return bin_read_2bytes(bin,offset);
case BIN_3BYTES: return bin_read_3bytes(bin,offset);
case BIN_4BYTES: return bin_read_4bytes(bin,offset);
case BIN_VARIABLE: return bin_read_variable_length(bin,offset);
default: fatal_error("Illegal encoding value in bin_read\n");
}
return -1;
}


/**
 * Writes a 2 byte-value. Updates the offset.
 */
void bin_write_2bytes(unsigned char* bin,int value,int *offset) {
bin[(*offset)++]=(value>>8) & 255;
bin[(*offset)++]=value & 255;
}


/**
 * Writes a 3 byte-value. Updates the offset.
 */
void bin_write_3bytes(unsigned char* bin,int value,int *offset) {
bin[(*offset)++]=(value>>16) & 255;
bin[(*offset)++]=(value>>8) & 255;
bin[(*offset)++]=value & 255;
}


/**
 * Writes a 4 byte-value. Updates the offset.
 */
void bin_write_4bytes(unsigned char* bin,int value,int *offset) {
bin[(*offset)++]=(value>>24) & 255;
bin[(*offset)++]=(value>>16) & 255;
bin[(*offset)++]=(value>>8) & 255;
bin[(*offset)++]=value & 255;
}


/**
 * Writes a variable length value. Updates the offset.
 */
void bin_write_variable_length(unsigned char* bin,int value,int *offset) {
if (value<(1<<7)) {
	bin[(*offset)++]=(unsigned char)value;
	return;
}
if (value<(1<<14)) {
	bin[(*offset)++]=(unsigned char)((value>>7)+128);
	bin[(*offset)++]=(unsigned char)((value & 127));
	return;
}
if (value<(1<<21)) {
	bin[(*offset)++]=(unsigned char)(((value>>14)+128));
	bin[(*offset)++]=(unsigned char)(((value>>7) & 127)+128);
	bin[(*offset)++]=(unsigned char)((value & 127));
	return;
}
if (value<(1<<28)) {
	bin[(*offset)++]=(unsigned char)((value>>21)+128);
	bin[(*offset)++]=(unsigned char)(((value>>14) & 127)+128);
	bin[(*offset)++]=(unsigned char)(((value>>7) & 127)+128);
	bin[(*offset)++]=(unsigned char)((value & 127));
	return;
}
bin[(*offset)++]=(unsigned char)((value>>28)+128);
bin[(*offset)++]=(unsigned char)(((value>>21) & 127)+128);
bin[(*offset)++]=(unsigned char)(((value>>14) & 127)+128);
bin[(*offset)++]=(unsigned char)(((value>>7) & 127)+128);
bin[(*offset)++]=(unsigned char)((value & 127));
}


void bin_write(unsigned char* bin,BinEncoding e,int value,int *offset) {
switch (e) {
case BIN_2BYTES: return bin_write_2bytes(bin,value,offset);
case BIN_3BYTES: return bin_write_3bytes(bin,value,offset);
case BIN_4BYTES: return bin_write_4bytes(bin,value,offset);
case BIN_VARIABLE: return bin_write_variable_length(bin,value,offset);
default: fatal_error("Illegal encoding value in bin_write\n");
}
}


/**
 * Reads the information associated to the current state in the dictionary, i.e.
 * finality and number of outgoing transitions. Returns the new position.
 * If the state is final and if it is a classic .bin, we store the inf code
 * in *code.
 */
int read_dictionary_state(Dictionary* d,int pos,int *final,int *n_transitions,int *code) {
if (d->state_encoding==BIN_CLASSIC_STATE) {
	*final=!(d->bin[pos] & 128);
	*n_transitions=((d->bin[pos] & 127)<<8)+d->bin[pos+1];
	pos=pos+2;
	if (*final) {
		*code=bin_read(d->bin,d->inf_number_encoding,&pos);
	} else {
		*code=-1;
	}
	return pos;
}
if (d->state_encoding!=BIN_NEW_STATE && d->state_encoding!=BIN_BIN2_STATE) {
	fatal_error("read_dictionary_state: unsupported state encoding\n");
}
int value=bin_read_variable_length(d->bin,&pos);
*final=value & 1;
*n_transitions=value>>1;
if (*final && d->state_encoding==BIN_NEW_STATE) {
	*code=bin_read(d->bin,d->inf_number_encoding,&pos);
} else {
	*code=-1;
}
return pos;
}


/**
 * Writes the information associated to the current state in the dictionary, i.e.
 * finality and number of outgoing transitions. Updates the position.
 */
void write_dictionary_state(unsigned char* bin,BinStateEncoding state_encoding,
							BinEncoding inf_number_encoding,int *pos,int final,int n_transitions,int code) {
if (state_encoding==BIN_CLASSIC_STATE) {
	int value=n_transitions & ((1<<15)-1);
	if (!final) value=value | (1<<15);
	bin_write_2bytes(bin,value,pos);
	if (final) {
		bin_write(bin,inf_number_encoding,code,pos);
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
	bin_write(bin,inf_number_encoding,code,pos);
}
}


/**
 * Reads the information associated to the current transition in the dictionary.
 * Returns the new position.
 */
int read_dictionary_transition(Dictionary* d,int pos,unichar *c,int *dest,Ustring* output) {
*c=(unichar)bin_read(d->bin,d->char_encoding,&pos);
*dest=bin_read(d->bin,d->offset_encoding,&pos);
if (d->type==BIN_CLASSIC) return pos;
if (d->type!=BIN_BIN2) {
	fatal_error("read_dictionary_state: unsupported dictionary type\n");
}
int is_output=(*dest) & 1;
(*dest)=(*dest)>>1;
if (is_output) {
	int tmp;
	while ((tmp=bin_read(d->bin,d->char_encoding,&pos))!='\0') {
		u_strcat(output,tmp);
	}
}
return pos;
}


/**
 * Writes the information associated to the current transition in the dictionary.
 * Updates the position.
 */
void write_dictionary_transition(unsigned char* bin,int *pos,BinEncoding char_encoding,
								BinEncoding offset_encoding,unichar c,int dest,
								BinType bin_type,unichar* output) {
bin_write(bin,char_encoding,c,pos);
if (bin_type==BIN_CLASSIC) {
	bin_write(bin,offset_encoding,dest,pos);
	return;
}
/* For .bin2, we have a bit to indicate whether there is an output or not */
dest=dest<<1;
if (output!=NULL && output[0]!='\0') {
	dest=dest|1;
}
bin_write(bin,offset_encoding,dest,pos);
if (dest & 1) {
	int i=-1;
	do {
		i++;
		bin_write(bin,char_encoding,output[i],pos);
	} while (output[i]!='\0');
}
}


int bin_get_value_variable_length(int v) {
if (v<(1<<7)) return 1;
if (v<(1<<14)) return 2;
if (v<(1<<21)) return 3;
if (v<(1<<28)) return 4;
return 5;
}


/**
 * Returns the number of bytes required to save the given value.
 */
int bin_get_value_length(int v,BinEncoding e) {
switch (e) {
case BIN_2BYTES: return 2;
case BIN_3BYTES: return 3;
case BIN_VARIABLE: return bin_get_value_variable_length(v);
default: fatal_error("bin_get_value_length: unsupported encoding\n");
}
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
	return 1;
}
if (d->bin[0]==1) {
	/* Type 2: modern style .bin/.inf dictionary with no limit on .bin size */
	d->type=BIN_CLASSIC;
	d->state_encoding=(BinStateEncoding)d->bin[1];
	d->inf_number_encoding=(BinEncoding)d->bin[2];
	d->char_encoding=(BinEncoding)d->bin[3];
	d->offset_encoding=(BinEncoding)d->bin[4];
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
bin[(*pos)++]=state_encoding;
bin[(*pos)++]=inf_number_encoding;
bin[(*pos)++]=char_encoding;
bin[(*pos)++]=offset_encoding;
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

