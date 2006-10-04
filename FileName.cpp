 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------

#include <string.h>

#include "FileName.h"
#include "Error.h"
//---------------------------------------------------------------------------


void file_name_extension(const char* s, char* res) {
int l;
l=strlen(s)-1;
while (l>=0 && s[l]!='/' && s[l]!='\\' && s[l]!='.') {
   l--;
}
if (l==0 || s[l]!='.') {
   // if there is no extension 
   res[0]='\0';
   return;
}
int i=-1;
do {
  i++;
  res[i]=s[l+i];
} while (res[i]!='\0');
return;
}



void name_without_extension(const char* s, char* res) {
int l;
strcpy(res,s);
l=strlen(res)-1;
while (l>=0 && res[l]!='/' && res[l]!='\\' && res[l]!='.') {
   l--;
}
if (res[l]=='.') res[l]='\0';
}



void get_filename_path(char* s,char* res) {
int l;
strcpy(res,s);
l=strlen(res)-1;
while (l>=0 && res[l]!='/' && res[l]!='\\') {
   l--;
}
if (l>=0) res[l+1]='\0';
else res[0]='\0';
}


void get_snt_path(char* s, char* res) {
int l;
strcpy(res,s);
l=strlen(res)-1;
while (l>=0 && res[l]!='/' && res[l]!='\\' && res[l]!='.') {
   l--;
}
if (res[l]=='.') res[l]='\0';
strcat(res,"_snt/");
}



void name_without_path(char* s,char* res) {
int l=strlen(s)-1;
while (l>=0 && s[l]!='/' && s[l]!='\\') {
   l--;
}
if (l<0) {
   strcpy(res,s);
   return;
}
int k=0;
for (int i=l+1;s[i]!='\0';i++) {
   res[k++]=s[i];
}
res[k]='\0';
}



/*
 * adds a suffix to the file name before the extension:
 * "tutu.txt" + "-old" => "tutu-old.txt"
 **/
void add_suffix_to_file_name(char* dest,char* src,const char* suffix) {
char ext[128];
name_without_extension(src,dest);
strcat(dest,suffix);
file_name_extension(src,ext);
strcat(dest,ext);
}


/*
 * adds a prefix to the file name:
 * "tutu.txt" + "old-" => "old-tutu.txt"
 **/
void add_prefix_to_file_name(char* dest,char* src,const char* prefix) {
char tmp[1024];
get_filename_path(src,dest);
strcat(dest,prefix);
name_without_path(src,tmp);
strcat(dest,tmp);
}

/***
 * Copy src to dest replacing suffix1 by suffix2, e.g.
 * file.grf -> file.fst2
 *
 * dest must be large enough to hold the resulting string.
 **/
void replace_suffix_in_file_name(char* dest,const char* src,
                                 const char* suffix1,const char* suffix2) {
  int i, j, k;
  j = strlen(src);
  k = strlen(suffix1);
  if ( k > j )
    fatal_error("Error in %s line %s:\n"
                "suffix \"%s\" is longer than \"%s\"\n",
                __FILE__, __LINE__, suffix1, src);
  for ( i=0; i<=k; i++ )
    {
      if ( src[(j-i)] != suffix1[(k-i)] )
        fatal_error("Error in %s line %s:\n"
                    "Can't replace suffix \"%s\" in \"%s\"\n",
                    __FILE__, __LINE__, suffix1, src);
    }
  strncpy(dest,src,(j-i+1));
  dest[(j-i+1)] = '\0';
  strcat(dest,suffix2);
}



/**
 * replace path separators ('/' resp. '\\') by the colon (':')
 * @param path
 */
void replace_pathseparator_by_colon(char* path) {
  while (*path)
  {
#ifdef _NOT_UNDER_WINDOWS
    if (*path == '/')
#else
    if (*path == '\\')
#endif
      { *path = ':'; }
    ++path;
  }
}

/**
 * replace colon (':'), the "universal" path separator
 * by system-dependent path separators ('/' resp. '\\')
 * @param path
 */
void replace_colon_by_pathseparator(char* path) {
  while (*path)
    {
#ifdef _NOT_UNDER_WINDOWS
      if (*path == ':') { *path = '/'; }
#else
      if (*path == ':') { *path = '\\'; }
#endif
      ++path;
    }
}

// /**
//  * replace path separators ('/' resp. '\\') by the colon (':')
//  * @param path
//  */
// void replace_pathseparator_by_colon(unichar* path) {
// unichar* ptr;
// for (ptr=path; *ptr!='\0'; ptr++)
//   {
// #ifdef _NOT_UNDER_WINDOWS
//     if (*ptr == (unichar) '/')
// #else
//     if (*ptr == (unichar) '\\')
// #endif
//       { *ptr = (unichar) ':'; }
//   }
// }

