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

#include "Fst2_to_Mft.h"
#include "Error.h"

//
// this function produces a mft file from a fst2 file
//
void convert_fst2_to_mft(Fst2* fst2,FILE* f) {
for (int i=1;i<=fst2->number_of_graphs;i++) {
   convert_and_save_sentence(fst2,i,f);
}
u_fprints_char("# end of text",f);
}



//
// this function copy src into dest unspecializing % signs with a backslash
//
void unspecialize_percent_signs(unichar* src,unichar* dest) {
int i=0;
int j=0;
while (src[i]!='\0') {
   if (src[i]=='%') {
      dest[j++]='\\';
   }
   dest[j++]=src[i++];
}
dest[j]='\0';
}



//
// this function converts one sentence from the FST2 format to the MFT format
//
void convert_and_save_sentence(Fst2* fst2,int N,FILE* f) {
char temp[1000];
sprintf(temp,"# Sentence #%d\n",N);
u_fprints_char(temp,f);
struct string_hash* tags=new_string_hash();
// first, we insert every tag in the string_hash
int limite=fst2->initial_states[N]+fst2->number_of_states_per_graphs[N];
for (int i=fst2->initial_states[N];i<limite;i++) {
   struct fst2Transition* trans=fst2->states[i]->transitions;
   while (trans!=NULL) {
      if (trans->tag_number < 0) {
         // if there is a subgraph call
         error("Error: a subgraph call was found in the text automaton.\n");
         error("This transition will be removed in the MFT file.\n");
         struct fst2Transition* tmp=trans->next;
         free(trans);
         trans=tmp;
      }
      else {
         get_value_index(fst2->tags[trans->tag_number]->input,tags);
         trans=trans->next;
      }
   }
}
// we write the number of tags and the number of states in the MFT
sprintf(temp,"%d %d\n",tags->size,fst2->number_of_states_per_graphs[N]);
u_fprints_char(temp,f);
unichar TMP[2000];
for (int i=0;i<tags->size;i++) {
   u_fprints_char("%",f);
   unspecialize_percent_signs(tags->value[i],TMP);
   u_fprints(TMP,f);
}
u_fprints_char("%\n",f);
// then, we write the content of the states
for (int i=fst2->initial_states[N];i<limite;i++) {
   if (is_final_state(fst2->states[i])) {
      u_fprints_char("t ",f);
   }
   else {
      u_fprints_char(": ",f);
   }
   struct fst2Transition* trans=fst2->states[i]->transitions;
   int debut=fst2->initial_states[N];
   int num;
   while (trans!=NULL) {
      num=get_value_index(fst2->tags[trans->tag_number]->input,tags);
      sprintf(temp,"%d %d ",num,(trans->state_number)-debut+1);
      u_fprints_char(temp,f);
      trans=trans->next;
   }
   u_fprints_char("-1\n",f);
}
u_fprints_char("f\n",f);
free_string_hash(tags);
}

