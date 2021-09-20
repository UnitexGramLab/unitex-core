/*
 * Unitex - Performance optimization code 
 *
 * File created and contributed by Gilles Vollant, working with François Liger
 * as part of an UNITEX optimization and reliability effort, first descibed at
 * http://www.smartversion.com/unitex-contribution/Unitex_A_NLP_engine_from_the_lab_to_the_iPhone.pdf
 *
 * Free software when used with Unitex 3.2 or later
 *
 * Copyright (C) 2021-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "base/macro/helper/test.h"  // UNITEX_HAVE, UNITEX_USE
#include "base/thread/model.h"       // SINGLE_THREADED
/* ************************************************************************** */
#if UNITEX_USE(SINGLE_THREADED)
#include <stdlib.h>
#include "MiniMutex.h"
/* for monothread app, no mutex needed */

ULB_VFFUNC MINIMUTEX_OBJECT* ULIB_CALL BuildMutex()
{
    return NULL;
}

ULB_VFFUNC void ULIB_CALL GetMiniMutex(MINIMUTEX_OBJECT*)
{
}

ULB_VFFUNC void ULIB_CALL ReleaseMiniMutex(MINIMUTEX_OBJECT*)
{
}

ULB_VFFUNC void ULIB_CALL DeleteMiniMutex(MINIMUTEX_OBJECT*)
{
}
#endif
