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

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "File.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Takes a file name and copies its extension in a given string
 * that is supposed to be allocated. If there is no extension, the
 * empty string is copied.
 */
void get_extension(const char* filename,char* extension) {
int l;
l=(int)strlen(filename)-1;
while (l>=0 && filename[l]!='/' && filename[l]!='\\' && filename[l]!='.') {
   l--;
}
if (l==0 || filename[l]!='.') {
   /* If there is no extension */
   extension[0]='\0';
   return;
}
int i=-1;
do {
   i++;
   extension[i]=filename[l+i];
} while (extension[i]!='\0');
}


/**
 * Takes a file name and removes its extension, if any.
 */
void remove_extension(char* filename) {
int l;
l=(int)strlen(filename)-1;
while (l>=0 && filename[l]!='/' && filename[l]!='\\' && filename[l]!='.') {
   l--;
}
if (l>=0 && filename[l]=='.') filename[l]='\0';
}


/**
 * Takes a file name and copies it into 'result' without its extension,
 * if any.
 */
void remove_extension(const char* filename,char* result) {
strcpy(result,filename);
remove_extension(result);
}


/**
 * Takes a file name and copies its path, with the separator character,
 * into 'path'. If the file name is a relative one, then a relative path is
 * returned.
 */
void get_path(const char* filename,char* path) {
int l;
strcpy(path,filename);
l=(int)strlen(path)-1;
while (l>=0 && path[l]!='/' && path[l]!='\\') {
   l--;
}
if (l>=0) path[l+1]='\0';
else path[0]='\0';
}

/**
 * @brief Unitex implementation of realpath()
 *
 * Derives from the pathname pointed to by file_name, an
 * absolute pathname that names the same file, whose
 * resolution does not involve ".", "..", or symbolic
 * links.
 * 
 * @param[in]  file_name null-terminated string, with a filename to resolve
 * @param[out] resolved_name null-terminated string, up to a maximum of {FILENAME_MAX} bytes
 * @return     different from SUCCESS_RETURN_CODE if fails
 *
 * @remark     If resolved_name is a null pointer, function will fail
 */ 
int get_real_path(const char* file_name, char* resolved_name) {
 // check that the arguments are not NULL pointers
 fatal_assert(!file_name,    "NULL error in get_realpath file_name\n");  
 fatal_assert(!resolved_name,"NULL error in get_realpath resolved_name\n");

 // check that the file name is not empty
 if (file_name[0]=='\0') {
  return DEFAULT_ERROR_CODE;
 }

 // check that the file exists and can be read
 if(!fexists(file_name)) {
  return DEFAULT_ERROR_CODE;
 }
 
 // if the file is under the abstract file layer
 // the name is already resolved
 if(is_filename_in_abstract_file_space(file_name)) {
  strcpy(resolved_name, file_name);
  return SUCCESS_RETURN_CODE;
 }

 // code to be returned by the function
 int return_code = DEFAULT_ERROR_CODE;

 // temporal buffer to store the resolved_name
 char buffer[FILENAME_MAX + 1] = {0}; 
 
#ifdef _NOT_UNDER_WINDOWS
 // try to get the resolved filename
 if (realpath(file_name, buffer) != NULL) {
  return_code = SUCCESS_RETURN_CODE;
 }
#else   // under Windows
 // try to get the resolved filename
 if (_fullpath(buffer, file_name, FILENAME_MAX)) {
  return_code = SUCCESS_RETURN_CODE; 
 }
#endif  // #ifdef _NOT_UNDER_WINDOWS

 // if we have a resolved name copy it into the
 // resolved_name output buffer
 if(return_code == SUCCESS_RETURN_CODE) {
   strncpy(resolved_name, buffer, FILENAME_MAX);
 }
  
 return return_code;
}

/**
 * Takes a file name, removes its extension and adds the suffix "_snt"
 * followed by the separator character.
 *
 * Example: filename="C:\English\novel.txt" => result="C:\English\novel_snt\"
 */
void get_snt_path(const char* filename,char* result) {
remove_extension(filename,result);
strcat(result,"_snt");
strcat(result,PATH_SEPARATOR_STRING);
}


/**
 * Takes a file name and copies it without its path, if any, into 'result'.
 */
void remove_path(const char* filename,char* result) {
int l=(int)strlen(filename)-1;
while (l>=0 && filename[l]!='/' && filename[l]!='\\') {
   l--;
}
if (l<0) {
   strcpy(result,filename);
   return;
}
int k=0;
for (int i=l+1;filename[i]!='\0';i++) {
   result[k++]=filename[i];
}
result[k]='\0';
}

/**
 * Takes a file name and return pointer without its path
 */
const char* filename_without_path(const char* filename) {
int l=(int)strlen(filename)-1;
while (l>=0 && filename[l]!='/' && filename[l]!='\\') {
   l--;
}
if (l<0) {
   return filename;
}
return filename + l + 1;
}

/**
 * Takes a file name and copies it without its path and extension, if
 * any, into 'result'.
 */
void remove_path_and_extension(const char* filename,char* result) {
char temp[FILENAME_MAX];
remove_path(filename,temp);
remove_extension(temp,result);
}


/*
 * Adds a suffix to the file name before the extension:
 * "tutu.txt" + "-old" => "tutu-old.txt"
 **/
void add_suffix_to_file_name(char* dest,const char* src,const char* suffix) {
char ext[128];
remove_extension(src,dest);
strcat(dest,suffix);
get_extension(src,ext);
strcat(dest,ext);
}


/*
 * Adds a prefix to the file name:
 * "tutu.txt" + "old-" => "old-tutu.txt"
 **/
void add_prefix_to_file_name(char* dest,const char* src,const char* prefix) {
char tmp[1024];
get_path(src,dest);
strcat(dest,prefix);
remove_path(src,tmp);
strcat(dest,tmp);
}


/**
 * Replaces path separators ('/' resp. '\\') by the colon (':').
 */
void replace_path_separator_by_colon(char* filename) {
while (*filename) {
   if (*filename==PATH_SEPARATOR_CHAR) {
      *filename = ':';
   }
   filename++;
}
}


/**
 * Replaces colon (':'), the "universal" path separator
 * by system-dependent path separators ('/' resp. '\\').
 */
void replace_colon_by_path_separator(char* filename) {
while (*filename) {
   if (*filename==':') {
      *filename=PATH_SEPARATOR_CHAR;
   }
   filename++;
}
}


/**
 * This function builds a new absolute file name from a path and a file name.
 * 'result' is supposed to be allocated.
 *
 * Example: path="/tmp/test/" name="hello.txt"
 *       => result="/tmp/test/toto.txt"
 */
void new_file(const char* path,const char* name,char* result) {
if (path==NULL || name==NULL) {
   fatal_error("NULL error in new_file\n");
}
if (path[0]=='\0') {
   fatal_error("Empty path in new_file\n");
}
if (name[0]=='\0') {
   fatal_error("Empty file name in new_file\n");
}
int l=(int)strlen(path);
/* Here we test if the path already contains a path separator char, but
 * we are tolerant: we admit to have a wrong separator char. */
int length_without_separator=l-((path[l-1]=='/' || path[l-1]=='\\')?1:0);
/* WARNING: we don't want to modify the existing path and we want to be sure
 * to put the correct path separator char. So, we copy the path except the
 * separator char, if any. */
strncpy(result,path,length_without_separator);
/* Then, we don't do a strcat since strncpy didn't put a '\0' at the end of 'res' */
result[length_without_separator]=PATH_SEPARATOR_CHAR;
/* We do the following to avoid putting a '\0' and then doing a strcat */
strcpy(&(result[length_without_separator+1]),name);
}


/**
 * This function copies the file 'src' into 'dest'.
 * Author: Olivier Blanc
 * Modified by Sébastien Paumier then Gilles Vollant
 */
void copy_file(const char* dest,const char* src) {
int res=af_copy(src,dest);
if (res!=0) {
    if (res==1) {
        fatal_error("error in writing when copy '%s' to '%s'\n",src,dest);
    }
    else
        fatal_error("error in reading when copy '%s' to '%s'\n",src,dest);
}
}


/**
 * Returns 1 if the given file exists and can be read; 0 otherwise.
 */
int fexists(const char* name) {
ABSTRACTFILE* f=af_fopen(name,"rb");
if (f==NULL) return 0;
af_fclose(f);
return 1;
}


/**
 * Returns a value corresponding to the file date.
 */
time_t get_file_date(const char* name) {
struct stat info;
stat(name,&info);
return info.st_mtime;
}


/**
 * Returns the size in bytes of the given file, or -1 if not found.
 */
long get_file_size(const char* name) {
U_FILE* f=u_fopen(ASCII,name,U_READ);
if (f==NULL) return -1;
fseek(f,0,SEEK_END);
long size=ftell(f);
u_fclose(f);
return size;
}


/**
 * Returns the size in bytes of the given file.
 */
long get_file_size(U_FILE* f) {
long old_pos=ftell(f);
fseek(f,0,SEEK_END);
long size=ftell(f);
fseek(f,old_pos,SEEK_SET);
return size;
}


/**
 * Adds the path separator char at the end of the given string if not
 * already present and returns 1. Returns 0 otherwise.
 */
int add_path_separator(char* path) {
int l=(int)strlen(path);
if (path[l-1]==PATH_SEPARATOR_CHAR) {
   return 0;
}
path[l]=PATH_SEPARATOR_CHAR;
path[l+1]='\0';
return 1;
}


/**
 * Returns 1 if the given path is absolute; 0 otherwise.
 */
int is_absolute_path(const char* path) {
#ifdef _NOT_UNDER_WINDOWS
return path[0]==PATH_SEPARATOR_CHAR;
#else
return ((path[0]>='a' && path[0]<='z') || (path[0]>='A' && path[0]<='Z'))
       && path[1]==':' && path[2]=='\\';
#endif
}

/**
 * Returns 1 if the given path is a root; 0 otherwise.
 */
int is_root(const char* path) {
#ifdef _NOT_UNDER_WINDOWS
return !strcmp(path,PATH_SEPARATOR_STRING);
#else
return ((path[0]>='a' && path[0]<='z') || (path[0]>='A' && path[0]<='Z'))
       && path[1]==':' && path[2]=='\\' && path[3]=='\0';
#endif
}


#if defined(WINAPI_FAMILY) && defined(WINAPI_FAMILY_APP)
#if WINAPI_FAMILY==WINAPI_FAMILY_APP
#ifndef PREVENT_USING_METRO_INCOMPATIBLE_FUNCTION
#define PREVENT_USING_METRO_INCOMPATIBLE_FUNCTION 1
#endif
#endif
#endif

#ifdef PREVENT_USING_METRO_INCOMPATIBLE_FUNCTION
#include "DirHelper.h"
static int create_path(const char* path) {
	return mkDirPortable(path);
}
#else
static int create_path(const char* path) {
char command[10+FILENAME_MAX];
#ifdef _NOT_UNDER_WINDOWS
sprintf(command,"mkdir -p %s",path);
return system(command);
#else
sprintf(command,"mkdir %s",path);
return system(command);
#endif
}
#endif

/**
 * Tries to create all directories that lead to 'name' with
 * system("mkdir ...");
 */
int create_path_to_file(const char* name) {
char path[FILENAME_MAX];
get_path(name,path);
return create_path(path);
}

} // namespace unitex
