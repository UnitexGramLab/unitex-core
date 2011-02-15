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

static int read_bin_header(Dictionary*);

/**
 * Loads and returns a compressed dictionary.
 */
Dictionary* new_Dictionary(const char* bin,const char* inf,Abstract_allocator prv_alloc) {
Dictionary* d=(Dictionary*)malloc_cb(sizeof(Dictionary),prv_alloc);
if (d==NULL) {
	fatal_alloc_error("new_Dictionary");
}
d->bin=load_BIN_file(bin,&d->bin_size,prv_alloc);
if (d->bin==NULL) {
	free(d);
	return NULL;
}
if (!read_bin_header(d)) {
	free_BIN_file(d->bin,prv_alloc);
	free(d);
	return NULL;
}
if (d->type==BIN_CLASSIC) {
	if (inf==NULL) {
		error("NULL .inf file in new_Dictionary\n");
		free_BIN_file(d->bin,prv_alloc);
		free(d);
		return NULL;
	}
	d->inf=load_INF_file(inf,prv_alloc);
	if (d->inf==NULL) {
		free_BIN_file(d->bin,prv_alloc);
		free(d);
		return NULL;
	}
} else {
	fatal_error("new_Dictionary: unsupported dictionary format\n");
}
return d;
}


/**
 * Frees all resources associated to the given dictionary
 */
void free_Dictionary(Dictionary* d,Abstract_allocator prv_alloc) {
if (d==NULL) return;
if (d->bin!=NULL) free_BIN_file(d->bin,prv_alloc);
if (d->inf!=NULL) {
	free_INF_codes(d->inf,prv_alloc);
}
free(d);
}



/**
 * Loads a .bin file into an unsigned char array that is returned.
 * Returns NULL if an error occurs.
 */
unsigned char* load_BIN_file(const char* name,long *file_size,Abstract_allocator prv_alloc) {
U_FILE* f;
/* We open the file as a binary one */
f=u_fopen(BINARY,name,U_READ);
unsigned char* tab;
if (f==NULL) {
   error("Cannot open %s\n",name);
   return NULL;
}
*file_size=get_file_size(name);
if (*file_size==0) {
   error("Error: empty file %s\n",name);
   u_fclose(f);
   return NULL;
}
tab=(unsigned char*)malloc_cb(sizeof(unsigned char)*(*file_size),prv_alloc);
if (tab==NULL) {
   fatal_alloc_error("load_BIN_file");
   return NULL;
}
if (*file_size!=(int)fread(tab,sizeof(char),*file_size,f)) {
   error("Error while reading %s\n",name);
   free_cb(tab,prv_alloc);
   u_fclose(f);
   return NULL;
}
u_fclose(f);
return tab;
}


/**
 * Frees all the memory allocated for the given structure.
 */
void free_BIN_file(unsigned char* BIN,Abstract_allocator prv_alloc) {
free_cb(BIN,prv_alloc);
}


/**
 * This function assumes that the .bin file has already been loaded.
 * It analyzes its header to determine the kind of .bin it is.
 * Returns 0 in case of error; 1 otherwise.
 */
static int read_bin_header(Dictionary* d) {
if (d->bin[0]==0) {
	/* Type 1: old style .bin/.inf dictionary */
	d->header_size=4;
	if (d->bin_size<d->header_size+2) {
		/* The minimal empty dictionary is made of the header plus a 2 bytes empty state */
		error("Invalid .bin size\n");
		return 0;
	}
	/* If we have an old style .bin */
	d->type=BIN_CLASSIC;
	d->inf_number_encoding=BIN_3BYTES;
	d->char_encoding=BIN_2BYTES;
	d->offset_encoding=BIN_3BYTES;
	return 1;
}
if (d->bin[0]==1) {
	/* Type 2: modern style .bin/.inf dictionary with no limit on .bin size */
	d->header_size=4;
	if (d->bin_size<d->header_size+2) {
		/* The minimal empty dictionary is made of the header plus a 2 bytes empty state */
		error("Invalid .bin size\n");
		return 0;
	}
	d->type=BIN_CLASSIC;
	d->inf_number_encoding=(BinEncoding)d->bin[1];
	d->char_encoding=(BinEncoding)d->bin[2];
	d->offset_encoding=(BinEncoding)d->bin[3];
	return 1;
}
error("Unknown dictionary type: %d\n",d->bin[0]);
return 0;
}


/**
 * Reads a 2 byte-value. Updates the offset.
 */
int bin_read_2bytes(unsigned char* bin,int *offset) {
int v=(bin[*offset]<<8)+bin[(*offset)+1];
(*offset)+=2;
return v;
}


/**
 * Reads a 3 byte-value. Updates the offset.
 */
int bin_read_3bytes(unsigned char* bin,int *offset) {
int v=(bin[*offset]<<16)+(bin[(*offset)+1]<<8)+bin[(*offset)+2];
(*offset)+=3;
return v;
}


/**
 * Reads a variable length value. Updates the offset.
 */
int bin_read_variable_length(unsigned char* bin,int *offset) {
int v=0;
do {
	v=(v<<7) | (bin[(*offset)++] & 127);
} while (bin[(*offset)-1] & 128);
return v;
}


int bin_read(unsigned char* bin,BinEncoding e,int *offset) {
switch (e) {
case BIN_2BYTES: return bin_read_2bytes(bin,offset);
case BIN_3BYTES: return bin_read_3bytes(bin,offset);
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
 * Writes a variable length value. Updates the offset.
 */
void bin_write_variable_length(unsigned char* bin,int value,int *offset) {
if (value<(1<<7)) {
	bin[(*offset)++]=value;
	return;
}
if (value<(1<<14)) {
	bin[(*offset)++]=(value>>7)+128;
	bin[(*offset)++]=(value & 127);
	return;
}
if (value<(1<<21)) {
	bin[(*offset)++]=(value>>14)+128;
	bin[(*offset)++]=((value>>7) & 127)+128;
	bin[(*offset)++]=(value & 127);
	return;
}
if (value<(1<<28)) {
	bin[(*offset)++]=(value>>21)+128;
	bin[(*offset)++]=((value>>14) & 127)+128;
	bin[(*offset)++]=((value>>7) & 127)+128;
	bin[(*offset)++]=(value & 127);
	return;
}
bin[(*offset)++]=(value>>28)+128;
bin[(*offset)++]=((value>>21) & 127)+128;
bin[(*offset)++]=((value>>14) & 127)+128;
bin[(*offset)++]=((value>>7) & 127)+128;
bin[(*offset)++]=(value & 127);
}


void bin_write(unsigned char* bin,BinEncoding e,int value,int *offset) {
switch (e) {
case BIN_2BYTES: return bin_write_2bytes(bin,value,offset);
case BIN_3BYTES: return bin_write_3bytes(bin,value,offset);
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
if (d->type!=BIN_CLASSIC) {
	fatal_error("read_dictionary_state: unsupported dictionary type\n");
}
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


/**
 * Reads the information associated to the current transition in the dictionary.
 * Returns the new position.
 */
int read_dictionary_transition(Dictionary* d,int pos,unichar *c,int *dest) {
if (d->type!=BIN_CLASSIC) {
	fatal_error("read_dictionary_state: unsupported dictionary type\n");
}
*c=bin_read(d->bin,d->char_encoding,&pos);
*dest=bin_read(d->bin,d->offset_encoding,&pos);
return pos;
}

