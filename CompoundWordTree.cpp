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

#include "LocatePattern.h"
#include "CompoundWordTree.h"
#include "Error.h"


void free_DLC_tree_node(struct DLC_tree_node*);

/**
 * Allocates, initializes and returns a compound word tree node.
 */
struct DLC_tree_node* new_DLC_tree_node() {
struct DLC_tree_node* n;
n=(struct DLC_tree_node*)malloc(sizeof(struct DLC_tree_node));
if (n==NULL) {
	fatal_error("Not enough memory in new_DLC_tree_node\n");
}
n->patterns=NULL;
n->number_of_patterns=0;
n->array_of_patterns=NULL;
n->transitions=NULL;
n->number_of_transitions=0;
n->destination_tokens=NULL;
n->destination_nodes=NULL;
return n;
}


/**
 * Allocates, initializes and returns a compound word tree transition.
 * The default token number is -1.
 */
struct DLC_tree_transition* new_DLC_tree_transition() {
struct DLC_tree_transition* t;
t=(struct DLC_tree_transition*)malloc(sizeof(struct DLC_tree_transition));
if (t==NULL) {
	fatal_error("Not enough memory in new_DLC_tree_transition\n");
}
t->token=-1;
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
	fatal_error("Not enough memory in init_DLC_tree\n");
}
DLC_tree->root=new_DLC_tree_node();
DLC_tree->index=(struct DLC_tree_node**)malloc(number_of_tokens*sizeof(struct DLC_tree_node*));
if (DLC_tree->index==NULL) {
	fatal_error("Not enough memory in new_DLC_tree\n");
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
		free_DLC_tree_node(node->destination_nodes[i]);
	}
	free(node->destination_nodes);
}
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
void tokenize_compound_word(unichar* word,int tokens[],Alphabet* alph,struct string_hash* tok,
							int tokenization_mode,int SPACE) {
int i,c,n_token,j,k;
struct list_int* ptr;
unichar m[1024];
k=0;
n_token=0;
if (tokenization_mode==CHAR_BY_CHAR_TOKENIZATION) {
   /* If we must process the dictionary character by character */
   n_token=0;
   i=0;
   m[1]='\0';
   while (word[i]!='\0') {
      m[0]=word[i];
      j=get_value_index(m,tok,DONT_INSERT);
      if (j==-1) {
      	 /* If a token of a compound word is not a token of the text,
      	  * then we traduce it by an empty list. */
         tokens[n_token++]=BEGIN_CASE_VARIANT_LIST;
         tokens[n_token++]=END_CASE_VARIANT_LIST;
      }
      else tokens[n_token++]=j;
      i++;
   }
   tokens[n_token]=END_TOKEN_LIST;
   return;
}
/* If we are not in character by character mode */
while ((c=word[k])!='\0') {
  if (c==' ') {
              j=SPACE;
              /* If the text does not contain the token space,
               * then we traduce it by an empty list. */
              if (j==-1) {
                tokens[n_token++]=BEGIN_CASE_VARIANT_LIST;
                tokens[n_token++]=END_CASE_VARIANT_LIST;
              }
              else tokens[n_token++]=j;
              /* In case of several spaces, we consider them as just
               * one space. Note that this case should not happen. */
              while ((c=word[++k])==' ');
            }
    else if (is_letter((unichar)c,alph)) {
              /* If we have a letter, then we look for a word */
              i=0;
              do {
                m[i++]=(unichar)c;
                c=word[++k];
              }
              while (is_letter((unichar)c,alph));
              m[i]='\0';
              /* The test (n_token>0) is used to allow case variants on the first
               * token in any case. */
              if (n_token>0 && !ALL_CASE_VARIANTS_ARE_ALLOWED) {
                /* Here we compute no case variant, we only look for the exact
                 * matching token */
                j=get_value_index(m,tok,DONT_INSERT);
                if (j==-1) {
                  tokens[n_token++]=BEGIN_CASE_VARIANT_LIST;
                  tokens[n_token++]=END_CASE_VARIANT_LIST;
                }
                else tokens[n_token++]=j;
              }
              else {
                /* Here we compute all case variants */
                tokens[n_token++]=BEGIN_CASE_VARIANT_LIST;
                ptr=get_token_list_for_sequence(m,alph,tok);
                struct list_int* ptr_copy = ptr; // s.n.
                while (ptr!=NULL) {
                  j=ptr->n;
                  tokens[n_token++]=j;
                  ptr=ptr->next;
                }
                free_list_int(ptr_copy); // s.n.
                tokens[n_token++]=END_CASE_VARIANT_LIST;
              }
            }
    		else {
    		   /* If we have a non letter character, then we take
    		    * it as a token. */
               m[0]=(unichar)c;
               m[1]='\0';
               k++;
               j=get_value_index(m,tok,DONT_INSERT);
               /* If the text does not contain this token,
               * then we traduce it by an empty list. */
               if (j==-1) {
                  tokens[n_token++]=BEGIN_CASE_VARIANT_LIST;
                  tokens[n_token++]=END_CASE_VARIANT_LIST;
               } else tokens[n_token++]=j;
    		}
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
struct DLC_tree_node* get_DLC_tree_node(struct DLC_tree_node* node,int token,int create_if_necessary) {
struct DLC_tree_transition* l;
if (node->transitions==NULL) {
  /* If the list is empty */
  /* We return if the function must not create the node */
  if (!create_if_necessary) return NULL;
  /* Otherwise, we create a transition */
  l=new_DLC_tree_transition();
  l->token=token;
  l->next=node->transitions;
  /* And we create the destination node of this transition */
  l->node=new_DLC_tree_node();
  node->transitions=l;
  (node->number_of_transitions)++;
  /* Finally we return the created node */
  return l->node;
}
if (node->transitions->token==token)
  /* If the head of the list is the one we look for, we return it */
  return node->transitions->node;
if (node->transitions->token>token) {
  /* If we must insert at the beginning of the list */
  /* We return if the function must not create the node */
  if (!create_if_necessary) return NULL;
  /* Otherwise, we create a transition */
  l=new_DLC_tree_transition();
  l->token=token;
  l->next=node->transitions;
  /* And we create the destination node of this transition */
  l->node=new_DLC_tree_node();
  node->transitions=l;
  (node->number_of_transitions)++;
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
	if (previous->next->token==token) return previous->next->node;
	else if (previous->next->token<token) previous=previous->next;
	else stop=1;
}
/* We return if the function must not create the node */
if (!create_if_necessary) return NULL;
/* Otherwise, we create a transition */
l=new_DLC_tree_transition();
l->token=token;
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
struct DLC_tree_node* ptr;
int next_token,first_token;
if (token_list[pos]==END_TOKEN_LIST) {
  /* If we are at the end of the token list, we
   * add the pattern number to the current node */
  add_pattern_to_DLC_tree_node(node,pattern);
  return;
}
first_token=(pos==0);
if (token_list[pos]!=BEGIN_CASE_VARIANT_LIST) {
	/* If the token is a single token, then we get the corresponding node */
	ptr=get_DLC_tree_node(node,token_list[pos],1);
	if (first_token) {
		/* If the token is the first of the list, then we update the
		 * index with it */
		DLC_tree->index[token_list[pos]]=ptr;
	}
	/* And we add the pattern number to this node */
	associate_pattern_to_compound_word(token_list,pos+1,ptr,pattern,DLC_tree);
	return;
}
/* If we have a token list instead of a single token,
 * we look for the index of the next token, that is to say
 * to position that follows the end of the token list */
next_token=pos;
do {
	next_token++;
} while (token_list[next_token]!=END_CASE_VARIANT_LIST);
next_token++;
/* Then we parse the token list */
pos++;
while (token_list[pos]!=END_CASE_VARIANT_LIST)  {
	/* We get the node that correspond to the single token (nested token 
	 * lists are forbidden) */
	ptr=get_DLC_tree_node(node,token_list[pos],1);
	if (first_token) {
		/* If the token is the first of the list, then we update the
		 * index with it */
		DLC_tree->index[token_list[pos]]=ptr;
	}
	/* And we add the pattern number to this node */
	associate_pattern_to_compound_word(token_list,next_token,ptr,pattern,DLC_tree);
	pos++;
}
}


/**
 * Adds a compound word to the tree 'DLC_tree' with the value
 * COMPOUND_WORD_PATTERN. This is used when the user looks
 * for any compound word, regardless the pattern, with <DIC> or <CDIC>.
 */
void add_compound_word_with_no_pattern(unichar* word,Alphabet* alph,struct string_hash* tok,
							struct DLC_tree_info* DLC_tree,int tokenization_mode,int SPACE) {
add_compound_word_with_pattern(word,COMPOUND_WORD_PATTERN,alph,tok,DLC_tree,
							tokenization_mode,SPACE);
}


/**
 * Adds a compound word to the tree 'DLC_tree' with the pattern number
 * 'pattern'.
 */
void add_compound_word_with_pattern(unichar* word,int pattern,Alphabet* alph,struct string_hash* tok,
							struct DLC_tree_info* DLC_tree,int tokenization_mode,int SPACE) {
int token_list[MAX_TOKEN_IN_A_COMPOUND_WORD];
tokenize_compound_word(word,token_list,alph,tok,tokenization_mode,SPACE);
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
struct DLC_tree_node* ptr;
int next_token,result;
result=0;
if (token_list[pos]==END_TOKEN_LIST) {
	/* If we are at the end of the token list */
	return conditional_pattern_insertion(node,pattern1,pattern2);
}
if (token_list[pos]!=BEGIN_CASE_VARIANT_LIST) {
	/* If the token is a single token, we get the corresponding node,
	 * without creating it if it does not exist */
  ptr=get_DLC_tree_node(node,token_list[pos],0);
  /* If the node exists, we go on recursively, otherwise we fail */
  if (ptr!=NULL) return conditional_insertion_in_DLC_tree_node(token_list,pos+1,ptr,pattern1,pattern2);
  else return 0;
}
/* If we have a token list instead of a single token,
 * we look for the index of the next token, that is to say
 * to position that follows the end of the token list */
next_token=pos;
do {
	next_token++;
} while (token_list[next_token]!=END_CASE_VARIANT_LIST);
next_token++;
/* Then we parse the token list */
pos++;
while (token_list[pos]!=END_CASE_VARIANT_LIST)  {
	/* We get the node that correspond to the single token (nested token 
	 * lists are forbidden). Note that we do not create the node if it does
	 * not exist */
	ptr=get_DLC_tree_node(node,token_list[pos],0);
	/* If the node exist, we go on recursively. If at least one token leads
	 * to a success, then the global result will be a success. */
	if (ptr!=NULL) result=result+conditional_insertion_in_DLC_tree_node(token_list,next_token,ptr,pattern1,pattern2);
	pos++;
}
return result;
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
 #warning reorganize parameters
int conditional_insertion_in_DLC_tree(unichar* word,int pattern1,int pattern2,Alphabet* alph,
						struct string_hash* tok,struct DLC_tree_info* infos,int tokenization_mode,
                  int SPACE) {
int token_list[MAX_TOKEN_IN_A_COMPOUND_WORD];
tokenize_compound_word(word,token_list,alph,tok,tokenization_mode,SPACE);
return conditional_insertion_in_DLC_tree_node(token_list,0,infos->root,pattern1,pattern2);
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
   i=0;
   while (n->patterns!=NULL) {
     n->array_of_patterns[i++]=n->patterns->n;
     tmp=n->patterns;
     n->patterns=n->patterns->next;
     free(tmp);
   }
}
if (n->number_of_transitions!=0) {
   /* We allocate the arrays for representing (token,node) pairs of
    * transitions and fill them */
   n->destination_tokens=(int*)malloc(sizeof(int)*n->number_of_transitions);
   n->destination_nodes=(struct DLC_tree_node**)malloc(sizeof(struct DLC_tree_node*)*n->number_of_transitions);
   i=0;
   while (n->transitions!=NULL) {
     n->destination_tokens[i]=n->transitions->token;
     n->destination_nodes[i]=n->transitions->node;
     t=n->transitions;
     n->transitions=n->transitions->next;
     /* Recursively, we optimize the destination node */
     optimize_DLC_node(n->destination_nodes[i]);
     i++;
     free(t);
   }
}
}


/**
 * This function optimizes the given compound word tree, by replacing
 * all transition and pattern lists by sorted arrays.
 */
void optimize_DLC(struct DLC_tree_info* DLC_tree) {
optimize_DLC_node(DLC_tree->root);
}

