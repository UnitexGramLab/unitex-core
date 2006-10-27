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
#include "Loading_dic.h"
#include "Error.h"
#include "BitArray.h"
//---------------------------------------------------------------------------


struct noeud *racine=nouveau_noeud();

int est_un_mot_simple(unichar* m,Alphabet* alph,int tokenization_mode) {
int i;
i=0;
if (tokenization_mode==CHAR_BY_CHAR_TOKENIZATION && u_strlen(m)>1) return 0;

while (m[i]!='\0' /*&& m[i]!='<' && m[i]!='>' && m[i]!='_'*/ && is_letter(m[i],alph)) {
   i++;
}
if (m[i]=='\0') {
   return 1;
}
return 0;
}


/**
 * This function loads a DLF or a DLC. It computes information about tokens
 * that will be used during the Locate operation. For instance, if we have the
 * following line:
 * 
 *   extended,.A
 * 
 * and if the .fst2 to be applied to the text contains the pattern <A> with,
 * number 456, then the function will mark the "extended" token to be matched 
 * by the pattern 456. Moreover, all case variations will be taken into account,
 * so that the "Extended" and "EXTENDED" tokens will also be updated.
 * 
 * The three parameters 'is_DIC_pattern', 'is_CDIC_pattern' and 'is_SDIC_pattern'
 * indicates if the .fst2 contains the corresponding patterns. For instance, if
 * the pattern "<SDIC>" is used in the grammar, it means that any token that is a
 * simple word must be marked as be matched by this pattern.
 */
void load_dic_for_locate(char* dic_name,Alphabet* alphabet,struct string_hash* tokens,
                         int number_of_patterns,int is_DIC_pattern,
                         int is_CDIC_pattern,int is_SDIC_pattern,
                         struct DLC_tree_info* DLC_tree,int tokenization_mode) {
FILE* f;
unichar line[DIC_LINE_SIZE];
f=u_fopen(dic_name,U_READ);
if (f==NULL) {
   error("Cannot open dictionary %s\n",dic_name);
   return;
}
/* We parse all the lines */
while (u_read_line(f,line)) {
   if (line[0]=='/') {
      /* NOTE: DLF and DLC files are not supposed to contain comment
       *       lines, but we test them, just in the case */
      continue;
   }
   struct dela_entry* entry=tokenize_DELAF_line(line,1);
   if (entry==NULL) {
      /* This case should never happen */
      fatal_error("Invalid dictionary line in load_dic_for_locate\n");
   }
   /* We add the inflected form to the list of forms associated to the lemma.
    * This will be used to replace patterns like "<be>" by the actual list of
    * forms that can be matched by it, for optimization reasons */
   ajouter_forme_flechie(entry->lemma,0,racine,entry->inflected);
   /* We get the list of all tokens that can be matched by the inflected form of this
    * this entry, with regards to case variations (see the "extended" example above). */
   struct list_int* ptr=get_token_list_for_sequence(entry->inflected,alphabet,tokens);
   /* We save the list pointer to free it later */
   struct list_int* ptr_copy=ptr;
   /* Here, we will deal with all simple words */
   while (ptr!=NULL) {
      int i=ptr->n;
      /* If the current token can be matched, then it can be recognized by the "<DIC>" pattern */
      index_controle[i]=(unsigned char)(get_controle(tokens->tab[i],alphabet,NULL,tokenization_mode)|DIC_TOKEN_BIT_MASK);
      if (number_of_patterns) {
         /* We look for matching patterns only if there are some */
         int nothing_before=0;
         /* Then, we get the list of all grammatical patterns that can match the entry */
         if (matching_patterns[i]==NULL) {
            matching_patterns[i]=new_bit_array(number_of_patterns,ONE_BIT);
            nothing_before=1;
         }
         int number_of_matching_patterns=get_matching_patterns(entry,matching_patterns[i]);
         if (number_of_matching_patterns==0 && nothing_before) {
            /* WARNING! This is very important to test correctly when the bit array can
             * be emptied. For instance, let us consider the pattern "<A>" and the two dictionary
             * lines:
             * 
             * extended,.A
             * extended,extend.V:K:I1s:I2s:I3s:I1p:I2p:I3p
             * 
             * When processing the first line, we will see that is can be matched by 1
             * pattern (<A>). However, the second one will be matched by 0 pattern, so if we
             * don't take care, we will free the bit array of the "extended" token, whereas it
             * is actually matched by a pattern. */
            free_bit_array(matching_patterns[i]);
            matching_patterns[i]=NULL;
         }
      }
      ptr=ptr->next;
   }
   /* Finally, we free the token list */
   free_list_int(ptr_copy);
   if (!est_un_mot_simple(entry->inflected,alphabet,tokenization_mode)) {
      /* If the inflected form is a compound word */
      if (is_DIC_pattern || is_CDIC_pattern) {
         /* If the .fst2 contains "<DIC>" and/or "<CDIC>", then we
          * must note that all compound words can be matched by them */
         add_compound_word_with_no_pattern(entry->inflected,alphabet,tokens,DLC_tree,tokenization_mode);
      }
      if (number_of_patterns) {
         /* We look for matching patterns only if there are some */
         struct bit_array* code_gramm_temp=new_bit_array(number_of_patterns,ONE_BIT);
         /* We look if the compound word can be matched by some patterns */
         int number_of_matching_patterns=get_matching_patterns(entry,code_gramm_temp);
         if (number_of_matching_patterns==0) {
            /* No free condition here, since this is a temp variable */
            free_bit_array(code_gramm_temp);
         }
         else {
            /* If the word is matched by at least one pattern, we store it.
             * 'k' is here for optimization in the case where we look a few patterns within
             * a lot of possible ones (ex: number_of_patterns=245 and k=2) */
            int k=0;
            for (int j=0;j<number_of_patterns && k<number_of_matching_patterns;j++) {
               if (get_value(code_gramm_temp,j)) {
                  add_compound_word_with_pattern(entry->inflected,j,alphabet,tokens,DLC_tree,tokenization_mode);
                  k++;
               }
            }
            /* We don't forget to free the temp variable */
            free_bit_array(code_gramm_temp);
         }
      }
   }
   free_dic_entry(entry);
}
u_fclose(f);
}



struct noeud* nouveau_noeud() {
struct noeud* n;
n=(struct noeud*)malloc(sizeof(struct noeud));
n->controle=0;
n->numero=-1;
n->formes_flechies=NULL;
n->lettre=1;
n->l=NULL;
return n;
}



struct noeud* get_sous_noeud(struct noeud *n,unichar c,int creer) {
struct liste_feuilles* ptr;
struct liste_feuilles* ptr2;
struct noeud* res;
ptr=n->l;
while ((ptr!=NULL)&& (ptr->node!=NULL) && ((ptr->node)->lettre!=c)) {
  ptr=ptr->suivant;
}
if (ptr==NULL) {        // si on veut juste savoir si le noeud existe
  if (!creer) {          // et que le noeud n'existe pas, on renvoie NULL
     return NULL;
  }
  res=nouveau_noeud();
  res->lettre=c;
  ptr2=(struct liste_feuilles*)malloc(sizeof(struct liste_feuilles));
  ptr2->node=res;
  ptr2->suivant=NULL;
  if (n->l==NULL)
    n->l=ptr2;
  else {
         ptr=n->l;
         while (ptr->suivant!=NULL)
           ptr=ptr->suivant;
         ptr->suivant=ptr2;
       }
  return res;
}
return ptr->node;
}



void ajouter_flechi(struct noeud* n,unichar* s) {
int i,j,l,l2,debut;
unichar tmp[20000];
if (n->formes_flechies==NULL) {
  n->formes_flechies=(unichar*)malloc((u_strlen(s)+1)*sizeof(unichar));
  u_strcpy(n->formes_flechies,s);
  return;
}
tmp[0]='\0';
u_strcpy(tmp,n->formes_flechies);
i=0;
j=0;
debut=0;
l=u_strlen(tmp);
l2=u_strlen(s);
while ((i<l) && (j<l2)) {
  if (tmp[i++]==s[j]) {
     j++;
  }
  else {j=0;debut=i;}
  if ((j==l2) && (tmp[i]!=SEPARATOR_CHAR) && (tmp[i]!='\0')) {
     j=0;
     debut=i;
  }
}
if (((i==l) && (j<l2)) || (debut!=0 && tmp[debut-1]!=SEPARATOR_CHAR)) {
   j=0;
}
if (!j) {
  char t[2];
  sprintf(t,"%c",SEPARATOR_CHAR);
  u_strcat_char(tmp,t);
  u_strcat(tmp,s);
  free(n->formes_flechies);
  n->formes_flechies=(unichar*)malloc((u_strlen(tmp)+1)*sizeof(unichar));
  u_strcpy(n->formes_flechies,tmp);
}
}



void ajouter_forme_flechie(unichar* canonique,int i,struct noeud* n,unichar* flechi) {
struct noeud* sous_noeud;
sous_noeud=get_sous_noeud(n,canonique[i],1);
i++;
if (canonique[i]=='\0') {
   sous_noeud->controle=(unsigned char)(sous_noeud->controle|1);
   ajouter_flechi(sous_noeud,flechi);
}
else ajouter_forme_flechie(canonique,i,sous_noeud,flechi);
}



/**
 * This function checks for each tag token like "{extended,extend.V:K}"
 * if it verifies some patterns. Its behaviour is very similar to the one
 * of the load_dic_for_locate function. However, as a side effect, this
 * function fills 'tag_token_list' with the list of tag token numbers.
 * This list is later used during Locate preprocessings.
 */
void check_patterns_for_tag_tokens(Alphabet* alphabet,struct string_hash* tokens,int number_of_patterns,
									struct DLC_tree_info* DLC_tree,int tokenization_mode,
                           struct list_int** tag_token_list) {
for (int i=0;i<tokens->N;i++) {
   if (tokens->tab[i][0]=='{' && u_strcmp_char(tokens->tab[i],"{S}")  && u_strcmp_char(tokens->tab[i],"{STOP}")) {
      /* If the token is tag like "{today,.ADV}", we add its number to the tag token list */
      *tag_token_list=head_insert(i,*tag_token_list);
      /* And we look for the patterns that can match it */
      struct dela_entry* entry=tokenize_tag_token(tokens->tab[i]);
      if (entry==NULL) {
         /* This should never happen */
         fatal_error("Invalid tag token in function check_patterns_for_tag_tokens\n");
      }
      /* We add the inflected form to the list of forms associated to the lemma.
      * This will be used to replace patterns like "<be>" by the actual list of
      * forms that can be matched by it, for optimization reasons */
      ajouter_forme_flechie(entry->lemma,0,racine,tokens->tab[i]);
      index_controle[i]=(unsigned char)(get_controle(tokens->tab[i],alphabet,NULL,tokenization_mode)|DIC_TOKEN_BIT_MASK);
      if (number_of_patterns) {
         /* We look for matching patterns only if there are some */
         int nothing_before=0;
         if (matching_patterns[i]==NULL) {
            /* WARNING! See the note about freeing this bit array in the comments
             * of function load_dic_for_locate */
            matching_patterns[i]=new_bit_array(number_of_patterns,ONE_BIT);
            nothing_before=1;
         }
         int number_of_matching_patterns=get_matching_patterns(entry,matching_patterns[i]);
         if (number_of_matching_patterns==0 && nothing_before) {
            free_bit_array(matching_patterns[i]);
            matching_patterns[i]=NULL;
         }
      }
      /* At the opposite of DLC lines, a compound word tag like "{all around,.ADV}"
       * does not need to be put in the compound word tree, since the tag is already
       * characterized by its token number. */
      free_dic_entry(entry);
   }
}
}

