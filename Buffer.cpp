 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdlib.h>
#include "Buffer.h"
#include "Error.h"


/**
 * Allocates initializes and returns a new buffer of the given capacity.
 * Its size is initialized to 0.
 */
struct buffer* new_buffer(int capacity) {
struct buffer* buffer=(struct buffer*)malloc(sizeof(struct buffer));
if (buffer==NULL) {
	fatal_error("Not enough memory in new_buffer\n");
}
buffer->buffer=(int*)malloc(sizeof(int)*capacity);
if (buffer->buffer==NULL) {
	fatal_error("Not enough memory in new_buffer\n");
}
buffer->MAXIMUM_BUFFER_SIZE=capacity;
buffer->size=0;
return buffer;
}


/**
 * Frees a buffer, assuming that the field 'buffer' was not already freed.
 */
void free_buffer(struct buffer* buffer) {
if (buffer==NULL) return;
free(buffer->buffer);
free(buffer);
}

