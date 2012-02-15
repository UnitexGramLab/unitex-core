/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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



/**
 * This library provides an abstraction for mapping and unmapping files
 * in read-only mode in a portable way.
 *  This implementation is "dummy" with only fread/malloc for full compatibility
 *  when Posix or Windows API are not avaiable
 */




#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include "Error.h"
#include "MappedFileHelper.h"

using namespace unitex;

struct _MAPFILE_REAL {
        FILE*f;
        long filesize;
        };
typedef struct _MAPFILE_REAL MAPFILE_REAL;

MAPFILE* iomap_open_mapfile(const char*name,int /* option*/, size_t /*value_for_option*/)
{
    MAPFILE_REAL* mfr=(MAPFILE_REAL*)malloc(sizeof(MAPFILE_REAL));
    if (mfr == NULL) {
        fatal_alloc_error("iomap_open_mapfile");
        return NULL;
    }
    mfr->f = NULL;

    mfr->f=fopen(name,"rb");
    if (mfr -> f == NULL)
    {
        free(mfr);
        return NULL;
    }
    fseek(mfr->f,0,SEEK_END);
    mfr->filesize=ftell(mfr->f);
    fseek(mfr->f,0,SEEK_SET);

    return (MAPFILE*)mfr;
}

size_t iomap_get_mapfile_size(MAPFILE* mf)
{
    MAPFILE_REAL* mfr=(MAPFILE_REAL*)mf;
    if (mfr==NULL)
        return 0;
    return mfr->filesize;
}

const void* iomap_get_mapfile_pointer(MAPFILE* mf, size_t pos, size_t sizemap)
{
    MAPFILE_REAL* mfr=(MAPFILE_REAL*)mf;
    if (mfr==NULL)
        return 0;
    if ((pos==0) && (sizemap==0))
        sizemap=mfr->filesize;
    if (pos+sizemap > ((size_t)mfr->filesize))
        return NULL;
    void* buf = (void*)malloc(sizemap);
    if (buf == NULL) {
        fatal_alloc_error("iomap_get_mapfile_pointer");
        return NULL;
    }

    fseek(mfr->f,(long)pos,SEEK_SET);
    if (fread(buf,1,sizemap,mfr->f) != sizemap) {
        free(buf);
        buf=NULL;
    }

    return buf;
}

void iomap_release_mapfile_pointer(MAPFILE *mf, const void*buf, size_t sizemap)
{    
    MAPFILE_REAL* mfr=(MAPFILE_REAL*)mf;
    if (mfr==NULL)
        return ;
    if ((sizemap==0))
        sizemap=mfr->filesize;
    free((void*)buf);
}

void iomap_close_mapfile(MAPFILE* mf)
{
    MAPFILE_REAL* mfr=(MAPFILE_REAL*)mf;
    if (mfr==NULL)
        return ;
    fclose(mfr->f);
    free(mfr);
}
