/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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




/* SyncLogger.h
*/

#ifndef _SYNC_LOGGER_H
#define _SYNC_LOGGER_H

#include "AbstractCallbackFuncModifier.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* hTimeElasped;

UNITEX_FUNC hTimeElasped UNITEX_CALL SyncBuidTimeMarkerObject();
UNITEX_FUNC unsigned int UNITEX_CALL SyncGetMSecElapsed(hTimeElasped ptr);
UNITEX_FUNC unsigned int UNITEX_CALL SyncGetMSecElapsedNotDestructive(hTimeElasped ptr, int destructObject);

#define SYNC_CALLBACK_UNITEX ABSTRACT_CALLBACK_UNITEX
typedef void (SYNC_CALLBACK_UNITEX* t_thread_func)(void* privateDataPtr,unsigned int iNbThread);

UNITEX_FUNC int UNITEX_CALL IsSeveralThreadsPossible();
UNITEX_FUNC void UNITEX_CALL SyncDoRunThreads(unsigned int iNbThread,t_thread_func thread_func,void** privateDataPtrArray);


typedef void* SYNC_Mutex_OBJECT;

UNITEX_FUNC SYNC_Mutex_OBJECT UNITEX_CALL SyncBuildMutex();
UNITEX_FUNC void UNITEX_CALL SyncGetMutex(SYNC_Mutex_OBJECT pMut);
UNITEX_FUNC void UNITEX_CALL SyncReleaseMutex(SYNC_Mutex_OBJECT pMut);
UNITEX_FUNC void UNITEX_CALL SyncDeleteMutex(SYNC_Mutex_OBJECT pMut);


typedef void* SYNC_TLS_OBJECT;

UNITEX_FUNC SYNC_TLS_OBJECT UNITEX_CALL SyncBuildTls();
UNITEX_FUNC int UNITEX_CALL SyncTlsSetValue(SYNC_TLS_OBJECT pTls,void* pUsrPtr);
UNITEX_FUNC void* UNITEX_CALL SyncTlsGetValue(SYNC_TLS_OBJECT pTls);
UNITEX_FUNC void UNITEX_CALL SyncDeleteTls(SYNC_TLS_OBJECT pTls);

#ifdef __cplusplus
}
#endif

#endif
