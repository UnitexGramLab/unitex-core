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



#ifndef _ABSTRACT_ALLOCATOR_PLUG_CALLBACK_INCLUDED
#define _ABSTRACT_ALLOCATOR_PLUG_CALLBACK_INCLUDED 1


#include "AbstractCallbackFuncModifier.h"
#include "AbstractAllocator.h"


#ifdef __cplusplus
extern "C" {
//using namespace unitex;
#endif

/* use this header to define an Unitex allocator
   Reading and understanding AbstractFilePlugCallback.h before
   this using this file is suggested

 */


typedef int (ABSTRACT_CALLBACK_UNITEX* t_fnc_Init_AllocatorSpace)(void* privateSpacePtr);
typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_Uninit_AllocatorSpace)(void* privateSpacePtr);


typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_is_param_allocator_compatible)(const char*creator,int flagAllocator,size_t,const void* private_create_ptr,void* privateSpacePtr);

/* Unlike the load callback for dictionnary, p_fst2_free_info can be NULL
   this mean the caller (Locate) will modify the Fst2* object, and then call free_Fst2

   To reuse an Fst2 object, the solution is to call new_Fst2_clone
*/

/* try create an abstract allocator with parameter
if successful, fill the Abstract_allocator struct and return 1, else return 0 */
typedef int (ABSTRACT_CALLBACK_UNITEX* t_fnc_create_abstract_allocator)(abstract_allocator_info_public_with_allocator*,const char*creator,int flagAllocator,size_t,const void* private_create_ptr,void* privateAllocatorSpacePtr);






typedef struct
{
    unsigned int size_struct;

    t_fnc_Init_AllocatorSpace fnc_Init_AllocatorSpace;
    t_fnc_Uninit_AllocatorSpace fnc_Uninit_AllocatorSpace;

    t_fnc_is_param_allocator_compatible fnc_is_param_allocator_compatible;

    t_fnc_create_abstract_allocator fnc_create_abstract_allocator;
    t_fnc_delete_abstract_allocator fnc_delete_abstract_allocator;

} t_allocator_func_array;

/* these functions respectively add and remove allocator.
  you can add several allocator with the same func_array callback set, but with different privateAllocatorPtr
  privateAllocatorPtr is the parameters which can be set as the last parameter of each callback */
UNITEX_FUNC int UNITEX_CALL AddAllocatorSpace(const t_allocator_func_array* func_array,void* privateAllocatorPtr);
UNITEX_FUNC int UNITEX_CALL RemoveAllocatorSpace(const t_allocator_func_array* func_array,void* privateAllocatorPtr);

/* just return the number of Allocator Installed */
UNITEX_FUNC int UNITEX_CALL GetNbAllocatorSpaceInstalled();

UNITEX_FUNC Abstract_allocator UNITEX_CALL BuildAbstractAllocatorFromSpecificAllocatorSpace(const t_allocator_func_array*,void* privateAllocatorSpacePtr,const char*creator,int flagAllocator,size_t expected_size_item,const void* private_create_ptr);

/**********************************************************************/





#ifdef __cplusplus
}
#endif

#endif
