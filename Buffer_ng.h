
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


#ifndef BUFFER_NG_H
#define BUFFER_NG_H

#include <stdio.h>
#include <stdlib.h>

#define byte unsigned char

typedef struct {

	byte * beg;
	byte * end;
	byte * cpos;
	size_t size; // capacity of the buffer
	size_t fill; // size of the buffer

	FILE * file;
	size_t fpos; // pos of beg in the file
	size_t fsize;// size of the file

} buffer_ng;


/**
 * \brief Init the buffer. Return 0 on success and 1 on failure (malloc error)
 *
 * @param b    the buffer struct to be initialized. Should not be null
 * @param size the size of the buffer in the memory
 * @param file the file pointer that will be managed by the buffer. 
 *             should be opened before passing it to this function.
 */
int   buffer_init( buffer_ng *b, size_t size, FILE *file );

/**
 * \brief Go n bytes forward in the file by making sure that 
 *        there is still s bytes to read forward.
 */
byte *buffer_next( buffer_ng *b, size_t n, size_t s ); 

/**
 * \brief Go n bytes backward in the file by making sure that 
 *        there is still s bytes to read forward 
 */
byte *buffer_prev( buffer_ng *b, size_t n, size_t s ); 

/** 
 * \brief Set buffer pointer to the to pth byte of the file 
 *        by making sure that there is still s bytes to read forward.
 */
byte *buffer_set ( buffer_ng *b, size_t p, size_t s );

/**
 * \brief Set buffer pointer to (p-size)/2 
 */
byte *buffer_set_mid ( buffer_ng *b, size_t p );

/**
 * \brief You'll end up with b->size bytes more available memory 
 */
void  buffer_free( buffer_ng *b );

#undef byte

#endif

