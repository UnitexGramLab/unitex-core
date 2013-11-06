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




#include <direct.h>
#include "AbstractFilePlugCallback.h"
#include "Af_stdio.h"

#include "DirHelper.h"

#if (defined(_WIN32)) || defined(WIN32)
#include <windows.h>
#endif

#if defined(WINAPI_FAMILY_PARTITION) && (!(defined(UNITEX_USING_WINRT_API)))
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#if (!(defined(UNITEX_USING_WINRT_API))) && (!(defined(UNITEX_PREVENT_USING_WINRT_API)))
#define UNITEX_USING_WINRT_API 1
#endif
#endif
#endif

int mkDirPortable(const char* dirname)
{
    if (is_filename_in_abstract_file_space(dirname) != 0)
        return 0;

#ifdef UNITEX_USING_WINRT_API
    return CreateDirectoryA(dirname,NULL) ? 0 : -1;
#else
    return mkdir(dirname);
#endif
}

#ifndef PREVENT_USING_METRO_INCOMPATIBLE_FUNCTION
int chDirPortable(const char* dirname)
{
    if (is_filename_in_abstract_file_space(dirname) != 0)
        return 0;

    return chdir(dirname);
}
#endif

int rmDirPortable(const char* dirname)
{
    if (is_filename_in_abstract_file_space(dirname) != 0)
        return 0;

    return rmdir(dirname);
}
