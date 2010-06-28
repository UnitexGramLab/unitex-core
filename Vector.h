/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef VectorH
#define VectorH

#include <stdlib.h>
#include "Error.h"

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


typedef struct vector_float {
  int nbelems;
  float* tab;
  int size;
} vector_float;


inline vector_ptr* new_vector_ptr(int size=16) {
vector_ptr* vec=(vector_ptr*)malloc(sizeof(vector_ptr));
if (vec==NULL) {
   fatal_alloc_error("new_vector_ptr");
}
if (size<=0) {
   size=1;
}
vec->tab=(void**)malloc(size*sizeof(void*));
if (vec->tab==NULL) {
   fatal_alloc_error("new_vector_ptr");
}
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
vec->tab=(void**)realloc(vec->tab,size*sizeof(void*));
if (vec->tab==NULL) {
   fatal_alloc_error("vector_ptr_resize");
}
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
vector_int* vec=(vector_int*)malloc(sizeof(vector_int));
if (vec==NULL) {
   fatal_alloc_error("new_vector_int");
}
if (size<=0) {
   size=1;
}
vec->tab=(int*)malloc(size*sizeof(int));
if (vec==NULL) {
   fatal_alloc_error("new_vector_int");
}
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
vec->tab=(int*)realloc(vec->tab,size*sizeof(int));
if (vec->tab==NULL) {
   fatal_alloc_error("vector_int_resize");
}
vec->size=size;
}


inline int vector_int_add(vector_int* vec,int data) {
while (vec->nbelems>=vec->size) {
   vector_int_resize(vec,vec->size*2);
}
vec->tab[vec->nbelems++]=data;
return vec->nbelems-1;
}


inline vector_float* new_vector_float(int size=16) {
vector_float* vec=(vector_float*)malloc(sizeof(vector_float));
if (vec==NULL) {
   fatal_alloc_error("new_vector_float");
}
if (size<=0) {
   size=1;
}
vec->tab=(float*)malloc(size*sizeof(float));
if (vec==NULL) {
   fatal_alloc_error("new_vector_float");
}
vec->size=size;
vec->nbelems=0;
return vec;
}


inline void free_vector_float(vector_float* vec) {
if (vec==NULL) return;
free(vec->tab);
free(vec);
}


inline void vector_float_resize(vector_float* vec,int size) {
if (size<=0) {
   size=1;
}
if (size<vec->nbelems) {
   fatal_error("vector_float_resize: size=%d && nbelems=%d\n",size,vec->nbelems);
}
vec->tab=(float*)realloc(vec->tab,size*sizeof(float));
if (vec->tab==NULL) {
   fatal_alloc_error("vector_float_resize");
}
vec->size=size;
}


inline int vector_float_add(vector_float* vec,float data) {
while (vec->nbelems>=vec->size) {
   vector_float_resize(vec,vec->size*2);
}
vec->tab[vec->nbelems++]=data;
return vec->nbelems-1;
}

#endif
