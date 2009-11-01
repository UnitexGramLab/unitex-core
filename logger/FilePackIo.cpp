 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/* ioapi.c -- IO base function header for compress/uncompress .zip
   files using zlib + zip or unzip API


   Written 1998-2005 Gilles Vollant 
   http://www.winimage.com/zLibDll/minizip.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "Af_stdio.h"

#include "FilePack.h"
#include "FilePackIo.h"

#ifdef NO_ZLIB



#ifndef OF /* function prototypes */
#  ifdef STDC
#    define OF(args)  args
#  else
#    define OF(args)  ()
#  endif
#endif

#else
#include "zlib.h"
#endif




/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif

voidpf ZCALLBACK afopen_file_func OF((
   voidpf opaque,
   const char* filename,
   int mode));

uLong ZCALLBACK afread_file_func OF((
   voidpf opaque,
   voidpf stream,
   void* buf,
   uLong size));

uLong ZCALLBACK afwrite_file_func OF((
   voidpf opaque,
   voidpf stream,
   const void* buf,
   uLong size));

long ZCALLBACK aftell_file_func OF((
   voidpf opaque,
   voidpf stream));

long ZCALLBACK afseek_file_func OF((
   voidpf opaque,
   voidpf stream,
   uLong offset,
   int origin));

int ZCALLBACK afclose_file_func OF((
   voidpf opaque,
   voidpf stream));

int ZCALLBACK aferror_file_func OF((
   voidpf opaque,
   voidpf stream));


voidpf ZCALLBACK afopen_file_func (
   voidpf opaque,
   const char* filename,
   int mode)
{
    ABSTRACTFILE* file = NULL;
    const char* mode_fopen = NULL;

    (void)opaque;

    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    else
    if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    else
    if (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    if ((filename!=NULL) && (mode_fopen != NULL))
        file = af_fopen_unlogged(filename, mode_fopen);
    return file;
}


uLong ZCALLBACK afread_file_func (
   voidpf opaque,
   voidpf stream,
   void* buf,
   uLong size)
{
    uLong ret;

    (void)opaque;

    ret = (uLong)af_fread(buf, 1, (size_t)size, (ABSTRACTFILE *)stream);
    return ret;
}


uLong ZCALLBACK afwrite_file_func (
   voidpf opaque,
   voidpf stream,
   const void* buf,
   uLong size)
{
    uLong ret;

    (void)opaque;

    ret = (uLong)af_fwrite(buf, 1, (size_t)size, (ABSTRACTFILE *)stream);
    return ret;
}

long ZCALLBACK aftell_file_func (
   voidpf opaque,
   voidpf stream)
{
    long ret;

    (void)opaque;

    ret = af_ftell((ABSTRACTFILE *)stream);
    return ret;
}

long ZCALLBACK afseek_file_func (
   voidpf opaque,
   voidpf stream,
   uLong offset,
   int origin)
{
    int fseek_origin=0;
    long ret;

    (void)opaque;

    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        break;
    case ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        break;
    case ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        break;
    default: return -1;
    }
    ret = 0;
    af_fseek((ABSTRACTFILE *)stream, offset, fseek_origin);
    return ret;
}

int ZCALLBACK afclose_file_func (
   voidpf opaque,
   voidpf stream)
{
    int ret;

    (void)opaque;

    ret = af_fclose_unlogged((ABSTRACTFILE *)stream);
    return ret;
}

int ZCALLBACK aferror_file_func (
   voidpf opaque,
   voidpf stream)
{
    int ret;

    (void)opaque;
    (void)stream;

    /* ret = ferror((ABSTRACTFILE *)stream); */
    /* ferror is not implemented in ABSTRACTFILE */
    ret = 0;
    return ret;
}

void fill_afopen_filefunc (
  zlib_filefunc_def* pzlib_filefunc_def)
{
    pzlib_filefunc_def->zopen_file = afopen_file_func;
    pzlib_filefunc_def->zread_file = afread_file_func;
    pzlib_filefunc_def->zwrite_file = afwrite_file_func;
    pzlib_filefunc_def->ztell_file = aftell_file_func;
    pzlib_filefunc_def->zseek_file = afseek_file_func;
    pzlib_filefunc_def->zclose_file = afclose_file_func;
    pzlib_filefunc_def->zerror_file = aferror_file_func;
    pzlib_filefunc_def->opaque = NULL;
}
