 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef String_hashH
#define String_hashH

#include "Unicode.h"
#include "Alphabet.h"


#define ENLARGE_IF_NEEDED 0
#define DONT_ENLARGE 1

#define NO_VALUE_INDEX -1
#define DONT_USE_VALUES -1

#define INSERT_IF_NEEDED 0
#define DONT_INSERT 1


/**
 * This is a list of transitions in the hash tree. It is tagged by a letter and it
 * points to a tree node. 'next' is the following transition in the list.
 */
struct string_hash_tree_transition {
   unichar letter;
   struct string_hash_tree_node* node;
   struct string_hash_tree_transition* next;
};


/**
 * This is a tree node with a control integer to know if it is a final one
 * and a list of transitions. If the node is a final one, 'value_index' 
 * contains an integer that is the index of the value in the 'value' array.
 * It contains NO_VALUE_INDEX if the node is not final.
 */
struct string_hash_tree_node {
   int value_index;
   struct string_hash_tree_transition* trans;
};


/**
 * This structure is used to manage unicode string pairs like (key,value). 
 * We use a tree in order to associate an integer to each key, and a string
 * array that contains the values. For instance, if we insert the pair ("abc","ABC"),
 * the tree may give us the number 37 for "abc", and we will have value[37]="ABC".
 * 'size' is the actual number of pairs in the structure. 'capacity' is the maximum
 * size of the 'value' array. 'bound_policy' is used to define what to do when 'value'
 * is full, raising an error or enlarge the array. 'root' is the root of the key tree.
 * 
 * Note that this structure is often used with key=value in order to have a bijection
 * between strings and integers:
 * - if we know the string, the key tree provides us the number
 * - if we know the number, value[number] provides us the string
 */
struct string_hash {
   int size;
   int capacity;
   int bound_policy;
   struct string_hash_tree_node* root;
   unichar** value;
};


struct string_hash* new_string_hash(int,int);
struct string_hash* new_string_hash(int);
struct string_hash* new_string_hash();
void free_string_hash(struct string_hash*);
int get_value_index(unichar*,struct string_hash*,int,unichar*);
int get_value_index(unichar*,struct string_hash*,int);
int get_value_index(unichar*,struct string_hash*);
struct string_hash* load_key_list(char*);
struct string_hash* load_key_value_list(char*,unichar);
void dump_values(FILE*,struct string_hash*);
int get_longest_key_index(unichar*,int*,struct string_hash*);

#endif
