/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS) 
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */




#ifndef _ZLIB_H_REPLACE_TYPE
#define _ZLIB_H_REPLACE_TYPE 1


#define _ZLIB_H
#define ZEXPORT

#ifndef OF /* function prototypes */
#if defined(STDC) || (defined(__STDC__) || defined(__cplusplus))
#    define OF(args)  args
#  else
#    define OF(args)  ()
#  endif
#endif

typedef unsigned long  uInt; /* 32 bits or more */
typedef unsigned long  uLong; /* 32 bits or more */
typedef unsigned char  Byte;  /* 8 bits */
typedef unsigned char  Bytef;  /* 8 bits */
typedef void *voidp;
typedef void *voidpf;

#define z_off_t size_t 



typedef struct z_stream_s {
    Bytef    *next_in;  /* next input byte */
    uInt     avail_in;  /* number of bytes available at next_in */
    uLong    total_in;  /* total nb of input bytes read so far */

    Bytef    *next_out; /* next output byte should be put there */
    uInt     avail_out; /* remaining free space at next_out */
    uLong    total_out; /* total nb of bytes output so far */

    char     *msg;      /* last error message, NULL if no error */
    //struct internal_state FAR *state; /* not visible by applications */

//    alloc_func zalloc;  /* used to allocate the internal state */
//    free_func  zfree;   /* used to free the internal state */
    voidpf     opaque;  /* private data object passed to zalloc and zfree */

    int     data_type;  /* best guess about the data type: binary or text */
    uLong   adler;      /* adler32 value of the uncompressed data */
    uLong   reserved;   /* reserved for future use */
} z_stream;


#define Z_ERRNO (-1)
#define Z_STREAM_ERROR (-2)
#define Z_MEM_ERROR    (-4)
#define Z_OK            0
#define MAX_WBITS   15
#define MAX_MEM_LEVEL 9
#define Z_DEFAULT_STRATEGY    0
#define Z_STREAM_END    1

#define Z_BINARY   0
#define Z_TEXT     1
#define Z_ASCII    Z_TEXT   /* for compatibility with 1.2.2 and earlier */

#endif
