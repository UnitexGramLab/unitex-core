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
#include "String_hash.h"
#include "Error.h"
//---------------------------------------------------------------------------


#define DEFAULT_STRING_HASH_SIZE 4096


struct string_hash_tree_node* new_string_hash_tree_node();
void free_arbre_hash(struct string_hash_tree_node*);


/**
 * Allocates, initializes and returns a string_hash object, with
 * the given capacity and bound policy. If 'capacity' is set to
 * DONT_USE_VALUES, then the value array won't be created.
 */
struct string_hash* new_string_hash(int capacity,int bound_policy) {
struct string_hash* s;
s=(struct string_hash*)malloc(sizeof(struct string_hash));
if (s==NULL) {
   fatal_error("Not enough memory in new_string_hash\n");
}
s->size=0;
s->capacity=capacity;
s->bound_policy=bound_policy;
if (s->capacity==DONT_USE_VALUES) {
   /* If the user doesn't want to use the value array, we set it to NULL */
   s->value=NULL;
} else {
   s->value=(unichar**)malloc(capacity*sizeof(unichar*));
   if (s->value==NULL) {
      fatal_error("Not enough memory in new_string_hash\n");
   }
}
s->root=new_string_hash_tree_node();
return s;
}


/**
 * Returns a new string_hash object with the given capacity. Its
 * bound policy will be to enlarge the 'value' array if needed.
 */
struct string_hash* new_string_hash(int capacity) {
return new_string_hash(capacity,ENLARGE_IF_NEEDED);
}


/**
 * Returns a new string_hash object with the default capacity.
 * Its bound policy will be to enlarge the 'value' array if needed.
 */
struct string_hash* new_string_hash() {
return new_string_hash(DEFAULT_STRING_HASH_SIZE,ENLARGE_IF_NEEDED);
}


/**
 * Allocates, initializes and returns a new string_hash_tree_node.
 */
struct string_hash_tree_node* new_string_hash_tree_node() {
struct string_hash_tree_node* node;
node=(struct string_hash_tree_node*)malloc(sizeof(struct string_hash_tree_node));
if (node==NULL) {
   fatal_error("Not enough memory in new_string_hash_tree_node\n");
}
node->value_index=NO_VALUE_INDEX;
node->trans=NULL;
return node;
}


/**
 * Allocates, initializes and returns a new string_hash_tree_transition.
 */
struct string_hash_tree_transition* new_string_hash_tree_transition() {
struct string_hash_tree_transition* transition;
transition=(struct string_hash_tree_transition*)malloc(sizeof(struct string_hash_tree_transition));
if (transition==NULL) {
   fatal_error("Not enough memory in new_string_hash_tree_transition\n");
}
transition->letter='\0';
transition->node=NULL;
transition->next=NULL;
return transition;
}


/**
 * Frees a string_hash_tree transition list.
 */
void free_string_hash_tree_transition(struct string_hash_tree_transition* t) {
struct string_hash_tree_transition* tmp;
while (t!=NULL) {
   free_arbre_hash(t->node);
   tmp=t;
   t=t->next;
   free(tmp);
}
}


/**
 * Frees a string_hash_tree.
 */
void free_arbre_hash(struct string_hash_tree_node* node) {
if (node==NULL) return;
free_string_hash_tree_transition(node->trans);
free(node);
}


/**
 * Frees a string_hash object.
 */
void free_string_hash(struct string_hash* s) {
if (s==NULL) return;
free_arbre_hash(s->root);
if (s->value!=NULL) {
   /* One may have not used the value array */
   for (int i=0;i<s->size;i++) {
      if (s->value!=NULL) {
         /* We take this precaution because one might have
          * associated the NULL value to a key. */
         free(s->value[i]);
      }
   }
   free(s->value);
}
free(s);
}


/**
 * Looks in a transition list if there is one tagged by the given letter and
 * returns it, or NULL if there is not such transition.
 */
struct string_hash_tree_transition* get_transition(unichar letter,struct string_hash_tree_transition* list) {
while (list!=NULL) {
   if (list->letter==letter) return list;
   list=list->next;
}
return NULL;
}


/**
 * Returns the index value associated to the given key in the given string_hash.
 * 'pos' is the current position the key and 'node' is the current node in the 
 * string_hash_tree. If 'insert_if_needed' is non null, the key will be added
 * in the string_hash if not allready present. Otherwise, the function will
 * return NO_VALUE_INDEX if the key is not in the string_hash.
 */
int get_value_index_(unichar* key,int pos,struct string_hash_tree_node* node,
                    struct string_hash* hash,int insert_policy,unichar* value) {
if (node==NULL) {
   fatal_error("NULL error in get_value_index\n");
}
if (key[pos]=='\0') {
   /* If we are at the end of the key */
   if (insert_policy==DONT_INSERT) {
      /* If we just consult the string_hash with no insert, we just
       * have to return the value_index of the node */
      return node->value_index;
   }
   if (node->value_index!=NO_VALUE_INDEX) {
      /* If the key allready exists, we return its value index */
      return node->value_index;
   }
   /* Here, we have to build a new value index */
   if (hash->capacity==DONT_USE_VALUES) {
      /* If don't uses the 'value' array, there is no limitation */
      node->value_index=hash->size;
      (hash->size)++;
   }
   /* Otherwise: if there is a maximum capacity */
   if (hash->size==hash->capacity) {
      /* We check if we have reached the end of the 'value' array */
      if (hash->bound_policy==DONT_ENLARGE) {
         /* If we can't enlarge the 'value' array, we fail */
         fatal_error("Too much elements in a non extensible array in get_value_index\n");
      }
      /* If we can enlarge the 'value' array, we do it, doubling its capacity */
      hash->capacity=2*hash->capacity;
      hash->value=(unichar**)realloc(hash->value,sizeof(unichar*)*hash->capacity);
      if (hash->value==NULL) {
         fatal_error("Not enough memory in get_value_index\n");
      }
   }
   node->value_index=hash->size;
   (hash->size)++;
   /* u_strdup is supposed to return NULL if 'value' is NULL */
   hash->value[node->value_index]=u_strdup(value);
   return node->value_index;
}
/* If we are not at the end of the key, we look for the transition to follow */
struct string_hash_tree_transition* t=get_transition(key[pos],node->trans);
if (t==NULL) {
   /* If there is no suitable transition */
   if (insert_policy==DONT_INSERT) {
      /* If we just look, then we say that we have not found the key */
      return NO_VALUE_INDEX;
   }
   /* Otherwise, we create a transition */
   t=new_string_hash_tree_transition();
   t->letter=key[pos];
   t->next=node->trans;
   t->node=new_string_hash_tree_node();
   node->trans=t;
}
return get_value_index_(key,pos+1,t->node,hash,insert_policy,value);
}


/**
 * Returns the index value associated to the given key. If the given string_hash
 * tolerates values, 'value' will be associated to the given key if the key is
 * not allready present in the string_hash.
 */
int get_value_index(unichar* key,struct string_hash* hash,int insert_policy,unichar* value) {
return get_value_index_(key,0,hash->root,hash,insert_policy,value);
}


/**
 * Returns the index value associated to the given key. If the given string_hash
 * tolerates values, the key will be used as value.
 */
int get_value_index(unichar* key,struct string_hash* hash,int insert_policy) {
return get_value_index_(key,0,hash->root,hash,insert_policy,key);
}


/**
 * Returns the index value associated to the given key, inserting it if needed.
 * In that case, the key itself will be used as value.
 */
int get_value_index(unichar* key,struct string_hash* hash) {
return get_value_index_(key,0,hash->root,hash,INSERT_IF_NEED,key);
}


/**
 * Loads the lines of a text file info a string_hash and returns it, or NULL
 * if the file can not be opened. We arbitrary fix the limit of a line to 4096
 * characters.
 */
struct string_hash* load_string_list(char* name) {
FILE* f=u_fopen(name,U_READ);
if (f==NULL) return NULL;
struct string_hash* hash=new_string_hash();
unichar temp[4096];
while (u_read_line(f,temp)) {
   get_value_index(temp,hash);
}
u_fclose(f);
return hash;
}


/**
 * Dumps the values of the given string_hash into the given file (one
 * string per line). Raises a fatal error if the strign_hash has no value.
 */
void dump_values(FILE *f,struct string_hash* hash) {
if (hash==NULL) {
   fatal_error("NULL error in dump_values\n");
}
if (hash->value==NULL) {
   fatal_error("No values to dump in dump_values\n");
}
for (int i=0;i<hash->size;i++) {
   u_fprints(hash->value[i],f);
   u_fprints_char("\n",f);
}
}

