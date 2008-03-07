 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "utils.h"
#include "stack.h"
#include "hash_str_table.h"


#warning à unifier avec String_hash ou HashTable


static hash_tree_t * hash_tree_new();


static hash_trans_t * hash_trans_new(unichar c, hash_trans_t * next) {
  hash_trans_t * res = (hash_trans_t *) xmalloc(sizeof(hash_trans_t));
  res->c  = c;
  res->to = hash_tree_new();
  res->next = next;
  return res;
}


static hash_tree_t * hash_tree_new() {
  hash_tree_t * res = (hash_tree_t *) xmalloc(sizeof(hash_tree_t));
  res->no = -1;
  res->trans = NULL;
  return res;
}

static void hash_tree_delete(hash_tree_t * tree) {

  stack_type * stack = stack_new();
  stack_push(stack, tree);

  while (! stack_empty(stack)) {
	 
    tree = (hash_tree_t *) stack_pop(stack);

    hash_trans_t * t = tree->trans;

    while (t) {
      hash_trans_t * next = t->next;
      stack_push(stack, t->to);
      free(t);
      t = next;
    }
    free(tree);
  }
  stack_delete(stack);
}



static inline hash_trans_t * get_trans(hash_trans_t * trans, unichar c) {
  while (trans && (trans->c != c)) { trans = trans->next; }
  return trans;
}


static hash_tree_t * hash_tree_add(hash_tree_t * tree, unichar * s) {

  hash_trans_t * t;

  while (*s) {

    t = get_trans(tree->trans, *s);

    if (t == NULL) { tree->trans = t = hash_trans_new(*s, tree->trans); }

    tree = t->to;
    s++;
  }

  return tree;
}


static int hash_tree_lookup(hash_tree_t * tree, unichar * s) {

  hash_trans_t * t;

  while (*s) {

    t = get_trans(tree->trans, *s);

    if (t == NULL) { return -1; }

    tree = t->to;
    s++;
  }

  return tree->no;
}



hash_str_table_t * hash_str_table_new(int size) {

  hash_str_table_t * res = (hash_str_table_t *) xmalloc(sizeof(hash_str_table_t));

  res->nbelems = 0;
  res->root    = hash_tree_new();

  res->tab     = (void **) xmalloc(size * sizeof(void *));
  res->tabsize = size;

  return res;
}


typedef void (*release_f)(void * data);

void hash_str_table_delete(hash_str_table_t * table, void (*release)(void * data)) {

  hash_tree_delete(table->root);

  if (release) {
    for (int i = 0; i < table->nbelems; i++) { release(table->tab[i]); }
  }

  free(table->tab);
  free(table);
}


void hash_str_table_empty(hash_str_table_t * table, void (*release)(void * data)) {

  if (release) {
    for (int i = 0; i < table->nbelems; i++) { release(table->tab[i]); }
  }

  hash_tree_delete(table->root);
  table->root = hash_tree_new();

  table->nbelems = 0;
}



int hash_str_table_add(hash_str_table_t * table, unichar * key, void * value) {

  hash_tree_t * leaf = hash_tree_add(table->root, key);

  if (leaf->no == -1) { leaf->no = table->nbelems; }

  int idx = leaf->no;

  while (idx >= table->tabsize) {
    table->tabsize = 2 * table->tabsize;
    table->tab = (void **) xrealloc(table->tab, table->tabsize * sizeof(void *));
  }

  table->tab[idx] = value;
  table->nbelems++;

  return idx;
}


int hash_str_table_add_if_not_present(hash_str_table_t * table, unichar * key, void * value) {

  hash_tree_t * leaf = hash_tree_add(table->root, key);

  if (leaf->no != -1) { return leaf->no; }

  int idx = leaf->no = table->nbelems;

  while (idx >= table->tabsize) {
    table->tabsize = 2 * table->tabsize;
    table->tab = (void **) xrealloc(table->tab, table->tabsize * sizeof(void *));
  }

  table->tab[idx] = value;
  table->nbelems++;

  return idx;
}



int hash_str_table_idx_lookup(hash_str_table_t * table, unichar * key) {
  return hash_tree_lookup(table->root, key);
}


void * hash_str_table_lookup(hash_str_table_t * table, unichar * key) {

  int idx = hash_tree_lookup(table->root, key);

  if (idx == -1) { return NULL; }

  return table->tab[idx];
}


#define MAXBUF 1024



/*void hash_str_table_dump(hash_str_table_t * table, void (*dump)(void * data, FILE * f), FILE * f) {

  unichar buf[MAXBUF];

  fprintf(f, "table: nbelems=%d\n", table->nbelems);

  _table_dump(table, table->root, buf, 0, dump, f);
}*/
