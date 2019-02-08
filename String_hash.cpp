/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "String_hash.h"
#include "Error.h"
#include "StringParsing.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define DEFAULT_STRING_HASH_SIZE 4096


struct string_hash_tree_node* new_string_hash_tree_node(struct string_hash*);
void free_arbre_hash(struct string_hash_tree_node*,int,int,struct string_hash*);


/**
 * Allocates, initializes and returns a string_hash object, with
 * the given capacity and bound policy. If 'capacity' is set to
 * DONT_USE_VALUES, then the value array won't be created.
 */
struct string_hash* new_string_hash(int capacity,int bound_policy) {
struct string_hash* s;
s=(struct string_hash*)malloc(sizeof(struct string_hash));
if (s==NULL) {
   fatal_alloc_error("new_string_hash");
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
      fatal_alloc_error("new_string_hash");
   }
}
s->allocator_tree_node=NULL;
s->allocator_tree_transition=NULL;

#define VA
#ifdef VA
s->allocator_tree_node=create_abstract_allocator("new_string_hash.tree_node",
                                                 AllocatorCreationFlagAutoFreePrefered | AllocatorFreeOnlyAtAllocatorDelete,
                                                 sizeof(struct string_hash_tree_node),NULL);
s->allocator_tree_transition=create_abstract_allocator("new_string_hash.tree_transition",
                                                       AllocatorCreationFlagAutoFreePrefered | AllocatorFreeOnlyAtAllocatorDelete,
                                                       sizeof(struct string_hash_tree_transition),NULL);
#endif
s->root=new_string_hash_tree_node(s);
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
struct string_hash_tree_node* new_string_hash_tree_node(struct string_hash* s) {
struct string_hash_tree_node* node;
node=(struct string_hash_tree_node*)malloc_cb(sizeof(struct string_hash_tree_node),s->allocator_tree_node);
if (node==NULL) {
   fatal_alloc_error("new_string_hash_tree_node");
}
node->value_index=NO_VALUE_INDEX;
node->trans=NULL;
return node;
}


/**
 * Allocates, initializes and returns a new string_hash_tree_transition.
 */
struct string_hash_tree_transition* new_string_hash_tree_transition(struct string_hash* s) {
struct string_hash_tree_transition* transition;
transition=(struct string_hash_tree_transition*)malloc_cb(sizeof(struct string_hash_tree_transition),s->allocator_tree_transition);
if (transition==NULL) {
   fatal_alloc_error("new_string_hash_tree_transition");
}
transition->letter='\0';
transition->node=NULL;
transition->next=NULL;
return transition;
}


/**
 * Frees a string_hash_tree transition list.
 */
void free_string_hash_tree_transition(struct string_hash_tree_transition* t,int free_tree_node_struct,int free_tree_transition_struct,struct string_hash* s) {
struct string_hash_tree_transition* tmp;
while (t!=NULL) {
   free_arbre_hash(t->node,free_tree_node_struct,free_tree_transition_struct,s);
   tmp=t;
   t=t->next;
   if (free_tree_transition_struct) {
     free_cb(tmp,s->allocator_tree_transition);
   }
}
}


/**
 * Frees a string_hash_tree.
 */
void free_arbre_hash(struct string_hash_tree_node* node,int free_tree_node_struct,int free_tree_transition_struct,struct string_hash* s) {
if (node==NULL) return;
free_string_hash_tree_transition(node->trans,free_tree_node_struct,free_tree_transition_struct,s);
if (free_tree_node_struct) {
  free_cb(node,s->allocator_tree_node);
}
}


/**
 * Frees a string_hash object.
 */
void free_string_hash(struct string_hash* s) {
if (s==NULL) return;
int free_tree_node_struct=(get_allocator_cb_flag(s->allocator_tree_node) & AllocatorGetFlagAutoFreePresent) ? 0 : 1;
int free_tree_transition_struct=(get_allocator_cb_flag(s->allocator_tree_transition) & AllocatorGetFlagAutoFreePresent) ? 0 : 1;
if (free_tree_node_struct || free_tree_transition_struct) {
    free_arbre_hash(s->root,free_tree_node_struct,free_tree_transition_struct,s);
}

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
close_abstract_allocator(s->allocator_tree_node);
close_abstract_allocator(s->allocator_tree_transition);
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
 * in the string_hash if not already present. Otherwise, the function will
 * return NO_VALUE_INDEX if the key is not in the string_hash.
 */
int get_value_index_(const unichar* key,int pos,struct string_hash_tree_node* node,
                    struct string_hash* hash,int insert_policy,const unichar* value) {

for (;;) {
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
          /* If the key already exists, we return its value index */
          return node->value_index;
       }
       /* Here, we have to build a new value index */
       if (hash->capacity==DONT_USE_VALUES) {
          /* If don't uses the 'value' array, there is no limitation */
          node->value_index=hash->size;
          (hash->size)++;
       } else {
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
             fatal_alloc_error("get_value_index");
           }
         }
         node->value_index=hash->size;
         (hash->size)++;
         /* u_strdup is supposed to return NULL if 'value' is NULL */
         hash->value[node->value_index]=u_strdup(value);
       }
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
       t=new_string_hash_tree_transition(hash);
       t->letter=key[pos];
       t->next=node->trans;
       t->node=new_string_hash_tree_node(hash);
       node->trans=t;
    }

    pos++;
    node=t->node;
}
}


/**
 * Returns the index value associated to the given key. If the given string_hash
 * tolerates values, 'value' will be associated to the given key if the key is
 * not already present in the string_hash.
 */
int get_value_index(const unichar* key,struct string_hash* hash,int insert_policy,const unichar* value) {
return get_value_index_(key,0,hash->root,hash,insert_policy,value);
}


/**
 * Returns the index value associated to the given key. If the given string_hash
 * tolerates values, the key will be used as value.
 */
int get_value_index(const unichar* key,struct string_hash* hash,int insert_policy) {
return get_value_index_(key,0,hash->root,hash,insert_policy,key);
}


/**
 * Returns the index value associated to the given key, inserting it if needed.
 * In that case, the key itself will be used as value.
 */
int get_value_index(const unichar* key,struct string_hash* hash) {
return get_value_index_(key,0,hash->root,hash,INSERT_IF_NEEDED,key);
}


/**
 * Returns the index value associated to the given key, inserting it if needed.
 * In that case, the key itself will be used as value.
 */
int get_value_index(const unichar* key,struct string_hash* hash,const unichar* value) {
return get_value_index_(key,0,hash->root,hash,INSERT_IF_NEEDED,value);
}



/**
 * Loads the lines of a text file info a string_hash and returns it, or NULL
 * if the file can not be opened.
 * For each line, we ignore the carriage return, if any, and we use
 * the remaining string as key and value. An error message will be printed if
 * an empty line is found.
 */
struct string_hash* load_key_list(const VersatileEncodingConfig* vec,const char* name) {
U_FILE* f=u_fopen(vec,name,U_READ);
if (f==NULL) return NULL;
struct string_hash* hash=new_string_hash(DONT_USE_VALUES);
Ustring* temp=new_Ustring(1024);
while (EOF!=readline(temp,f)) {
   if (temp->str[0]=='\0') {
      error("Empty line in %s\n",name);
   } else {
      get_value_index(temp->str,hash);
   }
}
free_Ustring(temp);
u_fclose(f);
return hash;
}


/**
 * Takes a sequence turns every every \x into x, except \r and
 * \n that are turned into 0x0D and 0x0A.
 */
static void normalize_CR_LF(unichar* s) {
int i=0,j=0;
while (s[i]!='\0') {
    if (s[i]=='\\') {
        switch (s[i+1]) {
        case '\0': fatal_error("Unexpected backslash at end of line in normalize_CR_LF\n");
        case 'r': s[j++]=0x0D; i+=2; break;
        case 'n': s[j++]=0x0A; i+=2; break;
        default: s[j++]=s[i+1]; i+=2; break;
        }
    } else {
        s[j++]=s[i++];
    }
}
s[j]='\0';
}


/**
 * Loads the lines of a text file into a string_hash and returns it, or NULL
 * if the file can not be opened. We arbitrary fix the limit of a line to 4096
 * characters. Each line is splitted into a key and a value, according to a
 * given separator character. An error message will be printed if a line does not
 * contain the separator character, if an empty line is found, or if a line contains
 * an empty key. In case of empty values, the empty string will be used.
 * Note that keys and values can contain characters protected with the \ character,
 * including protected new lines like:
 *
 * 123\
 * =ONE_TWO_THREE_NEW_LINE
 *
 * However, the preferred way to manipulate new lines is to use \r and \n in
 * keys and values.
 */
struct string_hash* load_key_value_list(const char* name, const VersatileEncodingConfig* vec,unichar separator) {
U_FILE* f=u_fopen(vec,name,U_READ);
if (f==NULL) return NULL;
struct string_hash* hash=new_string_hash();
unichar temp[4096];
unichar key[4096];
unichar value[4096];
/* We build a string with the separator character */
unichar stop[2];
stop[0]=separator;
stop[1]='\0';
unichar to_keep_protected[4];
to_keep_protected[0]='r';
to_keep_protected[1]='n';
to_keep_protected[2]='\\';
to_keep_protected[3]='\0';

int line_length = 0;
while (EOF!=(line_length=u_fgets2(temp,f))) {
   if (line_length==0) {
      error("Empty line\n");
   }
   else {
      /* First, we try to read a non empty key */
      int pos=0;
      int code=parse_string(temp,&pos,key,stop,P_EMPTY,to_keep_protected);
      if (code==P_BACKSLASH_AT_END) {
         error("Backslash at end of line:<%S>\n\n",temp);
      }
      else if (pos==0 && temp[pos]=='\0') {
         /* Empty line */
          continue;
      }
      else if (pos==0) {
         /* If the line starts with the separator */
         error("Line with empty key:\n<%S>\n",temp);
      }
      else if (pos>=line_length) {
        /* If the line doesn't have a separator */
        error("Line without separator:\n<%S>\n",temp);
      }
      else {
         /* We jump over the separator */
         pos++;
         /* We initialize 'value' with the empty string in case it is not
          * defined in the file */
         value[0]='\0';
         if(P_BACKSLASH_AT_END==parse_string(temp,&pos,value,P_EMPTY,P_EMPTY,to_keep_protected)) {
             error("Backslash at end of line:\n<%S>\n",temp);
         }
         else {
            /* If we have a valid (key,value) pair, we insert it into the string_hash */
            normalize_CR_LF(value);
            normalize_CR_LF(key);
            get_value_index(key,hash,INSERT_IF_NEEDED,value);
         }
      }
   }
}
u_fclose(f);
return hash;
}


/**
 * Dumps the values of the given string_hash into the given file (one
 * string per line). Raises a fatal error if the string_hash has no value.
 */
void dump_values(U_FILE *f,struct string_hash* hash) {
if (hash==NULL) {
   fatal_error("NULL error in dump_values\n");
}
if (hash->value==NULL) {
   fatal_error("No values to dump in dump_values\n");
}
for (int i=0;i<hash->size;i++) {
   u_fprintf(f,"%S\n",hash->value[i]);
}
}

/**
 * Dumps the first num values of the given string_hash into the given file
 * (one string per line). Raises a fatal error if the string_hash has no values.
 */
void dump_n_values(U_FILE* f, const struct string_hash* hash, int num) {
  if (hash == NULL) {
    fatal_error("NULL error in dump_values\n");
  }
  if (hash->value == NULL) {
    fatal_error("No values to dump in dump_values\n");
  }
  if (num > hash->size) {
    fatal_error("Too many values requested to dump in dump_values\n");
  }
  for (int i = 0; i < num; ++i) {
    u_fprintf(f, "%S\n", hash->value[i]);
  }
}

/**
 * This function explores the string 's' in order to find the longest prefix that is a key
 * in the string_hash. 'pos' is the current position in 's'. 'node' is the current node
 * in the string_hash tree.
 */
int get_longest_key_index_(const unichar* s,int pos,int *key_length,struct string_hash_tree_node* node) {
int index=-1;
if (node->value_index!=NO_VALUE_INDEX) {
   /* If we have a key, we check if its length is greater than the previous one, if any */
   if (pos>(*key_length)) {
      /* If the key is the longest one we have found, we update the index, but we
       * don't return, since we must look for longer keys */
      index=node->value_index;
      (*key_length)=pos;
   }
}
if (s[pos]=='\0') {
   /* If there is nothing more in the string, we must return */
   return index;
}
/* If we are not at the end of 's', we look for the transition to follow */
struct string_hash_tree_transition* t=get_transition(s[pos],node->trans);
if (t==NULL) {
   /* If there is none, we must return */
   return index;
}
/* If there is one, we look for a longer key */
int new_index=get_longest_key_index_(s,pos+1,key_length,t->node);
if (new_index!=NO_VALUE_INDEX) {
   /* If we have found a key, it is necessary a longer key, so we prefer it
    * to the previous one, if any */
   return new_index;
}
return index;
}


/**
 * This function looks in the string 's' for the longest prefix that is a key
 * in the given string_hash, and returns its value index, or NO_VALUE_INDEX if
 * no key matches. If a key is found, its length is returned in 'key_length'.
 */
int get_longest_key_index(const unichar* s,int *key_length,struct string_hash* hash) {
(*key_length)=0;
return get_longest_key_index_(s,0,key_length,hash->root);
}



/**
 * Returns a new string_hash_ptr object with the given capacity.
 * All such objects use values. If not, the normal string_hash should be
 * used. The 'value' array will be enlarged if needed.
 */
struct string_hash_ptr* new_string_hash_ptr(int capacity) {
struct string_hash_ptr* s=(struct string_hash_ptr*)malloc(sizeof(struct string_hash_ptr));
if (s==NULL) {
   fatal_alloc_error("new_string_hash_ptr");
}
/* We don't use the unichar* values of the normal string hash */
s->hash=new_string_hash(DONT_USE_VALUES);
s->capacity=capacity;
s->size=0;
s->value=(void**)calloc(capacity,sizeof(void*));
if (s->value==NULL) {
   fatal_alloc_error("new_string_hash_ptr");
}
return s;
}


/**
 * Returns a new string_hash_ptr object with the default capacity.
 * All such objects use values. If not, the normal string_hash should be
 * used. The 'value' array will be enlarged if needed.
 */
struct string_hash_ptr* new_string_hash_ptr() {
return new_string_hash_ptr(DEFAULT_CAPACITY);
}


/**
 * This function frees the given string_hash_ptr, using 'free_' (if not NULL)
 * to free the elements of the 'value' array.
 */
void free_string_hash_ptr(struct string_hash_ptr* s,void (*free_)(void*)) {
if (s==NULL) return;
/* If necessary, we free the 'value' array */
if (free_!=NULL) {
   for (int i=0;i<s->hash->size;i++) {
      free_(s->value[i]);
   }
}
free(s->value);
free_string_hash(s->hash);
free(s);
}


/**
 * Returns the index value associated to the given key.
 */
int get_value_index(const unichar* key,struct string_hash_ptr* hash,int insert_policy) {
return get_value_index_(key,0,hash->hash->root,hash->hash,insert_policy,NULL);
}


/**
 * Returns the index value associated to the given key, inserting it if needed.
 */
int get_value_index(const unichar* key,struct string_hash_ptr* hash) {
return get_value_index_(key,0,hash->hash->root,hash->hash,INSERT_IF_NEEDED,NULL);
}


/**
 * Returns the index value associated to the given key. 'value' will be associated to
 * the given key if the key is not already present in the string_hash_ptr.
 */
int get_value_index(const unichar* key,struct string_hash_ptr* hash,int insert_policy,void* value) {
int size=hash->hash->size;
int index=get_value_index_(key,0,hash->hash->root,hash->hash,insert_policy,NULL);
if (index==-1) {
   /* If the key was neither found nor inserted, we return -1 */
   return -1;
}
if (hash->hash->size!=size) {
   hash->size=hash->hash->size;
   /* If the key was inserted, we add the corresponding value into the 'value' array */
   /* Otherwise: if there is a maximum capacity */
   if (hash->hash->size==hash->capacity) {
      /* We enlarge the 'value' array, doubling its capacity */
      hash->capacity=2*hash->capacity;
      hash->value=(void**)realloc(hash->value,sizeof(void*)*hash->capacity);
      if (hash->value==NULL) {
         fatal_alloc_error("get_value_index\n");
      }
   }
   hash->value[index]=value;
}
return index;
}


/**
 * Returns the value associated to the given key, or NULL if not found in the
 * given string_hash_ptr.
 *
 * WARNING: NULL is also returned if it has been associated to the key, so
 *          it's not possible to distinguish the cases (key,NULL) and key not found.
 */
void* get_value(const unichar* key,struct string_hash_ptr* hash) {
int index=get_value_index(key,hash->hash,DONT_INSERT);
if (index==-1) return NULL;
return hash->value[index];
}


/**
 * Adds a value in the value array without associating it with a unicode string.
 * Returns the index of this value.
 */
int add_value(void* value,struct string_hash_ptr* hash) {
if (hash->capacity==DONT_USE_VALUES) {
   fatal_error("Value array doesn't exist in add_value\n");
}
(hash->hash->size)++;
hash->size=hash->hash->size;
int index=hash->size;
if (hash->hash->size==hash->capacity) {
   /* We enlarge the 'value' array, doubling its capacity */
   hash->capacity=2*hash->capacity;
   hash->value=(void**)realloc(hash->value,sizeof(void*)*hash->capacity);
   if (hash->value==NULL) {
      fatal_alloc_error("add_value");
   }
}
hash->value[index]=value;
return index;
}

} // namespace unitex
