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

/* ReworkArg.h
*/


#include <stdlib.h>
#include <string.h>

#include "File.h"
#include "ReworkArg.h"



const char* get_filename_to_copy(const char*filename)
{
    const char* filenamecpy=filename;


    if (((*filenamecpy)== '\\') && ((*(filenamecpy+1))== '\\'))
    {
        filenamecpy+=2;
        while (((*filenamecpy)!='\\') && ((*filenamecpy)!='\0'))
            filenamecpy++;
    }
    else
    {
        if (((*filenamecpy)== '.') && (((*(filenamecpy+1))== '\\') || ((*(filenamecpy+1))== '/')))
            filenamecpy+=2;
        else
        if ( ((((*filenamecpy)>= 'a') && ((*filenamecpy)<= 'z')) || (((*filenamecpy)>= 'A') && ((*filenamecpy)<= 'Z'))) &&
             ((*(filenamecpy+1))== ':'))
             filenamecpy+=2;
    }
    if (((*filenamecpy)== '\\') || ((*(filenamecpy))== '/'))
        filenamecpy++;
    return filenamecpy;
}


/* 
 * Search possible position for filename on an argument
 *
 * "-a" : return -1 (no filename possible)
 * "-oFileName.txt" : return 2
 * "--html" : return -1
 * "--alphabet=/Users/gutemberg/unitex/English/Alphabet.txt" return 11
 * "/Users/gutemberg/unitex/English/Alphabet.txt" return 0
 * "Alphabet.txt" return 0
 */
int SearchPossiblePosFileNameInArg(const char*arg)
{
    if ((*(arg)) == '-')
    {
        if ((*(arg+1)) != '-')
            return ((*(arg+2)) == '\0') ? -1 : 2;
        else /* we are in --xx args, we search --xx=yy */
        {
            unsigned int j=1;
            for (;;)
            {
                if ((*(arg+j)) == '\0')
                {
                    return -1;
                }

                if ((*(arg+j)) == '=')
                {
                    return j+1;
                }

                j++;
            }
        }
    }
    else
        return 0;
}

int countSeparatorInPathName(const char* pathname)
{
    int count=0;
    const char* browse=pathname;

    while ((*browse) != '\0')
    {
        if (((*browse) == '\\') || ((*browse) == '/'))
            count++;
        browse++;
    }
    return count;
}

void CopyReworkedArgRemoving(char* dst,const char* arg,const char*portion_ignore_pathname)
{
    int PossiblePos = -1;
    if (portion_ignore_pathname != NULL)
        if ((*portion_ignore_pathname) != '\0')
            PossiblePos = SearchPossiblePosFileNameInArg(arg);

    if ((PossiblePos == -1) || (portion_ignore_pathname == NULL))
        strcpy(dst,arg);
    else
    {
        size_t len_arg = strlen(arg);
        size_t len_ignore = strlen(portion_ignore_pathname);
        int NeedSkip = 0;
        if (PossiblePos + len_ignore <= len_arg)
            NeedSkip = memcmp(arg+PossiblePos,portion_ignore_pathname,len_ignore) == 0;
        if (NeedSkip == 0)
            strcpy(dst,arg);
        else
        {
            int posFileNameCopy=0;
            memcpy(dst,arg,(size_t)PossiblePos);
            int nbSeparatorInPathName = countSeparatorInPathName(arg+PossiblePos+len_ignore);
            if (nbSeparatorInPathName == 0)
            {
                /* if we have a filename parameter without separator, we add a "./" */
                *(dst+PossiblePos+posFileNameCopy) = '.';
                *(dst+PossiblePos+posFileNameCopy+1) = PATH_SEPARATOR_CHAR;
                posFileNameCopy=2;
            }
            strcpy(dst+PossiblePos+posFileNameCopy,arg+PossiblePos+len_ignore);
        }
    }
}

const char* GetFileNameRemovePrefixIfFound(const char* filename,const char*portion_ignore_pathname)
{
    if (portion_ignore_pathname == NULL)
        return filename;

    size_t len_filename = strlen(filename);
    size_t len_ignore_pathname = strlen(portion_ignore_pathname);

    if (len_filename > len_ignore_pathname)
        if (memcmp(filename,portion_ignore_pathname,len_ignore_pathname) == 0)
            return filename + len_ignore_pathname;
    return filename;
}

const char* ExtractUsablePortionOfFileNameForPack(const char*filenamecpy)
{
    while (((*filenamecpy) == '*') || ((*filenamecpy)== '#'))
        filenamecpy++;

    if (((*filenamecpy)== '\\') && ((*(filenamecpy+1))== '\\'))
    {
        filenamecpy+=2;
        while (((*filenamecpy)!='\\') && ((*filenamecpy)!='\0'))
            filenamecpy++;
    }
    else
        if ( ((((*filenamecpy)>= 'a') && ((*filenamecpy)<= 'z')) || (((*filenamecpy)>= 'A') && ((*filenamecpy)<= 'Z'))) &&
             ((*(filenamecpy+1))== ':'))
             filenamecpy+=2;
    if (((*filenamecpy)== '\\') || ((*(filenamecpy))== '/'))
        filenamecpy++;

    if ((*filenamecpy) == '.')
        if (((*(filenamecpy+1)) == '/') || ((*(filenamecpy+1)) == '\\'))
            filenamecpy+=2;
    return filenamecpy;
}

void reworkCommandLineAddPrefix(char*dest,const char*arg,const char* FileAddRunPath)
{
    int PossiblePos = -1;
    if (FileAddRunPath != NULL)
        if ((*FileAddRunPath) != '\0')
            PossiblePos = SearchPossiblePosFileNameInArg(arg);

    if (PossiblePos != -1)
    {
        const char* searchSep = arg + PossiblePos;
        int found_sep = 0;

        while ((*searchSep) != 0)
        {
            if (((*searchSep)=='\\') || ((*searchSep)=='/'))
                found_sep = 1;
            searchSep++;
        }

        if (found_sep == 0)
            PossiblePos = -1;
    }

    if (PossiblePos == -1)
        strcpy(dest,arg);
    else
    {
        size_t len_FileAddRunPath = strlen(FileAddRunPath);
        int iAddSeparator = 
        (((*(FileAddRunPath + len_FileAddRunPath - 1)) == '\\') || ((*(FileAddRunPath + len_FileAddRunPath - 1)) == '/')) ? 
            0 : 1;

        memcpy(dest,arg,PossiblePos);
        memcpy(dest+PossiblePos,FileAddRunPath,len_FileAddRunPath);
        if (iAddSeparator != 0)
            *(dest+PossiblePos+len_FileAddRunPath) = PATH_SEPARATOR_CHAR;
        strcpy(dest+PossiblePos+len_FileAddRunPath+iAddSeparator,get_filename_to_copy(arg + PossiblePos));

        char *browse=dest;

        while ((*browse) != 0)
        {
            if (((*browse)=='\\') || ((*browse)=='/'))
                *browse = PATH_SEPARATOR_CHAR;
            browse++;
        }
    }
}
