 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef _HASH_STR_TABLE_HH_
#define _HASH_STR_TABLE_HH_

#include <stdio.h>

#include "ustring.h"



struct tree_hash_t;

typedef struct hash_trans_t {

  unichar c;
  struct hash_tree_t  * to;
  struct hash_trans_t * next;

} hash_trans_t;


typedef struct hash_tree_t {

  int no; // -1 means not final
  hash_trans_t * trans;

} hash_tree_t;



typedef struct hash_str_table_t {
  hash_tree_t * root;
  int nbelems;
  void ** tab;
  int tabsize;
} hash_str_table_t;


typedef void (*release_f)(void * data);
typedef void (*dump_f)(void * data, FILE  * f);

hash_str_table_t * hash_str_table_new(int size = 16);
void hash_str_table_delete(hash_str_table_t * table, void (*release)(void * data) = NULL);

void hash_str_table_empty(hash_str_table_t * table, release_f release = NULL);

int hash_str_table_add(hash_str_table_t * table, unichar * key, void * value);

int hash_str_table_idx_lookup(hash_str_table_t * table, unichar * key);
void * hash_str_table_lookup(hash_str_table_t * table, unichar * key);



inline int hash_str_table_add(hash_str_table_t * table, Ustring * key, void * value) {
  return hash_str_table_add(table, key->str, value);
}

inline int hash_str_table_idx_lookup(hash_str_table_t * T, Ustring * k) { return hash_str_table_idx_lookup(T, k->str); }
inline void * hash_str_table_lookup(hash_str_table_t * table, Ustring * k) { return hash_str_table_lookup(table, k->str); }


#endif
