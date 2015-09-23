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



char** buildListFileInDiskDir(const char*dirname)
{
	DIR *dir;
	struct dirent *ent;
	dir = opendir(dirname);


	if (dir == NULL)
		return NULL;

	int count = 0;
	char**ret = (char**)malloc(sizeof(char*));
	if (ret == NULL)
	{
		closedir(dir);
		return NULL;
	}
	*ret = NULL;

	while ((ent = readdir(dir)) != NULL) {
	{
		char** newret = (char**)realloc(ret, sizeof(char*)*(count + 2));
		if (newret == NULL)
		{
			break;
		}
		ret = newret;
		*(newret + count) = (char*)malloc(strlen(ent->d_name) + 1);
		if ((*(newret + count)) == NULL)
			break;
		strcpy(*(newret + count), ent->d_name);
		count++;
		*(newret + count) = NULL;
	}

	closedir(dir);
	return ret;
}
