/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "BitArray.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates, initializes and returns a new bit array of the given number
 * of elements. 'info_length' corresponds to the length in bits of one
 * information.
 */
struct bit_array* new_bit_array(int number_of_elements,InfoLength info_length,Abstract_allocator prv_alloc) {
struct bit_array* bit_array=(struct bit_array*)malloc_cb(sizeof(struct bit_array),prv_alloc);
if (bit_array==NULL) {
    fatal_alloc_error("new_bit_array");
}
if (number_of_elements<0) {
   fatal_error("Invalid number of elements (%d) in new_bit_array\n",number_of_elements);
}
bit_array->size_in_elements=number_of_elements;
switch(info_length) {
   case ONE_BIT: bit_array->divider=8; break;
   case TWO_BITS: bit_array->divider=4; break;
   case FOUR_BITS: bit_array->divider=2; break;
};
bit_array->size_in_bytes=(number_of_elements/bit_array->divider)+1;
bit_array->array=(unsigned char*)malloc_cb(sizeof(unsigned char)*(bit_array->size_in_bytes),prv_alloc);
if (bit_array->array==NULL) {
    fatal_alloc_error("new_bit_array");
}
memset(bit_array->array,0,sizeof(unsigned char)*(bit_array->size_in_bytes));
bit_array->info_length=info_length;
return bit_array;
}


/**
 * Frees a bit array, assuming that the field 'array' was not already freed.
 */
void free_bit_array(struct bit_array* bit_array,Abstract_allocator prv_alloc) {
if (bit_array==NULL) return;
free_cb(bit_array->array,prv_alloc);
free_cb(bit_array,prv_alloc);
}


/**
 * Associates 'value' to the 'n'th element of the given bit array.
 * A fatal error will be raised if 'n' is not in the bounds
 * of the array or if 'value' is not compatible with the
 * information length of the array.
 */
void set_value(struct bit_array* array,int n,int value) {
if (array==NULL) {
   fatal_error("NULL error in set_value\n");
}
if (n<0 || n>=array->size_in_elements) {
   fatal_error("Element (%d) out of bounds in set_value\n",n);
}
int length=(int)array->info_length;
int divider=array->divider;
if (value<0 || value>=(1<<length)) {
   fatal_error("Invalid value (%d) in set_value; should be in [0;%d]\n",value,(1<<length)-1);
}
int cleaning_mask=0xFF-(((1<<length)-1)<<((n%divider)*length));
int value_mask=value<<((n%divider)*length);
/* First, we clean the previous information */
array->array[n/divider]=array->array[n/divider] & cleaning_mask;
/* And then we set the new value */
array->array[n/divider]=(unsigned char)(array->array[n/divider] | value_mask);
}


/**
 * Returns the value associated to the 'n'th element of the bit array, or raises
 * a fatal error if 'n' is not in the bounds of the array.
 */
int get_value(const struct bit_array* array,int n) {
if (array==NULL) {
   fatal_error("NULL error in get_value\n");
}
if (n<0 || n>=array->size_in_elements) {
   fatal_error("Element (%d) out of bounds in get_value\n",n);
}
int length=(int)array->info_length;
int divider=array->divider;
int mask=((1<<length)-1)<<((n%divider)*length);
int unshifted_value=array->array[n/array->divider] & mask;
return unshifted_value>>((n%divider)*length);
}

} // namespace unitex
