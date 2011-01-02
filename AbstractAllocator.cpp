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

const AllocatorSpace * GetAllocatorSpaceForParam(const char*creator,int flagAllocator,size_t expected_size_item,const void* private_create_ptr)
{
	const struct List_AllocatorSpace* tmp = p_allocator_info_list;
	const AllocatorSpace * best_aas = NULL;
	int best_priority = 0;

	while (tmp != NULL)
	{
		const AllocatorSpace * test_aas = &(tmp->aas);

        int cur_priority = tmp->aas.func_array.fnc_is_param_allocator_compatible(creator,flagAllocator,expected_size_item,private_create_ptr,tmp->aas.privateAllocatorSpacePtr) ;

        if ((cur_priority>0) && (cur_priority>best_priority))
        {
            best_aas = test_aas;
            best_priority = cur_priority;
        }

		tmp = tmp->next;
	}
	return best_aas;
}


Abstract_allocator build_Abstract_allocator_from_AllocatorSpace(const t_allocator_func_array *p_func_array,void* privateAllocatorSpacePtr,const char*creator,int creation_flagAllocator,size_t expected_size_item,const void* private_create_ptr)
{
    Abstract_allocator aas;

    aas=(abstract_allocator*)malloc(sizeof(abstract_allocator));
    if (aas == NULL)
        return NULL;
    memset(&(aas->pub),0,sizeof(abstract_allocator_info_public_with_allocator));
    if (p_func_array->fnc_create_abstract_allocator(&(aas->pub),creator,creation_flagAllocator,expected_size_item,private_create_ptr,privateAllocatorSpacePtr) == 0)
    {
        free(aas);
        return NULL;
    }

    aas->fnc_delete_abstract_allocator = p_func_array->fnc_delete_abstract_allocator;
    aas->privateAllocatorSpacePtr = privateAllocatorSpacePtr;
    aas->creation_flag = creation_flagAllocator;
    aas->expected_creation_size = expected_size_item;
    aas->creator = strdup(creator);

    return aas;
}

UNITEX_FUNC Abstract_allocator UNITEX_CALL BuildAbstractAllocatorFromSpecificAllocatorSpace(const t_allocator_func_array *p_func_array,
                    void* privateAllocatorSpacePtr,const char*creator,int flagAllocator,size_t expected_size_item,const void* private_create_ptr)
{
    return build_Abstract_allocator_from_AllocatorSpace(p_func_array,privateAllocatorSpacePtr,creator,flagAllocator,expected_size_item,private_create_ptr);
}

Abstract_allocator create_abstract_allocator(const char*creator,int flagAllocator,size_t expected_size_item,const void* private_create_ptr)
{
    const AllocatorSpace * paas = GetAllocatorSpaceForParam(creator,flagAllocator,expected_size_item,private_create_ptr) ;
    if (paas == NULL)
        return NULL;

    return build_Abstract_allocator_from_AllocatorSpace(&(paas->func_array),paas->privateAllocatorSpacePtr,creator,flagAllocator,expected_size_item,private_create_ptr);
}

void close_abstract_allocator(Abstract_allocator aa)
{
    if (aa != NULL)
    {
        abstract_allocator* aas = aa;
        if (aas->fnc_delete_abstract_allocator != NULL)
        {
            aas->fnc_delete_abstract_allocator(&(aa->pub),aas->privateAllocatorSpacePtr);
        }
        if (aas->creator != NULL)
            free(aas->creator);
        free(aa);
    }
}

int get_allocator_flag(Abstract_allocator aa)
{
    int ret=0;
    if (aa != NULL)
    {
        if (aa->pub.fnc_get_flag_allocator != NULL)
            ret = (aa->pub.fnc_get_flag_allocator)(aa->pub.abstract_allocator_ptr);      
    }
    return ret;
}


int get_allocator_creation_flag(Abstract_allocator aa)
{
    int ret=0;
    if (aa != NULL)
    {
        ret = (aa->creation_flag);      
    }
    return ret;
}

size_t get_allocator_expected_creation_size(Abstract_allocator aa)
{
    size_t ret=0;
    if (aa != NULL)
    {
        ret = (aa->expected_creation_size);      
    }
    return ret;
}

int get_allocator_statistic_info(Abstract_allocator aa,int iStatNum,size_t*p_value)
{
    int ret=0;
    if (aa != NULL)
    {
        if (aa->pub.fnc_get_statistic_allocator_info != NULL)
            ret = (aa->pub.fnc_get_statistic_allocator_info)(iStatNum,p_value,aa->pub.abstract_allocator_ptr);   
    }
    return ret;
}

int clean_allocator(Abstract_allocator aa)
{
	int flag=0;
    if (aa != NULL)
    {
        if (aa->pub.fnc_get_flag_allocator != NULL)
            flag = (aa->pub.fnc_get_flag_allocator)(aa->pub.abstract_allocator_ptr);
		if ((flag & AllocatorCleanPresent) != 0)
			if (aa->pub.fnc_clean_allocator != NULL) {
				(aa->pub.fnc_clean_allocator)(aa->pub.abstract_allocator_ptr);
				return 1;
			}
    }
    return 0;
}

const char* get_allocator_creator(Abstract_allocator aa)
{
    const char* ret=NULL;
    if (aa != NULL)
    {
        ret = (aa->creator);
    }
    return ret;
}

abstract_allocator_info_public_with_allocator* get_abstract_allocator_info_public_with_allocator(Abstract_allocator aa)
{
    abstract_allocator_info_public_with_allocator* ret=NULL;
    if (aa != NULL)
    {
        ret = &(aa->pub);
    }
    return ret;
}

