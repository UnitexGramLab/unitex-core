 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "TextAutomaton.h"
#include "StringParsing.h"
#include "List_ustring.h"
#include "Error.h"
#include "SingleGraph.h"
#include "DELA.h"
#include "NormalizationFst2.h"
#include "BitMasks.h"
#include "Transitions.h"


/**
 * This is an internal structure only used to give a set of parameters to some functions.
 */
struct info {
   struct text_tokens* tok;
   int* buffer;
   Alphabet* alph;
   int SPACE;
   int length_max;
};



/**
 * This function returns the number of space tokens that are in the
 * given buffer.
 */
int count_non_space_tokens(int* buffer,int length,int SPACE) {
int n=0;
for (int i=0;i<length;i++) {
   if (buffer[i]!=SPACE) {
      n++;
   }
}
return n;
}
 

/**
 * This function explores in parallel a text token 'token' and the dictionary tree
 * 'tree'. 'n' is the current node in this tree. 'pos' is the current position in 
 * 'token'. 'inflected' is the inflected form as in the text, i.e. with the same case.
 * It's not the same than 'token' because it can be a composed inflected form. 
 * 'pos_inflected' is the current position in this string. 'state' is the current
 * state in the sentence automaton we are building, so, if we find transitions to
 * add, we will add them from it. 'shift' represents the number of non space tokens
 * in 'inflected' and 'start_node_index' represents the index of the current
 * state. We use it to guess the number of the state that must be pointed out
 * by transitions. For instance, if the first state is 6 and if we have to add
 * a transition tagged with the inflected form "black-eyed", we know that the state
 * to reach will be 6+3=9. We also use 'shift' to know if we are dealing with the first
 * token of the inflected form. This is useful to determine if the first token is
 * an unknown token or not. 'current_token_index' is the position of the current token
 * in the token buffer. 'tmp_tags' represents the tags of the current graph.
 */
void explore_dictionary_tree(int pos,unichar* token,unichar* inflected,int pos_inflected,
                             struct string_hash_tree_node* n,struct DELA_tree* tree,
                             struct info* INFO,SingleGraphState state,int shift,
                             int start_state_index,int *is_not_unknown_token,
                             int current_token_index,struct string_hash* tmp_tags) {
if (token[pos]=='\0') {
   if (shift==1 && n->value_index!=-1) {
      /* If we are on the first token and if there are some DELA entries for it, 
       * then it is not an unknown one */
      (*is_not_unknown_token)=1;
   }
   struct dela_entry_list* list=NULL;
   if (n->value_index!=-1) list=tree->dela_entries[n->value_index];
   inflected[pos_inflected]='\0';
   while (list!=NULL) {
      /* If there are DELA entries for the current inflected form,
       * we add the corresponding transitions to the automaton */
      unichar tag[4096];
      build_tag(list->entry,inflected,tag);
      add_outgoing_transition(state,get_value_index(tag,tmp_tags),start_state_index+shift);
      list=list->next;
   }
   /* We try to go on with the next token in the sentence */
   if (current_token_index<INFO->length_max-1) {
      /* but only if we are not at the end of the sentence */
      if (INFO->buffer[current_token_index]!=INFO->SPACE) {
         /* The states do not take spaces into acccount, so, if we have read
          * a space, we do nothing; otherwise, we can move one state right */
         shift++;
      }
      current_token_index++;
      explore_dictionary_tree(0,INFO->tok->token[INFO->buffer[current_token_index]],
                              inflected,pos_inflected,n,tree,INFO,state,shift,
                              start_state_index,is_not_unknown_token,current_token_index,tmp_tags);
   }
   return;
}
struct string_hash_tree_transition* trans=n->trans;
inflected[pos_inflected]=token[pos];
while (trans!=NULL) {
   if (is_equal_or_uppercase(trans->letter,token[pos],INFO->alph)) {
      /* For each transition, we follow it if its letter matches the
       * token one */
      explore_dictionary_tree(pos+1,token,inflected,pos_inflected+1,trans->node,tree,INFO,state,
                              shift,start_state_index,is_not_unknown_token,current_token_index,
                              tmp_tags);
   }
   trans=trans->next;
}
}


/**
 * This function explores the given graph, trying to find for each state the path with
 * the lowest number of unknown tokens made of letters that leads to it. These
 * numbers of tokens are stored in 'weight'.
 */
void compute_best_paths(int state,SingleGraph graph,int* weight,struct string_hash* tmp_tags) {
int w;
w=weight[state];
Transition* trans=graph->states[state]->outgoing_transitions;
while (trans!=NULL) {
   unichar* tmp=tmp_tags->value[trans->tag_number];
   int w_tmp=w;
   if (tmp[0]!='{' && u_is_letter(tmp[0])) {
      /* If the transition is tagged by an unknown token made of letters */
      w_tmp=w_tmp+1;
   }
   if (w_tmp < weight[trans->state_number]) {
      /* If we have a better path than the existing one, we keep it */
      weight[trans->state_number]=w_tmp;
      /* and we mark the destination state as modified */
      set_bit_mask(&(graph->states[trans->state_number]->control),MARK1_BIT_MASK);
   }
   trans=trans->next;
}
trans=graph->states[state]->outgoing_transitions;
while (trans!=NULL) {
   if (is_bit_mask_set(graph->states[trans->state_number]->control,MARK1_BIT_MASK)) {
      /* If the state was modified, we reset its control value */
      unset_bit_mask(&(graph->states[trans->state_number]->control),MARK1_BIT_MASK);
      /* and we explore it recursively */
      compute_best_paths(trans->state_number,graph,weight,tmp_tags);
   }
   trans=trans->next;
}
}


/**
 * Before this function is called, for each state i, weight[i] must have been
 * set with the minimal weight of the state #i, that is to say the minimal number
 * of transitions tagged with unknown tokens made of letters to follow to reach
 * this state. Then, this function takes a list of transitions and removes all
 * those that reach a state #x with a weight > weight[x].
 */
Transition* remove_bad_path_transitions(int min_weight,Transition* trans,
                                        SingleGraph graph,int* weight,
                                        struct string_hash* tmp_tags) {
if (trans==NULL) return NULL;
unichar* s=tmp_tags->value[trans->tag_number];
if ((s[0]!='{' && u_is_letter(s[0]) && weight[trans->state_number] < (min_weight+1))
    || (weight[trans->state_number] < min_weight )) {
   /* If we have to remove the transition */
   Transition* tmp=trans->next;
   free(trans);
   return remove_bad_path_transitions(min_weight,tmp,graph,weight,tmp_tags);
}
trans->next=remove_bad_path_transitions(min_weight,trans->next,graph,weight,tmp_tags);
return trans;
}


/**
 * This function applies the following heuristic to remove paths:
 * if there are several paths to go from A to B, then we will keep those
 * that are tagged with the smallest number of unknown token made of letters.
 * For instance, if we have the 2 concurrent paths:
 * 
 *   Aujourd ' {hui,huir.V:Kms}
 *   {Aujourd'hui,aujourd'hui.ADV+z1}
 * 
 * we will remove the first one because it contains 1 unkown token made of letters
 * ("Aujourd") while the second contains none.
 */
void keep_best_paths(SingleGraph graph,struct string_hash* tmp_tags) {
int i;
int* weight=(int*)malloc(sizeof(int)*graph->number_of_states);
if (weight==NULL) {
   fatal_error("Not enough memory in keep_best_paths\n");
}
/* We initialize the initial state with 0 untagged transition */
weight[0]=0;
/* and all other states with a value that is larger that the
 * longest possible path. As the automaton is acyclic, the longest
 * path cannot be greater than the number of states */
for (i=1;i<graph->number_of_states;i++) {
   weight[i]=graph->number_of_states;
}
compute_best_paths(0,graph,weight,tmp_tags);
for (i=0;i<graph->number_of_states;i++) {
   graph->states[i]->outgoing_transitions=remove_bad_path_transitions(weight[i],
                                              graph->states[i]->outgoing_transitions,graph,weight,
                                              tmp_tags);
}
free(weight);
}


/**
 * This function takes an output sequence 's' and adds all the corresponding
 * transitions in the given graph. For instance, if we have to add a path
 * from the state 4 to 7 corresponding to the sequence "{de,.PREP} {le,.PRO:ms}",
 * we will create a new intermediate state, for instance 19, and add two transitions:
 * 
 *   4 -- {de,.PREP} ----> 19
 *  19 -- {le,.PRO:ms} --> 7
 */
void add_path_to_sentence_automaton(int start_state_index,Alphabet* alph,
                                    SingleGraph graph,struct string_hash* tmp_tags,
                                    unichar* s,int destination_state_index) {
struct list_ustring* l=tokenize_normalization_output(s,alph);
if (l==NULL) {
   /* If the output to be generated have no interest, we do nothing */
   return;
}
struct list_ustring* tmp;
int current_state=start_state_index;
while (l!=NULL) {
   if (l->next==NULL) {
      /* If this is the last transition to create, we make it point to the
       * destination state */
      add_outgoing_transition(graph->states[current_state],get_value_index(l->string,tmp_tags),destination_state_index);
   }
   else {
      /* If this transition is not the last one, we must create a new state */
      add_outgoing_transition(graph->states[current_state],get_value_index(l->string,tmp_tags),graph->number_of_states);
      current_state=graph->number_of_states;
      add_state(graph);
   }
   tmp=l;
   l=l->next;
   free_list_ustring_element(tmp);
}
}


/**
 * This function explores in parallel the text and the normalization tree.
 * If a sequence matches, then we add the transitions corresponding to the
 * normalized sequences. For instance, if we have a rule of the form:
 * 
 * j' => {je,.PRO:1s}
 * 
 * we will add a transition by "{je,.PRO:1s}" if we find "j'" in the text. This
 * is not the same than the exploration of the dictionary since here we ignore
 * the text sequence ("j'" is not taken into account for building the tag).
 */
void explore_normalization_tree(int pos_in_buffer,int token,struct info* INFO,
                                SingleGraph graph,struct string_hash* tmp_tags,
                                struct normalization_tree* norm_tree_node,
                                int first_state_index,int shift) {
struct list_ustring* outputs=norm_tree_node->outputs;
while (outputs!=NULL) {
   /* If there are outputs, we add paths in the text automaton */
   add_path_to_sentence_automaton(first_state_index,INFO->alph,graph,tmp_tags,
                                  outputs->string,first_state_index+shift-1);
   outputs=outputs->next;
}
/* Then, we explore the transitions from this node. Note that transitions
 * are tagged with token numbers that are unique. So, at most one transition
 * can match. */
struct normalization_tree_transition* trans=norm_tree_node->trans;
while (trans!=NULL) {
   if (trans->token==token) {
      /* If we have a transition for the current token */
      int i=0;
      if (token!=INFO->SPACE) {
         /* We must ignore spaces to get the correct state index */
         i=1;
      }
      explore_normalization_tree(pos_in_buffer+1,INFO->buffer[pos_in_buffer+1],
                                 INFO,graph,tmp_tags,trans->node,
                                 first_state_index,shift+i);
      /* As there can be only one matching transition, we exit the while */
      trans=NULL;
   }
   else {
      trans=trans->next;
   }
}
}


/**
 * This function builds the sentence automaton that correspond to the
 * given token buffer. It saves it into the given file.
 */
void build_sentence_automaton(int* buffer,int length,struct text_tokens* tokens,
                               struct DELA_tree* DELA_tree,struct string_hash* tags,
                               Alphabet* alph,FILE* out,int sentence_number,
                               int we_must_clean,
                               struct normalization_tree* norm_tree) {
/* We declare the graph that will represent the sentence as well as
 * a temporary string_hash that will be used to store the tags of this
 * graph. We don't put tags directly in the main tags, because a tag can
 * be introduced and then removed by cleaning */
SingleGraph graph=new_SingleGraph();
struct string_hash* tmp_tags=new_string_hash(32);
get_value_index(tags->value[0],tmp_tags);
int i;
/* We add +1 for the final node */
int n_nodes=1+count_non_space_tokens(buffer,length,tokens->SPACE);
for (i=0;i<n_nodes;i++) {
   add_state(graph);
}
set_initial_state(graph->states[0]);
set_final_state(graph->states[n_nodes-1]);
struct info INFO;
INFO.tok=tokens;
INFO.buffer=buffer;
INFO.alph=alph;
INFO.SPACE=tokens->SPACE;
INFO.length_max=length;
int current_state=0;
int is_not_unknown_token;
unichar inflected[4096];
for (i=0;i<length;i++) {
   if (buffer[i]!=tokens->SPACE && buffer[i]!=tokens->SENTENCE_MARKER) {
      /* We try to produce every transition from the current token */
      is_not_unknown_token=0;
      explore_dictionary_tree(0,tokens->token[buffer[i]],inflected,0,DELA_tree->inflected_forms->root,DELA_tree,&INFO,graph->states[current_state],1,
                              current_state,&is_not_unknown_token,i,tmp_tags);
      if (norm_tree!=NULL) {
         /* If there is a normalization tree, we explore it */
         explore_normalization_tree(i,buffer[i],&INFO,graph,tmp_tags,norm_tree,current_state,1);
      }
      if (!is_not_unknown_token) {
         /* If the token was not matched in the dictionary, we put it as an unknown one */
         int tag_number=get_value_index(tokens->token[buffer[i]],tmp_tags);
         add_outgoing_transition(graph->states[current_state],tag_number,current_state+1);
      }
      current_state++;
   }
}
if (we_must_clean) {
   /* If necessary, we apply the "good paths" heuristic */
   keep_best_paths(graph,tmp_tags);
}
/* We minimize the sentence automaton. It will remove the unused states and may
 * factorize suffixes introduced during the application of the normalization tree. */
minimize(graph,1);
/* Finally, we save the sentence automaton */
u_fprintf(out,"-%d ",sentence_number);
for (int z=0;z<length;z++) {
   u_fprintf(out,"%S",tokens->token[buffer[z]]);
}
u_fprintf(out,"\n");
for (i=0;i<graph->number_of_states;i++) {
   if (is_final_state(graph->states[i])) {
      u_fprintf(out,"t ");
   } else {
      u_fprintf(out,": ");
   }
   Transition* trans=graph->states[i]->outgoing_transitions;
   while (trans!=NULL) {
      /* For each tag of the graph that is actually used, we put it in the main
       * tags and we use this index in the fst2 transition */
      u_fprintf(out,"%d %d ",get_value_index(tmp_tags->value[trans->tag_number],tags),trans->state_number);
      trans=trans->next;
   }
   u_fprintf(out,"\n");
}
u_fprintf(out,"f \n");
free_SingleGraph(graph);
free_string_hash(tmp_tags);
}

