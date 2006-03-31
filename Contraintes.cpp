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
#include "Contraintes.h"
//---------------------------------------------------------------------------

struct contrainte** contrainte1=NULL;
struct contrainte** contrainte2=NULL;
struct contrainte** etiq=NULL;

struct contrainte* new_contrainte() {
struct contrainte* c;
c=(struct contrainte*)malloc(sizeof(struct contrainte));
c->controle=0;
c->raw=NULL;
c->flechi=NULL;
c->canonique=NULL;
c->codes_gramm=NULL;
c->n_codes_gramm=0;
c->codes_flex=NULL;
c->n_codes_flex=0;
c->contrainte_alternative=NULL;
return c;
}



void free_contrainte(struct contrainte* c) {
if (c==NULL) return;
if (c->raw!=NULL) free(c->raw);
if (c->flechi!=NULL) free(c->flechi);
if (c->canonique!=NULL) free(c->canonique);
if (c->codes_gramm!=NULL) {
   for (int i=0;i<c->n_codes_gramm;i++)
      if (c->codes_gramm[i]!=NULL) free(c->codes_gramm[i]);
   free(c->codes_gramm);
}
if (c->codes_flex!=NULL) {
   for (int i=0;i<c->n_codes_flex;i++)
      if (c->codes_flex[i]!=NULL) free(c->codes_flex[i]);
   free(c->codes_flex);
}
if (c->contrainte_alternative!=NULL) free_contrainte(c->contrainte_alternative);
free(c);
}



int negation(struct contrainte* c) {
return (c->controle & 2);
}



void set_negation(struct contrainte* c) {
c->controle=(char)(c->controle | 2);
}



int variantes_min_maj(struct contrainte* c) {
return (c->controle & 4);
}



void set_variantes_min_maj(struct contrainte* c) {
c->controle=(char)(c->controle | 4);
}



void decouper_infos_gramm(unichar* s,struct contrainte* c) {
if (s==NULL) return;
unichar* codes_gramm[50];
unichar* codes_flex[50];
int n_codes_gramm=0;
int n_codes_flex=0;
int i;
int j;
unichar temp[3000];
i=0;
// reading the semantic codes
while (i==0 || s[i]=='+') {
  if (i!=0) i++;
  j=0;
  while (s[i]!='\0' && s[i]!='+' && s[i]!=':') {
    if (s[i]=='\\') {
      i++;
      if (s[i]=='\0') {
        return;
      }
    }
    temp[j++]=s[i++];
  }
  temp[j]='\0';
  codes_gramm[n_codes_gramm]=(unichar*)malloc(sizeof(unichar)*(u_strlen(temp)+1));
  u_strcpy(codes_gramm[n_codes_gramm],temp);
  n_codes_gramm++;
}
if (n_codes_gramm!=0) {
   c->codes_gramm=(unichar**)malloc(sizeof(unichar*)*n_codes_gramm);
   c->n_codes_gramm=n_codes_gramm;
   for (j=0;j<n_codes_gramm;j++)
     c->codes_gramm[j]=codes_gramm[j];
}

// reading the inflectional codes
while (s[i]==':') {
  i++;
  j=0;
  while (s[i]!='\0' && s[i]!=':') {
    if (s[i]=='\\') {
      i++;
      if (s[i]=='\0') {
        return;
      }
    }
    temp[j++]=s[i++];
  }
  temp[j]='\0';
  codes_flex[n_codes_flex]=(unichar*)malloc(sizeof(unichar)*(u_strlen(temp)+1));
  u_strcpy(codes_flex[n_codes_flex],temp);
  n_codes_flex++;
}
if (n_codes_flex!=0) {
   c->codes_flex=(unichar**)malloc(sizeof(unichar*)*n_codes_flex);
   c->n_codes_flex=n_codes_flex;
   for (j=0;j<n_codes_flex;j++)
     c->codes_flex[j]=codes_flex[j];
}
}



struct contrainte* construire_contrainte_depuis_etiquette(struct fst2Tag* e) {
if (e==NULL) return NULL;
struct contrainte* c=new_contrainte();
if (e->flechi==NULL && e->canonique==NULL && e->infos_gramm==NULL) {
   // if the tag is a raw one
   if (e->controle & 4) {
      // if the min/maj variants are allowed
      set_variantes_min_maj(c);
   }
   if (e->contenu[0]=='<' && e->contenu[1]=='!') {
      set_negation(c);
      c->raw=(unichar*)malloc(6*sizeof(unichar));
      if (!u_strcmp_char(e->contenu,"<!MOT>")) u_strcpy_char(c->raw,"<MOT>");
      else if (!u_strcmp_char(e->contenu,"<!DIC>")) u_strcpy_char(c->raw,"<DIC>");
      else if (!u_strcmp_char(e->contenu,"<!MAJ>")) u_strcpy_char(c->raw,"<MAJ>");
      else if (!u_strcmp_char(e->contenu,"<!MIN>")) u_strcpy_char(c->raw,"<MIN>");
      else if (!u_strcmp_char(e->contenu,"<!PRE>")) u_strcpy_char(c->raw,"<PRE>");
      return c;
   }
   c->raw=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(e->contenu)));
   u_strcpy(c->raw,e->contenu);
   return c;
}
if (e->controle & 2) set_negation(c);
if (e->flechi!=NULL && e->canonique==NULL && e->infos_gramm==NULL) {
   // we can't formally distinguish <manger> and <N>; so when we find
   // such a tag, we have <manger> with canonique!=NULL and we
   // build <N> with the following code
   c->contrainte_alternative=new_contrainte();
   decouper_infos_gramm(e->flechi,c->contrainte_alternative);
   if (negation(c)) set_negation(c->contrainte_alternative);
   c->canonique=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(e->flechi)));
   u_strcpy(c->canonique,e->flechi);
   return c;
}
if (e->flechi!= NULL && e->canonique!=NULL) {
   c->flechi=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(e->flechi)));
   u_strcpy(c->flechi,e->flechi);
   c->canonique=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(e->canonique)));
   u_strcpy(c->canonique,e->canonique);
}
decouper_infos_gramm(e->infos_gramm,c);
return c;
}



struct contrainte* construire_contrainte2_depuis_string(unichar* s) {
if (s==NULL || s[0]=='\0') return NULL;
struct contrainte* c=new_contrainte();
if (s[0]!='<' && s[0]!='{') {
   // case of a raw constraint
   c->raw=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(s)));
   u_strcpy(c->raw,s);
   return c;
}
unichar temp[1000];
int i,j;
j=0;
i=1;
if (s[i]=='!') {
  i++;
  set_negation(c);
}
while ((s[i]!=',') && (s[i]!='.') && (s[i]!='>') && (s[i]!='}'))
  temp[j++]=s[i++];
temp[j]='\0';
// cas <avoir>, <V> ou <MOT>
if (s[i]=='>') {
  if (!u_strcmp_char(temp,"MOT")) {
     c->raw=(unichar*)malloc(6*sizeof(unichar));
     u_strcpy_char(c->raw,"<MOT>");
     return c;
  }
  if (!u_strcmp_char(temp,"DIC")) {
     c->raw=(unichar*)malloc(6*sizeof(unichar));
     u_strcpy_char(c->raw,"<DIC>");
     return c;
  }
  if (!u_strcmp_char(temp,"MAJ")) {
     c->raw=(unichar*)malloc(6*sizeof(unichar));
     u_strcpy_char(c->raw,"<MAJ>");
     return c;
  }
  if (!u_strcmp_char(temp,"MIN")) {
     c->raw=(unichar*)malloc(6*sizeof(unichar));
     u_strcpy_char(c->raw,"<MIN>");
     return c;
  }
  if (!u_strcmp_char(temp,"PRE")) {
     c->raw=(unichar*)malloc(6*sizeof(unichar));
     u_strcpy_char(c->raw,"<PRE>");
     return c;
  }
  // on a <avoir> ou <V>
  c->canonique=(unichar*)malloc(sizeof(unichar)*(j+1));
  u_strcpy(c->canonique,temp);
  // dans ce cas, on prevoit la deuxieme contrainte:
  c->contrainte_alternative=new_contrainte();
  decouper_infos_gramm(c->canonique,c->contrainte_alternative);
  if (negation(c)) set_negation(c->contrainte_alternative);
  return c;
}
// cas <eu,avoir.V>
if (s[i]==',') {
  c->flechi=(unichar*)malloc(sizeof(unichar)*(j+1));
  u_strcpy(c->flechi,temp);
  i++;
  j=0;
  while ((s[i]!='.') && (s[i]!='>') && (s[i]!='}'))
    temp[j++]=s[i++];
  temp[j]='\0';
  c->canonique=(unichar*)malloc(sizeof(unichar)*(j+1));
  u_strcpy(c->canonique,temp);
  if (s[i]=='>' || s[i]=='}') {
     return c;
  }
  i++;
  j=0;
  while (s[i]!='>' && s[i]!='}')
    temp[j++]=s[i++];
  temp[j]='\0';
  decouper_infos_gramm(temp,c);
  return c;
}
// cas <avoir.N>
c->canonique=(unichar*)malloc(sizeof(unichar)*(j+1));
u_strcpy(c->canonique,temp);
i++;
j=0;
while (s[i]!='>' && s[i]!='}')
  temp[j++]=s[i++];
temp[j]='\0';
decouper_infos_gramm(temp,c);
return c;
}



struct contrainte** allouer_contraintes(int n) {
struct contrainte** c;
c=(struct contrainte**)malloc(n*sizeof(struct contrainte*));
for (int i=0;i<n;i++)
  c[i]=NULL;
return c;
}



void free_contraintes(int n,struct contrainte** c) {
int i;
for (i=0;i<n;i++)
   free_contrainte(c[i]);
free(c);
}



void calculer_contraintes(Automate_fst2* text,Automate_fst2* grammar) {
int i;
for (i=0;i<text->nombre_etiquettes;i++) {
   etiq[i]=construire_contrainte_depuis_etiquette(text->etiquette[i]);
}
for (i=0;i<grammar->nombre_etiquettes;i++) {
   contrainte1[i]=construire_contrainte_depuis_etiquette(grammar->etiquette[i]);
   if (grammar->etiquette[i]->transduction!=NULL) {
      contrainte2[i]=construire_contrainte2_depuis_string(grammar->etiquette[i]->transduction);
   }
}
}



//
// this function checks if a tag of the text automaton is matched by a couple of  constraints
// it returns 1 if the first constraint is verified, 0 else
// the parameter c2_verifiee takes the value 1 if the second constraint is verified, 0 else
//
int contrainte1_verifiee(struct contrainte* etiq_FST,struct contrainte* c1,
                         struct contrainte* c2,int* c2_verifiee,Alphabet* alph) {
(*c2_verifiee)=contrainte_verifiee(etiq_FST,c2,alph);
return contrainte_verifiee(etiq_FST,c1,alph);
}



//
// this function checks if a tag of the text automaton is matched by a single constraint
// it returns 1 on success, 0 else
//
int contrainte_verifiee(struct contrainte* etiq_FST,struct contrainte* C,Alphabet* alph) {
if (C==NULL) {
   // if there is no constraint (typically with an <E> transition), we return true
   return 1;
}
if (etiq_FST->raw!=NULL) {
   // if we have a non dic tag in the text automaton
   if (C->raw==NULL) {
      // the only possibility to match a raw tag is to have a raw tag, so we can return false
      return 0;
   }
   if (C->raw[0]=='<' && C->raw[1]!='\0') {
      // we have a meta (<MOT>, <MIN>, <MAJ>, <DIC> or <PRE>) with or without negation
      if (!is_letter(etiq_FST->raw[0],alph)) {
         // if the text automaton tag is not a letter sequence
         return negation(C);
      }
      // there we know that we have a letter sequence tag
      if (!u_strcmp_char(etiq_FST->raw,"<MOT>")) {
         // case of <MOT>
         return !negation(C);
      }
      if (!u_strcmp_char(etiq_FST->raw,"<DIC>")) {
         // case of <DIC>
         return negation(C);
      }
      if (!u_strcmp_char(etiq_FST->raw,"<PRE>")) {
         // case of <PRE>
         int x=is_upper(etiq_FST->raw[0],alph);
         return (!negation(C) && x) ||
                (negation(C) && !x);
      }
      if (!u_strcmp_char(etiq_FST->raw,"<MIN>")) {
         // case of <MIN>
         int x=is_sequence_of_lowercase_letters(etiq_FST->raw,alph);
         return (!negation(C) && x) ||
                (negation(C) && !x);
      }
      if (!u_strcmp_char(etiq_FST->raw,"<MAJ>")) {
         // case of <MAJ>
         int x=is_sequence_of_uppercase(etiq_FST->raw,alph);
         return (!negation(C) && x) ||
                (negation(C) && !x);
      }
      // we should never arrive here
      return 0;
   }
   // there we have raw tags in both text automaton and grammar
   if (variantes_min_maj(C)) {
      // if there can be some min/maj variants
      int x=is_equal_or_uppercase(etiq_FST->raw,C->raw,alph);
      return (x && !negation(C)) ||
             (!x && negation(C));
   }
   else {
      int x=u_strcmp(etiq_FST->raw,C->raw);
      return (!x && !negation(C)) ||
             (x && negation(C));
   }
}

// there, we have a DELA tag in the text automaton
if (C->raw!=NULL) {
   // we have a raw tag in the grammar but not in the text automaton
   if (C->raw[0]=='<' && C->raw[1]!='\0') {
      // we have a meta (<MOT>, <MIN>, <MAJ>, <DIC> or <PRE>) with or without negation
      if (!is_letter(etiq_FST->flechi[0],alph)) {
         // if the text automaton tag is not a letter sequence
         return negation(C);
      }
      // there we know that we have a letter sequence tag
      if (!u_strcmp_char(etiq_FST->flechi,"<MOT>")) {
         // case of <MOT>
         return !negation(C);
      }
      if (!u_strcmp_char(etiq_FST->flechi,"<DIC>")) {
         // case of <DIC>
         return negation(C);
      }
      if (!u_strcmp_char(etiq_FST->flechi,"<PRE>")) {
         // case of <PRE>
         int x=is_upper(etiq_FST->flechi[0],alph);
         return (!negation(C) && x) ||
                (negation(C) && !x);
      }
      if (!u_strcmp_char(etiq_FST->flechi,"<MIN>")) {
         // case of <MIN>
         int x=is_sequence_of_lowercase_letters(etiq_FST->flechi,alph);
         return (!negation(C) && x) ||
                (negation(C) && !x);
      }
      if (!u_strcmp_char(etiq_FST->flechi,"<MAJ>")) {
         // case of <MAJ>
         int x=is_sequence_of_uppercase(etiq_FST->flechi,alph);
         return (!negation(C) && x) ||
                (negation(C) && !x);
      }
      // we should never arrive here
      return 0;
   }
   // there we have raw tags in both text automaton and grammar
   if (variantes_min_maj(C)) {
      // if there can be some min/maj variants
      int x=is_equal_or_uppercase(etiq_FST->flechi,C->raw,alph);
      return (x && !negation(C)) ||
             (!x && negation(C));
   }
   else {
      int x=u_strcmp(etiq_FST->flechi,C->raw);
      return (!x && !negation(C)) ||
             (x && negation(C));
   }
}
// if we are there, we have a DELA tag in the text automaton and a non
// raw constraint in the grammar

if (C->flechi==NULL && C->canonique!=NULL && C->codes_gramm==NULL) {
   // we are in the case <le> or <DET>
   if (C->contrainte_alternative==NULL) {
      // we should never arrive here
      fprintf(stderr,"erreur: contrainte alternative NULL\n");
      return 0;
   }

   if (contrainte_verifiee(etiq_FST,C->contrainte_alternative,alph)) {
      // we check the alternative constraint <DET>
      return 1;
   }
   // we check the normal constraint <le>
   int x=!u_strcmp(etiq_FST->canonique,C->canonique);
   return (x && !negation(C)) ||
          (!x && negation(C));
}

if (C->flechi==NULL && C->canonique!=NULL && C->codes_gramm!=NULL) {
   // we are in the case <le.DET>
   int x=!u_strcmp(etiq_FST->canonique,C->canonique) &&
         infos_gramm_compatibles(etiq_FST,C);
   return (x && !negation(C)) ||
          (!x && negation(C));
}

if (C->flechi!=NULL && C->canonique!=NULL && C->codes_gramm!=NULL) {
   int x=is_equal_or_uppercase(etiq_FST->flechi,C->flechi,alph) &&
         !u_strcmp(etiq_FST->canonique,C->canonique) &&
         infos_gramm_compatibles(etiq_FST,C);
   return (x && !negation(C)) ||
          (!x && negation(C));
}

if (C->flechi==NULL && C->canonique==NULL && C->codes_gramm!=NULL) {
   int x=infos_gramm_compatibles(etiq_FST,C);
   return (x && !negation(C)) ||
          (!x && negation(C));
}
// we should never arrive here
printf("oooooooooops");
return 0;
}



//
// compares 2 grammatical tags like A = "DET+z1" and B = "DET"
// -> each tag of B must appear in A
//
int codes_gramm_compatibles(struct contrainte* etiq_FST,struct contrainte* C) {
int present;
for (int i=0;i<C->n_codes_gramm;i++) {
   present=0;
   for (int j=0;!present && j<etiq_FST->n_codes_gramm;j++) {
      if (!u_strcmp(etiq_FST->codes_gramm[j],C->codes_gramm[i])) present=1;
   }
   if (!present) return 0;
}
return 1;
}



//
// compares 2 single inflectional tags like A = "mp" and B = "m"
//
int single_codes_flex_compatibles(unichar* etiq,unichar* C) {
int present,L_etiq,L_C;
L_etiq=u_strlen(etiq);
L_C=u_strlen(C);
for (int i=0;i<L_C;i++) {
   present=0;
   for (int j=0;!present && j<L_etiq;j++) {
      if (etiq[j]==C[i]) present=1;
   }
   if (!present) return 0;
}
return 1;
}



//
// compares 2 inflectional tags like A = "mp:ms" and B = "m:f"
// -> a sequence of B must must included in a sequence of A
//
int codes_flex_compatibles(struct contrainte* etiq_FST,struct contrainte* C) {
if (C->n_codes_flex==0) {
   // if there is no constraint we return true
   return 1;
}
for (int i=0;i<C->n_codes_flex;i++) {
   for (int j=0;j<etiq_FST->n_codes_flex;j++) {
      if (single_codes_flex_compatibles(etiq_FST->codes_flex[j],C->codes_flex[i])) return 1;
   }
}
return 0;
}



//
// compares 2 tags like "DET+z1:mp" and "DET:m"
//
int infos_gramm_compatibles(struct contrainte* etiq_FST,struct contrainte* C) {
return codes_gramm_compatibles(etiq_FST,C) && codes_flex_compatibles(etiq_FST,C);
}



void afficher_contrainte(struct contrainte* c) {

if (c==NULL) {
   printf("c NULL");
   getchar();
   return;
}

printf("\nraw=");
if (c->raw!=NULL) u_prints(c->raw);
else printf("NULL");

printf("\nflechi=");
if (c->flechi!=NULL) u_prints(c->flechi);
else printf("NULL");

printf("\ncanonique=");
if (c->canonique!=NULL) u_prints(c->canonique);
else printf("NULL");

printf("\ncode gramm=");
for (int i=0;i<c->n_codes_gramm;i++) {
  u_prints(c->codes_gramm[i]);
  printf("_");
}

printf("\ncode flex=");
for (int i=0;i<c->n_codes_flex;i++) {
  u_prints(c->codes_flex[i]);
  printf("_");
}

printf("\ncontrainte alternative:");
if (c->contrainte_alternative==NULL) printf(" NULL");
else printf(" YES");
getchar();
}
