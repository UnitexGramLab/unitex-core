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

#ifndef VectorH
#define VectorH

#include <stdlib.h>
#include "AbstractAllocator.h"
#include "Error.h"

/**
 * This library provides implementations of autoresizable arrays:
 * 
 * - vector_ptr:    data type=void*
 * - vector_int:    data type=int
 * - vector_float:  data type=float
 * - vector_double: data type=double
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


typedef struct vector_double {
  int nbelems;
  double* tab;
  int size;
} vector_double;


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


inline vector_int* new_vector_int(int size=16,Abstract_allocator prv_alloc=NULL) {
vector_int* vec=(vector_int*)malloc_cb(sizeof(vector_int),prv_alloc);
if (vec==NULL) {
   fatal_alloc_error("new_vector_int");
}
if (size<=0) {
   size=1;
}
vec->tab=(int*)malloc_cb(size*sizeof(int),prv_alloc);
if (vec==NULL) {
   fatal_alloc_error("new_vector_int");
}
vec->size=size;
vec->nbelems=0;
return vec;
}


inline void free_vector_int(vector_int* vec,Abstract_allocator prv_alloc=NULL) {
if (vec==NULL) return;
free_cb(vec->tab,prv_alloc);
free_cb(vec,prv_alloc);
}


inline void vector_int_resize(vector_int* vec,int size,Abstract_allocator prv_alloc=NULL) {
if (size<=0) {
   size=1;
}
if (size<vec->nbelems) {
   fatal_error("vector_int_resize: size=%d && nbelems=%d\n",size,vec->nbelems);
}
vec->tab=(int*)realloc_cb(vec->tab,vec->size*sizeof(int),size*sizeof(int),prv_alloc);
if (vec->tab==NULL) {
   fatal_alloc_error("vector_int_resize");
}
vec->size=size;
}


inline int vector_int_add(vector_int* vec,int data,Abstract_allocator prv_alloc=NULL) {
while (vec->nbelems>=vec->size) {
   vector_int_resize(vec,vec->size*2,prv_alloc);
}
vec->tab[vec->nbelems++]=data;
return vec->nbelems-1;
}


inline void vector_int_copy(vector_int* dst,vector_int* src,Abstract_allocator prv_alloc=NULL) {
if (dst==NULL) return;
dst->nbelems=0;
if (src==NULL) return;
for (int i=0;i<src->nbelems;i++) {
	vector_int_add(dst,src->tab[i],prv_alloc);
}
}


inline vector_int* vector_int_dup(vector_int* src,Abstract_allocator prv_alloc=NULL) {
if (src==NULL) return NULL;
vector_int* dst=new_vector_int(src->nbelems,prv_alloc);
for (int i=0;i<src->nbelems;i++) {
	vector_int_add(dst,src->tab[i],prv_alloc);
}
return dst;
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


inline vector_double* new_vector_double(int size=16) {
vector_double* vec=(vector_double*)malloc(sizeof(vector_double));
if (vec==NULL) {
   fatal_alloc_error("new_vector_double");
}
if (size<=0) {
   size=1;
}
vec->tab=(double*)malloc(size*sizeof(double));
if (vec==NULL) {
   fatal_alloc_error("new_vector_double");
}
vec->size=size;
vec->nbelems=0;
return vec;
}


inline void free_vector_double(vector_double* vec) {
if (vec==NULL) return;
free(vec->tab);
free(vec);
}


inline void vector_double_resize(vector_double* vec,int size) {
if (size<=0) {
   size=1;
}
if (size<vec->nbelems) {
   fatal_error("vector_double_resize: size=%d && nbelems=%d\n",size,vec->nbelems);
}
vec->tab=(double*)realloc(vec->tab,size*sizeof(double));
if (vec->tab==NULL) {
   fatal_alloc_error("vector_double_resize");
}
vec->size=size;
}


inline int vector_double_add(vector_double* vec,double data) {
while (vec->nbelems>=vec->size) {
   vector_double_resize(vec,vec->size*2);
}
vec->tab[vec->nbelems++]=data;
return vec->nbelems-1;
}


inline int vector_int_contains(vector_int* v,int n) {
for (int i=0;i<v->nbelems;i++) {
	if (v->tab[i]==n) return i;
}
return -1;
}


inline int vector_float_contains(vector_float* v,float n) {
for (int i=0;i<v->nbelems;i++) {
	if (v->tab[i]==n) return i;
}
return -1;
}


inline int vector_double_contains(vector_double* v,double n) {
for (int i=0;i<v->nbelems;i++) {
	if (v->tab[i]==n) return i;
}
return -1;
}


inline int vector_ptr_contains(vector_ptr* v,void* n) {
for (int i=0;i<v->nbelems;i++) {
	if (v->tab[i]==n) return i;
}
return -1;
}


inline void vector_int_add_if_absent(vector_int* vec,int data) {
if (-1==vector_int_contains(vec,data)) {
	vector_int_add(vec,data);
}
}


inline void vector_float_add_if_absent(vector_float* vec,float data) {
if (-1==vector_float_contains(vec,data)) {
	vector_float_add(vec,data);
}
}


inline void vector_double_add_if_absent(vector_double* vec,double data) {
if (-1==vector_double_contains(vec,data)) {
	vector_double_add(vec,data);
}
}


inline void vector_ptr_add_if_absent(vector_ptr* vec,void* data) {
if (-1==vector_ptr_contains(vec,data)) {
	vector_ptr_add(vec,data);
}
}


/**
 * Returns 1 if the element was actually removed; 0 if it was absent.
 */
inline int vector_int_remove(vector_int* vec,int data) {
int n=vector_int_contains(vec,data);
if (n==-1) return 0;
vec->tab[n]=vec->tab[vec->nbelems-1];
(vec->nbelems)--;
return 1;
}


inline int vector_float_remove(vector_float* vec,float data) {
int n=vector_float_contains(vec,data);
if (n==-1) return 0;
vec->tab[n]=vec->tab[vec->nbelems-1];
(vec->nbelems)--;
return 1;
}


inline int vector_double_remove(vector_double* vec,double data) {
int n=vector_double_contains(vec,data);
if (n==-1) return 0;
vec->tab[n]=vec->tab[vec->nbelems-1];
(vec->nbelems)--;
return 1;
}


inline int vector_ptr_remove(vector_ptr* vec,void* data) {
int n=vector_ptr_contains(vec,data);
if (n==-1) return 0;
vec->tab[n]=vec->tab[vec->nbelems-1];
(vec->nbelems)--;
return 1;
}


/**
 * Returns 1 if a and b contain the same elements in the same order; 0 otherwise.
 */
inline int vector_int_equals(vector_int* a,vector_int* b) {
if (a->nbelems!=b->nbelems) return 0;
for (int i=0;i<a->nbelems;i++) {
	if (a->tab[i]!=b->tab[i]) return 0;
}
return 1;
}


/**
 * Returns 1 if a and b contain the same elements in the same order; 0 otherwise.
 */
inline int vector_float_equals(vector_float* a,vector_float* b) {
if (a->nbelems!=b->nbelems) return 0;
for (int i=0;i<a->nbelems;i++) {
	if (a->tab[i]!=b->tab[i]) return 0;
}
return 1;
}


/**
 * Returns 1 if a and b contain the same elements in the same order; 0 otherwise.
 */
inline int vector_double_equals(vector_double* a,vector_double* b) {
if (a->nbelems!=b->nbelems) return 0;
for (int i=0;i<a->nbelems;i++) {
	if (a->tab[i]!=b->tab[i]) return 0;
}
return 1;
}


/**
 * Returns 1 if a and b contain the same elements in the same order; 0 otherwise.
 */
inline int vector_ptr_equals(vector_ptr* a,vector_ptr* b) {
if (a->nbelems!=b->nbelems) return 0;
for (int i=0;i<a->nbelems;i++) {
	if (a->tab[i]!=b->tab[i]) return 0;
}
return 1;
}


static int cmp_int(const void* a,const void* b) {
int* x=(int*)a;
int* y=(int*)b;
return *y-*x;
}

inline void vector_int_sort(vector_int* v) {
qsort(v->tab,v->nbelems,sizeof(int),cmp_int);
}


inline int vector_int_equals_disorder(vector_int* a,vector_int* b) {
vector_int* x=vector_int_dup(a);
vector_int* y=vector_int_dup(b);
vector_int_sort(x);
vector_int_sort(y);
int ret=vector_int_equals(x,y);
free_vector_int(x);
free_vector_int(y);
return ret;
}

#endif
