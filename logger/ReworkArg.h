/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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




/* ReworkArg.h
*/

#ifndef _REWORK_ARG_H
#define _REWORK_ARG_H


#ifdef __cplusplus
extern "C" {
#endif

unsigned int get_filename_withoutpath_position(const char*filename);

int SearchPossiblePosFileNameInArg(const char*arg);

void CopyReworkedArgRemoving(char* dst,const char* arg,const char*portion_ignore_pathname);

void reworkCommandLineAddPrefix(char*dest,const char*org,const char* FileRunPath,
                                const char**p_position_filename,const char** p_portionFileNameFromParam);

const char* get_filename_to_copy(const char*filename,int skip_star);

const char* GetFileNameRemovePrefixIfFound(const char* filename,const char*portion_ignore_pathname);

const char* ExtractUsablePortionOfFileNameForPack(const char*filenamecpy);

#ifdef __cplusplus
}
#endif

#endif
