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



#ifndef _USER_CANCELLING_PLUG_CALLBACK_INCLUDED
#define _USER_CANCELLING_PLUG_CALLBACK_INCLUDED 1


#include "AbstractCallbackFuncModifier.h"
#include "UserCancelling.h"

#ifdef __cplusplus
extern "C" {
#endif

/* use this header to define an Unitex Cancel Callback
   Reading and understanding AbstractFilePlugCallback.h before
   this using this file is suggested

   An Cancel is a set of one callback called during some operation
   If this callback return 1; the operation is cancelled as soon as possible
   (after freeing memory and remove temporary file : no leak !)
 */


/* there fopen callback are called when an Unitex tool open a file 
 *  In the Unitex context, MODE is one of these value :
 *   - "rb" : open the file in read only mode
 *   - "wb" : open the file in write only mode (the previous file is erased, if exist)
 *   - "r+b" or "ab" : open the file in read and write mode ("ab" mean append)
 */
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_is_cancelling)(void* privateCancelPtr);

/* two optional (can be just NULL) callbacks to initialize and uninitialize */
typedef int (ABSTRACT_CALLBACK_UNITEX* t_fnc_Init_User_Cancelling)(void* privateCancelPtr);
typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_Uninit_User_Cancelling)(void* privateCancelPtr);

typedef struct
{
    unsigned int size_struct;

    t_fnc_is_cancelling fnc_is_cancelling;

    t_fnc_Init_User_Cancelling fnc_Init_User_Cancelling;
    t_fnc_Uninit_User_Cancelling fnc_Uninit_User_Cancelling;
} t_user_cancelling_func_array;

/* these functions respectively add and remove user cancelling.
  you can add several with the same func_array callback set, but with different privateCancelPtr
  privateCancelPtr is the parameters which can be set as the last parameter of each callback */
UNITEX_FUNC int UNITEX_CALL AddUserCancellingInfo(const t_user_cancelling_func_array* func_array,void* privateCancelPtr);
UNITEX_FUNC int UNITEX_CALL RemoveUserCancellingInfo(const t_user_cancelling_func_array* func_array,void* privateCancelPtr);

/* just return the number of user cancelling Installed */
UNITEX_FUNC int UNITEX_CALL GetNbUserCancellingInfoInstalled();

UNITEX_FUNC int UNITEX_CALL UnitexIsUserCancellingRequested();

/**********************************************************************/



#ifdef __cplusplus
}
#endif

#endif
