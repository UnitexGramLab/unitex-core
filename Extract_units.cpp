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
#include "Extract_units.h"
#include "Buffer.h"
#include "Matches.h"
#include "Error.h"
//---------------------------------------------------------------------------


#define MAX_TOKENS_BY_SENTENCE 100000


void read_one_sentence(struct buffer*,FILE*,struct text_tokens*,int*);
struct liste_matches* is_a_match_in_the_sentence(struct liste_matches*,int*,int,int);


/**
 * This function takes a concordance index file and a .snt text file and it builds
 * an output file 'result' made of all sentences that contain an
 * occurrence. if 'extract_matching_units' is null, the function extracts 
 * all sentences that do NOT contain any occurrence.
 */
void extract_units(char extract_matching_units,FILE* snt,struct text_tokens* tokens,
                   FILE* concord,FILE* result) {
int i,N_TOKENS_READ;
int current_beginning,current_end,RESULT;
/* We load the match list */
struct liste_matches* l=load_match_list(concord,&i);
current_end=-1;

struct buffer* buffer=new_buffer(MAX_TOKENS_BY_SENTENCE,INTEGER_BUFFER);
read_one_sentence(buffer,snt,tokens,&N_TOKENS_READ);
printf("Extracting %smatching units...\n",extract_matching_units?"":"un");
while (buffer->size!=0) {
   current_beginning=current_end+1;
   current_end=current_end+N_TOKENS_READ;
   l=is_a_match_in_the_sentence(l,&RESULT,current_beginning,current_end);
   if ((RESULT && extract_matching_units) || (!RESULT && !extract_matching_units)) {
      /* if we must print this sentence, we print it */
      for (i=0;i<buffer->size;i++) {
         u_fprints(tokens->token[buffer->int_buffer[i]],result);
      }
      u_fprints_char("\n",result);
   }
   if (l==NULL && extract_matching_units) {
      /* If there is no more match and if we must extract the matching units,
       * we can stop */
      free_buffer(buffer);
      return;
   }
   read_one_sentence(buffer,snt,tokens,&N_TOKENS_READ);
}
free_buffer(buffer);
}


/**
 * This function reads a sentence in the text.
 * buffer->size will contain the length of the sentence, in the limit of MAX_TOKENS_BY_SENTENCE
 * N_TOKENS_READ returns the length of tokens actually read (can be more than MAX_TOKENS_BY_SENTENCE)
 */
void read_one_sentence(struct buffer* buffer,FILE* text,struct text_tokens* tok,
                       int* N_TOKENS_READ) {
int i=0;
int t=-15;
int res=-15;
while ((res=fread(&t,sizeof(int),1,text)) && (t!=tok->SENTENCE_MARKER) && (i!=MAX_TOKENS_BY_SENTENCE)) {
   buffer->int_buffer[i++]=t;
}
if (i==MAX_TOKENS_BY_SENTENCE) {
   error("Sentence too long to be entirely displayed\n");
   /* We show the error and we read the sentence till its end in order
    * to keep a valid sentence numerotation */
   while ((1==fread(&t,sizeof(int),1,text)) && (t!=tok->SENTENCE_MARKER)) i++;
   buffer->size=MAX_TOKENS_BY_SENTENCE;
   (*N_TOKENS_READ)=i;
   return;
}
if (res!=0 && t==tok->SENTENCE_MARKER && i!=MAX_TOKENS_BY_SENTENCE) {
    buffer->int_buffer[i++]=t;
}
buffer->size=i;
(*N_TOKENS_READ)=i;
}


/**
 * RESULT returns 1 if at least one match of L has its beginning in the
 * range [beginning,end], 0 otherwise.
 * The function returns the match list where all the matches in the range
 * where removed.
 */
struct liste_matches* is_a_match_in_the_sentence(struct liste_matches* L,int* RESULT,int beginning,int end) {
if (L==NULL) {
   /* case of an empty match list */
   (*RESULT)=0;
   return NULL;
}
if (L->fin < beginning) {
   fatal_error("Error in the function is_a_match_in_the_sentence\n");
}
if (L->debut > end) {
   /* If the match starts after the end of the range, we remove nothing and return 0 */
   (*RESULT)=0;
   return L;
}
/* We are now in the case of a match within the range, we must return 1 */
(*RESULT)=1;
/* And we remove all the matches within the range */
struct liste_matches* tmp;
while (L!=NULL && L->debut<=end) {
   tmp=L;
   L=L->suivant;
   free_liste_matches(tmp);
}
return L;
}

