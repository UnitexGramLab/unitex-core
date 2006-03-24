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
#ifndef String_listH
#define String_listH
//---------------------------------------------------------------------------

#include "unicode.h"

struct string_list {
   unichar* s;
   struct string_list* suivant;
};



struct string_list* new_string_list(unichar*);
void free_string_list(struct string_list*);
void free_string_list_element(struct string_list*);
struct string_list* insert_in_string_list(unichar*,struct string_list*);
struct string_list* insert_at_end_of_string_list(unichar*,struct string_list*);

#endif
