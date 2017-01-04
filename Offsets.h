/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

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
void process_offsets(const vector_offset* old_offsets, const vector_offset* new_offsets,
        U_FILE* f);
vector_offset* modified_offsets_to_common(const vector_offset* offsets, int old_size, int new_size);
vector_offset* common_offsets_to_modified(const vector_offset* offsets, int old_size, int new_size);
vector_offset* process_common_offsets(const vector_offset* old_offsets, const vector_offset* new_offsets);
vector_offset* process_offsets(const vector_offset* first_offsets, const vector_offset* second_offsets);

int save_snt_offsets(vector_int*,const char*);
vector_int* load_snt_offsets(const char*);
void add_snt_offsets(vector_int*,int,int,int);


void save_offsets(U_FILE* f, const vector_offset* offsets);
int save_offsets(const VersatileEncodingConfig* vec, const char* filename, const vector_offset* offsets);

// these function prevent direct access to vector_int componenent of vector_uima_offset
typedef vector_int vector_uima_offset;
vector_uima_offset* load_uima_offsets(const VersatileEncodingConfig*,const char* name);

inline int uima_offset_token_start_pos(const vector_uima_offset* v,int token) {
    return ((const vector_int*) v)->tab[token * 2];
}

inline int uima_offset_token_end_pos(const vector_uima_offset* v, int token) {
    return ((const vector_int*) v)->tab[(token * 2)+1];
}

inline int uima_offset_tokens_count(const vector_uima_offset* v) {
    return (((const vector_int*)v)->nbelems) / 2;
}

inline void free_uima_offsets(vector_uima_offset* v) {
    free_vector_int((vector_int*)v);
}
typedef struct {
    int position_to_translate; // in
    int sort_order; // in
    int translated_position; // out
    int translation_pos_in_common; // out
} offset_translation;

void translate_offset(offset_translation* ofs, int nb_translations, const vector_offset* offsets, int revert);

} // namespace unitex


#endif
