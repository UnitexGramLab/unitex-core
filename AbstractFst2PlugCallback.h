 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef _ABSTRACT_FST2_CALLBACK_INCLUDED
#define _ABSTRACT_FST2_CALLBACK_INCLUDED 1

#include "AbstractCallbackFuncModifier.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_is_filename_Fst2Space_object)(const char*name,void* privateSpacePtr);

typedef int (ABSTRACT_CALLBACK_UNITEX* t_fnc_Init_Fst2Space)(void* privateSpacePtr);
typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_Uninit_Fst2Space)(void* privateSpacePtr);



/* Unlike the load callback for dictionnary, p_fst2_free_info can be NULL
   this mean the caller (Locate) will modify the Fst2* object, and then call free_Fst2

   To reuse an Fst2 object, the solution is to call new_Fst2_clone
*/


typedef Fst2* (ABSTRACT_CALLBACK_UNITEX* t_fnc_load_abstract_fst2)(const char* name,int read_names,struct FST2_free_info* p_fst2_free_info,void* privateSpacePtr);

typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_free_abstract_fst2)(Fst2* fst2,struct FST2_free_info* p_inf_free_info,void* privateSpacePtr);




typedef struct
{
    t_fnc_is_filename_Fst2Space_object fnc_is_filename_object;

    t_fnc_Init_Fst2Space fnc_Init_Fst2Space;
    t_fnc_Uninit_Fst2Space fnc_Uninit_Fst2Space;

    t_fnc_load_abstract_fst2 fnc_load_abstract_fst2;
    t_fnc_free_abstract_fst2 fnc_free_abstract_fst2;

} t_persistent_fst2_func_array;

UNITEX_FUNC int UNITEX_CALL AddAbstractFst2Space(const t_persistent_fst2_func_array* func_array,void* privateSpacePtr);
UNITEX_FUNC int UNITEX_CALL RemoveAbstractFst2Space(const t_persistent_fst2_func_array* func_array,void* privateSpacePtr);


/* just return the number of AbstractFst2Space Installed */
UNITEX_FUNC int UNITEX_CALL GetNbAbstractFst2SpaceInstalled();


#ifdef __cplusplus
}
#endif

#endif
