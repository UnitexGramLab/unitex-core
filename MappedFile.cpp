 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include "MappedFile.h"
#include "File.h"


#ifdef _NOT_UNDER_WINDOWS
#include <stdio.h>
#include <sys/mman.h>

/**
 * Linux-like file mapping in read-only mode.
 */
void* my_mmap(U_FILE* f) {
int fd=fileno((FILE*)(f->f->dummy));
int filesize=get_file_size(f);
return mmap(NULL,filesize,PROT_READ,MAP_PRIVATE,fd,0);
}


/**
 * Linux-like file unmapping.
 */
void my_munmap(void* ptr,int size) {
munmap(ptr,size);
}

#else

#include "Error.h"

/**
 * Windows-like file mapping in read-only mode.
 */
void* my_mmap(U_FILE* f) {
int filesize=get_file_size(f);
void* ptr=malloc(filesize);
if (ptr==NULL) {
	fatal_alloc_error("my_mmap");
}
int n=fread(ptr,1,size,f);
if (n!=size) {
	fatal_error("Unable to read whole file in my_mmap\n");
}
return ptr;
}

/**
 * Windows-like file unmapping.
 */
void my_munmap(void* ptr,int size) {
free(ptr);
}

#endif
