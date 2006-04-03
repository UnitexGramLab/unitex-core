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


//
// this function produces a mft file from a fst2 file
//
void convert_fst2_to_mft(Automate_fst2* fst2,FILE* f) {
for (int i=1;i<=fst2->nombre_graphes;i++) {
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
void convert_and_save_sentence(Automate_fst2* fst2,int N,FILE* f) {
char temp[1000];
sprintf(temp,"# Sentence #%d\n",N);
u_fprints_char(temp,f);
struct string_hash* tags=new_string_hash();
// first, we insert every tag in the string_hash
int limite=fst2->debut_graphe_fst2[N]+fst2->nombre_etats_par_grf[N];
for (int i=fst2->debut_graphe_fst2[N];i<limite;i++) {
   struct transition_fst* trans=fst2->etat[i]->transitions;
   while (trans!=NULL) {
      if (trans->etiquette < 0) {
         // if there is a subgraph call
         fprintf(stderr,"Error: a subgraph call was found in the text automaton.\n");
         fprintf(stderr,"This transition will be removed in the MFT file.\n");
         struct transition_fst* tmp=trans->suivant;
         free(trans);
         trans=tmp;
      }
      else {
         get_hash_number(fst2->etiquette[trans->etiquette]->input,tags);
         trans=trans->suivant;
      }
   }
}
// we write the number of tags and the number of states in the MFT
sprintf(temp,"%d %d\n",tags->N,fst2->nombre_etats_par_grf[N]);
u_fprints_char(temp,f);
unichar TMP[2000];
for (int i=0;i<tags->N;i++) {
   u_fprints_char("%",f);
   unspecialize_percent_signs(tags->tab[i],TMP);
   u_fprints(TMP,f);
}
u_fprints_char("%\n",f);
// then, we write the content of the states
for (int i=fst2->debut_graphe_fst2[N];i<limite;i++) {
   if (fst2->etat[i]->control & FST2_FINAL_STATE_BIT_MASK) {
      u_fprints_char("t ",f);
   }
   else {
      u_fprints_char(": ",f);
   }
   struct transition_fst* trans=fst2->etat[i]->transitions;
   int debut=fst2->debut_graphe_fst2[N];
   int num;
   while (trans!=NULL) {
      num=get_hash_number(fst2->etiquette[trans->etiquette]->input,tags);
      sprintf(temp,"%d %d ",num,(trans->arr)-debut+1);
      u_fprints_char(temp,f);
      trans=trans->suivant;
   }
   u_fprints_char("-1\n",f);
}
u_fprints_char("f\n",f);
free_string_hash(tags);
}

