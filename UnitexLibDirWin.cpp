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


#include <string.h>
#include <windows.h>
#include "UnitexLibDir.h"

#ifdef WIN32


#if defined(WINAPI_FAMILY_PARTITION) && (!(defined(UNITEX_USING_WINRT_API)))
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#if (!(defined(UNITEX_USING_WINRT_API))) && (!(defined(UNITEX_PREVENT_USING_WINRT_API)))
#define UNITEX_USING_WINRT_API 1
#endif
#endif
#endif

// todo : implement using http://stackoverflow.com/questions/1468774/recursive-directory-deletion-with-win32
// or http://www.codeproject.com/KB/files/deletedir.aspx



#ifdef UNITEX_USING_WINRT_API
#include <strsafe.h>
static BOOL IsDotsW(const WCHAR* str) {
	if ((*str)!='.')
		return FALSE;
	if ((*str)=='\0')
		return TRUE;
	if ((*(str+1))=='.')
		if ((*(str+2))=='\0')
			return TRUE;
    return FALSE;
}

BOOL DeleteDirectoryW(const WCHAR* sPath) {



    HANDLE hFind;  // file handle

    WIN32_FIND_DATAW FindFileData;
    
    WCHAR DirPath[MAX_PATH+0x200];
    WCHAR FileName[MAX_PATH+0x200];
    
    StringCchCopyW(DirPath,MAX_PATH+0x200,sPath);
    StringCchCat(DirPath,MAX_PATH+0x200,L"\\*");    // searching all files
    
    StringCchCopyW(FileName,MAX_PATH+0x200,sPath);
    StringCchCatW(FileName,MAX_PATH+0x200,L"\\");
    
    hFind = FindFirstFileExW(DirPath,FindExInfoBasic,&FindFileData,FindExSearchNameMatch,NULL,0); // find the first file
    if(hFind == INVALID_HANDLE_VALUE) return FALSE;
    StringCchCopyW(DirPath,MAX_PATH+0x200,FileName);
    
    bool bSearch = true;
    while(bSearch) { // until we finds an entry
        if(FindNextFileW(hFind,&FindFileData)) {
            if(IsDotsW(FindFileData.cFileName)) continue;
			StringCchCatW(FileName,MAX_PATH+0x200,FindFileData.cFileName);
            //strcat(FileName,FindFileData.cFileName);
            if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                
                // we have found a directory, recurse
                if(!DeleteDirectoryW(FileName)) { 
                    FindClose(hFind); 
                    return FALSE; // directory couldn't be deleted
                }
                RemoveDirectoryW(FileName); // remove the empty directory
                //strcpy(FileName,DirPath);
				StringCchCopyW(FileName,MAX_PATH+0x200,DirPath);
            }
            else {

                if(!DeleteFileW(FileName)) {  // delete the file
                    FindClose(hFind); 
                    return FALSE; 
                }                 
                //strcpy(FileName,DirPath);
				StringCchCopy(FileName,MAX_PATH+0x200,DirPath);
            }
        }
        else {
            if(GetLastError() == ERROR_NO_MORE_FILES) // no more files there
                bSearch = false;
            else {
                // some error occured, close the handle and return FALSE
                FindClose(hFind); 
                return FALSE;
            }
            
        }
        
    }
    FindClose(hFind);  // closing file handle
    
    return RemoveDirectoryW(sPath); // remove the empty directory
    
}


BOOL DeleteDirectory(const char* sPath) {
	WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
	MultiByteToWideChar(CP_ACP,0,sPath,-1,filenameW,FILENAME_MAX + 0x200);
	return DeleteDirectoryW(filenameW);
}
#else

static BOOL IsDots(const char* str) {
    if(strcmp(str,".") && strcmp(str,"..")) return FALSE;
    return TRUE;
}

BOOL DeleteDirectory(const char* sPath) {
    HANDLE hFind;  // file handle

    WIN32_FIND_DATAA FindFileData;
    
    char DirPath[MAX_PATH];
    char FileName[MAX_PATH];
    
    strcpy(DirPath,sPath);
    strcat(DirPath,"\\*");    // searching all files
    
    strcpy(FileName,sPath);
    strcat(FileName,"\\");
    
    hFind = FindFirstFileA(DirPath,&FindFileData); // find the first file
    if(hFind == INVALID_HANDLE_VALUE) return FALSE;
    strcpy(DirPath,FileName);
    
    bool bSearch = true;
    while(bSearch) { // until we finds an entry
        if(FindNextFileA(hFind,&FindFileData)) {
            if(IsDots(FindFileData.cFileName)) continue;
            strcat(FileName,FindFileData.cFileName);
            if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                
                // we have found a directory, recurse
                if(!DeleteDirectory(FileName)) { 
                    FindClose(hFind); 
                    return FALSE; // directory couldn't be deleted
                }
                RemoveDirectoryA(FileName); // remove the empty directory
                strcpy(FileName,DirPath);
            }
            else {

                if(!DeleteFileA(FileName)) {  // delete the file
                    FindClose(hFind); 
                    return FALSE; 
                }                 
                strcpy(FileName,DirPath);
            }
        }
        else {
            if(GetLastError() == ERROR_NO_MORE_FILES) // no more files there
                bSearch = false;
            else {
                // some error occured, close the handle and return FALSE
                FindClose(hFind); 
                return FALSE;
            }
            
        }
        
    }
    FindClose(hFind);  // closing file handle
    
    return RemoveDirectoryA(sPath); // remove the empty directory
    
}

#endif

int recursiveDelete(const char* dirname)
{    
    return DeleteDirectory(dirname) ? 0 : -1;
}

int RemoveFileSystemFolder(const char*foldername)
{
    return recursiveDelete(foldername);
}

#endif
