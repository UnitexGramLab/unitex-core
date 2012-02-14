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




#include "Unicode.h"
#include "AbstractCallbackFuncModifier.h"
#include "SyncLogger.h"
#include <time.h>

namespace unitex {
namespace logger {

UNITEX_FUNC int UNITEX_CALL IsSeveralThreadsPossible()
{
    return 0;
}

UNITEX_FUNC void UNITEX_CALL SyncDoRunThreads(unsigned int iNbThread,t_thread_func thread_func,void** privateDataPtrArray)
{
    if (iNbThread>0)
        (*thread_func)(*privateDataPtrArray,0);
}


typedef struct
{
    clock_t startTime;
    //time_t t_start;
} TIMEBEGIN;

UNITEX_FUNC hTimeElasped UNITEX_CALL SyncBuidTimeMarkerObject()
{
    TIMEBEGIN* pBegin = (TIMEBEGIN*)malloc(sizeof(TIMEBEGIN));
    pBegin->startTime=clock();
    //time(&pBegin->t_start);
    return (hTimeElasped)pBegin;
}


UNITEX_FUNC unsigned int UNITEX_CALL SyncGetMSecElapsedNotDestructive(hTimeElasped ptr, int destructObject)
{
    TIMEBEGIN* pBegin = (TIMEBEGIN*)ptr;
    unsigned int iRet;
/*
    time_t t_end;
    time(&t_end);
    iRet = (unsigned int)(difftime(t_end,pBegin->t_start) * 1000);
	*/
	clock_t endTime=clock();
	iRet= (int)((((double)(endTime-(pBegin->startTime))) / CLOCKS_PER_SEC) * 1000);
    if (destructObject != 0)
      free(pBegin);

    return iRet;
}


UNITEX_FUNC unsigned int UNITEX_CALL SyncGetMSecElapsed(hTimeElasped ptr)
{
    return SyncGetMSecElapsedNotDestructive(ptr,1);
}


/*
Mutex implementation for Posix API
 (Linux, MacOS X, BSD...)
Documentation about posix thread API
http://manpages.ubuntu.com/manpages/dapper/fr/man3/pthread_mutex_init.3.html
http://developer.apple.com/documentation/Darwin/Reference/Manpages/man3/pthread_mutex_init.3.html
http://www.linux-kheops.com/doc/man/manfr/man-html-0.9/man3/pthread_mutex_init.3.html
*/

typedef struct
{
    int dummy;
} SYNC_Mutex_OBJECT_INTERNAL;

UNITEX_FUNC SYNC_Mutex_OBJECT UNITEX_CALL SyncBuildMutex()
{
    SYNC_Mutex_OBJECT_INTERNAL* pMoi = (SYNC_Mutex_OBJECT_INTERNAL*)malloc(sizeof(SYNC_Mutex_OBJECT_INTERNAL));
    if (pMoi == NULL)
        return NULL;
    
    return (SYNC_Mutex_OBJECT)pMoi;
}

UNITEX_FUNC void UNITEX_CALL SyncGetMutex(SYNC_Mutex_OBJECT)
{
}

UNITEX_FUNC void UNITEX_CALL SyncReleaseMutex(SYNC_Mutex_OBJECT)
{
}

UNITEX_FUNC void UNITEX_CALL SyncDeleteMutex(SYNC_Mutex_OBJECT pMut)
{
    SYNC_Mutex_OBJECT_INTERNAL* pMoi = (SYNC_Mutex_OBJECT_INTERNAL*)pMut;
    if (pMoi != NULL)
    {
        free(pMoi);
    }
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

} // namespace logger
} // namespace unitex
