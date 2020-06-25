/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates, initializes and returns a new buffer of the given capacity.
 * Its size is initialized to 0. Its type (integer or unichar) is specified by
 * the 'type' parameter.
 */
struct buffer* new_buffer(int capacity,BufferType type) {
struct buffer* buffer=(struct buffer*)malloc(sizeof(struct buffer));
if (buffer==NULL) {
    fatal_alloc_error("new_buffer");
}
buffer->type=type;
switch (type) {
   case INTEGER_BUFFER:
      buffer->int_buffer=(int*)malloc(sizeof(int)*capacity);
      if (buffer->int_buffer==NULL) {
         fatal_alloc_error("new_buffer");
      }
      break;
   case UNICHAR_BUFFER:
      /* In case of a unichar buffer, we add 1 to the size in order to store a \0,
       * even if the buffer is full. This precaution is useful in order to
       * do string parsing in a unichar buffer, avoiding the risk of an out of
       * bounds error */
      buffer->unichar_buffer=(unichar*)malloc(sizeof(unichar)*(capacity+1));
      if (buffer->unichar_buffer==NULL) {
         fatal_alloc_error("new_buffer");
      }
      break; /* Useless, except if we add something in the future... */
}
buffer->MAXIMUM_BUFFER_SIZE=capacity;
buffer->size=0;
buffer->end_of_file=0;
return buffer;
}


/**
 * Allocates, initializes and returns a new buffer of file size length
 */
struct buffer* new_buffer_for_file(BufferType type,U_FILE* fileread,int capacity_limit) {
int item_size=1;
switch (type) {
   case INTEGER_BUFFER:
       item_size = sizeof(int);
       break;
   case UNICHAR_BUFFER:
       int is_UTF16 = u_is_UTF16(fileread);
       if ((is_UTF16 == UTF16_LITTLE_ENDIAN_FILE) || (is_UTF16 == UTF16_BIG_ENDIAN_FILE))
           item_size = sizeof(unichar);
       break;
}
long save_pos=ftell(fileread);
fseek(fileread,0,SEEK_END);
long file_size=ftell(fileread);
fseek(fileread,save_pos,SEEK_SET);
int capacity=(int)((file_size/item_size)+0x10);
if ((capacity_limit != 0) && (capacity>capacity_limit)) {
    capacity=capacity_limit;
}
return new_buffer(capacity,type);
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
 *
 *
 * The function returns 1, except in one case: when the function has to fill a
 * character buffer, if skips '\0' chars that should not appear in a text file. In that
 * case, the function ignore those characters and returns 0.
 */
int fill_buffer(struct buffer* buffer,int pos,int raw,U_FILE* f) {
int new_position=-1;
int n_element_read=-1;
int OK=1;
switch (buffer->type) {
   case INTEGER_BUFFER: {
      int* i_array=buffer->int_buffer;
      /* if the two buffer don't overlap, we use optimized memcpy */
      if (((buffer->MAXIMUM_BUFFER_SIZE-pos) < pos) && ((buffer->MAXIMUM_BUFFER_SIZE-pos)>=0)) {
          memcpy(&i_array[0],&i_array[pos],(buffer->MAXIMUM_BUFFER_SIZE-pos)*sizeof(int));
      }
      else
      {
         for (int i=pos;i<buffer->MAXIMUM_BUFFER_SIZE;i++) {
            /* First, we copy the end of the buffer at the beginning */
            i_array[i-pos]=i_array[i];
         }
      }
      new_position=buffer->MAXIMUM_BUFFER_SIZE-pos;
      n_element_read=(int)fread(&(i_array[new_position]),sizeof(int),pos,f);
      break;
   }
   case UNICHAR_BUFFER: {
      unichar* u_array=buffer->unichar_buffer;
      /* if the two buffer don't overlap, we use optimized memcpy */
      if (((buffer->MAXIMUM_BUFFER_SIZE-pos) < pos) && ((buffer->MAXIMUM_BUFFER_SIZE-pos)>=0)) {
          memcpy(&u_array[0],&u_array[pos],(buffer->MAXIMUM_BUFFER_SIZE-pos)*sizeof(unichar));
      }
      else
      {
         for (int i=pos;i<buffer->MAXIMUM_BUFFER_SIZE;i++) {
            /* First, we copy the end of the buffer at the beginning */
            u_array[i-pos]=u_array[i];
         }
      }
      new_position=buffer->MAXIMUM_BUFFER_SIZE-pos;
      /* Here, we must not use a 'fread', since it would not unify \r\n
       * into the single \n that is used in Unitex programs */
      int tmp;
      n_element_read=(raw!=0) ? u_fread_raw(&(u_array[new_position]),pos,f,&tmp) : u_fread(&(u_array[new_position]), pos, f, &tmp);
      if (!tmp) {
         OK=0;
      }
      /* We add an extra \0 for string parsing reasons */
      u_array[new_position+n_element_read]='\0';
      break;
   }
   default: fatal_error("Invalid buffer type in fill_buffer\n");
}
buffer->size=new_position+n_element_read;
buffer->end_of_file=(n_element_read==0) || (buffer->size<buffer->MAXIMUM_BUFFER_SIZE);
return OK;
}


/**
 * This function fills the given buffer from the given file.
 * See above for details about the returned value.
 */
int fill_buffer(struct buffer* buffer,U_FILE* f) {
return fill_buffer(buffer,buffer->MAXIMUM_BUFFER_SIZE,0,f);
}


int fill_buffer(struct buffer* buffer, int pos, U_FILE* f) {
    return fill_buffer(buffer, pos, 0, f);
}


int fill_buffer_raw(struct buffer* buffer, U_FILE* f) {
    return fill_buffer(buffer, buffer->MAXIMUM_BUFFER_SIZE, 1, f);
}


int fill_buffer_raw(struct buffer* buffer, int pos, U_FILE* f) {
    return fill_buffer(buffer, pos, 1, f);
}

int fill_buffer_keepCR_option(struct buffer* buffer, int pos, int keep_cr, U_FILE* f) {
    return fill_buffer(buffer, pos, keep_cr, f);
}

int fill_buffer_keepCR_option(struct buffer* buffer, int keep_cr, U_FILE* f) {
    return fill_buffer(buffer, buffer->MAXIMUM_BUFFER_SIZE, keep_cr, f);
}


} // namespace unitex
