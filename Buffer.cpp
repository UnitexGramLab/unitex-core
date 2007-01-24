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

#include <stdio.h>
#include <stdlib.h>
#include "Buffer.h"
#include "Error.h"


/**
 * Allocates, initializes and returns a new buffer of the given capacity.
 * Its size is initialized to 0. Its type (integer or unichar) is specified by
 * the 'type' parameter.
 */
struct buffer* new_buffer(int capacity,BufferType type) {
struct buffer* buffer=(struct buffer*)malloc(sizeof(struct buffer));
if (buffer==NULL) {
	fatal_error("Not enough memory in new_buffer\n");
}
buffer->type=type;
switch (type) {
   case INTEGER_BUFFER: 
      buffer->int_buffer=(int*)malloc(sizeof(int)*capacity);
      if (buffer->int_buffer==NULL) {
         fatal_error("Not enough memory in new_buffer\n");
      }
      break;
   case UNICHAR_BUFFER: 
      /* In case of a unichar buffer, we add 1 to the size in order to store a \0,
       * even if the buffer is full. This precaution is useful in order to
       * do string parsing in a unichar buffer, avoiding the risk of an out of
       * bounds error */
      buffer->unichar_buffer=(unichar*)malloc(sizeof(unichar)*(capacity+1));
      if (buffer->unichar_buffer==NULL) {
         fatal_error("Not enough memory in new_buffer\n");
      }
      break; /* Useless, except if we add something in the future... */
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
switch (buffer->type) {
   case INTEGER_BUFFER: free(buffer->int_buffer); break;
   case UNICHAR_BUFFER: free(buffer->unichar_buffer); break;
}
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
int new_position;
int n_element_read;
switch (buffer->type) {
   case INTEGER_BUFFER: {
      int* array=buffer->int_buffer;
      for (int i=pos;i<buffer->MAXIMUM_BUFFER_SIZE;i++) {
        /* First, we copy the end of the buffer at the beginning */
        array[i-pos]=array[i];
      }
      new_position=buffer->MAXIMUM_BUFFER_SIZE-pos;
      n_element_read=fread(&(array[new_position]),sizeof(int),pos,f);
      break;
   }
   case UNICHAR_BUFFER: {
      unichar* array=buffer->unichar_buffer;
      for (int i=pos;i<buffer->MAXIMUM_BUFFER_SIZE;i++) {
        /* First, we copy the end of the buffer at the beginning */
        array[i-pos]=array[i];
      }
      new_position=buffer->MAXIMUM_BUFFER_SIZE-pos;
      /* Here, we must not use a 'fread', since it would not unify \r\n 
       * into the single \n that is used in Unitex programs */
      n_element_read=u_fread(&(array[new_position]),pos,f);
      /* We add an extra \0 in order for string parsing reasons */
      array[new_position+n_element_read]='\0';
      break;
   }
}
buffer->size=new_position+n_element_read;
buffer->end_of_file=(n_element_read==0) || (buffer->size<buffer->MAXIMUM_BUFFER_SIZE);
}


/**
 * This function fills the given buffer from the given file.
 */
void fill_buffer(struct buffer* buffer,FILE* f) {
fill_buffer(buffer,buffer->MAXIMUM_BUFFER_SIZE,f);
}

