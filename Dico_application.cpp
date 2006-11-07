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
#include <time.h>
#include "Dico_application.h"
#include "Error.h"
#include "Matches.h"
#include "FileName.h"
//---------------------------------------------------------------------------

#define DEBUG 0

/*
 * "pomme de terre" is made of 5 tokens : "pomme" SPACE "de" SPACE "terre"
 * Here we define the maximum number of tokens in a compound.
 */
#define TOKENS_IN_A_COMPOUND 256

/* This margin is used for compound words: when we are at less
 * than 'MARGIN_BEFORE_BUFFER_END' from the end of the buffer, we will
 * refill it, unless we are at the end of the input file. */
#define MARGIN_BEFORE_BUFFER_END 200


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
   fatal_error("Not enough memory in new_word_struct_array\n");
}
res->element=(struct word_struct**)malloc(sizeof(struct word_struct*)*n);
if (res->element==NULL) {
   fatal_error("Not enough memory in new_word_struct_array\n");
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
free(w);
}


/**
 * Creates, initializes and returns a word struct.
 */
struct word_struct* new_word_struct() {
struct word_struct* res;
res=(struct word_struct*)malloc(sizeof(struct word_struct));
if (res==NULL) {
   fatal_error("Not enough memory in new_word_struct\n");
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
   fatal_error("Not enough memory in new_word_transition\n");
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
struct offset_list* get_offset(int offset,struct offset_list* l,unichar* content) {
if (l==NULL) {
   /* If the offset is not in the list, we add it */
   l=(struct offset_list*)malloc(sizeof(struct offset_list));
   if (l==NULL) {
      fatal_error("Not enough memory in get_offset\n");
   }
   l->offset=offset;
   l->content=u_strdup(content);
   l->next=NULL;
   return l;
}
/* If we have it, we return it */
if (l->offset==offset) return l;
/* Otherwise, we look further */
l->next=get_offset(offset,l->next,content);
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
                          int token_number,int offset,unichar* content) {
if (word_array->element[token_number]==NULL) {
   word_array->element[token_number]=new_word_struct();
}
word_array->element[token_number]->list=get_offset(offset,word_array->element[token_number]->list,content);
}


/**
 * Frees an offset list.
 */
void free_offset_list(struct offset_list* l) {
struct offset_list* tmp;
while (l!=NULL) {
  free(l->content);
  tmp=l;
  l=l->next;
  free(tmp);
}
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
                              int offset,unichar* token,unichar* inflected,
                              int pos,int token_number,int priority) {
/* We compute the number of transitions that outgo from the current node */
int n_transitions=((unsigned char)info->bin[offset])*256+(unsigned char)info->bin[offset+1];
offset=offset+2;
if (token[pos]=='\0') {
   /* If we are at the end of the token */
   inflected[pos]='\0';
   add_offset_for_token(info->word_array,token_number,offset-2,inflected);
   if (!(n_transitions & 32768)) {
      /* If the node is final */
      int p=get_value(info->simple_word,token_number);
      if (p==0 || p==priority) {
         /* We save the token only if it has not already been matched by
          * dictionary with a greater priority. Moreover, we indicate that
          * this token is part of a word and that it has been processed. */
         set_value(info->part_of_a_word,token_number,1);
         set_value(info->simple_word,token_number,priority);
         /* We compute the INF line number and we get the associated compressed lines */
         int inf_number=((unsigned char)info->bin[offset])*256*256+((unsigned char)info->bin[offset+1])*256+(unsigned char)info->bin[offset+2];
         struct list_ustring* tmp=info->inf->codes[inf_number];
         /* Then, we produce the DELAF line corresponding to each compressed line */
         while (tmp!=NULL) {
            unichar line[DIC_LINE_SIZE];
            uncompress_entry(inflected,tmp->string,line);
            u_fprints(line,info->dlf);
            u_fprints_char("\n",info->dlf);
            tmp=tmp->next;
         }
      }
   }
   /* If we are at the end of the token, there is no need to look at the
    * outgoing transitions. */
   return;
}
if ((n_transitions & 32768)) {
   /* If we are in a normal node, we remove the control bit to
    * have the good number of transitions */
   n_transitions=n_transitions-32768;
} else {
   /* If we are in a final node, we must jump after the reference to the INF
    * line number */
   offset=offset+3;
}
for (int i=0;i<n_transitions;i++) {
   /* For each outgoing transition, we look if the transition character is
    * compatible with the token's one */
   unichar c=(unichar)(((unsigned char)info->bin[offset])*256+(unsigned char)info->bin[offset+1]);
   offset=offset+2;
   int offset_dest=((unsigned char)info->bin[offset])*256*256+((unsigned char)info->bin[offset+1])*256+(unsigned char)info->bin[offset+2];
   offset=offset+3;
   if (is_equal_or_uppercase(c,token[pos],info->alphabet)) {
      /* We copy the transition character so that 'inflected' will contain
       * the exact inflected form */
      inflected[pos]=c;
      explore_bin_simple_words(info,offset_dest,token,inflected,pos+1,token_number,priority);
   }
}
}


/**
 * This function looks for every token of the text if it can
 * be a simple word. If it is the case, the corresponding DELAF lines
 * are saved in 'info->dlf' if the word has not already been matched
 * by a dictionary with a greater priority.
 */
void look_for_simple_words(struct dico_application_info* info,int priority) {
unichar entry[DIC_WORD_SIZE];
for (int i=0;i<info->tokens->N;i++) {
   explore_bin_simple_words(info,4,info->tokens->token[i],entry,0,i,priority);
}
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
 * 
 * If we find a compound that has not already been matched by a dictionary
 * with a greater priority, we save it to 'info->dlc'.
 */
void explore_bin_compound_words(struct dico_application_info* info,
                                int offset,unichar* current_token,unichar* inflected,
                                int pos_in_current_token,
                                int pos_in_inflected,struct word_struct *w,int pos_offset,
                                int* token_sequence,int pos_token_sequence,int priority,
                                int current_start_pos) {
int n_transitions=((unsigned char)info->bin[offset])*256+(unsigned char)info->bin[offset+1];
offset=offset+2;
if (current_token[pos_in_current_token]=='\0') {
   /* If we are at the end of the current token, we look for the 
    * corresponding node in the token tree */
   struct word_transition* trans;
   w->trans=insert_word_transition(w->trans,&trans,info->buffer->buffer[current_start_pos+pos_offset]);
   if (trans->node==NULL) {
      /* If the node does not exist in the token tree, we create it */
      trans->node=new_word_struct();
   }
   inflected[pos_in_inflected]='\0';
   /* We add the current token to the token sequence */
   token_sequence[pos_token_sequence++]=trans->token_number;
   /* And we add the current offset to the node list */
   trans->node->list=get_offset(offset-2,trans->node->list,inflected);
   if (!(n_transitions & 32768)) {
      /* If this node is final */
      token_sequence[pos_token_sequence]=-1;
      /* We look if the compound word has already been matched */
      int w=was_allready_in_tct_hash(token_sequence,info->tct_h,priority);
      if (w==0 || w==priority) {
         /* If the compound has not already been matched by a dictionary
          * with a greater priority */
         for (int k=current_start_pos;k<=current_start_pos+pos_offset;k++) {
            /* We say that its tokens are not unknown words */
            set_value(info->part_of_a_word,info->buffer->buffer[k],1);
         }
         /* We get the INF line number */
         int inf_number=((unsigned char)info->bin[offset])*256*256+((unsigned char)info->bin[offset+1])*256+(unsigned char)info->bin[offset+2];
         unichar line[DIC_LINE_SIZE];
         struct list_ustring* tmp=info->inf->codes[inf_number];
         /* We increase the number of compound word occurrences.
          * Note that we count occurrences and not number of entries, so that
          * if we find "copy and paste" in the text we will count one more
          * compound occurrence, even if this word can be a noun and a verb. */
         info->COMPOUND_WORDS++;
         while (tmp!=NULL) {
            /* For each compressed code of the INF line, we save the corresponding
             * DELAF line in 'info->dlc' */
            uncompress_entry(inflected,tmp->string,line);
            u_fprints(line,info->dlc);
            u_fprints_char("\n",info->dlc);
            tmp=tmp->next;
         }
      }
   }
   pos_offset++;
   /* Then, we go on with the next token in the text, so we update 'current_token' */
   current_token=info->tokens->token[info->buffer->buffer[current_start_pos+pos_offset]];
   pos_in_current_token=0;
   w=trans->node;
   /* TRICK! We don't need to perform a call to 'explore_bin_compound_words', since
    * we would arrive after the next closing round bracket in the same conditions
    * than now. */
}
/* If we are not at the end of the current token */
if ((n_transitions & 32768)) {
   /* If we are in a normal node, we remove the control bit to
    * have the good number of transitions */
   n_transitions=n_transitions-32768;
} else {
   /* If we are in a final node, we must jump after the reference to the INF line number */
   offset=offset+3;
}
for (int i=0;i<n_transitions;i++) {
   unichar c=(unichar)(((unsigned char)info->bin[offset])*256+(unsigned char)info->bin[offset+1]);
   offset=offset+2;
   int adr=((unsigned char)info->bin[offset])*256*256+((unsigned char)info->bin[offset+1])*256+(unsigned char)info->bin[offset+2];
   offset=offset+3;
   if (is_equal_or_uppercase(c,current_token[pos_in_current_token],info->alphabet)) {
      /* We explore the rest of the dictionary only if the
       * dictionary char is compatible with the token char. In that case,
       * we copy in 'inflected' the exact chararacter that is in the dictionary. */
      inflected[pos_in_inflected]=c;
      explore_bin_compound_words(info,adr,current_token,inflected,pos_in_current_token+1,pos_in_inflected+1,w,pos_offset,token_sequence,pos_token_sequence,priority,current_start_pos);
   }
}
}


/**
 * This function looks for compound words in the text file set in 'info'.
 * When a compound word is found, the corresponding DELAF lines are saved in
 * 'info->dlc' if the word has not already been matched by a dictionary with
 * a greater priority.
 */
void look_for_compound_words(struct dico_application_info* info,int priority) {
unichar inflected[DIC_WORD_SIZE];
int token_sequence[TOKENS_IN_A_COMPOUND];
struct word_struct* w;
/* We go at the beginning of the file and we fill the buffer */
fseek(info->text_cod,0,SEEK_SET);
fill_buffer(info->buffer,info->text_cod);
int current_start_pos=0;
printf("First block...              \r");
int current_block=1;
while (current_start_pos<info->buffer->size) {
   if (!info->buffer->end_of_file
       && current_start_pos>(info->buffer->size-MARGIN_BEFORE_BUFFER_END)) {
      /* If we must change of block and if we can */
      printf("Block %d...              \r",++current_block);
      fill_buffer(info->buffer,current_start_pos,info->text_cod);
      current_start_pos=0;
   }
   int token_number=info->buffer->buffer[current_start_pos];
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
         trans=get_word_transition(w->trans,info->buffer->buffer[current_start_pos+pos_offset]);
         if (trans==NULL) {
            /* If there is no more possibility to go on */
            no_more_word_transition=1;
         }
         else {
            w=trans->node;
            /* If we can go on, we add the current token to our token sequence */
            token_sequence[current_token_in_compound++]=info->buffer->buffer[current_start_pos+pos_offset];
            /* We add -1 at the end in the case this token would be the last
             * of the compound word */
            token_sequence[current_token_in_compound]=-1;
            pos_offset++;
         }
      }
      struct offset_list* l=w->list;
      while (l!=NULL) {
         /* If there are dictionary nodes to explore, we do so. For each node
          * we copy into 'entry' the sequence of character that leads to it in
          * the .bin */
         u_strcpy(inflected,l->content);
         explore_bin_compound_words(info,l->offset,info->tokens->token[info->buffer->buffer[current_start_pos+pos_offset]],inflected,0,u_strlen(inflected),w,pos_offset,token_sequence,current_token_in_compound/*0*/,priority,current_start_pos);
         l=l->next;
      }
   }
   current_start_pos++;
}
printf("\n");
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
          u_fprints(info->tokens->token[i],info->err);
          u_fprints_char("\n",info->err);
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
                                                    FILE* dlf,FILE* dlc,FILE* err,
                                                    FILE* text_cod,Alphabet* alphabet) {
struct dico_application_info* info=(struct dico_application_info*)malloc(sizeof(struct dico_application_info));
if (info==NULL) {
   fatal_error("Not enough memory in init_dico_application\n");
}
info->text_cod=text_cod;
info->tokens=tokens;
info->dlf=dlf;
info->dlc=dlc;
info->err=err;
info->buffer=new_buffer(BUFFER_SIZE);
info->alphabet=alphabet;
info->bin=NULL;
info->inf=NULL;
info->word_array=NULL;
info->part_of_a_word=new_bit_array(tokens->N,ONE_BIT);
info->simple_word=new_bit_array(tokens->N,TWO_BITS);
info->n_occurrences=(int*)malloc(tokens->N*sizeof(int));
if (info->part_of_a_word==NULL || info->simple_word==NULL || info->n_occurrences==NULL) {
   fatal_error("Not enough memory in init_dico_application\n");
}
for (int j=0;j<tokens->N;j++) {
   info->n_occurrences[j]=0;
}
info->tct_h=new_tct_hash(TCT_HASH_SIZE,TCT_HASH_BLOCK_SIZE);
info->SIMPLE_WORDS=0;
info->COMPOUND_WORDS=0;
info->UNKNOWN_WORDS=0;
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
free_buffer(info->buffer);
free_bit_array(info->part_of_a_word);
free_bit_array(info->simple_word);
free(info->n_occurrences);
free_tct_hash(info->tct_h);
free(info);
}


/**
 * This function launches the application of the given .bin dictionary.
 * 
 * @author Alexis Neme
 * Modified by Sébastien Paumier
 */
void dico_application(char* name_bin,struct dico_application_info* info,int priority) {
char name_inf[FILENAME_MAX];
remove_extension(name_bin,name_inf);
strcat(name_inf,".inf");
/* We load the .inf file */
info->inf=load_INF_file(name_inf);
if (info->inf==NULL) {
   error("Cannot open %s\n",name_inf);
   return;
}
/* We load the .bin file */
info->bin=load_BIN_file(name_bin);
if (info->bin==NULL) {
   free_INF_codes(info->inf);
   error("Cannot open %s\n",name_bin);
   return;
}
info->word_array=new_word_struct_array(info->tokens->N);
/* And then we look simple and then compound words.
 * IMPORTANT: it is crucial to look for simple words first, since
 *            some initializations are made there that are used
 *            when looking for compound words.
 */
printf("Looking for simple words...\n");
look_for_simple_words(info,priority);
printf("Looking for compound words...\n");
/* We measure the elapsed time */
clock_t startTime=clock();
look_for_compound_words(info,priority);
clock_t endTime = clock();
double  elapsedTime = (double) (endTime - startTime);
if (DEBUG) printf("%2.8f seconds\n", elapsedTime);
free_word_struct_array(info->word_array);
free_INF_codes(info->inf);
free(info->bin);
}


/**
 * @author Alexis Neme
 * Modified by Sébastien Paumier
 */
int merge_dic_locate_results(struct dico_application_info* info,char* concord_filename,
                             int priority) {
/* This array is used to represent a compound word at a token sequence ended by -1.
 * Example: cinquante-deux could be represented by (1347,35,582,-1) */
int token_tab_coumpounds[TOKENS_IN_A_COMPOUND];
printf("Merging dic/locate result...\n");
/* First, we load the match list */
FILE* f=u_fopen(concord_filename,U_READ);
if (f==NULL) {
   error("Cannot open %s\n",concord_filename);
   return 0;
}
struct liste_matches* l=load_match_list(f,NULL);
u_fclose(f);
while (l!=NULL) {
   /* We test if the match is a valid dictionary entry */
   struct dela_entry* entry=tokenize_DELAF_line(l->output,1);
   if (entry!=NULL) {
      /* If the entry is valid */
      if(l->debut==l->fin) {  
         /* If it is a simple word */
         int token_number=get_token_number(entry->inflected,info->tokens);
         int p=get_value(info->simple_word,token_number);
         if (p==0 || p==priority) {
            /* We save the simple word only if it hasn't already been processed with
             * a greater priority */
            set_value(info->part_of_a_word,token_number,priority);             
            set_value(info->simple_word,token_number,priority);
            /* We save it to the DLF */
            u_fprints(l->output,info->dlf);
            u_fprints_char("\n",info->dlf);
         }
      }
      else if(l->debut<l->fin)    {
         /* If it is a compound word, we turn it into a token sequence 
          * ended by -1 */
         build_complex_token_tab(entry->inflected,info->tokens,token_tab_coumpounds);
         int w=was_allready_in_tct_hash(token_tab_coumpounds,info->tct_h,priority);
         if (w==0 || w==priority) {
            /* We save the compound word only if it hasn't already been processed
             * with a greater priority */
            for (int k=0;token_tab_coumpounds[k]!=-1;k++) {
               /* If we have matched a compound word, then all its part all not
                * unknown words */
               set_value(info->part_of_a_word,token_tab_coumpounds[k],priority);
            }
            /* We save it to the DLC */
            u_fprints(l->output,info->dlc);
            u_fprints_char("\n",info->dlc);
         }
      } else {
         error("Invalid match in concord.ind\n");
      }
      /* Finally, we free the entry */
      free_dela_entry(entry);
   }
   /* If the match is not a valid entry, an error message has already
    * been produced by tokenize_DELAF_line, so there is nothing to do. */
   l=l->suivant;
}
return 1;
}


/**
 * This function reads the whole 'tokens.cod' file and computes the number of
 * occurrences of each token.
 */
void count_token_occurrences(struct dico_application_info* info) {
fseek(info->text_cod,0,SEEK_SET);
int* buffer=info->buffer->buffer;
do {
   fill_buffer(info->buffer,info->text_cod);
   for (int i=0;i<info->buffer->size;i++) {
      info->n_occurrences[buffer[i]]++;
   }
} while (!info->buffer->end_of_file);
}


