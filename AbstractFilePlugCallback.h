/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * additional information: http://www.smartversion.com/unitex-contribution/
 * contact : info@winimage.com
 *
 */



#ifndef _ABSTRACT_FILE_CALLBACK_INCLUDED
#define _ABSTRACT_FILE_CALLBACK_INCLUDED 1

#include "AbstractCallbackFuncModifier.h"

#define HAS_FILE_FUNC_ARRAY_EXTENSIBLE 1
#define HAS_FILE_FUNC_ARRAY_GETLIST 1

#ifdef __cplusplus
extern "C" {
#endif

typedef void* ABSTRACTFILE_PTR;

typedef long vfs_size_operation_type ;

typedef enum
{
  OPEN_READ_MF,             /* open an existing file in read only mode  */
  OPEN_READWRITE_MF,    /* open an existing file in read/write mode */
  OPEN_CREATE_MF            /* create a new empty file */
}
TYPEOPEN_MF;



/* afs_size_type is the type used to describe file sizes.
   it must be either a 32 or 64 bits signed int. can be 64 bits
   on a 32 bits system for instance, be the same type as the one
   returned by ftell
*/
typedef size_t afs_size_type;


/*
An abstract file space is a manager for a set of files.

To define an abstract file space, you must define a function which determines
  if a file is member of this filespace (by checking its name, we suggest you
  use a marker as the first char of the filename to choose if a file is member
  of this filespace). e.g. '*'

You must also define a set of basic file IO callbacks, which are similar to a
  subset of the C file stdio functions.
The privateSpacePtr is a private value pertaining to the filespace that can be
  filled with a user-provided value. this value is passed to all callbacks.
This can be useful in avoiding carrying around global variables
*/

/* this callback must return 1 is the filename is a member of the filespace */
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_is_filename_vf_object)(const char*name,void* privateSpacePtr);

/* two optional (can be just NULL) callbacks to initialize and uninitialize the filespace */
typedef int (ABSTRACT_CALLBACK_UNITEX* t_fnc_Init_FileSpace)(void* privateSpacePtr);
typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_Uninit_FileSpace)(void* privateSpacePtr);


typedef ABSTRACTFILE_PTR (ABSTRACT_CALLBACK_UNITEX *t_fnc_memOpenLowLevel)
        (const char* FileName, TYPEOPEN_MF TypeOpen, void* privateSpacePtr);

/* these callback are similar to fWrite and fRead and return the number of bytes read or written */
typedef size_t (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelWrite)(ABSTRACTFILE_PTR llFile, void const *Buf, size_t size,void* privateSpacePtr);
typedef size_t (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelRead)(ABSTRACTFILE_PTR llFile, void *Buf, size_t size,void* privateSpacePtr);

/* TypeSeek is similar to the third parameter of fseek (origin) */
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelSeek)(ABSTRACTFILE_PTR llFile, afs_size_type Pos, int TypeSeek,void* privateSpacePtr);
/* returns the size of an opened file in *pPos */
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelGetSize)(ABSTRACTFILE_PTR llFile, afs_size_type* pPos,void* privateSpacePtr);
/* returns the current position on *pPos */
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelTell)(ABSTRACTFILE_PTR llFile, afs_size_type* pPos,void* privateSpacePtr);
/* closes the file and returns 0 if successful */
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelClose)(ABSTRACTFILE_PTR llFile,void* privateSpacePtr);
/* just after creating a new file, memLowLevelSetSizeReservation is an optional callback
  (can be NULL). In some instances, when a new file is created, this callback will be called with
  a projected size for the full file. It can be used to reserve space and, perhaps prevent fragmentation
  returns 1 if reservation is done successfully and 0 otherwise. */
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memLowLevelSetSizeReservation)(ABSTRACTFILE_PTR llFile, afs_size_type size_reserv,void* privateSpacePtr);


/* similar to rename and remove in stdio.h, these two callbacks remove and rename a file */
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memFileRemove)(const char* lpFileName,void* privateSpacePtr);
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_memFileRename)(const char * _OldFilename, const char * _NewFilename,void* privateSpacePtr);

/* get a read only pointer on a portion of a file, from pos to pos+len (file mapped io logic). Can
        be NULL if the file space don't support this function (but Unitex will use more CPU time and memory */
typedef const void* (ABSTRACT_CALLBACK_UNITEX *t_fnc_memFile_getMapPointer)(ABSTRACTFILE_PTR llFile, afs_size_type pos, afs_size_type len,int options,afs_size_type value_for_options,void* privateSpacePtr);

/* release the pointer returned by t_fnc_memFile_getMapPointer, if needed */
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_memFile_releaseMapPointer)(ABSTRACTFILE_PTR llFile, const void* ,afs_size_type len, void* privateSpacePtr);


typedef char** (ABSTRACT_CALLBACK_UNITEX *t_fnc_memFile_getList)(void* privateSpacePtr);
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_memFile_releaseList)(char** listFile, void* privateSpacePtr);


typedef struct
{
    size_t size_func_array;
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

    t_fnc_memFile_getMapPointer fnc_memFile_getMapPointer;
    t_fnc_memFile_releaseMapPointer fnc_memFile_releaseMapPointer;

    t_fnc_memFile_getList fnc_memFile_getList;
    t_fnc_memFile_releaseList fnc_memFile_releaseList;
} t_fileio_func_array_extensible;
/* these functions respectively add and remove filespaces.
  you can add several filespaces with the same func_array callback set, but with different privateSpacePtr
  privateSpacePtr is the parameters which can be set as the last parameter of each callback */
UNITEX_FUNC int UNITEX_CALL AddAbstractFileSpaceExtensible(const t_fileio_func_array_extensible* func_array_ex,void* privateSpacePtr);

UNITEX_FUNC int UNITEX_CALL RemoveAbstractFileSpaceExtensible(const t_fileio_func_array_extensible* func_array_ex,void* privateSpacePtr);

/* just return the number of AbstractFileSpace Installed */
UNITEX_FUNC int UNITEX_CALL GetNbAbstractFileSpaceInstalled();

/**********************************************************************/


/* There is a set of callbacks for rerouting stdin, stdout and stderr IO */
/* t_fnc_stdOutWrite (for stdout and stderr) and t_fnc_stdIn (for stdin) define
   the callback.
   the callback must return the number of char processed (the actual size if operating normally)
*/

enum stdwrite_kind { stdwrite_kind_out=0, stdwrite_kind_err } ;

typedef size_t (ABSTRACT_CALLBACK_UNITEX *t_fnc_stdOutWrite)(const void*Buf, size_t size,void* privatePtr);
/* SetStdWriteCB sets the callback for one of the two (stdout or stderr) output streams
   if trashOutput == 1, fnc_stdOutWrite must be NULL and the output will just be ignored
   if trashOutput == 0 and fnc_stdOutWrite == NULL and the output will be the standard output
   if trashOutput == 0 and fnc_stdOutWrite != NULL the callback will be used

   the callback is called with a zero size in only one case: when SetStdWriteCB is called, the preceding
   callback is called a last time (with its associated privatePtr) with a zero size.
   This may allow you, for instance, to close a file or free some memory...

   privatePtr is a private value which is passed as the last parameters of a callback
   GetStdWriteCB sets the current value on *p_trashOutput, *p_fnc_stdOutWrite and **p_privatePtr

   returns 1 if successful and 0 if an error occurred on SetStdWriteCB or GetStdWriteCB
   */

UNITEX_FUNC int UNITEX_CALL SetStdWriteCB(enum stdwrite_kind swk, int trashOutput,
                                        t_fnc_stdOutWrite fnc_stdOutWrite,void* privatePtr);
UNITEX_FUNC int UNITEX_CALL GetStdWriteCB(enum stdwrite_kind swk, int* p_trashOutput,
                                        t_fnc_stdOutWrite* p_fnc_stdOutWrite,void** p_privatePtr);


/* similar logic as the one above for stdin IO */
typedef size_t (ABSTRACT_CALLBACK_UNITEX *t_fnc_stdIn)(void *Buf, size_t size,void* privatePtr);
UNITEX_FUNC int UNITEX_CALL SetStdInCB(t_fnc_stdIn fnc_stdInRead,void* privatePtr);
UNITEX_FUNC int UNITEX_CALL GetStdInCB(t_fnc_stdIn* p_fnc_stdInRead,void** p_privatePtr);



#ifdef __cplusplus
}
#endif

#endif


