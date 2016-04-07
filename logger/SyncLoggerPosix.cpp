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
#include "SyncLogger.h"
#include "Error.h"
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {


typedef struct
{
    t_thread_func thread_func;
    void* privateDataPtr;
    unsigned int iNbThread;
} SYNC_THEAD_INFO;


static void* SyncThreadWrkFuncPosix(void*pv)
{
    SYNC_THEAD_INFO* ptwi=(SYNC_THEAD_INFO*)pv;
    (*(ptwi->thread_func))(ptwi->privateDataPtr,ptwi->iNbThread);
    pthread_exit(NULL);
    return NULL;
}



UNITEX_FUNC int UNITEX_CALL IsSeveralThreadsPossible()
{
    return 1;
}

// see http://linux.die.net/man/3/pthread_create

static void internal_do_run_threads(unsigned int iNbThread,t_thread_func thread_func,void** privateDataPtrArray, unsigned int stackSize)
{
    unsigned int i;
    pthread_t * pTid=(pthread_t*)malloc(sizeof(pthread_t)*iNbThread);
    pthread_attr_t attr;
    SYNC_THEAD_INFO* pThreadInfoArray=(SYNC_THEAD_INFO*)malloc(sizeof(SYNC_THEAD_INFO)*iNbThread);
    memset(&attr, 0, sizeof(pthread_attr_t));

    if (stackSize != 0)
    {
        int s = pthread_attr_init(&attr);
        if (s != 0)
        {
            error("error in pthread_attr_init");
            return;
        }
        s = pthread_attr_setstacksize(&attr, (size_t)stackSize);
        if (s != 0)
        {
            error("invalid stack size %u", stackSize);
        }
    }

    for (i=0;i<iNbThread;i++)
    {
        void* privateDataPtr=*(privateDataPtrArray+i);
        (pThreadInfoArray+i)->privateDataPtr = privateDataPtr;
        (pThreadInfoArray+i)->thread_func = thread_func;
        (pThreadInfoArray+i)->iNbThread = i;
        pthread_create(pTid+i, (stackSize != 0) ? (&attr) : NULL, SyncThreadWrkFuncPosix, (pThreadInfoArray+i));
    }

    for (i=0;i<iNbThread;i++)
        pthread_join(*(pTid+i),NULL);

    if (stackSize != 0)
    {
        pthread_attr_destroy(&attr);
    }

    free(pThreadInfoArray);
    free(pTid);
}


UNITEX_FUNC void UNITEX_CALL SyncDoRunThreads(unsigned int iNbThread, t_thread_func thread_func, void** privateDataPtrArray)
{
    internal_do_run_threads(iNbThread, thread_func, privateDataPtrArray, 0);
}


UNITEX_FUNC void UNITEX_CALL SyncDoRunThreadsWithStackSize(unsigned int iNbThread, t_thread_func thread_func, void** privateDataPtrArray, unsigned int stackSize)
{
    internal_do_run_threads(iNbThread, thread_func, privateDataPtrArray, stackSize);
}


typedef struct
{
    pthread_key_t pthread_key;
} SYNC_TLS_OBJECT_INTERNAL_POSIX;





UNITEX_FUNC SYNC_TLS_OBJECT UNITEX_CALL SyncBuildTls()
{
    SYNC_TLS_OBJECT_INTERNAL_POSIX* pstoi = (SYNC_TLS_OBJECT_INTERNAL_POSIX*)malloc(sizeof(SYNC_TLS_OBJECT_INTERNAL_POSIX));

    if (pstoi != NULL)
    {
        if (pthread_key_create(&pstoi -> pthread_key,NULL) != 0)
        {
            free(pstoi);
            return NULL;
        }
    }

    return (SYNC_TLS_OBJECT)pstoi;
}

UNITEX_FUNC int UNITEX_CALL SyncTlsSetValue(SYNC_TLS_OBJECT pTls,void* pUsrPtr)
{
    SYNC_TLS_OBJECT_INTERNAL_POSIX* pstoi = (SYNC_TLS_OBJECT_INTERNAL_POSIX*)pTls;
    if (pstoi == NULL)
        return 0;

    if (pthread_setspecific(pstoi->pthread_key,pUsrPtr) == 0)
        return 1;
    else
        return 0;
}

UNITEX_FUNC void* UNITEX_CALL SyncTlsGetValue(SYNC_TLS_OBJECT pTls)
{
    SYNC_TLS_OBJECT_INTERNAL_POSIX* pstoi = (SYNC_TLS_OBJECT_INTERNAL_POSIX*)pTls;
    if (pstoi == NULL)
        return NULL;

    return pthread_getspecific(pstoi->pthread_key);
}

UNITEX_FUNC void UNITEX_CALL SyncDeleteTls(SYNC_TLS_OBJECT pTls)
{
    SYNC_TLS_OBJECT_INTERNAL_POSIX* pstoi = (SYNC_TLS_OBJECT_INTERNAL_POSIX*)pTls;
    if (pstoi != NULL)
    {
        pthread_key_delete(pstoi->pthread_key);
        free(pstoi);
    }
}

UNITEX_FUNC void UNITEX_CALL TlsCleanupCurrentThread()
{
}

} // namespace logger
} // namespace unitex
