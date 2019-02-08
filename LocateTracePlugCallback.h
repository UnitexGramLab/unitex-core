/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * additional information: http://www.smartversion.com/unitex-contribution/
 * contact : info@winimage.com
 *
 */



#ifndef _LOCATE_TRACE_PLUG_CALLBACK_H_INCLUDED
#define _LOCATE_TRACE_PLUG_CALLBACK_H_INCLUDED 1


#include "AbstractCallbackFuncModifier.h"
#include "LocatePattern.h"

#ifdef __cplusplus
using namespace unitex;
extern "C" {
#endif

typedef void* (ABSTRACT_CALLBACK_UNITEX *t_fnc_open_locate_trace)(void* privatePtrGlobal,struct locate_parameters* p);
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_close_locate_trace)(void* open_value,void* privatePtrGlobal,struct locate_parameters* p);


typedef void* (ABSTRACT_CALLBACK_UNITEX *t_fnc_open_locate_trace_ex)(void* privatePtrGlobal,struct locate_parameters* p,char* const trace_params[]);
typedef struct
{
    unsigned int size_struct;

    t_fnc_open_locate_trace fnc_open_locate_trace;
    t_fnc_close_locate_trace fnc_close_locate_trace;

    t_fnc_locate_trace_step fnc_locate_trace_step;
} t_locate_trace_func_array;


typedef struct
{
    unsigned int size_struct;

    t_fnc_open_locate_trace fnc_open_locate_trace;
    t_fnc_close_locate_trace fnc_close_locate_trace;

    t_fnc_locate_trace_step fnc_locate_trace_step;

    t_fnc_open_locate_trace_ex fnc_open_locate_trace_ex;
} t_locate_trace_func_array_ex;

/* these functions respectively add and remove user cancelling.
  you can add several with the same func_array callback set, but with different privateCancelPtr
  privateCancelPtr is the parameters which can be set as the last parameter of each callback */
UNITEX_FUNC int UNITEX_CALL SetLocateTraceInfo(const t_locate_trace_func_array* func_array,void* privatePtrGlobal);
UNITEX_FUNC int UNITEX_CALL RemoveLocateTraceInfo(const t_locate_trace_func_array* func_array,void* privatePtrGlobal);

/* just return the number of user cancelling Installed */
UNITEX_FUNC int UNITEX_CALL IsLocateTraceInfoInstalled();

/**********************************************************************/



#ifdef __cplusplus
}
#endif

#endif
