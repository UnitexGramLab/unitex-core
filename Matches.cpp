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
#include "Text_parsing.h"
#include "Matches.h"
//---------------------------------------------------------------------------



struct liste_matches *liste_match=NULL;
int nombre_match=0;
int nombre_output=0;
int longueur_avant=0;
int longueur_apres=0;
int statut_match;
int transduction_mode;
int ambig_transduction_mode = IGNORE_AMBIG_TRANSDUCTIONS; // reset in Text_parsing.cpp
                                                          // according to transduction_mode
int SEARCH_LIMITATION=-1;

int start_position_last_printed_match=-1; // used in ecrire_index_des_matches
int end_position_last_printed_match=-1;



void init_matches() {
liste_match=NULL;
nombre_match=0;
nombre_output=0;
longueur_avant=0;
longueur_apres=0;
start_position_last_printed_match=-1;
end_position_last_printed_match=-1;
}


/* Adds a match (struct liste_matches) to the global list of matches
   (struct liste_matches liste_match).
   # Changed to allow different outputs in merge/replace
   mode when the grammar is an ambiguous transducer (S.N.) */
void afficher_match_fst2(int fin,unichar* s) {

  int i;
  struct liste_matches *l;

  if (liste_match==NULL) {
    l=nouveau_match(s);
    l->debut=origine_courante+N_INT_ALLREADY_READ;
    l->fin=fin;
    l->suivant=NULL;
    liste_match=l;
  }
  else {

    switch (statut_match) {
      case LONGUEST_MATCHES: {
        // places new matches in front of the list
        if (liste_match->fin<fin) { // consider only matches ending later
          // only one match per range (start and end position)
          if (liste_match->debut==origine_courante+N_INT_ALLREADY_READ) {
            // overwrite matches starting at same position but ending earlier
            // We do this by deleting the actual head element of the list
            // and calling the function recursively.
            // This works also for different outputs from ambiguous transducers,
            // i.e. we may delete more than one match in the list "liste_match"
            l=liste_match;
            liste_match=liste_match->suivant;
            free(l);
            afficher_match_fst2(fin,s);
          }
          else {
            // add shorter matches but with other start position 
            l=nouveau_match(s);
            l->debut=origine_courante+N_INT_ALLREADY_READ;
            l->fin=fin;
            l->suivant=liste_match;
            liste_match=l;
          }
        }
        // for ambiguous transducers matches with same range
        // but different outputs are possible
        else if ((ambig_transduction_mode==ALLOW_AMBIG_TRANSDUCTIONS)
                 && (liste_match->fin==fin)
                 && (liste_match->debut==origine_courante+N_INT_ALLREADY_READ)
                 && u_strcmp(liste_match->output,s)) {
          // because matches with same range and same output may not come
          // one after another, we have to look if a match with same output already exists
          l=liste_match;
          while ((l!=NULL)
                 && u_strcmp(l->output,s))
            l=l->suivant;
          if (l==NULL) {
            l=nouveau_match(s);
            l->debut=origine_courante+N_INT_ALLREADY_READ;
            l->fin=fin;
            l->suivant=liste_match;
            liste_match=l;
          }
        }
      }
        break;
      case ALL_MATCHES: {
        // appends new matches to the head of the list
        l=liste_match;
        while ((l!=NULL)
               // unify matches, i.e. skip matches with same range (start and end)
               && !((l->debut==origine_courante+N_INT_ALLREADY_READ)
                    && (l->fin==fin)
                    && (ambig_transduction_mode!=ALLOW_AMBIG_TRANSDUCTIONS
                        // for ambig. tr. allow if the output is different:
                        || u_strcmp(l->output,s))))
          l=l->suivant;
        if (l==NULL) {
          l=nouveau_match(s);
          l->debut=origine_courante+N_INT_ALLREADY_READ;
          l->fin=fin;
          l->suivant=liste_match;
          liste_match=l;
        }
        break;
      }
      case SHORTEST_MATCHES: {
        // adds new matches to the head of the list
        i=0;
        liste_match=eliminer_shortest_match_fst2(liste_match,fin,&i,s);
        if (i==0) {
          l=nouveau_match(s);
          l->debut=origine_courante+N_INT_ALLREADY_READ;
          l->fin=fin;
          l->suivant=liste_match;
          liste_match=l;
        }
      }
        break;
    }
  }
}


struct liste_matches* nouveau_match(unichar* s) {
  struct liste_matches *l;
  l=(struct liste_matches*)malloc(sizeof(struct liste_matches));
  if (s==NULL) {
    l->output=NULL;
  } else {
    l->output=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(s)));
    u_strcpy(l->output,s);
  }
  l->suivant=NULL;
  return l;
}



/* for given actual match:
     1. checks if there are "longer" matches in the list and eliminates them
     2. if there is no "shorter" match in the list, adds the actual match to the list
   # added for support of ambiguous transducers:
     3. matches with same range but different output are also accepted
 */
struct liste_matches* eliminer_shortest_match_fst2(struct liste_matches *ptr,
                                                   int fin,int *i,unichar* output) {
  struct liste_matches *l;
  if (ptr==NULL)
    return NULL;
  if ((ambig_transduction_mode==ALLOW_AMBIG_TRANSDUCTIONS)
      && (ptr->debut==origine_courante+N_INT_ALLREADY_READ)&&(ptr->fin==fin)
      && u_strcmp(ptr->output,output)) {
    // in the case of ambiguous transductions producing different output
    // we accept matches with same range
    ptr->suivant=eliminer_shortest_match_fst2(ptr->suivant,fin,&(*i),output);
    return ptr;
  }
  else if ((ptr->debut<=origine_courante+N_INT_ALLREADY_READ)&&(ptr->fin>=fin)) {
    // actual match is shorter (or of equal length) than that in list
    // ==> replace match in list by actual match
    if (*i) {
      l=ptr->suivant;
      free_liste_matches(ptr);
      return eliminer_shortest_match_fst2(l,fin,&(*i),output);
    } else {
      ptr->debut=origine_courante+N_INT_ALLREADY_READ;
      ptr->fin=fin;
      if (ptr->output!=NULL) free(ptr->output);
      if (output==NULL) {
        ptr->output=NULL;
      }
      else {
        ptr->output=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(output)));
        u_strcpy(ptr->output,output);
      }
      (*i)=1;
      ptr->suivant=eliminer_shortest_match_fst2(ptr->suivant,fin,&(*i),output);
      return ptr;
    }
  }
  else if ((ptr->debut>=origine_courante+N_INT_ALLREADY_READ)&&(ptr->fin<=fin)) {
    // actual match is longer than that in list: skip actual match
    (*i)=1;
    return ptr;
  }
  else {
    // disjunct ranges or overlapping ranges without inclusion
    ptr->suivant=eliminer_shortest_match_fst2(ptr->suivant,fin,&(*i),output);
    return ptr;
  }
}


/* writes the matches to the file concord.ind
   the matches are in left-most longest order
=> wrong results for all matches when modifying text ??
<E>/[[ <MIN>* <PRE> <MIN>* <E>/]]
  left-most stehen am Anfang der Liste
*/
struct liste_matches* ecrire_index_des_matches(struct liste_matches *l,int pos,
                                               long int *N,FILE* fichier_match) {
struct liste_matches *ptr;
unichar tmp[100];
if (l==NULL) return NULL;
if (l->fin<pos) {
   // we can save the match (necessary for SHORTEST_MATCHES: there may be no shorter match)
   u_int_to_string(l->debut,tmp);
   u_fprints(tmp,fichier_match);
   u_fprints_char(" ",fichier_match);
   u_int_to_string(l->fin,tmp);
   u_fprints(tmp,fichier_match);
   if (l->output!=NULL) {
      // if there is an output
      u_fprints_char(" ",fichier_match);
      u_fprints(l->output,fichier_match);
   }
   u_fprints_char("\n",fichier_match);

   if (ambig_transduction_mode == ALLOW_AMBIG_TRANSDUCTIONS) {
     nombre_output++;
     // if we allow different output for ambiguous transducers,
     // we have to distinguish between matches and outputs
     // The algorithm is based on the following considerations:
     //  - l (resp. liste_match) has all matches with same starting point in one block,
     //    because they are inserted in one turn (Locate runs from left to right through the text)
     //  - since we consider only matches right from actual position,
     //    matches with same range (start and end position) always follow consecutively.
     //  - the start and end positions of the last printed match are stored in globals
     //  - if the range differs (start and/or end position are different),
     //    a new match is counted
     if (!(start_position_last_printed_match == l->debut
           && end_position_last_printed_match == l->fin))
       nombre_match++;
   }
   else
     nombre_match++;
   // To count the number of matched tokens this won't work: 
   //   nombre_unites_reconnues=nombre_unites_reconnues+(l->fin+1)-(l->debut);
   // or you get ouputs like:
   //  1647 matches
   //  4101 recognized units
   //  (221.916% of the text is covered)
   // because of overlapping matches or the option "all matches" is choosed.
   // For options "shortest" and "longest matches" the globals for last start and end
   // position are sufficient to calculate the correct coverage.
   // For all matches this is not the case. Suppose you have the matches at token pos:
   //  0 1 2 3 4 5
   //  XXX
   //    YYY
   //  ZZZZZ
   // The resulting concord.ind file will look like (for sort ordering see above)
   //   0 1 X
   //   1 2 Y
   //   0 2 Z
   // So when processing match Z we don't know, that token 0 has been already counted.
   // I guess an bit array is needed to count correctly.
   // But since for "longest matches" only Z, and for "shortest" only X and Y are
   // accepted, and the option "all matches" is rarely used, I (S.N.) propose:
   if (end_position_last_printed_match != pos-1) {
     // initial (non-recursive) call of function:
     // then check if match is out of range of previous matches
     if (end_position_last_printed_match < l->debut) { // out of range
       nombre_unites_reconnues += (l->fin+1)-(l->debut);
     }
     else {
       nombre_unites_reconnues += (l->fin+1)-(end_position_last_printed_match+1);
     }
   }
   // else:
   //  recursive call, i.e. end position of match was already counted:
   //  for "longest" and "shortest" matches all is done, for option "all"
   //  it is possible that a token won't be counted (in the above example,
   //  when there is no match X), this will lead to an incorrect displayed
   //  coverage, lower than correct.
   else {
     // this may make the coverage greater than correct:
     if (start_position_last_printed_match > l->debut)
       nombre_unites_reconnues += start_position_last_printed_match-(l->debut);
   }
   start_position_last_printed_match = l->debut;
   end_position_last_printed_match = l->fin;

   if (nombre_match==SEARCH_LIMITATION) {
      // if we have reached the search limitation, we free the remaining
      // matches and return
      while (l!=NULL) {
         ptr=l;
         l=l->suivant;
         free_liste_matches(ptr);
      }
      return NULL;
   }

   ptr=l->suivant;
   free_liste_matches(l);
   return ecrire_index_des_matches(ptr,pos,N,fichier_match);
}
l->suivant=ecrire_index_des_matches(l->suivant,pos,N,fichier_match);
return l;
}



struct liste_matches* load_match_list(FILE* f,int *TRANDUCTION_MODE) {
struct liste_matches* l=NULL;
struct liste_matches* end_of_list=l;
int c;
int start,end;
unichar output[3000];
char is_an_output;
u_fgetc(f);   // we jump the # char
c=u_fgetc(f);
if (TRANDUCTION_MODE!=NULL) {
	switch(c) {
		case 'I': *TRANDUCTION_MODE=IGNORE_TRANSDUCTIONS; break;
		case 'M': *TRANDUCTION_MODE=MERGE_TRANSDUCTIONS; break;
		case 'R': *TRANDUCTION_MODE=REPLACE_TRANSDUCTIONS; break;
	}
}
u_fgetc(f);
while ((c=u_fgetc(f))!=EOF) {
   start=0;
   do {
      start=start*10+(c-'0');
   } while (u_is_digit((unichar)(c=u_fgetc(f))));
   c=u_fgetc(f);
   end=0;
   do {
      end=end*10+(c-'0');
   } while (u_is_digit((unichar)(c=u_fgetc(f))));
   if (c==' ') {
      // if we have an output to read
      int i=0;
      while ((c=u_fgetc(f))!='\n') {
         output[i++]=(unichar)c;
      }
      output[i]='\0';
      is_an_output=1;
   } else {
      is_an_output=0;
   }
   if (end_of_list==NULL) {
      if (is_an_output)
         l=nouveau_match(output);
      else l=nouveau_match(NULL);
      l->debut=start;
      l->fin=end;
      end_of_list=l;
   } else {
      if (is_an_output)
         end_of_list->suivant=nouveau_match(output);
      else end_of_list->suivant=nouveau_match(NULL);
      end_of_list->suivant->debut=start;
      end_of_list->suivant->fin=end;
      end_of_list=end_of_list->suivant;
   }
}
return l;
}



void free_liste_matches(struct liste_matches* l) {
if (l->output!=NULL) free(l->output);
free(l);
}


