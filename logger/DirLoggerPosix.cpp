 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


#include <stdlib.h>
#include <string.h>
#include <stdio.h>



#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "DirLogger.h"

int mkDirPortable(const char* dirname)
{
    int retMkDir = (int)(mkdir(dirname,S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP|S_IXGRP|S_IXOTH));
    /* retMkDir is 0 for success */
    return (retMkDir);
    //return mkdir(dirname);
}

/* Upon successful completion, 0 shall be returned. */
int chDirPortable(const char* dirname)
{
    return chdir(dirname);
}

/* Upon successful completion, 0 shall be returned. */
int rmDirPortable(const char* dirname)
{
    return rmdir(dirname);
}
