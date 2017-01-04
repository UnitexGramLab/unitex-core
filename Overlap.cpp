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

#include "Overlap.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Tests if a overlaps b.
 */
Overlap overlap(int a_start,int a_end,int b_start,int b_end) {
if (a_start==a_end && a_end==b_start) return B_INCLUDES_A;
if (a_end<=b_start) return A_BEFORE_B;
if (b_end<=a_start) return A_AFTER_B;
if (a_start<b_start && a_end>b_start && a_end<b_end) return A_BEFORE_B_OVERLAP;
if (a_start==b_start && a_end==b_end) return A_EQUALS_B;
if (a_start<=b_start && a_end>=b_end) return A_INCLUDES_B;
if (a_start>=b_start && a_end<=b_end) return B_INCLUDES_A;
if (a_start>b_start && a_start<b_end && a_end>b_end) return A_AFTER_B_OVERLAP;
fatal_error("Unexpected case in overlap");
return (Overlap)-1;
}

} // namespace unitex
