/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <time.h>
#include "Text_tokens.h"
#include "ApplyDic.h"
#include "Error.h"
#include "File.h"
#include "BuildTextAutomaton.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/*
 * "pomme de terre" is made of 5 tokens : "pomme" SPACE "de" SPACE "terre"
 * Here we define the maximum number of tokens in a compound.
 */
#define TOKENS_IN_A_COMPOUND 256

/* This margin is used for compound words: when we are at less
 * than 'MARGIN_BEFORE_BUFFER_END' from the end of the buffer, we will
 * refill it, unless we are at the end of the input file. */
//#define MARGIN_BEFORE_BUFFER_END 200


void free_word_struct(struct word_struct*);
void free_word_transition(struct word_transition*);
void free_offset_list(struct offset_list*);


/**
 * Creates, initializes and returns a new word struct array with the
 * given capacity.
 */
struct word_struct_array* new_word_struct_array(int n) {
struct word_struct_array* res;
res=(struct word_struct_array*)malloc(sizeof(struct word_struct_array));
if (res==NULL) {
   fatal_alloc_error("new_word_struct_array");
}
res->element=(struct word_struct**)malloc(sizeof(struct word_struct*)*n);
if (res->element==NULL) {
   fatal_alloc_error("new_word_struct_array");
}
for (int i=0;i<n;i++) {
   res->element[i]=NULL;
}
res->N=n;
return res;
}


/**
 * Frees a word struct array.
 */
void free_word_struct_array(struct word_struct_array* w) {
if (w==NULL) return;
for (int i=0;i<w->N;i++) {
  free_word_struct(w->element[i]);
}
free(w->element);
free(w);
}


/**
 * Creates, initializes and returns a word struct.
 */
struct word_struct* new_word_struct() {
struct word_struct* res;
res=(struct word_struct*)malloc(sizeof(struct word_struct));
if (res==NULL) {
   fatal_alloc_error("new_word_struct");
}
res->list=NULL;
res->trans=NULL;
return res;
}


/**
 * Frees a word struct.
 */
void free_word_struct(struct word_struct* w) {
if (w==NULL) return;
free_word_transition(w->trans);
free_offset_list(w->list);
free(w);
}


/**
 * Creates, initializes and returns a word struct.
 */
struct word_transition* new_word_transition() {
struct word_transition* res;
res=(struct word_transition*)malloc(sizeof(struct word_transition));
if (res==NULL) {
   fatal_alloc_error("new_word_transition");
}
res->node=NULL;
res->next=NULL;
return res;
}


/**
 * Looks for a word transition in a sorted list.
 */
struct word_transition* get_word_transition(struct word_transition* t,int token) {
while (t!=NULL && t->token_number<=token) {
   if (t->token_number==token) {
      return t;
   }
   t=t->next;
}
return NULL;
}


/**
 * Gets a word transition from a sorted list. If neccessary, the word transition is
 * created and inserted in the list. Returns the list. The result element is stored
 * in the 'result' parameter.
 */
struct word_transition* insert_word_transition(struct word_transition* list,
                                               struct word_transition** result,
                                               int token) {
if (list==NULL) {
   (*result)=new_word_transition();
   (*result)->token_number=token;
   return (*result);
}
if (list->token_number==token) {
   (*result)=list;
   return list;
}
if (list->token_number<token) {
   list->next=insert_word_transition(list->next,result,token);
   return list;
}
/* If we are here, we must insert a new word transition in the list */
(*result)=new_word_transition();
(*result)->token_number=token;
(*result)->next=list;
return (*result);
}


/**
 * Frees a word transition.
 */
void free_word_transition(struct word_transition* t) {
struct word_transition* tmp;
while (t!=NULL) {
   free_word_struct(t->node);
   tmp=t;
   t=t->next;
   free(tmp);
}
}


/**
 * This function returns a struct offset_list* that contains the given offset.
 * If the offset is not in the list, the function adds it.
 */
struct offset_list* get_offset(int offset,struct offset_list* l,unichar* content,int base,unichar* output) {
if (l==NULL) {
   /* If the offset is not in the list, we add it */
   l=(struct offset_list*)malloc(sizeof(struct offset_list));
   if (l==NULL) {
      fatal_alloc_error("get_offset");
   }
   l->offset=offset;
   l->content=u_strdup(content);
   l->base=base;
   l->output=u_strdup(output);
   l->next=NULL;
   return l;
}
/* If we have it, we return it */
if (l->offset==offset) return l;
/* Otherwise, we look further */
l->next=get_offset(offset,l->next,content,base,output);
return l;
}


/**
 * This function associates an offset in the .bin to the given token.
 * 'content' is the token as found in the dictionary. For instance, if
 * token #45="APPLE", word_array->element[45] will contain a reference to
 * the offset that correspond to the word "apple", and "apple" will be copied
 * in the content associated to this offset.
 *
 * Note that several offset/content pairs can be assigned to a token. For instance,
 * the token "JACK" can be associated to both entries "Jack" (proper name)  and "jack"
 * (noun: electrical connection stuff, card figure, etc).
 */
void add_offset_for_token(struct word_struct_array* word_array,
                          int token_number,int offset,unichar* content,
                          int base,Ustring* output) {
if (word_array->element[token_number]==NULL) {
   word_array->element[token_number]=new_word_struct();
}
unichar* s=NULL;
if (output!=NULL && output->len!=0) {
    s=output->str;
}
word_array->element[token_number]->list=get_offset(offset,word_array->element[token_number]->list,content,base,s);
}


/**
 * Frees an offset list.
 */
void free_offset_list(struct offset_list* l) {
struct offset_list* tmp;
while (l!=NULL) {
   free(l->content);
   free(l->output);
   tmp=l;
   l=l->next;
   free(tmp);
}
}


/* display uncompress entry
 * function extracted from explore_bin_simple_words, because each recursive call
 * allocated 4096 unichar (and produce stack overflow)
 */
void display_uncompressed_entry(U_FILE* f,unichar* inflected,unichar* INF_code) {
Ustring* s=new_Ustring(DIC_LINE_SIZE);
uncompress_entry(inflected,INF_code,s);
u_fprintf(f,"%S\n",s->str);
free_Ustring(s);
}


/**
 * This function explores a .bin dictionary in order to test if 'token' is a
 * simple word. 'offset' is the offset of the current dictionary node. 'inflected'
 * is the exact entry in the dictionary. It may differ from 'token' because of
 * case variation (for instance, if 'token'="WRITTEN", we will have
 * 'inflected'="written"). 'pos' is the current position in 'token'.
 * 'token_number' is the number of the current token. If the token is found
 * to be an entry of the dictionary and if it has not already been matched by
 * dictionary with a greater priority, we save the corresponding DELAF line
 * in the DLF.
 */
void explore_bin_simple_words(struct dico_application_info* info,
                              int offset,const unichar* token,unichar* inflected,
                              int pos,int token_number,int priority,Ustring* ustr,int base) {
int final,n_transitions,inf_number;
/* We compute the number of transitions that outgo from the current node */
int z=save_output(ustr);
int new_offset=read_dictionary_state(info->d,offset,&final,&n_transitions,&inf_number);
if (token[pos]=='\0') {
   /* If we are at the end of the token */
   inflected[pos]='\0';
   if (final) {
      /* If the node is final */
       if (info->word_array!=NULL) add_offset_for_token(info->word_array,token_number,offset,inflected,0,NULL);
      int p=0;
      if (info->simple_word!=NULL) p=get_value(info->simple_word,token_number);
      if (p==0 || p==priority) {
         /* We save the token only if it has not already been matched by
          * dictionary with a greater priority. Moreover, we indicate that
          * this token is part of a word and that it has been processed. */
          if (info->part_of_a_word!=NULL) set_value(info->part_of_a_word,token_number,1);
         if (info->simple_word!=NULL) set_value(info->simple_word,token_number,priority);
         /* We get the INF codes */
         struct list_ustring* head;
         int to_be_freed=get_inf_codes(info->d,inf_number,ustr,&head,base);
         struct list_ustring* tmp=head;
         /* Then, we produce the DELAF line corresponding to each compressed line */
         while (tmp!=NULL) {
             if (info->dic_name[0]!='\0') {
                u_fprintf(info->dlf,"%s\n",info->dic_name);
                info->dic_name[0]='\0';
             }
             display_uncompressed_entry(info->dlf,inflected,tmp->string);
             tmp=tmp->next;
         }
         if (to_be_freed) free_list_ustring(head);
         base=ustr->len;
      }
   } else {
       /* The node is not final */
       if (info->word_array!=NULL) add_offset_for_token(info->word_array,token_number,offset,inflected,base,ustr);
   }
   /* If we are at the end of the token, there is no need to look at the
    * outgoing transitions */
   restore_output(z,ustr);
   return;
}
/* If we are in a final node */
if (final) {
    base=ustr->len;
}
offset=new_offset;
unichar c;
int offset_dest;
for (int i=0;i<n_transitions;i++) {
   /* For each outgoing transition, we look if the transition character is
    * compatible with the token's one */
    offset=read_dictionary_transition(info->d,offset,&c,&offset_dest,ustr);
    if (is_equal_or_uppercase(c,token[pos],info->alphabet)) {
      /* We copy the transition character so that 'inflected' will contain
       * the exact inflected form */
      inflected[pos]=c;
      explore_bin_simple_words(info,offset_dest,token,inflected,pos+1,token_number,priority,ustr,base);
   }
    restore_output(z,ustr);
}
}


/**
 * This function looks for every token of the text if it can
 * be a simple word. If it is the case, the corresponding DELAF lines
 * are saved in 'info->dlf' if the word has not already been matched
 * by a dictionary with a greater priority.
 */
void look_for_simple_words(struct dico_application_info* info,int priority) {
/* this function is called only once by dico application, so we will use heap instead stack */
unichar* entry=(unichar*)malloc(sizeof(unichar)*DIC_WORD_SIZE);
if (entry==NULL) {
   fatal_alloc_error("look_for_simple_words");
}
Ustring* ustr=new_Ustring();
for (int i=0;i<info->tokens->N;i++) {
   explore_bin_simple_words(info,info->d->initial_state_offset,info->tokens->token[i],entry,0,i,priority,ustr,0);
}
free_Ustring(ustr);
free(entry);
}


/**
 * This function explores a .bin dictionary in order to find out compound words.
 * - 'offset' is the offset of the current .bin node.
 * - 'current_token' is the subsequence that we are trying to match. For speed reason,
 *   we don't try to match the whole sequence. For instance, if we already have matched
 *   "grand-maman", we know where to start in the .bin if we have "grand-". Then,
 *   if now we need to match "grand-papa Joe", we will only look for "papa Joe" starting from
 *   the offset corresponding to "grand-" that we have cached. In this example,
 *   'current_token' would be "papa" and 'inflected' would be "grand-". Note that
 *   inflected contains the exact entry with no case variation. For instance, if
 *   there is "BLACK-EYED" in the text, entry will contain "black-eyed".
 * - 'pos_offset' is the number of tokens in the compound -1. In the "grand-papa Joe"
 *   example, it would be 2 at the first call and 4 at when the whole sequence is
 *   processed, since "grand-papa Joe" contains 5 tokens.
 * - 'token_sequence' is the array that contains the number of the tokens that
 *   compose the word, ended by -1. If "grand"=token 45, "-"=token 2,
 *   "papa"=token 324, " "=token 4 and "Joe"=token 17, we would have
 *   'token_sequence'={45,2,324,4,17,-1}
 * - 'current_start_pos' is the offset of the first token of the sequence in the
 *   text buffer
 * - 'line_buf' is a private unichar array of DIC_LINE_SIZE item provided by called
 *
 * If we find a compound that has not already been matched by a dictionary
 * with a greater priority, we save it to 'info->dlc'.
 */
void explore_bin_compound_words(struct dico_application_info* info,
                                int offset,unichar* current_token,unichar* inflected,
                                int pos_in_current_token,
                                int pos_in_inflected,struct word_struct* ws,int pos_offset,
                                int* token_sequence,int pos_token_sequence,int priority,
                                int current_start_pos,Ustring* line_buf,Ustring* ustr,int base) {
int final,n_transitions,inf_number;
int z=save_output(ustr);
int new_offset=read_dictionary_state(info->d,offset,&final,&n_transitions,&inf_number);
if (current_token[pos_in_current_token]=='\0') {
   /* If we are at the end of the current token, we look for the
    * corresponding node in the token tree */
   struct word_transition* trans;
   ws->trans=insert_word_transition(ws->trans,&trans,info->text_cod_buf[current_start_pos+pos_offset]);
   if (trans->node==NULL) {
      /* If the node does not exist in the token tree, we create it */
      trans->node=new_word_struct();
   }
   inflected[pos_in_inflected]='\0';
   /* We add the current token to the token sequence */
   token_sequence[pos_token_sequence++]=trans->token_number;
   /* And we add the current offset to the node list */
   if (final) {
      /* If this node is final */
       trans->node->list=get_offset(offset,trans->node->list,inflected,0,NULL);
      token_sequence[pos_token_sequence]=-1;
      /* We look if the compound word has already been matched */
      int w=was_already_in_tct_hash(token_sequence,info->tct_h,priority);
      if (w==0 || w==priority) {
         /* If the compound has not already been matched by a dictionary
          * with a greater priority */
         for (int k=current_start_pos;k<=current_start_pos+pos_offset;k++) {
            /* We say that its tokens are not unknown words */
            set_value(info->part_of_a_word,info->text_cod_buf[k],1);
         }
         /* We get the INF codes */
         struct list_ustring* head;
         int to_be_freed=get_inf_codes(info->d,inf_number,ustr,&head,base);
         struct list_ustring* tmp=head;
         /* We increase the number of compound word occurrences.
          * Note that we count occurrences and not number of entries, so that
          * if we find "copy and paste" in the text we will count one more
          * compound occurrence, even if this word can be a noun and a verb. */
         info->COMPOUND_WORDS++;
         while (tmp!=NULL) {
            /* For each compressed code of the INF line, we save the corresponding
             * DELAF line in 'info->dlc' */
            uncompress_entry(inflected,tmp->string,line_buf);
            u_fprintf(info->dlc,"%S\n",line_buf->str);
            tmp=tmp->next;
         }
         if (to_be_freed) free_list_ustring(head);
      }
      base=ustr->len;
   } else {
       /* The node is not final */
       trans->node->list=get_offset(offset,trans->node->list,inflected,base,ustr->str);
   }
   pos_offset++;
   /* Then, we go on with the next token in the text, so we update 'current_token',
    * but only if we haven't reached the end of the text buffer */
   if (current_start_pos+pos_offset >= info->text_cod_size_nb_int) {
       restore_output(z,ustr);
      return;
   }
   current_token=info->tokens->token[info->text_cod_buf[current_start_pos+pos_offset]];
   pos_in_current_token=0;
   ws=trans->node;
   /* TRICK! We don't need to perform a call to 'explore_bin_compound_words', since
    * we would arrive after the next closing round bracket in the same conditions
    * than now. */
}
/* If we are not at the end of the current token */
/* Do not recursively explore deeper paths if we already have
 * reached the end of the current token. */
if (current_token[pos_in_current_token]=='\0') {
   restore_output(z,ustr);
    return;
}
/* If we are in a final node */
if (final) {
    base=ustr->len;
}
unichar c;
int adr;
offset=new_offset;
for (int i=0;i<n_transitions;i++) {
   offset=read_dictionary_transition(info->d,offset,&c,&adr,ustr);
   if (is_equal_or_uppercase(c,current_token[pos_in_current_token],info->alphabet)) {
      /* We explore the rest of the dictionary only if the
       * dictionary char is compatible with the token char. In that case,
       * we copy in 'inflected' the exact chararacter that is in the dictionary. */
      inflected[pos_in_inflected]=c;
      explore_bin_compound_words(info,adr,current_token,inflected,pos_in_current_token+1,pos_in_inflected+1,ws,
        pos_offset,token_sequence,pos_token_sequence,priority,current_start_pos,line_buf,ustr,base);
   }
   restore_output(z,ustr);
}
}


/**
 * This function looks for compound words in the text file set in 'info'.
 * When a compound word is found, the corresponding DELAF lines are saved in
 * 'info->dlc' if the word has not already been matched by a dictionary with
 * a greater priority.
 */
void look_for_compound_words(struct dico_application_info* info,int priority) {
/* this function is called only once by dico application, so we will use heap instead stack */
unichar* inflected=(unichar*)malloc(sizeof(unichar)*DIC_WORD_SIZE);
if (inflected==NULL) {
   fatal_alloc_error("look_for_simple_words");
}
int* token_sequence=(int*)malloc(sizeof(int)*TOKENS_IN_A_COMPOUND);
if (token_sequence==NULL) {
   fatal_alloc_error("look_for_simple_words");
}
struct word_struct* w;
/* We go at the beginning of the file and we fill the buffer */
/*
fseek(info->text_cod,0,SEEK_SET);
fill_buffer(info->buffer,info->text_cod);
*/
Ustring* line_buf=new_Ustring(4096);
Ustring* ustr=new_Ustring();
int current_start_pos=0;
u_printf("First block...              \r");
while (current_start_pos<info->text_cod_size_nb_int) {/*
   if (!info->buffer->end_of_file
       && current_start_pos>(info->buffer->size-MARGIN_BEFORE_BUFFER_END)) {
      // If we must change of block and if we can
      u_printf("Block %d...              \r",++current_block);
      fill_buffer(info->buffer,current_start_pos,info->text_cod);
      current_start_pos=0;
   }*/
   int token_number=info->text_cod_buf[current_start_pos];
   /* We look for compound words that start with the current token */
   w=info->word_array->element[token_number];
   if (w!=NULL) {
      /* If there are some */
      struct word_transition* trans;
      int no_more_word_transition=0;
      /* 'pos_offset' is the number to add to 'current_start_pos' in order
       * to have the current position in the text buffer */
      int pos_offset=1;
      /* We put the first token in the token sequence */
      int current_token_in_compound=0;
      token_sequence[current_token_in_compound++]=token_number;
      /* We try to go in the text as far as possible, using the information cached
       * in info->word_array to avoid some computation */
      while (!no_more_word_transition) {
         trans=NULL;
         if (current_start_pos+pos_offset < info->text_cod_size_nb_int)
           trans=get_word_transition(w->trans,info->text_cod_buf[current_start_pos+pos_offset]);
         if (trans==NULL) {
            /* If there is no more possibility to go on */
            no_more_word_transition=1;
         }
         else {
            w=trans->node;
            /* If we can go on, we add the current token to our token sequence */
            token_sequence[current_token_in_compound++]=info->text_cod_buf[current_start_pos+pos_offset];
            /* We add -1 at the end in the case this token would be the last
             * of the compound word */
            token_sequence[current_token_in_compound]=-1;
            pos_offset++;
         }
      }
      struct offset_list* l=w->list;

      if (current_start_pos+pos_offset < info->text_cod_size_nb_int) {
        while (l!=NULL) {
           /* If there are dictionary nodes to explore, we do so. For each node
            * we copy into 'entry' the sequence of character that leads to it in
            * the .bin */
           u_strcpy_sized(inflected,DIC_WORD_SIZE,l->content);
           u_strcpy(ustr,l->output);
           explore_bin_compound_words(info,l->offset,info->tokens->token[info->text_cod_buf[current_start_pos+pos_offset]],inflected,0,u_strlen(inflected),w,
             pos_offset,token_sequence,current_token_in_compound,priority,current_start_pos,line_buf,ustr,l->base);
           l=l->next;
        }
      }
   }
   current_start_pos++;
}
u_printf("\n");
free_Ustring(line_buf);
free_Ustring(ustr);
free(inflected);
free(token_sequence);
}


/**
 * This functions dumps the unknown words into the 'err' file. As a side effect,
 * the number of occurrences of simple and unknown words are computed.
 */
void save_unknown_words(struct dico_application_info* info) {
info->SIMPLE_WORDS=0;
info->UNKNOWN_WORDS=0;
for (int i=0;i<info->tokens->N;i++) {
   if (is_letter(info->tokens->token[i][0],info->alphabet)) {
      /* We examine all the tokens that are made of letters */
      if (!get_value(info->part_of_a_word,i)) {
         /* To be an unknown word, a token must not be a part of a word */
          info->UNKNOWN_WORDS=info->UNKNOWN_WORDS+info->n_occurrences[i];
          u_fprintf(info->err,"%S\n",info->tokens->token[i]);
          if (!get_value(info->part_of_a_word2,i)) {
              if (info->tags_err!=NULL) {
                  u_fprintf(info->tags_err,"%S\n",info->tokens->token[i]);
              }
          }
      }
      else {
         /* If the token is part of a word and if it is a simple word,
          * we update the number of simple word occurrences. */
         if (get_value(info->simple_word,i)) {
            info->SIMPLE_WORDS=info->SIMPLE_WORDS+info->n_occurrences[i];
         }
      }
   }
}
}


/**
 * This function initializes and returns a structure that all
 * the information needed for the application of dictionaries.
 */
struct dico_application_info* init_dico_application(struct text_tokens* tokens,
                                                    U_FILE* dlf,U_FILE* dlc,U_FILE* err,U_FILE* tags_err,U_FILE* morpho,
                                                    const char* tags,const char* text_cod,Alphabet* alphabet,
                                                    const VersatileEncodingConfig* vec) {
struct dico_application_info* info=(struct dico_application_info*)malloc(sizeof(struct dico_application_info));
if (info==NULL) {
   fatal_alloc_error("init_dico_application");
}
info->map_text_cod=af_open_mapfile(text_cod,MAPFILE_OPTION_READ,0);
info->text_cod_buf=(const int*)af_get_mapfile_pointer(info->map_text_cod);
info->text_cod_size_nb_int=(int)(af_get_mapfile_size(info->map_text_cod)/sizeof(int));
info->tokens=tokens;
info->dlf=dlf;
info->dlc=dlc;
info->err=err;
info->tags_err=tags_err;
info->morpho=morpho;
info->dic_name[0]='\0';
strcpy(info->tags_ind,tags);
info->alphabet=alphabet;
info->d=NULL;
info->word_array=NULL;
info->part_of_a_word=new_bit_array(tokens->N,ONE_BIT);
info->part_of_a_word2=new_bit_array(tokens->N,ONE_BIT);
info->simple_word=new_bit_array(tokens->N,TWO_BITS);
info->n_occurrences=(int*)malloc(tokens->N*sizeof(int));
if (info->part_of_a_word==NULL || info->part_of_a_word2==NULL
        || info->simple_word==NULL || info->n_occurrences==NULL) {
   fatal_alloc_error("init_dico_application");
}
for (int j=0;j<tokens->N;j++) {
   info->n_occurrences[j]=0;
}
info->tct_h=new_tct_hash();
info->tct_h_tags_ind=new_tct_hash();
info->SIMPLE_WORDS=0;
info->COMPOUND_WORDS=0;
info->UNKNOWN_WORDS=0;
info->tag_sequences=NULL;
info->n_tag_sequences=0;
info->tag_sequences_capacity=0;
info->vec=*vec;
return info;
}


/**
 * Frees all the memory allocated for the given structure.
 *
 * IMPORTANT: note that info->alphabet and info->word_array are
 * not freed; this is the responsability of the function
 * that allocated these objects.
 */
void free_dico_application(struct dico_application_info* info) {
if (info==NULL) return;
af_release_mapfile_pointer(info->map_text_cod,info->text_cod_buf);
af_close_mapfile(info->map_text_cod);
free_bit_array(info->part_of_a_word);
free_bit_array(info->part_of_a_word2);
free_bit_array(info->simple_word);
free(info->n_occurrences);
free_tct_hash(info->tct_h);
free_tct_hash(info->tct_h_tags_ind);
for (int i=0;i<info->n_tag_sequences;i++) {
    free_match_list_element(info->tag_sequences[i]);
}
free_Dictionary(info->d);
free(info->tag_sequences);
free(info);
}


/**
 * This function launches the application of the given .bin dictionary.
 *
 * @author Alexis Neme
 * Modified by Sébastien Paumier
 */
int dico_application(const VersatileEncodingConfig* vec,const char* name_bin,struct dico_application_info* info,int priority) {
char name_inf[FILENAME_MAX];
remove_extension(name_bin,name_inf);
strcat(name_inf,".inf");
info->d=new_Dictionary(vec,name_bin,name_inf);
if (info->d==NULL) {
    error("Cannot open dictionary %s\n",name_bin);
    return 1;
}
info->word_array=new_word_struct_array(info->tokens->N);
/* And then we look simple and then compound words.
 * IMPORTANT: it is crucial to look for simple words first, since
 *            some initializations are made there that are used
 *            when looking for compound words.
 */
u_printf("Looking for simple words...\n");
look_for_simple_words(info,priority);
u_printf("Looking for compound words...\n");
/* We measure the elapsed time */
#ifdef DEBUG
clock_t startTime=clock();
#endif
look_for_compound_words(info,priority);
#ifdef DEBUG
clock_t endTime = clock();
double  elapsedTime = (double) (endTime - startTime);
u_printf("%2.8f seconds\n",elapsedTime);
#endif
free_word_struct_array(info->word_array);
free_Dictionary(info->d);
info->d=NULL;
return 0;
}


/**
 * This function launches the application of the given .bin dictionary.
 *
 * @author Alexis Neme
 * Modified by Sébastien Paumier
 */
int dico_application_simplified(const VersatileEncodingConfig* vec,const unichar* text,const char* name_bin,struct dico_application_info* info) {
char name_inf[FILENAME_MAX];
remove_extension(name_bin,name_inf);
strcat(name_inf,".inf");
info->d=new_Dictionary(vec,name_bin,name_inf);
if (info->d==NULL) return 1;
unichar entry[DIC_WORD_SIZE];
Ustring* ustr=new_Ustring();
explore_bin_simple_words(info,info->d->initial_state_offset,text,entry,0,-1,0,ustr,0);
free_Ustring(ustr);
free_Dictionary(info->d);
info->d=NULL;
/*free_abstract_INF(info->inf,&info->inf_free);
free_abstract_BIN(info->bin,&info->bin_free);*/
return 0;
}


/**
 * Adds the given match to the tag sequence array, enlarging it if needed.
 * Returns 1 if the match was actually added; 0 otherwise.
 */
int add_tag_sequence(struct dico_application_info* info,struct match_list* match,int priority) {
/* First, we test if the current match has not already been matched with
 * a greater priority */
int foo[3]={match->m.start_pos_in_token,match->m.end_pos_in_token,-1};
int w=was_already_in_tct_hash(foo,info->tct_h_tags_ind,priority);
if (w!=0 && w!=priority) {
    /* If the match has already been processed
     * with a greater priority, we skip it */
    return 0;
}
/* And we note that the match has been taken into account
 * with that priority */
add_tct_token_sequence(foo,info->tct_h_tags_ind,priority);
if (info->n_tag_sequences==info->tag_sequences_capacity) {
   /* If we have to enlarge the array, doubling its capacity */
    if (info->tag_sequences_capacity==0) {
        info->tag_sequences_capacity=32;
    }
    else {
        info->tag_sequences_capacity=2*info->tag_sequences_capacity;
    }
    info->tag_sequences=(struct match_list**)realloc(info->tag_sequences,info->tag_sequences_capacity*sizeof(struct match_list*));
    if (info->tag_sequences==NULL) {
       fatal_alloc_error("add_tag_sequence");
    }
}
info->tag_sequences[(info->n_tag_sequences)++]=match;
return 1;
}


void check_tag_sequence_validity(unichar* s,Alphabet* alph) {
if (s==NULL || s[0]=='\0') {
    fatal_error("Invalid tag sequence: %S\n",s);
}
vector_ptr* v=tokenize_normalization_output(s,alph);
if (v==NULL) {
    fatal_error("Invalid tag sequence: %S\n",s);
}
free_vector_ptr(v,(void(*)(void*))free_output_info);
}


/**
 * @author Alexis Neme
 * Modified by Sébastien Paumier
 */
int merge_dic_locate_results(struct dico_application_info* info,const char* concord_filename,
                             int priority,int export_to_morpho_dic) {
/* This array is used to represent a compound word at a token sequence ended by -1.
 * Example: cinquante-deux could be represented by (1347,35,582,-1) */
int token_tab_coumpounds[TOKENS_IN_A_COMPOUND];
u_printf("Merging dic/locate result...\n");
/* First, we load the match list */
U_FILE* f=u_fopen(&(info->vec),concord_filename,U_READ);
if (f==NULL) {
   error("Cannot open %s\n",concord_filename);
   return 0;
}
struct match_list* l=load_match_list(f,NULL,NULL);
u_fclose(f);


Abstract_allocator merge_dic_locate_results_abstract_allocator=NULL;
merge_dic_locate_results_abstract_allocator=create_abstract_allocator("merge_dic_locate_results",
                                                        AllocatorFreeOnlyAtAllocatorDelete|AllocatorTipGrowingOftenRecycledObject,
                                                        0);
while (l!=NULL) {
   if (l->output!=NULL && l->output[0]=='/') {
       /* If we have a tag sequence to be used at the time of
        * building the text automaton */
       check_tag_sequence_validity(l->output+1,info->alphabet);
       /* If the tag sequence is not valid, a fatal error will be raised */
       if (add_tag_sequence(info,l,priority)) {
           /* If we have found and handled a valid tag sequence, we process
            * the next match in the list, AND WE DON'T FREE THE CURRENT
            * MATCH, since it's now in a pointer array. */
           for (int i=l->m.start_pos_in_token;i<=l->m.end_pos_in_token;i++) {
               set_value(info->part_of_a_word2,info->text_cod_buf[i],1);
           }
           l=l->next;
       } else {
           /* The match was already there, we have to free it */
           struct match_list* tmp=l->next;
           free_match_list_element(l);
           l=tmp;
       }
       continue;
   }
   /* We test if the match is a valid dictionary entry */
   struct dela_entry* entry=tokenize_DELAF_line(l->output, 1, merge_dic_locate_results_abstract_allocator);
   if (entry!=NULL) {
      /* If the entry is valid */
      if (is_sequence_of_letters(entry->inflected,info->alphabet)) {
         /* If it is a simple word */
         int token_number=get_token_number(entry->inflected,info->tokens);
         if (token_number==-1) {
            /* If we find in the dictionary a token that is not in the text,
             * we fail */
             error("Ignoring line because the inflected form does not appear in the text:\n%S\n",l->output);
         } else {
            int p=get_value(info->simple_word,token_number);
            if (p==0 || p==priority) {
               /* We save the simple word only if it hasn't already been processed with
                * a greater priority */
               set_value(info->part_of_a_word,token_number,1);
               set_value(info->simple_word,token_number,priority);
               /* We save it to the DLF */
               u_fprintf(info->dlf,"%S\n",l->output);
               /* If needed, we save it to the morpho.dic file */
               if (export_to_morpho_dic) {
                  u_fprintf(info->morpho,"%S\n",l->output);
               }
            }
         }
      }
      else {
         /* If it is a compound word, we turn it into a token sequence
          * ended by -1 */
         if (build_token_sequence(entry->inflected,info->tokens,token_tab_coumpounds)) {
             int w=was_already_in_tct_hash(token_tab_coumpounds,info->tct_h,priority);
             if (w==0 || w==priority) {
                 /* We save the compound word only if it hasn't already been processed
                  * with a greater priority */
                 for (int k=0;token_tab_coumpounds[k]!=-1;k++) {
                     /* If we have matched a compound word, then all its part all not
                      * unknown words */
                     set_value(info->part_of_a_word,token_tab_coumpounds[k],1);
                 }
                 /* We save it to the DLC */
                 u_fprintf(info->dlc,"%S\n",l->output);
                 /* If needed, we save it to the morpho.dic file */
                 if (export_to_morpho_dic) {
                     u_fprintf(info->morpho,"%S\n",l->output);
                 }
             }
         }
      }
      /* Finally, we free the entry */
      free_dela_entry(entry, merge_dic_locate_results_abstract_allocator);
   }
   /* If the match is not a valid entry, an error message has already
    * been produced by tokenize_DELAF_line, so there is nothing to do. */
   struct match_list* tmp=l->next;
   free_match_list_element(l);
   l=tmp;
}
close_abstract_allocator(merge_dic_locate_results_abstract_allocator);
return 1;
}


/**
 * This function reads the whole 'tokens.cod' file and computes the number of
 * occurrences of each token.
 */
void count_token_occurrences(struct dico_application_info* info) {

const int* buffer=info->text_cod_buf;
for (int i=0;i<info->text_cod_size_nb_int;i++) {
   info->n_occurrences[buffer[i]]++;
}
}


/**
 * This function is used to sort matches by start/end positions.
 */
int compare_matches(const void* a,const void* b) {
struct match_list** A=(struct match_list**)a;
struct match_list** B=(struct match_list**)b;
switch (compare_matches(&((*A)->m),&((*B)->m))) {
   case A_BEFORE_B:
   case A_BEFORE_B_OVERLAP:
   case A_INCLUDES_B: return -1;

   case A_EQUALS_B: return u_strcmp((*A)->output,(*B)->output);

   case A_AFTER_B:
   case A_AFTER_B_OVERLAP:
   case B_INCLUDES_A: return 1;
}
fatal_error("Internal error in compare_matches\n");
return 0; /* Just to avoid a warning */
}


/**
 * Does as explained in the function name.
 */
void save_and_sort_tag_sequences(struct dico_application_info* info) {
qsort(info->tag_sequences,info->n_tag_sequences,sizeof(struct match_list*),compare_matches);
U_FILE* f=u_fopen(&(info->vec),info->tags_ind,U_WRITE);
if (f==NULL) {return;}
/* We use the header T, just to say something different from I, M and R */
u_fprintf(f,"#T\n");
struct match_list* tmp;
for (int i=0;i<info->n_tag_sequences;i++) {
   tmp=info->tag_sequences[i];
   /* We take tmp->output+1 in order to avoid copying the / character */
   u_fprintf(f,"%d.0.0 %d.%d.0 %S\n",tmp->m.start_pos_in_token,tmp->m.end_pos_in_token,tmp->m.end_pos_in_char,tmp->output+1);
}
u_fclose(f);
}

} // namespace unitex

