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



/**
 * This library provides an abstraction for mapping and unmapping files
 * in read-only mode in a portable way.
 */

//#ifndef _NOT_UNDER_WINDOWS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <windows.h>
#include "Error.h"
#include "MappedFileHelper.h"


#if defined(WINAPI_FAMILY_PARTITION) && (!(defined(UNITEX_USING_WINRT_API)))
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#if (!(defined(UNITEX_USING_WINRT_API))) && (!(defined(UNITEX_PREVENT_USING_WINRT_API)))
#define UNITEX_USING_WINRT_API 1
#endif
#endif
#endif

using namespace unitex;

struct _MAPFILE_REAL {
        HANDLE hFile;
        HANDLE hMap;
        LARGE_INTEGER li_size;
        };
typedef struct _MAPFILE_REAL MAPFILE_REAL;

MAPFILE* iomap_open_mapfile(const char*name,int /* option*/, size_t /*value_for_option*/)
{
    MAPFILE_REAL* mfr=(MAPFILE_REAL*)malloc(sizeof(MAPFILE_REAL));
    if (mfr == NULL) {
        fatal_alloc_error("iomap_open_mapfile");
        return NULL;
    }
    mfr->hFile = mfr->hMap = NULL;

#ifdef UNITEX_USING_WINRT_API
  WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
  MultiByteToWideChar(CP_ACP,0,name,-1,filenameW,FILENAME_MAX + 0x200);
  WIN32_FILE_ATTRIBUTE_DATA fad;
  GetFileAttributesExW(filenameW,GetFileExInfoStandard,&fad);
  //mfr->len = fad.nFileSizeLow;
  mfr->li_size.u.LowPart = fad.nFileSizeLow;
  mfr->li_size.u.HighPart = fad.nFileSizeHigh;
  mfr->hFile = CreateFile2(filenameW, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, /*FILE_FLAG_RANDOM_ACCESS,*/ NULL);
    if ((mfr->hFile != INVALID_HANDLE_VALUE) && (mfr->hFile != NULL))
        mfr->hMap = CreateFileMappingFromApp(mfr->hFile, NULL,
                        PAGE_READONLY, 0, NULL);
#else
    mfr->hFile=CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL,
                  OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
    if ((mfr->hFile != INVALID_HANDLE_VALUE) && (mfr->hFile != NULL))
        mfr->hMap = CreateFileMapping(mfr->hFile, NULL,
                        PAGE_READONLY, 0, 0, NULL);
#endif
    if ((mfr->hMap == INVALID_HANDLE_VALUE) || (mfr->hMap == NULL))
    {
        if ((mfr->hFile != INVALID_HANDLE_VALUE) && (mfr->hFile != NULL))
            CloseHandle(mfr->hFile);
        free(mfr);
        return NULL;
    }

#ifdef UNITEX_USING_WINRT_API
#else
    mfr->li_size.u.HighPart = 0;
    mfr->li_size.u.LowPart = GetFileSize(mfr->hFile,(DWORD*)&(mfr->li_size.u.HighPart));
#endif
    return (MAPFILE*)mfr;
}

size_t iomap_get_mapfile_size(MAPFILE* mf)
{
    MAPFILE_REAL* mfr=(MAPFILE_REAL*)mf;
    if (mfr==NULL)
        return 0;

    return (size_t) (mfr->li_size.QuadPart);
}

const void* iomap_get_mapfile_pointer(MAPFILE* mf, size_t pos, size_t sizemap)
{
    LARGE_INTEGER li;
    li.QuadPart = pos;
    MAPFILE_REAL* mfr=(MAPFILE_REAL*)mf;
    if (mfr==NULL)
        return 0;
    if ((pos==0) && (sizemap==0))
        sizemap=(size_t)(mfr->li_size.QuadPart);
#ifdef UNITEX_USING_WINRT_API
    return MapViewOfFileFromApp(mfr->hMap,FILE_MAP_READ,li.QuadPart,sizemap);
#else
    return MapViewOfFile(mfr->hMap,FILE_MAP_READ,li.u.HighPart,li.u.LowPart,sizemap);
#endif
}

void iomap_release_mapfile_pointer(MAPFILE *, const void*buf,size_t)
{
    if ((buf==NULL))
        return ;
    UnmapViewOfFile((void*)buf);
}

void iomap_close_mapfile(MAPFILE* mf)
{
    MAPFILE_REAL* mfr=(MAPFILE_REAL*)mf;
    if (mfr==NULL)
        return ;
    CloseHandle(mfr->hMap);
    CloseHandle(mfr->hFile);
    free(mfr);
}

//#endif
