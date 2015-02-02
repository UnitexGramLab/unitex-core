/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "BitMasks.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Tests if the given bit mask is set on the given byte.
 */
int is_bit_mask_set(unsigned char value,unsigned char mask) {
return value & mask;
}


/**
 * Set the given mask on the given value.
 */
void set_bit_mask(unsigned char *value,unsigned char mask) {
unset_bit_mask(value,mask);
(*value)=(*value) | mask;
}


/**
 * Unset the given mask on the given value.
 */
void unset_bit_mask(unsigned char *value,unsigned char mask) {
(*value)=(*value) & (0xFF-mask);
}

} // namespace unitex
