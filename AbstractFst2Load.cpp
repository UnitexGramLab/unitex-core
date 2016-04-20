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



#include "Unicode.h"
#include "Fst2.h"

#include "AbstractFst2Load.h"
#include "AbstractFst2PlugCallback.h"
#include "Persistence.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

//namespace unitex {

using namespace unitex;

struct AbstractFst2Space {
	t_persistent_fst2_func_array func_array;
	void* privateSpacePtr;
} ;


struct List_AbstractFst2Space {
	AbstractFst2Space afs;
	List_AbstractFst2Space* next;
} ;


struct List_AbstractFst2Space* p_abstract_fst2_space_list=NULL;



UNITEX_FUNC int UNITEX_CALL AddAbstractFst2Space(const t_persistent_fst2_func_array* func_array,void* privateSpacePtr)
{
	struct List_AbstractFst2Space* new_item;
	new_item = (struct List_AbstractFst2Space*)malloc(sizeof(struct List_AbstractFst2Space));
	if (new_item == NULL)
		return 0;

	new_item->afs.func_array = *func_array;
	new_item->afs.privateSpacePtr = privateSpacePtr;
	new_item->next = NULL;

	if (p_abstract_fst2_space_list == NULL)
		p_abstract_fst2_space_list = new_item;
	else {
		struct List_AbstractFst2Space* tmp = p_abstract_fst2_space_list;
		while ((tmp->next) != NULL)
			tmp = tmp->next;
		tmp->next = new_item;
	}

	if ((new_item->afs.func_array.fnc_Init_Fst2Space) != NULL)
		(*(new_item->afs.func_array.fnc_Init_Fst2Space))(new_item->afs.privateSpacePtr);

	return 1;
}

UNITEX_FUNC int UNITEX_CALL RemoveAbstractFst2Space(const t_persistent_fst2_func_array* func_array,void* privateSpacePtr)
{
	struct List_AbstractFst2Space* tmp = p_abstract_fst2_space_list;
	struct List_AbstractFst2Space* tmp_previous = NULL;

	while (tmp != NULL)
	{
		if ((memcmp(&tmp->afs.func_array,func_array,sizeof(t_persistent_fst2_func_array))==0) &&
			(tmp->afs.privateSpacePtr == privateSpacePtr))
		{
			if (tmp_previous == NULL)
				p_abstract_fst2_space_list = tmp->next;
			else
				tmp_previous->next = tmp->next;

			if ((tmp->afs.func_array.fnc_Uninit_Fst2Space) != NULL)
				(*(tmp->afs.func_array.fnc_Uninit_Fst2Space))(tmp->afs.privateSpacePtr);

			free(tmp);
			return 1;
		}
		tmp_previous = tmp;
		tmp = tmp->next;
	}
	return 0;
}


UNITEX_FUNC int UNITEX_CALL GetNbAbstractFst2SpaceInstalled()
{
    int count=0;
    struct List_AbstractFst2Space* tmp = p_abstract_fst2_space_list;
	while (tmp != NULL)
	{
        count++;
		tmp = tmp->next;
	}
	return count;
}


const AbstractFst2Space * GetFst2SpaceForFileName(const char*name)
{
	const struct List_AbstractFst2Space* tmp = p_abstract_fst2_space_list;

	while (tmp != NULL)
	{
		const AbstractFst2Space * test_afs = &(tmp->afs);
		if (tmp->afs.func_array.fnc_is_filename_object(name,tmp->afs.privateSpacePtr) != 0)
			return test_afs;

		tmp = tmp->next;
	}
	return NULL;
}

/*******************************/


Fst2* load_abstract_fst2(const VersatileEncodingConfig* vec,const char* filename,int read_names,struct FST2_free_info* p_fst2_free_info)
{
	Fst2* res = NULL;
	const AbstractFst2Space * pads = GetFst2SpaceForFileName(filename) ;
	if (pads == NULL)
	{
		res = load_fst2(vec, filename, read_names);

		if ((res != NULL) && (p_fst2_free_info != NULL))
		{
			p_fst2_free_info->must_be_free = 1;
			p_fst2_free_info->func_free_fst2 = NULL;
			p_fst2_free_info->private_ptr = NULL;
		}
		return res;
	}
	else
	{
		if (p_fst2_free_info != NULL)
		{
			p_fst2_free_info->must_be_free = 0;
			p_fst2_free_info->private_ptr = NULL;
			p_fst2_free_info->privateSpacePtr = pads->privateSpacePtr;
			p_fst2_free_info->func_free_fst2 = (void*)(pads->func_array.fnc_free_abstract_fst2);
		}
		res = (*(pads->func_array.fnc_load_abstract_fst2))(vec, filename,read_names,p_fst2_free_info,pads->privateSpacePtr);
		return res;
	}
}

int is_abstract_fst2_filename(const char* filename)
{
	return ((GetFst2SpaceForFileName(filename) == NULL) ? 0 : 1);
}


int is_abstract_or_persistent_fst2_filename(const char* filename)
{
	if (GetFst2SpaceForFileName(filename))
		return 1;
	if (get_persistent_structure(filename))
		return 1;
	return 0;
}


void free_abstract_Fst2(Fst2* fst2,struct FST2_free_info* p_fst2_free_info)
{
    if (fst2 != NULL)
    {
        if (p_fst2_free_info == NULL)
            free_Fst2(fst2);
        else
        {
		    if (p_fst2_free_info->must_be_free != 0)
		    {
			    if (p_fst2_free_info->func_free_fst2 == NULL)
				    free_Fst2(fst2);
			    else
			    {
				    t_fnc_free_abstract_fst2 fnc_free_abstract_fst2 = (t_fnc_free_abstract_fst2)(p_fst2_free_info->func_free_fst2);
				    if (fnc_free_abstract_fst2 != NULL)
					    (*fnc_free_abstract_fst2)(fst2,p_fst2_free_info,p_fst2_free_info->privateSpacePtr);
			    }
		    }
        }
    }
}

//} // namespace unitex
