 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "utils.h"

typedef struct vector_t {
  int     nbelems;
  void ** tab;
  int     size;
} vector_t;


inline vector_t * vector_new(int size = 16) {
  vector_t * vec = (vector_t *) xmalloc(sizeof(vector_t));
  if (size <= 0) { size = 1; }
  vec->tab = (void **) xmalloc(size * sizeof(void *));
  vec->size = size;
  vec->nbelems = 0;
  return vec;
}


typedef void (*release_f) (void *);

inline void vector_delete(vector_t * vec, release_f release = NULL) {
  if (release) { for (int i = 0; i< vec->nbelems; i++) { release(vec->tab[i]); } }
  free(vec->tab);
  free(vec);
}


inline void vector_resize(vector_t * vec, int size) {
  if (size <= 0) { size = 1; }
  if (size < vec->nbelems) {
    fatal_error("vec_resize: size=%d && nbelems=%d\n", size, vec->nbelems);
  }
  vec->tab = (void **) xrealloc(vec->tab, size * sizeof(void *));
  vec->size = size;
}


inline int vector_add(vector_t * vec, void * data) {

  while (vec->nbelems >= vec->size) { vector_resize(vec, vec->size * 2); }

  vec->tab[vec->nbelems++] = data;

  return vec->nbelems - 1;
}


#endif
