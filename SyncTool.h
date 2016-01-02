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




/* SyncLogger.h
*/

#ifndef _SYNC_TOOL_H
#define _SYNC_TOOL_H

#include "AbstractCallbackFuncModifier.h"

#ifdef __cplusplus
#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
extern "C" {
#endif

typedef void* hTimeElapsed;

#define HTIME_ELASPED_TYPO_FIXED 1

UNITEX_FUNC hTimeElapsed UNITEX_CALL SyncBuidTimeMarkerObject();
UNITEX_FUNC unsigned int UNITEX_CALL SyncGetMSecElapsed(hTimeElapsed ptr);
UNITEX_FUNC unsigned int UNITEX_CALL SyncGetMSecElapsedNotDestructive(hTimeElapsed ptr, int destructObject);



typedef void* SYNC_Mutex_OBJECT;

UNITEX_FUNC SYNC_Mutex_OBJECT UNITEX_CALL SyncBuildMutex();
UNITEX_FUNC void UNITEX_CALL SyncGetMutex(SYNC_Mutex_OBJECT pMut);
UNITEX_FUNC void UNITEX_CALL SyncReleaseMutex(SYNC_Mutex_OBJECT pMut);
UNITEX_FUNC void UNITEX_CALL SyncDeleteMutex(SYNC_Mutex_OBJECT pMut);




#ifdef __cplusplus
} // extern "C"
} // namespace unitex
#endif

#endif
