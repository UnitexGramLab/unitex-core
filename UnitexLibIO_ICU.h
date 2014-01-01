/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * https://github.com/ergonotics/JNI-for-Unitex-2.1
 * contact : unitex-contribution@ergonotics.com
 *
 */

/* these sample function uses UnitexLibIO to read and write UnitexFile 
  (in virtual file system or hard disk) from/to icu UnicodeString */


#ifndef UnitexLibIO_ICU_H
#define UnitexLibIO_ICU_H

#include <unicode/ustring.h>
#include <unicode/unistr.h>

#include "UnitexLibIO.h"

#ifdef __cplusplus
extern "C" {
#endif

	// note : uima c++ user can uses UnicodeStringRef
	UNITEX_FUNC int UNITEX_CALL WriteUnicodeUnitexFile(const char*, icu::UnicodeString const& uString);

	UNITEX_FUNC int UNITEX_CALL GetUnicodeStringFromUnitexFile(const char* filename, icu::UnicodeString& uString);

#ifdef __cplusplus
}
#endif

#endif
