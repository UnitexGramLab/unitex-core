/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
  
#ifndef AsciiSearchTreeH
#define AsciiSearchTreeH

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure is used to integer values associated to strings.
 * 
 * - 'c' is the character of the node
 * - 'is_final' is null if the node is not a final one and non null otherwise
 * - 'value' is the value associated to the string represented by this nod
 * - 'left' is the tree of all strings that start with a letter < 'c'
 * - 'middle' is the tree of all strings that start with 'c'
 * - 'right' is the tree of all strings that start with a letter > 'c'
 */
struct search_tree_node {
	char c;
	char is_final;
	int value;
	struct search_tree_node* left;
	struct search_tree_node* middle;
	struct search_tree_node* right;
};


int insert_string(struct search_tree_node**,const char*,int);
int get_string_number(const struct search_tree_node*,const char*,int*);
void free_search_tree_node(struct search_tree_node*);

} // namespace unitex

#endif


