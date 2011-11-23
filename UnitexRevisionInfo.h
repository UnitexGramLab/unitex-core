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

#ifndef UnitexRevisionInfoH
#define UnitexRevisionInfoH


#include "AbstractCallbackFuncModifier.h"

int get_unitex_revision();
void get_unitex_version(unsigned int* major_version_number, unsigned int* minor_version_number);

#ifdef __cplusplus
extern "C" {
#endif


UNITEX_FUNC int UNITEX_CALL GetUnitexRevision();
UNITEX_FUNC void UNITEX_CALL GetUnitexVersion(unsigned int* major_version_number, unsigned int* minor_version_number);

#ifdef __cplusplus
}
#endif


#endif

