 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "Copyright.h"
#include "Fst2Automaton.h"
#include "ElagFstFilesIO.h"
#include "Error.h"
#include "File.h"
#include "LanguageDefinition.h"
#include "SingleGraph.h"
#include "getopt.h"
#include "String_hash.h"
#include "List_int.h"



static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: TagsetNormTfst [OPTIONS] <tfst>\n"
         "\n"
         "  <tfst>: text automaton to normalize\n"
         "\n"
         "OPTIONS:\n"
         "  -o OUT/--output=OUT: specifies the output .tfst file. By default, the input\n"
         "                       text automaton is modified.\n"
         "  -t TAGSET/--tagset=TAGSET: tagset description file\n"
         "  -h/--help: this help\n"
         "\n"
         "Normalizes the specified text automaton according to the tagset description\n"
         "file, discarding undeclared dictionary codes and incoherent lexical entries.\n"
         "Inflectional features are unfactorized so that '{rouge,.A:fs:ms}' will be\n"
         "divided into the 2 tags '{rouge,.A:fs}' and '{rouge,.A:ms}'.\n"
         "The input text automaton is modified if -o option is not used.\n");
}


int get_tfst_tag_index(vector_ptr*,unichar*,int,int,int,int);


int main_TagsetNormTfst(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":o:t:h";
const struct option lopts[]= {
      {"output",required_argument,NULL,'o'},
      {"tagset",required_argument,NULL,'t'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
char tfst[FILENAME_MAX]="";
char tind[FILENAME_MAX]="";
char output_tfst[FILENAME_MAX]="";
char output_tind[FILENAME_MAX]="";
char tagset[FILENAME_MAX]="";
int val,index=-1;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'o': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(output_tfst,optarg);
             remove_extension(output_tfst,output_tind);
             strcat(output_tind,".tind");
             break;      
   case 't': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty tagset file name\n");
             }
             strcpy(tagset,optarg);
             break;      
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",optopt); 
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",optopt); 
             else fatal_error("Invalid option --%s\n",optarg);
             break;
   }
   index=-1;
}

if (optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

if (tagset[0]=='\0') {
   fatal_error("You must specify the tagset file\n");
}
strcpy(tfst,argv[optind]);
remove_extension(argv[optind],tind);
strcat(tind,".tind");
int no_explicit_output=0;
if (output_tfst[0]=='\0') {
   no_explicit_output=1;
   sprintf(output_tfst,"%s.new",tfst);
   sprintf(output_tind,"%s.new",tind);
}

u_printf("Loading tagset...\n");
language_t* language=load_language_definition(tagset);
set_current_language(language);

Elag_Tfst_file_in* txtin=load_tfst_file(tfst,LANGUAGE);
if (txtin==NULL) {
   fatal_error("Unable to load text automaton '%s'\n",tfst);
}

FILE* out_tfst=u_fopen(output_tfst,U_WRITE);
if (out_tfst==NULL) {
   fatal_error("Unable to open text automaton '%s'\n",output_tfst);
}
u_fprintf(out_tfst,"%010d\n",txtin->tfst->N);
FILE* out_tind=fopen(output_tind,"wb");
if (out_tind==NULL) {
   fatal_error("Unable to open text automaton index '%s'\n",output_tind);
}

u_printf("Cleaning text automaton...\n");
unichar foo[4096];
for (int current_sentence=1;current_sentence<=txtin->tfst->N;current_sentence++) {
   load_sentence(txtin->tfst,current_sentence);
   /* The 'renumbering' array is used to indicate that for
    * any transition tagged by the tag #i, we must add transitions tagged by the
    * integers of 'renumbering[i]'. As some tags will be inserted, we must remember
    * the original size of the tags array. */ 
   int original_number_of_tags=txtin->tfst->tags->nbelems;
   struct list_int* renumbering[original_number_of_tags];
   /* We clean all the tags. A rejected tag is marked with its content set to NULL */
   for (int i=0;i<original_number_of_tags;i++) {
      renumbering[i]=NULL;
      TfstTag* t=(TfstTag*)(txtin->tfst->tags->tab[i]);
      if (t->type==T_STD && t->content[0]=='{' && t->content[1]!='\0') {
         /* If the tag is a dictionary entry */
         struct dela_entry* e=tokenize_tag_token(t->content);
         if (e==NULL) continue;
         e=filter_dela_entry(e,t->content,LANGUAGE,1);
         if (e==NULL) {
            /* We note that the entry has been rejected by the tagset by
             * setting the tag content to NULL */
            free(t->content);
            t->content=NULL;
         } else if (e->n_inflectional_codes<=1) {
            /* If there is at most one inflectional code, there is no need to explode the tag.
             * So, we just rebuild the tag according to the filtered entry */
            build_tag(e,NULL,foo);
            free(t->content);
            t->content=u_strdup(foo);
            free_dela_entry(e);
         } else {
            /* If there are several inflectional codes, we must explode them.
             * In that case, we create as many new TfstTag as necessary, and
             * we set the current one to NULL. This, in order to avoid bugs, because
             * if we have "{jeunes,jeune.A:ms:fs}", replacing it by "{jeunes,jeune.A:ms}"
             * and creating, if necessary, a new tag "{jeunes,jeune.A:fs}", could lead
             * to a bug if the tag "{jeunes,jeune.A:ms}" already exists. */
            txtin->tfst->tags->tab[i]=NULL;
            int N=e->n_inflectional_codes;
            e->n_inflectional_codes=1;
            for (int j=0;j<N;j++) {
               /* We don't want to recreate many dela_entry */
               unichar* old_first_code=e->inflectional_codes[0];
               e->inflectional_codes[0]=e->inflectional_codes[j];
               build_tag(e,NULL,foo);
               int index=get_tfst_tag_index(txtin->tfst->tags,foo,t->start_pos_token,t->start_pos_char,
                     t->end_pos_token,t->end_pos_char);
               renumbering[i]=new_list_int(index,renumbering[i]);
               e->inflectional_codes[0]=old_first_code;
            }
         }
      }
   }
   SingleGraph g=txtin->tfst->automaton;
   TfstTag** tags=(TfstTag**)(txtin->tfst->tags->tab);
   unichar tmp[4096];
   struct string_hash* tmp_tags=new_string_hash(txtin->tfst->tags->nbelems);
   unichar EPSILON[]={'@','<','E','>','\n','.','\n','\0'};
   /* The epsilon tag is always the first one */ 
   get_value_index(EPSILON,tmp_tags);
   for (int i=0;i<g->number_of_states;i++) {
      Transition** t=&(g->states[i]->outgoing_transitions);
      while ((*t)!=NULL) {
         if (tags[(*t)->tag_number]==NULL) {
            /* A NULL tag means that the tag must be replaced by a list of new tags.
             * First, we recycle the current transition. */
            struct list_int* x=renumbering[(*t)->tag_number];
            TfstTag_to_string(tags[x->n],tmp);
            (*t)->tag_number=get_value_index(tmp,tmp_tags);
            /* Then, we create the following ones */
            x=x->next;
            while (x!=NULL) {
               TfstTag_to_string(tags[x->n],tmp);
               int tag_number=get_value_index(tmp,tmp_tags);
               (*t)->next=new_Transition(tag_number,(*t)->state_number,(*t)->next);
               t=&((*t)->next);
               x=x->next;
            }
            /* Finally, we go on the next transition */
            t=&((*t)->next);
         } else if (tags[(*t)->tag_number]->content==NULL) {
            /* If we must remove the current transition, because its
             * tag has been rejected by the tagset */
            Transition* foo=(*t);
            (*t)=(*t)->next;
            free_Transition(foo,NULL);
            t=&(*t);
         } else if (renumbering[(*t)->tag_number]==NULL) {
            /* If we have not modified the original transition */
            TfstTag_to_string(tags[(*t)->tag_number],tmp);
            (*t)->tag_number=get_value_index(tmp,tmp_tags);
            t=&((*t)->next);
         } else {
            /* Should not happen */
            fatal_error("Internal error in TagsetNormTfst\n");
         }
      }
   }
   /* We have to free the renumbering array */
   for (int i=0;i<original_number_of_tags;i++) {
      free_list_int(renumbering[i]);
   }
   trim(g);
   if (g->number_of_states!=0) {
      minimize(g,1);
   }
   if (g->number_of_states==0) {
      error("Sentence %d is empty\n",current_sentence);
      SingleGraphState initial=add_state(g);
      set_initial_state(initial);
      free_vector_ptr(txtin->tfst->tags,(void (*)(void*))free_TfstTag);
      txtin->tfst->tags=new_vector_ptr(1);
      vector_ptr_add(txtin->tfst->tags,new_TfstTag(T_EPSILON));
      save_current_sentence(txtin->tfst,out_tfst,out_tind,NULL,0);
   } else {
      save_current_sentence(txtin->tfst,out_tfst,out_tind,(unichar**)tmp_tags->value,tmp_tags->size);
   }
   free_string_hash(tmp_tags);
   if (current_sentence%100==0) {
      u_printf("Sentence %d/%d ...      \r",current_sentence,txtin->tfst->N);
   }
}
u_printf("Sentence %d/%d.\nDone: text automaton is normalized.\n",txtin->tfst->N,txtin->tfst->N);
tfst_file_close_in(txtin);
u_fclose(out_tfst);
fclose(out_tind);

/* Finally, we rename files if we must modify the input text automaton */
if (no_explicit_output) {
   remove(tfst);
   remove(tind);
   rename(output_tfst,tfst);
   rename(output_tind,tind);
}
return 0;
}


/**
 * Returns the index of the TfstTag corresponding to the given information,
 * adding it to the given vector if necessary.
 */
int get_tfst_tag_index(vector_ptr* tags,unichar* content,
                       int start_pos_token,int start_pos_char,
                       int end_pos_token,int end_pos_char) {
for (int i=0;i<tags->nbelems;i++) {
   TfstTag* t=(TfstTag*)(tags->tab[i]);
   if (t!=NULL 
         /* We must test for NULL, because some tags may have been removed */
    && t->type==T_STD
    && !u_strcmp(t->content,content)
    && t->start_pos_token==start_pos_token
    && t->start_pos_char==start_pos_char
    && t->end_pos_token==end_pos_token
    && t->end_pos_char==end_pos_char) {
      return i;
   }
}
/* We must create it */
TfstTag* t=new_TfstTag(T_STD);
t->content=u_strdup(content);
t->start_pos_token=start_pos_token;
t->start_pos_char=start_pos_char;
t->end_pos_token=end_pos_token;
t->end_pos_char=end_pos_char;
return vector_ptr_add(tags,t);
}

