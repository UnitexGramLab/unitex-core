/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "UnusedParameter.h"
#include <time.h>

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {

UNITEX_FUNC int UNITEX_CALL IsSeveralThreadsPossible()
{
    return 0;
}


static void internal_do_run_threads(unsigned int iNbThread, t_thread_func thread_func, void** privateDataPtrArray, unsigned int stackSize)
{
    DISCARD_UNUSED_PARAMETER(stackSize)
    for (int i = 0;i < iNbThread; i++)
        (*thread_func)(*(privateDataPtrArray+i), 0);
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
    void* value;
} SYNC_TLS_OBJECT_INTERNAL_POSIX;





UNITEX_FUNC SYNC_TLS_OBJECT UNITEX_CALL SyncBuildTls()
{
    SYNC_TLS_OBJECT_INTERNAL_POSIX* pstoi = (SYNC_TLS_OBJECT_INTERNAL_POSIX*)malloc(sizeof(SYNC_TLS_OBJECT_INTERNAL_POSIX));

    if (pstoi != NULL)
    {
        pstoi->value = NULL;
    }

    return (SYNC_TLS_OBJECT)pstoi;
}

UNITEX_FUNC int UNITEX_CALL SyncTlsSetValue(SYNC_TLS_OBJECT pTls,void* pUsrPtr)
{
    SYNC_TLS_OBJECT_INTERNAL_POSIX* pstoi = (SYNC_TLS_OBJECT_INTERNAL_POSIX*)pTls;
    if (pstoi == NULL)
        return 0;

    pstoi->value = pUsrPtr;
    return 1;
}

UNITEX_FUNC void* UNITEX_CALL SyncTlsGetValue(SYNC_TLS_OBJECT pTls)
{
    SYNC_TLS_OBJECT_INTERNAL_POSIX* pstoi = (SYNC_TLS_OBJECT_INTERNAL_POSIX*)pTls;
    if (pstoi == NULL)
        return NULL;

    return pstoi->value;
}

UNITEX_FUNC void UNITEX_CALL SyncDeleteTls(SYNC_TLS_OBJECT pTls)
{
    SYNC_TLS_OBJECT_INTERNAL_POSIX* pstoi = (SYNC_TLS_OBJECT_INTERNAL_POSIX*)pTls;
    if (pstoi != NULL)
    {
        free(pstoi);
    }
}

UNITEX_FUNC void UNITEX_CALL TlsCleanupCurrentThread()
{
}

} // namespace logger
} // namespace unitex
