 /*
  * Unitex
  *
  * Copyright (C) 2001-2004 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "qsort.h"



static inline void swap(char * a, char * b, int size) {
  char tmp;
  while (size--) {
    tmp = *a; *a = *b; *b = tmp;
    a++; b++;
  }
}



void qsort(void * tab, int nb, int size,
           int (*compare)(void * a, void * b, void * data), void * data) {

  stack_type * stack = stack_new();

  char * lo = (char *) tab;
  char * mid;
  char * hi = ((char *) tab) + (nb - 1) * size;

  char* pivot=(char*)malloc(sizeof(char)*size);


  stack_push(stack, (void *) lo);
  stack_push(stack, (void *) hi);


  while (! stack_empty(stack)) {

    hi = (char *) stack_pop(stack);
    lo = (char *) stack_pop(stack);

    if (lo >= hi) { continue; }

    mid = lo + size * (((hi - lo) / size) / 2);

    if (compare(mid, lo, data) < 0) {
      swap(mid, lo, size);
    }

    if (compare(hi, mid, data) < 0) {

      swap(hi, mid, size);

      if (compare(mid, lo, data) < 0) {
	swap(mid, lo, size);
      }
    }

    memcpy(pivot, mid, size);

    char * left  = lo + size;
    char * right = hi - size;

    while (left <= right) {

      while (compare(left, pivot, data)  < 0) { left  = left  + size; }
      while (compare(pivot, right, data) < 0) { right = right - size; }
      if (left <= right) {

	if (left != right) { swap(left, right, size); }

	left  = left  + size;
	right = right - size;
      }
    }

    stack_push(stack, lo);   stack_push(stack, right);
    stack_push(stack, left); stack_push(stack, hi);
  }
free(pivot);
}
