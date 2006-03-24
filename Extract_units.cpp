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
#include "Concordance.h"
#include "Extract_units.h"
//---------------------------------------------------------------------------



int sentence_buffer[MAX_TOKENS_BY_SENTENCE];

void extract_units(char yes_no,FILE* text,struct text_tokens* tok,
                   FILE* concord,FILE* result) {
int i,BUFFER_LENGTH,N_TOKENS_READ;
int current_beginning,current_end,RESULT;

struct liste_matches* l=load_match_list(concord,&i);
current_end=-1;

//int sentence=1;

read_one_sentence(buffer,text,tok,&BUFFER_LENGTH,&N_TOKENS_READ);
printf("Extracting %smatching units...\n",yes_no?"":"un");
while (BUFFER_LENGTH!=0) {
   current_beginning=current_end+1;
   current_end=current_end+N_TOKENS_READ;
   l=is_a_match_in_the_sentence(l,&RESULT,current_beginning,current_end);
   if ((RESULT && yes_no) || (!RESULT && !yes_no)) {
      // if we must print this sentence, we print it
      for (i=0;i<BUFFER_LENGTH;i++) {
         u_fprints(tok->token[buffer[i]],result);
      }
      u_fprints_char("\n",result);
   }
   if (l==NULL && yes_no) {
      // if there is no more match and if we must extract the matching units,
      // we can stop
      return;
   }
   read_one_sentence(buffer,text,tok,&BUFFER_LENGTH,&N_TOKENS_READ);
   //sentence++;
}
printf("Done.\n");
}



//
// this function reads a sentence in the text;
// BUFFER_SIZE returns the length of the sentence, in the limit of MAX_TOKENS_BY_SENTENCE
// N_TOKENS_READ returns the length of tokens actually read (can be more than MAX_TOKENS_BY_SENTENCE)
//
void read_one_sentence(int buffer[],FILE* text,struct text_tokens* tok,
                      int* BUFFER_LENGTH,int* N_TOKENS_READ) {
int i=0;
int t=-15;
int res=-15;
while ((res=fread(&t,sizeof(int),1,text)) && (t!=tok->SENTENCE_MARKER) && (i!=MAX_TOKENS_BY_SENTENCE)) {
   buffer[i++]=t;
}
if (i==MAX_TOKENS_BY_SENTENCE) {
   fprintf(stderr,"Sentence too long to be entirely displayed\n");
   // we show the error and we read the sentence till its end in order
   // to keep a valid sentence numerotation
   while ((1==fread(&t,sizeof(int),1,text)) && (t!=tok->SENTENCE_MARKER)) i++;
   (*BUFFER_LENGTH)=MAX_TOKENS_BY_SENTENCE;
   (*N_TOKENS_READ)=i;
   return;
}
if (res!=0 && t==tok->SENTENCE_MARKER && i!=MAX_TOKENS_BY_SENTENCE) {
    buffer[i++]=t;
}
(*BUFFER_LENGTH)=i;
(*N_TOKENS_READ)=i;
}



//
// RESULT returns true if at least one match of L has its beginning in the
// range [beginning,end], false otherwise;
// the function returns the match list where all the matches in the range
// where removed
//
struct liste_matches* is_a_match_in_the_sentence(struct liste_matches* L,int* RESULT,int beginning,int end) {
if (L==NULL) {
   // case of an empty match list
   (*RESULT)=0;
   return NULL;
}
if (L->fin < beginning) {
   fprintf(stderr,"Error in the function is_a_match_in_the_sentence\n");
   exit(1);
}
if (L->debut > end) {
   // if the match starts after the end of the range, we remove nothing and return false
   (*RESULT)=0;
   return L;
}
// we are now in the case of a match within the range
// we must return true
(*RESULT)=1;
// and remove all the matches within the range
struct liste_matches* tmp;
while (L!=NULL && L->debut<=end) {
   tmp=L;
   L=L->suivant;
   free_liste_matches(tmp);
}
return L;
}
