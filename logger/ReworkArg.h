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

#ifndef _REWORK_ARG_H
#define _REWORK_ARG_H


#ifdef __cplusplus
extern "C" {
#endif


int SearchPossiblePosFileNameInArg(const char*arg);

void CopyReworkedArgRemoving(char* dst,const char* arg,const char*portion_ignore_pathname);

void reworkCommandLineAddPrefix(char*dest,const char*org,const char* FileRunPath);

const char* get_filename_to_copy(const char*filename);

const char* GetFileNameRemovePrefixIfFound(const char* filename,const char*portion_ignore_pathname);

#ifdef __cplusplus
}
#endif

#endif
