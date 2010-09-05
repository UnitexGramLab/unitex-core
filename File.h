/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef FileNameH
#define FileNameH


/**
 * This library provides functions for manipulating files and file
 * names. File names should be stored in arrays like:
 *
 *    char filename[FILENAME_MAX];
 *
 * FILENAME_MAX is a system-dependent constant that defines the maximum
 * size of a file name. It is defined in <stdio.h>.
 */

#ifdef _NOT_UNDER_WINDOWS
   #define PATH_SEPARATOR_CHAR '/'
   #define PATH_SEPARATOR_STRING "/"
#else
   #define PATH_SEPARATOR_CHAR '\\'
   #define PATH_SEPARATOR_STRING "\\"
#endif

#include <sys/types.h>
#include <stdio.h>
#include "Unicode.h"

void add_suffix_to_file_name(char*,const char*,const char*);
void add_prefix_to_file_name(char*,const char*,const char*);
void get_extension(const char*,char*);
void remove_extension(char*);
void remove_extension(const char*,char*);
void get_path(const char*,char*);
void get_snt_path(const char*,char*);
void remove_path(const char*,char*);
void remove_path_and_extension(const char*,char*);
void replace_path_separator_by_colon(char*);
void replace_colon_by_path_separator(char*);
void new_file(const char*,const char*,char*);
void copy_file(const char*,const char*);
int fexists(const char*);
time_t get_file_date(const char* name);
long get_file_size(const char*);
long get_file_size(U_FILE*);
int add_path_separator(char*);
int is_absolute_path(const char*);

#endif
