/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef PersistenceH
#define PersistenceH

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This library is designed to maintain persistent pointers associated
 * to file names. This may be useful when Unitex is used as a library
 * for repetitive batch processing.
 *
 * NOTE: persistence features are not activated if Unitex is not built
 *       as a library (either normal or JNI)
 */

void* get_persistent_structure(const char* filename);
void set_persistent_structure(const char* filename,void* ptr);
int is_persistent_structure(void* ptr);

} // namespace unitex

#endif
