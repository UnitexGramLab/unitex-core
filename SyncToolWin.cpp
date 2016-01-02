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




#include "Unicode.h"
#include "AbstractCallbackFuncModifier.h"
#include "SyncTool.h"

#if (defined(_WIN32)) || defined(WIN32)

#include <windows.h>
#include <windowsx.h>

#if defined(WINAPI_FAMILY_PARTITION) && (!(defined(UNITEX_USING_WINRT_API)))
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#if (!(defined(UNITEX_USING_WINRT_API))) && (!(defined(UNITEX_PREVENT_USING_WINRT_API)))
#define UNITEX_USING_WINRT_API 1
#endif
#endif
#endif


#endif

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {


#if (defined(_WIN32)) || defined(WIN32)

typedef struct
{
    DWORD Tick;
} TIMEBEGIN;

UNITEX_FUNC hTimeElapsed UNITEX_CALL SyncBuidTimeMarkerObject()
{
    TIMEBEGIN* pBegin = (TIMEBEGIN*)malloc(sizeof(TIMEBEGIN));
#ifdef UNITEX_USING_WINRT_API
    pBegin->Tick = (DWORD)GetTickCount64();
#else
    pBegin->Tick = GetTickCount();
#endif
    return pBegin;
}


UNITEX_FUNC unsigned int UNITEX_CALL SyncGetMSecElapsedNotDestructive(hTimeElapsed ptr, int destructObject)
{
    TIMEBEGIN tBegin;
    TIMEBEGIN tEnd;
    TIMEBEGIN* pBegin = (TIMEBEGIN*)ptr;
    unsigned int iRet;
    tBegin = *pBegin;
#ifdef UNITEX_USING_WINRT_API
    tEnd.Tick = (DWORD)GetTickCount64();
#else
    tEnd.Tick = GetTickCount();
#endif
    if (destructObject != 0)
      free(pBegin);


    iRet = (unsigned int)(tEnd.Tick - tBegin.Tick) ;
    return iRet;
}

UNITEX_FUNC unsigned int UNITEX_CALL SyncGetMSecElapsed(hTimeElapsed ptr)
{
    return SyncGetMSecElapsedNotDestructive(ptr,1);
}


/**************************/



/* Mutex implementation for Win32 API */
/* see http://msdn.microsoft.com/en-us/library/ms686927(VS.85).aspx */

/*
UNITEX_FUNC SYNC_Mutex_OBJECT UNITEX_CALL SyncBuildMutex()
{
    HANDLE hMutex;
    hMutex = CreateMutex(NULL,FALSE,NULL);
    if (hMutex == INVALID_HANDLE_VALUE)
        hMutex = NULL;

    return (SYNC_Mutex_OBJECT)hMutex;
}

UNITEX_FUNC void UNITEX_CALL SyncGetMutex(SYNC_Mutex_OBJECT pMut)
{
    HANDLE hMutex = (HANDLE)pMut;
    if (hMutex != NULL)
        WaitForSingleObject(hMutex,INFINITE);
}

UNITEX_FUNC void UNITEX_CALL SyncReleaseMutex(SYNC_Mutex_OBJECT pMut)
{
    HANDLE hMutex = (HANDLE)pMut;
    if (hMutex != NULL)
        ReleaseMutex(hMutex);
}

UNITEX_FUNC void UNITEX_CALL SyncDeleteMutex(SYNC_Mutex_OBJECT pMut)
{
    HANDLE hMutex = (HANDLE)pMut;
    if (hMutex != NULL)
        CloseHandle(hMutex);
}

*/


typedef struct
{
    CRITICAL_SECTION cs;
} SYNC_MUTEX_OBJECT_INTERNAL;

UNITEX_FUNC SYNC_Mutex_OBJECT UNITEX_CALL SyncBuildMutex()
{
    SYNC_MUTEX_OBJECT_INTERNAL* pMoi = (SYNC_MUTEX_OBJECT_INTERNAL*)malloc(sizeof(SYNC_MUTEX_OBJECT_INTERNAL));
    if (pMoi == NULL)
        return NULL;
#ifdef UNITEX_USING_WINRT_API
    InitializeCriticalSectionEx(&(pMoi->cs),0,0);
#else
    InitializeCriticalSection(&(pMoi->cs));
#endif

    return (SYNC_Mutex_OBJECT)pMoi;
}

UNITEX_FUNC void UNITEX_CALL SyncGetMutex(SYNC_Mutex_OBJECT pMut)
{
    SYNC_MUTEX_OBJECT_INTERNAL* pMoi = (SYNC_MUTEX_OBJECT_INTERNAL*)pMut;
    if (pMoi != NULL)
        EnterCriticalSection(&pMoi->cs);
}

UNITEX_FUNC void UNITEX_CALL SyncReleaseMutex(SYNC_Mutex_OBJECT pMut)
{
    SYNC_MUTEX_OBJECT_INTERNAL* pMoi = (SYNC_MUTEX_OBJECT_INTERNAL*)pMut;
    if (pMut != NULL)
        LeaveCriticalSection(&pMoi->cs);
}

UNITEX_FUNC void UNITEX_CALL SyncDeleteMutex(SYNC_Mutex_OBJECT pMut)
{
    SYNC_MUTEX_OBJECT_INTERNAL* pMoi = (SYNC_MUTEX_OBJECT_INTERNAL*)pMut;
    if (pMoi != NULL)
    {
        DeleteCriticalSection(&pMoi->cs);
        free(pMoi);
    }
}




#endif

} // namespace unitex
