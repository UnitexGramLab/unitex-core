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



#ifdef __cplusplus
extern "C" {
#endif

typedef void* ABSTRACTFILE_PTR;

typedef enum
{
  OPEN_READ_MF, /* file will be open for read only */
  OPEN_READWRITE_MF, /* open an existing file to read and write into */
  OPEN_CREATE_MF /* create a new empty file */
}
TYPEOPEN_MF;



/* afs_size_type is the type of size of file
   must be 32 or 64 bits signed int. Can be 64 bits on a 32 bits system 
   we can using the same type than ftell return type, by example */
typedef size_t afs_size_type;


/*
An abstract file space is a manager of a set of file.
Each file, member of this file space, is full managed by 

To define an abstract file space, you must define a function which determine
  if a file is member of this filespace (by checking the name, we suggest you
  use first char of the filename to choose if a file is member of this filespace).

You must also define a set of basic file IO callback, which are just like a
  subset of C File stdio function.
The privateSpacePtr is a private value of filespace for callback you fill with 
  your own value, and which is given to all callback. This can help you not using
  global variable !
*/

/* this callback must return 1 is the filename is member of the filespace */
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_is_filename_vf_object)(const char*name,void* privateSpacePtr);

/* two optional (can be just NULL) callback to initialise and uninitialise the filespace */
typedef int (ABSTRACT_CALLBACK_UNITEX* t_fnc_Init_FileSpace)(void* privateSpacePtr);
typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_Uninit_FileSpace)(void* privateSpacePtr);



typedef ABSTRACTFILE_PTR (ABSTRACT_CALLBACK_UNITEX *t_fnc_memOpenLowLevel)
        (const char* FileName, TYPEOPEN_MF TypeOpen, void* privateSpacePtr);
/* these callback are like fWrite and fRead. return the number of bytes read or write */
typedef size_t (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelWrite)(ABSTRACTFILE_PTR llFile, void const *Buf, size_t size,void* privateSpacePtr);
typedef size_t (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelRead)(ABSTRACTFILE_PTR llFile, void *Buf, size_t size,void* privateSpacePtr);
/* TypeSeek is like the third parameter of fseek (origin) */
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelSeek)(ABSTRACTFILE_PTR llFile, afs_size_type Pos, int TypeSeek,void* privateSpacePtr);
/* return the size of an opened file on *pPos */
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelGetSize)(ABSTRACTFILE_PTR llFile, afs_size_type* pPos,void* privateSpacePtr);
/* return the current position on *pPos */
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelTell)(ABSTRACTFILE_PTR llFile, afs_size_type* pPos,void* privateSpacePtr);
/* close the file. Return 0 if success */
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelClose)(ABSTRACTFILE_PTR llFile,void* privateSpacePtr);
/* just after creating a new file, memLowLevelSetSizeReservation is an optional callback
  (can be NULL). Sometime, when a new file is created, this callback will be called with
  a projected size of the full file. Can be used to reserve space and, pehaps, prevent fragmentation 
  return 1 if reservation is done well, 0 elsewhere. */
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelSetSizeReservation)(ABSTRACTFILE_PTR llFile, afs_size_type size_reserv,void* privateSpacePtr);


/* like rename and remove of stdio.h, remove and rename file */
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

/* these funciton add and remove filespace.
  you can add several filespace with same func_array callback set, but with different privateSpacePtr
  privateSpacePtr is the parameters which be send as latest parameters of all callback */
UNITEX_FUNC int UNITEX_CALL AddAbstractFileSpace(const t_fileio_func_array* func_array,void* privateSpacePtr);
UNITEX_FUNC int UNITEX_CALL RemoveAbstractFileSpace(const t_fileio_func_array* func_array,void* privateSpacePtr);

/**********************************************************************/


/* There is a set of callback for taking stdin, stdout and stderr IO */
/* t_fnc_stdOutWrite (for stdout and stderr) and t_fnc_stdIn (for stdin) define
   the callback.
   the callback must return the number of char processed (so size if all is normal)
*/

enum stdwrite_kind { stdwrite_kind_out=0, stdwrite_kind_err } ;

typedef size_t (ABSTRACT_CALLBACK_UNITEX *t_fnc_stdOutWrite)(const void*Buf, size_t size,void* privatePtr);
/* SetStdWriteCB set the callback for one of the two (stdout or stderr) output
   if trashOutput == 1, fnc_stdOutWrite must be NULL and the output will just be ignored
   if trashOutput == 0 and fnc_stdOutWrite == NULL and the output will be standard output
   if trashOutput == 0 and fnc_stdOutWrite != NULL your callback will be used

   on the callback, size==0 only one : when a new call SetStdWriteCB change the parameter.
     This can allow you, by example closing a file or free some memory...

   privatePtr is a private value which will be just send as latest parameters of callback 
   GetStdWriteCB just set the current value on *p_trashOutput, *p_fnc_stdOutWrite and **p_privatePtr

   return value is 1 for success and 0 for error on SetStdWriteCB and GetStdWriteCB
   */

UNITEX_FUNC int UNITEX_CALL SetStdWriteCB(enum stdwrite_kind swk, int trashOutput, 
										t_fnc_stdOutWrite fnc_stdOutWrite,void* privatePtr);
UNITEX_FUNC int UNITEX_CALL GetStdWriteCB(enum stdwrite_kind swk, int* p_trashOutput, 
										t_fnc_stdOutWrite* p_fnc_stdOutWrite,void** p_privatePtr);


/* same logic of callback for stdin IO */
typedef size_t (ABSTRACT_CALLBACK_UNITEX *t_fnc_stdIn)(void *Buf, size_t size,void* privatePtr);
UNITEX_FUNC int UNITEX_CALL SetStdInCB(t_fnc_stdIn fnc_stdInRead,void* privatePtr);
UNITEX_FUNC int UNITEX_CALL GetStdInCB(t_fnc_stdIn* p_fnc_stdInRead,void** p_privatePtr);


#ifdef __cplusplus
}
#endif

#endif
