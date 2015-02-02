/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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




#include <stdlib.h>
#include <string.h>
#include <stdio.h>



#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Error.h"
#include "AbstractFilePlugCallback.h"
#include "Af_stdio.h"

#include "DirHelper.h"

int mkDirPortable(const char* dirname)
{
    if (is_filename_in_abstract_file_space(dirname) != 0)
        return 0;
    int retMkDir = (int)(mkdir(dirname,S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP|S_IXGRP|S_IXOTH));
    /* retMkDir is 0 for success */
    return (retMkDir);
    //return mkdir(dirname);
}

/* Upon successful completion, 0 shall be returned. */
int chDirPortable(const char* dirname)
{
    if (is_filename_in_abstract_file_space(dirname) != 0)
        return 0;

    return chdir(dirname);
}

/* Upon successful completion, 0 shall be returned. */
int rmDirPortable(const char* dirname)
{
    if (is_filename_in_abstract_file_space(dirname) != 0)
        return 0;

    return rmdir(dirname);
}
