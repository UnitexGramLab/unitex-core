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

#include <stdio.h>
#include "Unicode.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Fst2.h"
#include "String_hash.h"
#include "DELA.h"
#include "List_pointer.h"
#include "Error.h"
#include "Transitions.h"


void implode(Fst2*,FILE*);


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: ImploseFst2 <txtauto> -o <out>\n"
         "\n"
         "where :\n"
         " <txtauto>     : input text automaton FST2 file,\n"
         " <out>         : resulting text automaton\n"
         "\n"
         "Implodes the specified text automaton by merging together lexical entries\n"
         "which only differ in their inflectional features.\n"
         "The resulting text automaton is stored in <out>.\n\n");
}


int main(int argc, char ** argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();  

char* txtname=NULL;
char* outname=NULL;
argv++;
argc--;
if (argc==0) {
   usage();
   return 0;
}
while (argc) {
   if (**argv!='-') {
      txtname=(*argv);
   } else {
      if (!strcmp(*argv,"-h")) {
         usage();
         return 0;
      }
      else if (!strcmp(*argv,"-o")) {
         *argv++;
         argc--;
         if (argc==0) {
            fatal_error("'-o' needs an additionnal argument\n");
         }
         outname=*argv;
      }
   }
   *argv++;
   argc--;
}
if (txtname==NULL) {
   fatal_error("No text automaton specified\n");
}
if (outname==NULL) {
   int len=strlen(txtname);
   outname=(char*)malloc((len+10)*sizeof(char));
   if (outname==NULL) {
      fatal_error("Not enough memory in main\n");
   }
   strcpy(outname,txtname);
   if (!strcmp(txtname+len-5,".fst2")) {
      strcpy(outname+len-5,"-imp.fst2");
   } else {
      strcat(outname,"-imp.fst2");
   }
}
u_printf("Loading '%s'...\n",txtname);
Fst2* txtauto=load_fst2(txtname,1);
if (txtauto==NULL) {
   fatal_error("Unable to load '%s'\n",txtname);
}
FILE* output=u_fopen(outname,U_WRITE);
if (output==NULL) {
   fatal_error("Cannot open '%s' for writing\n",outname);
}
implode(txtauto,output);
u_fclose(output);
free_Fst2(txtauto);
return 0;
}


/**
 * Returns  0 if a=b
 *         <0 if a<b
 *         >0 if a>b
 * Note that and b are supposed to be non NULL.
 */
int compare_for_implode(Transition* a,Transition* b) {
if (a->state_number==b->state_number) {
   return a->tag_number-b->tag_number;
}
return a->state_number-b->state_number;
}


/**
 * Inserts the given transition into the given transition list.
 * Removes duplicates.
 */
Transition* sorted_insert_for_implode(Transition* list,Transition* element) {
if (list==NULL) {
   element->next=NULL;
   return element;
}
int res=compare_for_implode(list,element);
if (res==0) {
   free_Transition_list(element);
   return list;
}
if (res>0) {
   /* If we must insert at the beginning of the list */
   element->next=list;
   return element;
}
list->next=sorted_insert_for_implode(list->next,element);
return list;
}


/**
 * Sorts the given transition list in increasing order
 * 1) by destination state and 2) by tag number.
 */
Transition* sort_for_implode(Transition* list) {
Transition* result=NULL;
while (list!=NULL) {
   Transition* tmp=list;
   list=list->next;
   result=sorted_insert_for_implode(result,tmp);
}
return result;
}


/**
 * This function takes a list of DELA entries and an entry to merge with them.
 */
struct list_pointer* merge_entry(struct list_pointer* list,struct dela_entry* entry) {
if (list==NULL) {
   return new_list_pointer(entry,NULL);
}
struct dela_entry* e=(struct dela_entry*)list->pointer;
/* If we can merge the entry with the current list element */
if (!u_strcmp(e->inflected,entry->inflected) && !u_strcmp(e->lemma,entry->lemma)
    && same_semantic_codes(e,entry)) {
   merge_inflectional_codes(e,entry);
   /* We can free the entry */
   free_dela_entry(entry);
   return list;
}
list->next=merge_entry(list->next,entry);
return list;
}


/**
 * This function takes a .fst2 representing a text automaton
 * and implodes its tags, i.e. it merges transitions that point
 * on the same state and whose tags share the same semantic
 * codes. For instance, if there are two transition from X to Y
 * that are tagged with "{rouge,.A:fs}" and "{rouge,.A:ms}", then
 * we will merge them into the single one "{rouge,.A:fs:ms}".
 * The resulting .fst2 is stored into the given file. 'output'
 * is supposed to have been opened but not written in.
 */
void implode(Fst2* fst2,FILE* output) {
u_printf("Imploding tags...\n");
struct string_hash* new_tags=new_string_hash();
unichar epsilon[4]={'<','E','>','\0'};
get_value_index(epsilon,new_tags);
unichar tag[4096];
u_fprintf(output,"%010d\n",fst2->number_of_graphs);
for (int i=1;i<=fst2->number_of_graphs;i++) {
   if (i%100==0) {
      u_printf("Sentence %d/%d              \r",i,fst2->number_of_graphs);
   }
   u_fprintf(output,"-%d %S\n",i,fst2->graph_names[i]);
   int initial_state=fst2->initial_states[i];
   int max_states=initial_state+fst2->number_of_states_per_graphs[i];
   for (int j=initial_state;j<max_states;j++) {
      /* For each state state of each automaton, we try to
       * implode the tags of the transition list */
      Fst2State state=fst2->states[j];
      u_fprintf(output,"%C ",is_final_state(state)?'t':':');
      state->transitions=sort_for_implode(state->transitions);
      int previous_state=-1;
      struct list_pointer* entry_list=NULL;
      for (Transition* t=state->transitions;;t=t->next) {
         if (previous_state!=-1 && (t==NULL || t->state_number!=previous_state)) {
            /* If we have finished to deal with a set of transitions
             * pointing on the same state number, then we have to save
             * the corresponding transition list */
            while (entry_list!=NULL) {
               struct dela_entry* entry=(struct dela_entry*)entry_list->pointer;
               build_tag(entry,NULL,tag);
               free_dela_entry(entry);
               u_fprintf(output,"%d %d ",get_value_index(tag,new_tags),previous_state-initial_state);
               struct list_pointer* ptr=entry_list;
               entry_list=entry_list->next;
               free(ptr);
            }
         }
         if (t==NULL) break;
         previous_state=t->state_number;
         unichar* input=fst2->tags[t->tag_number]->input;
         if (input[0]=='{' && input[1]!='\0') {
            /* We don't test {S} or {STOP} since we are supposed
             * to manipulate a text automaton that should not
             * contain such things */
            struct dela_entry* entry=tokenize_tag_token(input);
            entry_list=merge_entry(entry_list,entry);
         } else {
            /* If we have a tag that is not a DELA entry, we just add
             * the corresponding transition */
            u_fprintf(output,"%d %d ",get_value_index(input,new_tags),previous_state-initial_state);
         }
      }
      u_fprintf(output,"\n");
   }
   u_fprintf(output,"f \n");
}
/* We save the tags */
for (int i=0;i<new_tags->size;i++) {
   u_fprintf(output,"%%%S\n",new_tags->value[i]);
}
u_fprintf(output,"f\n");
free_string_hash(new_tags);
u_printf("Done.                          \n");
}

