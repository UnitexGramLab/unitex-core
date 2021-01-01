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
#if (((defined(_WIN32)) || defined(WIN32)))

#include <windows.h>
#include "MiniMutex.h"

/* Mutex implementation for Win32 API */
/* see http://msdn.microsoft.com/en-us/library/ms686927(VS.85).aspx */


/*
ULB_VFFUNC MINIMUTEX_OBJECT* ULIB_CALL BuildMutex()
{
    HANDLE hMutex;
    hMutex = CreateMutex(NULL,FALSE,NULL);
    if (hMutex == INVALID_HANDLE_VALUE)
        hMutex = NULL;

    return (MINIMUTEX_OBJECT*)hMutex;
}

ULB_VFFUNC void ULIB_CALL GetMiniMutex(MINIMUTEX_OBJECT* pMut)
{
    HANDLE hMutex = (HANDLE)pMut;
    if (hMutex != NULL)
        WaitForSingleObject(hMutex,INFINITE);
}

ULB_VFFUNC void ULIB_CALL ReleaseMiniMutex(MINIMUTEX_OBJECT* pMut)
{
    HANDLE hMutex = (HANDLE)pMut;
    if (hMutex != NULL)
        ReleaseMutex(hMutex);
}

ULB_VFFUNC void ULIB_CALL DeleteMiniMutex(MINIMUTEX_OBJECT* pMut)
{
    HANDLE hMutex = (HANDLE)pMut;
    if (hMutex != NULL)
        CloseHandle(hMutex);
}
*/

typedef struct
{
    CRITICAL_SECTION cs;
} SYNC_MUTEX_OBJECT_VFS_INTERNAL;

ULB_VFFUNC MINIMUTEX_OBJECT* ULIB_CALL BuildMutex()
{
    SYNC_MUTEX_OBJECT_VFS_INTERNAL* pMoi = (SYNC_MUTEX_OBJECT_VFS_INTERNAL*)malloc(sizeof(SYNC_MUTEX_OBJECT_VFS_INTERNAL));
    if (pMoi == NULL)
        return NULL;
#ifdef UNITEX_USING_WINRT_API
    InitializeCriticalSectionEx(&(pMoi->cs),0,0);
#else
    InitializeCriticalSection(&(pMoi->cs));
#endif

    return (MINIMUTEX_OBJECT*)pMoi;
}

ULB_VFFUNC void ULIB_CALL GetMiniMutex(MINIMUTEX_OBJECT* pMut)
{
    SYNC_MUTEX_OBJECT_VFS_INTERNAL* pMoi = (SYNC_MUTEX_OBJECT_VFS_INTERNAL*)pMut;
    if (pMoi != NULL)
        EnterCriticalSection(&pMoi->cs);
}

ULB_VFFUNC void ULIB_CALL ReleaseMiniMutex(MINIMUTEX_OBJECT* pMut)
{
    SYNC_MUTEX_OBJECT_VFS_INTERNAL* pMoi = (SYNC_MUTEX_OBJECT_VFS_INTERNAL*)pMut;
    if (pMut != NULL)
        LeaveCriticalSection(&pMoi->cs);
}

ULB_VFFUNC void ULIB_CALL DeleteMiniMutex(MINIMUTEX_OBJECT* pMut)
{
    SYNC_MUTEX_OBJECT_VFS_INTERNAL* pMoi = (SYNC_MUTEX_OBJECT_VFS_INTERNAL*)pMut;
    if (pMoi != NULL)
    {
        DeleteCriticalSection(&pMoi->cs);
        free(pMoi);
    }
}


#endif
