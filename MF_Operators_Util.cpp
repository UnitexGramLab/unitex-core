/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "MF_Operators_Util.h"
#include "Error.h"


/*
 * fonction comparer  qu'un code de sortie est conforme ou non aux filtres
 */


int compare(unichar *tmp,unichar **filtre) {
int i,res;

    res=0;i=1;
    while(filtre[i]!=NULL && !res) {
        if (u_strcmp(tmp,filtre[i]) == 0) res = 1;
        i++;
    }
    if (filtre[0][0] == 'p') return res;
    else if (filtre[0][0] == 'n') return (1-res);
    return 0; // evite warning
}

int filtrer(unichar* sortie, unichar** filtre) {
unichar result[L2],tmp[L1];
int i,j,l,retour;

    i=0; j = 0; retour=1; result[0]='\0';
    if (sortie[i] == ':' ) i++;
    else  {//fprintf(stderr,"erreur Sortie sans : -->");
    //u_fprintf(stderr,"%S",sortie);
    //fprintf(stderr,"\n");
    retour=0;}

    l = u_strlen(sortie);
    while( i <= l ) {
      if (sortie[i] != ':' && sortie[i] != '\0') tmp[j++] = sortie[i++];
      else {
        tmp[j] = '\0'; i++; j = 0;
        if (compare(tmp,filtre)) {
           u_strcat(result,":");
           u_strcat(result,tmp);
        }
      }
    }
u_strcpy(sortie,result);

return retour;
}


/********************* Fonctions operateurs *****************************/

int is_var_op(unichar *s, int pos) {

  if (pos > 0 && u_is_digit(s[pos]) && (s[pos-1] == '$' || s[pos-1] == POUND)) return 1;
  if (pos >= 0 && (s[pos] == '$' || s[pos] == POUND)) return 1;
  return 0;
}

int get_var_op(unichar *var, unichar *s, int *pos) {
int i;
  i = *pos;
  if (i > 0 && u_is_digit(s[i]) && (s[i-1] == '$' || s[i-1] == POUND)) {
     var[1] = s[i]; var[0] = s[i-1]; var[2] = '\0';
     (*pos)--;
     return 1;
  }
  else if (i >= 0 && (s[i] == '$' || s[i] == POUND)) {
     int k=0;
     var[k++] = s[i];
     if (u_is_digit(s[i+1])) var[k++] = s[i+1];
     var[k] = '\0';
     return 1;
  }
  else { var[0] = '\0';
     return 0;
  }
}

void aff_flag_var(unsigned int flags) {
unsigned int i,res;
 u_printf("flags:|");
  for ( i = 0; i < 32; i++) {
  res = flags & 01;
  putchar(res+48);
  flags >>= 1;
  }
 u_printf("|\n");
}

unsigned int get_flag_var(int indice,unsigned int flags) {
unsigned int res;

  res = ((flags >> indice) & 01);
  return res;
}

void add_flag_var(int indice,unsigned int *flags) {
unsigned int res=1;

   res <<= indice;
   (*flags) |= res;
}

int get_indice_var_op(unichar *var) {
int indice = 0;
  if (var[0] == POUND ) indice += 11;
  if (var[1] != '\0' ) indice = indice + var[1] - '0' + 1 ;
return indice;
}

void init_var(unichar *var_name,unichar (*Variables)[L1],unichar *pile, int debut, int fin) {
int i,k,ind;

  ind = get_indice_var_op(var_name);
  if (debut >= 0 ) {
     for (k=0, i = debut; i <= fin  ; i++,k++) Variables[ind][k] =  pile[i];
     Variables[ind][k]='\0';
  }
  else Variables[ind][0]='\0';
}


int get_factor(unichar *factor, unichar *s, int *pos,unsigned int *mode) {
int i,j,k;
unichar tmp[L1];

 *mode=0;
 i = *pos; k = 0;
 while (i > 0 && !is_var_op(s,i) && s[i] != '\'') {
  tmp[k++] = s[i--];
 }
 if (k == 0 && s[i] == '\'') {
     i--;
     while (i > 0 && s[i] != '\'') {
     tmp[k++] = s[i--];
     }
     if (s[i] == '\'') {*mode = PROTEGE;i--;}
 }

 if ( k == 0) return 0;
 tmp[k]= '\0';
 for (j= 0 ; j < k; j++) factor[j] = tmp[k -j -1];
 factor[j] = '\0';
 *pos = i+1;

 return 1;
}

int get_pos_factor(unichar *pile,int *pos,unichar *facteur,unsigned int match_type,int *pos_match) {
int l,pos_test,last_pos_match,last_pos,parcours_fact,parcours_pile,MATCH,BEGIN;
unsigned int protege;
  last_pos = 0;
  protege = match_type; match_type &= 3; protege >>= 2;


  BEGIN = 0;l=0;
  if (facteur[0]=='^') { l=0;
     while (facteur[l] != '\0') {
        facteur[l] = facteur[l+1];
        l++;
     }
     BEGIN = 1;
  }
  last_pos_match = -1; *pos_match = -1;
  l = u_strlen(facteur);
  pos_test = *pos  -l + 1;
  if (VERBOSE) fprintf(stderr,"l = %d\n",l);

  if ( pos_test < 0 ) {
    if (VERBOSE) fprintf(stderr,"facteur > pile \n");
    return 0;
  }
  while ( pos_test >= 0 ) {
    parcours_pile = pos_test;
    parcours_fact = 0;
    while (parcours_fact < l && pile[parcours_pile] == facteur[parcours_fact]){
      if (VERBOSE) fprintf(stderr,"Equality: %c\n",pile[parcours_pile]);
      parcours_pile++; parcours_fact++;
    }
    if (VERBOSE) fprintf(stderr,"parcours_fact = %d\n",parcours_fact);
    if ( parcours_fact == l ) MATCH = 1; else MATCH = 0;

    if ( match_type == IMMEDIAT ) {
      if (MATCH) {
          if (!protege) *pos = pos_test;
          *pos_match = pos_test + l - 1;
                   if (!BEGIN) return 1;
                   else {if (*pos == 0) return 1;
                         else return 0;
                   }
      }
      else return 0;
    }
    else if ( match_type == SHORTEST ) {
      if (MATCH) {*pos = pos_test; *pos_match = pos_test + l - 1; return 1;}
      else pos_test--;
    }
    else if ( match_type == LONGEST ) {
      if (MATCH) { last_pos_match = pos_test; last_pos = pos_test; }
      pos_test--;
    }
  }
  if ( match_type == LONGEST && last_pos_match != -1) {
       *pos_match = last_pos_match + l - 1;
       *pos = last_pos; return 1;
  }
  else return 0;
}

int flex_op_with_var(unichar (*Variables)[L1],unichar *pile,unichar *etiq,int *pos,int *pos_etiq,unsigned int *var_in_use){
int ind,init_pos,pos_pattern,var_precede,match_type,retour,pos_match,var_end;
unichar var_name[L1],facteur[L1];
unsigned int mode=0;
 match_type = 0;
 if (!u_strcmp(etiq,"<!>")) {
    *pos_etiq +=3;
    return 1;
}

    init_pos = *pos;
    if (VERBOSE) error("PATTERN: %s\n",etiq);
    while (etiq[*pos_etiq] != '\0' && etiq[(*pos_etiq)++] != '>' ) {} // Valeur de retour
    pos_pattern = *pos_etiq -2;
    var_precede = 0;
    retour = 0;

    while (etiq[pos_pattern] != '<' ) {
       //if (VERBOSE) u_fprintf(stderr,"***DEBUT***\n");
      if (get_var_op(var_name,etiq,&pos_pattern)) {
    //if (VERBOSE) u_fprintf(stderr,"GET Variable name  : %S\n",var_name);
	 if (is_var_op(etiq,pos_pattern-1) || etiq[pos_pattern-1] == '<') {
         // si <$  ou <$1$  ou (POUND?) $ ....on capte le contenu de la variable
             (*pos)--;
             var_end = *pos;
             if (var_name[0] == '$' ) {pos_match = var_end-1;}
	     else { pos_match = -1; *pos = 0;}
     // if (VERBOSE) u_fprintf(stderr,"pos_match = %d var_end = %d \n",pos_match,var_end);
             init_var(var_name,Variables,pile,pos_match + 1,var_end);
             ind = get_indice_var_op(var_name);
             add_flag_var(ind,var_in_use);
     // if (VERBOSE) u_fprintf(stderr,"SPECIAL pos = %d  pos_pattern =  %dVariable %S = %S\n",*pos,pos_pattern,var_name,Variables[ind]);
            if (*pos == -1) *pos = 0;
            retour = 1;
          }
	 else {var_precede = 1;}

      pos_pattern--;
      }
      if (get_factor(facteur,etiq,&pos_pattern,&mode) ) {
       // if (VERBOSE) u_fprintf(stderr,"FACTOR facteur : %S\n",facteur);
         if (var_precede == 0) match_type = IMMEDIAT;
         else if (var_name[0] == '$') match_type = SHORTEST;
         else if (var_name[0] == POUND) match_type = LONGEST;
         match_type |= mode;
         if (VERBOSE) fprintf(stderr,"var_name[0]:%c match_type: %d\n",var_name[0],match_type);
         pos_match = 0;
         int tmp_pos = 0;
         if (mode == PROTEGE) tmp_pos = (*pos);
         (*pos)--;
     // if (VERBOSE) u_fprintf(stderr,"APPEL pos:%d match_type:%d facteur: %s\n",*pos,match_type,facteur);
         var_end = *pos;
         retour = get_pos_factor(pile,pos,facteur,match_type,&pos_match);
         if (mode == PROTEGE)  (*pos) = tmp_pos;
	 if (retour == 0) { *pos = init_pos; return retour;}
     //if (VERBOSE) u_fprintf(stderr,"RETOUR %d pos:%d match_type:%d facteur:%s pos_match %d\n",retour,*pos,match_type,facteur,pos_match);
         if (var_precede) {
            init_var(var_name,Variables,pile,pos_match + 1,var_end);
            ind = get_indice_var_op(var_name);
            add_flag_var(ind,var_in_use);
   //if (VERBOSE) u_fprintf(stderr,"Variable  %S = %S\n",var_name,Variables[ind]);
         }
         var_precede = 0;
         pos_pattern--;
      }
    }
   //if (VERBOSE) u_fprintf(stderr,"FIN TRAITEMENT PATTERN\n");
    if (retour == 0) *pos = init_pos;
return retour;
}

