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

#include <string.h>
#include <memory.h>
#include "Error.h"
#include "AbstractAllocator.h"
#include "AbstractAllocatorPlugCallback.h"


struct AllocatorSpace {
	t_allocator_func_array func_array;
	void* privateAllocatorSpacePtr;
} ;


struct List_AllocatorSpace {
	AllocatorSpace aas;
	List_AllocatorSpace* next;
} ;


struct List_AllocatorSpace* p_allocator_info_list=NULL;



UNITEX_FUNC int UNITEX_CALL AddAllocatorSpace(const t_allocator_func_array* func_array,void* privateAllocatorSpacePtr)
{
	struct List_AllocatorSpace* new_item;
	new_item = (struct List_AllocatorSpace*)malloc(sizeof(struct List_AllocatorSpace));
	if (new_item == NULL)
		return 0;

	new_item->aas.func_array = *func_array;
	new_item->aas.privateAllocatorSpacePtr = privateAllocatorSpacePtr;
	new_item->next = NULL;

	if (p_allocator_info_list == NULL)
		p_allocator_info_list = new_item;
	else {
		struct List_AllocatorSpace* tmp = p_allocator_info_list;
		while ((tmp->next) != NULL)
			tmp = tmp->next;
		tmp->next = new_item;
	}

	if ((new_item->aas.func_array.fnc_Init_AllocatorSpace) != NULL)
		(*(new_item->aas.func_array.fnc_Init_AllocatorSpace))(new_item->aas.privateAllocatorSpacePtr);

	return 1;
}

UNITEX_FUNC int UNITEX_CALL RemoveAllocatorSpace(const t_allocator_func_array* func_array,void* privateAllocatorSpacePtr)
{
	struct List_AllocatorSpace* tmp = p_allocator_info_list;
	struct List_AllocatorSpace* tmp_previous = NULL;

	while (tmp != NULL)
	{
		if ((memcmp(&tmp->aas.func_array,func_array,sizeof(t_allocator_func_array))==0) &&
			(tmp->aas.privateAllocatorSpacePtr == privateAllocatorSpacePtr))
		{
			if (tmp_previous == NULL)
				p_allocator_info_list = tmp->next;
			else
				tmp_previous->next = tmp->next;

			if ((tmp->aas.func_array.fnc_Uninit_AllocatorSpace) != NULL)
				(*(tmp->aas.func_array.fnc_Uninit_AllocatorSpace))(tmp->aas.privateAllocatorSpacePtr);

			free(tmp);
			return 1;
		}
		tmp_previous = tmp;
		tmp = tmp->next;
	}
	return 0;
}


UNITEX_FUNC int UNITEX_CALL GetNbAllocatorSpaceInstalled()
{
    int count=0;
    struct List_AllocatorSpace* tmp = p_allocator_info_list;
	while (tmp != NULL)
	{
        count++;
		tmp = tmp->next;
	}
	return count;
}

const AllocatorSpace * GetAllocatorSpaceForParam(const char*creator,int flagAllocator,size_t expected_size)
{
	const struct List_AllocatorSpace* tmp = p_allocator_info_list;

	while (tmp != NULL)
	{
		const AllocatorSpace * test_aas = &(tmp->aas);
        if (tmp->aas.func_array.fnc_is_param_allocator_compatible(creator,flagAllocator,expected_size,tmp->aas.privateAllocatorSpacePtr) != 0)
			return test_aas;		

		tmp = tmp->next;
	}
	return NULL;
}




Abstract_allocator create_abstract_allocator(const char*creator,int flagAllocator,size_t expected_size)
{
    Abstract_allocator aas;
    const AllocatorSpace * paas = GetAllocatorSpaceForParam(creator,flagAllocator,expected_size) ;
    if (paas == NULL)
        return NULL;

    aas=(abstract_allocator*)malloc(sizeof(abstract_allocator));
    if (aas == NULL)
        return NULL;
    if (paas->func_array.fnc_create_abstract_allocator(aas,creator,flagAllocator,expected_size,paas->privateAllocatorSpacePtr) == 0)
    {
        free(aas);
        return NULL;
    }

    aas->fnc_delete_abstract_allocator = paas->func_array.fnc_delete_abstract_allocator;
    aas->privateAllocatorSpacePtr = paas->privateAllocatorSpacePtr;

    return aas;
}

void close_abstract_allocator(Abstract_allocator aa)
{
    if (aa != NULL)
    {
        abstract_allocator* aas = aa;
        if (aas->fnc_delete_abstract_allocator != NULL)
        {
            aas->fnc_delete_abstract_allocator(aa,aas->privateAllocatorSpacePtr);
        }
        free(aa);
    }
}

size_t get_allocator_size(Abstract_allocator aa)
{
    size_t ret=0;
    if (aa != NULL)
    {
        if (aa->fnc_get_size_allocator != NULL)
            ret = (aa->fnc_get_size_allocator)(aa->abstract_allocator_ptr);      
    }
    return ret;
}
