/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "DictionaryTree.h"
#include "Error.h"
#include "Ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates, initializes and returns a dictionary node.
 */
struct dictionary_node* new_dictionary_node(Abstract_allocator prv_alloc) {
struct dictionary_node* a=(struct dictionary_node*)malloc_cb(sizeof(struct dictionary_node),prv_alloc);
if (a==NULL) {
    fatal_alloc_error("new_dictionary_node");
}
a->single_INF_code_list=NULL;
a->offset=-1;
a->trans=NULL;
a->incoming=0;
a->n_trans=0;
a->INF_code=0;
return a;
}


/**
 * Allocates, initializes and returns a dictionary node transition.
 */
static inline struct dictionary_node_transition* new_dictionary_node_transition(Abstract_allocator prv_alloc) {
struct dictionary_node_transition* t=(struct dictionary_node_transition*)malloc_cb(sizeof(struct dictionary_node_transition),prv_alloc);
if (t==NULL) {
    fatal_alloc_error("new_dictionary_node_transition");
}
t->letter='\0';
t->output=NULL;
t->node=NULL;
t->next=NULL;
return t;
}


/**
 * Frees all the dictionary graph whose root is 'a'.
 */
void free_dictionary_node(struct dictionary_node* a,Abstract_allocator prv_alloc) {
if (a==NULL) return;

if (get_allocator_cb_flag(prv_alloc) & AllocatorGetFlagAutoFreePresent) return;

if (a->incoming>1) {
    /* We don't free a state that is still pointed by someone else
     * in order to avoid double freeing problems. */
   (a->incoming)--;
    return;
}
free_list_int(a->single_INF_code_list,prv_alloc);
free_dictionary_node_transition(a->trans,prv_alloc);
free_cb(a,prv_alloc);
}


/**
 * Frees the dictionary node transition 't', and all the dictionary graph whose root is
 * the dictionary node pointed out by 't'.
 */
void free_dictionary_node_transition(struct dictionary_node_transition* t,Abstract_allocator prv_alloc) {
struct dictionary_node_transition* tmp;
while (t!=NULL) {
   free(t->output);
   free_dictionary_node(t->node,prv_alloc);
   tmp=t;
   t=t->next;
   free_cb(tmp,prv_alloc);
}
}


/**
 * This function looks for a transition of the dictionary node '*n' that is
 * tagged with 'c'. If the transition exists, it is returned. Otherwise, the
 * transition is created, inserted according to the Unicode order and returned.
 * In that case, the destination dictionary node is NULL.
 */
static struct dictionary_node_transition* get_transition(unichar c,struct dictionary_node** n,Abstract_allocator prv_alloc) {
struct dictionary_node_transition* tmp;
if ((*n)->trans==NULL || c<((*n)->trans->letter)) {
   /* If we must insert at first position */
   tmp=new_dictionary_node_transition(prv_alloc);
   tmp->letter=c;
   tmp->next=(*n)->trans;
   tmp->node=NULL;
   (*n)->trans=tmp;
   return tmp;
}
if (c==((*n)->trans->letter)) {
   /* If the transition exists, we return it */
   return (*n)->trans;
}
/* Otherwise, we look for the transition, or the correct
 * place to insert it */
tmp=(*n)->trans;
while (tmp->next!=NULL && c>tmp->next->letter) {
   tmp=tmp->next;
}
if (tmp->next==NULL || (tmp->next->letter)>c) {
   /* If we must insert between tmp and tmp->next */
   struct dictionary_node_transition* ptr;
   ptr=new_dictionary_node_transition(prv_alloc);
   ptr->letter=c;
   ptr->next=tmp->next;
   ptr->node=NULL;
   tmp->next=ptr;
   return ptr;
}
/* If we have (tmp->next->c == c) */
return tmp->next;
}


/**
 * This structure is used to pass several constant parameters to the function
 * 'add_entry_to_dictionary_tree'.
 */
struct dictionnary_info {
    unichar* INF_code;
    struct string_hash* INF_code_list;
};


/**
 * This function create a string from two string, and build the hash value
 * This function was extracted from add_entry_to_dictionary_tree, because each recursive call
 * allocated a lot of unichar (and produce stack overflow)
 */
#define DEFAULT_TMP_GET_VALUE_INDEX_BUFFER_SIZE (0x200)
static int get_value_index_for_string_colon_string(const unichar* str1,const unichar* str2,struct string_hash* hash) {
   int value;
   unichar*allocated_buffer = NULL;
   unichar tmp_default[DEFAULT_TMP_GET_VALUE_INDEX_BUFFER_SIZE];
   unichar*tmp=tmp_default;
   int nb_unichar_buffer=u_strlen(str1)+u_strlen(str2)+2;
   if (nb_unichar_buffer>DEFAULT_TMP_GET_VALUE_INDEX_BUFFER_SIZE) {
       tmp=allocated_buffer=(unichar*)malloc(sizeof(unichar*)*nb_unichar_buffer);
       if (allocated_buffer==NULL) {
          fatal_alloc_error("get_value_index_for_string_colon_string");
       }
   }
   u_sprintf(tmp,"%S,%S",str1,str2);
   value=get_value_index(tmp,hash);
   if (allocated_buffer != NULL) {
     free(allocated_buffer);
   }
   return value;
}

/**
 * This function explores a dictionary tree in order to insert an entry.
 * 'inflected' is the inflected form to insert, and 'pos' is the current position
 * in the string 'inflected'. 'node' is the current node in the dictionary tree.
 * 'infos' is used to access to constant parameters.
 */
static void add_entry_to_dictionary_tree(const unichar* inflected,int pos,struct dictionary_node* node,
                                  struct dictionnary_info* infos,int /*line*/, Abstract_allocator prv_alloc) {
for (;;) {
if (inflected[pos]=='\0') {
   /* If we have reached the end of 'inflected', then we are in the
    * node where the INF code must be inserted */
   int N=get_value_index(infos->INF_code,infos->INF_code_list);
   if (node->single_INF_code_list==NULL) {
      /* If there is no INF code in the node, then
       * we add one and we return */
      node->single_INF_code_list=new_list_int(N,prv_alloc);
      node->INF_code=N;
      return;
   }
   /* If there is an INF code list in the node ...*/
   if (is_in_list(N,node->single_INF_code_list)) {
      /* If the INF code has already been taken into account for this node
       * (case of duplicates), we do nothing */
      return;
   }
   /* Otherwise, we add it to the INF code list */
   node->single_INF_code_list=head_insert(N,node->single_INF_code_list,prv_alloc);
    /* And we update the global INF line for this node */
   node->INF_code=get_value_index_for_string_colon_string(infos->INF_code_list->value[node->INF_code],infos->INF_code,infos->INF_code_list);
   return;
}
/* If we are not at the end of 'inflected', then we look for
 * the correct outgoing transition and we follow it */
struct dictionary_node_transition* t=get_transition(inflected[pos],&node,prv_alloc);
if (t->node==NULL) {
   /* We create the node if necessary */
   t->node=new_dictionary_node(prv_alloc);
   (t->node->incoming)++;
}

node=t->node;
pos++;
}
}

/**
 * This function inserts an entry in a dictionary tree. 'inflected' represents
 * the inflected form of the entry, and 'INF_code' represents the compressed line
 * that will be used for rebuilding the whole DELAF line. 'root' is the root
 * of the dictionary tree and 'INF_code_list' is the structure that contains
 * all the INF codes that are used in the given dictionary tree.
 */
void add_entry_to_dictionary_tree(unichar* inflected,unichar* INF_code,
                                  struct dictionary_node* root,struct string_hash* INF_code_list,
                                  int line,Abstract_allocator prv_alloc) {
struct dictionnary_info infos;
infos.INF_code=INF_code;
infos.INF_code_list=INF_code_list;
add_entry_to_dictionary_tree(inflected,0,root,&infos,line,prv_alloc);
}





/******************************************************************
 *
 *
 * The following code is the minimization of the dictionary tree.
 *
 *
 ******************************************************************/


/* The maximum height of the tree corresponds to the length of the
 * longest word in the dictionary */
#define MAXIMUM_HEIGHT 10000

/* We define here the maximum number of transitions for a given height.
 * We use a hack to get bigger size on 64bit-architectures */
//#define MAXIMUM_TRANSITIONS ((sizeof(void*) > 4) ? 134217728 : 4194304 )


/**
 * We define here a list of transitions in the dictionary tree.
 * It's here and not in the .h because we don't want to export this
 * internal data structure.
 */
struct transition_list {
   struct dictionary_node_transition* transition;
   struct transition_list* next;
};

static inline int compare_nodes(const struct dictionary_node_transition*,const struct dictionary_node_transition*);
//void init_minimize_arrays(struct transition_list***,struct dictionary_node_transition***);
static void init_minimize_arrays_transition_list(struct transition_list***);
static void init_minimize_arrays_dictionary_node_transition(struct dictionary_node_transition***,unsigned int nb);
static void free_minimize_arrays(struct transition_list**,struct dictionary_node_transition**);
static int sort_by_height(struct dictionary_node*,struct transition_list**,struct bit_array*,Abstract_allocator);
static struct transition_list* new_transition_list(struct dictionary_node_transition*,struct transition_list*,Abstract_allocator);
static int convert_list_to_array(unsigned int,struct transition_list**,
                          struct dictionary_node_transition**,unsigned int,Abstract_allocator);
static int convert_list_to_array_size(unsigned int height,struct transition_list** transitions_by_height);
static void quicksort(int,int,struct dictionary_node_transition**);
static void merge(int,struct dictionary_node_transition**,Abstract_allocator);
static inline void check_nodes(const struct dictionary_node_transition* a);



/**
 * This function takes a dictionary tree and minimizes it using
 * Dominique Revuz's algorithm. 'used_inf_values' is used to mark
 * INF codes that are actually used in the .bin.
 */
void minimize_tree(struct dictionary_node* root,struct bit_array* used_inf_values,Abstract_allocator prv_alloc) {
u_printf("Minimizing...                      \n");
struct transition_list** transitions_by_height;
struct dictionary_node_transition** transitions;
//init_minimize_arrays(&transitions_by_height,&transitions);

init_minimize_arrays_transition_list(&transitions_by_height);


unsigned int H=sort_by_height(root,transitions_by_height,used_inf_values,prv_alloc);

unsigned int nb=0;
for (unsigned int k1=0;k1<=H;k1++) {
    unsigned int nbcur = convert_list_to_array_size(k1,transitions_by_height);
    if (nbcur>nb) {
        nb = nbcur;
    }
}
init_minimize_arrays_dictionary_node_transition(&transitions,nb);
float z;
for (unsigned int k=0;k<=H;k++) {
   int size=convert_list_to_array(k,transitions_by_height,transitions,nb,prv_alloc);
   for (int l=0;l<size;l++) {
       check_nodes(transitions[l]);
   }
   quicksort(0,size-1,transitions);
   merge(size,transitions,prv_alloc);
   z=(float)(100.0*(float)(k)/(float)H);
   if (z>100.0) z=(float)100.0;
   u_printf("%2.0f%% completed...    \r",z);
}
u_printf("Minimization done.                     \n");
free_minimize_arrays(transitions_by_height,transitions);
}


static inline void check_nodes(const struct dictionary_node_transition* a) {
if (a==NULL || a->node==NULL) {
   fatal_error("Internal error in check_nodes\n");
}
}

/**
 * This function compares two tree nodes as follow:
 * 1) by the unichar that lead to them
 * 2) by their hash_number (n� of line in INF file)
 * 3) by the transition that get out of them
 */
static inline int compare_nodes(const struct dictionary_node_transition* a,const struct dictionary_node_transition* b) {
/* If the nodes have not the same INF codes, they are different */

    struct dictionary_node* a_node = a->node;
    struct dictionary_node* b_node = b->node;

    if (a_node->single_INF_code_list!=b_node->single_INF_code_list) {
        if (a_node->single_INF_code_list!=NULL && b_node->single_INF_code_list==NULL) return -1;
        if (a_node->single_INF_code_list==NULL && b_node->single_INF_code_list!=NULL) return 1;
        }
    if (a_node->single_INF_code_list!=NULL && b_node->single_INF_code_list!=NULL &&
            a_node->INF_code!=b_node->INF_code)
            return (a_node->INF_code - b_node->INF_code);

/* Then, we compare all the outgoing transitions, two by two */
a=a_node->trans;
b=b_node->trans;
while(a!=NULL && b!=NULL) {
   /* If the 2 current transitions are not tagged by the same
    * character, then the nodes are different */
   if (a->letter - b->letter != 0) return (a->letter - b->letter);
   int output_diff=u_strcmp(a->output,b->output);
   if (output_diff!=0) return output_diff;
   /* If the characters are equal and destination nodes are different... */
   if (((int)(a->node - b->node)) != 0) return (int)(a->node - b->node);
   a=a->next;
   b=b->next;
}
if (a==b) {
   /* If a==b==NULL, the transition lists are equal, the nodes are equivalent */
   return 0;
}
if (a==NULL) {
   /* If the first list is shorter than the second */
   return -1;
}
/* If the first list is longer then the second */
return 1;
}


/**
 * We allocate and initialize 2 arrays used by the minimization.
 */
static void init_minimize_arrays_transition_list(struct transition_list** *transitions_by_height) {
(*transitions_by_height)=(struct transition_list**)malloc(MAXIMUM_HEIGHT*sizeof(struct transition_list*));
if (*transitions_by_height==NULL) {
   fatal_alloc_error("init_minimize_arrays");
}
memset(*transitions_by_height,0,MAXIMUM_HEIGHT*sizeof(struct transition_list*));
}

static void init_minimize_arrays_dictionary_node_transition(struct dictionary_node_transition** *transitions,unsigned int nb)
{
(*transitions)=(dictionary_node_transition**)malloc(nb*sizeof(struct dictionary_node_transition*));
if (*transitions==NULL) {
   fatal_alloc_error("init_minimize_arrays");
}
}


/**
 * Frees all the given transition list.
 */
static void free_transition_list(struct transition_list* l) {
struct transition_list* ptr;
while (l!=NULL) {
   ptr=l;
   l=l->next;
   free(ptr);
}
}


/**
 * Frees the 2 arrays used by the minimization.
 */
static void free_minimize_arrays(struct transition_list** transitions_by_height,
                          struct dictionary_node_transition** transitions) {
for (int i=0;i<MAXIMUM_HEIGHT;i++) {
   free_transition_list(transitions_by_height[i]);
}
free(transitions_by_height);
/* The 'transitions' array is supposed to be already cleaned */
free(transitions);
}


/**
 * This function explores the dictionary and puts its transitions into the
 * 'transitions' array. For a given height, the transitions are not sorted,
 * they will be later in the 'minimize_tree' function.
 * The function returns the height of the given node.
 */
static int sort_by_height(struct dictionary_node* n,struct transition_list** transitions_by_height,
                    struct bit_array* used_inf_values,Abstract_allocator prv_alloc) {
if (n==NULL) {
   fatal_error("NULL error in sort_by_height\n");
}
/* We mark used INF codes */
if (n->single_INF_code_list!=NULL) {
    set_value(used_inf_values,n->INF_code,1);
}
if (n->trans==NULL) {
   /* If the node is a leaf, we have nothing to do */
   return 0;
}
struct dictionary_node_transition* trans=n->trans;
int height=-1;
int k;
while (trans!=NULL) {
   /* We process recursively the node pointed out by the current transition */
   k=sort_by_height(trans->node,transitions_by_height,used_inf_values,prv_alloc);
   if (k==MAXIMUM_HEIGHT) {
      fatal_error("Maximum height reached in sort_by_height\n");
   }
   /* We insert the transition in the appropriate list */
   transitions_by_height[k]=new_transition_list(trans,transitions_by_height[k],prv_alloc);
   /* The height of the current node is the maximum of his sons' heights... */
   if (height<k) height=k;
   trans=trans->next;
}
/* ... +1 because of the current node itself */
return 1+height;
}


/**
 * Allocates, initializes and returns a new transition list element.
 */
static struct transition_list* new_transition_list(struct dictionary_node_transition* transition,
                                      struct transition_list* next,Abstract_allocator prv_alloc) {
struct transition_list* t;
t=(struct transition_list*)malloc_cb(sizeof(struct transition_list),prv_alloc);
if (t==NULL) {
   fatal_alloc_error("new_transition_list");
}
t->transition=transition;
t->next=next;
return t;
}


/**
 * We convert the list of transitions corresponding the given height
 * into an array, in order to apply the quicksort.
 */
static int convert_list_to_array(unsigned int height,struct transition_list** transitions_by_height,
                          struct dictionary_node_transition** transitions,unsigned int maximum_transition,Abstract_allocator prv_alloc) {
unsigned int size=0;
struct transition_list* l=transitions_by_height[height];
struct transition_list* ptr;
transitions_by_height[height]=NULL;
while (l!=NULL) {
   if (size>=maximum_transition) {
      fatal_error("MAX_TRANS=%u reached: exiting!\n",maximum_transition);
   }
   transitions[size++]=l->transition;
   ptr=l;
   l=l->next;
   free_cb(ptr,prv_alloc);
}
return size;
}

static int convert_list_to_array_size(unsigned int height,struct transition_list** transitions_by_height)
{
unsigned int size=0;
struct transition_list* l=transitions_by_height[height];

while (l!=NULL) {
   size++;
   l=l->next;
}
return size;
}

/**
 * Builds a quicksort partition of the given array.
 */
static inline int partition(int end,struct dictionary_node_transition** transitions) {
struct dictionary_node_transition* pivot;
struct dictionary_node_transition* tmp;
int i=-1;
/* Final pivot index */
int j=end+1;
pivot=transitions[(end)/2];
for (;;) {
   do j--;
   while ((j>(-1))&&(compare_nodes(pivot,transitions[j]) < 0));
   do i++;
   while ((i<end+1)&&(compare_nodes(transitions[i],pivot) < 0));
   if (i<j) {
      tmp=transitions[i];
      transitions[i]=transitions[j];
      transitions[j]=tmp;
   } else return j;
}
}


/**
 * Sorts the given transition array, according to Dominique Revuz's criteria.
 */
static void quicksort(int end,struct dictionary_node_transition** transitions) {
int p;
if (end>0) {
   p=partition(end,transitions);
   quicksort(p,transitions);
   quicksort(end-(p+1),transitions+p+1);
}
}

static void quicksort(int start,int end,struct dictionary_node_transition** transitions) {
if (start<end) {
   quicksort(end-start,transitions+start);
}
}

/**
 * 'transitions' is supposed to be sorted, i.e. equivalent transitions
 * are contiguous. If two transitions point to equivalent nodes, we
 * redirect the second transition on the first's node and we free the
 * node that is not pointed anymore.
 */
static void merge(int size,struct dictionary_node_transition** transitions,Abstract_allocator prv_alloc) {
int i=1;
struct dictionary_node_transition* base=transitions[0];
while (i<size) {
   if (compare_nodes(base,transitions[i])==0) {
      /* If the base transition is equivalent to the current one
       * then we must destroy the current one's destination node */
      free_dictionary_node(transitions[i]->node,prv_alloc);
      /* We modify the current one's destination node */
      transitions[i]->node=base->node;
      /* And we increase the number of references of the node, in order
       * to know later when it could be freed */
      (base->node->incoming)++;
   }
   else {
      base=transitions[i];
   }
   i++;
}
}


/**
 * This function takes a prefix and a string s. It replaces
 * pfx by the longest prefix that is common to pfx and s.
 */
static void get_longest_common_prefix(Ustring* pfx,unichar* s) {
if (s==NULL) {
    empty(pfx);
    return;
}
int i=0;
while (pfx->str[i]==s[i] && s[i]!='\0') {
    i++;
}
pfx->len=i;
pfx->str[i]='\0';
}


/**
 * This function removes a prefix of length n in string s.
 *
 * n=3 s=abcdef => def
 */
static void remove_prefix(int n,unichar* s) {
if (n==0) return;
for (int i=n;s[i-1]!='\0';i++) {
    s[i-n]=s[i];
}
}


/**
 * This function moves outputs from final nodes to transitions leading to final nodes.
 */
static void subsequential_to_normal_transducer(struct dictionary_node* root,
        struct dictionary_node* node,
        struct string_hash* inf_codes,
        int pos,unichar* z,
        Ustring* normalizedOutput) {
struct dictionary_node_transition* tmp=node->trans;
int prefix_set=0;
Ustring* prefix=new_Ustring();
while (tmp!=NULL) {
    z[pos]=tmp->letter;
    z[pos+1]='\0';
    subsequential_to_normal_transducer(root,tmp->node,inf_codes,pos+1,z,normalizedOutput);
    /* First, if the destination state is final, we place its output on the output
     * of the current transition */

    if (tmp->node->single_INF_code_list!=NULL) {
        //error("<%S>: output=<%S>\n",z,normalizedOutput->str);
        tmp->output=u_strdup(inf_codes->value[tmp->node->INF_code]);
    }
    if (normalizedOutput->len!=0) {
        /* Then, we add the normalized output obtained recursively, if any */
        //error("<%S>: moving normalized output <%S>\n",z,normalizedOutput->str);
        if (tmp->output==NULL) {
            tmp->output=u_strdup(normalizedOutput->str);
        } else {
            tmp->output=(unichar*)realloc(tmp->output,sizeof(unichar)*(1+normalizedOutput->len+u_strlen(tmp->output)));
        }
    }
    if (!prefix_set) {
        prefix_set=1;
        u_strcpy(prefix,tmp->output);
    } else {
        get_longest_common_prefix(prefix,tmp->output);
    }
    tmp=tmp->next;
}
if (node==root || node->single_INF_code_list!=NULL) {
    /* If we are in the initial state or a final one, we let the transitions as they are, since
     * their outputs can not move more to the left */
    z[pos]='\0';
    free_Ustring(prefix);
    empty(normalizedOutput);
    return;
}
tmp=node->trans;
while (tmp!=NULL) {
    //error("prefix removal: <%S> => ",tmp->output);
    remove_prefix(prefix->len,tmp->output);
    //error("<%S>\n",tmp->output);
    tmp=tmp->next;
}
z[pos]='\0';
u_strcpy(normalizedOutput,prefix);
free_Ustring(prefix);
}


/**
 * This function takes a lexicographic tree with inf codes stored as
 * integer on nodes, and turns it into a real transducer where outputs
 * are stored on transitions.
 */
void move_outputs_on_transitions(struct dictionary_node* root,struct string_hash* inf_codes) {
int pos=0;
unichar z[0x400];
Ustring* normalizedOutput=new_Ustring();
subsequential_to_normal_transducer(root,root,inf_codes,pos,z,normalizedOutput);
free_Ustring(normalizedOutput);
}

} // namespace unitex
