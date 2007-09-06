 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Universit� de Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef CollocationH
#define CollocationH

#include "Unicode.h"
#include "Array.h"
#include "Snt.h"

/**
 * This structure is used to store information about the given
 * Colloc arguments. It is used to avoid giving too much parameters
 * to functions.
 */

typedef struct {
	unsigned rstart;
	unsigned rend;
	unichar **spos;
	unichar **swords;
	int threshold;
	int compact;
	int quiet;
} colloc_opt;

int     colloc_print(array_t array, colloc_opt option);
array_t colloc_generate_candidates( struct snt_files *, colloc_opt option );

#endif

