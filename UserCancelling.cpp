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
#include <stdlib.h>
#include <memory.h>
#include "UserCancelling.h"
#include "UserCancellingPlugCallback.h"



struct UserCancellingInfo {
	t_user_cancelling_func_array func_array;
	void* privateCancelPtr;
} ;


struct List_UserCancellingInfo {
	UserCancellingInfo lgi;
	List_UserCancellingInfo* next;
} ;


struct List_UserCancellingInfo* p_user_cancelling_info_list=NULL;



UNITEX_FUNC int UNITEX_CALL AddUserCancellingInfo(const t_user_cancelling_func_array* func_array,void* privateCancelPtr)
{
	struct List_UserCancellingInfo* new_item;
    if (func_array->fnc_is_cancelling == NULL)
        return 0;

	new_item = (struct List_UserCancellingInfo*)malloc(sizeof(struct UserCancellingInfo));
	if (new_item == NULL)
		return 0;

	new_item->lgi.func_array = *func_array;
	new_item->lgi.privateCancelPtr = privateCancelPtr;
	new_item->next = NULL;

	if (p_user_cancelling_info_list == NULL)
		p_user_cancelling_info_list = new_item;
	else {
		struct List_UserCancellingInfo* tmp = p_user_cancelling_info_list;
		while ((tmp->next) != NULL)
			tmp = tmp->next;
		tmp->next = new_item;
	}

	if ((new_item->lgi.func_array.fnc_Init_User_Cancelling) != NULL)
		(*(new_item->lgi.func_array.fnc_Init_User_Cancelling))(new_item->lgi.privateCancelPtr);

	return 1;
}

UNITEX_FUNC int UNITEX_CALL RemoveUserCancellingInfo(const t_user_cancelling_func_array* func_array,void* privateCancelPtr)
{
	struct List_UserCancellingInfo* tmp = p_user_cancelling_info_list;
	struct List_UserCancellingInfo* tmp_previous = NULL;

	while (tmp != NULL)
	{
		if ((memcmp(&tmp->lgi.func_array,func_array,sizeof(t_user_cancelling_func_array))==0) &&
			(tmp->lgi.privateCancelPtr == privateCancelPtr))
		{
			if (tmp_previous == NULL)
				p_user_cancelling_info_list = tmp->next;
			else
				tmp_previous->next = tmp->next;

			if ((tmp->lgi.func_array.fnc_Uninit_User_Cancelling) != NULL)
				(*(tmp->lgi.func_array.fnc_Uninit_User_Cancelling))(tmp->lgi.privateCancelPtr);

			free(tmp);
			return 1;
		}
		tmp_previous = tmp;
		tmp = tmp->next;
	}
	return 0;
}


UNITEX_FUNC int UNITEX_CALL GetNbUserCancellingInfoInstalled()
{
    int count=0;
    struct List_UserCancellingInfo* tmp = p_user_cancelling_info_list;
	while (tmp != NULL)
	{
        count++;
		tmp = tmp->next;
	}
	return count;
}

int is_cancelling_requested()
{
    struct List_UserCancellingInfo* tmp = p_user_cancelling_info_list;
	while (tmp != NULL)
	{
        if (((*(tmp->lgi.func_array.fnc_is_cancelling))(tmp->lgi.privateCancelPtr)) != 0)
            return 1;
        tmp = tmp->next;
	}
    return 0;
}


/*
 * this is a library function for external use
 */
UNITEX_FUNC int UNITEX_CALL UnitexIsUserCancellingRequested()
{
    return is_cancelling_requested();
}
