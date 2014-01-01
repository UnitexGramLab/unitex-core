/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef BufferH
#define BufferH

#include "Unicode.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This enumeration describes the possible kind of buffers.
 */
enum buffer_type_ {
   INTEGER_BUFFER,
   UNICHAR_BUFFER
};
typedef enum buffer_type_ BufferType;

/**
 * This structure represents a buffer of integer or unichar, with its maximum capacity and
 * its actual size. The 'end_of_file' field is set to 1 when no data can be
 * read from the input file.
 */
struct buffer {
   BufferType type;
	int MAXIMUM_BUFFER_SIZE;
   union {
	   int* int_buffer;
      unichar* unichar_buffer;
   };
	int size;
   int end_of_file;
};


struct buffer* new_buffer(int,BufferType);
struct buffer* new_buffer_for_file(BufferType,U_FILE*,int capacity_limit);
void free_buffer(struct buffer*);
int fill_buffer(struct buffer*,int,U_FILE*);
int fill_buffer(struct buffer*,U_FILE*);

} // namespace unitex

#endif

