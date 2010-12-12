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

#include <stdio.h>
#include <stdlib.h>
#include "AsciiSearchTree.h"
#include "Error.h"



/**
 * Allocates, initializes and returns a search tree node.
 */
struct search_tree_node* new_search_tree() {
struct search_tree_node* tree=(struct search_tree_node*)malloc(sizeof(struct search_tree_node));
if (tree==NULL) {
   fatal_alloc_error("new_search_tree");
}
tree->is_final=0;
/* There is no need to give a default value since because the value field
 * is not supposed to be read if the node is not final */
tree->left=NULL;
tree->middle=NULL;
tree->right=NULL;
return tree;
}


/**
 * This function inserts a string in the tree whose root is 'node'. 'position' is the current
 * position in the string. If the string is already associated to a value, 0 is returned;
 * 1 otherwise.
 * 
 * IMPORTANT: this function assumes that the given string is neither NULL
 *            nor empty
 */
int insert_string(struct search_tree_node* *node,const char* string,int position,
                                  int value) {
if ((*node)==NULL) {
	/* If we need to allocate the tree node */
	(*node)=new_search_tree();
	(*node)->c=string[position];
}
if (string[position+1]=='\0' && string[position]==(*node)->c) {
	/* If we are at the end of the string on the correct node */
	if ((*node)->is_final) {
		/* If the string is already associated to a value, 
		 * then we raise a fatal error. */
		 return 0;
	}
	(*node)->is_final=1;
	(*node)->value=value;
	return 1;
}
/* If we are not at the end of the string */
if (string[position]==(*node)->c) {
	/* If the current node is tagged by the current letter, then
	 * we increase the position in the string and we explore the
	 * 'middle' tree. */
	 return insert_string(&((*node)->middle),string,position+1,value);
}
if (string[position]<(*node)->c) {
	/* If the current node is tagged by a letter that is greater than
	 * the current letter, we look for the good node in the 'left' tree, but
	 * we do not modify the position in the string. */
	 return insert_string(&((*node)->left),string,position,value);
}
/* Finally, if the current node is tagged by a that is lower than
 * the current letter, we look for the good node in the 'right' tree, but
 * we do not modify the position in the string. */
return insert_string(&((*node)->right),string,position,value);
}


/**
 * This function inserts a string in the given tree and returns 1, or 0 if
 * the string is already associated to a value. NULL or empty string
 * will raise a fatal error.
 */
int insert_string(struct search_tree_node* *root,const char* string,int value) {
if (string==NULL || string[0]=='\0') {
	fatal_error("NULL or empty string in insert_string\n");
}
return insert_string(root,string,0,value);
}


/**
 * This function looks for the given string in the tree whose root
 * is 'node'. 'position' is the current position in the string. If the
 * string is not in the tree or if it has no value, 0 is returned;
 * otherwise, the function returns 1 and the value associated to the
 * string is saved in the 'result' parameter.
 */
int get_string_number(const struct search_tree_node* node,const char* string,int position,int *result) {
if (node==NULL) {
	/* If we find a NULL node, it means that the tree does not
	 * contain the string, so we return 0. */
	return 0;	
}
if (string[position]==node->c && string[position+1]=='\0') {
	/* If we are on the node with the correct letter, and if we are
	 * at the end of the string, and if the node is final, then we
	 * store the associated value and we return 1; otherwise we return 0. */
	if (!node->is_final) return 0;
	(*result)=node->value;
	return 1;
}
if (string[position]==node->c) {
	/* If have the correct letter but we are not at the end of the string,
	 * then we explore the next letter in the 'middle' tree. */
	return get_string_number(node->middle,string,position+1,result);
}
if (string[position]<node->c) {
	/* If the current node is tagged by a letter that is greater than
	 * the current letter, we look for the good node in the 'left' tree, but
	 * we do not modify the position in the string. */
	return get_string_number(node->left,string,position,result);
}
/* Finally, if the current node is tagged by a that is greater than
 * the current letter, we look for the good node in the 'right' tree, but
 * we do not modify the position in the string. */
return get_string_number(node->right,string,position,result);
}


/**
 * Returns 1 if there is a value associated to the given string in the given tree.
 * In that case, this value is saved in 'result'. 0 is returned if the string is
 * not in the tree or has no value. A fatal error will be raised if the string
 * is NULL or empty.
 */
int get_string_number(const struct search_tree_node* root,const char* string,int *result) {
if (string==NULL || string[0]=='\0') {
	fatal_error("NULL or empty string in get_string_number\n");
}
return get_string_number(root,string,0,result);
}


/*
 * free memory of a search tree node
 */
void free_search_tree_node(struct search_tree_node* root)
{
	if (root != NULL)
	{
		free_search_tree_node(root->left);
		free_search_tree_node(root->middle);
		free_search_tree_node(root->right);
		free(root);
	}
}
