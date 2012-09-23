/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


#include <string.h>
#include "UnitexLibDir.h"

// see http://stackoverflow.com/questions/3833581/recursive-file-delete-in-c-on-linux

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

#define printf(a,b)
#define perror(a) return (-1)

int recursiveDelete(const char* dirname) {
    
    DIR *dp;
    struct dirent *ep;
    
    char abs_filename[FILENAME_MAX];
    
    dp = opendir (dirname);
    if (dp != NULL)
    {
        while (NULL != (ep = readdir (dp))) {
            struct stat stFileInfo;
            
            snprintf(abs_filename, FILENAME_MAX, "%s/%s", dirname, ep->d_name);
            
            if (lstat(abs_filename, &stFileInfo) < 0)
                perror ( abs_filename );
            
            if(S_ISDIR(stFileInfo.st_mode)) {
                if(strcmp(ep->d_name, ".") && 
                   strcmp(ep->d_name, "..")) {
                    printf("%s directory\n",abs_filename);
                    recursiveDelete(abs_filename);
                }
            } else {
                printf("%s file\n",abs_filename);
                remove(abs_filename);
            }
        }
        (void) closedir (dp);
    }
    else
        perror ("Couldn't open the directory");
    
    
    remove(dirname);
    return 0;
    
}

int RemoveFileSystemFolder(const char*foldername)
{
    return recursiveDelete(foldername);
}
