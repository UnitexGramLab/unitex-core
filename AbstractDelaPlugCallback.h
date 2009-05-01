 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef _ABSTRACT_DELA_CALLBACK_INCLUDED
#define _ABSTRACT_DELA_CALLBACK_INCLUDED 1

#include "FuncDeclModifier.h"
#include "AbstractCallbackFuncModifier.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_is_filename_DelaSpace_object)(const char*name,void* privateSpacePtr);

typedef int (ABSTRACT_CALLBACK_UNITEX* t_fnc_Init_DelaSpace)(void* privateSpacePtr);
typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_Uninit_DelaSpace)(void* privateSpacePtr);


typedef struct INF_codes* (ABSTRACT_CALLBACK_UNITEX* t_fnc_load_abstract_INF_file)(char* name,struct INF_free_info* p_inf_free_info,void* privateSpacePtr);

typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_free_abstract_INF)(struct INF_codes* INF,struct INF_free_info* p_inf_free_info,void* privateSpacePtr);

typedef unsigned char* (ABSTRACT_CALLBACK_UNITEX* t_fnc_load_abstract_BIN_file)(char* name,struct BIN_free_info* p_bin_free_info,void* privateSpacePtr);

typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_free_abstract_BIN)(unsigned char* BIN,struct BIN_free_info* p_bin_free_info,void* privateSpacePtr);



typedef struct
{
	t_fnc_is_filename_DelaSpace_object fnc_is_filename_object;

	t_fnc_Init_DelaSpace fnc_Init_DelaSpace;
	t_fnc_Uninit_DelaSpace fnc_Uninit_DelaSpace;

	t_fnc_load_abstract_INF_file fnc_load_abstract_INF_file;
	t_fnc_free_abstract_INF fnc_free_abstract_INF;
	t_fnc_load_abstract_BIN_file fnc_load_abstract_BIN_file;
	t_fnc_free_abstract_BIN fnc_free_abstract_BIN;

} t_persistent_dic_func_array;

UNITEX_FUNC int UNITEX_CALL AddAbstractDelaSpace(const t_persistent_dic_func_array* func_array,void* privateSpacePtr);
UNITEX_FUNC int UNITEX_CALL RemoveAbstractDelaSpace(const t_persistent_dic_func_array* func_array,void* privateSpacePtr);



#ifdef __cplusplus
}
#endif

#endif