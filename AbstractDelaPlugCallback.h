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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS) in the framework 
 * of UNITEX optimization and UNITEX industrialization / reliability
 *
 * More information : http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */



#ifndef _ABSTRACT_DELA_CALLBACK_INCLUDED
#define _ABSTRACT_DELA_CALLBACK_INCLUDED 1

#include "AbstractCallbackFuncModifier.h"

/* use this header to define an abstract space for dictionary
   Reading and understanding AbstractFilePlugCallback.h before
   this using this file is suggested

An abstract dictionary space is a manager for persistent dictionary. This is a way to load
   a dictionary and reuse it without loading from disk (or virtual file space) on
   each use. This can save significant time with large dictionaries.

You must define a function which determines if a filename is member of this space

You must also define a set of callbacks, used by Unitex when a dictionary is needed.

The privateSpacePtr is a private value pertaining to the space that can be
  filled with a user-provided value. this value is passed to all callbacks.
This can be useful in avoiding carrying around global variable

The dictionary space manager must use load_INF_file and free_INF_codes to load
  the INF_codes* object(s) it will maintain.
  BIN objects are just arrays of unsigned char* with the raw content of the file.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* this callback must return 1 is the filename is a member of the space
   if the callback return 0, this mean the dictionary is not member of the space
   Unitex will try other space (if any) or just load and free the dictionary
   using load_INF_file/load_BIN_file - free_INF_codes/free

   It is very highly suggested this function return the same value for the
   both .inf and .bin files belonging to the the same dictionary.

   Note that Unitex will always uses dictionaries in read only mode, so they
   can be used several time and shared between thread */
typedef int (ABSTRACT_CALLBACK_UNITEX *t_fnc_is_filename_DelaSpace_object)(const char*name,void* privateSpacePtr);

/* two optional (can be just NULL) callbacks to initialize and uninitialize the filespace */
typedef int (ABSTRACT_CALLBACK_UNITEX* t_fnc_Init_DelaSpace)(void* privateSpacePtr);
typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_Uninit_DelaSpace)(void* privateSpacePtr);

/* t_fnc_load_abstract_INF_file callback returns an INF_codes* structure from
     the dictionary space. It is called if the fnc_is_filename_DelaSpace_object function
       returned 1 with the filename
   if you want call a t_fnc_free_abstract_INF when the object is not used, you
     must set p_inf_free_info->must_be_free to 1. You can also set p_inf_free_info->private_ptr with
     a private value.
   the p_inf_free_info pointer is never NULL */

typedef struct INF_codes* (ABSTRACT_CALLBACK_UNITEX* t_fnc_load_abstract_INF_file)(const char* name,
                   struct INF_free_info* p_inf_free_info,void* privateSpacePtr);

/* t_fnc_free_abstract_INF can be NULL.
  if t_fnc_free_abstract_INF is not NULL and if p_inf_free_info->must_be_free was 1,
  this function is called. This can be used if you can increase/decrease usage count, for
  example */

typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_free_abstract_INF)(struct INF_codes* INF,
                   struct INF_free_info* p_inf_free_info,void* privateSpacePtr);

/* these functions use the same logic as the ones operating on INF files.
  The pointer returned is a readonly pointer to the raw file content.
  Tips : you can use it with file mapped io (CreateFileMapping on Win32, mmap on Posix)
   the p_bin_free_info pointer is never NULL */

typedef unsigned char* (ABSTRACT_CALLBACK_UNITEX* t_fnc_load_abstract_BIN_file)(const char* name,
                   struct BIN_free_info* p_bin_free_info,void* privateSpacePtr);

typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_free_abstract_BIN)(unsigned char* BIN,
                   struct BIN_free_info* p_bin_free_info,void* privateSpacePtr);



typedef struct
{
    t_fnc_is_filename_DelaSpace_object fnc_is_filename_object;

    t_fnc_Init_DelaSpace fnc_Init_DelaSpace;
    t_fnc_Uninit_DelaSpace fnc_Uninit_DelaSpace;

    t_fnc_load_abstract_INF_file fnc_load_abstract_INF_file;
    t_fnc_free_abstract_INF fnc_free_abstract_INF;
    t_fnc_load_abstract_BIN_file fnc_load_abstract_BIN_file;
    t_fnc_free_abstract_BIN fnc_free_abstract_BIN;

} t_persistent_dic_func_array;

UNITEX_FUNC int UNITEX_CALL AddAbstractDelaSpace(const t_persistent_dic_func_array* func_array,void* privateSpacePtr);
UNITEX_FUNC int UNITEX_CALL RemoveAbstractDelaSpace(const t_persistent_dic_func_array* func_array,void* privateSpacePtr);

/* just return the number of AbstractDelaSpace Installed */
UNITEX_FUNC int UNITEX_CALL GetNbAbstractDelaSpaceInstalled();


#ifdef __cplusplus
}
#endif

#endif
