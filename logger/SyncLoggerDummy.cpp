#include "Unicode.h"
#include "AbstractCallbackFuncModifier.h"
#include "SyncLogger.h"
#include <time.h>

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
    //clock_t startTime;
    time_t t_start;
} TIMEBEGIN;

UNITEX_FUNC hTimeElasped UNITEX_CALL SyncBuidTimeMarkerObject()
{
    TIMEBEGIN* pBegin = (TIMEBEGIN*)malloc(sizeof(TIMEBEGIN));
    // pBegin->startTime=clock();
    time(&pBegin->t_start);
    return (hTimeElasped)pBegin;
}


UNITEX_FUNC unsigned int UNITEX_CALL SyncGetMSecElapsed(hTimeElasped ptr)
{
    TIMEBEGIN* pBegin = (TIMEBEGIN*)ptr;
    unsigned int iRet;

    time_t t_end;
    time(&t_end);
    iRet = (unsigned int)(difftime(t_end,pBegin->t_start) * 1000);
    free(pBegin);

    return iRet;
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
