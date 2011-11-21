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

#ifndef OffsetsH
#define OffsetsH

#include <stdio.h>
#include <stdlib.h>
#include "Error.h"
#include "Unicode.h"
#include "Vector.h"


/**
 * This library is used to manipulate offset information so that
 * it is possible to synchronize offsets in a concordance with
 * the original .txt file, before being modified by any Unitex operation.
 * An offset file is made of lines containing 4 numbers A B C D, representing
 * positions in characters.
 *
 * [A;B[ is a sequence in the text file before a modification and [C;D[ is
 * the corresponding sequence in the text file after the modification.
 *
 * For instance if the text "I   like you" is normalized into "I like you",
 * we will have the following line:
 *
 * 1 4 1 2
 *
 * meaning that [1;2[ in the normalized text file is mapped to [1;4[ in the
 * original one.
 */

typedef struct {
	int old_start;
	int old_end;
	int new_start;
	int new_end;
} Offsets;


typedef struct {
  int nbelems;
  Offsets* tab;
  int size;
} vector_offset;


inline vector_offset* new_vector_offset(int size=16) {
vector_offset* vec=(vector_offset*)malloc(sizeof(vector_offset));
if (vec==NULL) {
   fatal_alloc_error("new_vector_offset");
}
if (size<=0) {
   size=1;
}
vec->tab=(Offsets*)malloc(size*sizeof(Offsets));
if (vec==NULL) {
   fatal_alloc_error("new_vector_offset");
}
vec->size=size;
vec->nbelems=0;
return vec;
}


inline void free_vector_offset(vector_offset* vec) {
if (vec==NULL) return;
free(vec->tab);
free(vec);
}


inline void vector_offset_resize(vector_offset* vec,int size) {
if (size<=0) {
   size=1;
}
if (size<vec->nbelems) {
   fatal_error("vector_offset_resize: size=%d && nbelems=%d\n",size,vec->nbelems);
}
vec->tab=(Offsets*)realloc(vec->tab,size*sizeof(Offsets));
if (vec->tab==NULL) {
   fatal_alloc_error("vector_offset_resize");
}
vec->size=size;
}


inline int vector_offset_add(vector_offset* vec,int a,int b,int c,int d) {
while (vec->nbelems>=vec->size) {
   vector_offset_resize(vec,vec->size*2);
}
vec->tab[vec->nbelems].old_start=a;
vec->tab[vec->nbelems].old_end=b;
vec->tab[vec->nbelems].new_start=c;
vec->tab[vec->nbelems].new_end=d;
vec->nbelems++;
return vec->nbelems-1;
}


vector_offset* load_offsets(const VersatileEncodingConfig*,const char*);
void process_offsets(vector_offset* old_offsets, vector_offset* new_offsets,
		U_FILE* f);

int save_snt_offsets(vector_int*,const char*);
vector_int* load_snt_offsets(char*);
void add_snt_offsets(vector_int*,int,int,int);

vector_int* load_uima_offsets(const VersatileEncodingConfig*,const char* name);


#endif
