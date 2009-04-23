 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include "LocatePattern.h"
#include "CompoundWordTree.h"
#include "Error.h"
#include "Tokenization.h"



void free_DLC_tree_node(struct DLC_tree_node*);
void quicksort(int*,int);
void quicksort2(int*,void**,int);
IntSequence clone_IntSequence(IntSequence);
int compare_IntSequence(IntSequence,IntSequence);


/**
 * Allocates, initializes and returns a compound word tree node.
 */
struct DLC_tree_node* new_DLC_tree_node() {
struct DLC_tree_node* n;
n=(struct DLC_tree_node*)malloc(sizeof(struct DLC_tree_node));
if (n==NULL) {
	fatal_alloc_error("new_DLC_tree_node");
}
n->patterns=NULL;
n->number_of_patterns=0;
n->array_of_patterns=NULL;
n->transitions=NULL;
n->number_of_transitions=0;
n->destination_tokens=NULL;
n->destination_nodes=NULL;
n->duplicates=NULL;
return n;
}


/**
 * Allocates, initializes and returns a compound word tree transition.
 */
struct DLC_tree_transition* new_DLC_tree_transition() {
struct DLC_tree_transition* t;
t=(struct DLC_tree_transition*)malloc(sizeof(struct DLC_tree_transition));
if (t==NULL) {
	fatal_alloc_error("new_DLC_tree_transition");
}
t->token_sequence=NULL;
t->node=NULL;
t->next=NULL;
return t;
}


/**
 * This function initializes the root of the compound word tree and
 * the 'index' array that will be used to access directly to the
 * compound words that begin by a given token. 'number_of_tokens'
 * is supposed to represent the total number of distinct tokens in
 * the text to parse. The function returns a structure that contains the
 * root and the index.
 */
struct DLC_tree_info* new_DLC_tree(int number_of_tokens) {
struct DLC_tree_info* DLC_tree=(struct DLC_tree_info*)malloc(sizeof(struct DLC_tree_info));
if (DLC_tree==NULL) {
	fatal_alloc_error("new_DLC_tree");
}
DLC_tree->root=new_DLC_tree_node();
DLC_tree->index=(struct DLC_tree_node**)malloc(number_of_tokens*sizeof(struct DLC_tree_node*));
if (DLC_tree->index==NULL) {
   fatal_alloc_error("new_DLC_tree");
}
for (int i=0;i<number_of_tokens;i++) {
	DLC_tree->index[i]=NULL;
}
return DLC_tree;
}


/**
 * Frees a transition list. For each transition, the destination node is
 * also freed.
 */
void free_DLC_tree_transitions(struct DLC_tree_transition* transitions) {
struct DLC_tree_transition* tmp;
while (transitions!=NULL) {
	free_DLC_tree_node(transitions->node);
   if (transitions->token_sequence!=NULL) free(transitions->token_sequence);
	tmp=transitions->next;
	free(transitions);
	transitions=tmp;
}
}


/**
 * Frees the compound word tree whose root is 'node'.
 *
 * WARNING: this function tries to free both 'transitions' and 'destination_nodes',
 *          so, in order to avoid double freeing, the programmer must take care not
 *          to have a same node referenced in both 'transitions' and 'destination_nodes'.
 */
void free_DLC_tree_node(struct DLC_tree_node* node) {
if (node==NULL) return;
free_list_int(node->patterns);
if (node->array_of_patterns!=NULL) free(node->array_of_patterns);
free_DLC_tree_transitions(node->transitions);
if (node->destination_tokens!=NULL) free(node->destination_tokens);
if (node->destination_nodes!=NULL) {
	for (int i=0;i<node->number_of_transitions;i++) {
		if (node->duplicates[i] == 0) {
			bool bDup = false;

			for (int j = 0; j < i; j++)
				if (node->destination_nodes[i] == node->destination_nodes[j]) {
					bDup = true;
					break;
				}
			if (!bDup)
				free_DLC_tree_node(node->destination_nodes[i]);
		}
	}
	free(node->destination_nodes);
}
free(node->duplicates);
free(node);
}


/**
 * Frees a compound word tree.
 */
void free_DLC_tree(struct DLC_tree_info* DLC_tree) {
if (DLC_tree==NULL) return;
if (DLC_tree->index!=NULL) free(DLC_tree->index);
free_DLC_tree_node(DLC_tree->root);
free(DLC_tree);
}


/**
 * This function takes a unicode string 'word' representing a compound word, and
 * tokenizes it into tokens. The output is an array 'tokens' that contains the
 * numbers of the tokens that constitute the word. If case variants are allowed,
 * a token can be replaced by a token list delimited by the special values
 * BEGIN_CASE_VARIANT_LIST and END_CASE_VARIANT_LIST. The token list is ended
 * by END_TOKEN_LIST.
 *
 * The array 'tokens' is supposed to be large enough. 'tok' represents the text tokens.
 * 'tokenization_mode' indicates if the word must be tokenized character by character
 * or not.
 */
void tokenize_compound_word(unichar* word,int tokens[],Alphabet* alphabet,
                            struct string_hash* tok,TokenizationPolicy tokenization_mode) {

int i,n_token,j;
struct list_ustring* list=tokenize(word,tokenization_mode,alphabet);
struct list_ustring* tmp;
struct list_int* ptr;
i=0;
n_token=0;
while (list!=NULL) {
   j=get_value_index(list->string,tok,DONT_INSERT);
   /* If a token of a compound word is not a token of the text,
    * we MUST NOT ignore it. For instance, if we have the compound
    * word "a priori" and if the text only contains "PRIORI", it is not
    * an error case. The error case is when there is no case equivalent of
    * "priori" in the text. In such a situation, we traduce it by an empty
    * list. We don't raise an error because if there is by accident a token
    * in a dictionary that is not in the text, it would block the Locate
    * without necessity. */
   if (is_letter2(list->string[0],alphabet) || j==-1) {
      /* If the current token is made of letters, we look for all
       * its case variants. If we have a non letter token that is
       * not in the text tokens, we handle it here to produce an
       * empty case variant list. */
      tokens[n_token++]=BEGIN_CASE_VARIANT_LIST;
      ptr=get_token_list_for_sequence(list->string,alphabet,tok);
      struct list_int* ptr_copy=ptr; // s.n.
      while (ptr!=NULL) {
         j=ptr->n;
         tokens[n_token++]=j;
         ptr=ptr->next;
      }
      free_list_int(ptr_copy); // s.n.
      tokens[n_token++]=END_CASE_VARIANT_LIST;
   } else {
      /* If we have a non letter single character, we just add its number to
       * the token array */
      tokens[n_token++]=j;
   }
   tmp=list;
   list=list->next;
   free_list_ustring_element(tmp);
}
/* Finally, we end the token list. */
tokens[n_token]=END_TOKEN_LIST;
}


/**
 * This function adds a pattern number to the pattern list of a given
 * compound word tree node.
 */
void add_pattern_to_DLC_tree_node(struct DLC_tree_node* node,int pattern) {
struct list_int *previous;
if (node->patterns==NULL) {
  /* If the list is empty, we add the pattern */
  node->patterns=new_list_int(pattern);
  /* We update the length of the list */
  (node->number_of_patterns)++;
  return;
}
if (node->patterns->n==pattern)
  /* If the first element of the list is the same than 'pattern'
   * we have nothing to do */
  return;
if (node->patterns->n>pattern) {
  /* If we must insert 'pattern' at the beginning of the list */
  node->patterns=head_insert(pattern,node->patterns);
  /* We update the length of the list */
  (node->number_of_patterns)++;
  return;
}
/* General case */
previous=node->patterns;
int stop=0;
/* We parse the list until we have found the pattern or the place
 * to insert the pattern */
while (!stop && previous->next!=NULL) {
	/* If we find the pattern in the list, we have nothing to do */
	if (previous->next->n==pattern) return;
	else if (previous->next->n<pattern) previous=previous->next;
	else stop=1;
}
/* If must insert the pattern */
previous->next=head_insert(pattern,previous->next);
/* We update the length of the list */
(node->number_of_patterns)++;
return;
}


/**
 * This function looks if 'token' tags a transition that outgoes from 'node'. If the
 * parameter 'create_if_necessary' is set to 0, NULL is returned if no transition
 * tagged by 'token' is found. Otherwise, a transition is created, and the destination
 * node of this transition is created and returned.
 */
struct DLC_tree_node* get_DLC_tree_node(struct DLC_tree_node* node,IntSequence token_sequence,int create_if_necessary) {
struct DLC_tree_transition* l;
if (node->transitions==NULL) {
  /* If the list is empty */
  /* We return if the function must not create the node */
  if (!create_if_necessary) return NULL;
  /* Otherwise, we create a transition */
  l=new_DLC_tree_transition();
  l->token_sequence=clone_IntSequence(token_sequence);
  l->next=node->transitions;
  /* And we create the destination node of this transition */
  l->node=new_DLC_tree_node();
  node->transitions=l;
  /* Finally we return the created node */
  return l->node;
}
int compare=compare_IntSequence(node->transitions->token_sequence,token_sequence);
if (compare==0) {
   /* If the head of the list is the one we look for, we return it */
   return node->transitions->node;
}
if (compare<0) {
  /* If we must insert at the beginning of the list */
  /* We return if the function must not create the node */
  if (!create_if_necessary) return NULL;
  /* Otherwise, we create a transition */
  l=new_DLC_tree_transition();
  l->token_sequence=clone_IntSequence(token_sequence);
  l->next=node->transitions;
  /* And we create the destination node of this transition */
  l->node=new_DLC_tree_node();
  node->transitions=l;
  /* Finally we return the created node */
  return l->node;
}
/* General case */
struct DLC_tree_transition* previous=node->transitions;
int stop=0;
/* We parse the list until we have found the node or the place
 * to insert the node */
while (!stop && previous->next!=NULL) {
	/* If we find the node, we return it */
   compare=compare_IntSequence(previous->next->token_sequence,token_sequence);
	if (compare==0) return previous->next->node;
	else if (compare<0) previous=previous->next;
	else stop=1;
}
/* We return if the function must not create the node */
if (!create_if_necessary) return NULL;
/* Otherwise, we create a transition */
l=new_DLC_tree_transition();
l->token_sequence=clone_IntSequence(token_sequence);
/* And we create the destination node of this transition */
l->node=new_DLC_tree_node();
l->next=previous->next;
previous->next=l;
(node->number_of_transitions)++;
/* Finally we return the created node */
return l->node;
}


/**
 * This function explores the compound word tree in order to associate
 * a pattern number to a compound word. If the compound word is not already
 * in the tree, it is inserted. As a side effect, the index of 'DLC_tree'
 * is updated.
 *
 * 'token_list' is an array representing the sequence of tokens that
 * constitute the compound word. This array has been produced by the
 * function 'tokenize_compound_word'.
 *
 * 'pos' is the current position in the array 'token_list'.
 *
 * 'node' is the current node in the tree.
 *
 * 'pattern' is the pattern number to add.
 *
 * 'DLC_tree' represents the tree and the compound word index.
 */
void associate_pattern_to_compound_word(int* token_list,int pos,struct DLC_tree_node* node,
				int pattern,struct DLC_tree_info* DLC_tree) {
if (token_list[pos]==END_TOKEN_LIST) {
   /* If we are at the end of the token list, we
    * add the pattern number to the current node */
   add_pattern_to_DLC_tree_node(node,pattern);
   return;
}
int int_sequence[256];
if (token_list[pos]!=BEGIN_CASE_VARIANT_LIST) {
   /* If the token is a single token, then we put it into the token
    * sequence */
   int_sequence[0]=token_list[pos];
   int_sequence[1]=-1;
   pos++;
} else {
   /* If we have a token sequence, we build it */
   int j=0;
   do {
      pos++;
      int_sequence[j++]=token_list[pos];
   } while (token_list[pos]!=END_CASE_VARIANT_LIST);
   /* We use j-1 because the END_CASE_VARIANT_LIST value has been put
    * at position j-1 */
   int_sequence[j-1]=-1;
   pos++;
   /* And we sort it */
   quicksort(int_sequence,j-1);
}
/* Then, we look for the node that we can reach with our int sequence */
struct DLC_tree_node* ptr=get_DLC_tree_node(node,int_sequence,1);
associate_pattern_to_compound_word(token_list,pos,ptr,pattern,DLC_tree);
}


/**
 * Adds a compound word to the tree 'DLC_tree' with the value
 * COMPOUND_WORD_PATTERN. This is used when the user looks
 * for any compound word, regardless the pattern, with <DIC> or <CDIC>.
 */
void add_compound_word_with_no_pattern(unichar* word,Alphabet* alph,struct string_hash* tok,
							struct DLC_tree_info* DLC_tree,TokenizationPolicy tokenization_mode) {
add_compound_word_with_pattern(word,COMPOUND_WORD_PATTERN,alph,tok,DLC_tree,
							tokenization_mode);
}


/**
 * Adds a compound word to the tree 'DLC_tree' with the pattern
 * number 'pattern'.
 */
void add_compound_word_with_pattern(unichar* word,int pattern,Alphabet* alph,struct string_hash* tok,
							struct DLC_tree_info* DLC_tree,TokenizationPolicy tokenization_mode) {
int token_list[MAX_TOKEN_IN_A_COMPOUND_WORD];
tokenize_compound_word(word,token_list,alph,tok,tokenization_mode);
associate_pattern_to_compound_word(token_list,0,DLC_tree->root,pattern,DLC_tree);
}



/**
 * This function inserts 'pattern2' in the pattern list of 'node' if and
 * only if the liste contains 'pattern1'. It returns 1 on success, 0
 * otherwise.
 */
int conditional_pattern_insertion(struct DLC_tree_node* node,int pattern1,int pattern2) {
if (is_in_list(pattern1,node->patterns)) {
  add_pattern_to_DLC_tree_node(node,pattern2);
  return 1;
}
return 0;
}


/**
 * This function explores the compound word tree in order to associate
 * the pattern number 'pattern2' to a compound word if and only if this
 * has already the pattern number 'pattern1' in its pattern list.
 * If the compound word is not in the tree, or if it has not 'pattern1' in its
 * pattern list, the function fails and returns 0. Otherwise, 'pattern2' is added
 * to the pattern list and the function returns a non-zero value.
 *
 *
 * 'token_list' is an array representing the sequence of tokens that
 * constitute the compound word. This array has been produced by the
 * function 'tokenize_compound_word'.
 *
 * 'pos' is the current position in the array 'token_list'.
 *
 * 'node' is the current node in the tree.
 *
 * 'pattern1' is the pattern number that must be in the pattern list of 'node'.

 * 'pattern2' is the pattern number to add.
 *
 * 'DLC_tree' represents the tree and the compound word index.
 */
int conditional_insertion_in_DLC_tree_node(int* token_list,int pos,struct DLC_tree_node* node,
										int pattern1,int pattern2) {
if (token_list[pos]==END_TOKEN_LIST) {
   /* If we are at the end of the token list */
   return conditional_pattern_insertion(node,pattern1,pattern2);
}
int int_sequence[256];
if (token_list[pos]!=BEGIN_CASE_VARIANT_LIST) {
   /* If the token is a single token, then we put it into the token
    * sequence */
   int_sequence[0]=token_list[pos];
   int_sequence[1]=-1;
   pos++;
} else {
   /* If we have a token sequence, we build it */
   int j=0;
   do {
      pos++;
      int_sequence[j++]=token_list[pos];
   } while (token_list[pos]!=END_CASE_VARIANT_LIST);
   int_sequence[j]=-1;
   pos++;
   /* And we sort it */
   quicksort(int_sequence,j);
}
/* Then, we look for the node that we can reach with our int sequence */
struct DLC_tree_node* ptr=get_DLC_tree_node(node,int_sequence,0);
if (ptr!=NULL) return conditional_insertion_in_DLC_tree_node(token_list,pos,ptr,pattern1,pattern2);
return 0;
}


/**
 * This function associates the pattern number 'pattern2' to the word 'word'
 * in the compound word tree 'DLC_tree', if and only if:
 *   1) 'word' is already in the tree
 *   2) 'pattern1' is in the pattern list associated to 'word'
 * The function returns 0 if one of these conditions is not verified; otherwise
 * it inserts 'pattern2' in the pattern list associated to 'word' and returns a
 * non-zero value.
 */
int conditional_insertion_in_DLC_tree(unichar* word,int pattern1,int pattern2,Alphabet* alph,
						struct string_hash* tok,struct DLC_tree_info* infos,TokenizationPolicy tokenization_mode) {
int token_list[MAX_TOKEN_IN_A_COMPOUND_WORD];
tokenize_compound_word(word,token_list,alph,tok,tokenization_mode);
return conditional_insertion_in_DLC_tree_node(token_list,0,infos->root,pattern1,pattern2);
}


/**
 * This function takes a DLC tree node and returns the number
 * of transitions that there is actually: for each outgoing
 * transition, we count the tokens in the transition's token
 * sequence.
 */
int count_actual_transitions(struct DLC_tree_node* node) {
if (node==NULL) {
   fatal_error("NULL error in count_actual_transitions");
}
int res=0;
DLC_tree_transition* t=node->transitions;
while (t!=NULL) {
   int* token_sequence=t->token_sequence;
   for (int i=0;token_sequence[i]!=-1;i++) {
      res++;
   }
   t=t->next;
}
return res;
}


/**
 * This function optimizes the given compound word tree node.
 * The pattern list is emptied and replaced by a sorted array.
 * The transition list is also emptied and replaced by 2 sorted
 * arrays that correspond to the 'token' and 'node' fields of the
 * DLC_tree_transition structure.
 */
void optimize_DLC_node(struct DLC_tree_node* n) {
struct list_int* tmp;
struct DLC_tree_transition* t;
int i;
if (n==NULL) return;
if (n->number_of_patterns!=0) {
   /* We allocate the array for pattern numbers and we fill it */
   n->array_of_patterns=(int*)malloc(sizeof(int)*n->number_of_patterns);
   if (n->array_of_patterns==NULL) {
      fatal_alloc_error("optimize_DLC_node");
   }
   i=0;
   while (n->patterns!=NULL) {
     n->array_of_patterns[i++]=n->patterns->n;
     tmp=n->patterns;
     n->patterns=n->patterns->next;
     free(tmp);
   }
}
n->number_of_transitions=count_actual_transitions(n);
if (n->transitions!=NULL) {
   /* We allocate the arrays for representing (token,node) pairs of
    * transitions and fill them */
   n->destination_tokens=(int*)malloc(sizeof(int)*n->number_of_transitions);
   if (n->destination_tokens==NULL) {
      fatal_alloc_error("optimize_DLC_node");
   }
   n->destination_nodes=(struct DLC_tree_node**)malloc(sizeof(struct DLC_tree_node*)*n->number_of_transitions);
   if (n->destination_nodes==NULL) {
      fatal_alloc_error("optimize_DLC_node");
   }
   n->duplicates = (short*)malloc(sizeof(short) * n->number_of_transitions);
   i=0;
   while (n->transitions!=NULL) {
     /* Recursively, we optimize the destination node */
     optimize_DLC_node(n->transitions->node);
     int* token_sequence=n->transitions->token_sequence;
     for (int j=0;token_sequence[j]!=-1;j++) {
    	 if (j > 0)
    		 n->duplicates[i] = 1;
    	 else
    		 n->duplicates[i] = 0;
        n->destination_nodes[i]=n->transitions->node;
        n->destination_tokens[i]=token_sequence[j];
        i++;
     }
     t=n->transitions;
     n->transitions=n->transitions->next;
     /* WARNING: don't call free_DLC_tree_transitions(t), because it would
      *          free the destination node that is still used. */
     free(t->token_sequence);
     free(t);
   }
   /* Finally, we sort the n->destination_nodes and n->destination_tokens
    * arrays according to the tokens' numbers. */
   quicksort2(n->destination_tokens,(void**)(n->destination_nodes),n->number_of_transitions);
}
}


/**
 * This function optimizes the given compound word tree, by replacing
 * all transition and pattern lists by sorted arrays.
 */
void optimize_DLC(struct DLC_tree_info* DLC_tree) {
optimize_DLC_node(DLC_tree->root);
}


/**
 * Builds a quicksort partition of the given array.
 */
int partition(int start,int end,int* array) {
int pivot;
int tmp;
int i=start-1;
/* Final pivot index */
int j=end+1;
pivot=array[(start+end)/2];
while (true) {
   do j--;
   while ((j>(start-1)) && array[j]>pivot);
   do i++;
   while ((i<end+1) && array[i]<pivot);
   if (i<j) {
      tmp=array[i];
      array[i]=array[j];
      array[j]=tmp;
   } else return j;
}
}


/**
 * Sorts the given int array.
 */
void quicksort(int start,int end,int* array) {
int p;
if (start<end) {
   p=partition(start,end,array);
   quicksort(start,p,array);
   quicksort(p+1,end,array);
}
}


/**
 * Sorts the given int array.
 */
void quicksort(int* array,int size) {
quicksort(0,size-1,array);
}


/**
 * Builds a quicksort partition of the given arrays.
 */
int partition2(int start,int end,int* array1,void** array2) {
int pivot;
int tmp;
int i=start-1;
/* Final pivot index */
int j=end+1;
pivot=array1[(start+end)/2];
void* tmp2;
while (true) {
   do j--;
   while ((j>(start-1)) && array1[j]>pivot);
   do i++;
   while ((i<end+1) && array1[i]<pivot);
   if (i<j) {
      /* We perform the pivot test on the first array, bu we
       * apply the element permutations to both. */
      tmp=array1[i];
      array1[i]=array1[j];
      array1[j]=tmp;
      tmp2=array2[i];
      array2[i]=array2[j];
      array2[j]=tmp2;
   } else return j;
}
}


/**
 * Sorts the given int arrays.
 */
void quicksort2(int start,int end,int* array1,void** array2) {
int p;
if (start<end) {
   p=partition2(start,end,array1,array2);
   quicksort2(start,p,array1,array2);
   quicksort2(p+1,end,array1,array2);
}
}


/**
 * Sorts the given int arrays. The sort is done according to the values of
 * 'array1', but the element permutations are done on both arrays.
 */
void quicksort2(int* array1,void** array2,int size) {
quicksort2(0,size-1,array1,array2);
}


/**
 * Allocates and returns a copy of the given IntSequence.
 */
IntSequence clone_IntSequence(IntSequence src) {
if (src==NULL) return NULL;
int l;
for (l=0;src[l]!=-1;l++) {}
IntSequence dst=(IntSequence)malloc((l+1)*sizeof(int));
if (dst==NULL) {
   fatal_alloc_error("clone_IntSequence");
}
l=0;
while ((dst[l]=src[l])!=-1) l++;
return dst;
}


/**
 * Compares two IntSequence according to the lexicographic order
 * and returns:
 * - 0 if a==b
 * - a value <0 if a<b
 * - a value >0 if a>b
 *
 * Raises a fatal error if a sequence is NULL.
 */
int compare_IntSequence(IntSequence a,IntSequence b) {
if (a==NULL || b==NULL) {
   fatal_error("NULL error in compare_IntSequence\n");
}
register const int *a_p = a;
register const int *b_p = b;
register int a_c;
register int b_c;
do {
   a_c=(int)*a_p++;
   b_c=(int)*b_p++;
   if (a_c==-1) return a_c-b_c;
} while (a_c==b_c);
return a_c - b_c;
}

