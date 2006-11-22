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
#ifndef List_ustringH
#define List_ustringH
//---------------------------------------------------------------------------

#include "unicode.h"

/**
 * This is a simple structure for manipulating unicode string lists.
 */
struct list_ustring {
   unichar* string;
   struct list_ustring* next;
};


struct list_ustring* new_list_ustring(unichar*,struct list_ustring*);
struct list_ustring* new_list_ustring(unichar*);
void free_list_ustring(struct list_ustring*);
void free_list_ustring_element(struct list_ustring*);
struct list_ustring* sorted_insert(unichar*,struct list_ustring*);
struct list_ustring* head_insert(unichar*,struct list_ustring*);
struct list_ustring* insert_at_end_of_list(unichar*,struct list_ustring*);
int is_in_list(unichar*,struct list_ustring*);
int equal(struct list_ustring*,struct list_ustring*);
struct list_ustring* clone(struct list_ustring*);

#endif

