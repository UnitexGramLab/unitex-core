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
#ifndef FileNameH
#define FileNameH
//---------------------------------------------------------------------------

#define REPLACE_FILE 0
#define PREFIX_SRC 1
#define SUFFIX_SRC 2
#define PREFIX_DEST 3
#define SUFFIX_DEST 4


void add_suffix_to_file_name(char*,char*,const char*);
void add_prefix_to_file_name(char*,char*,const char*);
void file_name_extension(const char*,char*);
void name_without_extension(const char*,char*);
void get_filename_path(char*,char*);
void get_snt_path(char*,char*);
void name_without_path(char*,char*);
void replace_suffix_in_file_name(char*,const char*,const char*,const char*);
void replace_pathseparator_by_colon(char*);
//void replace_pathseparator_by_colon(unichar*);
void replace_colon_by_pathseparator(char*);
#endif
