/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdlib.h>
#include <string.h>

#include "CompressedDic.h"
#include "Fst2.h"
#include "AbstractDelaLoad.h"
#include "AbstractFst2Load.h"
#include "Alphabet.h"

#include "PersistenceInterface.h"
#include "Error.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#ifndef UNITEX_PREVENT_EXPOSE_MINI_PERSISTANCE_IN_INTERFACE

int standard_load_persistence_dictionary(const char*filename,char* persistent_filename_buffer,size_t buffer_size)
{
  if ((persistent_filename_buffer == NULL) || (buffer_size <= strlen(filename)))
    return 0;
  strcpy(persistent_filename_buffer,filename);
  return load_persistent_dictionary(filename);
}

void standard_unload_persistence_dictionary(const char*filename)
{
  free_persistent_dictionary(filename);
}

int standard_load_persistence_fst2(const char*filename,char* persistent_filename_buffer,size_t buffer_size)
{
  if ((persistent_filename_buffer == NULL) || (buffer_size <= strlen(filename)))
    return 0;
  strcpy(persistent_filename_buffer,filename);
  return load_persistent_fst2(filename);
}

void standard_unload_persistence_fst2(const char*filename)
{
  free_persistent_fst2(filename);
}

int standard_load_persistence_alphabet(const char*filename,char* persistent_filename_buffer,size_t buffer_size)
{
  if ((persistent_filename_buffer == NULL) || (buffer_size <= strlen(filename)))
    return 0;
  strcpy(persistent_filename_buffer,filename);
  return load_persistent_alphabet(filename);
}

void standard_unload_persistence_alphabet(const char*filename)
{
  free_persistent_alphabet(filename);
}

#endif

UNITEX_FUNC int UNITEX_CALL persistence_public_load_dictionary(const char*filename,char* persistent_filename_buffer,size_t buffer_size)
{
  return standard_load_persistence_dictionary(filename, persistent_filename_buffer, buffer_size);
}

UNITEX_FUNC void UNITEX_CALL persistence_public_unload_dictionary(const char*filename)
{
  return standard_unload_persistence_dictionary(filename);
}

UNITEX_FUNC int UNITEX_CALL persistence_public_load_fst2(const char*filename,char* persistent_filename_buffer,size_t buffer_size)
{
  return standard_load_persistence_fst2(filename, persistent_filename_buffer, buffer_size);
}

UNITEX_FUNC void UNITEX_CALL persistence_public_unload_fst2(const char*filename)
{
  return standard_unload_persistence_fst2(filename);
}

UNITEX_FUNC int UNITEX_CALL persistence_public_load_alphabet(const char*filename,char* persistent_filename_buffer,size_t buffer_size)
{
  return standard_load_persistence_alphabet(filename, persistent_filename_buffer, buffer_size);
}

UNITEX_FUNC void UNITEX_CALL persistence_public_unload_alphabet(const char*filename)
{
  return standard_unload_persistence_alphabet(filename);
}

UNITEX_FUNC int UNITEX_CALL persistence_public_is_persisted_fst2_filename(const char*filename)
{
  return is_abstract_or_persistent_fst2_filename(filename);
}

UNITEX_FUNC int UNITEX_CALL persistence_public_is_persisted_dictionary_filename(const char*filename)
{
  return is_abstract_or_persistent_dictionary_filename(filename);
}

UNITEX_FUNC int UNITEX_CALL persistence_public_is_persisted_alphabet_filename(const char*filename)
{
  return is_abstract_or_persistent_alphabet_filename(filename);
}

} // namespace unitex
