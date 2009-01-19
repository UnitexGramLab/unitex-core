 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef List_intH
#define List_intH


/**
 * This is a simple structure for manipulating int lists.
 */
struct list_int {
   int n;
   struct list_int* next;
};


struct list_int* new_list_int(int,struct list_int*);
struct list_int* new_list_int(int);
void free_list_int(struct list_int*);
struct list_int* sorted_insert(int,struct list_int*);
int is_in_list(int,struct list_int*);
int equal_list_int(struct list_int*,struct list_int*);
struct list_int* head_insert(int,struct list_int*);
unsigned int hash_list_int(struct list_int*);
int length(struct list_int*);
struct list_int* destructive_sorted_merge(struct list_int*,struct list_int*);
struct list_int* sorted_merge(struct list_int*,struct list_int*);
void delete_head(struct list_int**);
void delete_tail(struct list_int**);
struct list_int* clone(struct list_int*);
int* dump(struct list_int*,int*);
int remove(int,struct list_int**);

#endif

