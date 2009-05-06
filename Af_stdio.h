 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef _ABSTRACT_FILE_STDIO_INCLUDED
#define _ABSTRACT_FILE_STDIO_INCLUDED



#ifdef __cplusplus
extern "C" {
#endif

struct _ABSTRACTFILE {
        void* dummy;
        };
typedef struct _ABSTRACTFILE ABSTRACTFILE;

size_t af_fread(void *ptr,size_t size,size_t nmemb,ABSTRACTFILE *stream);

size_t af_fwrite(const void *ptr,size_t size,size_t nmemb,ABSTRACTFILE *stream);

char *af_fgets(char * _Buf, int _MaxCount, ABSTRACTFILE * _File);

ABSTRACTFILE* af_fopen(const char*,const char*);

int af_fseek(ABSTRACTFILE* stream, long offset, int whence) ;

long af_ftell(ABSTRACTFILE* stream) ;

int af_feof(ABSTRACTFILE* stream) ;

int af_ungetc(int, ABSTRACTFILE* stream) ;

int af_fclose(ABSTRACTFILE* stream) ;

void af_setsizereservation(ABSTRACTFILE* stream, long size_planned);

int af_rename(const char * OldFilename, const char * NewFilename);

int af_remove(const char * Filename);

int af_copy(const char* srcFile, const char* dstFile);

ABSTRACTFILE* return_af_stdin();
ABSTRACTFILE* return_af_stdout();
ABSTRACTFILE* return_af_stderr();

int IsStdIn(ABSTRACTFILE*);


#ifdef __cplusplus
}
#endif


#endif
