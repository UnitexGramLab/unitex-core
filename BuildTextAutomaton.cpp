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

#include "BuildTextAutomaton.h"
#include "StringParsing.h"
#include "List_ustring.h"
#include "Error.h"
#include "SingleGraph.h"
#include "DELA.h"
#include "NormalizationFst2.h"
#include "BitMasks.h"
#include "Transitions.h"
#include "SingleGraph.h"
#include "Vector.h"
#include "Tfst.h"
#include "ProgramInvoker.h"
#include "korean/Txt2Fst2Kr.h"
#include "korean/Syl2Jamo.h"
#include "korean/Jamo2Syl.h"
#include "Korean.h"
#include "File.h"
#include "AbstractFst2Load.h"

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
                             int first_token_index,int current_token_index,
                             struct string_hash* tmp_tags,Ustring* foo,
                             language_t* language) {
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
      struct dela_entry* entry=list->entry;
      if (language!=NULL) {
         entry=filter_dela_entry(list->entry,NULL,language,0);
      }
      if (entry!=NULL) {
         unichar tag[4096];
         build_tag(entry,inflected,tag);
         u_sprintf(foo,"@STD\n@%S\n@%d.0.0-%d.%d.0\n.\n",tag,first_token_index,current_token_index,pos-1);
         add_outgoing_transition(state,get_value_index(foo->str,tmp_tags),start_state_index+shift);
         if (entry!=list->entry) {
            free_dela_entry(entry);
         }
      }
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
                              start_state_index,is_not_unknown_token,
                              first_token_index,current_token_index,tmp_tags,foo,language);
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
                              shift,start_state_index,is_not_unknown_token,
                              first_token_index,current_token_index,tmp_tags,foo,language);
   }
   trans=trans->next;
}
}


/**
 * Returns 1 if the given tag description seems to describe a {xxx,xxx.xxx} tag; 0 otherwise.
 */
int is_high_weight_tag(unichar* s) {
if (u_starts_with(s,"@<E>\n")) {
   /* If we have an EPSILON tag */
   fatal_error("Unexpected <E> tag in is_high_weight_tag\n");
   return 0;
}
if (u_starts_with(s,"@STD\n")) {
   if (s[6]=='{' && s[7]!='\n') {
      return 1;
   }
   return 0;
}
fatal_error("Unsupported tag type in is_high_weight_tag\n");
return 0;
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
   if (!is_high_weight_tag(tmp)) {
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
if ((!is_high_weight_tag(s) && weight[trans->state_number] < (min_weight+1))
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
   fatal_alloc_error("keep_best_paths");
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
 * This structure is used to compute the list of tags to be inserted in the text automaton,
 * on the base of information taken from either the normalization fst2 or the tags.ind file.
 */
struct output_info {
   /* The output to a appear in the .tfst */
   unichar* output;
   /* The content of the tag. If the tag is of the form {xxx,yyy.zzz},
    * it means xxx; otherwise it is the same than 'output'. */
   unichar* content;
   /* Bounds of the sequence in the sentence, given in the form (X,Y) (W,Z) where
    * X is the start position in tokens, Y is the position in char in this token,
    * W is the end position in tokens, Z is the position in char in this token. */
   int start_pos;
   int end_pos;
   int start_pos_char;
   int end_pos_char;
};


/**
 * Allocates, initializes and returns a struct output_info*
 */
struct output_info* new_output_info(unichar* tag) {
if (tag==NULL) {
   fatal_error("Invalid NULL tag in new_output_info\n");
}
struct output_info* x=(struct output_info*)malloc(sizeof(struct output_info));
if (x==NULL) {
   fatal_alloc_error("new_output_info");
}
x->output=u_strdup(tag);
if (tag[0]=='{' && tag[1]!='\0') {
   struct dela_entry* entry=tokenize_tag_token(tag);
   if (entry==NULL) {
      fatal_error("new_output_info: Invalid tag token %S\n",tag);
   }
   x->content=u_strdup(entry->inflected);
   free_dela_entry(entry);
} else {
   x->content=u_strdup(tag);
}
x->start_pos=-1;
x->end_pos=-1;
x->start_pos_char=-1;
x->end_pos_char=-1;
return x;
}


/**
 * Frees all the memory associated to the given struct output_info*
 */
void free_output_info(struct output_info* x) {
if (x==NULL) return;
free(x->output);
free(x->content);
free(x);
}


/**
 * This function takes an output s like " {de,.PREP} {le,.DET} "
 * and returns a vector containing description of the tags that must be produced:
 *
 * "{de,.PREP}" and "{le,.DET}"
 *
 * The vector contains struct output_info*
 */
vector_ptr* tokenize_normalization_output(unichar* s,Alphabet* alph) {
if (s==NULL) return NULL;
vector_ptr* result=new_vector_ptr(4);
unichar tmp[2048];
int i;
int j;
i=0;
while (s[i]!='\0') {
   while (s[i]==' ') {
      /* We ignore spaces */
      i++;
   }
   if (s[i]!='\0') {
      /* If we are not at the end of the string */
      if (s[i]=='{') {
         /* Case of a tag like "{de,.PREP}" */
         j=0;
         while (s[i]!='\0' && s[i]!='}') {
            tmp[j++]=s[i++];
         }
         if (s[i]!='\0') {
            /* The end of string is an error (no closing '}'), so we save the
             * tag only if it is a valid one */
            tmp[j]='}';
            tmp[j+1]='\0';
            /* We go on the next char */
            i++;
            vector_ptr_add(result,new_output_info(tmp));
         }
      }
      else
      if (is_letter(s[i],alph)) {
         /* Case of a letter sequence like "Rivoli" */
         j=0;
         while (is_letter(s[i],alph)) {
            tmp[j++]=s[i++];
         }
         tmp[j]='\0';
         /* We don't have to go on the next char, we are already on it */
         vector_ptr_add(result,new_output_info(tmp));
      }
      else {
         /* Case of a single non-space char like "-" */
         tmp[0]=s[i];
         tmp[1]='\0';
         /* We go on the next char of the string */
         i++;
         vector_ptr_add(result,new_output_info(tmp));
      }
   }
}
if (result->nbelems==0) {
   free_vector_ptr(result,(void (*)(void*))free_output_info);
   return NULL;
}
return result;
}


/**
 * This function does its best to compute the start/end values for each tag
 * to be produced.
 */
void solve_alignment_puzzle(vector_ptr* vector,int start,int end,struct info* INFO,Alphabet* alph) {
if (vector==NULL) {
   fatal_error("NULL vector in solve_alignment_puzzle\n");
}
if (vector->nbelems==0) {
   fatal_error("Empty vector in solve_alignment_puzzle\n");
}
struct output_info** tab=(struct output_info**)vector->tab;

if (vector->nbelems==1) {
   /* If there is only one tag, there is no puzzle to solve */
   tab[0]->start_pos=start;
   tab[0]->start_pos_char=0;
   tab[0]->end_pos=end;
   tab[0]->end_pos_char=u_strlen(INFO->tok->token[INFO->buffer[end]])-1;
   return;
}

/* We try to find an exact match (modulo case variations) between the text and the tags' content */
int current_token=start;
int current_tag=0;
int current_pos_in_char_in_token=0;
int current_pos_in_char_in_tag=0;
tab[current_tag]->start_pos=current_token;
tab[current_tag]->start_pos_char=0;
unichar* token=INFO->tok->token[INFO->buffer[current_token]];
for(;;) {
   if (tab[current_tag]->content[current_pos_in_char_in_tag]=='\0') {
      /* If we are at the end of a tag */
      tab[current_tag]->end_pos=current_token;
      tab[current_tag]->end_pos_char=current_pos_in_char_in_token-1;
      if (current_tag==vector->nbelems-1) {
         /* If we are at the end of the last tag, we must also be
          * at the end of the last token if we want to have a perfect alignment */
         if (current_token==end && token[current_pos_in_char_in_token]=='\0') {
            return;
         }
         break;
      }
      /* We are not at the last tag */
      current_tag++;
      tab[current_tag]->start_pos=current_token;
      tab[current_tag]->start_pos_char=current_pos_in_char_in_token;
      current_pos_in_char_in_tag=0;
      continue;
   }
   /* We are not at the end of a tag, but we can be at the end of the current token */
   if (token[current_pos_in_char_in_token]=='\0') {
      if (current_token==end) {
         /* If this is the last token, then we cannot have a perfect alignement */
         break;
      }
      current_token++;
      if (current_pos_in_char_in_tag==0) {
         /* If we change of token whereas we have not started to read the current tag,
          * we can say that the current tag starts on the current token */
         (tab[current_tag]->start_pos)++;
         tab[current_tag]->start_pos_char=0;
      }
      if (INFO->buffer[current_token]==INFO->SPACE && current_pos_in_char_in_tag==0) {
         /* If 1) the new token is a space and 2) we are at the beginning of a tag
          * (that, by construction, cannot start with a space), then we must skip this
          * space token */
         if (current_token==end) {
            /* If this space token was the last token, then we cannot have a perfect alignement */
            u_printf("current tag=%S/%d  current token=%S/%d\n",tab[current_tag]->content,
                      current_pos_in_char_in_tag,token,current_pos_in_char_in_token);
            break;
         }
         /* By construction, we cannot have 2 contiguous spaces, but we raise an error in
          * order not to forget that the day this rule will change */
         current_token++;
         /* See comment above */
         (tab[current_tag]->start_pos)++;
         tab[current_tag]->start_pos_char=0;
         if (INFO->buffer[current_token]==INFO->SPACE) {
            fatal_error("Contiguous spaces not handled in solve_alignment_puzzle\n");
         }
      }
      token=INFO->tok->token[INFO->buffer[current_token]];
      current_pos_in_char_in_token=0;
      continue;
   }
   /* We are neither at the end of the tag nor at the end of the token */
   if (!is_equal_ignore_case(tab[current_tag]->content[current_pos_in_char_in_tag],
                             token[current_pos_in_char_in_token],
                             alph)) {
      break;
   }
   current_pos_in_char_in_tag++;
   current_pos_in_char_in_token++;
}

/* Default case: all tags will have the same start/end bounds */
for (int i=0;i<vector->nbelems;i++) {
   tab[i]->start_pos=start;
   tab[i]->start_pos_char=0;
   tab[i]->end_pos=end;
   tab[i]->end_pos_char=u_strlen(INFO->tok->token[INFO->buffer[end]])-1;
}
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
void add_path_to_sentence_automaton(int start_pos,int end_pos,
                                    int start_state_index,Alphabet* alph,
                                    SingleGraph graph,struct string_hash* tmp_tags,
                                    unichar* s,int destination_state_index,Ustring* foo,
                                    struct info* INFO) {
vector_ptr* vector=tokenize_normalization_output(s,alph);
if (vector==NULL) {
   /* If the output to be generated has no interest, we do nothing */
   return;
}
solve_alignment_puzzle(vector,start_pos,end_pos,INFO,alph);
int current_state=start_state_index;
for (int i=0;i<vector->nbelems;i++) {
   struct output_info* info=(struct output_info*)(vector->tab[i]);
   u_sprintf(foo,"@STD\n@%S\n@%d.%d.0-%d.%d.0\n.\n",info->output,info->start_pos,info->start_pos_char,info->end_pos,info->end_pos_char);
   if (i==vector->nbelems-1) {
      /* If this is the last transition to create, we make it point to the
       * destination state */
      add_outgoing_transition(graph->states[current_state],get_value_index(foo->str,tmp_tags),destination_state_index);
   }
   else {
      /* If this transition is not the last one, we must create a new state */
      add_outgoing_transition(graph->states[current_state],get_value_index(foo->str,tmp_tags),graph->number_of_states);
      current_state=graph->number_of_states;
      add_state(graph);
   }
}
free_vector_ptr(vector,(void (*)(void*))free_output_info);
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
 *
 * WARNING: this function MUST NOT BE CALLED when the current token is a space.
 */
void explore_normalization_tree(int first_pos_in_buffer,int current_pos_in_buffer,
                                int token,struct info* INFO,
                                SingleGraph graph,struct string_hash* tmp_tags,
                                struct normalization_tree* norm_tree_node,
                                int first_state_index,int shift,Ustring* foo,
                                int increment,language_t* language) {
struct list_ustring* outputs=norm_tree_node->outputs;
while (outputs!=NULL) {
   /* If there are outputs, we add paths in the text automaton */
   add_path_to_sentence_automaton(first_pos_in_buffer,current_pos_in_buffer-increment,
                                  first_state_index,INFO->alph,graph,tmp_tags,
                                  outputs->string,first_state_index+shift-1,foo,INFO);
   outputs=outputs->next;
}
/* Then, we explore the transitions from this node. Note that transitions
 * are tagged with token numbers that are unique. So, at most one transition
 * can match. */
struct normalization_tree_transition* trans=norm_tree_node->trans;
while (trans!=NULL) {
   if (trans->token==token) {
      /* If we have a transition for the current token */
      int increment=1;
      if (INFO->buffer[current_pos_in_buffer+1]==INFO->SPACE) {
         increment++;
      }
      explore_normalization_tree(first_pos_in_buffer,current_pos_in_buffer+increment,
                                 INFO->buffer[current_pos_in_buffer+increment],
                                 INFO,graph,tmp_tags,trans->node,
                                 first_state_index,shift+1,foo,increment,language);
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
                               struct DELA_tree* DELA_tree,
                               Alphabet* alph,U_FILE* out_tfst,U_FILE* out_tind,
                               int sentence_number,
                               int we_must_clean,
                               struct normalization_tree* norm_tree,
                               struct match_list* *tag_list,
                               int current_global_position_in_tokens,
                               int current_global_position_in_chars,
                               language_t* language) {
/* We declare the graph that will represent the sentence as well as
 * a temporary string_hash 'tmp_tags' that will be used to store the tags of this
 * graph. We don't put tags directly in the main 'tags', because a tag can
 * be introduced and then removed by cleaning */
Tfst* tfst=new_Tfst(NULL,NULL,0);
tfst->current_sentence=sentence_number;
tfst->automaton=new_SingleGraph();
tfst->offset_in_tokens=current_global_position_in_tokens;
tfst->offset_in_chars=current_global_position_in_chars;

struct string_hash* tags=new_string_hash(32);
struct string_hash* tmp_tags=new_string_hash(32);
unichar EPSILON_TAG[]={'@','<','E','>','\n','.','\n','\0'};
/* The epsilon tag is always the first one */
get_value_index(EPSILON_TAG,tmp_tags);
get_value_index(EPSILON_TAG,tags);

int i;
/* We add +1 for the final node */
int n_nodes=1+count_non_space_tokens(buffer,length,tokens->SPACE);
for (i=0;i<n_nodes;i++) {
   add_state(tfst->automaton);
}
set_initial_state(tfst->automaton->states[0]);
set_final_state(tfst->automaton->states[n_nodes-1]);
struct info INFO;
INFO.tok=tokens;
INFO.buffer=buffer;
INFO.alph=alph;
INFO.SPACE=tokens->SPACE;
INFO.length_max=length;
int current_state=0;
int is_not_unknown_token;
unichar inflected[4096];
/* Temp string used to build tags with bound control */
Ustring* foo=new_Ustring(1);
/* We compute the text and tokens of the sentence */
tfst->tokens=new_vector_int(length);
tfst->token_sizes=new_vector_int(length);
for (int i=0;i<length;i++) {
   vector_int_add(tfst->tokens,buffer[i]);
   int l=u_strlen(tokens->token[buffer[i]]);
   vector_int_add(tfst->token_sizes,l);
   u_strcat(foo,tokens->token[buffer[i]],l);
}
tfst->text=u_strdup(foo->str);


for (i=0;i<length;i++) {
   if (buffer[i]==tokens->SENTENCE_MARKER) {
      fatal_error("build_sentence_automaton: unexpected {S} token\n");
   }
   if (buffer[i]!=tokens->SPACE) {
      /* We try to produce every transition from the current token */
      is_not_unknown_token=0;
      explore_dictionary_tree(0,tokens->token[buffer[i]],inflected,0,DELA_tree->inflected_forms->root,DELA_tree,&INFO,tfst->automaton->states[current_state],1,
                              current_state,&is_not_unknown_token,i,i,tmp_tags,foo,language);
      if (norm_tree!=NULL) {
         /* If there is a normalization tree, we explore it */
         explore_normalization_tree(i,i,buffer[i],&INFO,tfst->automaton,tmp_tags,norm_tree,current_state,1,foo,0,language);
      }
      if (!is_not_unknown_token) {
         /* If the token was not matched in the dictionary, we put it as an unknown one */
         u_sprintf(foo,"@STD\n@%S\n@%d.0.0-%d.%d.0\n.\n",tokens->token[buffer[i]],i,i,tfst->token_sizes->tab[i]-1);
         int tag_number=get_value_index(foo->str,tmp_tags);
         add_outgoing_transition(tfst->automaton->states[current_state],tag_number,current_state+1);
      }
      current_state++;
   }
}

/* Now, we insert the tag sequences found in the 'tags.ind' file, if any */
struct match_list* tmp;
while ((*tag_list)!=NULL && (*tag_list)->m.start_pos_in_token>=current_global_position_in_tokens
       && (*tag_list)->m.start_pos_in_token<=current_global_position_in_tokens+length) {
   if ((*tag_list)->m.end_pos_in_token>current_global_position_in_tokens+length) {
      /* If we have a tag sequence that overlap two sentences, we must ignore it */
      tmp=(*tag_list)->next;
      free_match_list_element((*tag_list));
      (*tag_list)=tmp;
      continue;
   }
   /* We compute the local bounds of the tag sequence */
   int start_index=(*tag_list)->m.start_pos_in_token-current_global_position_in_tokens;
   int end_index=(*tag_list)->m.end_pos_in_token-current_global_position_in_tokens;
   int start_pos_in_token=start_index;
   int end_pos_in_token=end_index;
   /* And we adjust them to our state indexes, because spaces
    * must be ignored */
   for (int i=start_index;i>=0;i--) {
      if (buffer[i]==tokens->SPACE) {
         start_index--;
      }
   }
   for (int i=end_index;i>=0;i--) {
      if (buffer[i]==tokens->SPACE) {
         end_index--;
      }
   }
   add_path_to_sentence_automaton(start_pos_in_token,end_pos_in_token,start_index,
                                  INFO.alph,tfst->automaton,tmp_tags,
                                  (*tag_list)->output,end_index+1,foo,&INFO);
   tmp=(*tag_list)->next;
   free_match_list_element((*tag_list));
   (*tag_list)=tmp;
}

if (we_must_clean) {
   /* If necessary, we apply the "good paths" heuristic */
   keep_best_paths(tfst->automaton,tmp_tags);
}
trim(tfst->automaton);
if (tfst->automaton->number_of_states==0) {
   /* Case 1: the automaton has been emptied because of the tagset filtering */
   error("Sentence %d is empty\n",tfst->current_sentence);
   SingleGraphState initial=add_state(tfst->automaton);
   set_initial_state(initial);
   free_vector_ptr(tfst->tags,(void (*)(void*))free_TfstTag);
   tfst->tags=new_vector_ptr(1);
   vector_ptr_add(tfst->tags,new_TfstTag(T_EPSILON));
   save_current_sentence(tfst,out_tfst,out_tind,NULL,0);
} else {
   /* Case 2: the automaton is not empty */

   /* We minimize the sentence automaton. It will remove the unused states and may
    * factorize suffixes introduced during the application of the normalization tree. */
   minimize(tfst->automaton,1);
   /* We explore all the transitions of the automaton in order to renumber transitions */
   for (i=0;i<tfst->automaton->number_of_states;i++) {
      Transition* trans=tfst->automaton->states[i]->outgoing_transitions;
      while (trans!=NULL) {
         /* For each tag of the graph that is actually used, we put it in the main
          * tags and we use this index in the tfst transition */
         trans->tag_number=get_value_index(tmp_tags->value[trans->tag_number],tags);
         trans=trans->next;
      }
   }
   save_current_sentence(tfst,out_tfst,out_tind,tags->value,tags->size);
}
close_text_automaton(tfst);
free_string_hash(tmp_tags);
free_string_hash(tags);
free_Ustring(foo);
}


/**
 * Loads a whole file as a string. This is used to load the content
 * of the current sentence. 
 */
unichar* load_as_a_string(char* name) {
U_FILE* f=u_fopen(UTF16_LE,name,U_READ);
if (f==NULL) {
   fatal_error("Cannot open %s in load_as_a_string\n",name);
}
long size=get_file_size(f);
unichar* res=(unichar*)malloc(sizeof(unichar)*size/2);
if (res==NULL) {
   fatal_alloc_error("load_as_a_string");
}
u_fgets(res,f);
u_fclose(f);
return res;
}


/**
 * This function stores in dest:
 * - a copy of src if src is not a dictionary tag
 * - its inflected form otherwise
 */
void get_tag_content(unichar* src,unichar* dest) {
if (src[0]!='{' || src[1]=='\0') {
   u_strcpy(dest,src);
   return;
}
struct dela_entry* e=tokenize_tag_token(src);
if (e==NULL) {
   fatal_error("Invalid tag in get_tag_content: %S\n",src);
}
u_strcpy(dest,e->inflected);
free_dela_entry(e);
}


int token_contains_Hangul_or_Chinese_chars(unichar* s,Alphabet* a) {
for (int i=0;s[i]!='\0';i++) {
   if (u_is_korea_syllabe_letter(s[i]) 
         || a->korean_equivalent_syllab[s[i]]!=0
         || (s[i]!=0x318D && u_is_Hangul_Compatility_Jamo(s[i]))) {
      return 1;
   }
}
return 0;
}


/**
 * Computes the end positions for the given tag. Also updates current positions.
 * Note that *end_pos_token, *end_pos_char and *end_pos_letter are supposed to
 * have been initialized with the current positions.
 */
void compute_end_positions(unichar* jamo_tag,unichar* jamo_text,int *pos_in_jamo_text,
      unichar* syllab_text,int *pos_in_syllab_text,
      int *end_pos_token,int *end_pos_char,int *end_pos_letter,
      int *new_pos_in_token,int *new_pos_in_char,int *new_pos_in_letter,
      int* token_sizes,Alphabet* alphabet) {
int current_token=*end_pos_token;
int current_char=*end_pos_char;
int current_letter=*end_pos_letter;
if (/*u_is_korea_syllabe_letter(jamo_tag[0]) || alphabet->korean_equivalent_syllab[jamo_tag[0]]!=0*/
      /*jamo_tag[0]!=0x318D && !u_is_Hangul_Jamo(jamo_tag[0])*/
      token_contains_Hangul_or_Chinese_chars(jamo_tag,alphabet)
   ) {
   if ((*end_pos_letter)!=0
         /* With this test, we deal with the case where a Jamo char was in the original text.
          * Then, we must update here the position in the text. It cannot be done
          * elsewhere, because this complicated case of "end of token even if there
          * is still Jamo char in the text" cannot be detected easily */
         && syllab_text[*pos_in_syllab_text]!=jamo_tag[0]
      ) {
      fatal_error("compute_end_positions: cannot match a syllabic char when current_letter!=0\n"
                  "current_letter=%d  current token size=%d  tag=%S  syllab_text[%d]=%C  pos_in_jamo_text=%d\n"
                  "current token=%d\n",
                  *end_pos_letter,token_sizes[current_token],jamo_tag,*pos_in_syllab_text,
                  syllab_text[*pos_in_syllab_text],*pos_in_jamo_text,current_token);
   }
   int was_a_Jamo_in_syllab_text=0;
   if (syllab_text[*pos_in_syllab_text]!=0x318D && u_is_Hangul_Compatility_Jamo(syllab_text[*pos_in_syllab_text])) {
      /* If necessary, we update the positions */
      error("updating letter position for tag %S\n",jamo_tag);
      current_letter=0;
      was_a_Jamo_in_syllab_text=1;
   }
   for (int i=0;jamo_tag[i]!='\0';i++) {
      if (jamo_tag[i]!=syllab_text[*pos_in_syllab_text]) {
         fatal_error("compute_end_positions: mismatch between text and tag syllabic character\n"
                     "tag[%d]=%C  text[%d]=%C\n",i,jamo_tag[i],*pos_in_syllab_text,syllab_text[*pos_in_syllab_text]);
      }
      (*pos_in_syllab_text)++;
      (*end_pos_char)=current_char;
      current_char++;
      /* We also must go on in the Jamo text */
      if ((i>0 || !was_a_Jamo_in_syllab_text) 
            /* The previous char was a Jamo ? */
          && *pos_in_jamo_text>0 && u_is_Hangul_Jamo(jamo_text[(*pos_in_jamo_text)-1])
            /* Not a Jamo syllab bound ? */ 
          && jamo_text[*pos_in_jamo_text]!=0x318D
            /* Not a non Jamo letter ? */
          && !is_letter(jamo_text[*pos_in_jamo_text],alphabet)) {
         fatal_error("compute_end_positions: should have found a syllab bound character\n"
                     "jamo_tag=%S pos_in_syllab_text=%d jamo_text[%d]=%C\n"
                     "i=%d  was a jamo in syllab text=%d\n",
                     jamo_tag,*pos_in_syllab_text,*pos_in_jamo_text,jamo_text[*pos_in_jamo_text],
                     i,was_a_Jamo_in_syllab_text);
      }
      (*pos_in_jamo_text)++;
      current_letter=0;
      if (!was_a_Jamo_in_syllab_text) {
         /* If there was a Jamo char in the syllab text, then the (*pos_in_jamo_text)++; above
          * has already moved correctly the position in the Jamo text */
         while (u_is_Hangul_Jamo(jamo_text[*pos_in_jamo_text])) {
            /*error("pour le tag %S, on zappe le char %C a la position %d\n",jamo_tag,
                  jamo_text[*pos_in_jamo_text],*pos_in_jamo_text);*/
            (*end_pos_letter)=current_letter;
            current_letter++;
            (*pos_in_jamo_text)++;
         }
      }
   }
   if (current_char==token_sizes[current_token]) {
      /* If we have reached the end of the current token */
      current_token++;
      current_char=0;
      current_letter=0;
   } else {
      /* If not, we are supposed to have reached the end of a syllab */
      current_letter=0;
   }
   (*new_pos_in_token)=current_token;
   (*new_pos_in_char)=current_char;
   (*new_pos_in_letter)=current_letter;
   return;
}
/* We have a tag made of any characters, except Korean syllabic ones */
for (int i=0;jamo_tag[i]!='\0';i++) {
   if (jamo_tag[i]==0x318D) {
      /* We skip the sentence bound character in the tag */
      continue;
   }
   if (!u_is_Hangul_Jamo(jamo_tag[i])) {
      /* If the current character is not a Jamo letter, we must compare it to the
       * one in the text */
      if (jamo_tag[i]!=jamo_text[*pos_in_jamo_text]) {
         fatal_error("compute_end_positions: mismatch between text and tag Jamo character\n"
                              "tag[%d]=%C  text[%d]=%C\n",i,jamo_tag[i],*pos_in_jamo_text,jamo_text[*pos_in_jamo_text]);
      }
      (*pos_in_jamo_text)++;
      /* A non Jamo character always corresponds to one character in the syllab text */
      (*pos_in_syllab_text)++;
      (*end_pos_token)=current_token;
      (*end_pos_char)=current_char;
      (*end_pos_letter)=current_letter;
      if (current_char+1!=token_sizes[current_token]) {
         /* If we are not at the end of the current token */
         current_char++;
         current_letter=0;
      } else {
         /* We have reached the end of the current token */
         current_char=0;
         current_letter=0;
         current_token++;
      }
   } else {
      /* We have a Jamo letter */
      if (jamo_text[*pos_in_jamo_text]==0x318D) {
         /* If the text contains a syllab bound character, we must reset the letter position,
          * and that's all since char and syllab positions are modified when we reached the
          * end of a syllab (see below) */
         current_letter=0;
         (*pos_in_jamo_text)++;
         if (!u_is_Hangul_Jamo(jamo_text[*pos_in_jamo_text])) {
            fatal_error("compute_end_positions: non Jamo letter after syllab bound character\n");
         }
      }
      /* When we have a Jamo letter, we just have to compare it with the current one in
       * the text */
      if (jamo_tag[i]!=jamo_text[*pos_in_jamo_text]) {
         fatal_error("compute_end_positions: mismatch #2 between text and tag character\n"
                     "jamo_tag[%d]=%C  jamo_text[%d]=%C\n",i,jamo_tag[i],*pos_in_jamo_text,jamo_text[*pos_in_jamo_text]);
      }
      (*pos_in_jamo_text)++;
      (*end_pos_token)=current_token;
      (*end_pos_char)=current_char;
      (*end_pos_letter)=current_letter;
      /*if (*pos_in_jamo_text==57) {
         error("pos_in_jamo_text==57  pour le tag %S\n",jamo_tag);
         error("  => current_char=%d  current token size=%d\n",current_char,token_sizes[current_token]);
         error("  => prochain char=%C (%X) jamo=%d\n",jamo_text[*pos_in_jamo_text],jamo_text[*pos_in_jamo_text],u_is_Hangul_Jamo(jamo_text[*pos_in_jamo_text]));
         error("for %C, syllab[%d]=%C\n",0x1105,*pos_in_syllab_text,syllab_text[(*pos_in_syllab_text)]);
         error("    is Jamo=%d  is Jamo compatible=%d\n",u_is_Hangul_Jamo(syllab_text[(*pos_in_syllab_text)])
               ,u_is_Hangul_Compatility_Jamo(syllab_text[(*pos_in_syllab_text)]));
         error("current letter=%d\n",current_letter);
      }*/
      
      if (current_char+1==token_sizes[current_token] &&
            /* If we are on the last char of the current token */
            (!u_is_Hangul_Jamo(jamo_text[*pos_in_jamo_text])
               /* and if 1) there is no more Jamo char in the Jamo text */ 
            || 
               /* or 2) if there is a Jamo char but we are in the case were this char
                * comes from a Jamo char that was present in the original syllabic text */
               u_is_Hangul_Jamo(syllab_text[(*pos_in_syllab_text)+1])
               || u_is_Hangul_Compatility_Jamo(syllab_text[(*pos_in_syllab_text)+1])
            )) {
         if (u_is_Hangul_Jamo(syllab_text[(*pos_in_syllab_text)+1])
               || u_is_Hangul_Compatility_Jamo(syllab_text[(*pos_in_syllab_text)+1])) {
            error("pos_in_jamo_text=%d:   end of token because of Jamo char %C in the syllabic text\n",
                  *pos_in_jamo_text,syllab_text[(*pos_in_syllab_text)+1]);
         }
         current_char=0;
         current_letter=0;
         current_token++;
         (*pos_in_syllab_text)++;
      } else {
         /* We are not at the end of the current token */
         if (u_is_Hangul_Jamo(jamo_text[*pos_in_jamo_text])) {
            /* If the next character is a Jamo letter, then we just increase the 
             * letter position */
            current_letter++;
         } else {
            if (jamo_text[*pos_in_jamo_text]!=0x318D) {
               fatal_error("compute_end_positions: syllab bound character expected but not found\n");
            }
            /* If the next character is not a Jamo letter, then we must reset the letter position
             * and increase the char position */
            current_letter=0;
            current_char++;
            (*pos_in_syllab_text)++;
         }
      }
   }
}
/* Finally, we set the positions for the next move */
(*new_pos_in_token)=current_token;
(*new_pos_in_char)=current_char;
(*new_pos_in_letter)=current_letter;
}


/* the function is the recursive work for explore_korean_automaton_for_positions
   with jamo_tag and syllab_tag private array of 256 unichar provided
 */
void explore_korean_automaton_for_positions_with_buffer(Tfst* tfst,Fst2* jamo,unichar* jamo_text,
      int current_state,int pos_in_token,int pos_in_char,int pos_in_letter,
      int pos_in_syllab_text,int pos_in_jamo_text,Alphabet* alphabet,
      unichar* jamo_tag,unichar* syllab_tag) {
Transition* t=tfst->automaton->states[current_state]->outgoing_transitions;
TfstTag* tag;
while (t!=NULL) {
   tag=(TfstTag*)(tfst->tags->tab[t->tag_number]);
   if (tag->type!=T_EPSILON && tag->m.end_pos_in_token==-1) {
      /* We only process non epsilon tags that have not been explored */
      if (!u_strcmp(tag->content,"<BL>")) {
         /* The special <BL> tag was used to represent the space in a text automaton.
          * Now, we just increase our positions and then turn this tag into an 
          * epsilon one, after some error checking */
         if (jamo_text[pos_in_jamo_text]!=' ') {
            fatal_error("explore_korean_automaton_for_positions_with_buffer: jamo_text <BL> error\n");
         }
         if (tfst->text[pos_in_syllab_text]!=' ') {
            fatal_error("explore_korean_automaton_for_positions_with_buffer: syllab_text <BL> error\n"
                        "text[%d]=%C\n",pos_in_syllab_text,tfst->text[pos_in_syllab_text]);
         }
         /* We set the tag number to 0 in order to replace this <BL> transition by an epsilon one */
         t->tag_number=0;
         
         explore_korean_automaton_for_positions_with_buffer(tfst,jamo,jamo_text,t->state_number,
               /* We change of token */
               pos_in_token+1,0,0,pos_in_syllab_text+1,pos_in_jamo_text+1,alphabet,jamo_tag,syllab_tag);
      } else {
         /* Non <BL> tag */
         /* Remember that the original tag number was stored in 'start_pos_token' */
         get_tag_content(jamo->tags[tag->m.start_pos_in_token]->input,jamo_tag);
         if (!u_strcmp(jamo_tag,"<E>")) {
            /* If we have an empty surface form */
            tag->m.start_pos_in_token=pos_in_token;
            tag->m.end_pos_in_token=pos_in_token;
            tag->m.start_pos_in_char=pos_in_char;
            tag->m.end_pos_in_char=pos_in_char;
            tag->m.start_pos_in_letter=pos_in_letter;
            /* We note that we have a tag that correspond to the empty word in the text by
             * setting end_pos_letter to -1. We set all other fields with correct values in
             * order to know where this empty surface form occurs */
            tag->m.end_pos_in_letter=-1;
            explore_korean_automaton_for_positions_with_buffer(tfst,jamo,jamo_text,t->state_number,
                                       pos_in_token,pos_in_char,pos_in_letter,
                                       pos_in_syllab_text,pos_in_jamo_text,alphabet,jamo_tag,syllab_tag);
         } else {
            /* Normal tag */ 
            //get_tag_content(tag->content,syllab_tag);
            /* The start positions are the current ones */
            tag->m.start_pos_in_token=pos_in_token;
            tag->m.start_pos_in_char=pos_in_char;
            tag->m.start_pos_in_letter=pos_in_letter;
            /* We also initialize the end positions with the current ones */
            tag->m.end_pos_in_token=pos_in_token;
            tag->m.end_pos_in_char=pos_in_char;
            tag->m.end_pos_in_letter=pos_in_letter;
                        
            int new_pos_in_token,new_pos_in_char,new_pos_in_letter;
            int new_pos_in_jamo_text=pos_in_jamo_text;
            int new_pos_in_syllab_text=pos_in_syllab_text;
            compute_end_positions(jamo_tag,jamo_text,&new_pos_in_jamo_text
                  ,tfst->text,&new_pos_in_syllab_text
                  ,&(tag->m.end_pos_in_token),&(tag->m.end_pos_in_char),&(tag->m.end_pos_in_letter)
                  ,&new_pos_in_token,&new_pos_in_char,&new_pos_in_letter,
                  tfst->token_sizes->tab,alphabet);
            
            //error("\nafter tag %S, pos_token=%d pos_char=%d pos_letter=%d      pos_syllab=%d pos_jamo=%d\n",syllab_tag,
            //      new_pos_in_token,new_pos_in_char,new_pos_in_letter,new_pos_in_syllab_text,new_pos_in_jamo_text);
            explore_korean_automaton_for_positions_with_buffer(tfst,jamo,jamo_text,t->state_number,
                           new_pos_in_token,new_pos_in_char,new_pos_in_letter,
                           new_pos_in_syllab_text,new_pos_in_jamo_text,alphabet,jamo_tag,syllab_tag);
         }
      }
   }
   t=t->next;
}
}


/**
 * This function explores the sentence automaton in order to set up the positions
 * in its TfstTag. In order to avoid combinatorial explosion, we use 
 * TfstTag->end_pos_token!=-1 as a test to determine if a transition was already 
 * visited (we don't use TfstTag->start_pos_token because it is already used to 
 * know the number of the original tag).
 */
void explore_korean_automaton_for_positions(Tfst* tfst,Fst2* jamo,unichar* jamo_text,
      int current_state,int pos_in_token,int pos_in_char,int pos_in_letter,
      int pos_in_syllab_text,int pos_in_jamo_text,Alphabet* alphabet) {
unichar jamo_tag[256];
unichar syllab_tag[256];
  explore_korean_automaton_for_positions_with_buffer(tfst,jamo,jamo_text,
      current_state,pos_in_token,pos_in_char,pos_in_letter,
      pos_in_syllab_text,pos_in_jamo_text,alphabet,jamo_tag,syllab_tag) ;
}

/**
 * This function builds the sentence automaton that correspond to the
 * given Korean token buffer. It saves it into the given file.
 */
void build_korean_sentence_automaton(char* exe_path,int* buffer,int length,struct text_tokens* tokens,
                               Alphabet* alph,U_FILE* out_tfst,U_FILE* out_tind,
                               int sentence_number,
                               int we_must_clean,
                               int current_global_position_in_tokens,
                               int current_global_position_in_chars,
                               char* phrase_cod,char* jamoTable,char* kr_fst2) {
/* First of all, we compute the sentence automaton, using the special
 * Korean tools */
u_printf("sentence #%d\n",sentence_number);
char n[32];
char command[FILENAME_MAX];
sprintf(command,"%s%s",exe_path,"Txt2Fst2Kr");
//ProgramInvoker* invoker=new_ProgramInvoker(NULL,command);
ProgramInvoker* invoker=new_ProgramInvoker(main_Txt2Fst2Kr,command);
add_argument(invoker,"-e");
sprintf(n,"%d",sentence_number);
add_argument(invoker,n);
add_argument(invoker,phrase_cod);
//int ret_value=invoke_as_new_process(invoker);
int ret_value=invoke(invoker);
free_ProgramInvoker(invoker);
if (ret_value!=0) {
   fatal_error("Txt2Fst2Kr did not quit normally. Cannot go on constructing .tfst file\n");
}
sprintf(command,"%s%s",exe_path,"Jamo2Syl");
//invoker=new_ProgramInvoker(NULL,command);
invoker=new_ProgramInvoker(main_Jamo2Syl,command);
add_argument(invoker,"-m");
add_argument(invoker,jamoTable);
add_argument(invoker,kr_fst2);
char jamo_fst2[FILENAME_MAX];
char syllab_fst2[FILENAME_MAX];
get_path(phrase_cod,jamo_fst2);
strcat(jamo_fst2,"sentence.fst2");
get_path(phrase_cod,syllab_fst2);
strcat(syllab_fst2,"sentencesyl.fst2");
add_argument(invoker,jamo_fst2);
//ret_value=invoke_as_new_process(invoker);
ret_value=invoke(invoker);
free_ProgramInvoker(invoker);
if (ret_value!=0) {
   fatal_error("Jamo2Syl did not quit normally. Cannot go on constructing .tfst file\n");
}

/* We load the two versions of the Korean sentence automaton */ 
Fst2* jamo=load_abstract_fst2(jamo_fst2,1,NULL);
Fst2* syllab=load_abstract_fst2(syllab_fst2,0,NULL);


/* We declare the graph that will represent the sentence as well as
 * a temporary string_hash 'tmp_tags' that will be used to store the tags of this
 * graph. We don't put tags directly in the main 'tags', because a tag could
 * be introduced and then removed by cleaning */
Tfst* tfst=new_Tfst(NULL,NULL,0);
tfst->current_sentence=sentence_number;
tfst->automaton=new_SingleGraph();
tfst->offset_in_tokens=current_global_position_in_tokens;
tfst->offset_in_chars=current_global_position_in_chars;
/* We compute the text and tokens of the sentence */
/* Temp string used to build tags with bound control */
Ustring* foo=new_Ustring(1);
tfst->tokens=new_vector_int(length);
tfst->token_sizes=new_vector_int(length);
for (int i=0;i<length;i++) {
   vector_int_add(tfst->tokens,buffer[i]);
   int l=u_strlen(tokens->token[buffer[i]]);
   vector_int_add(tfst->token_sizes,l);
   u_strcat(foo,tokens->token[buffer[i]],l);
}
tfst->text=u_strdup(foo->str);
/* Now, we create cursentence.txt, but we save a modified version of the text in which
 * Chinese characters are replaced by their Hangul equivalents */
for (int i=0;i<foo->len;i++) {
   unichar c=alph->korean_equivalent_syllab[foo->str[i]];
   if (c!=0) {
      foo->str[i]=c;
   }
}
char cursentence_txt[FILENAME_MAX];
get_path(phrase_cod,cursentence_txt);
strcat(cursentence_txt,"cursentence.txt");
U_FILE* zz=u_fopen(UTF16_LE,cursentence_txt,U_WRITE);
if (zz==NULL) {
   fatal_error("Cannot create %s\n",cursentence_txt);
}
u_fprintf(zz,"%S",foo->str);
u_fclose(zz);
/* And we convert it to cursentencejm.txt */
sprintf(command,"%s%s",exe_path,"Syl2Jamo");
//invoker=new_ProgramInvoker(NULL,command);
invoker=new_ProgramInvoker(main_Syl2Jamo,command);
add_argument(invoker,"-j");
add_argument(invoker,"-m");
add_argument(invoker,jamoTable);
char current_sentence[FILENAME_MAX];
get_path(phrase_cod,current_sentence);
strcat(current_sentence,"cursentence.txt");
add_argument(invoker,current_sentence);
//ret_value=invoke_as_new_process(invoker);
ret_value=invoke(invoker);
free_ProgramInvoker(invoker);
if (ret_value!=0) {
   fatal_error("Syl2Jamo did not quit normally. Cannot go on constructing .tfst file\n");
}
/* Now, we load the letter version of the text of the current sentence */
get_path(phrase_cod,current_sentence);
strcat(current_sentence,"cursentencejm.txt");
unichar* jamo_text=load_as_a_string(current_sentence); 

tfst->tags=new_vector_ptr(256);
/* The epsilon tag must always be the first one */
vector_ptr_add(tfst->tags,new_TfstTag(T_EPSILON));

/* We create a copy of the Korean sentence automaton */
for (int i=0;i<jamo->number_of_states;i++) {
   Fst2State state_src=jamo->states[i];
   SingleGraphState state_dest=add_state(tfst->automaton);
   if (is_initial_state(state_src)) {
      set_initial_state(state_dest);
   }
   if (is_final_state(state_src)) {
      set_final_state(state_dest);
   }
   /* We recreate all transitions */
   Transition* trans_src=state_src->transitions;
   /* We use this complicated pointer trick with 'trans_dest' in order to keep
    * the same order in the resulting automaton, to facilitate debugging */
   Transition* *trans_dest=&(state_dest->outgoing_transitions);
   while (trans_src!=NULL) {
      int tag_number;
      if (trans_src->tag_number==0) {
         /* This should not occur: the f#!&*@ing program that constructed
          * the Korean sentence automaton shoudn't have inserted epsilon transition
          * in a sentence automaton !!!! */
         tag_number=0;
      } else {
         /* We have to create TfstTag structures, because we have not all the information
          * yet: we will need 1) to compute positions and 2) to replace <BR> transitions
          * by epsilon transitions. So, we cannot rawly produce strings ready to be dumped
          * into the .tfst file */   
         TfstTag* tag=new_TfstTag(T_STD);
         /* We say that the content of the transition is the syllabic version */
         unichar* q=syllab->tags[trans_src->tag_number]->input;
         if (q[0]=='\\' && q[2]=='\0') {
            /* The f#!&*@ing program that constructed the Korean sentence 
             * automaton shouldn't have protected things with \ */
            tag->content=u_strdup(q+1);
            /* We also modify the jamo and syllab .fst2 */
            u_strcpy(syllab->tags[trans_src->tag_number]->input,tag->content);
            u_strcpy(jamo->tags[trans_src->tag_number]->input,tag->content);
         } else {
            tag->content=u_strdup(q);
         }
         /* In order to compute later positions in text, we keep the number of the original
          * transition, arbitrarily in 'start_pos_token'. The trick is that the Jamo2Syl
          * program keeps tag numbering */
         tag->m.start_pos_in_token=trans_src->tag_number;
         tag_number=vector_ptr_add(tfst->tags,tag);
      }
      (*trans_dest)=new_Transition(tag_number,trans_src->state_number);
      trans_dest=&((*trans_dest)->next);
      trans_src=trans_src->next;
   }
}
/* Now, we have to explore the automaton we have just created in order
 * to compute the positions for each transition tag */ 
explore_korean_automaton_for_positions(tfst,jamo,jamo_text,0,0,0,0,0,0,alph);

/* We clean epsilon transitions */
remove_epsilon_transitions(tfst->automaton,0);

/* If we want to minimize, we must unify tags */
/* Finally, we replace the TfstTag by their string representations in order to
 * discard TfstTag that are not used anymore, like <BL> */
struct string_hash* tmp_tags=new_string_hash(32);
unichar EPSILON_TAG[]={'@','<','E','>','\n','.','\n','\0'};
/* The epsilon tag is always the first one */
get_value_index(EPSILON_TAG,tmp_tags);
unichar dest[4096];
for (int i=0;i<tfst->automaton->number_of_states;i++) {
   Transition* t=tfst->automaton->states[i]->outgoing_transitions;
   while (t!=NULL) {
      TfstTag* src_tag=(TfstTag*)(tfst->tags->tab[t->tag_number]);
      TfstTag_to_string(src_tag,dest);
      int old=t->tag_number;
      t->tag_number=get_value_index(dest,tmp_tags);
      if (!u_strcmp(src_tag->content,"<BL>")) {
          fatal_error("build_korean_sentence_automaton: <BL> in tag #%d that has replaced by %d\n",old,t->tag_number);
      }
      t=t->next;
   }
}
if (we_must_clean) {
   /* If necessary, we apply the "good paths" heuristic */
   keep_best_paths(tfst->automaton,tmp_tags);
   trim(tfst->automaton);
}
/* Then we minimize */
minimize(tfst->automaton,1);

/* Finally, we renumber one more time the tags, because the minimization may have 
 * removed unused tags */
struct string_hash* tags=new_string_hash(32);
/* The epsilon tag is always the first one */
get_value_index(EPSILON_TAG,tags);
for (int i=0;i<tfst->automaton->number_of_states;i++) {
   Transition* t=tfst->automaton->states[i]->outgoing_transitions;
   while (t!=NULL) {
      if (t->tag_number>tmp_tags->size) {
         fatal_error("build_korean_sentence_automaton: from state %d, invalid tag index %d (should be <%d)\n",i,t->tag_number,tmp_tags->size);
      }
      unichar* s=tmp_tags->value[t->tag_number];
      t->tag_number=get_value_index(s,tags);
      t=t->next;
   }
}
save_current_sentence(tfst,out_tfst,out_tind,tags->value,tags->size);
close_text_automaton(tfst);
free_string_hash(tmp_tags);
free_string_hash(tags);
free_Ustring(foo);
free_Fst2(jamo);
free_Fst2(syllab);
free(jamo_text);
return;



#if 0
/* We declare the graph that will represent the sentence as well as
 * a temporary string_hash 'tmp_tags' that will be used to store the tags of this
 * graph. We don't put tags directly in the main 'tags', because a tag can
 * be introduced and then removed by cleaning */
Tfst* tfst=new_Tfst(NULL,NULL,0);
tfst->current_sentence=sentence_number;
tfst->automaton=new_SingleGraph();
tfst->offset_in_tokens=current_global_position_in_tokens;
tfst->offset_in_chars=current_global_position_in_chars;

struct string_hash* tags=new_string_hash(32);
struct string_hash* tmp_tags=new_string_hash(32);
unichar EPSILON_TAG[]={'@','<','E','>','\n','.','\n','\0'};
/* The epsilon tag is always the first one */
get_value_index(EPSILON_TAG,tmp_tags);
get_value_index(EPSILON_TAG,tags);

int i;
/* We add +1 for the final node */
int n_nodes=1+count_non_space_tokens(buffer,length,tokens->SPACE);
for (i=0;i<n_nodes;i++) {
   add_state(tfst->automaton);
}
set_initial_state(tfst->automaton->states[0]);
set_final_state(tfst->automaton->states[n_nodes-1]);
struct info INFO;
INFO.tok=tokens;
INFO.buffer=buffer;
INFO.alph=alph;
INFO.SPACE=tokens->SPACE;
INFO.length_max=length;
int current_state=0;
int is_not_unknown_token;
unichar inflected[4096];
/* Temp string used to build tags with bound control */
Ustring* foo=new_Ustring(1);
/* We compute the text and tokens of the sentence */
tfst->tokens=new_vector_int(length);
tfst->token_sizes=new_vector_int(length);
for (int i=0;i<length;i++) {
   vector_int_add(tfst->tokens,buffer[i]);
   int l=u_strlen(tokens->token[buffer[i]]);
   vector_int_add(tfst->token_sizes,l);
   u_strcat(foo,tokens->token[buffer[i]],l);
}
tfst->text=u_strdup(foo->str);


for (i=0;i<length;i++) {
   if (buffer[i]==tokens->SENTENCE_MARKER) {
      fatal_error("build_sentence_automaton: unexpected {S} token\n");
   }
   if (buffer[i]!=tokens->SPACE) {
      /* We try to produce every transition from the current token */
      is_not_unknown_token=0;
      explore_dictionary_tree(0,tokens->token[buffer[i]],inflected,0,DELA_tree->inflected_forms->root,DELA_tree,&INFO,tfst->automaton->states[current_state],1,
                              current_state,&is_not_unknown_token,i,i,tmp_tags,foo,language);
      if (norm_tree!=NULL) {
         /* If there is a normalization tree, we explore it */
         explore_normalization_tree(i,i,buffer[i],&INFO,tfst->automaton,tmp_tags,norm_tree,current_state,1,foo,0,language);
      }
      if (!is_not_unknown_token) {
         /* If the token was not matched in the dictionary, we put it as an unknown one */
         u_sprintf(foo,"@STD\n@%S\n@%d-%d\n.\n",tokens->token[buffer[i]],i,i);
         int tag_number=get_value_index(foo->str,tmp_tags);
         add_outgoing_transition(tfst->automaton->states[current_state],tag_number,current_state+1);
      }
      current_state++;
   }
}

/* Now, we insert the tag sequences found in the 'tags.ind' file, if any */
struct match_list* tmp;
while ((*tag_list)!=NULL && (*tag_list)->start>=current_global_position_in_tokens
       && (*tag_list)->start<=current_global_position_in_tokens+length) {
   if ((*tag_list)->end>current_global_position_in_tokens+length) {
      /* If we have a tag sequence that overlap two sentences, we must ignore it */
      tmp=(*tag_list)->next;
      free_match_list_element((*tag_list));
      (*tag_list)=tmp;
      continue;
   }
   /* We compute the local bounds of the tag sequence */
   int start_index=(*tag_list)->start-current_global_position_in_tokens;
   int end_index=(*tag_list)->end-current_global_position_in_tokens;
   int start_pos_in_token=start_index;
   int end_pos_in_token=end_index;
   /* And we adjust them to our state indexes, because spaces
    * must be ignored */
   for (int i=start_index;i>=0;i--) {
      if (buffer[i]==tokens->SPACE) {
         start_index--;
      }
   }
   for (int i=end_index;i>=0;i--) {
      if (buffer[i]==tokens->SPACE) {
         end_index--;
      }
   }
   add_path_to_sentence_automaton(start_pos_in_token,end_pos_in_token,start_index,
                                  INFO.alph,tfst->automaton,tmp_tags,
                                  (*tag_list)->output,end_index+1,foo,&INFO);
   tmp=(*tag_list)->next;
   free_match_list_element((*tag_list));
   (*tag_list)=tmp;
}

if (we_must_clean) {
   /* If necessary, we apply the "good paths" heuristic */
   keep_best_paths(tfst->automaton,tmp_tags);
}
trim(tfst->automaton);
if (tfst->automaton->number_of_states==0) {
   /* Case 1: the automaton has been emptied because of the tagset filtering */
   error("Sentence %d is empty\n",tfst->current_sentence);
   SingleGraphState initial=add_state(tfst->automaton);
   set_initial_state(initial);
   free_vector_ptr(tfst->tags,(void (*)(void*))free_TfstTag);
   tfst->tags=new_vector_ptr(1);
   vector_ptr_add(tfst->tags,new_TfstTag(T_EPSILON));
   save_current_sentence(tfst,out_tfst,out_tind,NULL,0);
} else {
   /* Case 2: the automaton is not empty */

   /* We minimize the sentence automaton. It will remove the unused states and may
    * factorize suffixes introduced during the application of the normalization tree. */
   minimize(tfst->automaton,1);
   /* We explore all the transitions of the automaton in order to renumber transitions */
   for (i=0;i<tfst->automaton->number_of_states;i++) {
      Transition* trans=tfst->automaton->states[i]->outgoing_transitions;
      while (trans!=NULL) {
         /* For each tag of the graph that is actually used, we put it in the main
          * tags and we use this index in the tfst transition */
         trans->tag_number=get_value_index(tmp_tags->value[trans->tag_number],tags);
         trans=trans->next;
      }
   }
   save_current_sentence(tfst,out_tfst,out_tind,tags->value,tags->size);
}
close_text_automaton(tfst);
free_string_hash(tmp_tags);
free_string_hash(tags);
free_Ustring(foo);
#endif
}
