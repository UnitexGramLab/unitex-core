 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef _ABSTRACT_FILE_CALLBACK_INCLUDED
#define _ABSTRACT_FILE_CALLBACK_INCLUDED 1

#include "AbstractCallbackFuncModifier.h"

typedef void* ABSTRACTFILE_PTR;

typedef long vfs_size_operation_type ;

typedef enum
{
  OPEN_READ_MF,
  OPEN_READWRITE_MF,
  OPEN_CREATE_MF
}
TYPEOPEN_MF;

typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_is_filename_vf_object)(const char*name,void* privateSpacePtr);
typedef size_t afs_size_type;
//typedef long afs_size_type;

typedef int (ABSTRACT_CALLBACK_UNITEX* t_fnc_Init_FileSpace)(void* privateSpacePtr);
typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_Uninit_FileSpace)(void* privateSpacePtr);

typedef ABSTRACTFILE_PTR (ABSTRACT_CALLBACK_UNITEX *t_fnc_memOpenLowLevel)
        (const char* FileName, TYPEOPEN_MF TypeOpen, void* privateSpacePtr);
typedef size_t (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelWrite)(ABSTRACTFILE_PTR llFile, void const *Buf, size_t size,void* privateSpacePtr);
typedef size_t (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelRead)(ABSTRACTFILE_PTR llFile, void *Buf, size_t size,void* privateSpacePtr);
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelSeek)(ABSTRACTFILE_PTR llFile, afs_size_type Pos, int TypeSeek,void* privateSpacePtr);
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelGetSize)(ABSTRACTFILE_PTR llFile, afs_size_type* pPos,void* privateSpacePtr);
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelTell)(ABSTRACTFILE_PTR llFile, afs_size_type* pPos,void* privateSpacePtr);
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelClose)(ABSTRACTFILE_PTR llFile,void* privateSpacePtr);
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelSetSizeReservation)(ABSTRACTFILE_PTR llFile, afs_size_type size_reserv,void* privateSpacePtr);
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memFileRemove)(const char* lpFileName,void* privateSpacePtr);
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memFileRename)(const char * _OldFilename, const char * _NewFilename,void* privateSpacePtr);

typedef struct
{
	t_fnc_is_filename_vf_object fnc_is_filename_object;

	t_fnc_Init_FileSpace fnc_Init_FileSpace;
	t_fnc_Uninit_FileSpace fnc_Uninit_FileSpace;

	t_fnc_memOpenLowLevel fnc_memOpenLowLevel;
	t_fnc_memLowLevelWrite fnc_memLowLevelWrite;
	t_fnc_memLowLevelRead fnc_memLowLevelRead;
	t_fnc_memLowLevelSeek fnc_memLowLevelSeek;
	t_fnc_memLowLevelGetSize fnc_memLowLevelGetSize;
	t_fnc_memLowLevelTell fnc_memLowLevelTell;
	t_fnc_memLowLevelClose fnc_memLowLevelClose;
	t_fnc_memLowLevelSetSizeReservation fnc_memLowLevelSetSizeReservation;
	t_fnc_memFileRemove fnc_memFileRemove;
	t_fnc_memFileRename fnc_memFileRename;
} t_fileio_func_array;


UNITEX_FUNC int UNITEX_CALL AddAbstractFileSpace(const t_fileio_func_array* func_array,void* privateSpacePtr);
UNITEX_FUNC int UNITEX_CALL RemoveAbstractFileSpace(const t_fileio_func_array* func_array,void* privateSpacePtr);




enum stdwrite_kind { stdwrite_kind_out=0, stdwrite_kind_err } ;

typedef size_t (ABSTRACT_CALLBACK_UNITEX *t_fnc_stdOutWrite)(const void*Buf, size_t size,void* privatePtr);
UNITEX_FUNC int UNITEX_CALL SetStdWriteCB(enum stdwrite_kind swk, int trashOutput, 
										t_fnc_stdOutWrite fnc_stdOutWrite,void* privatePtr);
UNITEX_FUNC int UNITEX_CALL GetStdWriteCB(enum stdwrite_kind swk, int* p_trashOutput, 
										t_fnc_stdOutWrite* p_fnc_stdOutWrite,void** p_privatePtr);


typedef size_t (ABSTRACT_CALLBACK_UNITEX *t_fnc_stdIn)(void *Buf, size_t size,void* privatePtr);
UNITEX_FUNC int UNITEX_CALL SetStdInCB(t_fnc_stdIn fnc_stdInRead,void* privatePtr);
UNITEX_FUNC int UNITEX_CALL GetStdInCB(t_fnc_stdIn* p_fnc_stdInRead,void** p_privatePtr);
#endif
