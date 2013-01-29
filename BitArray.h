/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef BitArrayH
#define BitArrayH

#include "AbstractAllocator.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This enumeration describes the valid lengths for an information
 * in a bit array.
 */
enum length_ {
   ONE_BIT=1,
   TWO_BITS=2,
   FOUR_BITS=4
};
typedef enum length_ InfoLength;


/**
 * This structure represents a bit array. This can be used for encoding 1-bit, 2-bits 
 * or 4-bits information without wasting space. Its is defined by the number of 
 * elements to be encoded, the actual size in bytes of the array and the length (1, 2 or 4 bits) 
 * of one information. 'divider' is the divider that corresponds to the length. Its possible
 * values are respectively 8, 4 and 2.
 */
struct bit_array {
	int size_in_elements;
   int size_in_bytes;
	unsigned char* array;
   InfoLength info_length;
   int divider;
};


struct bit_array* new_bit_array(int,InfoLength,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_bit_array(struct bit_array*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void set_value(struct bit_array*,int,int);
int get_value(const struct bit_array*,int);

} // namespace unitex

#endif
