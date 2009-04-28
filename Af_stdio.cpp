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

#include <stdio.h>
#include <stdlib.h>
#include "Error.h"
#include "Af_stdio.h"


size_t af_fread(void *ptr,size_t size,size_t nmemb,ABSTRACTFILE *stream)
{
	return fread(ptr,size,nmemb,stream->fin);
}

size_t af_fwrite(const void *ptr,size_t size,size_t nmemb,ABSTRACTFILE *stream)
{
	return fwrite(ptr,size,nmemb,stream->fin);
}


char *af_fgets(char * _Buf, int _MaxCount, ABSTRACTFILE * _File)
{
	return fgets(_Buf,_MaxCount,_File->fin);
}

ABSTRACTFILE* af_fopen(const char* name,const char* MODE)
{
	ABSTRACTFILE* vf= (ABSTRACTFILE*)malloc(sizeof(ABSTRACTFILE));
	if (vf==NULL) {
		fatal_alloc_error("af_fopen");
		return NULL;
	}

	vf->fin = fopen(name,MODE);
	if (vf->fin == NULL) {
		free(vf);
		vf = NULL;
	}

	return vf;
}


int af_fseek(ABSTRACTFILE* stream, long offset, int whence)
{
	return fseek(stream->fin,offset,whence);
}

long af_ftell(ABSTRACTFILE* stream)
{
	return ftell(stream->fin);
}

void af_rewind(ABSTRACTFILE* stream)
{
	af_fseek( stream, 0L, SEEK_SET ); 
}

int af_feof(ABSTRACTFILE* stream)
{
	return feof(stream->fin);
}

int af_fgetc(ABSTRACTFILE* stream)
{
	return fgetc(stream->fin);
}

int af_ungetc(int ch, ABSTRACTFILE* stream)
{
	return ungetc(ch,stream->fin);
}

int af_fclose(ABSTRACTFILE* stream)
{
	FILE * fin = stream->fin;
	free(stream);
	return fclose(fin);
}

int af_rename(const char * OldFilename, const char * NewFilename)
{
	return rename(OldFilename,NewFilename);
}

int af_remove(const char * Filename)
{
	return remove(Filename);
}

ABSTRACTFILE VF_StdIn = { stdin };
ABSTRACTFILE VF_StdOut = { stdout };
ABSTRACTFILE VF_StdErr = { stderr };

ABSTRACTFILE* return_af_stdin()
{
	return &VF_StdIn;
}

ABSTRACTFILE* return_af_stdout()
{
	return &VF_StdOut;
}

ABSTRACTFILE* return_af_stderr()
{
	return &VF_StdErr;
}

int IsStdIn(ABSTRACTFILE* stream)
{
	return ((stream->fin)==stdin);
}