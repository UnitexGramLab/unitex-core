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
void* my_mmap(char* filename,MappedFile *m) {
m->f=u_fopen(BINARY,filename,U_READ);
int fd=fileno(m->f->f->dummy);
m->length=get_file_size(m->f);
m->ptr=mmap(NULL,m->length,PROT_READ,MAP_PRIVATE,fd,0);
return m;
}


/**
 * Linux-like file unmapping.
 */
void my_munmap(MappedFile m) {
munmap(m.ptr,m.length);
u_fclose(m.f);
}

#else

#include "Error.h"


/**
 * Windows-like file mapping in read-only mode.
 */
void* my_mmap(char* filename,MappedFile *m) {
m->file=CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL,NULL);
if (m->file==INVALID_HANDLE_VALUE) {
   fatal_error("Unable to open file in Windows-version of my_mmap\n");
}
m->mappedFile=CreateFileMapping(m->file,NULL,PAGE_READONLY,0,0,NULL);
if (m->mappedFile==NULL) {
   fatal_error("Unable to map file in Windows-version of my_mmap\n");
}
m->ptr=MapViewOfFile(m->mappedFile,FILE_MAP_READ,0,0,0);
return m->ptr;
}

/**
 * Windows-like file unmapping.
 */
void my_munmap(MappedFile m) {
UnmapViewOfFile(m.ptr);
CloseHandle(m.mappedFile);
CloseHandle(m.file);
}

#endif
