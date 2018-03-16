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
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */




#include "Unicode.h"
#include "AbstractCallbackFuncModifier.h"
#include "SyncTool.h"

#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {



typedef struct
{
    time_t tm_t;
    struct timeval tp;
} TIMEBEGIN;

UNITEX_FUNC hTimeElapsed UNITEX_CALL SyncBuidTimeMarkerObject()
{
    TIMEBEGIN* pBegin = (TIMEBEGIN*)malloc(sizeof(TIMEBEGIN));
    pBegin->tm_t = time(NULL);
    gettimeofday(&(pBegin->tp),NULL);
    return (hTimeElapsed)pBegin;
}


UNITEX_FUNC unsigned int UNITEX_CALL SyncGetMSecElapsedNotDestructive(hTimeElapsed ptr, int destructObject)
{
    TIMEBEGIN tBegin;
    TIMEBEGIN tEnd;
    TIMEBEGIN* pBegin = (TIMEBEGIN*)ptr;
    unsigned int iRet;
    tBegin = *pBegin;
    tEnd.tm_t = time(NULL);
    gettimeofday(&(tEnd.tp),NULL);
    if (destructObject != 0)
      free(pBegin);

    iRet = (unsigned int)(((tEnd.tp.tv_sec*1000) + (tEnd.tp.tv_usec/1000)) -
                 ((tBegin.tp.tv_sec*1000) + (tBegin.tp.tv_usec/1000))) ;
    return iRet;
}

UNITEX_FUNC unsigned int UNITEX_CALL SyncGetMSecElapsed(hTimeElapsed ptr)
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
    pthread_mutex_t pmt;
} SYNC_MUTEX_OBJECT_INTERNAL;

UNITEX_FUNC SYNC_Mutex_OBJECT UNITEX_CALL SyncBuildMutex()
{
    int retCreate;
    SYNC_MUTEX_OBJECT_INTERNAL* pMoi = (SYNC_MUTEX_OBJECT_INTERNAL*)malloc(sizeof(SYNC_MUTEX_OBJECT_INTERNAL));
    if (pMoi == NULL)
        return NULL;
    retCreate = pthread_mutex_init(&pMoi->pmt,NULL);
    if (retCreate != 0)
    {
        free(pMoi);
        return NULL;
    }

    return (SYNC_Mutex_OBJECT)pMoi;
}

UNITEX_FUNC void UNITEX_CALL SyncGetMutex(SYNC_Mutex_OBJECT pMut)
{
    SYNC_MUTEX_OBJECT_INTERNAL* pMoi = (SYNC_MUTEX_OBJECT_INTERNAL*)pMut;
    if (pMoi != NULL)
        pthread_mutex_lock(&pMoi->pmt);
}

UNITEX_FUNC void UNITEX_CALL SyncReleaseMutex(SYNC_Mutex_OBJECT pMut)
{
    SYNC_MUTEX_OBJECT_INTERNAL* pMoi = (SYNC_MUTEX_OBJECT_INTERNAL*)pMut;
    if (pMut != NULL)
        pthread_mutex_unlock(&pMoi->pmt);
}

UNITEX_FUNC void UNITEX_CALL SyncDeleteMutex(SYNC_Mutex_OBJECT pMut)
{
    SYNC_MUTEX_OBJECT_INTERNAL* pMoi = (SYNC_MUTEX_OBJECT_INTERNAL*)pMut;
    if (pMoi != NULL)
    {
        pthread_mutex_destroy(&pMoi->pmt);
        free(pMoi);
    }
}



} // namespace unitex
