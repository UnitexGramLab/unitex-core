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

#ifndef Load_DLF_DLCH
#define Load_DLF_DLCH


#include "Unicode.h"
#include "DELA.h"
#include "String_hash.h"


/* This structure represents a list of DELA entries */
struct dela_entry_list {
   struct dela_entry* entry;
   struct dela_entry_list* next;
};


/**
 * This structure is used to associate lists of DELA entries to inflected forms.
 * Each inflected form is associated to an integer i so that
 * 'dela_entries[i]' is the list of DELA entries associated to this form.
 * 'capacity' is the maximum size of this list array and 'size' is its actual
 * number of elements.
 */
struct DELA_tree {
   struct string_hash* inflected_forms;
   struct dela_entry_list** dela_entries;
   int size;
   int capacity;
};


struct DELA_tree* new_DELA_tree();
void free_DELA_tree(struct DELA_tree*);
void load_DELA(const VersatileEncodingConfig*,const char*,struct DELA_tree*);

#endif

