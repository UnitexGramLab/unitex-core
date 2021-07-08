/*
 * Unitex - Performance optimization code 
 *
 * File created and contributed by Gilles Vollant, working with François Liger
 * as part of an UNITEX optimization and reliability effort, first descibed at
 * http://www.smartversion.com/unitex-contribution/Unitex_A_NLP_engine_from_the_lab_to_the_iPhone.pdf
 *
 * Free software when used with Unitex 3.2 or later
 *
 * Copyright (C) 2021-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define TRY_AUTOLOAD_PERSISTANT 1

#include "AbstractFilePlugCallback.h"
#include "Af_stdio.h"

#ifdef TRY_AUTOLOAD_PERSISTANT
#include "DELA.h"
// VersatileEncodingConfigDefined was defined in Unitex near same time than introduce LoadInf.h
#ifdef VersatileEncodingConfigDefined
#include "LoadInf.h"
#endif
#endif

#include "VirtFileType.h"

#include "VirtualSpaceManager.h"
#include "VirtFileSystem.h"
#include "MiniMutex.h"



#ifdef TRY_AUTOLOAD_PERSISTANT


#include "VirtDela.h"

#include "AbstractFst2Load.h"
#include "AbstractFst2PlugCallback.h"
#include "VirtFst2.h"
BOOL TryAutoUnloadPersistant(const char* dfFileName);
BOOL TryAutoLoadPersistent(const char* dfFileName);
#endif


#ifdef _NOT_UNDER_WINDOWS
#include <dirent.h>
#include <sys/stat.h>

BOOL mkdirAtomic(const char* dirname)
{
    int retMkDir = (int)(mkdir(dirname,S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP|S_IXGRP|S_IXOTH));
    return (retMkDir != -1);
    //return mkdir(dirname);
}
#define DEBUG_ASSERT(a,b,c)
#else


#include <direct.h>

BOOL mkdirAtomic(const char* dirname)
{
    //return mkdir(dirname,S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP|S_IXGRP|S_IXOTH) != -1;
    return mkdir(dirname);
}
#if defined(_DEBUG) && defined(WIN32d)
#include <windows.h>

void ShowDebugError(const char*str,const char* fn)
{
    OutputDebugStringA(str);
    OutputDebugStringA(" ");
    if (fn != NULL)
      OutputDebugStringA(fn);
    puts(str);
    if (fn != NULL)
      puts(fn);
}

#define DEBUG_ASSERT(a,b,c) if (!(a)) { ShowDebugError(b,c); }
#define DEBUG_ASSERT_USED
#else
#define DEBUG_ASSERT(a,b,c)
#endif
#endif

//#include "export_unitex.h"
//#include "export_unitex_virtual_cb_file.h"

typedef void *dfvoidp;
typedef const void *dfvoidpc;
typedef unsigned char dfbyte;
typedef dfbyte *dfbytep;
typedef const dfbyte *dfbytepc;

typedef unsigned long dfuLong32;
typedef signed long dfsLong32;

//typedef unsigned int dfuInt;
//typedef unsigned short dfuShort;
//typedef unsigned short dfuInt16;

#define DFSCALLBACK

#ifndef BYTE
typedef dfbyte BYTE;
#endif

#ifndef VOID
typedef void VOID;
#endif

#ifndef CONST
#define CONST               const
#endif

#ifndef LPBYTE
typedef BYTE *LPBYTE, *PBYTE;
#endif

#ifndef LPCBYTE
typedef CONST BYTE *LPCBYTE, *PCBYTE;
#endif

#ifndef LPCVOID
typedef CONST VOID *LPCVOID, *PCVOID;
#endif

#ifndef LPVOID
typedef VOID *LPVOID, *PVOID;
#endif

#ifndef DWORD
typedef dfuLong32 DWORD;
#endif

#ifndef LPDWORD
typedef DWORD *LPDWORD, *PDWORD;
#endif


#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif



#ifdef wchar_t
typedef wchar_t dfwchar;
#else
typedef unsigned short dfwchar;
#endif

#ifndef Round4
#define Round4(x) ((((x)+3)/4)*4)
#endif


typedef long (DFSCALLBACK *tCompareData) (LPCVOID lpElem1, LPCVOID lpElem2);
typedef BOOL(DFSCALLBACK *tDestructorData) (LPCVOID lpElem);

#define DfsFree(a) (free((a)))
#define DfsMalloc(a) (malloc((a)))
#define DfsRealloc(a,b) (realloc((a),(b)))

#define SIZE_NULL_PAD_END_FILE (8)

typedef void* ABSTRACTFILE_PTR;

typedef struct
{
    afs_size_type dfFileSize;
    afs_size_type posLastWrite;
    afs_size_type dfFileSizeAlloc;
    const char* dfVirtualFileName;
    dfbytep   Buf;
    const void* pUserPtr;
    BOOL fIsPermanentPointer;
    int dfNbOpen;
} VIRTUALFILEITEMINFO;

typedef struct
{
    VIRTUALFILEITEMINFO* pvfii;
} VIRTUALFILEITEM;

typedef struct
{
    VIRTUALFILEITEMINFO* pvfi;
    afs_size_type pos;
} VIRTUALFILEITEMFILEOPEN;

typedef struct
{
    char* dfFileName;
    VIRTUALFILEITEMFILEOPEN* pvfio;
    BOOL fReadOnlyAccess;
    //HANDLE hlFile;
} LOWLEVELINTERNAL;


/***************************************************************************/
/*

typedef struct
{
    ABSTRACTFILE_PTR llfStd;
    BOOL fTrashOutput;
} STANDARD_OUT_ARRAY;

#define STANDARD_OUT_STDOUT (0)
#define STANDARD_OUT_STDERR (1)
*/
typedef struct
{
    STATICARRAYC sacVirtualFileNameSpace;
    afs_size_type dfTotalSizeVirtualSpace;
    BOOL      fLimitSizeTotalVirtualSpace;
    afs_size_type dfMaxSizeTotalVirtualSpace;
    /*
    ABSTRACTFILE_PTR llfStdOut;
    ABSTRACTFILE_PTR llfStdErr;
    BOOL fTrashOutputOut;
    BOOL fTrashOutputErr;
    */
    //STANDARD_OUT_ARRAY std_out_array[2];
    MINIMUTEX_OBJECT* pMiniMutex;
} VIRTUALFILESPACE ;



#define GRANULARITY_ALLOC_BUFFER_FILE (0x800)

#define AroundUpper(dwValue,dwModulo) (((((dwValue)) + ((dwModulo)) -1) / ((dwModulo))) * (dwModulo))

static VIRTUALFILESPACE* pVirtualFileSpace=NULL;

#define ALLOC_MUTEX \
   { pVirtualFileSpace->pMiniMutex = BuildMutex(); } ;

#define CLEAR_MUTEX \
   { \
            if (pVirtualFileSpace->pMiniMutex!=NULL) \
            { \
                DeleteMiniMutex(pVirtualFileSpace->pMiniMutex); \
                pVirtualFileSpace->pMiniMutex = NULL; \
            } \
   } ;

#define GET_MUTEX \
   { \
            if (pVirtualFileSpace->pMiniMutex!=NULL) \
                GetMiniMutex(pVirtualFileSpace->pMiniMutex); \
   } ;

#define RELEASE_MUTEX \
   { \
            if (pVirtualFileSpace->pMiniMutex!=NULL) \
                ReleaseMiniMutex(pVirtualFileSpace->pMiniMutex); \
   } ;

BOOL DFSCALLBACK fncDestructorVirtualItem(LPCVOID lpElem)
{
    VIRTUALFILEITEM* pvfi=(VIRTUALFILEITEM*)lpElem;
    if (pvfi->pvfii!=NULL)
    {
        if (!(pvfi->pvfii->fIsPermanentPointer))
        {
            if (pvfi->pvfii->Buf!=NULL)
            {
                DfsFree(pvfi->pvfii->Buf);
                pVirtualFileSpace->dfTotalSizeVirtualSpace -= pvfi->pvfii->dfFileSize;
            }
            pvfi->pvfii->Buf = NULL;
            pvfi->pvfii->posLastWrite = 0;
        }

        if (pvfi->pvfii->dfVirtualFileName!=NULL)
            DfsFree((dfvoidp)(pvfi->pvfii->dfVirtualFileName));
        pvfi->pvfii->dfVirtualFileName=NULL;

        DfsFree(pvfi->pvfii);
        pvfi->pvfii=NULL;
    }
    return TRUE;
}

long DFSCALLBACK fncCmpVirtualItem(LPCVOID lpElem1, LPCVOID lpElem2)
{
    VIRTUALFILEITEM* pvfi1=(VIRTUALFILEITEM*)lpElem1;
    VIRTUALFILEITEM* pvfi2=(VIRTUALFILEITEM*)lpElem2;

    return strcmp(pvfi1->pvfii->dfVirtualFileName,pvfi2->pvfii->dfVirtualFileName);
}


BOOL InitStaticArrayVirtualFileNameSpace()
{
  if (pVirtualFileSpace->sacVirtualFileNameSpace!=NULL)
      return TRUE;

    pVirtualFileSpace->sacVirtualFileNameSpace = InitStaticArrayC(sizeof(VIRTUALFILEITEM),0x100);
    if (pVirtualFileSpace->sacVirtualFileNameSpace ==NULL)
        return FALSE;

    SetFuncDestructor(pVirtualFileSpace->sacVirtualFileNameSpace,&fncDestructorVirtualItem);
    SetFuncCompareData(pVirtualFileSpace->sacVirtualFileNameSpace,&fncCmpVirtualItem);

    return TRUE;
}

ULB_VFFUNC BOOL ULIB_CALL InitVirtualFileNameSpace()
{
    if (pVirtualFileSpace!=NULL)
    {
        return InitStaticArrayVirtualFileNameSpace();
    }

    pVirtualFileSpace = (VIRTUALFILESPACE*)DfsMalloc(sizeof(VIRTUALFILESPACE));
    if (pVirtualFileSpace==NULL)
    {
        return FALSE;
    }

    ALLOC_MUTEX;

    pVirtualFileSpace->sacVirtualFileNameSpace = NULL;
    pVirtualFileSpace->dfTotalSizeVirtualSpace = 0;
    pVirtualFileSpace->dfMaxSizeTotalVirtualSpace = 0;
    pVirtualFileSpace->fLimitSizeTotalVirtualSpace = FALSE;
    /*
    pVirtualFileSpace->std_out_array[STANDARD_OUT_STDOUT].llfStd = NULL;
    pVirtualFileSpace->std_out_array[STANDARD_OUT_STDERR].llfStd = NULL;
    pVirtualFileSpace->std_out_array[STANDARD_OUT_STDOUT].fTrashOutput = FALSE;
    pVirtualFileSpace->std_out_array[STANDARD_OUT_STDERR].fTrashOutput = FALSE;
    */
    return InitStaticArrayVirtualFileNameSpace();
}

ULB_VFFUNC BOOL ULIB_CALL SetVirtualFileNameMaximumMemory(BOOL fLimitSize, afs_size_type dfMaxSize)
{
    if (!(InitVirtualFileNameSpace()))
        return FALSE;
    pVirtualFileSpace->fLimitSizeTotalVirtualSpace = fLimitSize;
    //pVirtualFileSpace->dfMaxSizeTotalVirtualSpace = dfMaxSize*1024;
    pVirtualFileSpace->dfMaxSizeTotalVirtualSpace = dfMaxSize<<10;
    return TRUE;
}

STATICARRAYC GetSacVirtualFileNameSpace()
{
    if (!(InitVirtualFileNameSpace()))
        return FALSE;
    if (pVirtualFileSpace==NULL)
        return NULL;
    return pVirtualFileSpace->sacVirtualFileNameSpace;
}

ULB_VFFUNC BOOL ULIB_CALL UnInitVirtualFileNameSpace(BOOL fClearMaximumMemoryValue)
{
    if (pVirtualFileSpace!=NULL)
    {
        //SetStdOutFile(NULL,FALSE);
        //SetStdErrFile(NULL,FALSE);

        if (pVirtualFileSpace->sacVirtualFileNameSpace != NULL)
        {
            //DeleteElem(pVirtualFileSpace->sacVirtualFileNameSpace, 0, GetNbElem(pVirtualFileSpace->sacVirtualFileNameSpace));

             DeleteStaticArrayC(pVirtualFileSpace->sacVirtualFileNameSpace);//+++
        }
        pVirtualFileSpace->sacVirtualFileNameSpace = NULL;

        pVirtualFileSpace->dfTotalSizeVirtualSpace = 0;


        if ((fClearMaximumMemoryValue) || (!pVirtualFileSpace->fLimitSizeTotalVirtualSpace))
        {
            CLEAR_MUTEX;
            DfsFree(pVirtualFileSpace);
            pVirtualFileSpace = NULL;
        }
    }

    return TRUE;
}


// this function is used to perform a full cleanup at exit (if there is no files), so valgrind is happy
static void CleanVirtualFileNameSpaceAtExit()
{
	if (pVirtualFileSpace != NULL)
	{
		//SetStdOutFile(NULL,FALSE);
		//SetStdErrFile(NULL,FALSE);

		if (pVirtualFileSpace->sacVirtualFileNameSpace != NULL)
		{
			if (GetNbElem(pVirtualFileSpace->sacVirtualFileNameSpace) == 0)
			{
				UnInitVirtualFileNameSpace(TRUE);
			}
		}
	}
}


/*
VIRTUALFILEITEM* GetVirtualFileItemForVirtualFileName(const char* FileName)
{
    VIRTUALFILEITEM vfiSearch;
    VIRTUALFILEITEMINFO vfiSearchInfo;
    DWORD dwPosItem;
    VIRTUALFILEITEM* pVfiFile;
    STATICARRAYC sacVirtualFileNameSpace=GetSacVirtualFileNameSpace();


    vfiSearchInfo.dfVirtualFileName = FileName;
    vfiSearch.pvfii = &vfiSearchInfo;

    if (!FindSameElemPos(sacVirtualFileNameSpace, &vfiSearch, &dwPosItem))
        return NULL;
    pVfiFile=(VIRTUALFILEITEM*)GetElemPtr(sacVirtualFileNameSpace,dwPosItem);
    return pVfiFile;
}
*/
char* strcpyAlloc(const char* str)
{
    char* ret = (char*)malloc(strlen(str)+1);
    if (ret != NULL)
        strcpy(ret,str);
    return (char*)ret;
}

ABSTRACTFILE_PTR ABSTRACT_CALLBACK_UNITEX memOpenLowLevel(const char* FileName, TYPEOPEN_MF TypeOpen,
                            void*)
{
    // check if exist
    VIRTUALFILEITEM vfiSearch;
    VIRTUALFILEITEMINFO vfiSearchInfo;
    VIRTUALFILEITEM* pVfiFile;
    DWORD dwPosItem=0;
    LOWLEVELINTERNAL* pLowLevelIntern;
    BOOL fCreatingFile = FALSE;
    STATICARRAYC sacVirtualFileNameSpace;
    BOOL fItemFound;
    BOOL fReadOnlyAccess = (TypeOpen == OPEN_READ_MF);
    afs_size_type dfProjectedSize=0;

    if (!InitVirtualFileNameSpace())
        return NULL;

    GET_MUTEX;

    sacVirtualFileNameSpace=GetSacVirtualFileNameSpace();



    vfiSearchInfo.dfVirtualFileName = FileName;
    vfiSearch.pvfii = &vfiSearchInfo;

    fItemFound = FindSameElemPos(sacVirtualFileNameSpace, &vfiSearch, &dwPosItem);

    if ((!fItemFound) && (TypeOpen!=OPEN_CREATE_MF))
    {
        RELEASE_MUTEX;
        return NULL;
    }

    if ((fItemFound) && (TypeOpen==OPEN_CREATE_MF))
    {
#ifdef DEBUG_ASSERT_USED
        VIRTUALFILEITEM* pVfiFileToDeleteCheck=(VIRTUALFILEITEM*)GetElemPtr(sacVirtualFileNameSpace,dwPosItem);
        DEBUG_ASSERT(pVfiFileToDeleteCheck->pvfii->dfNbOpen == 0,"VFS Warning: try delete opened file",pVfiFileToDeleteCheck->pvfii->dfVirtualFileName);
#endif

        BOOL fRetDel = DeleteElem(sacVirtualFileNameSpace,dwPosItem,1);
        if (!fRetDel)
        {
            RELEASE_MUTEX;
            return NULL;
        }
        fItemFound=FALSE;
    }

    if ((!fItemFound) && (TypeOpen==OPEN_CREATE_MF))
    {
        VIRTUALFILEITEM vfiAdd;

        vfiAdd.pvfii=(VIRTUALFILEITEMINFO*)DfsMalloc(sizeof(VIRTUALFILEITEMINFO));
        vfiAdd.pvfii->dfVirtualFileName = strcpyAlloc(FileName);
        vfiAdd.pvfii->dfFileSize = dfProjectedSize;
        vfiAdd.pvfii->dfFileSizeAlloc = AroundUpper(dfProjectedSize+2,GRANULARITY_ALLOC_BUFFER_FILE);
        vfiAdd.pvfii->Buf = (dfbytep)DfsMalloc((size_t)(vfiAdd.pvfii->dfFileSizeAlloc)+SIZE_NULL_PAD_END_FILE);
        vfiAdd.pvfii->posLastWrite = 0;
        vfiAdd.pvfii->fIsPermanentPointer = FALSE;
        vfiAdd.pvfii->pUserPtr = NULL;
        vfiAdd.pvfii->dfNbOpen = 0;
        //pVirtualFileSpace->dfTotalSize += dfProjectedSize;

        if (vfiAdd.pvfii->Buf == NULL)
        {  // mem error
            RELEASE_MUTEX;
            return NULL;
        }

        fCreatingFile = InsertSorted(sacVirtualFileNameSpace,&vfiAdd);
        if (!fCreatingFile )
        {
            RELEASE_MUTEX;
            return NULL;
        }
        if (!FindSameElemPos(sacVirtualFileNameSpace, &vfiSearch, &dwPosItem))
        {
            RELEASE_MUTEX;
            return NULL;
        }
    }

    pVfiFile=(VIRTUALFILEITEM*)GetElemPtr(sacVirtualFileNameSpace,dwPosItem);
    pLowLevelIntern=(LOWLEVELINTERNAL*)DfsMalloc(sizeof(LOWLEVELINTERNAL));
    if (pLowLevelIntern==NULL)
    {
        RELEASE_MUTEX;
        return NULL;
    }
    pLowLevelIntern->dfFileName = strcpyAlloc(FileName);
    pLowLevelIntern->fReadOnlyAccess = fReadOnlyAccess;
    pLowLevelIntern->pvfio = (VIRTUALFILEITEMFILEOPEN*)DfsMalloc(sizeof(VIRTUALFILEITEMFILEOPEN));
    pLowLevelIntern->pvfio->pvfi = pVfiFile->pvfii;
    pLowLevelIntern->pvfio->pos = 0;
    pLowLevelIntern->pvfio->pvfi->dfNbOpen++;
    RELEASE_MUTEX;
    return (ABSTRACTFILE_PTR) pLowLevelIntern;
}

int ABSTRACT_CALLBACK_UNITEX DeleteMemFilePrivParam(const char* lpFileName,void*)
{
    VIRTUALFILEITEM vfiSearch;
    VIRTUALFILEITEMINFO vfiSearchInfo;
    DWORD dwPosItem;
    BOOL fRet;
    VIRTUALFILEITEM* pVfiFileToDelete;
    size_t sizeFileName = strlen(lpFileName);
    if (sizeFileName > 0)
        if ((*(lpFileName + sizeFileName - 1)) == '*')
        {
            char * fileNameDeletePrefix = (char*)malloc(sizeFileName + 1);
            strcpy(fileNameDeletePrefix,lpFileName);
            *(fileNameDeletePrefix + sizeFileName - 1) = '\0';
            BOOL fRetDeletePrefix = DeleteMemFileForPrefix(fileNameDeletePrefix);
            free(fileNameDeletePrefix);
            return (fRetDeletePrefix != FALSE) ? 0 : -1;
        }
    STATICARRAYC sacVirtualFileNameSpace=GetSacVirtualFileNameSpace();

    vfiSearchInfo.dfVirtualFileName = lpFileName;
    vfiSearch.pvfii = &vfiSearchInfo;

    GET_MUTEX;

    if (!FindSameElemPos(sacVirtualFileNameSpace, &vfiSearch, &dwPosItem))
    {
        RELEASE_MUTEX;
        return -1;
    }

    pVfiFileToDelete=(VIRTUALFILEITEM*)GetElemPtr(sacVirtualFileNameSpace,dwPosItem);
#ifdef DEBUG_ASSERT_USED
    DEBUG_ASSERT(pVfiFileToDelete->pvfii->dfNbOpen == 0,"VFS Warning: try delete opened file",pVfiFileToDelete->pvfii->dfVirtualFileName);
#endif

#ifdef TRY_AUTOLOAD_PERSISTANT
    TryAutoUnloadPersistant(pVfiFileToDelete->pvfii->dfVirtualFileName);
#endif
/*
    if (pVirtualFileSpace->std_out_array[STANDARD_OUT_STDOUT].llfStd != NULL)
    {
        LOWLEVELINTERNAL* pLowLevelInternStdOut = (LOWLEVELINTERNAL*)pVirtualFileSpace->std_out_array[STANDARD_OUT_STDOUT].llfStd;
        if (pLowLevelInternStdOut->pvfio->pvfi == pVfiFileToDelete->pvfii)
        {
            SetStdOutFile(NULL,FALSE);
        }
    }

    if (pVirtualFileSpace->std_out_array[STANDARD_OUT_STDERR].llfStd != NULL)
    {
        LOWLEVELINTERNAL* pLowLevelInternStdErr = (LOWLEVELINTERNAL*)pVirtualFileSpace->std_out_array[STANDARD_OUT_STDERR].llfStd;
        if (pLowLevelInternStdErr->pvfio->pvfi == pVfiFileToDelete->pvfii)
        {
            SetStdErrFile(NULL,FALSE);
        }
    }
*/
#ifdef DEBUG_ASSERT_USED
    VIRTUALFILEITEM* pVfiFileToDeleteCheck=(VIRTUALFILEITEM*)GetElemPtr(sacVirtualFileNameSpace,dwPosItem);
    DEBUG_ASSERT(pVfiFileToDeleteCheck->pvfii->dfNbOpen == 0,"VFS Warning: try delete opened file",pVfiFileToDeleteCheck->pvfii->dfVirtualFileName);
#endif

    fRet = DeleteElem(sacVirtualFileNameSpace,dwPosItem,1);

    {
        DWORD dwNbElem=GetNbElem(sacVirtualFileNameSpace);
        if (dwNbElem==0)
        {
            //UnInitVirtualFileNameSpace(FALSE);
        }
    }

    RELEASE_MUTEX;
    return (fRet != FALSE) ? 0 : -1;
}

ULB_VFFUNC BOOL ULIB_CALL DeleteMemFile(const char* lpFileName)
{
    return (DeleteMemFilePrivParam(lpFileName,NULL) == 0) ? TRUE : FALSE;
}

size_t memCheckSize(LOWLEVELINTERNAL* pLowLevelIntern,size_t size, BOOL fTryExpand, BOOL fExpandMore)
{
    /* we immediatly return if we no need enlarge the file (reading or overwrite) */
    if (pLowLevelIntern->pvfio->pos + size <= pLowLevelIntern->pvfio->pvfi->dfFileSize)
        return size;
    /* return also if we do not want enlarge file (reading) */
    if (!fTryExpand)
        return ((pLowLevelIntern->pvfio->pvfi->dfFileSize) - (pLowLevelIntern->pvfio->pos));
    else
    { /* we add two for adding 8 null char, see GetMemoryBufferFromMemFile */
        afs_size_type dfNeededSizeBuffer = pLowLevelIntern->pvfio->pos + size + 8;
        if (dfNeededSizeBuffer <= pLowLevelIntern->pvfio->pvfi->dfFileSizeAlloc)
            return size;

        if (dfNeededSizeBuffer > pLowLevelIntern->pvfio->pvfi->dfFileSizeAlloc)
        {
            afs_size_type dfFileSizeAllocNew = 
                AroundUpper(dfNeededSizeBuffer + (fExpandMore ? (dfNeededSizeBuffer/4) : 0),GRANULARITY_ALLOC_BUFFER_FILE);
            dfbytep newbuf ;
            if (pLowLevelIntern->pvfio->pvfi->Buf == NULL)
                newbuf = (dfbytep)DfsMalloc(dfFileSizeAllocNew+SIZE_NULL_PAD_END_FILE);
            else
                newbuf = (dfbytep)DfsRealloc(pLowLevelIntern->pvfio->pvfi->Buf,dfFileSizeAllocNew+SIZE_NULL_PAD_END_FILE);
            if (newbuf != NULL)
            {
                pLowLevelIntern->pvfio->pvfi->Buf = newbuf;
                pLowLevelIntern->pvfio->pvfi->dfFileSizeAlloc = dfFileSizeAllocNew;
            }
            return size;
        }
    }
    return 0;
}

int ABSTRACT_CALLBACK_UNITEX memLowLevelSetSizeReservation(ABSTRACTFILE_PTR llFile, afs_size_type size_reserv,void*)
{
    afs_size_type sizeDone;
    LOWLEVELINTERNAL* pLowLevelIntern = (LOWLEVELINTERNAL*)llFile;

    if (pLowLevelIntern->pvfio->pvfi->fIsPermanentPointer)
        return 0;
    sizeDone=memCheckSize(pLowLevelIntern,size_reserv,TRUE,FALSE);
    return (sizeDone >= size_reserv);
}

size_t ABSTRACT_CALLBACK_UNITEX memLowLevelWrite(ABSTRACTFILE_PTR llFile, void const *Buf, size_t size,void*)
{
    LOWLEVELINTERNAL* pLowLevelIntern = (LOWLEVELINTERNAL*)llFile;

    if (pLowLevelIntern->pvfio->pvfi->fIsPermanentPointer)
        return 0;
    size=memCheckSize(pLowLevelIntern,size,TRUE,TRUE);
    pLowLevelIntern->pvfio->pos+=size;
    if (pLowLevelIntern->pvfio->pvfi->posLastWrite < pLowLevelIntern->pvfio->pos)
        pLowLevelIntern->pvfio->pvfi->posLastWrite = pLowLevelIntern->pvfio->pos;

    if (pLowLevelIntern->pvfio->pvfi->dfFileSize < pLowLevelIntern->pvfio->pvfi->posLastWrite)
        pLowLevelIntern->pvfio->pvfi->dfFileSize = pLowLevelIntern->pvfio->pvfi->posLastWrite;

    memcpy(pLowLevelIntern->pvfio->pvfi->Buf + pLowLevelIntern->pvfio->pos - size, Buf, size);

    return size;
}

/*
BOOL memLowLevelSetFileSize(ABSTRACTFILE_PTR llFile, afs_size_type Pos)
{
    LOWLEVELINTERNAL* pLowLevelIntern = (LOWLEVELINTERNAL*)llFile;
    return pLowLevelIntern->pvfio->pvfi->dfFileSize == Pos;
}
*/

#if defined(WIN32) && defined(_DEBUG)
#include <windows.h>
#endif

size_t ABSTRACT_CALLBACK_UNITEX memLowLevelRead(ABSTRACTFILE_PTR llFile, void *Buf, size_t size,void*)
{
    LOWLEVELINTERNAL* pLowLevelIntern = (LOWLEVELINTERNAL*)llFile;
    size=memCheckSize(pLowLevelIntern,size,FALSE,FALSE);

#if defined(WIN32) && defined(_DEBUG)
    if (IsBadReadPtr(pLowLevelIntern->pvfio->pvfi->Buf + pLowLevelIntern->pvfio->pos,size))
    {
        OutputDebugStringA("** bad read ptr in memRead\n");
        puts("**bad read ptr\n");
    }
    if (IsBadWritePtr(Buf,size))
    {
        OutputDebugStringA("** bad write ptr in memRead\n");
        puts("**bad write ptr\n");
    }
#endif

    memcpy(Buf,pLowLevelIntern->pvfio->pvfi->Buf + pLowLevelIntern->pvfio->pos,size);
    pLowLevelIntern->pvfio->pos+=size;
    return size;
}

int ABSTRACT_CALLBACK_UNITEX memLowLevelSeek(ABSTRACTFILE_PTR llFile, afs_size_type Pos, int TypeSeek,void*)
{
    int iRet=0;
    LOWLEVELINTERNAL* pLowLevelIntern = (LOWLEVELINTERNAL*)llFile;
    afs_size_type newpos = Pos;

    if (TypeSeek==SEEK_SET)
    {
        if (newpos > pLowLevelIntern->pvfio->pvfi->dfFileSize)
            iRet=EOF;
        else
            newpos = Pos;
    }

    if (TypeSeek==SEEK_END)
    {
        if (newpos > pLowLevelIntern->pvfio->pvfi->dfFileSize)
            iRet=EOF;
        else
            newpos = pLowLevelIntern->pvfio->pvfi->dfFileSize - Pos;
    }

    if (TypeSeek==SEEK_CUR)
    {
        if (pLowLevelIntern->pvfio->pos + Pos > pLowLevelIntern->pvfio->pvfi->dfFileSize)
            iRet=EOF;
        else
            newpos = pLowLevelIntern->pvfio->pos + Pos;
    }

    if (iRet == 0)
      pLowLevelIntern->pvfio->pos=newpos;
    return iRet;
}

void ABSTRACT_CALLBACK_UNITEX memLowLevelGetSize(ABSTRACTFILE_PTR llFile, afs_size_type* pPos,void*)
{
    LOWLEVELINTERNAL* pLowLevelIntern = (LOWLEVELINTERNAL*)llFile;
    if (pPos!=NULL)
        *pPos = pLowLevelIntern->pvfio->pvfi->dfFileSize;
}


void ABSTRACT_CALLBACK_UNITEX memLowLevelTell(ABSTRACTFILE_PTR llFile, afs_size_type* pPos,void*)
{
    LOWLEVELINTERNAL* pLowLevelIntern = (LOWLEVELINTERNAL*)llFile;
    if (pPos!=NULL)
        *pPos = pLowLevelIntern->pvfio->pos;
}

const void* ABSTRACT_CALLBACK_UNITEX memLowLevel_getMapPointer(ABSTRACTFILE_PTR llFile, afs_size_type pos, afs_size_type len,int,afs_size_type,void* )
{
    LOWLEVELINTERNAL* pLowLevelIntern = (LOWLEVELINTERNAL*)llFile;
    if ((pos == 0) && (len == 0))
        len = pLowLevelIntern->pvfio->pvfi->dfFileSize;
    if (pos + len > pLowLevelIntern->pvfio->pvfi->dfFileSize)
        return NULL;
    return (const void*)(((const char*)pLowLevelIntern->pvfio->pvfi->Buf) + pos);
}

#ifdef TRY_AUTOLOAD_PERSISTANT
BOOL TryAutoUnloadPersistant(const char* dfFileName)
{
    BOOL res=FALSE;
      size_t lenFileName = strlen(dfFileName);
      if ((lenFileName > 4) && ((*dfFileName) == '#'))
      {
          const char* ext= dfFileName + lenFileName - 4;
          if (strcmp(ext,".bin") || strcmp(ext,".inf"))
          {
              char* dupname = strdup(dfFileName);
              strcpy(dupname+lenFileName-4,".bin");
              res=DeleteVirtualDic(dupname);
              free(dupname);
          }

          if (strcmp(ext,".fst2"))
          {
              res=DeleteVirtualGraphFst2(dfFileName,1);
          }
      }
      return res;
}

BOOL TryAutoLoadPersistent(const char*dfFileName)
{
    BOOL res=FALSE;
      size_t lenFileName = strlen(dfFileName);
      if ((lenFileName > 4) && ((*dfFileName) == '#'))
      {
          const char* ext= dfFileName + lenFileName - 4;
          if ((strcmp(ext,".bin")==0) || (strcmp(ext,".inf")==0))
          {
              char* dupname = strdup(dfFileName);
              strcpy(dupname+lenFileName-4,".bin");
              res=LoadVirtualDicFromVirtualFile(dupname,dupname,TRUE);
              free(dupname);
          }

          if (strcmp(ext,".fst2")==0)
          {
              res=LoadVirtualGraphFst2FromVirtualFile(dfFileName,dfFileName,1);
          }
      }
      return res;
}
#endif

int ABSTRACT_CALLBACK_UNITEX memLowLevelClose(ABSTRACTFILE_PTR llFile,void*)
{
  LOWLEVELINTERNAL* pLowLevelIntern = (LOWLEVELINTERNAL*)llFile;

#ifdef TRY_AUTOLOAD_PERSISTANT
  const char* dfFileName=pLowLevelIntern->dfFileName;
  if (dfFileName != NULL)
      if (pLowLevelIntern->fReadOnlyAccess == FALSE)
          TryAutoLoadPersistent(dfFileName);
#endif

  GET_MUTEX;
#ifdef DEBUG_ASSERT_USED
  DEBUG_ASSERT(pLowLevelIntern->pvfio->pvfi->dfNbOpen>0,"VFS Warning: NB opened is not positive!",pLowLevelIntern->pvfio->pvfi->dfVirtualFileName);
#endif
  pLowLevelIntern->pvfio->pvfi->dfNbOpen--;
  RELEASE_MUTEX;

  if (pLowLevelIntern->dfFileName!=NULL)
    DfsFree(pLowLevelIntern->dfFileName);
  if (pLowLevelIntern->pvfio!=NULL)
      DfsFree(pLowLevelIntern->pvfio);
  DfsFree(pLowLevelIntern);
  return 0;
}

ULB_VFFUNC BOOL ULIB_CALL BuildMemFileFromMemoryBufferUsrPtrWithPrefixOrPermanent(const char* FileName,
                                                       const void* pBufPrefix, afs_size_type size_FilePrefix,
                                                       const void* pBufSuffix, afs_size_type size_FileSuffix,
                                                       BOOL fIsPermanentPointer,
                                                       const void*pUserPtr)
{
    VIRTUALFILEITEM vfiSearch;
    VIRTUALFILEITEMINFO vfiSearchInfo;
    //VIRTUALFILEITEM* pVfiFile;
    DWORD dwPosItem=0;
    BOOL fCreatingFile = FALSE;
    STATICARRAYC sacVirtualFileNameSpace;
    BOOL fItemFound;
    if (!InitVirtualFileNameSpace())
        return FALSE;

    if ((fIsPermanentPointer) && (size_FilePrefix != 0))
        return FALSE;

    GET_MUTEX;

    sacVirtualFileNameSpace=GetSacVirtualFileNameSpace();

    vfiSearchInfo.dfVirtualFileName = FileName;
    vfiSearch.pvfii = &vfiSearchInfo;

    fItemFound = FindSameElemPos(sacVirtualFileNameSpace, &vfiSearch, &dwPosItem);


    if (fItemFound)
    {
#ifdef DEBUG_ASSERT_USED
        VIRTUALFILEITEM* pVfiFileToDeleteCheck=(VIRTUALFILEITEM*)GetElemPtr(sacVirtualFileNameSpace,dwPosItem);
        DEBUG_ASSERT(pVfiFileToDeleteCheck->pvfii->dfNbOpen == 0,"VFS Warning: try delete opened file",pVfiFileToDeleteCheck->pvfii->dfVirtualFileName);
#endif

        BOOL fRetDel = DeleteElem(sacVirtualFileNameSpace,dwPosItem,1);
        if (!fRetDel)
        {
            RELEASE_MUTEX;
            return FALSE;
        }
    }


    {
        VIRTUALFILEITEM vfiAdd;

        vfiAdd.pvfii=(VIRTUALFILEITEMINFO*)DfsMalloc(sizeof(VIRTUALFILEITEMINFO));
        vfiAdd.pvfii->dfVirtualFileName = strcpyAlloc(FileName);
        vfiAdd.pvfii->dfFileSize = size_FilePrefix + size_FileSuffix;
        vfiAdd.pvfii->dfFileSizeAlloc = max(vfiAdd.pvfii->dfFileSize,GRANULARITY_ALLOC_BUFFER_FILE);
        vfiAdd.pvfii->posLastWrite = 0;
        vfiAdd.pvfii->fIsPermanentPointer = fIsPermanentPointer;
        vfiAdd.pvfii->pUserPtr = pUserPtr;
        vfiAdd.pvfii->dfNbOpen = 0;

        if (fIsPermanentPointer)
        {
            vfiAdd.pvfii->Buf = (dfbytep)pBufSuffix;
            vfiAdd.pvfii->posLastWrite = size_FileSuffix;
        }
        else
        {
            afs_size_type posWriteBuf = 0;
            vfiAdd.pvfii->Buf = (dfbytep)DfsMalloc((size_t)(vfiAdd.pvfii->dfFileSizeAlloc+SIZE_NULL_PAD_END_FILE));


            if (vfiAdd.pvfii->Buf == NULL)
            {  // mem error
                RELEASE_MUTEX;
                return FALSE;
            }

            if (pBufPrefix != NULL)
            {
                memcpy(vfiAdd.pvfii->Buf+posWriteBuf,pBufPrefix,size_FilePrefix);
                posWriteBuf += size_FilePrefix;
            }

            if (pBufSuffix != NULL)
            {
                memcpy(vfiAdd.pvfii->Buf+posWriteBuf,pBufSuffix,size_FileSuffix);
                posWriteBuf += size_FilePrefix;
            }

            vfiAdd.pvfii->posLastWrite = posWriteBuf;

            pVirtualFileSpace->dfTotalSizeVirtualSpace += posWriteBuf;
        }

        fCreatingFile = InsertSorted(sacVirtualFileNameSpace,&vfiAdd);
        if (!fCreatingFile )
        {
            RELEASE_MUTEX;
            return FALSE;
        }

        if (!FindSameElemPos(sacVirtualFileNameSpace, &vfiSearch, &dwPosItem))
        {
            RELEASE_MUTEX;
            return FALSE;
        }
    }

    RELEASE_MUTEX;
    return TRUE;
}

ULB_VFFUNC BOOL ULIB_CALL BuildMemFileFromMemoryBufferUsrPtr(const char* FileName, const void* pBuf,
                                                   afs_size_type size_File, BOOL fIsPermanentPointer,
                                                   const void*pUserPtr)
{
    return BuildMemFileFromMemoryBufferUsrPtrWithPrefixOrPermanent(FileName,NULL,0,pBuf,size_File,
                 fIsPermanentPointer,pUserPtr);
}

ULB_VFFUNC BOOL ULIB_CALL BuildMemFileFromMemoryBuffer(const char* FileName, const void* pBuf, afs_size_type size_File, BOOL fIsPermanentPointer)
{
    return BuildMemFileFromMemoryBufferUsrPtr(FileName,pBuf,size_File,fIsPermanentPointer,NULL);
}


ULB_VFFUNC BOOL ULIB_CALL BuildMemFileFromMemoryBufferWithPrefix(const char* FileName,
                                                       const void* pBufPrefix, afs_size_type size_FilePrefix,
                                                       const void* pBuf, afs_size_type size_File)
{
    return BuildMemFileFromMemoryBufferUsrPtrWithPrefixOrPermanent(FileName,
        pBufPrefix,size_FilePrefix,pBuf,size_File,FALSE,NULL);
}

ULB_VFFUNC BOOL ULIB_CALL BuildMemFileFromMemoryBufferWithPrefixUsrPtr(const char* FileName,
                                                             const void* pBufPrefix, afs_size_type size_FilePrefix,
                                                             const void* pBuf, afs_size_type size_File,
                                                             const void*pUserPtr)
{
    return BuildMemFileFromMemoryBufferUsrPtrWithPrefixOrPermanent(FileName,
        pBufPrefix,size_FilePrefix,pBuf,size_File,FALSE,pUserPtr);
}

static VIRTUALFILEITEM* GetpVfiFileFromFileName(const char* FileName)
{
    VIRTUALFILEITEM vfiSearch;
    VIRTUALFILEITEMINFO vfiSearchInfo;
    VIRTUALFILEITEM* pVfiFile=NULL;
    DWORD dwPosItem=0;
    STATICARRAYC sacVirtualFileNameSpace;
    BOOL fItemFound;
    if (!InitVirtualFileNameSpace())
        return FALSE;

    sacVirtualFileNameSpace=GetSacVirtualFileNameSpace();

    vfiSearchInfo.dfVirtualFileName = FileName;
    vfiSearch.pvfii = &vfiSearchInfo;

    fItemFound = FindSameElemPos(sacVirtualFileNameSpace, &vfiSearch, &dwPosItem);
    if (fItemFound)
        pVfiFile=(VIRTUALFILEITEM*)GetElemPtr(sacVirtualFileNameSpace,dwPosItem);

    return pVfiFile;
}

static VIRTUALFILEITEMINFO* GetpVfiiFileFromFileName(const char* FileName)
{
    VIRTUALFILEITEMINFO* pVfiiFile = NULL;
    VIRTUALFILEITEM* pVfiFile;

    /* when other file are created, pVfiFile can change, but not pVfiiFile */

    pVfiFile = GetpVfiFileFromFileName(FileName);
    if (pVfiFile != NULL)
        pVfiiFile = pVfiFile->pvfii;

    return pVfiiFile;
}

static void AddNullAfterNoPermanentPointerMemFile(VIRTUALFILEITEMINFO* pVfiiFile)
{
    /* if we have no permanent pointer, we add 8 null char after the file, if the file contain
       text file and user want a null terminated string
       Probably 2 char is ok, but the cost of 8 instead 2 is not big.
       memCheckSize does the work */
    if (!(pVfiiFile->fIsPermanentPointer))
    {
        int i;
        for (i=0;i<SIZE_NULL_PAD_END_FILE;i++)
            *((pVfiiFile->Buf)+(pVfiiFile->dfFileSize)+i) = 0;
    }
}

ULB_VFFUNC BOOL ULIB_CALL GetMemoryBufferFromMemFileUsrPtr(const char* FileName, const void** ppBuf,
                                                 afs_size_type* size_File,
                                                 const void**ppUserPtr)
{
    VIRTUALFILEITEMINFO* pVfiiFile;

    if (!InitVirtualFileNameSpace())
        return FALSE;

    GET_MUTEX;

    pVfiiFile=GetpVfiiFileFromFileName(FileName);
    if (pVfiiFile == NULL)
    {
        RELEASE_MUTEX;
        return FALSE;
    }

    if (size_File != NULL)
        *size_File = pVfiiFile->dfFileSize;
    if (ppBuf != NULL)
    {
        /* if we have no permanent pointer, we add 8 null char after the file, if the file contain
           text file and user want a null terminated string
           Probably 2 char is ok, but the cost of 8 instead 2 is not big.
           memCheckSize does the work */

        AddNullAfterNoPermanentPointerMemFile(pVfiiFile);
        *ppBuf = pVfiiFile->Buf;
    }
    if (ppUserPtr!=NULL)
        *ppUserPtr = pVfiiFile->pUserPtr;
    RELEASE_MUTEX;
    return TRUE;
}

ULB_VFFUNC BOOL ULIB_CALL GetMemoryBufferFromMemFile(const char* FileName, const void** ppBuf, afs_size_type* size_File)
{
    return GetMemoryBufferFromMemFileUsrPtr(FileName,ppBuf,size_File,NULL);
}

ULB_VFFUNC BOOL ULIB_CALL ZeroMemFile(const char* FileName)
{
    VIRTUALFILEITEM* pVfiFile;

    if (!InitVirtualFileNameSpace())
        return FALSE;

    GET_MUTEX;

    pVfiFile=GetpVfiFileFromFileName(FileName);
    if (pVfiFile == NULL)
    {
        RELEASE_MUTEX;
        return FALSE;
    }

    if (pVfiFile->pvfii->fIsPermanentPointer)
    {
        RELEASE_MUTEX;
        return FALSE;
    }

    pVfiFile->pvfii->dfFileSize = 0;
    pVfiFile->pvfii->posLastWrite = 0;

    RELEASE_MUTEX;

    return TRUE;
}

int ABSTRACT_CALLBACK_UNITEX RenameMemFilePrivParam(const char * _OldFilename, const char * _NewFilename,void*)
{
    VIRTUALFILEITEM vfiSearch;
    VIRTUALFILEITEMINFO vfiSearchInfo;
    DWORD dwPosItem;
    BOOL fDeletingFile;
    VIRTUALFILEITEM* pVfiFileToRename;
    STATICARRAYC sacVirtualFileNameSpace=GetSacVirtualFileNameSpace();
    VIRTUALFILEITEMINFO * pvfiiFileRenamed;

    vfiSearchInfo.dfVirtualFileName = _OldFilename;
    vfiSearch.pvfii = &vfiSearchInfo;

    GET_MUTEX;
    if ((GetpVfiFileFromFileName(_NewFilename) != NULL) ||
        (!FindSameElemPos(sacVirtualFileNameSpace, &vfiSearch, &dwPosItem)))
    {
        RELEASE_MUTEX;
        return -1;
    }

    pVfiFileToRename=(VIRTUALFILEITEM*)GetElemPtr(sacVirtualFileNameSpace,dwPosItem);

    pvfiiFileRenamed = pVfiFileToRename->pvfii ;

#ifdef DEBUG_ASSERT_USED
    VIRTUALFILEITEM* pVfiFileToDeleteCheck=(VIRTUALFILEITEM*)GetElemPtr(sacVirtualFileNameSpace,dwPosItem);
    DEBUG_ASSERT(pVfiFileToDeleteCheck->pvfii->dfNbOpen == 0,"VFS Warning: try rename opened file",pVfiFileToDeleteCheck->pvfii->dfVirtualFileName);
#endif

    // we want to remove the item, but not call destructor !
    pVfiFileToRename->pvfii = NULL ;

    fDeletingFile = DeleteElem(sacVirtualFileNameSpace, dwPosItem, 1);

    if (pvfiiFileRenamed -> dfVirtualFileName != NULL)
        DfsFree((dfvoidp)(pvfiiFileRenamed -> dfVirtualFileName));
    pvfiiFileRenamed -> dfVirtualFileName = strcpyAlloc(_NewFilename);

    VIRTUALFILEITEM vfiAdd;
    vfiAdd.pvfii = pvfiiFileRenamed;
    BOOL fInsertingFile = InsertSorted(sacVirtualFileNameSpace,&vfiAdd);

    RELEASE_MUTEX;
    return (((fInsertingFile != FALSE) && (fDeletingFile != FALSE)) ? 0 : -1);
}

ULB_VFFUNC BOOL ULIB_CALL RenameMemFile(const char * _OldFilename, const char * _NewFilename)
{
    return (RenameMemFilePrivParam(_OldFilename, _NewFilename,NULL)==0);
}

ULB_VFFUNC BOOL ULIB_CALL SetUserPtrFile(const char* FileName,const void*pUserPtr)
{
    VIRTUALFILEITEM* pVfiFile;

    if (!InitVirtualFileNameSpace())
        return FALSE;

    GET_MUTEX;

    pVfiFile=GetpVfiFileFromFileName(FileName);
    if (pVfiFile == NULL)
    {
        RELEASE_MUTEX;
        return FALSE;
    }


    pVfiFile->pvfii->pUserPtr = pUserPtr;

    RELEASE_MUTEX;

    return TRUE;
}


ULB_VFFUNC const void* ULIB_CALL GetUserPtrFile(const char* FileName)
{
    VIRTUALFILEITEM* pVfiFile;
    const void* ret;

    if (!InitVirtualFileNameSpace())
        return NULL;

    GET_MUTEX;

    pVfiFile=GetpVfiFileFromFileName(FileName);
    if (pVfiFile == NULL)
    {
        RELEASE_MUTEX;
        return NULL;
    }


    ret = pVfiFile->pvfii->pUserPtr ;

    RELEASE_MUTEX;

    return ret;
}

ULB_VFFUNC void ULIB_CALL ClearVirtualFiles()
{
    GetSacVirtualFileNameSpace();
    UnInitVirtualFileNameSpace(TRUE);
}

/***************************************************************************/


size_t ABSTRACT_CALLBACK_UNITEX fnc_stdOutWrite_VirtualFile(void const *Buf, size_t size,void* privatePtr)
{
    if (privatePtr != NULL)
    {
        ABSTRACTFILE_PTR paf = (ABSTRACTFILE_PTR)privatePtr;
        if (size > 0)
            memLowLevelWrite(paf,Buf,size,NULL);
        else if (size == 0)
            memLowLevelClose(paf,NULL);
    }
    return size;
}

BOOL SetStdWriteFile(const char*FileName,BOOL fTrashOutput,enum stdwrite_kind swk)
{
    void *new_private_ptr_file = NULL;

    {
        t_fnc_stdOutWrite prev_fnc_stdOutWrite=NULL;
        void *prev_private_ptr = NULL;
        if (GetStdWriteCB(swk, NULL, &prev_fnc_stdOutWrite,&prev_private_ptr) == 0)
            return FALSE;
/*
        if ((prev_fnc_stdOutWrite == fnc_stdOutWrite_VirtualFile) && (prev_private_ptr!=NULL))
        {
            memLowLevelClose((ABSTRACTFILE_PTR)prev_private_ptr,NULL);
        }
        */
    }

    BOOL fRet = TRUE;
    if ((FileName != NULL) && (fTrashOutput==FALSE))
    {
        ABSTRACTFILE_PTR writeFile;

        writeFile = memOpenLowLevel(FileName, OPEN_READWRITE_MF,NULL);

        if (writeFile != NULL)
            memLowLevelSeek(writeFile, 0, SEEK_END,NULL);
        else
            writeFile = memOpenLowLevel(FileName, OPEN_CREATE_MF,NULL);

        if (writeFile != NULL)
            new_private_ptr_file = (void*)writeFile;
        else
            fRet = FALSE;
    }

    SetStdWriteCB(swk,
                  fTrashOutput ? 1 : 0,
                  (new_private_ptr_file != NULL) ? fnc_stdOutWrite_VirtualFile : NULL,
                  new_private_ptr_file);
    return fRet;
}

ULB_VFFUNC BOOL ULIB_CALL SetStdOutFile(const char* FileName, BOOL fTrashOutput)
{
    return SetStdWriteFile(FileName,fTrashOutput,stdwrite_kind_out);
}

ULB_VFFUNC BOOL ULIB_CALL SetStdErrFile(const char* FileName, BOOL fTrashOutput)
{
    return SetStdWriteFile(FileName,fTrashOutput,stdwrite_kind_err);
}


/*
// now this standard output is moved in mystdio
static ABSTRACTFILE_PTR GetLLF_Std(BOOL *pfTrashOutput,int numFile)
{
ABSTRACTFILE_PTR llRet ;
BOOL fTrashRet;

    if (pVirtualFileSpace == NULL)
    {
        llRet = NULL;
        fTrashRet = FALSE;
    }
    else
    {
        llRet = pVirtualFileSpace->std_out_array[numFile].llfStd;
        fTrashRet = pVirtualFileSpace->std_out_array[numFile].fTrashOutput;
    }
    if (pfTrashOutput!=NULL)
        *pfTrashOutput=fTrashRet;
    return llRet;
}

ABSTRACTFILE_PTR GetLLF_StdOut(BOOL *pfTrashOutput)
{
    return GetLLF_Std(pfTrashOutput,STANDARD_OUT_STDOUT);
}

ABSTRACTFILE_PTR GetLLF_StdErr(BOOL *pfTrashOutput)
{
    return GetLLF_Std(pfTrashOutput,STANDARD_OUT_STDERR);
}

static BOOL SetStdFile(const char* FileName, BOOL fTrashOutput,int numFile)
{
    BOOL fRet = TRUE;
    if (pVirtualFileSpace->std_out_array[numFile].llfStd != NULL)
      memLowLevelClose(pVirtualFileSpace->std_out_array[numFile].llfStd);
    pVirtualFileSpace->std_out_array[numFile].llfStd = NULL;
    pVirtualFileSpace->std_out_array[numFile].fTrashOutput = FALSE;

    if (FileName!=NULL)
    {
        pVirtualFileSpace->std_out_array[numFile].llfStd = memOpenLowLevel(FileName, OPEN_READ_MFWRITE,FALSE,0);
        if (pVirtualFileSpace->std_out_array[numFile].llfStd != NULL)
            memLowLevelSeek(pVirtualFileSpace->std_out_array[numFile].llfStd, 0, TYPESEEK_END);
        else
            pVirtualFileSpace->std_out_array[numFile].llfStd = memOpenLowLevel(FileName, OPEN_CREATE_MF,FALSE,0);
        fRet = (pVirtualFileSpace->std_out_array[numFile].llfStd) != NULL;
    }
    else
        pVirtualFileSpace->std_out_array[numFile].fTrashOutput = fTrashOutput;

    return fRet;
}

BOOL SetStdOutFile(const char* FileName, BOOL fTrashOutput)
{
    return SetStdFile(FileName,fTrashOutput,STANDARD_OUT_STDOUT);
}

BOOL SetStdErrFile(const char* FileName, BOOL fTrashOutput)
{
    return SetStdFile(FileName,fTrashOutput,STANDARD_OUT_STDERR);
}
*/

ULB_VFFUNC BOOL ULIB_CALL InitMemFileEnumerationEx(ENUM_VIRTUAL_FILE* pevf, afs_size_type* p_nb_item )
{
    if (p_nb_item != NULL)
        *p_nb_item = 0;
    if (!InitVirtualFileNameSpace())
        return FALSE;

    pevf->dwReservedMagicValue = 0;
    GET_MUTEX;
    if (p_nb_item != NULL)
    {
        STATICARRAYC sacVirtualFileNameSpace=GetSacVirtualFileNameSpace();
        *p_nb_item = (afs_size_type) GetNbElem(sacVirtualFileNameSpace);
    }
    return TRUE;
}

ULB_VFFUNC BOOL ULIB_CALL InitMemFileEnumeration(ENUM_VIRTUAL_FILE* pevf)
{
    return InitMemFileEnumerationEx(pevf, NULL);
}

ULB_VFFUNC BOOL ULIB_CALL GetNextMemFileEnumeration(ENUM_VIRTUAL_FILE* pevf)
{
    STATICARRAYC sacVirtualFileNameSpace;
    VIRTUALFILEITEM* pVfiFile;
    VIRTUALFILEITEMINFO* pVfiiFile;

    if (!InitVirtualFileNameSpace())
        return FALSE;


    sacVirtualFileNameSpace=GetSacVirtualFileNameSpace();
    pVfiFile = (VIRTUALFILEITEM*)GetElemPtr(sacVirtualFileNameSpace,pevf->dwReservedMagicValue);
    if (pVfiFile == NULL)
        return FALSE;
    pVfiiFile = pVfiFile->pvfii;

    // GV 2015/09/24 : we can enumerate opened file, so this is dangerous
    // AddNullAfterNoPermanentPointerMemFile(pVfiiFile);

    pevf->FileName = pVfiiFile->dfVirtualFileName;
    pevf->fIsPermanentPointer = pVfiiFile->fIsPermanentPointer;
    pevf->size_File = pVfiiFile->dfFileSize;
    pevf->pBuf = pVfiiFile->Buf;
    pevf->pUserPtr = pVfiiFile->pUserPtr;

    pevf->dwReservedMagicValue++;
    return TRUE;
}

ULB_VFFUNC BOOL ULIB_CALL CloseMemFileEnumeration(ENUM_VIRTUAL_FILE*)
{
    if (!InitVirtualFileNameSpace())
        return FALSE;

    RELEASE_MUTEX;
    return TRUE;
}



BOOL MyMkdirStepByStep(const char*dirname)
{
  if ((*dirname)!='\0')
  {
      char* dupname=strdup(dirname);
      char* lpParc = dupname;

      if ((*lpParc)!=0)
          lpParc++;

      while ((*lpParc)!=0)
      {
          if ( ((*(lpParc))=='\\')|| ((*(lpParc))=='/') )
              {
                  char c=*(lpParc);
                  *(lpParc) = '\0';
                  mkdirAtomic (dupname);
                  *(lpParc) = c;
              }

          lpParc++;
      }
      free(dupname);
  }

  return mkdirAtomic(dirname);
}

#ifdef _NOT_UNDER_WINDOWS
   #define FILEPATH_SEPARATOR_CHAR '/'
   #define FILEPATH_SEPARATOR_STRING "/"
#else
   #define FILEPATH_SEPARATOR_CHAR '\\'
   #define FILEPATH_SEPARATOR_STRING "\\"
#endif


ULB_VFFUNC BOOL ULIB_CALL DumpMemFileOnDisk(const char* DestDir)
{
    ENUM_VIRTUAL_FILE evf;
    int num=0;
    int lenDestDir=0;
    int need_add_separator =0;
    BOOL fRet=TRUE;

    evf.dwReservedMagicValue = 0;
    evf.FileName = NULL;
    evf.fIsPermanentPointer = FALSE;
    evf.pBuf = NULL;
    evf.pUserPtr = NULL;
    evf.size_File = 0;

    if (DestDir!=NULL)
        lenDestDir = (int)strlen(DestDir);

    if (lenDestDir>0)
        if (((*(DestDir+lenDestDir-1))!='\\') && ((*(DestDir+lenDestDir-1))!='/'))
            need_add_separator=1;

    if (InitMemFileEnumeration(&evf))
    {
        while (GetNextMemFileEnumeration(&evf))
        {
            char* extrName = (char*)malloc(lenDestDir+strlen(evf.FileName)+0x10);
            if (lenDestDir>0)
              strcpy(extrName,DestDir);
            else
                *extrName=0;
            if (need_add_separator!=0)
            strcat(extrName,FILEPATH_SEPARATOR_STRING);
            strcat(extrName,evf.FileName+1);

            char* latestSep=NULL;
            char* extrNameBrowse=extrName;
            while ((*extrNameBrowse)!='\0')
            {
                if (((*extrNameBrowse)=='/') || ((*extrNameBrowse)=='\\'))
                    latestSep=extrNameBrowse;
                extrNameBrowse++;
            }

            if (extrNameBrowse!=NULL)
            {
                char cSave=*latestSep;
                *latestSep=0;
                MyMkdirStepByStep(extrName);
                *latestSep=cSave;
            }



            BOOL fSuccessThis=FALSE;
            ABSTRACTFILE* vfWrite;

            vfWrite = af_fopen(extrName,"wb");
            if (vfWrite != NULL)
            {
                int iWriteDone = (int)af_fwrite(evf.pBuf,1,evf.size_File,vfWrite);
                if (iWriteDone==(int)evf.size_File)
                    fSuccessThis = TRUE;
                af_fclose(vfWrite);
            }

            if (fSuccessThis==FALSE)
                fRet=FALSE;

            /*printf("filename = %40s, size=%u, Permanent:%c, extracted to %s : %c\n",evf.FileName,(unsigned int)evf.size_File,
                evf.fIsPermanentPointer?'y':'n',extrName,fSuccessThis?'y':'n');*/
            free(extrName);
            num++;
        }
        CloseMemFileEnumeration(&evf);
    }
    /*printf("number of mem file : %u\n\n",num);*/
    return fRet;
}





ULB_VFFUNC BOOL ULIB_CALL DeleteMemFileCurrentlyEnumerated(ENUM_VIRTUAL_FILE* pevf)
{
    STATICARRAYC sacVirtualFileNameSpace;
    BOOL fRet;

    /* 0 : we have returned nothing ! */
    /* pevf->dwReservedMagicValue contain the item number of the next to be returned,
        so the last returned item is pevf->dwReservedMagicValue -1 */
    if (pevf->dwReservedMagicValue == 0)
        return FALSE;

    if (!InitVirtualFileNameSpace())
        return FALSE;

    sacVirtualFileNameSpace=GetSacVirtualFileNameSpace();

#ifdef DEBUG_ASSERT_USED
    VIRTUALFILEITEM* pVfiFileToDeleteCheck=(VIRTUALFILEITEM*)GetElemPtr(sacVirtualFileNameSpace,pevf->dwReservedMagicValue-1);
    DEBUG_ASSERT(pVfiFileToDeleteCheck->pvfii->dfNbOpen == 0,"VFS Warning: try delete opened file",pVfiFileToDeleteCheck->pvfii->dfVirtualFileName);
#endif

    fRet = DeleteElem(sacVirtualFileNameSpace,pevf->dwReservedMagicValue-1,1);
    if (fRet)
        pevf->dwReservedMagicValue--;
    return fRet;
}


int ABSTRACT_CALLBACK_UNITEX IsVirtualFileNever(const char * _Filename,void*)
{
    if (_Filename != NULL)
        if (((*(_Filename)) == '*') || ((*(_Filename)) == '#'))
            return 1;

#ifdef MODERN_VFS_REPLACE_MINI_VFS
	if (_Filename != NULL)
		if ((*(_Filename)) == '$')
			if ((*(_Filename + 1)) == ':')
			{
				if ((*(_Filename + 2)) == '$')
				{
					if ((*(_Filename + 3)) == ':')
						return 0;
				}
				return 1;
			}
#endif

    return 0;
}

/* can be optimized */

ULB_VFFUNC BOOL ULIB_CALL DeleteMemFileForPrefix(const char* szPrefix)
{
    BOOL fRes = FALSE;
    size_t len_prefix=strlen(szPrefix);
	ENUM_VIRTUAL_FILE enumvf;
	if (InitMemFileEnumeration(&enumvf)) {
		while (GetNextMemFileEnumeration(&enumvf)) {
            size_t len_filename=strlen(enumvf.FileName);
            if (len_filename>=len_prefix)
                if (memcmp(szPrefix,enumvf.FileName,len_prefix)==0)
                {
                    DeleteMemFileCurrentlyEnumerated(&enumvf);
                    fRes = TRUE;
                }
		}
		CloseMemFileEnumeration(&enumvf);
	}
    return fRes;
}

#ifdef HAS_FILE_FUNC_ARRAY_GETLIST


void ABSTRACT_CALLBACK_UNITEX memLowLevelReleaseList(char**list,void*)
{
	if (list==NULL)
		return;

	char** list_walk=list;
	while ((*list_walk)!=NULL)
	{
		free(*list_walk);
		list_walk++;
	}
	free(list);
}

char** ABSTRACT_CALLBACK_UNITEX memLowLevelGetList(void*)
{
    ENUM_VIRTUAL_FILE evf;
    evf.dwReservedMagicValue = 0;
    evf.FileName = NULL;
    evf.fIsPermanentPointer = FALSE;
    evf.pBuf = NULL;
    evf.pUserPtr = NULL;
    evf.size_File = 0;
	int num=0;
	char**list=NULL;


    if (InitMemFileEnumeration(&evf))
    {
        while (GetNextMemFileEnumeration(&evf))
        {
			char**newlist;
			if (num==0)
				newlist=(char**)malloc(2*sizeof(char*));
			else
				newlist=(char**)realloc(list,(num+2)*(sizeof(char*)));
			if (newlist == NULL)
			{
				if (list!=NULL)
				{
					memLowLevelReleaseList(list,NULL);
				}
				CloseMemFileEnumeration(&evf);
				return NULL;
			}
			list=newlist;
            char* extrName = (char*)malloc(strlen(evf.FileName)+0x1);
            strcpy(extrName,evf.FileName);
			*(list+num)=extrName;
			num++;
			*(list+num)=NULL;
        }
        CloseMemFileEnumeration(&evf);
    }

    return list;
}

typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_memFile_releaseList)(char** listFile, void* privateSpacePtr);

const t_fileio_func_array_extensible fileio_func_array_extensible =
{   sizeof(t_fileio_func_array_extensible),
    &IsVirtualFileNever,
    NULL,NULL,
    &memOpenLowLevel,
    &memLowLevelWrite,
    &memLowLevelRead,
    &memLowLevelSeek,
    &memLowLevelGetSize,
    &memLowLevelTell,
    &memLowLevelClose,
    &memLowLevelSetSizeReservation,
    &DeleteMemFilePrivParam,
    &RenameMemFilePrivParam,
    &memLowLevel_getMapPointer,NULL,
	memLowLevelGetList,memLowLevelReleaseList
};


class autoinstall_vf {
public:
    autoinstall_vf() {
        InitVirtualFileNameSpace();
        AddAbstractFileSpaceExtensible(&fileio_func_array_extensible,this);
    };

    ~autoinstall_vf() {
        RemoveAbstractFileSpaceExtensible(&fileio_func_array_extensible,this);
        CleanVirtualFileNameSpaceAtExit();
    };

} ;


#else

const t_fileio_func_array_ex fileio_func_array_ex =
{   &IsVirtualFileNever,
    NULL,NULL,
    &memOpenLowLevel,
    &memLowLevelWrite,
    &memLowLevelRead,
    &memLowLevelSeek,
    &memLowLevelGetSize,
    &memLowLevelTell,
    &memLowLevelClose,
    &memLowLevelSetSizeReservation,
    &DeleteMemFilePrivParam,
    &RenameMemFilePrivParam,
    &memLowLevel_getMapPointer,NULL
};


class autoinstall_vf {
public:
    autoinstall_vf() {
        AddAbstractFileSpaceEx(&fileio_func_array_ex,this);
    };

    ~autoinstall_vf() {
        RemoveAbstractFileSpaceEx(&fileio_func_array_ex,this);
    };

} ;
#endif

autoinstall_vf autoinstall_vf_instance;
