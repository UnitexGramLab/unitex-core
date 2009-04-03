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

#include "DELA_tree.h"
#include "Error.h"


void free_dela_entry_list(struct dela_entry_list*);


/**
 * Allocates, initializes and returns a new DELA tree.
 */
struct DELA_tree* new_DELA_tree() {
struct DELA_tree* tree;
tree=(struct DELA_tree*)malloc(sizeof(struct DELA_tree));
if (tree==NULL) {
   fatal_alloc_error("new_DELA_tree");
}
tree->inflected_forms=new_string_hash(DONT_USE_VALUES);
tree->size=0;
tree->capacity=256;
tree->dela_entries=(struct dela_entry_list**)malloc(tree->capacity*sizeof(struct dela_entry_list*));
if (tree->dela_entries==NULL) {
   fatal_alloc_error("new_DELA_tree");
}
return tree;
}


/**
 * Frees all the memory associated to the given DELA tree.
 */
void free_DELA_tree(struct DELA_tree* tree) {
if (tree==NULL) return;
free_string_hash(tree->inflected_forms);
for (int i=0;i<tree->size;i++) {
   free_dela_entry_list(tree->dela_entries[i]);
}
free(tree->dela_entries);
free(tree);
}


/**
 * Allocates, initializes and returns a new DELA entry. The 'clone' parameter
 * indicates whether the given entry must be cloned or not. If not, the function
 * only copies the pointed address.
 */
struct dela_entry_list* new_dela_entry_list(struct dela_entry* entry,int clone) {
struct dela_entry_list* l;
l=(struct dela_entry_list*)malloc(sizeof(struct dela_entry_list));
if (l==NULL) {
   fatal_alloc_error("new_dela_entry_list");
}
l->next=NULL;
if (clone) l->entry=clone_dela_entry(entry);
else l->entry=entry;
return l;
}


/**
 * Frees all the memory associated to the given DELA entry list.
 */
void free_dela_entry_list(struct dela_entry_list* l) {
struct dela_entry_list* tmp;
while (l!=NULL) {
   tmp=l;
   l=l->next;
   free_dela_entry(tmp->entry);
   free(tmp);
}
}


/**
 * Inserts the given entry in the given entry list, if not already present.
 * If the entry is already present, then it is freed.
 */
struct dela_entry_list* insert_if_not_present(struct dela_entry* entry,
                                              struct dela_entry_list* l) {
if (l==NULL) return new_dela_entry_list(entry,0);
if (equal(l->entry,entry)) {
   free_dela_entry(entry);
   return l;
}
l->next=insert_if_not_present(entry,l->next);
return l;
}


/**
 * Adds the given DELA entry to the given tree. If the entry is already
 * present in the tree, then it is freed. Otherwise, it is put in the tree
 * so that IT MUST NOT BE FREED!
 */
void add_entry(struct DELA_tree* tree,struct dela_entry* entry) {
int n=get_value_index(entry->inflected,tree->inflected_forms);
if (n==tree->size) {
   /* If there was no entry list for the given inflected form */
   if (n==tree->capacity) {
      /* If we must double the array capacity */
      tree->capacity=2*tree->capacity;
      tree->dela_entries=(struct dela_entry_list**)realloc(tree->dela_entries,tree->capacity*sizeof(struct dela_entry_list*));
      if (tree->dela_entries==NULL) {
         fatal_alloc_error("add_entry");
      }
   }
   tree->dela_entries[n]=NULL;
   (tree->size)++;
}
tree->dela_entries[n]=insert_if_not_present(entry,tree->dela_entries[n]);
}


/**
 * Loads the given DELA into the given DELA tree.
 */
void load_DELA(char* name,struct DELA_tree* tree) {
FILE* f=u_fopen(name,U_READ);
if (f==NULL) {
   error("Cannot load dictionary %s\n",name);
   return;
}
u_printf("Loading %s...\n",name);
unichar line[4096];
while (EOF!=u_fgets(line,f)) {
   struct dela_entry* entry=tokenize_DELAF_line(line,1);
   if (entry!=NULL) {
      add_entry(tree,entry);
   }
   /* We don't need to free the entry, since it's done (if needed)
    * in the insertion function */
}
u_fclose(f);
}
