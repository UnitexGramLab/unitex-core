/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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



#include "LocatePattern.h"
#include "LocateTrace.h"
#include "LocateTracePlugCallback.h"





UNITEX_FUNC int UNITEX_CALL SetLocateTraceInfo(const t_locate_trace_func_array* func_array,void* privatePtrGlobal);
UNITEX_FUNC int UNITEX_CALL RemoveLocateTraceInfo(const t_locate_trace_func_array* func_array,void* privatePtrGlobal);

/* just return the number of user cancelling Installed */
UNITEX_FUNC int UNITEX_CALL IsLocateTraceInfoInstalled();

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
int is_locate_trace_installed = 0;
t_locate_trace_func_array cur_locate_trace_func_array ;
void* privatePtrGlobal=NULL;

void open_locate_trace(struct locate_parameters* p,t_fnc_locate_trace_step * p_fnc_locate_trace_step,void** p_private_param_locate_trace)
{
    if (is_locate_trace_installed != 0)
    {
        *p_private_param_locate_trace = (*(cur_locate_trace_func_array.fnc_open_locate_trace))(privatePtrGlobal,p);
        *p_fnc_locate_trace_step = cur_locate_trace_func_array.fnc_locate_trace_step;
    }
}

void close_locate_trace(struct locate_parameters* p,t_fnc_locate_trace_step /*fnc_locate_trace_step*/,void* private_param_locate_trace)
{
    if (is_locate_trace_installed != 0)
    {
        (*(cur_locate_trace_func_array.fnc_close_locate_trace))(private_param_locate_trace,privatePtrGlobal,p);
    }
}


} // namespace unitex


UNITEX_FUNC int UNITEX_CALL SetLocateTraceInfo(const t_locate_trace_func_array* func_array,void* privatePtrGlobalSet)
{
    is_locate_trace_installed = 1;
    cur_locate_trace_func_array = *func_array;
    privatePtrGlobal = privatePtrGlobalSet;
    return 1;
}

UNITEX_FUNC int UNITEX_CALL RemoveLocateTraceInfo(const t_locate_trace_func_array* /*func_array*/,void* /*privatePtrGlobal*/)
{
    is_locate_trace_installed = 0;
    return 1;
}

UNITEX_FUNC int UNITEX_CALL IsLocateTraceInfoInstalled()
{
    return is_locate_trace_installed;
}
