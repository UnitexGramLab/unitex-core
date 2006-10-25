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

#include <stdio.h>
#include <stdlib.h>
#include "Buffer.h"
#include "Error.h"


/**
 * Allocates, initializes and returns a new buffer of the given capacity.
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
buffer->end_of_file=0;
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


/**
 * This function fill the given buffer from the given file. The 'pos'
 * parameter indicates where the current position in the buffer is. The
 * remaining values from 'pos' to the end of the buffer will be
 * moved at the beginning of the buffer, and then, data read from the input
 * file will be appended. If no data can be read, the flag 'buffer->end_of_file'
 * is set to 1.
 * 
 * buffer before:
 * 0                                                  pos          buffer->MAXIMUM_BUFFER_SIZE
 * ------------------------------------------------------------------------------
 * |                                                   |                        |
 * |                                                   | XXXXXXXXXXXXXXXXXXXXXX |
 * |                                                   |                        |
 * ------------------------------------------------------------------------------
 *
 * 
 * buffer after:
 * 0         buffer->MAXIMUM_BUFFER_SIZE-pos                       buffer->MAXIMUM_BUFFER_SIZE
 * ------------------------------------------------------------------------------
 * |                        |                                                   |
 * | XXXXXXXXXXXXXXXXXXXXXX |            new data read from input file          |
 * |                        |                                                   |
 * ------------------------------------------------------------------------------
 */
void fill_buffer(struct buffer* buffer,int pos,FILE* f) {
int* array=buffer->buffer;
for (int i=pos;i<buffer->MAXIMUM_BUFFER_SIZE;i++) {
  // first, we copy the end of the buffer at the beginning
  array[i-pos]=array[i];
}
int new_position=buffer->MAXIMUM_BUFFER_SIZE-pos;
int n_int_read=fread(&(array[new_position]),sizeof(int),pos,f);
buffer->end_of_file=(n_int_read==0);
buffer->size=new_position+n_int_read;
}


/**
 * This function fills the given buffer from the given file.
 */
void fill_buffer(struct buffer* buffer,FILE* f) {
fill_buffer(buffer,buffer->MAXIMUM_BUFFER_SIZE,f);
}

