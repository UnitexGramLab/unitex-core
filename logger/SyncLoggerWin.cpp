/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * additional information: http://www.smartversion.com/unitex-contribution/
 * contact : info@winimage.com
 *
 */




#include "Unicode.h"
#include "AbstractCallbackFuncModifier.h"
#include "SyncLogger.h"
#include "UnusedParameter.h"

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

#if defined(UNITEX_USING_WINRT_API) && !(defined(UNITEX_PREVENT_USING_WINRT_THREADEMULATION))
#include "ThreadEmulation.h"
#endif
#endif

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {

#if (defined(_WIN32)) || defined(WIN32)


typedef struct
{
    t_thread_func thread_func;
    void* privateDataPtr;
    unsigned int iNbThread;

    DWORD dwThreadId;
} SYNC_THEAD_INFO;


#if defined(UNITEX_USING_WINRT_API) && (defined(UNITEX_PREVENT_USING_WINRT_THREADEMULATION))
UNITEX_FUNC int UNITEX_CALL IsSeveralThreadsPossible()
{
    return 0;
}

UNITEX_FUNC void UNITEX_CALL SyncDoRunThreads(unsigned int iNbThread,t_thread_func thread_func,void** privateDataPtrArray)
{
    if (iNbThread>0)
        (*thread_func)(*privateDataPtrArray,0);
}
#else
static DWORD WINAPI SyncThreadWrkFunc(LPVOID lpv)
{
    SYNC_THEAD_INFO* ptwi=(SYNC_THEAD_INFO*)lpv;
    (*(ptwi->thread_func))(ptwi->privateDataPtr,ptwi->iNbThread);
    return 0;
}

UNITEX_FUNC int UNITEX_CALL IsSeveralThreadsPossible()
{
    return 1;
}

static void internal_do_run_threads(unsigned int iNbThread, t_thread_func thread_func, void** privateDataPtrArray, unsigned int stackSize)
{
    unsigned int i;
    HANDLE* pHandle=(HANDLE*)malloc(sizeof(HANDLE)*iNbThread);
    SYNC_THEAD_INFO* pThreadInfoArray=(SYNC_THEAD_INFO*)malloc(sizeof(SYNC_THEAD_INFO)*iNbThread);

    for (i=0;i<iNbThread;i++)
    {
        void* privateDataPtr=*(privateDataPtrArray+i);
        (pThreadInfoArray+i)->privateDataPtr = privateDataPtr;
        (pThreadInfoArray+i)->thread_func = thread_func;
        (pThreadInfoArray+i)->iNbThread = i;
#ifdef UNITEX_USING_WINRT_API
        *(pHandle+i) = ThreadEmulation::CreateThread(NULL,(size_t)stackSize,SyncThreadWrkFunc,(pThreadInfoArray+i),0,&((pThreadInfoArray+i)->dwThreadId));
#else
        *(pHandle+i) = CreateThread(NULL,(size_t)stackSize,SyncThreadWrkFunc,(pThreadInfoArray+i),0,&((pThreadInfoArray+i)->dwThreadId));
#endif
    }

#ifdef UNITEX_USING_WINRT_API
    WaitForMultipleObjectsEx(iNbThread,pHandle,TRUE,INFINITE,FALSE);
#else
    WaitForMultipleObjects(iNbThread,pHandle,TRUE,INFINITE);
#endif
    free(pHandle);
    free(pThreadInfoArray);
}
#endif


UNITEX_FUNC void UNITEX_CALL SyncDoRunThreads(unsigned int iNbThread, t_thread_func thread_func, void** privateDataPtrArray)
{
    internal_do_run_threads(iNbThread, thread_func, privateDataPtrArray, 0);
}


UNITEX_FUNC void UNITEX_CALL SyncDoRunThreadsWithStackSize(unsigned int iNbThread, t_thread_func thread_func, void** privateDataPtrArray, unsigned int stackSize)
{
    internal_do_run_threads(iNbThread, thread_func, privateDataPtrArray, stackSize);
}

/**************************/




#if ((defined(__VISUALC__)) || defined(_MSC_VER)) && (!(defined(SYNC_TLC_BY_MSVC_EXTENSION))) && defined(UNITEX_USING_WINRT_API) && defined(UNITEX_PREVENT_USING_WINRT_THREADEMULATION)
#define SYNC_TLC_BY_MSVC_EXTENSION 1
#endif

#ifdef SYNC_TLC_BY_MSVC_EXTENSION

static int NbTLSSlotInMap = 0;
static BOOL* TlsSlotMap = NULL;

#ifdef CUSTOM_THEAD_VAR_PREFIX
CUSTOM_THEAD_VAR_PREFIX void** TlsEmulationCurrentThreadSlotValues = NULL;
CUSTOM_THEAD_VAR_PREFIX int TlsEmulationCurrentThreadNbSlotValuesAllocated = 0;
#else
static __declspec(thread) void** TlsEmulationCurrentThreadSlotValues = NULL;
static __declspec(thread) int TlsEmulationCurrentThreadNbSlotValuesAllocated = 0;
#endif

typedef struct
{
    int slotPos;
} SYNC_TLS_OBJECT_INTERNAL_MSVC_EXTENSION;


UNITEX_FUNC SYNC_TLS_OBJECT UNITEX_CALL SyncBuildTls()
{
    SYNC_TLS_OBJECT_INTERNAL_MSVC_EXTENSION* pstoi = (SYNC_TLS_OBJECT_INTERNAL_MSVC_EXTENSION*)malloc(sizeof(SYNC_TLS_OBJECT_INTERNAL_MSVC_EXTENSION));

    if (pstoi != NULL)
    {
        int new_slot_pos;
        for (new_slot_pos=0;new_slot_pos<NbTLSSlotInMap;new_slot_pos++)
        {
            if (!(*(TlsSlotMap+new_slot_pos)))
                break;
        }

        if (new_slot_pos==NbTLSSlotInMap)
        {
            BOOL* newTlsSlotMap = (BOOL*)((TlsSlotMap == NULL) ?
                malloc(sizeof(BOOL)*(new_slot_pos+1)) :
                realloc(TlsSlotMap,sizeof(BOOL)*(new_slot_pos+1)));
            if (newTlsSlotMap == NULL)
            {
                free(pstoi);
                return NULL;
            }
            TlsSlotMap = newTlsSlotMap;
            NbTLSSlotInMap = new_slot_pos+1;
        }
        *(TlsSlotMap+new_slot_pos) = TRUE;
        pstoi->slotPos = new_slot_pos;
    }

    return (SYNC_TLS_OBJECT)pstoi;
}

UNITEX_FUNC void UNITEX_CALL SyncDeleteTls(SYNC_TLS_OBJECT pTls)
{
    SYNC_TLS_OBJECT_INTERNAL_MSVC_EXTENSION* pstoi = (SYNC_TLS_OBJECT_INTERNAL_MSVC_EXTENSION*)pTls;
    if (pstoi != NULL)
    {
        int slotPos = pstoi->slotPos;
        if (slotPos < NbTLSSlotInMap)
        {
            *(TlsSlotMap+slotPos) = FALSE;
        }

        int last_slot_used = -1;
        for (int loop_slot=0;loop_slot<NbTLSSlotInMap;loop_slot++)
        {
            if (*(TlsSlotMap+loop_slot))
                last_slot_used = TRUE;
        }

        if (last_slot_used == -1)
        {
            if (TlsSlotMap != NULL)
                free(TlsSlotMap);
            TlsSlotMap = NULL;
        }
        NbTLSSlotInMap = last_slot_used+1;

        free(pstoi);
    }
}

UNITEX_FUNC int UNITEX_CALL SyncTlsSetValue(SYNC_TLS_OBJECT pTls,void* pUsrPtr)
{
    SYNC_TLS_OBJECT_INTERNAL_MSVC_EXTENSION* pstoi = (SYNC_TLS_OBJECT_INTERNAL_MSVC_EXTENSION*)pTls;
    if (pstoi == NULL)
        return 0;

    int slotPos = pstoi->slotPos;

    if (pUsrPtr != NULL)
    {
        if (slotPos >= NbTLSSlotInMap)
            return 0;
        if (!(*(TlsSlotMap+slotPos)))
            return 0;
        if (TlsEmulationCurrentThreadNbSlotValuesAllocated <= slotPos)
        {
            void** newTlsEmulationCurrentThreadSlotValues = (void**) ((TlsEmulationCurrentThreadSlotValues == NULL) ?
                            malloc(sizeof(void*)*(slotPos+1)) :
                            realloc(TlsEmulationCurrentThreadSlotValues,sizeof(void*)*(slotPos+1)));
            if (newTlsEmulationCurrentThreadSlotValues == NULL)
                return 0;

            TlsEmulationCurrentThreadSlotValues = newTlsEmulationCurrentThreadSlotValues;
            for (int fill=TlsEmulationCurrentThreadNbSlotValuesAllocated;fill<slotPos;fill++)
                *(TlsEmulationCurrentThreadSlotValues + fill) = (void*)NULL;
            *(TlsEmulationCurrentThreadSlotValues + slotPos) = pUsrPtr;
            TlsEmulationCurrentThreadNbSlotValuesAllocated = slotPos + 1;
        }
    }
    else
    {
        if (TlsEmulationCurrentThreadNbSlotValuesAllocated > slotPos)
        {
            *(TlsEmulationCurrentThreadSlotValues + slotPos) = pUsrPtr;

            int isOneSlotFilled=0;
            for (int checkOneFilledLoop=0;(checkOneFilledLoop<TlsEmulationCurrentThreadNbSlotValuesAllocated) && (checkOneFilledLoop<NbTLSSlotInMap);checkOneFilledLoop++)
            {
                if (*(TlsSlotMap+checkOneFilledLoop))
                    if ((*(TlsEmulationCurrentThreadSlotValues + checkOneFilledLoop)) != (void*)NULL)
                    {
                        isOneSlotFilled=1;
                        break;
                    }
            }

            if (isOneSlotFilled == 0)
            {
                if (TlsEmulationCurrentThreadSlotValues != NULL)
                {
                    free(TlsEmulationCurrentThreadSlotValues);
                    TlsEmulationCurrentThreadSlotValues = NULL;
                }
                TlsEmulationCurrentThreadNbSlotValuesAllocated = 0;
            }
        }
    }

    return 1;
}

UNITEX_FUNC void* UNITEX_CALL SyncTlsGetValue(SYNC_TLS_OBJECT pTls)
{
    SYNC_TLS_OBJECT_INTERNAL_MSVC_EXTENSION* pstoi = (SYNC_TLS_OBJECT_INTERNAL_MSVC_EXTENSION*)pTls;
    if (pstoi == NULL)
        return NULL;
    int slotPos = pstoi->slotPos;
    if ((slotPos < TlsEmulationCurrentThreadNbSlotValuesAllocated) && (slotPos < NbTLSSlotInMap))
        if (*(TlsSlotMap+slotPos))
            return *(TlsEmulationCurrentThreadSlotValues + slotPos);

    return NULL;
}



UNITEX_FUNC void UNITEX_CALL TlsCleanupCurrentThread()
{
    if (TlsEmulationCurrentThreadSlotValues != NULL)
    {
        free(TlsEmulationCurrentThreadSlotValues);
        TlsEmulationCurrentThreadSlotValues = NULL;
    }
    TlsEmulationCurrentThreadNbSlotValuesAllocated = 0;
}

#else
typedef struct
{
    DWORD dwTls;
} SYNC_TLS_OBJECT_INTERNAL_WIN32;


UNITEX_FUNC SYNC_TLS_OBJECT UNITEX_CALL SyncBuildTls()
{
    SYNC_TLS_OBJECT_INTERNAL_WIN32* pstoi = (SYNC_TLS_OBJECT_INTERNAL_WIN32*)malloc(sizeof(SYNC_TLS_OBJECT_INTERNAL_WIN32));

    if (pstoi != NULL)
    {
#ifdef UNITEX_USING_WINRT_API
        pstoi -> dwTls = ThreadEmulation::TlsAlloc();
#else
        pstoi -> dwTls = TlsAlloc();
        if ((pstoi -> dwTls) == TLS_OUT_OF_INDEXES)
        {
            free(pstoi);
            return NULL;
        }
#endif
    }

    return (SYNC_TLS_OBJECT)pstoi;
}

UNITEX_FUNC int UNITEX_CALL SyncTlsSetValue(SYNC_TLS_OBJECT pTls,void* pUsrPtr)
{
    SYNC_TLS_OBJECT_INTERNAL_WIN32* pstoi = (SYNC_TLS_OBJECT_INTERNAL_WIN32*)pTls;
    if (pstoi == NULL)
        return 0;

#ifdef UNITEX_USING_WINRT_API
    if (ThreadEmulation::TlsSetValue(pstoi->dwTls,pUsrPtr))
#else
    if (TlsSetValue(pstoi->dwTls,pUsrPtr))
#endif
        return 1;
    else
        return 0;
}

UNITEX_FUNC void* UNITEX_CALL SyncTlsGetValue(SYNC_TLS_OBJECT pTls)
{
    SYNC_TLS_OBJECT_INTERNAL_WIN32* pstoi = (SYNC_TLS_OBJECT_INTERNAL_WIN32*)pTls;
    if (pstoi == NULL)
        return NULL;

#ifdef UNITEX_USING_WINRT_API
    return ThreadEmulation::TlsGetValue(pstoi->dwTls);
#else
    return TlsGetValue(pstoi->dwTls);
#endif
}

UNITEX_FUNC void UNITEX_CALL SyncDeleteTls(SYNC_TLS_OBJECT pTls)
{
    SYNC_TLS_OBJECT_INTERNAL_WIN32* pstoi = (SYNC_TLS_OBJECT_INTERNAL_WIN32*)pTls;
    if (pstoi != NULL)
    {
#ifdef UNITEX_USING_WINRT_API
        ThreadEmulation::TlsFree(pstoi->dwTls);
#else
        TlsFree(pstoi->dwTls);
#endif
        free(pstoi);
    }
}

UNITEX_FUNC void UNITEX_CALL TlsCleanupCurrentThread()
{
#ifdef UNITEX_USING_WINRT_API
        ThreadEmulation::TlsShutdown();
#else
#endif
}
#endif

#endif

} // namespace logger
} // namespace unitex
