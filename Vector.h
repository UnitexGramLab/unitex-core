 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "Utils.h"


/**
 * This library provides implementations of autoresizable arrays:
 * 
 * - vector_ptr: data type=void*
 * - vector_int: data type=int
 */


typedef struct vector_ptr {
  int nbelems;
  void** tab;
  int size;
} vector_ptr;


typedef struct vector_int {
  int nbelems;
  int* tab;
  int size;
} vector_int;


inline vector_ptr* new_vector_ptr(int size=16) {
vector_ptr* vec=(vector_ptr*)xmalloc(sizeof(vector_ptr));
if (size<=0) {
   size=1;
}
vec->tab=(void**)xmalloc(size*sizeof(void*));
vec->size=size;
vec->nbelems=0;
return vec;
}


typedef void (*release_f) (void *);

inline void free_vector_ptr(vector_ptr* vec,release_f release=NULL) {
if (vec==NULL) return;
if (release!=NULL) {
   for (int i=0;i<vec->nbelems;i++) {
      release(vec->tab[i]);
   }
}
free(vec->tab);
free(vec);
}


inline void vector_ptr_resize(vector_ptr* vec,int size) {
if (size<=0) {
   size=1;
}
if (size<=vec->nbelems) {
   fatal_error("vector_ptr_resize: size=%d && nbelems=%d\n",size,vec->nbelems);
}
vec->tab=(void**)xrealloc(vec->tab,size*sizeof(void*));
vec->size=size;
}


inline int vector_ptr_add(vector_ptr* vec,void* data) {
while (vec->nbelems>=vec->size) {
   vector_ptr_resize(vec,vec->size*2);
}
vec->tab[vec->nbelems++]=data;
return vec->nbelems-1;
}


inline vector_int* new_vector_int(int size=16) {
vector_int* vec=(vector_int*)xmalloc(sizeof(vector_int));
if (size<=0) {
   size=1;
}
vec->tab=(int*)xmalloc(size*sizeof(int));
vec->size=size;
vec->nbelems=0;
return vec;
}


inline void free_vector_int(vector_int* vec) {
if (vec==NULL) return;
free(vec->tab);
free(vec);
}


inline void vector_int_resize(vector_int* vec,int size) {
if (size<=0) {
   size=1;
}
if (size<vec->nbelems) {
   fatal_error("vector_int_resize: size=%d && nbelems=%d\n",size,vec->nbelems);
}
vec->tab=(int*)xrealloc(vec->tab,size*sizeof(int));
vec->size=size;
}


inline int vector_int_add(vector_int* vec,int data) {
while (vec->nbelems>=vec->size) {
   vector_int_resize(vec,vec->size*2);
}
vec->tab[vec->nbelems++]=data;
return vec->nbelems-1;
}

#endif
